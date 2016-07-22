#include <sys/uio.h>

#define TRAP 0xcc // int3
#define TRAP_SZ 1

#define BITSTR "[bits 64]\n"

#define AMD64_INIT_PROC_INFO(i) \
	do {\
		(i).regs   = (struct iovec) { .iov_base = &(i).regs_struct,   .iov_len = sizeof((i).regs_struct) }; \
		(i).fpregs = (struct iovec) { .iov_base = &(i).fpregs_struct, .iov_len = sizeof((i).fpregs_struct) }; \
	} while (0)

struct user_fpregs_struct_amd64
{
	unsigned short int    cwd;
	unsigned short int    swd;
	unsigned short int    ftw;
	unsigned short int    fop;
	unsigned long long int rip;
	unsigned long long int rdp;
	unsigned int      mxcsr;
	unsigned int      mxcr_mask;
	unsigned int      st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
	unsigned int      xmm_space[64];  /* 16*16 bytes for each XMM-reg = 256 bytes */
	unsigned int      padding[24];
};

struct user_regs_struct_amd64
{
	unsigned long long int r15;
	unsigned long long int r14;
	unsigned long long int r13;
	unsigned long long int r12;
	unsigned long long int rbp;
	unsigned long long int rbx;
	unsigned long long int r11;
	unsigned long long int r10;
	unsigned long long int r9;
	unsigned long long int r8;
	unsigned long long int rax;
	unsigned long long int rcx;
	unsigned long long int rdx;
	unsigned long long int rsi;
	unsigned long long int rdi;
	unsigned long long int orig_rax;
	unsigned long long int rip;
	unsigned long long int cs;
	unsigned long long int eflags;
	unsigned long long int rsp;
	unsigned long long int ss;
	unsigned long long int fs_base;
	unsigned long long int gs_base;
	unsigned long long int ds;
	unsigned long long int es;
	unsigned long long int fs;
	unsigned long long int gs;
};

struct proc_info_t {
	pid_t pid;

    struct user_regs_struct_amd64 regs_struct;
    struct user_regs_struct_amd64 old_regs_struct;
	struct iovec regs;

    struct user_fpregs_struct_amd64 fpregs_struct;
    struct user_fpregs_struct_amd64 old_fpregs_struct;
	struct iovec fpregs;

	int sig;
	long exit_code;
};

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
