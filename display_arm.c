#include <stdio.h>

#include "common.h"

#include "display.h"

extern struct options_t options;

void display_arm(
		const struct proc_info_t *const info)
{
	const struct user_regs_arm   *regs   = &info->regs_struct;
	const struct user_fpregs_arm *fpregs = &info->fpregs_struct;

	const struct user_regs_arm   *old_regs   = &info->old_regs_struct;
	const struct user_fpregs_arm *old_fpregs = &info->old_fpregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	PRINTREG32("R0:  ", uregs[0], regs, old_regs, "\t");
	PRINTREG32("R1:  ", uregs[1], regs, old_regs, "\t");
	PRINTREG32("R2:  ", uregs[2], regs, old_regs, "\t");
	PRINTREG32("R3:  ", uregs[3], regs, old_regs, "\n");

	PRINTREG32("R4:  ", uregs[4], regs, old_regs, "\t");
	PRINTREG32("R5:  ", uregs[5], regs, old_regs, "\t");
	PRINTREG32("R6:  ", uregs[6], regs, old_regs, "\t");
	PRINTREG32("R7:  ", uregs[7], regs, old_regs, "\n");

	PRINTREG32("R8:  ", uregs[8], regs, old_regs, "\t");
	PRINTREG32("R9:  ", uregs[9], regs, old_regs, "\t");
	PRINTREG32("R10: ", uregs[10], regs, old_regs, "\n");

	PRINTREG32("FP:  ", uregs[11], regs, old_regs, "\t");
	PRINTREG32("IP:  ", uregs[12], regs, old_regs, "\n");

	PRINTREG32("PC:  ", uregs[15], regs, old_regs, "\t");
	PRINTREG32("SP:  ", uregs[13], regs, old_regs, "\t");
	PRINTREG32("LR:  ", uregs[14], regs, old_regs, "\n");
	PRINTREG32("APSR:", uregs[16], regs, old_regs, "\n");

	if (options.allregs) {
		printf("FP Regs:\n");
		PRINTREG32("fpsr: ", fpsr, fpregs, old_fpregs, "\t");
		PRINTREG32("fpcr: ", fpcr, fpregs, old_fpregs, "\t");
		PRINTREG32("init_flag: ", init_flag, fpregs, old_fpregs, "\n");

		printf("ftype:");
		for (uint32_t i = 0; i < 8; ++i) {
			PRINTREG8("", ftype[i], fpregs, old_fpregs, " ");
		}
		printf("\n");

		for (uint32_t i = 0; i < 8; ++i) {
			printf("fpreg %d: ", i);
			PRINTBIT("sign1 ", fpregs->fpregs[i].sign1, old_fpregs->fpregs[i].sign1, " ");
			PRINTBIT("sign2 ", fpregs->fpregs[i].sign2, old_fpregs->fpregs[i].sign2, " ");
			PRINTREG16("exponent ", fpregs[i].exponent, fpregs, old_fpregs, " ");
			PRINTREG16("mantissa1 ", fpregs[i].mantissa1, fpregs, old_fpregs, " ");
			PRINTREG16("mantissa0 ", fpregs[i].mantissa0, fpregs, old_fpregs, "\n");
		}
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
