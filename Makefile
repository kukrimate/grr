CC	:= x86_64-elf-gcc
CFLAGS	:= -Ilibefi/api -Isrc -std=gnu89 -Wall \
	-ffreestanding -mgeneral-regs-only -mno-red-zone
LDFLAGS	:= -nostdlib -mno-red-zone
LDLIBS	:= -lgcc

SUBSYSTEM := 12

APP := driver.efi
OBJ := src/efi/main.o \
	src/efi/hook.o \
	src/vmm/helper.o \
	src/vmm/vmm.o \
	src/kernel/acpi.o  \
	src/kernel/alloc.o \
	src/kernel/smp.o \
	src/kernel/uart.o \
	src/kernel/init.o \
	src/kernel/string.o \
	src/kernel/kernel.o

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
