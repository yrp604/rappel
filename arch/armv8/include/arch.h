#include <sys/uio.h>
#include <stdint.h>

#define TRAP 0xd4200020
#define TRAP_SZ 4

#define ARMV8_INIT_PROC_INFO(i) \
	do {\
		(i).regs   = (struct iovec) { .iov_base = &(i).regs_struct,   .iov_len = sizeof((i).regs_struct) }; \
		(i).fpregs = (struct iovec) { .iov_base = &(i).fpregs_struct, .iov_len = sizeof((i).fpregs_struct) }; \
	} while (0)

struct user_regs_armv8
{
        uint64_t           regs[31];
        uint64_t           sp;
        uint64_t           pc;
        uint64_t           pstate;
};

struct user_fpregs_armv8
{
        __int128_t     vregs[32];
        uint32_t           fpsr;
        uint32_t           fpcr;
        uint32_t           __reserved[2];
};

// TODO NT_ARM_TLS regset

struct proc_info_t {
	pid_t pid;

	struct user_regs_armv8 regs_struct;
	struct user_regs_armv8 old_regs_struct;
	struct iovec regs;

	struct user_fpregs_armv8 fpregs_struct;
	struct user_fpregs_armv8 old_fpregs_struct;
	struct iovec fpregs;

	int sig;
	long exit_code;
};

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
