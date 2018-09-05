#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <histedit.h>

#include "assemble.h"
#include "common.h"
#include "arch.h"
#include "display.h"
#include "exedir.h"
#include "elf_gen.h"
#include "ptrace.h"
#include "ptrace_arch.h"

#include "ui.h"

extern struct options_t options;
extern int exiting;

int in_block;

static
char const* _prompt(
		EditLine *const e)
{
	if (in_block)
		return "_> ";
	else
		return "> ";
}

static
void _help(void)
{
	printf("Commands:\n");
	printf(".quit                    - quit\n");
	printf(".help                    - display this help\n");
	printf(".info                    - display registers\n");
	printf(".begin                   - start a block, input will not be assembled/run until '.end'\n");
	printf(".end                     - assemble and run the prior block\n");
	printf(".showmap                 - shortcut for cat /proc/<pid>/maps\n");
	printf(".read <address> [amount] - read <amount> bytes of data from address using ptrace [16]\n");
	printf(".write <address> <data>  - write data starting at address using ptrace\n");
}

static
void _ui_read(
		const pid_t child_pid,
		const char *line)
{
	char *dupline = strdup(line);

	if (!dupline) {
		perror("strdup");
		return;
	}

	char *saveptr;

	const char *dotread = strtok_r(dupline, " ", &saveptr);

	if (!dotread || strcasecmp(dotread, ".read"))
		goto bail;

	const char *addr_str = strtok_r(NULL, " ", &saveptr);

	if (!addr_str)
		goto bail;

	errno = 0;
	const unsigned long addr = strtoul(addr_str, NULL, 0);

	if (addr == ULONG_MAX && errno) {
		perror("strtoul");
		goto bail;
	}

	const char *sz_str = strtok_r(NULL, " ", &saveptr);

	unsigned long sz = 0x10;

	if (sz_str && strlen(sz_str)) {
		errno = 0;
		sz = strtoul(sz_str, NULL, 0);

		if (sz == ULONG_MAX && errno) {
			perror("strtoul");
			goto bail;
		}
	}

	uint8_t *buf = xmalloc(sz);

	if (!ptrace_read(child_pid, (void *)addr, buf, sz))
		dump(buf, sz, addr);

	free(buf);

bail:
	free(dupline);
}

static const
pid_t _gen_child(void) {
	uint8_t buf[PAGE_SIZE];
	mem_assign(buf, PAGE_SIZE, TRAP, TRAP_SZ);

	uint8_t *elf;
	const size_t elf_sz = gen_elf(&elf, options.start, (uint8_t *)buf, PAGE_SIZE);

	const int exe_fd = write_exe(elf, elf_sz, options.savefile);

	free(elf);

	const pid_t tracee = fork();

	if (tracee < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (tracee == 0) {
		ptrace_child(exe_fd);
		abort();
	}

	// Parent
	close(exe_fd);

	return tracee;
}

void interact(
		const char *const argv_0)
{
	EditLine *const el = el_init(argv_0, stdin, stdout, stderr);
	el_set(el, EL_PROMPT, &_prompt);
	el_set(el, EL_EDITOR, "emacs");

	History *const hist = history_init();
	if (!hist) {
		fprintf(stderr, "Could not initalize history\n");
		exit(EXIT_FAILURE);
	}

	HistEvent ev;
	history(hist, &ev, H_SETSIZE, 100);

	char hist_path[PATH_MAX] = { 0 };
	snprintf(hist_path, sizeof hist_path, "%s/history", options.rappel_dir);

	history(hist, &ev, H_LOAD, hist_path);

	el_set(el, EL_HIST, history, hist);

	const pid_t child_pid = _gen_child();

	verbose_printf("child process is %d\n", child_pid);

	if (options.verbose) _help();

	char buf[PAGE_SIZE];
	size_t buf_sz = 0;
	int end = 0, child_died = 0;

	struct proc_info_t info = {};
	ARCH_INIT_PROC_INFO(info);

	ptrace_launch(child_pid);
	ptrace_cont(child_pid, &info);
	ptrace_reap(child_pid, &info);

	display(&info);

	for (;;) {
		int count;
		const char *const line = el_gets(el, &count);

		if (count == -1) {
			perror("el_gets");
			exit(EXIT_FAILURE);
		}

		// count is 0 == ^d
		if (!count || strcasestr(line, ".quit") || strcasestr(line, ".exit")) break;

		// We have input, add it to the our history
		history(hist, &ev, H_ENTER, line);

		// If we start with a ., we have a command
		if (line[0] == '.') {
			if (strcasestr(line, "help")) {
				_help();
				continue;
			}

			if (strcasestr(line, "info")) {
				display(&info);
				continue;
			}

			if (strcasestr(line, "showmap")) {
				char cmd[PATH_MAX] = { 0 };
				snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps", child_pid);

				if (system(cmd))
					fprintf(stderr, "sh: %s failed\n", cmd);

				continue;
			}


			if (strcasestr(line, "read")) {
				_ui_read(child_pid, line);
				continue;
			}

			if (strcasestr(line, "write")) {
				continue;
			}

			if (strcasestr(line, "begin")) {
				in_block = 1;
				continue;
			}

			// Note the lack of continue. Need to fall through...
			if (strcasestr(line, "end")) {
				in_block = 0;
				end = 1;
			}
		}

		if (buf_sz + count > sizeof(buf)) {
			printf("Buffer full (max: 0x%lx), please use '.end'\n", sizeof(buf));
			continue;
		}

		// Since we fell through, we want to avoid adding adding .end to our buffer
		if (!end) {
			memcpy(buf + buf_sz, line, count);
			buf_sz += count;
		}

		if (!in_block) {
			verbose_printf("Trying to assemble(%zu):\n%s", buf_sz, buf);

			uint8_t bytecode[PAGE_SIZE];
			const size_t bytecode_sz = assemble(bytecode, sizeof(bytecode), buf, buf_sz);

			memset(buf, 0, sizeof(buf));
			buf_sz = 0;
			end    = 0;

			verbose_printf("Got asm(%zu):\n", bytecode_sz);
			verbose_dump(bytecode, bytecode_sz, -1);

			if (!bytecode_sz) {
				fprintf(stderr, "'%s' assembled to 0 length bytecode\n", buf);
				continue;
			}

			ptrace_write(child_pid, (void *)options.start, bytecode, bytecode_sz);
			ptrace_reset(child_pid, options.start);

			ptrace_cont(child_pid, &info);

			if (ptrace_reap(child_pid, &info)) {
				child_died = 1;
				break;
			}

			display(&info);
		}
	}

	if (!child_died)
		ptrace_detatch(child_pid, &info);

	printf("\n");

	// we close this one with a file pointer so we can truncate the file
	FILE *hist_save = fopen(hist_path, "wb");
	REQUIRE (hist_save != NULL);

	history(hist, &ev, H_SAVE_FP, hist_save);

	REQUIRE (fclose(hist_save) == 0);

	history_end(hist);
	el_end(el);
}
