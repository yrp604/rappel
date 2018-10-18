#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include "assemble.h"
#include "common.h"
#include "arch.h"
#include "display.h"
#include "dump_state.h"
#include "elf_gen.h"
#include "exedir.h"
#include "ptrace.h"

#include "pipe.h"

#define STDIN_BUF_SZ 64000000 // 64mb
#define BYTECODE_BUF_SZ 64000000 // 64mb

extern struct options_t options;
extern pid_t tracee;

static const
int _is_ascii(
		const uint8_t *const d,
		const size_t sz)
{
	for (size_t i = 0; i < sz; i++)
		if ((d[i] < 0x20 || d[i] > 0x7f) && d[i] != '\n')
			return 0;

	return 1;
}

static
void _semi_to_linebreak(
		char *const data,
		const size_t data_sz)
{
	for (size_t i = 0; i < data_sz; i++)
		if (data[i] == ';')
			data[i] = '\n';
}

static const
size_t _read_bytecode(
		uint8_t *const data,
		const size_t data_sz)
{
	uint8_t *const raw = xmalloc(STDIN_BUF_SZ);

	const size_t raw_sz = read_data(STDIN_FILENO, raw, STDIN_BUF_SZ);


	// If we've read in 64mb of data, we're just going to assume there's more 
	// we're not reading
	if (raw_sz >= STDIN_BUF_SZ) {
		fprintf(stderr, "Too much bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	if (!_is_ascii(raw, raw_sz) || options.raw) {
		if (raw_sz > data_sz) {
			fprintf(stderr, "Too much bytecode to copy, exiting...\n");
			exit(EXIT_FAILURE);
		} else {
			memcpy(data, raw, raw_sz);
			
			free(raw);

			return raw_sz;
		}
	} else {
		_semi_to_linebreak((char *)raw, raw_sz);

		const size_t asm_sz = assemble(data, data_sz, (char *)raw, raw_sz);
	
		free(raw);

		return asm_sz;
	}
}

void pipe_mode(void)
{
	uint8_t *elf;
	uint8_t *const data = xmalloc(BYTECODE_BUF_SZ);

	const size_t data_sz = _read_bytecode(data, BYTECODE_BUF_SZ);

	if (data_sz >= BYTECODE_BUF_SZ) {
		fprintf(stderr, "Too much assembled bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	if (!data_sz) {
		fprintf(stderr, "No bytecode from assembler, exiting...\n");
		exit(EXIT_FAILURE);
	}

	verbose_printf("Got assembly:\n");
	verbose_dump(data, data_sz, -1);

	const size_t elf_sz = gen_elf(&elf, options.start, data, data_sz);

	free(data);

	const int exe_fd = write_exe(elf, elf_sz, options.savefile);

	free(elf);

	const pid_t tracee = fork();

	if (tracee < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (tracee == 0) {
		ptrace_child(exe_fd);
		abort();
	} else {
		close(exe_fd);
		ptrace_launch(tracee);

		struct proc_info_t info = {};
		ARCH_INIT_PROC_INFO(info);

		ptrace_cont(tracee, &info);

		ptrace_reap(tracee, &info);
		ptrace_detatch(tracee, &info);
		if (options.dump)
			dump_state(&info);
		else
			display(&info);
	}
}
