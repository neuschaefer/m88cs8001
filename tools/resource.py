#!/usr/bin/python3
# SPDX-License-Identifier: MIT
from construct import *
import argparse

PrimaryDesc = Struct(
    'index' / Int8ul,
    'pad' / Int8ul,
    'length' / Int16ul,
    'offset' / Hex(Int32ul),
)

SecondaryDesc = Struct(
    'index' / Int32ul,
    'offset' / Hex(Int32ul),
)

StringtabHeader = Struct(
    'unk0' / Hex(Int16ul),
    'unk2' / Hex(Int16ul),
    'unk4' / Hex(Int32ul),
    'unk8' / Hex(Int32ul),
    'length' / Hex(Int16ul),
    'unke' / Hex(Int16ul),
)

def read_utf16_string(data, offset):
    s = bytearray()
    o = offset
    while data[o:o+2] != b'\0\0':
        s += data[o:o+2]
        o += 2
    return s.decode('utf16')

class ResourceFile:
    def __init__(self, data):
        self.data = data

    def primary_desc(self, i):
        p = PrimaryDesc.parse(self.data[i*8: i*8 + 8])
        if p.index == i:
            return p

    def primary_descs(self):
        for i in range(256):
            p = self.primary_desc(i)
            if not p:
                break
            yield p

    def secondary_desc(self, i, j):
        p = self.primary_desc(i)
        offset = p.offset + j * 8
        s = SecondaryDesc.parse(self.data[offset:offset + 8])
        if s.index == j + 1:
            return s, p.offset + s.offset

    def secondary_descs(self, i):
        p = self.primary_desc(i)
        for j in range(p.length):
            s, offset = self.secondary_desc(i, j)
            if not s:
                break
            yield s, offset

    def show(self):
        for p in self.primary_descs():
            print(f'[{p.index:3}] offset = 0x{p.offset:05x}, length = {p.length}')
            for s, offset in self.secondary_descs(p.index):
                print(f'[{p.index:3}, {s.index:3}] offset = 0x{s.offset:05x} -> 0x{offset:05x}')

    def show_stringtables(self):
        for sec, offset in self.secondary_descs(3):
            s = StringtabHeader.parse(self.data[offset:])
            print(f'[{sec.index:2}] length = 0x{s.length:x}')
            for i in range(s.length):
                o = Hex(Int16ul).parse(self.data[offset+0x10+i*2:])
                addr = offset+0x10+o
                string = read_utf16_string(data, addr)
                print(f'[{sec.index:2},{i:#6x}] offset = {o} -> 0x{addr:x} {string!r}')

parser = argparse.ArgumentParser(description = 'resource file handling')
parser.add_argument('file', help='decompressed resource file')
parser.add_argument('--show', help='show general layout', action='store_true')
parser.add_argument('--show-strings', help='show string tables', action='store_true')
args = parser.parse_args()
data = open(args.file, 'rb').read()
r = ResourceFile(data)
if args.show:
    r.show()
if args.show_strings:
    r.show_stringtables()
