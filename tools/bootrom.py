#!/usr/bin/python3
# SPDX-License-Identifier: MIT

import serial, time, sys, argparse, struct

def hexdump(data):
    if data:
        for offset in range(0, len(data), 16):
            d = data[offset:offset+16]
            line = f'{offset:08x}:  '
            line += ' '.join([f'{x:02x}' for x in d]).ljust(49)
            line += ''.join([chr(x) if (x >= 0x20 and x <= 0x7f) else '.' for x in d])
            print(line)

def load_boot1(filename, devname, listen, do_hexdump):
    s = serial.Serial(devname, baudrate=115200, timeout=0.1)

    program = bytearray(open(filename, 'rb').read())
    if len(program) <= 0x400:
        printf(f'Program too short ({len(program):#x} bytes), this is not going to work!')
        return

    program[0x00:0x04] = struct.pack('<I', 0x5a7d9cbf)
    program[0x14:0x18] = struct.pack('<I', len(program) - 0x300)
    if do_hexdump:
        hexdump(program)

    while True:
        s.write(b'\0');
        text = s.read_all()
        if text != b'':
            print(text)
        if text == b'R':
            print('BOOTROM FOUND')
            break
    for i, x in enumerate(program):
        print(f'\r                       \r0x{i:04x} {x:02}', end='')
        s.write(bytes([x]))
        text = s.read_all()
        if text != b'':
            print('\nTEXT:')
            print(text.decode('ascii'))
            break
    if listen:
        while True:
            text = s.read_all()
            if text != b'':
                sys.stdout.buffer.write(text)
                sys.stdout.buffer.flush()
            time.sleep(0.1)


if __name__ == '__main__':
    devname = '/dev/ttyUSB0'

    parser = argparse.ArgumentParser(description = 'Load bootloader stage 1 (boot1) via UART')
    parser.add_argument('filename', help='filename of the boot1 image')
    parser.add_argument('--hexdump', help='hexdump the program', action='store_true')
    parser.add_argument('--dev', help='device path (default: /dev/ttyUSB0)')
    parser.add_argument('--listen', help='listen for output ', action='store_true')
    args = parser.parse_args()

    if args.dev:
        devname = args.dev

    load_boot1(args.filename, devname, args.listen, args.hexdump)
