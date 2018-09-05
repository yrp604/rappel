#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "exedir.h"

#define RAPPEL_DIR ".rappel"

extern struct options_t options;

static const
int _reopen_ro(
		const int h,
		const char *const path)
{
	REQUIRE (close(h) == 0);

	const int ro_h = open(path, O_RDONLY | O_CLOEXEC);

	REQUIRE (ro_h >= 0);

	return ro_h;
}

static
void _clean_rappel_dir(void)
{
	char path[PATH_MAX] = { 0 };
	snprintf(path, sizeof(path), "%s/exe", options.rappel_dir);

	DIR *exedir = opendir(path);

	REQUIRE (exedir != NULL);
	
	struct dirent *f;
	while ((f = readdir(exedir))) {
		if (!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
			continue;

		if (!strcmp(f->d_name, "history"))
			continue;
		
		snprintf(path, sizeof(path), "%s/exe/%s", options.rappel_dir, f->d_name);

		if (unlink(path) == -1)
			fprintf(stderr, "Cannot unlink %s: %s\n", f->d_name, strerror(errno));
	}

	REQUIRE (closedir(exedir) == 0);
}

void init_rappel_dir(void)
{
	const char *home = getenv("HOME");

	if (!home) {
		fprintf(stderr, "HOME not set");
		exit(EXIT_FAILURE);
	}

	snprintf(
		options.rappel_dir,
		sizeof options.rappel_dir,
		"%s/%s",
		home,
		RAPPEL_DIR
	);

	if (mkdir(options.rappel_dir, 0755) == -1)
		REQUIRE (errno == EEXIST);

	char path[PATH_MAX] = { 0 };
	snprintf(path, sizeof path, "%s/%s", options.rappel_dir, "exe");

	if (mkdir(path, 0755) == -1)
		REQUIRE (errno == EEXIST);

	_clean_rappel_dir();
}

const
int write_exe(
		const uint8_t *data,
		const size_t data_sz,
		const char *name)
{
	if (name)
		return write_named_file(data, data_sz, name);
	else
		return write_tmp_file(data, data_sz);
}

const
int write_named_file(
		const uint8_t *data,
		const size_t data_sz,
		const char *name)
{
	const int h = open(name, O_WRONLY | O_CREAT, 
			S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	REQUIRE (h >= 0);

	write_data(h, data, data_sz);

	return _reopen_ro(h, name);
}

const
int write_tmp_file(
		const uint8_t *data,
		const size_t data_sz)
{
	char path[PATH_MAX] = { 0 };

	snprintf(path, sizeof(path), "%s/exe/rappel-exe.XXXXXX", options.rappel_dir);

	const int h = mkstemp(path);

	REQUIRE (h >= 0);

	write_data(h, data, data_sz);

	REQUIRE (fchmod(h, S_IXUSR | S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);

	const int h_ro = _reopen_ro(h, path);

	return h_ro;
}
