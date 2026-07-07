CC      = gcc
CFLAGS  = -std=c23 -Wall -Wextra -Wpedantic -g

AS      = riscv64-elf-as
LD      = riscv64-elf-ld
OBJCOPY = riscv64-elf-objcopy
OBJDUMP = riscv64-elf-objdump

EMU      = riscv-emu

# Emulator source files and headers
EMU_SRCS = main.c syscalls.c
EMU_HDRS = riscv.h

# Dynamically discover all assembly files and matching binaries
SRCS    = $(wildcard *.s)
BINS    = $(SRCS:.s=.bin)

# Helper to find the first compiled ELF for the dump macro
FIRST_ELF = $(firstword $(SRCS:.s=.elf))

all: $(EMU) $(BINS)

# Compiles the emulator whenever a .c file OR the .h file changes
$(EMU): $(EMU_SRCS) $(EMU_HDRS)
	$(CC) $(CFLAGS) -o $@ $(EMU_SRCS)

# Pattern Rules for automatic pipeline compilation
%.o: %.s
	$(AS) -march=rv32i -mabi=ilp32 -o $@ $<

%.elf: %.o
	$(LD) -m elf32lriscv -Ttext=0x0 -o $@ $<

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

# Your dump target, modernized for multiple files!
dump: $(BINS)
	@if [ -z "$(FIRST_ELF)" ]; then echo "No .elf file found to dump"; exit 1; fi
	$(OBJDUMP) -d -M no-aliases $(FIRST_ELF)

clean:
	rm -f $(EMU) *.o *.elf *.bin

.PHONY: all dump clean
