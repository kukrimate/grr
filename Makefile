include ../tools/Makefile.efi

APP := hdd/efi/boot/bootx64.efi
OBJ := src/grr.o src/linux.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
