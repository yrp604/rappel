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
void _child_assemble(
		const int fildes[2],
		int t)
{
	char path[PATH_MAX] = {0};
	snprintf(path, sizeof(path), "/dev/fd/%d", t);

	REQUIRE (dup2(fildes[0], STDIN_FILENO) != -1);

	REQUIRE (close(fildes[0]) == 0);
	REQUIRE (close(fildes[1]) == 0);

	execlp("as", "as", "-mfpu=neon", "-o", path, "-", (char *)NULL);
	perror("execlp");
	exit(EXIT_FAILURE);
}

static
void _child_objcopy(
	const int filedes[2],
	int t)
{
	char path[PATH_MAX] = {0};
	snprintf(path, sizeof(path), "/dev/fd/%d", t);

	REQUIRE (dup2(filedes[1], STDOUT_FILENO) != -1);

	REQUIRE (close(filedes[0]) == 0);
	REQUIRE (close(filedes[1]) == 0);

	execlp("objcopy", "objcopy", "-O", "binary", path, "/dev/stdout", (char *)NULL);
	perror("execlp");
	exit(EXIT_FAILURE);
}

const
size_t assemble_armv7(
		uint8_t *const bytecode,
		const size_t bytecode_sz,
		const char *const assembly,
		const size_t asm_sz)
{
	size_t sz = 0;

	char path[PATH_MAX];
	snprintf(path, sizeof(path), "/tmp/rappel-output.XXXXXX");

	const int t = mkstemp(path);

	if (t == -1) {
		perror("mkstemp");
		exit(EXIT_FAILURE);
	}

	REQUIRE (unlink(path) == 0);

	int fildes[2] = {-1, -1};
	REQUIRE (pipe(fildes) == 0);

	const pid_t asm_pid = fork();

	if (asm_pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (asm_pid == 0) {
		_child_assemble(fildes, t);
		// Not reached
		abort();
	}

	verbose_printf("as is pid %d\n", asm_pid);

	REQUIRE (close(fildes[0]) == 0);

	write_data(fildes[1], (uint8_t *)assembly, asm_sz);

	REQUIRE (close(fildes[1]) == 0);

	int asm_status = -1;
	REQUIRE (waitpid(asm_pid, &asm_status, 0) != -1);

	if (WIFSIGNALED(asm_status)) {
		fprintf(stderr, "as exited with signal %d.\n", WTERMSIG(asm_status));
	} else if (WIFEXITED(asm_status) && WEXITSTATUS(asm_status)) {
		fprintf(stderr, "as exited %d.\n", WEXITSTATUS(asm_status));
		goto exit;
	}

	REQUIRE (lseek(t, SEEK_SET, 0) != -1);

	int results[2] = {-1, -1};
	REQUIRE (pipe(results) == 0);

	const pid_t objcopy_pid = fork();

	if (objcopy_pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (objcopy_pid == 0) {
		_child_objcopy(results, t);
		// Not reached
		abort();
	}

	verbose_printf("objcopy is pid %d\n", objcopy_pid);

	REQUIRE (close(results[1]) == 0);

	mem_assign(bytecode, bytecode_sz, TRAP, TRAP_SZ);

	sz = read_data(results[0], bytecode, bytecode_sz);

	REQUIRE (close(results[0]) == 0);

	if (sz >= bytecode_sz) {
		fprintf(stderr, "Too much bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	int objcopy_status = -1;
	REQUIRE (waitpid(objcopy_pid, &objcopy_status, 0) != -1);

	if (WIFEXITED(objcopy_status) && WIFSIGNALED(objcopy_status))
		fprintf(stderr, "objcopy exited with signal %d.\n", WTERMSIG(objcopy_status));
	else if (WIFEXITED(objcopy_status) && WEXITSTATUS(objcopy_status))
		fprintf(stderr, "objcopy exited %d.\n", WEXITSTATUS(objcopy_status));

exit:
	REQUIRE (close(t) == 0);

	return sz;
}
