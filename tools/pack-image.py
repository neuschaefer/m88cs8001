#!/usr/bin/python3
# SPDX-License-Identifier: MIT
from construct import *
import zlib, lzma, argparse, os, json, hashlib, struct

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
assert Entry.sizeof() == 0x30

def default_entry():
    return dict(
        unk_0 = 0,
        flags = 0,
        unk_8 = 0,
        id = 0,
        unk_d = 0,
        unk_e = 0,
        offset = 0,
        size = 0,
        crc = 0,
        serial = 0,
        name = 0,
        unk_2c = 0,
    )

Table = Struct(
    Const(b'*^_^*DM(^o^)'),
    Padding(2),
    'size' / Int8ul,
    Padding(1),
    'entries' / Array(21, Entry)
)
assert Table.sizeof() == 0x400

def compress_lzma(data):
    # Compress data in LZMA "alone" format, and manually patch the uncompressed-length field
    l = lzma.LZMACompressor(format=lzma.FORMAT_ALONE)
    out = l.compress(data)
    out += l.flush()
    out = bytearray(out)
    assert out[5:5+8] == 8*b'\xff'
    out[5:5+8] = struct.pack('<Q', len(data))
    return bytes(out)

class Allocator:
    def __init__(self, size):
        self.size = size
        self.map = { 0: (0, size) }

    def __repr__(self):
        return '[ ' + ', '.join([f'({start:#x}, {size:#x})' for start, size in self.map.values()]) + ' ]'

    def insert(self, start, size):
        if size > 0:
            self.map[start] = (start, size)

    def iter(self):
        for s in sorted(self.map):
            yield self.map[s]

    def alloc_fixed(self, start, size):
        for mstart, msize in self.iter():
            #print(f'{start:#x}:{size:#x} vs {self}')
            if start >= mstart and start - mstart <= msize:
                # Found the region that includes the requested range. Now cut a hole in it.
                #  mstart       start      start+size             mstart+msize
                #  [             ][xxxxxxxxxxxx][                            ]
                del self.map[mstart]
                self.insert(mstart, start - mstart)
                self.insert(start+size, mstart+msize - (start+size))
                return start
        raise Exception(f'Allocation failure for start {start:#x}, size {size:#x} in {self}')

    def alloc_free(self, size):
        for mstart, msize in self.iter():
            #print(f'{size:#x} vs {mstart:#x}:{msize:#x}')
            if msize >= size:
                # Found a block that is big enough.
                del self.map[mstart]
                self.insert(mstart + size, msize - size)
                return mstart
        raise Exception(f'Allocation failure for size {size:#x} in {self}')

    def alloc(self, start, size):
        if start:
            return self.alloc_fixed(start, size)
        else:
            return self.alloc_free(size)

class Image:
    def __init__(self, directory):
        with open(f'{directory}/manifest.json', 'r') as f:
            self.manifest = json.load(f)

    def write(self, args):
        if args.output_file:
            output_file = args.output_file
        else:
            output_file = self.manifest['metadata']['filename']

        if args.force:
            f = open(output_file, 'wb')
        else:
            f = open(output_file, 'xb')

        d = bytearray(int(self.manifest['metadata']['filesize']))
        alloc = Allocator(len(d))
        entries = []

        d[0:len(d)] = len(d) * bytes([int(args.fill, 0)])

        for p in self.manifest['partitions']:
            if 'fixed_addr' in p:
                o = int(p['fixed_addr'], 0)
            else:
                o = None

            if 'special' in p:
                assert p['special'] == 'partition_table'
                assert 'fixed_addr' in p
                partition_table_offset = alloc.alloc(o, 0x400) # skip and fill in later
            else:
                pd = open(f'{args.directory}/{p["filename"]}', 'rb').read()
                if 'compression' in p:
                    assert p['compression'] == 'lzma'
                    pd = compress_lzma(pd)
                o = alloc.alloc(o, len(pd))
                d[o:o+len(pd)] = pd
                if 'partition_entry' in p:
                    ent = dict(p['partition_entry'])
                    ent['offset'] = o - TABLE_BASE
                    ent['size'] = len(pd)
                    ent['crc'] = NCRC
                    ent['id'] = int(ent['id'], 0)
                    ent['serial'] = ent['serial'].encode('ascii')
                    ent['name'] = ent['name'].encode('ascii').ljust(8, b'\0')
                    entries.append(ent)

        # Build partition table
        num_entries = len(entries)
        while len(entries) < 21:
            entries.append(default_entry())
        table = Table.build(dict(size = num_entries * 3, entries = entries))
        d[partition_table_offset:partition_table_offset+len(table)] = table

        # Write out
        f.write(d)
        f.close()

        # Verify result
        if args.verify:
            expected = self.manifest['metadata']['sha256sum']
            actual   = hashlib.sha256(d).digest().hex()
            if actual == expected:
                print(f'SHA256 {expected}: correct!')
            else:
                print(f'SHA256 {expected} -> MISMATCH!')
                print(f'       {actual} in generated file')



parser = argparse.ArgumentParser(description = 'Repack a firmware image')
parser.add_argument('directory', help='directory with partitions and manifest.json')
parser.add_argument('-o', '--output-file', help='filename (defaults to name specified in manifest.json')
parser.add_argument('-f', '--force', help='overwrite output file when it already exists', action='store_true')
parser.add_argument('--verify', help='verify the hash of the resulting file', action='store_true')
parser.add_argument('--fill', help='fill empty space with given byte (default 0xff)', action='store_true', default='0xff')
args = parser.parse_args()

img = Image(args.directory)
img.write(args)
