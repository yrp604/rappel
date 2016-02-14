#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <pid> <memory address>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	unsigned int pid   = strtol(argv[1], 0, 0);
	unsigned long addr = strtoull(argv[2], 0, 0);

	char path[PATH_MAX];
	snprintf(path, sizeof path, "/proc/%d/mem", pid);
	
	fprintf(stderr, "Trying open: %s ...\n", path);
	const int h = open(path, O_RDWR);
	if (h < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (lseek(h, addr, SEEK_SET) == (addr - 1)) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "Trying read: 0x%08lx ...\n", addr);

	unsigned long data;
	if (read(h, &data, 8) < 8) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("0x%08lx: %08lx\n", addr, data);

// Below was a test for irc people... 
#if 0
	if (lseek(h, addr, SEEK_SET) == (addr - 1)) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	unsigned char buf[] = "\x41\x41\x41\x41";

	if (write(h, buf, 4) < 4) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	if (lseek(h, addr, SEEK_SET) == (addr - 1)) {
		perror("lseek");
		exit(EXIT_FAILURE);
	}

	if (read(h, &data, 8) < 8) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("0x%08lx: %08lx\n", addr, data);
#endif

	exit(EXIT_SUCCESS);
	return 0;
}
