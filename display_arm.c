#include <stdio.h>

#include "common.h"

#include "display.h"

extern struct options_t options;

void display_arm(
		const struct proc_info_t *const info)
{
	const struct user_regs_arm   *regs   = &info->regs_struct;
	const struct user_fpregs_arm *fpregs = &info->fpregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	printf("R0 :" REGFMT32 "\tR1 :" REGFMT32 "\tR2 :" REGFMT32 " \tR3 :" REGFMT32"\n",
			regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3]);
	printf("R4 :" REGFMT32 "\tR5 :" REGFMT32 "\tR6 :" REGFMT32 " \tR7 :" REGFMT32"\n",
			regs->uregs[4], regs->uregs[5], regs->uregs[6], regs->uregs[7]);
	printf("R8 :" REGFMT32 "\tR9 :" REGFMT32 "\tR10:" REGFMT32 " \tR11:" REGFMT32"\n",
			regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11]);
	printf("R12 :" REGFMT32 "\tR13 :" REGFMT32 "\tR14:" REGFMT32 " \tR15:" REGFMT32"\n",
			regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15]);
	printf("R16 :" REGFMT32 "\tR17 :" REGFMT32 "\n",
			regs->uregs[16], regs->uregs[17]);

	if (options.allregs) {
		printf("FP Regs:\n");
		printf("fpsr:" REGFMT32"\tfpcr:" REGFMT32 "\tinit_flag: " REGFMT32 "\n", fpregs->fpsr, fpregs->fpcr, fpregs->init_flag);
		printf("ftype:");
		for (uint32_t i = 0; i < 8; ++i)
			printf(REGFMT8 " ", fpregs->ftype[i]);
		printf("\n");

		for (uint32_t i = 0; i < 8; ++i)
			printf("fpreg %d: sign1 %d sign2 %d exponent " REGFMT16 " j %d mantissa1 " REGFMT32 " mantissa2 " REGFMT32 "\n",
					fpregs->fpregs[i].sign1, fpregs->fpregs[i].sign2, fpregs->fpregs[i].exponent, fpregs->fpregs[i].j,
					fpregs->fpregs[i].mantissa1, fpregs->fpregs[i].mantissa0);
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
