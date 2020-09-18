# Use ELF toolchain
CC	:= x86_64-elf-gcc

# Build flags
CFLAGS	:= -I. -std=gnu89 -Wall -ffreestanding -fno-asynchronous-unwind-tables \
	-fpic -mgeneral-regs-only -mno-red-zone
LDFLAGS	:= -T core.ld -nostdlib -pie -mno-red-zone
LDLIBS	:= -lgcc

# Objects
OBJ := 	vmm/helper.o \
	vmm/vmm.o \
	kernel/acpi.o  \
	kernel/alloc.o \
	kernel/smp.o \
	kernel/uart.o \
	kernel/init.o \
	kernel/string.o \
	kernel/kernel.o

all: grr.elf

grr.elf: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $^

clean:
	rm -f grr.elf $(OBJ)
