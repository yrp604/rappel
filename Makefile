ARCH ?= $(shell uname -m)

ifeq ($(ARCH), i386)
	ARCH = x86
else ifeq ($(ARCH), i686)
	ARCH = x86
else ifeq ($(ARCH), x86_64)
	ARCH = amd64
else ifeq ($(ARCH), armv7l)
	ARCH = armv7
endif

BASE_ARCH = $(ARCH:%_llvm=%)

CFLAGS_ARCH  =-Ddisplay=display_$(BASE_ARCH) -Dgen_elf=gen_elf_$(BASE_ARCH) -Dptrace_reset=ptrace_reset_$(BASE_ARCH) -Ddump_state=dump_state_$(BASE_ARCH) \
		-Dptrace_reset=ptrace_reset_$(BASE_ARCH) -Dptrace_collect_regs=ptrace_collect_regs_$(BASE_ARCH)

CFLAGS_amd64 = -Dassemble=assemble_intel \
		-DREGFMT=REGFMT64 -DARCH_INIT_PROC_INFO=AMD64_INIT_PROC_INFO 
CFLAGS_amd64_llvm = $(CFLAGS_amd64)
CFLAGS_x86   = -Dassemble=assemble_intel \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=X86_INIT_PROC_INFO \
		-m32
CFLAGS_armv7 = -Dassemble=assemble_arm \
		-DREGFMT=REGFMT32 -DARCH_INIT_PROC_INFO=ARM_INIT_PROC_INFO

CFLAGS = -std=c11 -Wall -pedantic -Wno-gnu-empty-initializer $(CFLAGS_ARCH) $(CFLAGS_$(ARCH)) -O2 -fPIE -D_FORTIFY_SOURCE=2
CXXFLAGS = $(CFLAGS) -std=gnu++11 $(shell $(LLVM_CONFIG) --cxxflags) -O0
LLVM_CONFIG = llvm-config-3.8
LDFLAGS_amd64_llvm = $(shell $(LLVM_CONFIG) --ldflags --libs --system-libs engine)
INC = -Iinclude/ -Iarch/$(ARCH)/include
LIBS = -ledit

print-%  : ; @echo $* = $($*)

SRC = rappel.c exedir.c common.c ptrace.c ui.c pipe.c
SRC_ARCH = $(shell find arch/$(ARCH) -name "*.c")
SRC_ARCH_CXX = $(shell find arch/$(ARCH) -name "*.cpp")

ALL_SRC = $(SRC) $(SRC_ARCH)

OBJ = $(patsubst %.c, obj/%.o, $(ALL_SRC)) $(patsubst %.cpp, obj/%.o, $(SRC_ARCH_CXX))

TARGET = bin/rappel

.PHONY: clean

all: $(TARGET)
	@echo Done.

debug: CFLAGS += -g
debug: $(TARGET)

bin:
	mkdir -p bin

$(TARGET): $(OBJ) | bin
	$(CXX) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS_$(ARCH)) $(LIBS)

obj:
	mkdir -p obj
	mkdir -p obj/arch/$(ARCH)

obj/%.o: %.c | obj
	$(CC) $(CFLAGS) $(INC) -c $<  -o $@

obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) $(INC) -c $<  -o $@

clean:
	$(RM) obj/*.o *~ $(TARGET)
	$(RM) obj/arch/$(ARCH)/*.o

	-rmdir -p obj/arch/$(ARCH)

uninstall:
	$(RM) -rf ~/.rappel
