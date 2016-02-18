#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#include <linux/limits.h>

#include "common.h"

#include "assemble.h"

extern struct options_t options;

static 
void _child(
		const int fildes[2],
		int t)
{
	char path[PATH_MAX] = {0};
	snprintf(path, sizeof(path), "/dev/fd/%d", t);

	REQUIRE (dup2(fildes[1], STDOUT_FILENO) != -1);
	REQUIRE (close(fildes[0]) == 0);
	REQUIRE (close(fildes[1]) == 0);

	execlp("nasm", "nasm", "-o", "/dev/stdout", path, (char *)NULL);
	perror("execlp");
	exit(EXIT_FAILURE);
}

const
size_t assemble(
		uint8_t *const bytecode,
		const size_t bytecode_sz,
		const char *const assembly,
		const size_t asm_sz)
{
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "/tmp/rappel-input.XXXXXX");

	const int t = mkstemp(path);

	if (t == -1) {
		perror("mkstemp");
		exit(EXIT_FAILURE);
	}

	REQUIRE (unlink(path) == 0);

	dprintf(t, BITSTR);
	dprintf(t, "section .text vstart=%ld\n", options.start);

	write_data(t, (uint8_t *)assembly, asm_sz);

	int fildes[2];
	REQUIRE (pipe(fildes) == 0);

	const pid_t asm_pid = fork();

	if (asm_pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (asm_pid == 0) {
		_child(fildes, t);
		// Not reached
		abort();
	}

	verbose_printf("nasm is pid %d\n", asm_pid);

	REQUIRE (close(fildes[1]) == 0);

	mem_assign(bytecode, bytecode_sz, TRAP, TRAP_SZ);

	size_t sz = read_data(fildes[0], bytecode, bytecode_sz);

	if (sz >= bytecode_sz) {
		fprintf(stderr, "Too much bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	int status;
	REQUIRE (waitpid(asm_pid, &status, 0) != -1);

	if (WIFSIGNALED(status))
		fprintf(stderr, "nasm exited with signal %d.\n", WTERMSIG(status));
	else if (WIFEXITED(status) && WEXITSTATUS(status))
		fprintf(stderr, "nasm exited %d.\n", WEXITSTATUS(status));

	REQUIRE (close(t) == 0);

	return sz;
}
