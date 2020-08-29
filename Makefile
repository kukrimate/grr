include ../tools/Makefile.efi

APP := hdd/efi/boot/bootx64.efi
OBJ := src/efi/main.o \
	src/efi/loader.o \
	src/vmm/acpi.o \
	src/vmm/uart.o \
	src/vmm/helper.o \
	src/vmm/vmm.o \
	src/vmm/smp.o \
	src/vmm/kernel.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
