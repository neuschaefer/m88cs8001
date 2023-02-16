# SPDX-License-Identifier: MIT
__all__ = ['Peripheral']

class Peripheral:
    def __init__(self, base, size=0x1000):
        self.base = base
        self.size = size
        self.enable_trace = False

    def __repr__(self):
        return f'{self.__class__.__name__} @ {self.base:#08x}'

    def read(self, offset, size):
        if self.enable_trace:
            self.trace_read(offset, size)
        try:
            return self.do_read(offset, size)
        except Exception as e:
            print(f'[{self}] Exception during read, {offset=}: {e}')
            return 0

    def write(self, offset, size, value):
        if self.enable_trace:
            self.trace_write(offset, size, value)
        try:
            self.do_write(offset, size, value)
        except Exception as e:
            print(f'[{self}] Exception during write, {offset=}: {e}')

    def trace_read(self, offset, size):
        print(f'[{self}] READ  {offset:#x}:{size}')

    def trace_write(self, offset, size, value):
        print(f'[{self}] WRITE {offset:#x}:{size} <- {value:#x}')

    def do_read(self, offset, size):
        if size == 1:
            if hasattr(self, 'do_read8'):
                return self.do_read8(offset)
        if size == 2:
            if hasattr(self, 'do_read16'):
                return self.do_read16(offset)
            elif hasattr(self, 'do_read8'):
                return self.do_read8(offset) | self.do_read8(offset + 1) << 8
        if size == 4:
            if hasattr(self, 'do_read32'):
                return self.do_read32(offset)
            elif hasattr(self, 'do_read16'):
                return self.do_read16(offset) | self.do_read16(offset + 2) << 16
            elif hasattr(self, 'do_read8'):
                return self.do_read8(offset) | self.do_read8(offset + 1) << 8 | \
                        self.do_read8(offset + 2) << 16 | self.do_read8(offset + 3) << 24
        return 0

    def do_write(self, offset, size, value):
        # FIXME
        if size == 1:
            if hasattr(self, 'do_write8'):
                return self.do_write8(offset, value)
        if size == 2:
            if hasattr(self, 'do_write16'):
                return self.do_write16(offset, value)
            elif hasattr(self, 'do_write8'):
                return self.do_write8(offset) | self.do_write8(offset + 1) << 8
        if size == 4:
            if hasattr(self, 'do_write32'):
                return self.do_write32(offset)
            elif hasattr(self, 'do_write16'):
                return self.do_write16(offset) | self.do_write16(offset + 2) << 16
            elif hasattr(self, 'do_write8'):
                return self.do_write8(offset) | self.do_write8(offset + 1) << 8 | \
                        self.do_write8(offset + 2) << 16 | self.do_write8(offset + 3) << 24
        return 0

    def simulate(self):
        pass
