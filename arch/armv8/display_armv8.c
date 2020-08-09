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

	PRINTREG64(" x0=", regs[0], uregs, old_uregs, "\t");
	PRINTREG64(" x1=", regs[1], uregs, old_uregs, "\t");
	PRINTREG64(" x2=", regs[2], uregs, old_uregs, "\t");
	PRINTREG64(" x3=", regs[3], uregs, old_uregs, "\n");

	PRINTREG64(" x4=", regs[4], uregs, old_uregs, "\t");
	PRINTREG64(" x5=", regs[5], uregs, old_uregs, "\t");
	PRINTREG64(" x6=", regs[6], uregs, old_uregs, "\t");
	PRINTREG64(" x7=", regs[7], uregs, old_uregs, "\n");

	PRINTREG64(" x8=", regs[8], uregs, old_uregs, "\t");
	PRINTREG64(" x9=", regs[9], uregs, old_uregs, "\t");
	PRINTREG64("x10=", regs[10], uregs, old_uregs, "\t");
	PRINTREG64("x11=", regs[11], uregs, old_uregs, "\n");

	PRINTREG64("x12=", regs[12], uregs, old_uregs, "\t");
	PRINTREG64("x13=", regs[13], uregs, old_uregs, "\t");
	PRINTREG64("x14=", regs[14], uregs, old_uregs, "\t");
	PRINTREG64("x15=", regs[15], uregs, old_uregs, "\n");

	PRINTREG64("x16=", regs[16], uregs, old_uregs, "\t");
	PRINTREG64("x17=", regs[17], uregs, old_uregs, "\t");
	PRINTREG64("x18=", regs[18], uregs, old_uregs, "\t");
	PRINTREG64("x19=", regs[19], uregs, old_uregs, "\n");

	PRINTREG64("x20=", regs[20], uregs, old_uregs, "\t");
	PRINTREG64("x21=", regs[21], uregs, old_uregs, "\t");
	PRINTREG64("x22=", regs[22], uregs, old_uregs, "\t");
	PRINTREG64("x23=", regs[23], uregs, old_uregs, "\n");

	PRINTREG64("x24=", regs[24], uregs, old_uregs, "\t");
	PRINTREG64("x25=", regs[25], uregs, old_uregs, "\t");
	PRINTREG64("x26=", regs[26], uregs, old_uregs, "\t");
	PRINTREG64("x27=", regs[27], uregs, old_uregs, "\n");

	PRINTREG64("x28=", regs[28], uregs, old_uregs, "\t");
	PRINTREG64(" fp=", regs[29], uregs, old_uregs, "\t");
	PRINTREG64(" lr=", regs[30], uregs, old_uregs, "\t");
	PRINTREG64(" sp=", sp, uregs, old_uregs, "\n");

	PRINTREG64(" pc=", pc, uregs, old_uregs, "\t");
	PRINTREG64("psr=", pstate, uregs, old_uregs, "\n");

	if (options.allregs) {
		printf("FP Regs:\n");
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
