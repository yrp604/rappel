#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#include <sys/uio.h>

#include "user.h"

#define REQUIRE(x) \
	do {\
		if (!(x)) { \
			perror(#x); \
			exit(EXIT_FAILURE); \
		}\
	} while (0)

#if defined(__amd64__) || defined(i386)
#define TRAP 0xcc // int3
#define TRAP_SZ 1
#elif defined(__arm__)
//#define TRAP 0xfedeffe7
#define TRAP 0xe1200070
#define TRAP_SZ 4
#else
#error "No trap for architecture"
#endif

struct options_t {
	unsigned long start;
	int verbose;
	int allregs;
	const char *savefile;
};

#define REGFMT64 "0x%016" PRIx64
#define REGFMT32 "0x%08"  PRIx32
#define REGFMT16 "0x%04"  PRIx16
#define REGFMT8  "0x%02"  PRIx8

#define AMD64_INIT_PROC_INFO(i) \
	do {\
		i.regs   = (struct iovec) { .iov_base = &i.regs_struct,   .iov_len = sizeof(i.regs_struct) }; \
		i.fpregs = (struct iovec) { .iov_base = &i.fpregs_struct, .iov_len = sizeof(i.fpregs_struct) }; \
	} while (0)

#define X86_INIT_PROC_INFO(i) \
	do {\
		i.regs    = (struct iovec) { .iov_base = &i.regs_struct,    .iov_len = sizeof(i.regs_struct) }; \
		i.fpregs  = (struct iovec) { .iov_base = &i.fpregs_struct,  .iov_len = sizeof(i.fpregs_struct) }; \
		i.fpxregs = (struct iovec) { .iov_base = &i.fpxregs_struct, .iov_len = sizeof(i.fpxregs_struct) }; \
	} while (0)

#define ARM_INIT_PROC_INFO(i) \
	do {\
		i.regs   = (struct iovec) { .iov_base = &i.regs_struct,   .iov_len = sizeof(i.regs_struct) }; \
		i.fpregs = (struct iovec) { .iov_base = &i.fpregs_struct, .iov_len = sizeof(i.fpregs_struct) }; \
	} while (0)

struct proc_info_t {
	pid_t pid;

#if defined(__amd64__)
    struct user_regs_struct_amd64 regs_struct;
    struct user_regs_struct_amd64 old_regs_struct;
	struct iovec regs;

    struct user_fpregs_struct_amd64 fpregs_struct;
    struct user_fpregs_struct_amd64 old_fpregs_struct;
	struct iovec fpregs;
#elif defined(i386)
    struct user_regs_struct_x86 regs_struct;
    struct user_regs_struct_x86 old_regs_struct;
	struct iovec regs;

    struct user_fpregs_struct_x86 fpregs_struct;
    struct user_fpregs_struct_x86 old_fpregs_struct;
	struct iovec fpregs;

	struct user_fpxregs_struct_x86 fpxregs_struct;
	struct user_fpxregs_struct_x86 old_fpxregs_struct;
	struct iovec fpxregs;
#elif defined(__arm__)
    struct user_regs_arm regs_struct;
    struct user_regs_arm old_regs_struct;
	struct iovec regs;

    struct user_fpregs_arm fpregs_struct;
    struct user_fpregs_arm old_fpregs_struct;
	struct iovec fpregs;
#else
#error "No proc_info_t for architecture"
#endif

	int sig;
	long exit_code;
};

void mem_assign(
		uint8_t *,
		const size_t,
		const uint64_t,
		const size_t);

void* xmalloc(
		size_t);

void* xrealloc(
		void *,
		size_t);

const
size_t read_data(
		const int,
		uint8_t *const,
		const size_t);

void write_data(
		const int,
		const uint8_t *const,
		const size_t);

void verbose_printf(
		const char *const,
		...);

void verbose_dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);

void dump(
		const uint8_t *const,
		const size_t,
		const unsigned long long);
