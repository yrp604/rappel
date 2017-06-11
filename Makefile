ARCH ?= $(shell uname -m)

ifeq ($(ARCH), i386)
	ARCH = x86
else ifeq ($(ARCH), i686)
	ARCH = x86
else ifeq ($(ARCH), x86_64)
	ARCH = amd64
else ifeq ($(ARCH), armv7l)
	ARCH = armv7
else ifeq ($(ARCH), aarch64)
	ARCH = armv8
endif

CFLAGS_ARCH  =-Ddisplay=display_$(ARCH) -Dgen_elf=gen_elf_$(ARCH) -Dptrace_reset=ptrace_reset_$(ARCH) -Ddump_state=dump_state_$(ARCH) \
		-Dptrace_reset=ptrace_reset_$(ARCH) -Dptrace_collect_regs=ptrace_collect_regs_$(ARCH)

CFLAGS_amd64 = -Dassemble=assemble_intel \
		-DREGFMT=REGFMT64 -DARCH_INIT_PROC_INFO=AMD64_INIT_PROC_INFO 
CFLAGS_x86   = -Dassemble=assemble_intel \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=X86_INIT_PROC_INFO \
		-m32
CFLAGS_armv7 = -Dassemble=assemble_armv7 \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=ARMV7_INIT_PROC_INFO
CFLAGS_armv8 = -Dassemble=assemble_armv8 \
		-DREGFMT=REGFMT64 -DARCH_INIT_PROC_INFO=ARMV8_INIT_PROC_INFO

CFLAGS = -std=c11 -Wall -pedantic -Wno-gnu-empty-initializer $(CFLAGS_ARCH) $(CFLAGS_$(ARCH)) -O2 -fPIE -D_FORTIFY_SOURCE=2
LDFLAGS = 
INC = -Iinclude/ -Iarch/$(ARCH)/include
LIBS = -ledit

print-%  : ; @echo $* = $($*)

SRC = rappel.c exedir.c common.c ptrace.c ui.c pipe.c
SRC_ARCH = $(shell find arch/$(ARCH) -name "*.c")

ALL_SRC = $(SRC) $(SRC_ARCH)

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
	mkdir -p obj/arch/$(ARCH)

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

clean:
	$(RM) obj/*.o *~ $(TARGET)
	$(RM) obj/arch/$(ARCH)/*.o

	-rmdir -p obj/arch/$(ARCH)

uninstall:
	$(RM) -rf ~/.rappel
