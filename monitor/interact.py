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
        self.s = serial.Serial(device, baudrate=115200, timeout=1.0)
        self.prompt = b'> '
        self.debug = False
        self.echo_attempts = 3

    def connection_test(self):
        self.s.write(b'\n')
        time.sleep(0.2)
        answer = self.s.read_all()
        if (b'\r\n' + self.prompt) in answer:
            print("lolmon detected!")

    def debug_log(self, prefix, s):
        if self.debug:
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

    def enter_with_echo(self, cmd):
        if isinstance(cmd, str):
            cmd = cmd.encode('UTF-8')
        assert not b'\n' in cmd

        pos = 0
        while pos < len(cmd):
            chunk = cmd[pos:pos+0x38]
            assert len(chunk) >= 1
            self.s.write(chunk)
            echo = self.s.read(len(chunk))
            if self.debug:
                error(f'input {chunk} -> {echo}')
            if echo != chunk:
                error(f'Echo error! {chunk} -> {echo}')
                error((echo + self.s.read_all()).decode('ascii'))
                return False
            pos += len(chunk)
        return True

    def run_command(self, cmd):
        try:
            for _ in range(self.echo_attempts):
                if self.debug:
                    error(':> %s' % cmd)
                good = self.enter_with_echo(cmd)
                if not good:
                    # Clear the prompt (send Ctrl-U)
                    self.s.write(b'\025')
                    time.sleep(0.01)
                    self.s.read_all()
                    # ... and retry
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
        self.enter_with_echo(cmd)
        self.s.write(b'\n')
        assert self.s.read(2) == b'\r\n'

    def writeX(self, cmd, size, addr, value):
        #print('poke %s %08x %s' % (cmd, addr, value))
        if isinstance(value, bytes):
            value = [x for x in value]
        if isinstance(value, list):
            for v in value:
                self.writeX(cmd, size, addr, v)
                addr += size
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
        emc0.stop() # No DMA please!
        emc1.stop()
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

UART = Block


l = Lolmon('/dev/ttyUSB0')
l.connection_test()
exc = Exceptions(l, 0x80000000)
uart0 = UART(l, 0xbf540000)
uart1 = UART(l, 0xbf550000)

def scan_mem():
    for i in range(0x80000000, 0x80000000 + 64 * MiB, 0x8000):
        print(f'{i:x}:  ' + ' '.join([f'{l.read32(i+o*0x1000):08x}' for o in range(8)]))
