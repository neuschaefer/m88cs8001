#!/usr/bin/python3
# SPDX-License-Identifier: MIT
import lzma, argparse, struct

BASE = 0x83e10000

def write_with_header(filename, data):
    with open(filename, 'wb') as f:
        f.write(struct.pack('<III', len(data) + 4, BASE, BASE))
        f.write(struct.pack('>I', len(data)))
        f.write(data)
        f.close()

parser = argparse.ArgumentParser(description = 'pack av_cpu image file')
parser.add_argument('output', help='output filename')
parser.add_argument('--raw', help='raw input file')
parser.add_argument('--lzma', help='precompressed LZMA input file')
args = parser.parse_args()
if args.raw:
    data = lzma.compress(open(args.raw, 'rb').read())
    write_with_header(args.output, data)
elif args.lzma:
    data = open(args.lzma, 'rb').read()
    write_with_header(args.output, data)
