#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <sys/wait.h>

#include "common.h"
#include "exedir.h"
#include "pipe.h"
#include "ui.h"

// Defaults
struct options_t options = {
	.start = 0x400000,
	.verbose = 0,
	.raw = 0,
	.allregs = 0,
	.savefile = NULL,
	.rappel_dir = { 0 }
};

static
void _usage(
		const char *argv0)
{
	fprintf(stderr, "Usage: %s [options]\n"
			"\t-h\t\tDisplay this help\n"
			"\t-r\t\tTreat stdin as raw bytecode (useful for ascii shellcode)\n"
			"\t-p\t\tPass signals to child process (will allow child to kill itself via SIGSEGV, others)\n"
			"\t-s <filename>\tSave generated exe to <filename>\n"
			"\t-x\t\tDisplay all registers (FP)\n"
			"\t-v\t\tIncrease verbosity\n"
			, argv0);

	exit(EXIT_FAILURE);
}

static
void _parse_opts(
		int argc,
		char **argv) {
	int c;

	while ((c = getopt(argc, argv, "s:dhrpvx")) != -1)
		switch (c) {
			case 'h':
				_usage(argv[0]);
				break;
			case 'd':
				++options.dump;
				break;
			case 'r':
				++options.raw;
				break;
			case 's':
				options.savefile = optarg;
				break;
			case 'p':
				++options.passsig;
				break;
			case 'v':
				++options.verbose;
				break;
			case 'x':
				++options.allregs;
				break;
			default:
				exit(EXIT_FAILURE);
		}
}

int main(int argc, char **argv) {
	_parse_opts(argc, argv);

	init_rappel_dir();

	if (isatty(STDIN_FILENO))
		interact(argv[0]);
	else
		pipe_mode();

	return 0;
}
