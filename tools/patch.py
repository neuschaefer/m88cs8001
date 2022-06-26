#!/usr/bin/python3
# SPDX-License-Identifier: MIT
import zlib, argparse, os, lzma, struct
from construct import *

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

def bytes_to_str(b):
    return b.decode('ascii').strip('\0')

class Image:
    def __init__(self, filename):
        self.filename = os.path.basename(filename)
        with open(filename, 'rb') as f:
            self.data = bytearray(f.read())

    def write_to_file(self):
        with open(self.filename, 'wb') as f:
            f.write(self.data)

    def get(self):
        offset = TABLE_BASE + 0x14
        while offset + 0x30 <= len(self.data):
            entry = Entry.parse(self.data[offset:offset+0x30])
            if entry.name == b'\0\0\0\0\0\0\0\0':
                break
            yield entry, offset
            offset += 0x30

    def by_name(self, name):
        for entry, offset in self.get():
            if bytes_to_str(entry.name) == name:
                return entry, offset
        print(f'Partition "{name}" not found')

    def inject_data(self, offset, data):
        assert isinstance(data, bytes)
        assert offset + len(data) <= len(self.data)
        self.data[offset:offset+len(data)] = data

def compress_lzma(data):
    # Compress data in LZMA "alone" format, and manually patch the uncompressed-length field
    l = lzma.LZMACompressor(format=lzma.FORMAT_ALONE)
    out = l.compress(data)
    out += l.flush()
    out = bytearray(out)
    assert out[5:5+8] == 8*b'\xff'
    out[5:5+8] = struct.pack('<Q', len(data))
    return bytes(out)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = 'Partition table patcher')
    parser.add_argument('image', help='filename of the flash image')
    parser.add_argument('--patch', help='patch partition entry with given name')
    parser.add_argument('--raw', help='add data without compression')
    parser.add_argument('--lzma', help='add data, compress with LZMA')
    parser.add_argument('--addr', help='change base address of partition')
    args = parser.parse_args()

    img = Image(args.image)
    if args.raw and args.lzma:
        parser.error('Can\'t use --raw and --lzma together.')
    if not args.patch:
        parser.error('Action (--patch) required')
    if not args.raw and not args.lzma:
        parser.error('Data source (--raw or --lzma) required')

    entry, offset = img.by_name(args.patch)
    if args.addr:
        addr = int(args.addr, 16)
        if addr < TABLE_BASE or addr >= len(img.data):
            parser.error(f'Address 0x{addr:8x} out of range')
        entry.offset = addr - TABLE_BASE
    if args.raw:
        with open(args.raw, 'rb') as f:
            data = f.read()
        img.inject_data(TABLE_BASE + entry.offset, data)
        entry.size = len(data)
    if args.lzma:
        with open(args.lzma, 'rb') as f:
            data = compress_lzma(f.read())
        img.inject_data(TABLE_BASE + entry.offset, data)
        entry.size = len(data)
    entry.crc = NCRC
    img.inject_data(offset, Entry.build(entry))
    img.write_to_file()
