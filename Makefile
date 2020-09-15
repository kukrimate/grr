LIBEFI := libefi
include libefi/tools/Makefile.efi

CFLAGS += -Isrc -Wall -mgeneral-regs-only \
	-Wno-missing-braces -Wno-unused-function \
	-Wno-unused-variable -Wno-unused-but-set-variable

SUBSYSTEM := 12

APP := hdd/driver.efi
OBJ := src/efi/main.o \
	src/efi/hook.o \
	src/vmm/helper.o \
	src/vmm/vmm.o \
	src/kernel/acpi.o  \
	src/kernel/alloc.o \
	src/kernel/smp.o \
	src/kernel/uart.o \
	src/kernel/init.o \
	src/kernel/kernel.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
