include ../tools/Makefile.efi

APP := hdd/efi/boot/bootx64.efi
OBJ := src/efi/main.o \
	src/efi/loader.o \
	src/vmm/uart.o \
	src/vmm/alloc.o \
	src/vmm/helper.o \
	src/vmm/vmm.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
