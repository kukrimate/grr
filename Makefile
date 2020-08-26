include ../tools/Makefile.efi

APP := efi/grr/grr.efi
OBJ := src/grr.o src/linux.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
