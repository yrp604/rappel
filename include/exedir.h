#include <stdint.h>

void clean_exedir();

const
int write_exe(
		const uint8_t *,
		const size_t,
		const char *);

const
int write_tmp_file(
		const uint8_t *,
		const size_t);

const
int write_named_file(
		const uint8_t *,
		const size_t,
		const char *);
