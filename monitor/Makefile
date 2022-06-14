# SPDX-License-Identifier: MIT

AS := mips-linux-gnu-as
CC := mips-linux-gnu-gcc
CPUFLAGS := -EL -march=24kec

all: start.bin.lzma

start.o: start.S
	$(CC) -c $(CPUFLAGS) $< -o $@

start.bin: start.o
	objcopy -j .text -O binary $< $@

start.bin.lzma: start.bin
	@rm -f $@
	lzma -F alone -k $<
