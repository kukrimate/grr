include ../tools/Makefile.efi

APP := hdd/efi/boot/bootx64.efi
OBJ := src/grr.o src/loader.o src/vmm.o src/helper.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
