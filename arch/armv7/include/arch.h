#include <sys/uio.h>

#define TRAP 0xe1200070
#define TRAP_SZ 4

#define ARM_INIT_PROC_INFO(i) \
	do {\
		i.regs   = (struct iovec) { .iov_base = &(i).regs_struct,   .iov_len = sizeof((i).regs_struct) }; \
		i.fpregs = (struct iovec) { .iov_base = &(i).fpregs_struct, .iov_len = sizeof((i).fpregs_struct) }; \
	} while (0)

struct user_regs_arm
{
	unsigned long int uregs[18];
};

struct user_fpregs_arm
{
	struct fp_reg
	{
		unsigned int sign1:1;
		unsigned int unused:15;
		unsigned int sign2:1;
		unsigned int exponent:14;
		unsigned int j:1;
		unsigned int mantissa1:31;
		unsigned int mantissa0:32;
	} fpregs[8];
	unsigned int fpsr:32;
	unsigned int fpcr:32;
	unsigned char ftype[8];
	unsigned int init_flag;
};

struct proc_info_t {
	pid_t pid;

    struct user_regs_arm regs_struct;
    struct user_regs_arm old_regs_struct;
	struct iovec regs;

    struct user_fpregs_arm fpregs_struct;
    struct user_fpregs_arm old_fpregs_struct;
	struct iovec fpregs;

	int sig;
	long exit_code;
};

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
