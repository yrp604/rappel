#include <sys/ptrace.h>
#include <linux/elf.h>

#include "common.h"
#include "arch.h"

void ptrace_collect_regs_x86(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	info->pid = child_pid;

	info->old_regs_struct = info->regs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &info->regs) == 0);

	info->old_fpregs_struct = info->fpregs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRFPREG, &info->fpregs) == 0);

	info->old_fpxregs_struct = info->fpxregs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRXFPREG, &info->fpxregs) == 0);

	info->sig       = -1;
	info->exit_code = -1;
}

void ptrace_reset_x86(
		const pid_t child_pid,
		const unsigned long start)
{
	struct user_regs_struct_x86 regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.eip = start;

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
}
