# SPDX-License-Identifier: MIT
__all__ = ['SPI']

from peripheral import *
from memory import *

class SPI(Peripheral):
    def __init__(self, base, size=0x1000):
        super().__init__(base, size)
        self.flash = Flash(4 * MiB)

class Flash:
    def __init__(self, size):
        self.data = bytearray(size)

    def load(self, filename):
        self.filename = filename
        data = open(self.filename, 'rb').read()
        self.data[:len(data)] = data

    def read(self, addr, size):
        return self.data[addr:addr+size]

    def read32(self, addr):
        return get_u32(self.data, addr)
