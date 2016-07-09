#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "arch.h"

void dump_state_x86(
		const struct proc_info_t *const info)
{
	const struct user_regs_struct_x86    *regs    = &info->regs_struct;
	const struct user_fpregs_struct_x86  *fpregs  = &info->fpregs_struct;
	const struct user_fpxregs_struct_x86 *fpxregs = &info->fpxregs_struct;

	write_data(STDOUT_FILENO, (uint8_t *)regs, sizeof(struct user_regs_struct_x86));
	write_data(STDOUT_FILENO, (uint8_t *)fpregs, sizeof(struct user_fpregs_struct_x86));
	write_data(STDOUT_FILENO, (uint8_t *)fpxregs, sizeof(struct user_fpxregs_struct_x86));
}
