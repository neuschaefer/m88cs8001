# SPDX-License-Identifier: MIT

CROSS_COMPILE := mips-linux-gnu-
AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
CPUFLAGS := -EL -march=24kec
CFLAGS := -Os -fno-builtin -nostdlib -Wall -Wextra -Wno-unused-function -Wno-main -fno-pic
LDFLAGS := -T monitor.ld -EL

all: start.bin monitor.bin

%.o: %.S
	$(CC) -c $(CPUFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CPUFLAGS) $(CFLAGS) $< -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

start.bin: start.o
	objcopy -j .text -O binary $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

MONITOR_OBJS = start.o monitor.o
monitor.elf: $(MONITOR_OBJS) monitor.ld
	$(LD) $(LDFLAGS) $(MONITOR_OBJS) -o $@

.PHONY: clean

clean:
	rm -f start.bin monitor.bin *.o
