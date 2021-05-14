ARCH   := x86_64
LIBEFI := libefi
include libefi/tools/Makefile-$(ARCH).efi

CFLAGS += -Isrc -Wall \
	-Wno-missing-braces -Wno-unused-function \
	-Wno-unused-variable -Wno-unused-but-set-variable

# NOTE: link to 1MiB, we must be loaded under 4GiB otherwise the 32-bit SMP
# init code breaks
LDFLAGS += -Wl,--image-base,0x100000

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
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
