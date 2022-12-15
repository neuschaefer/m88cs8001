# SPDX-License-Identifier: MIT

CROSS_COMPILE := mips-linux-gnu-
AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
CPUFLAGS := -EL -march=24kec
CFLAGS := -Os -fno-builtin -nostdlib -Wall -Wextra -Wno-unused-function -Wno-main -fno-pic
LDFLAGS := -T talk.ld -EL

all: talk.bin

%.o: %.S
	$(CC) -c $(CPUFLAGS) $< -o $@

%.o: %.c slide_*.c
	$(CC) -c $(CPUFLAGS) $(CFLAGS) $< -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.bin: %.o
	$(OBJCOPY) -j .text -O binary $< $@

font_cozette.c: cozette.bdf bdfencode.py
	./bdfencode.py $< font_cozette > $@

TALK_OBJS = start.o talk.o font_cozette.o
talk.elf: $(TALK_OBJS) talk.ld
	$(LD) $(LDFLAGS) $(TALK_OBJS) -o $@

.PHONY: clean

clean:
	rm -f talk.bin *.o
