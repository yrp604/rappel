#include <stdio.h>

#include "common.h"
#include "arch.h"

#include "display.h"
#include "printfmt.h"

extern struct options_t options;

void display_armv8(
		const struct proc_info_t *const info)
{
	const struct user_regs_armv8    *uregs    = &info->regs_struct;
	const struct user_fpregs_armv8  *fpregs  = &info->fpregs_struct;

	const struct user_regs_armv8   *old_uregs     = &info->old_regs_struct;
	const struct user_fpregs_armv8 *old_fpregs   = &info->old_fpregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	PRINTREG64("X0:  ", regs[0], uregs, old_uregs, "\t");
	PRINTREG64("X1:  ", regs[1], uregs, old_uregs, "\t");
	PRINTREG64("X2:  ", regs[2], uregs, old_uregs, "\t");
	PRINTREG64("X3:  ", regs[3], uregs, old_uregs, "\n");

	PRINTREG64("X4:  ", regs[4], uregs, old_uregs, "\t");
	PRINTREG64("X5:  ", regs[5], uregs, old_uregs, "\t");
	PRINTREG64("X6:  ", regs[6], uregs, old_uregs, "\t");
	PRINTREG64("X7:  ", regs[7], uregs, old_uregs, "\n");

	PRINTREG64("X8:  ", regs[8], uregs, old_uregs, "\t");
	PRINTREG64("X9:  ", regs[9], uregs, old_uregs, "\t");
	PRINTREG64("X10: ", regs[10], uregs, old_uregs, "\t");
	PRINTREG64("X11: ", regs[11], uregs, old_uregs, "\n");

	PRINTREG64("X12: ", regs[12], uregs, old_uregs, "\t");
	PRINTREG64("X13: ", regs[13], uregs, old_uregs, "\t");
	PRINTREG64("X14: ", regs[14], uregs, old_uregs, "\t");
	PRINTREG64("X15: ", regs[15], uregs, old_uregs, "\n");

	PRINTREG64("X16: ", regs[16], uregs, old_uregs, "\t");
	PRINTREG64("X17: ", regs[17], uregs, old_uregs, "\t");
	PRINTREG64("X18: ", regs[18], uregs, old_uregs, "\t");
	PRINTREG64("X19: ", regs[19], uregs, old_uregs, "\n");

	PRINTREG64("X20: ", regs[20], uregs, old_uregs, "\t");
	PRINTREG64("X21: ", regs[21], uregs, old_uregs, "\t");
	PRINTREG64("X22: ", regs[22], uregs, old_uregs, "\t");
	PRINTREG64("X23: ", regs[23], uregs, old_uregs, "\n");

	PRINTREG64("X24: ", regs[24], uregs, old_uregs, "\t");
	PRINTREG64("X25: ", regs[25], uregs, old_uregs, "\t");
	PRINTREG64("X26: ", regs[26], uregs, old_uregs, "\t");
	PRINTREG64("X27: ", regs[27], uregs, old_uregs, "\n");

	PRINTREG64("X28: ", regs[28], uregs, old_uregs, "\t");
	PRINTREG64("X29: ", regs[29], uregs, old_uregs, "\t");
	PRINTREG64("X30: ", regs[30], uregs, old_uregs, "\n");


	PRINTREG64("PC:  ", pc, uregs, old_uregs, "\t");
	PRINTREG64("SP:  ", sp, uregs, old_uregs, "\t");
	PRINTREG64("PS:  ", pstate, uregs, old_uregs, "\n");

	if (options.allregs) {
		printf("FP Regs:\n");
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
