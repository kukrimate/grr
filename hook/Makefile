LIBEFI := libefi
include libefi/tools/Makefile.efi

# Boot services driver
SUBSYSTEM := 11

APP := hook.efi
OBJ := src/main.o \
	src/hook.o

all: $(APP)

$(APP): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(APP) $(OBJ)
