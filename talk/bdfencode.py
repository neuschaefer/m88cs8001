#!/usr/bin/python3

import argparse

class Char:
    def __init__(self):
        self.encoding = 0
        self.bbx = [0,0,0,0]
        self.bitmap = []

    def __repr__(self):
        return f'Char({self.encoding:#02x}, {self.bbx}, {self.bitmap})'

class BDF:
    def __init__(self, filename):
        self.chars = []
        f = open(filename, 'r')
        char = None
        in_bitmap = False
        for line in f:
            line = line.strip()
            if line == 'ENDCHAR':
                self.chars.append(char)
                char = None
                in_bitmap = False
            elif in_bitmap:
                char.bitmap += [int(line, 16)]
            elif line.startswith('STARTCHAR '):
                char = Char()
            elif line.startswith('ENCODING '):
                char.encoding = int(line.split(' ')[1])
            elif line.startswith('BBX '):
                char.bbx = [int(x) for x in line.split(' ')[1:]]
            elif line == 'BITMAP':
                in_bitmap = True
        self.by_encoding = { c.encoding: c for c in self.chars }

    def get_char(self, c):
        return self.by_encoding[ord(c)]

    def encode_ascii(self, name):
        chars = [self.by_encoding[i] for i in range(0x20, 0x7f)]
        print(f'#include <stdint.h>')
        print(f'#include "font.h"')
        print(f'// length: {sum([len(c.bitmap) for c in chars])}')
        print(f'// #chars: {len(chars)}')
        print(f'// #bytes: {sum([len(c.bitmap) for c in chars]) + 4 * len(chars) + 16}')

        bitmap_indices = []
        bbx_biases = [min([c.bbx[i] for c in self.chars]) for i in range(4)]

        # List of bitmap bytes
        print(f'static uint8_t {name}_bitmaps[] = {{')
        index = 0
        for c in chars:
            print(f'    /* {c.encoding:#04x} \'{chr(c.encoding)}\' @{index:5} */ ' + ''.join(['%#04x,' % x for x in c.bitmap]))
            bitmap_indices.append(index)
            index += len(c.bitmap)
        print('};')

        # List for each character:
        print(f'static struct chardef {name}_chars[] = {{')
        for i, c in enumerate(chars):
            # u16: bounding box, four bits per part, biased by lowest value
            # u16: bitmap index
            bbx = [c.bbx[i] - bbx_biases[i] for i in range(4)]
            for b in bbx:
                assert 0 <= b <= 15
            print(f'    {{ {bitmap_indices[i]:4}, {len(c.bitmap):2}, {{ {bbx[0]}, {bbx[1]}, {bbx[2]}, {bbx[3]} }} }},')
        print('};')

        # Font control block:
        #   - pointers to the other things
        #   - Biases: BBX, char index
        #   - highest index
        print(f'struct font {name} = {{')
        print(f'    {name}_bitmaps,')
        print(f'    {name}_chars,')
        print(f'    {{ {bbx_biases[0]}, {bbx_biases[1]}, {bbx_biases[2]}, {bbx_biases[3]} }},')
        print(f'    {chars[0].encoding:#x}, {len(chars):#x}')
        print('};')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = 'Convert BDF font file to C structs')
    parser.add_argument('file', help='font file')
    parser.add_argument('name', help='name of the font object')
    args = parser.parse_args()

    bdf = BDF(args.file)
    bdf.encode_ascii(args.name)
