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

#include <linux/elf.h> // NT_PRSTATUS

#include "common.h"
#include "arch.h"
#include "ptrace.h"

#include "ptrace_arch.h"

extern struct options_t options;

// Round up to the nearest multiple
// https://gist.github.com/aslakhellesoy/1134482
#define ROUNDUP(n, m) n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;

static
void _exited_collect_regs(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	ptrace_collect_regs(child_pid, info);

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

	// Doesn't exist on my armv8 board, so kill it for now...
	//REQUIRE (ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_EXITKILL) == 0);

	REQUIRE (ptrace(PTRACE_SETOPTIONS, child_pid, NULL, PTRACE_O_TRACEEXIT) == 0);
}

void ptrace_cont(
		const pid_t child_pid,
		struct proc_info_t *const info)
{
	ptrace_collect_regs(child_pid, info);

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

	ptrace_collect_regs(child_pid, info);

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
