#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#include <linux/limits.h>

#include "common.h"
#include "arch.h"

#include "assemble.h"

extern struct options_t options;

static 
void _child(
		const int out[2],
		const int err[2],
		int t)
{
	char path[PATH_MAX] = {0};
	snprintf(path, sizeof(path), "/dev/fd/%d", t);

	REQUIRE (dup2(out[1], STDOUT_FILENO) != -1);
	REQUIRE (close(out[0]) == 0);
	REQUIRE (close(out[1]) == 0);

	REQUIRE (dup2(err[1], STDERR_FILENO) != -1);
	REQUIRE (close(err[0]) == 0);
	REQUIRE (close(err[1]) == 0);

	execlp("nasm", "nasm", "-o", "/proc/self/fd/1", path, (char *)NULL);
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

	int out[2] = { -1, -1};
	REQUIRE (pipe(out) == 0);

	int err[2] = { -1, -1};
	REQUIRE (pipe(err) == 0);

	const pid_t asm_pid = fork();

	if (asm_pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (asm_pid == 0) {
		_child(out, err, t);
		// Not reached
		abort();
	}

	REQUIRE (close(out[1]) == 0);
	REQUIRE (close(err[1]) == 0);

	verbose_printf("nasm is pid %d\n", asm_pid);

	mem_assign(bytecode, bytecode_sz, TRAP, TRAP_SZ);

	const size_t sz = read_data(out[0], bytecode, bytecode_sz);

	REQUIRE(close(out[0]) == 0);

	if (sz >= bytecode_sz) {
		fprintf(stderr, "Too much bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	char err_text[0x1000] = { 0 };

	const size_t err_sz = read_data(err[0], (uint8_t *) err_text, sizeof(err_text));

	REQUIRE(close(err[0]) == 0);

	int status = -1;
	REQUIRE (waitpid(asm_pid, &status, 0) != -1);

	if (WIFSIGNALED(status))
		fprintf(stderr, "nasm exited with signal %d.\n", WTERMSIG(status));
	else if (WIFEXITED(status) && WEXITSTATUS(status))
		fprintf(stderr, "nasm exited %d.\n", WEXITSTATUS(status));

	REQUIRE (close(t) == 0);

	if (err_sz) {
		fprintf(stderr, "%s", err_text);
		return 0;
	}

	return sz;
}
