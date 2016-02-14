ARCH ?= $(shell uname -m)

CFLAGS_x86_64 = -Ddisplay=display_amd64 -Dgen_elf=gen_elf_amd64 -Dptrace_reset=ptrace_reset_amd64 \
		-Dassemble=assemble_intel \
		-DREGFMT=REGFMT64 -DARCH_INIT_PROC_INFO=AMD64_INIT_PROC_INFO
CFLAGS_i386   = -Ddisplay_x86=display -Dgen_elf_x86=gen_elf -Dptrace_reset=ptrace_reset_x86 \
		-Dassemble=assemble_intel \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=X86_INIT_PROC_INFO \
		-m32
CFLAGS_armv7l = -Ddisplay=display_arm -Dgen_elf=gen_elf_arm -Dptrace_reset=ptrace_reset_arm \
		-Dassemble=assemble_arm \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=ARM_INIT_PROC_INFO

CFLAGS = -Wall -pedantic -Wno-gnu-empty-initializer -Wno-format $(CFLAGS_$(ARCH))
LDFLAGS = 
INC = -Iinclude/ 
LIBS = -ledit

SRC = rappel.c exedir.c common.c ptrace.c ui.c pipe.c
SRC_x86_64 = elf_amd64.c display_amd64.c assemble_intel.c
SRC_i386   = elf_x86.c   display_x86.c   assemble_intel.c
SRC_armv7l = elf_arm.c   display_arm.c   assemble_arm.c

ALL_SRC = $(SRC) $(SRC_$(ARCH))

OBJ = $(patsubst %.c, obj/%.o, $(ALL_SRC))

TARGET = bin/rappel

.PHONY: clean

all: $(TARGET)
	@echo Done.

debug: CFLAGS += -g
debug: $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

obj:
	mkdir -p obj

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

test: $(TARGET)
	$(TARGET) -vvv < t/hello-$(ARCH).bin
	$(TARGET) -vvv < t/nop.asm

clean:
	$(RM) obj/*.o *~ $(TARGET)

uninstall:
	$(RM) -rf ~/.rappel
