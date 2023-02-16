# SPDX-License-Identifier: MIT
__all__ = ['KiB', 'MiB', 'GiB',
           'get_u8', 'get_u16', 'get_u32',
           'make_u8', 'make_u16', 'make_u32']

import struct

KiB = 1024
MiB = 1024 * KiB
GiB = 1024 * MiB

def get_u8 (data, o=0): return data[o+0]
def get_u16(data, o=0): return data[o+0] | data[o+1] << 8
def get_u32(data, o=0): return data[o+0] | data[o+1] << 8 | data[o+2] << 16 | data[o+3] << 24

def make_u8 (value): return struct.pack('<B', value)
def make_u16(value): return struct.pack('<H', value)
def make_u32(value): return struct.pack('<I', value)

