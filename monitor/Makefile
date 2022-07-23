# SPDX-License-Identifier: MIT

CROSS_COMPILE := mips-linux-gnu-
AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
CPUFLAGS := -EL -march=24kec
CFLAGS := -Os -fno-builtin -nostdlib -Wall -Wextra -Wno-unused-function -Wno-main -fno-pic
LDFLAGS := -T monitor.ld -EL

all: monitor.bin scream.bin

%.o: %.S
	$(CC) -c $(CPUFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CPUFLAGS) $(CFLAGS) $< -o $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.bin: %.o
	$(OBJCOPY) -j .text -O binary $< $@

MONITOR_OBJS = start.o monitor.o
monitor.elf: $(MONITOR_OBJS) monitor.ld
	$(LD) $(LDFLAGS) $(MONITOR_OBJS) -o $@

.PHONY: clean

clean:
	rm -f monitor.bin *.o
