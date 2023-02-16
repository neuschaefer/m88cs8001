# SPDX-License-Identifier: MIT
import sys

from memory import *
from unicorn import *
from unicorn.mips_const import *
from capstone import *
from peripheral import *

__all__ = ['CPU']

keyboard_interrupt = False

def read_cb(uc, offset, size, p):
    try:
        return p.read(offset, size)
    except KeyboardInterrupt:
        global keyboard_interrupt
        keyboard_interrupt = True
        uc.emu_stop()
    except Exception as e:
        print(f'[{p}] Exception during read: {e}')
    except:
        print(f'[{p}] Unknown exception during read')
    return 0

def write_cb(uc, offset, size, value, p):
    try:
        return p.write(offset, size, value)
    except KeyboardInterrupt:
        uc.emu_stop()
    except Exception as e:
        print(f'[{p}] Exception during write: {e}')
    except:
        print(f'[{p}] Unknown exception during write')

def hook_cb(uc, addr, size, ctx):
    cpu, n = ctx
    cpu.hook(addr, size, n)

class CPU:
    def __init__(self):
        self.uc = Uc(UC_ARCH_MIPS, UC_MODE_32)
        self.enable_trace = False
        self.uc.hook_add(UC_HOOK_CODE, hook_cb, (self, UC_HOOK_CODE))

    def __repr__(self):
        return f'{self.__class__.__name__} @ {self.get_pc():#08x}'

    def map_ram(self, addr, size):
        self.uc.mem_map(addr, size, UC_PROT_ALL)

    def map_peripheral(self, p):
        self.uc.mmio_map(p.base, p.size, read_cb, p, write_cb, p)

    def read(self, addr, size):
        return self.uc.mem_read(addr, size)

    def write(self, addr, value):
        self.uc.mem_write(addr, value)

    def read8 (self, addr): return get_u8 (self.uc.mem_read(addr, 1))
    def read16(self, addr): return get_u16(self.uc.mem_read(addr, 2))
    def read32(self, addr): return get_u32(self.uc.mem_read(addr, 4))

    def write8 (self, addr, value): self.uc.mem_write(addr, make_u8 (value))
    def write16(self, addr, value): self.uc.mem_write(addr, make_u16(value))
    def write32(self, addr, value): self.uc.mem_write(addr, make_u32(value))

    def hook(self, addr, size, nr):
        if nr == UC_HOOK_CODE and self.enable_trace:
            self.disas_pc()

    def set_pc(self, addr):
        self.uc.reg_write(UC_MIPS_REG_PC, addr)

    def get_pc(self):
        return self.uc.reg_read(UC_MIPS_REG_PC)

    def set_r(self, r, value):
        assert r >= 0 and r < 32
        return self.uc.reg_write(UC_MIPS_REG_0 + r, value)

    def get_r(self, r):
        assert r >= 0 and r < 32
        return self.uc.reg_read(UC_MIPS_REG_0 + r)

    def disas_pc(self):
        cs = Cs(CS_ARCH_MIPS, CS_MODE_32)
        pc = self.get_pc()
        code = self.read(pc, 4)
        #cs.syntax = CS_OPT_SYNTAX_NOREGNAME  Currently not supported in capstone
        for i in cs.disasm(code, pc):
            insn = self.read32(i.address)
            print(f'[{self}] {insn:08x} {i.mnemonic} {i.op_str}')

    def dump_disas(self):
        pc = self.get_pc()
        start = pc - 16
        code = self.read(start, 32)
        cs = Cs(CS_ARCH_MIPS, CS_MODE_32)
        #cs.syntax = CS_OPT_SYNTAX_NOREGNAME  Currently not supported in capstone
        for i in cs.disasm(code, start):
            insn = self.read32(i.address)
            mark = '>' if i.address == pc else '|'
            print(f'[{self}] {mark} {i.address:08x}: {insn:08x} {i.mnemonic} {i.op_str}')

    def dump_regs(self):
        for rbase in range(0, 32, 4):
            def show(roff):
                r = rbase + roff
                return f'${r:02} = {self.get_r(r):08x}'
            print(f'[{self}] ' + ', '.join([show(o) for o in range(4)]))

    def dump_state(self):
        self.dump_disas()
        self.dump_regs()

    def simulate(self):
        try:

            self.uc.emu_start(self.get_pc(), -1)
            if keyboard_interrupt:
                sys.exit(1)
        except UcError as e:
            print(f'[{self}] Error: {e}')
            self.dump_state()
            sys.exit(1)
