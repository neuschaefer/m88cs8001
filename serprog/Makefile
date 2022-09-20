# SPDX-License-Identifier: MIT

CROSS_COMPILE := mips-linux-gnu-
AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
CPUFLAGS := -EL -march=24kec
CFLAGS := -Os -fno-builtin -nostdlib -Wall -Wextra -Wno-unused-function -Wno-main -fno-pic -mno-dsp
LDFLAGS := -T monitor.ld -EL
LDFLAGS_BOOT1 := -T boot1.ld -EL

all: serprog.bin

%.o: %.S
	$(CC) -c $(CPUFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CPUFLAGS) $(CFLAGS) $< -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.bin: %.o
	$(OBJCOPY) -j .text -O binary $< $@

%.lzma: %.bin
	../tools/lzma-compress.py $< $@

SERPROG_OBJS = boot1.o serprog.o
serprog.elf: $(SERPROG_OBJS) boot1.ld
	$(LD) $(LDFLAGS_BOOT1) $(SERPROG_OBJS) -o $@

.PHONY: clean

clean:
	rm -f serprog.bin *.o
