#!/usr/bin/python3
# SPDX-License-Identifier: MIT
from construct import *
import zlib, lzma, argparse, os, json, hashlib

TABLE_BASE = 0x10000
NCRC = Int32ul.parse(b'NCRC')

Entry = Struct(
    'unk_0' / Hex(Int32ul),
    'flags' / Hex(Int32ul),
    'unk_8' / Hex(Int32ul),
    'id' / Hex(Int8ul),
    'unk_d' / Hex(Int8ul),
    'unk_e' / Hex(Int16ul),
    'offset' / Hex(Int32ul),  # offset from start of part. table
    'size' / Hex(Int32ul),
    'crc' / Hex(Int32ul),
    'serial' / Bytes(8),
    'name' / Bytes(8),
    'unk_2c' / Hex(Int32ul),
)

class EntryInfo():
    def __init__(self, e):
        self.e = e

    def to_json(self):
        return {
                'unk_0': self.e.unk_0,
                'flags': self.e.flags,
                'unk_8': self.e.unk_8,
                'id': f'{self.e.id:#02x}',
                'unk_d': self.e.unk_d,
                'unk_e': self.e.unk_e,
                'serial': self.e.serial.decode('ascii'),
                'name': self.e.name.decode('ascii').strip('\0'),
                'unk_2c': self.e.unk_2c,
        }

    def has_fixed_address(self):
        return self.e.id in [0xa4, 0xa8, 0xaa, 0x7c]

    def compression(self):
        if self.e.id in [0x88, 0x89, 0x8c]:
            return 'lzma'

class Image:
    def __init__(self, filename):
        self.filename = os.path.basename(filename)
        self.f = open(filename, 'rb')
        self.f.seek(TABLE_BASE)
        self.tabledata = self.f.read(0x400)

    def get(self):
        offset = 0x10
        while offset + 0x30 <= len(self.tabledata):
            entry = Entry.parse(self.tabledata[offset:offset+0x30])
            if entry.name == b'\0\0\0\0\0\0\0\0':
                break
            yield entry
            offset += 0x30

    def list(self, long=False):
        if not long:
            print('nr. flags    offset   size     crc32    id name')
        for i, e in enumerate(self.get()):
            if long:
                print(f'Partition {i}:  {e}')
            else:
                name = e.name.decode('ascii').strip('\0')
                print(f'{i:2}  {e.flags:08x} {e.offset+TABLE_BASE:8x} {e.size:8x} {e.crc:08x} {e.id:02x} {name}')
            if e.crc != NCRC:
                self.f.seek(TABLE_BASE + e.offset)
                d = self.f.read(e.size)
                crc = zlib.crc32(d) ^ 0xffffffff
                if crc == e.crc:
                    print('CRC is correct!')
                else:
                    print('CRC mismatch!')

    def sha256sum(self):
        self.f.seek(0)
        return hashlib.sha256(self.f.read()).digest()

    def filesize(self):
        return self.f.seek(0, 2)

    def extract(self, long_names, output_dir):
        manifest = {}
        manifest['partitions'] = []
        manifest['metadata'] = {
                'filename': self.filename,
                'sha256sum': self.sha256sum().hex(),
                'filesize': self.filesize(),
        }

        # Create output directory
        os.mkdir(output_dir)

        # Extract static partitions
        print(f'Extracting bootloader')
        for filename, offset, size, special in [
                ('boot.bin', 0, TABLE_BASE, None),
                (None, TABLE_BASE, 0x400, 'partition_table')
        ]:
            if long_names and filename:
                filename = f'{self.filename}_{offset:08x}_{filename}'

            if filename:
                self.f.seek(offset)
                d = self.f.read(size)
                with open(f'{output_dir}/{filename}', 'wb') as out:
                    out.write(d)
                    out.close()
            info = {}
            info['fixed_addr'] = f'{offset:#x}'
            if filename: info['filename'] = filename
            if special:  info['special']  = special
            manifest['partitions'].append(info)

        # Extract partitions
        for e in self.get():
            info = {}
            offset = TABLE_BASE + e.offset
            self.f.seek(offset)
            d = self.f.read(e.size)
            name = e.name.decode('ASCII').strip('\0')
            print(f'Extracting partition "{name}"')
            if EntryInfo(e).compression():
                info['compression'] = EntryInfo(e).compression()
                assert info['compression'] == 'lzma'
                d = lzma.decompress(d)
            if long_names:
                filename = f'{self.filename}_{offset:08x}_{name}.bin'
            else:
                filename = f'{name}.bin'
            with open(f'{output_dir}/{filename}', 'wb') as out:
                out.write(d)
                out.close()
            info['filename'] = filename
            info['partition_entry'] = EntryInfo(e).to_json()
            if EntryInfo(e).has_fixed_address():
                info['fixed_addr'] = f'{offset:#x}'
            manifest['partitions'].append(info)

        # Write manifest
        with open(f'{output_dir}/manifest.json', 'w') as out:
            print(f'Writing manifest.json')
            json.dump(manifest, out, indent=4)
            out.close()


parser = argparse.ArgumentParser(description = 'Partition table parser')
parser.add_argument('filename', help='filename of the flash image')
parser.add_argument('--list', help='list partitions', action='store_true')
parser.add_argument('--extract', help='extract partitions', action='store_true')
parser.add_argument('--long-filenames', help='use long filenames when extracting partitions', action='store_true')
parser.add_argument('-o', '--output-dir', help='output direction when extracting partitions', default='.')
args = parser.parse_args()

img = Image(args.filename)
if args.list:
    img.list()
if args.extract:
    img.extract(args.long_filenames, args.output_dir)
