#!/usr/share/python3
# SPDX-License-Identifier: MIT
# Usage: python3 -i ./interact.py

import serial, time, re, struct, sys, random, socket, os

KiB = 1 << 10
MiB = 1 << 20
GiB = 1 << 30

def BIT(x):
    return 1 << x

def MASK(x):
    return BIT(x) - 1

def bswap16(x):
    x = (x & 0xff00ff00) >>  8 | (x & 0x00ff00ff) <<  8
    return x

def bswap32(x):
    x = (x & 0xffff0000) >> 16 | (x & 0x0000ffff) << 16
    x = (x & 0xff00ff00) >>  8 | (x & 0x00ff00ff) <<  8
    return x

def get_be16(data, offset):
    return data[offset] << 8 | data[offset+1]

def get_be32(data, offset):
    return get_be16(data, offset) << 16 | get_be16(data, offset+2)

def to_be24(n):
    return [n >> 16 & 0xff, n >> 8 & 0xff, n & 0xff]

def to_be32(n):
    return [n >> 24 & 0xff, n >> 16 & 0xff, n >> 8 & 0xff, n & 0xff]

def from_be32(data):
    word = 0
    for i, b in enumerate(data[:4]):
        word |= b << (3 - i) * 8
    return word

def to_le32(n):
    return [n & 0xff, n >> 8 & 0xff, n >> 16 & 0xff, n >> 24 & 0xff]

def from_le32(data):
    word = 0
    for i, b in enumerate(data[:4]):
        word |= b << i * 8
    return word

def hexdump(data):
    if data:
        for offset in range(0, len(data), 16):
            d = data[offset:offset+16]
            line = f'{offset:08x}:  '
            line += ' '.join([f'{x:02x}' for x in d]).ljust(49)
            line += ''.join([chr(x) if (x >= 0x20 and x <= 0x7f) else '.' for x in d])
            print(line)


def error(s):
    sys.stderr.write(s)
    sys.stderr.write('\n')
    sys.stderr.flush()

class Lolmon:
    def __init__(self, device):
        self.device = device
        self.s = serial.Serial(device, baudrate=115200, timeout=0.2)
        self.prompt = b'> '
        self.debug = 0
        self.echo_attempts = 3
        self.chunksize = 0x38

    def connection_test(self):
        self.s.write(b'\n')
        time.sleep(0.2)
        answer = self.s.read_all()
        if (b'\r\n' + self.prompt) in answer:
            print("lolmon detected!")

    def debug_log(self, prefix, s):
        if self.debug >= 2:
            error(f'{prefix}: {s}')
        return s

    def read_until_prompt(self):
        answer = bytearray()
        timeout = 1
        while True:
            if self.s.readable():
                answer += self.debug_log('until prompt', self.s.read_all())
                if self.prompt in answer:
                    return bytes(answer)[:-len(self.prompt)], True
            else:
                time.sleep(0.05)
                timeout -= 0.05

            if timeout < 0:
                return bytes(answer), False

    def flush(self):
        while self.debug_log('flush', self.s.read_all()) != b'':
            time.sleep(0.05)
        self.run_command('')

    def enter_with_echo(self, cmd, chunksize):
        if isinstance(cmd, str):
            cmd = cmd.encode('UTF-8')
        assert not b'\n' in cmd

        pos = 0
        while pos < len(cmd):
            chunk = cmd[pos:pos+chunksize]
            assert len(chunk) >= 1
            self.s.write(chunk)
            echo = self.s.read(len(chunk))
            if self.debug >= 2:
                error(f'input {chunk} -> {echo}')
            if echo != chunk:
                error(f'Echo error! {chunk} -> {echo}')
                error((echo + self.s.read_all()).decode('ascii'))
                return False
            pos += len(chunk)
        return True

    def run_command(self, cmd):
        try:
            chunksize = self.chunksize
            for _ in range(self.echo_attempts):
                if self.debug:
                    error(':> %s' % cmd)
                good = self.enter_with_echo(cmd, chunksize)
                if not good:
                    # Clear the prompt (send Ctrl-U)
                    self.s.write(b'\025')
                    time.sleep(0.01)
                    while self.s.read_all() != b'':
                        time.sleep(0.01)
                    # ... and retry
                    chunksize = 1
                    continue

                self.s.write(b'\n')
                assert self.s.read(2) == b'\r\n'

                answer, good = self.read_until_prompt()
                if not good:
                    error('Command \'%s\' timed out:\n%s' % (cmd, answer.decode('UTF-8')))
                    return b''
                return answer
            return b''
        except KeyboardInterrupt as e:
            time.sleep(0.10)
            self.flush()
            raise e

    def run_command_noreturn(self, cmd):
        if self.debug:
            error(':> %s' % cmd)
        self.enter_with_echo(cmd, self.chunksize)
        self.s.write(b'\n')
        #assert self.s.read(2) == b'\r\n'
        self.s.read(2)

    def writeX(self, cmd, size, addr, value):
        #print('poke %s %08x %s' % (cmd, addr, value))
        if isinstance(value, bytes):
            value = [x for x in value]
        if hasattr(value, '__iter__'):
            value = list(value)
        if isinstance(value, list):
            v = value
            while len(v) > 0:
                line = f'{cmd} {addr:x}'; i = 0
                while i < 14 and i < len(v) and len(line + f' {v[i]}') <= 128:
                    line += f' {v[i]}'
                    i += 1
                self.run_command(line)
                v = v[i:]
                addr += i * size
        else:
            self.run_command("%s %08x %#x" % (cmd, addr, value))

    def write8(self, addr, value):  return self.writeX('wb', 1, addr, value)
    def write16(self, addr, value): return self.writeX('wh', 2, addr, value)
    def write32(self, addr, value): return self.writeX('ww', 4, addr, value)

    def write_file(self, addr, filename):
        with open(filename, 'rb') as f:
            data = f.read()
            f.close()
            self.write8(addr, data)

    def flash(self, memaddr, flashaddr, size):
        self.run_command("fl %08x %08x %#x" % (memaddr, flashaddr, size))

    def memset(self, addr, value, size):
        value16 = value << 8 | value
        value32 = value16 << 16 | value16
        while size > 0:
            if addr & 3 != 0 or size < 4:
                n = size & 3 or 1
                self.write8(addr, [value] * n)
                addr += n
                size -= n
            else:
                n = size // 4
                self.write32(addr, [value32] * n)
                addr += n * 4
                size -= n * 4

    def parse_r_output(self, s):
        array = []
        s = s.decode('UTF-8')
        for line in s.splitlines():
            if re.match('[0-9a-f]{8}: [0-9a-f]+', line):
                for n in line[10:].split(' '):
                    array.append(int(n, base=16))
        return array


    def readX(self, cmd, size, addr, num):
        output = self.run_command("%s %08x %d" % (cmd, addr, num))
        a = self.parse_r_output(output)
        if num == 1:  return a[0]
        elif size==1: return bytes(a)
        else:         return a

    def read8(self, addr, num=1):  return self.readX('rb', 1, addr, num)
    def read16(self, addr, num=1): return self.readX('rh', 2, addr, num)
    def read32(self, addr, num=1): return self.readX('rw', 4, addr, num)

    def copyX(self, cmd, dest, src, num):
        self.run_command("%s %08x %08x %d" % (cmd, src, dest, num))

    def copy8(self, dest, src, num):  self.copyX('cb', dest, src, num)
    def copy16(self, dest, src, num): self.copyX('ch', dest, src, num)
    def copy32(self, dest, src, num): self.copyX('cw', dest, src, num)

    def make_setclr(rd, wr):
        def fn(self, addr, bit, value):
            x = rd(self, addr)
            if value: wr(self, addr, x |  (1 << bit))
            else:     wr(self, addr, x & ~(1 << bit))
        return fn

    setclr8 = make_setclr(read8, write8)
    setclr16 = make_setclr(read16, write16)
    setclr32 = make_setclr(read32, write32)

    def make_dump(cmd):
        def fn(self, addr, length):
            res = self.run_command('%s %08x %d' % (cmd, addr, length))
            print(res.decode('ascii').strip())
        return fn

    dump8 = make_dump('rb')
    dump16 = make_dump('rh')
    dump32 = make_dump('rw')

    def call(self, addr, a=0, b=0, c=0, d=0):
        self.run_command_noreturn('call %x %d %d %d %d' % (addr, a, b, c, d))

    def call_linux_and_run_microcom(self, addr):
        self.call(addr, 0, 0xffffffff, 0)
        os.system(f'busybox microcom -s 115200 /dev/ttyUSB0')

class Block:
    def __init__(self, lolmon, base=None):
        self.l = lolmon
        if base:
            self.base = base

    def read8(self, offset): return self.l.read8(self.base + offset)
    def read16(self, offset): return self.l.read16(self.base + offset)
    def read32(self, offset): return self.l.read32(self.base + offset)

    def write8(self, offset, value): return self.l.write8(self.base + offset, value)
    def write16(self, offset, value): return self.l.write16(self.base + offset, value)
    def write32(self, offset, value): return self.l.write32(self.base + offset, value)

    def setclr8(self, offset, bit, value): return self.l.setclr8(self.base + offset, bit, value)
    def setclr16(self, offset, bit, value): return self.l.setclr16(self.base + offset, bit, value)
    def setclr32(self, offset, bit, value): return self.l.setclr32(self.base + offset, bit, value)

    def dump(self):
        self.l.dump32(self.base, 0x20)

class Asm:
    zero = 0
    AT = 1
    v0 = 2
    v1 = 3
    a0 = 4
    a1 = 5
    a2 = 6
    a3 = 7
    a4 = 8
    a5 = 9
    a6 = 10
    a7 = 11
    t0 = 12
    t1 = 13
    t2 = 14
    t3 = 15
    s0 = 16
    s1 = 17
    s2 = 18
    s3 = 19
    s4 = 20
    s5 = 21
    s6 = 22
    s7 = 23
    t8 = 24
    t9 = 25
    jp = 25
    k0 = 26
    k1 = 27
    gp = 28
    sp = 29
    fp = 30
    s8 = 30
    ra = 31

    @staticmethod
    def ADDI(rs, rt, imm):
        assert rs == rs & 31
        assert rt == rt & 31
        assert imm == imm & 0xffff
        return 0b001000 << 26 | rs << 21 | rt << 16 | imm

    def NOP():
        return 0x00000000

    @staticmethod
    def J(target):
        return 0b000010 << 26 | ((target >> 2) & MASK(26))

class Exceptions(Block):
    def install_reset(self):
        for o in range(0x000, 0x400, 8):
            self.write32(o + 0, Asm.J(0x80008000))
            self.write32(o + 4, Asm.NOP())
        self.l.run_command('sync')


class CLK(Block):
    REG20 = 0x20
    REG20_SLOW_MUX = BIT(30)

    SPI0_MUX = 0x4c
    SPI0_MUX_SLOW = 0
    SPI0_MUX_594M = 2
    SPI0_MUX_405M = 3
    SPI0_MUX_SLOW_DIV2 = 4
    SPI0_MUX_306M = 5
    SPI0_MUX_297M = 6
    SPI0_MUX_202M5 = 7

    def set_spi0_mux(self, value):
        self.write32(self.SPI0_MUX, self.read32(self.SPI0_MUX) & ~7 | value)

    def rate_slow(self):
        if self.read32(self.REG20) & self.REG20_SLOW_MUX:
            return 24000000
        else:
            return 27000000


class SPI(Block):
    TRXFIFO = 0x0
    TRXLEN = 0x120
    CONTROL = 0x124
    CONTROL_TX = 3
    CONTROL_RX = 5
    CONTROL_CMDLEN_MASK = 0x003f0000
    CONTROL_CMDLEN_SHIFT = 16
    STATUS = 0x140
    STATUS_BUSY = BIT(16)
    STATUS_TXLVL_MASK = 0x00003f00
    STATUS_TXLVL_HIGH = 0x00001e00  # not sure
    STATUS_TXLVL_SHIFT = 8
    STATUS_RXLVL_MASK = 0x0000003f
    STATUS_RXLVL_SHIFT = 0
    CMDFIFO = 0x148


    def dump(self):
        self.l.dump32(self.base + 0x100, 0x20)

    def diag(self):
        control = self.read32(self.CONTROL)
        status = self.read32(self.STATUS)
        busy = 'BUSY' if status & self.STATUS_BUSY else 'idle'
        cmd = (control & self.CONTROL_CMDLEN_MASK) >> self.CONTROL_CMDLEN_SHIFT
        trx = self.read32(self.TRXLEN)
        rx = (status & self.STATUS_RXLVL_MASK) >> self.STATUS_RXLVL_SHIFT
        tx = (status & self.STATUS_TXLVL_MASK) >> self.STATUS_TXLVL_SHIFT
        print(f'SPI @ {self.base:08x}, {busy}, FIFO promises: CMD {cmd}, TRX {trx}; levels: RX {rx}, TX {tx}')

    def init(self):
        # useful for testing with a logic analyzer
        clk.set_spi0_mux(clk.SPI0_MUX_SLOW)

        #self.write32(0x104, 0x67050703)
        #self.write32(0x128, 0x100807)
        #self.l.write32(0xbf510008, self.l.read32(0xbf510008) |  (7 << 20))
        #self.l.write32(0xbf510018, self.l.read32(0xbf510018) & ~(7 << 20))
        #self.l.write32(0xbf510018, self.l.read32(0xbf510018) |  (7 << 20))
        #self.l.write32(0xbf510008, self.l.read32(0xbf510008) & ~(7 << 20))
        self.write32(self.CONTROL, 0x200)

    def can_tx(self):
        return (self.read32(self.STATUS) & self.STATUS_TXLVL_MASK) < self.STATUS_TXLVL_HIGH

    def can_rx(self):
        return (self.read32(self.STATUS) & self.STATUS_RXLVL_MASK) != 0

    def do_transfer(self, cmd, tx, rxlen):
        assert len(tx) == 0 or rxlen == 0
        assert len(cmd) < 0x20

        control = len(cmd) << 16
        if len(tx):
            control |= self.CONTROL_TX
            self.write32(self.TRXLEN, len(tx))
        else:
            control |= self.CONTROL_RX
            self.write32(self.TRXLEN, rxlen)
        self.write32(self.CONTROL, self.read32(self.CONTROL) & 0x1e00)
        self.write32(self.CONTROL, self.read32(self.CONTROL) | control)

        for x in cmd:
            self.write32(self.CMDFIFO, x)

        if len(tx):
            for i in range(0, len(tx), 4):
                while not self.can_tx(): pass
                self.write32(self.TRXFIFO, from_be32(tx[i:i+4]))
        if rxlen:
            rx = []
            for i in range(0, rxlen, 4):
                while not self.can_rx(): pass
                rx += to_le32(self.read32(self.TRXFIFO))
            return rx[0:rxlen]

    def flash_read(self, addr, length):
        return self.do_transfer([0x03] + to_be24(addr), [], length)


class GPIO(Block):
    OUT = [0x00, 0x10]
    DIR = [0x04, 0x14]
    IN  = [0x08, 0x18]

    def set(self, offset, value):
        self.setclr32(self.OUT[offset // 32], offset % 32, value)

    def dir(self, offset, value):
        self.setclr32(self.DIR[offset // 32], offset % 32, value)

    def get(self, offset):
        return bool(self.read32(self.IN[offset // 32]) & BIT(offset % 32))

class GPIOWrap:
    def __init__(self, g):
        self.g = g

    def set(self, offset, value):
        self.g[offset // 64].set(offset % 64, value)

    def dir(self, offset, value):
        self.g[offset // 64].dir(offset % 64, value)

    def get(self, offset):
        return self.g[offset // 64].get(offset % 64)

class I2C(Block):
    STATUS = 0x04
    WRITE = 0x08
    READ = 0x0c
    CONTROL = 0x14
    CONTROL_DONE  = 0x04
    CONTROL_SPECIAL = 0x04
    CONTROL_READ  = 0x10
    CONTROL_WRITE = 0x20
    CONTROL_STOP  = 0x40
    CONTROL_START = 0x80

    def finish(self):
        while self.read8(self.CONTROL) != self.CONTROL_DONE:
            print(hex(self.read8(self.CONTROL)))

        status = self.read8(self.STATUS)
        if status & 0xa0:
            print(f'I2C status: {status:02x}')

    def write(self, value, start):
        control = self.CONTROL_WRITE
        if start:
            control |= self.CONTROL_START

        self.write8(self.WRITE, value)
        self.write8(self.CONTROL, control)
        self.finish()

    def stop(self):
        self.write8(self.CONTROL, self.CONTROL_STOP)
        self.finish()

    def read(self, special):
        control = self.CONTROL_READ
        if special:
            control |= self.CONTROL_SPECIAL

        self.write8(self.CONTROL, control)
        self.finish()
        return self.read8(self.READ)

# FD650B-S frontpanel
class Frontpanel:
    def __init__(self, i2c):
        self.i2c = i2c

    def run_command(self, cmd):
        self.i2c.write((cmd >> 7) & 0x3e | 0x40, 1)
        self.i2c.write(cmd & 0xff, 0)
        self.i2c.stop()

    def enable(self):
        self.run_command(0x441)

    # Digits:
    #    0
    #    ——
    # 5 |  | 1
    #   6——
    # 4 |  | 2
    #    ——
    #    3
    def set_digit(self, num, value):
        assert 0 <= num <= 3
        assert value & 0xff == value
        self.run_command((0x14 + num) << 8 | value)

    def set_digits(self, digits):
        for i, d in enumerate(digits):
            self.set_digit(i, d)

    # Keycodes:
    # - Left:  0x1f
    # - Right: 0x07
    # - Power: 0x17
    # - Add 0x40 while key is pressed
    def get_keys(self):
        self.i2c.write(0x40 | 7 << 1 | 1, 1)
        keys = self.i2c.read(1)
        self.i2c.stop()
        return keys

    def hack(self):
        self.enable()
        self.set_digits([0b1110110, 0b1110111, 0b111001, 0b111001])


class UART(Block):
    BAUD_DIV = 0x18
    BAUD_FRAC = 0x1c

    @staticmethod
    def calc_baud_divisors(baud):
        c = clk.rate_slow()
        sample_rate = baud * 16
        div = c // sample_rate
        rem = c % sample_rate
        frac = rem // baud
        return div, frac, 1 - int((rem * 2) < baud)

    def set_baud_rate(self, baud):
        div, frac, _ = self.calc_baud_divisors(baud)
        assert div < 256
        assert frac < 16
        self.l.run_command_noreturn(f'ww {self.base + self.BAUD_DIV:08x} {div} {frac}')
        if self == uart0:
            self.l.s.baudrate = baud
            self.l.connection_test()


l = Lolmon('/dev/ttyUSB0')
l.connection_test()
exc = Exceptions(l, 0x80000000)
spi0 = SPI(l, 0xbf010000)
gpio0 = GPIO(l, 0xbf0a0000)
spi1 = SPI(l, 0xbf159000)
clk = CLK(l, 0xbf500000)
uart0 = UART(l, 0xbf540000)
uart1 = UART(l, 0xbf550000)
gpio1 = GPIO(l, 0xbf155000)
i2c0 = I2C(l, 0xbf560000)
i2c1 = I2C(l, 0xbf570000)
i2c2 = I2C(l, 0xbf158000)
i2c3 = I2C(l, 0xbf15c000)
i2c4 = I2C(l, 0xbf580000)
gpio = GPIOWrap([gpio0, gpio1])
fp = Frontpanel(i2c2)


spi0.init()

def scan_mem():
    for i in range(0x80000000, 0x80000000 + 64 * MiB, 0x8000):
        print(f'{i:x}:  ' + ' '.join([f'{l.read32(i+o*0x1000):08x}' for o in range(8)]))
