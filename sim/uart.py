# SPDX-License-Identifier: MIT
__all__ = ['UART']

import os
from termios import *
from select import select

from peripheral import *

# Indexes for termios list.
IFLAG = 0
OFLAG = 1
CFLAG = 2
LFLAG = 3
ISPEED = 4
OSPEED = 5
CC = 6

def make_sufficiently_raw(mode):
    mode[LFLAG] &= ~(ECHO | ICANON)
    mode[IFLAG] &= ~(ICRNL)
    mode[OFLAG] &= ~(ONLRET)
    mode[CC][VMIN] = 0
    mode[CC][VTIME] = 0

class UART(Peripheral):
    def attach_tty(self, name):
        tty = os.open(name, os.O_RDWR)
        if os.isatty(tty):
            self.saved_mode = tcgetattr(tty)
            mode = tcgetattr(tty)
            make_sufficiently_raw(mode)
            tcsetattr(tty, TCSAFLUSH, mode)
        self.tty = tty

    def detach_tty(self):
        if hasattr(self, 'tty'):
            tcsetattr(self.tty, TCSAFLUSH, self.saved_mode)

    def write_bytes(self, buf):
        if hasattr(self, 'tty'):
            os.write(self.tty, buf)

    def read_byte(self):
        if hasattr(self, 'tty'):
            return os.read(self.tty, 1)

    def can_read_byte(self):
        if hasattr(self, 'tty'):
            r, _, _ = select([self.tty], [], [], 0)
            if self.tty in r:
                return True
        return False

    def do_read16(self, offset):
        if offset == 0x14: # receive FIFO level
            return self.can_read_byte()
        if offset == 0x10:
            return 0
        elif offset == 0x200:
            b = self.read_byte() or b'\0'
            return b[0]
        return 0

    def do_write16(self, offset, value):
        if offset == 0x100:
            self.write_bytes(bytes([value]))
