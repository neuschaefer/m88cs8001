# SPDX-License-Identifier: MIT

AS := mips-linux-gnu-as
CPUFLAGS := -EL -march=24kec

all: start.bin.lzma

start.o: start.s
	$(AS) $(CPUFLAGS) $< -o $@

start.bin: start.o
	objcopy -j .text -O binary $< $@

start.bin.lzma: start.bin
	@rm -f $@
	lzma -F alone -k $<
