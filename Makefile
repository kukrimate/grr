include ../tools/Makefile.efi

CFLAGS += -Isrc

APP := hdd/efi/boot/bootx64.efi
OBJ := src/efi/main.o \
	src/efi/loader.o \
	src/vmm/helper.o \
	src/vmm/vmm.o \
	src/kernel/acpi.o  \
	src/kernel/smp.o \
	src/kernel/uart.o \
	src/kernel/init.o \
	src/kernel/kernel.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
