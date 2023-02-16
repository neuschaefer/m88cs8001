# SPDX-License-Identifier: MIT
import ctypes

from memory import *
from cpu import *
from peripheral import *
from uart import *
from spi import *

class SoC:
    def __init__(self):
        self.cpu = CPU()
        self.peripherals = []

        self.cpu.map_ram(0, 64 * MiB)
        self.cpu.map_ram(0x1e000000, 4 * MiB)
        self.cpu.map_ram(0x1e800000, 1 * MiB)

        self.add_peripherals(
                spi0  = SPI(0x1f010000),
                bf100 = Peripheral(0x1f100000),
                bf104 = Peripheral(0x1f104000),
                bf121 = Peripheral(0x1f121000),
                bf130 = Peripheral(0x1f130000),
                bf13c = Peripheral(0x1f13c000),
                bf140 = Peripheral(0x1f140000),
                bf15  = Peripheral(0x1f150000, 0x10000),
                bf2   = Peripheral(0x1f200000, 0x100000),
                bf3   = Peripheral(0x1f300000, 0x100000),
                bf443 = Peripheral(0x1f443000),
                bf500 = Peripheral(0x1f500000),
                bf5d0 = Peripheral(0x1f5d0000),
                uart0 = UART(0x1f540000),
                uart1 = UART(0x1f550000),
        )

    def add_peripherals(self, **kwargs):
        for name, p in kwargs.items():
            self.__setattr__(name, p)
            self.peripherals.append(p)
            self.cpu.map_peripheral(p)

    def simulate_bootrom(self):
        # TODO: try to boot from UART0

        # Try to boot from SPI0 flash
        magic = self.spi0.flash.read32(0)
        if magic == 0x5a7d9cbf:
            size = self.spi0.flash.read32(0x14) + 0x300
            data = self.spi0.flash.read(0, size)
            for i, x in enumerate(data):
                self.cpu.write8(0x9e800000 + i, x)
            self.cpu.set_pc(0x9e800400)

    def simulate(self):
        self.cpu.simulate()
        for p in self.peripherals:
            p.simulate()
