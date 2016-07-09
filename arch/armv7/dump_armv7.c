#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "arch.h"

void dump_state_armv7(
		const struct proc_info_t *const info)
{
	const struct user_regs_arm   *regs   = &info->regs_struct;
	const struct user_fpregs_arm *fpregs = &info->fpregs_struct;

	write_data(STDOUT_FILENO, (uint8_t *)regs, sizeof(struct user_regs_arm));
	write_data(STDOUT_FILENO, (uint8_t *)fpregs, sizeof(struct user_fpregs_arm));
}
