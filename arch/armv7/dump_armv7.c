#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <msgpack.h>

#include "common.h"
#include "arch.h"

void dump_state_armv7(
		const struct proc_info_t *const info)
{
	const struct user_regs_arm    *regs    = &info->regs_struct;
	const struct user_fpregs_arm  *fpregs  = &info->fpregs_struct;
	const struct user_vfpregs_arm *vfpregs = &info->vfpregs_struct;

	msgpack_sbuffer sbuf;
	msgpack_packer pk;

	msgpack_sbuffer_init(&sbuf);
	msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

	// 16 r regs, 32 d regs, 32 s regs, 16 q regs
	// There are at least 3 fpregs we could also get: fpsid, fpscr, fpexc
	// but its not clear what ptrace values map to what gdb values
	msgpack_pack_map(&pk, 16 + 32 + 32 + 16);

	uint32_t *r = (uint32_t *) regs;
	for (size_t i = 0; i < 16; ++i) {
		char name[32] = { 0 };
		snprintf(name, sizeof(name), "r%zu", i);

		// key
		msgpack_pack_raw(&pk, strlen(name));
		msgpack_pack_raw_body(&pk, name, strlen(name));

		// value
		msgpack_pack_uint32(&pk, r[i]);
	}

	uint32_t *s = (uint32_t *) vfpregs;
	for (size_t i = 0; i < 32; ++i) {
		char name[32] = { 0 };
		snprintf(name, sizeof(name), "s%zu", i);

		// key
		msgpack_pack_raw(&pk, strlen(name));
		msgpack_pack_raw_body(&pk, name, strlen(name));

		// value
		msgpack_pack_uint32(&pk, s[i]);
	}

	uint64_t *d = (uint64_t *) vfpregs;
	for (size_t i = 0; i < 32; ++i) {
		char name[32] = { 0 };
		snprintf(name, sizeof(name), "d%zu", i);

		// key
		msgpack_pack_raw(&pk, strlen(name));
		msgpack_pack_raw_body(&pk, name, strlen(name));

		// value
		msgpack_pack_uint64(&pk, d[i]);
	}

	uint64_t *q = (uint64_t *) vfpregs;
	for (size_t i = 0; i < 16; ++i) {
		char name[32] = { 0 };
		snprintf(name, sizeof(name), "q%zu", i);

		// key
		msgpack_pack_raw(&pk, strlen(name));
		msgpack_pack_raw_body(&pk, name, strlen(name));

		// value
		msgpack_pack_array(&pk, 2);
		msgpack_pack_uint64(&pk, q[i*2]);
		msgpack_pack_uint64(&pk, q[i*2 + 1]);
	}

	write_data(STDOUT_FILENO, (uint8_t *) sbuf.data, sbuf.size);
	msgpack_sbuffer_destroy(&sbuf);
}
