#!/usr/bin/python3
# SPDX-License-Identifier: MIT
from construct import *
import zlib, argparse, os

TABLE_BASE = 0x10000
NCRC = Int32ul.parse(b'NCRC')

Entry = Struct(
    'flags' / Hex(Int32ul),
    'unk_4' / Hex(Int32ul),
    'unk_8' / Hex(Int32ul),
    'offset' / Hex(Int32ul),  # offset from start of part. table
    'size' / Hex(Int32ul),
    'crc' / Hex(Int32ul),
    'serial' / Bytes(8),
    'name' / Bytes(8),
    'unk_28' / Hex(Int32ul),
    'unk_2c' / Hex(Int32ul),
)

class Image:
    def __init__(self, filename):
        self.filename = os.path.basename(filename)
        self.f = open(filename, 'rb')
        self.f.seek(TABLE_BASE)
        self.data = self.f.read(0x400)

    def get(self):
        offset = 0x14
        while offset + 0x30 <= len(self.data):
            entry = Entry.parse(self.data[offset:offset+0x30])
            if entry.name == b'\0\0\0\0\0\0\0\0':
                break
            yield entry
            offset += 0x30

    def list(self, long=False):
        if not long:
            print('nr. flags    offset   size     crc32    name')
        for i, e in enumerate(self.get()):
            if long:
                print(f'Partition {i}:  {e}')
            else:
                name = e.name.decode('ascii').strip('\0')
                print(f'{i:2}  {e.flags:08x} {e.offset:8x} {e.size:8x} {e.crc:08x} {name}')
            if e.crc != NCRC:
                self.f.seek(TABLE_BASE + e.offset)
                d = self.f.read(e.size)
                crc = zlib.crc32(d) ^ 0xffffffff
                if crc == e.crc:
                    print('CRC is correct!')
                else:
                    print('CRC mismatch!')

    def extract(self, long_names):
        for e in self.get():
            offset = TABLE_BASE + e.offset
            self.f.seek(offset)
            d = self.f.read(e.size)
            name = e.name.decode('ASCII').strip('\0')
            print(f'Extracting partition "{name}"')
            if long_names:
                filename = f'{self.filename}_{offset:08x}_{name}.bin'
            else:
                filename = f'{name}.bin'
            with open(filename, 'wb') as out:
                out.write(d)
                out.close()

parser = argparse.ArgumentParser(description = 'Partition table parser')
parser.add_argument('filename', help='filename of the flash image')
parser.add_argument('--list', help='list partitions', action='store_true')
parser.add_argument('--extract', help='extract partitions', action='store_true')
parser.add_argument('--long-filenames', help='use long filenames when extracting partitions', action='store_true')
args = parser.parse_args()

img = Image(args.filename)
if args.list:
    img.list()
if args.extract:
    img.extract(args.long_filenames)
