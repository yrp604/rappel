#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <elf.h> // NT_PRSTATUS

#include "common.h"
#include "ptrace.h"

extern struct options_t options;

// Round up to the nearest multiple
// https://gist.github.com/aslakhellesoy/1134482
#define ROUNDUP(n, m) n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;

static
void _collect_regs(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	info->pid = child_pid;

	info->old_regs_struct = info->regs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &info->regs) == 0);

	info->old_fpregs_struct = info->fpregs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_FPREGSET, &info->fpregs) == 0);

#if defined(i386)
	info->old_fpxregs_struct = info->fpxregs_struct;

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRXFPREG, &info->fpxregs) == 0);
#endif

	info->sig       = -1;
	info->exit_code = -1;
}

static
void _exited_collect_regs(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	_collect_regs(child_pid, info);

	siginfo_t si;
	REQUIRE (ptrace(PTRACE_GETSIGINFO, child_pid, NULL, &si) == 0);

	info->sig = si.si_signo;

	REQUIRE (ptrace(PTRACE_GETEVENTMSG, child_pid, NULL, &info->exit_code) == 0);
}

const
int ptrace_write(
		const pid_t child_pid,
		const void *const base,
		const uint8_t *const data,
		const size_t data_sz)
{
	int ret = 0;

	// round up to nearest ptr_sz + size of at least one trap
	const size_t alloc_sz = ROUNDUP(data_sz + TRAP_SZ, sizeof(long));

	unsigned long *const copy = xmalloc(alloc_sz);

	mem_assign((uint8_t *)copy, alloc_sz, TRAP, TRAP_SZ);
	memcpy(copy, data, data_sz);

	for (unsigned i = 0; i < alloc_sz / sizeof(long); i++) {
		const unsigned long addr = (unsigned long)base + i * sizeof(long);
		const unsigned long val  = copy[i];

		verbose_printf("ptrace_write: " REGFMT " = " REGFMT "\n", addr, val);

		if (ptrace(PTRACE_POKETEXT, child_pid, addr, val) == -1) {
			ret = -1;
			fprintf(stderr, "ptrace() - failed to write value " REGFMT " to " REGFMT "\n", val, addr);
		}
	}

	free(copy);

	return ret;
}

const
int ptrace_read(
		const pid_t child_pid,
		const void *const base,
		void *const out,
		const size_t out_sz)
{
	int ret = 0;

	const size_t alloc_sz = ROUNDUP(out_sz, sizeof(long));

	unsigned long *const copy = xmalloc(alloc_sz);

	for (unsigned i = 0; i < alloc_sz / sizeof(long); i++) {
		const unsigned long addr = (unsigned long)base + i * sizeof(long);

		verbose_printf("ptrace_read: " REGFMT "\n", addr);

		errno = 0;
		copy[i] = ptrace(PTRACE_PEEKDATA, child_pid, addr, 0);

		if (errno) {
			ret = -1;
			fprintf(stderr, "ptrace() - failed to read value at " REGFMT "\n", addr);
		}
	}

	memcpy(out, copy, out_sz);

	free(copy);

	return ret;
}

void ptrace_reset_amd64(
		const pid_t child_pid,
		const unsigned long start)
{
	struct user_regs_struct_amd64 regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.rip = start;

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
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

void ptrace_reset_arm(
		const pid_t child_pid,
		const unsigned long start)
{
	struct user_regs_arm regs_struct = {};
	struct iovec regs = {.iov_base = &regs_struct, .iov_len = sizeof(regs_struct) };

	REQUIRE (ptrace(PTRACE_GETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);

	regs_struct.uregs[15] = start;

	REQUIRE (ptrace(PTRACE_SETREGSET, child_pid, NT_PRSTATUS, &regs) == 0);
}

void ptrace_child(
		const int exe_fd)
{
	char *const av[] = { NULL };
	char *const ep[] = { NULL };

	REQUIRE (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == 0);

	fexecve(exe_fd, av, ep);

	perror("fexecve");
	exit(EXIT_FAILURE);
}

void ptrace_launch(
		const pid_t child_pid)
{
	int status;
	REQUIRE (waitpid(child_pid, &status, 0) != -1);

	REQUIRE (ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_EXITKILL) == 0);

	REQUIRE (ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACEEXIT) == 0);
}

void ptrace_cont(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	_collect_regs(child_pid, info);

	REQUIRE (ptrace(PTRACE_CONT, child_pid, NULL, NULL) == 0);
}

const
int ptrace_reap(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	// If shellcode forks, this will have to be revisited.
	int status;

	REQUIRE (waitpid(child_pid, &status, 0) != -1);

	if (WIFEXITED(status)) {
		printf("pid %d exited: %d\n", child_pid, WEXITSTATUS(status));
		return 1;
	} if (WIFSIGNALED(status)) {
		printf("pid %d exited on signal %d\n", child_pid, WTERMSIG(status));
		return 1;
	}

	// We've exited
	if (status>>8 == (SIGTRAP | (PTRACE_EVENT_EXIT<<8))) {
		_exited_collect_regs(child_pid, info);
		return 1;
	}

	_collect_regs(child_pid, info);

	if (status>>8 == SIGTRAP)
		return 0;

	// Otherwise pass the signal on to the child process
	printf("pid %d got signal %d, %s.\n",
			child_pid,
			WSTOPSIG(status),
			(options.passsig) ? "delivering" : "not delivering");

	if (options.passsig)
		REQUIRE (ptrace(PTRACE_CONT, child_pid, 0, WSTOPSIG(status)) == 0);

	return 0;
}

void ptrace_detatch(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	REQUIRE (ptrace(PTRACE_DETACH, child_pid, NULL, NULL) == 0);

	int status;
	REQUIRE (waitpid(child_pid, &status, 0) != -1);

	if (WIFEXITED(status))
		info->exit_code = WEXITSTATUS(status);
	if (WIFSIGNALED(status))
		info->sig       = WTERMSIG(status);
}
