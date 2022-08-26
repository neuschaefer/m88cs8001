#!/usr/bin/python3
# SPDX-License-Identifier: MIT
import argparse, lzma, struct

l = lzma.LZMACompressor(format=lzma.FORMAT_ALONE)

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
    parser = argparse.ArgumentParser(description = 'LZMA compression compatible with M88CS8001 bootloader')
    parser.add_argument('input', help='name of the input file')
    parser.add_argument('output', help='name of the output file')
    args = parser.parse_args()

    with open(args.input, 'rb') as f:
        data = compress_lzma(f.read())
    with open(args.output, 'wb') as f:
        f.write(data)
        f.close()
