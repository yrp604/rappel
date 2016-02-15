#include <stdio.h>

#include "common.h"

#include "display.h"

extern struct options_t options;

void display_x86(
		const struct proc_info_t *const info)
{
	const struct user_regs_struct_x86    *regs    = &info->regs_struct;
	const struct user_fpregs_struct_x86  *fpregs  = &info->fpregs_struct;
	const struct user_fpxregs_struct_x86 *fpxregs = &info->fpxregs_struct;

	const struct user_regs_struct_x86    *old_regs    = &info->old_regs_struct;
	const struct user_fpregs_struct_x86  *old_fpregs  = &info->old_fpregs_struct;
	const struct user_fpxregs_struct_x86 *old_fpxregs = &info->old_fpxregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	PRINTREG32(eax, regs, old_regs, "\t");
	PRINTREG32(ebx, regs, old_regs, "\t");
	PRINTREG32(ecx, regs, old_regs, "\t");
	PRINTREG32(edx, regs, old_regs, "\n");

	PRINTREG32(esi, regs, old_regs, "\t");
	PRINTREG32(edi, regs, old_regs, "\n");

	PRINTREG32(eip, regs, old_regs, "\t");
	PRINTREG32(esp, regs, old_regs, "\t");
	PRINTREG32(ebp, regs, old_regs, "\n");

	if (options.allregs) {
		printf("cs:"); DUMPREG32(xcs, regs, old_regs); printf("\t");
		printf("ss:"); DUMPREG32(xss, regs, old_regs); printf("\t");
		printf("ds:"); DUMPREG32(xds, regs, old_regs); printf("\n");

		printf("es:"); DUMPREG32(xss, regs, old_regs); printf("\t");
		printf("fs:"); DUMPREG32(xfs, regs, old_regs); printf("\t");
		printf("gs:"); DUMPREG32(xgs, regs, old_regs); printf("\n");
	}

    PRINTREG32(eflags, regs, old_regs, " ");

	const uint8_t of = (regs->eflags & 1024) >> 11;
	const uint8_t old_of = (old_regs->eflags & 1024) >> 11;

	const uint8_t sf = (regs->eflags & 128) >> 7;
	const uint8_t old_sf = (regs->eflags & 128) >> 7;

	const uint8_t zf = (regs->eflags & 64) >> 6;
	const uint8_t old_zf = (old_regs->eflags & 64) >> 6;

	const uint8_t af = (regs->eflags & 16) >> 4;
	const uint8_t old_af = (old_regs->eflags & 16) >> 4;

	const uint8_t pf = (regs->eflags & 4) >> 3;
	const uint8_t old_pf = (old_regs->eflags & 4) >> 3;

	const uint8_t cf = regs->eflags & 1;
	const uint8_t old_cf = old_regs->eflags & 1;

	printf("[");
	PRINTBIT("cf:", cf, old_cf, ", ");
	PRINTBIT("zf:", zf, old_zf, ", ");
	PRINTBIT("of:", of, old_of, ", ");
	PRINTBIT("sf:", sf, old_sf, ", ");
	PRINTBIT("pf:", pf, old_pf, ", ");
	PRINTBIT("af:", af, old_af, "");
	printf("]\n");


	if (options.allregs) {
		printf("FP Regs:\n");
		PRINTREG32(cwd, fpregs, old_fpregs, "\t");
		PRINTREG32(swd, fpregs, old_fpregs, "\t");
		PRINTREG32(twd, fpregs, old_fpregs, "\t");
		PRINTREG32(fip, fpregs, old_fpregs, "\n");

		PRINTREG32(fcs, fpregs, old_fpregs, "\t");
		PRINTREG32(foo, fpregs, old_fpregs, "\t");
		PRINTREG32(fos, fpregs, old_fpregs, "\n");

		printf("st_space:\n");
		for (uint32_t i = 0; i < 20/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j) {
				DUMPREG32(st_space[j], fpregs, old_fpregs);
				printf("\t");
			}
			printf("\n");
		}

		fpregs = NULL;

		printf("FPX Regs:\n");
		PRINTREG32(cwd, fpxregs, old_fpxregs, "\t");
		PRINTREG32(swd, fpxregs, old_fpxregs, "\t");
		PRINTREG32(twd, fpxregs, old_fpxregs, "\t");
		PRINTREG32(fop, fpxregs, old_fpxregs, "\n");

		PRINTREG32(fip, fpxregs, old_fpxregs, "\t");
		PRINTREG32(fcs, fpxregs, old_fpxregs, "\t");
		PRINTREG32(foo, fpxregs, old_fpxregs, "\t");
		PRINTREG32(fos, fpxregs, old_fpxregs, "\n");

		PRINTREG32(mxcsr, fpxregs, old_fpxregs, "\n");

		printf("st_space:\n");
		for (uint32_t i = 0; i < 32/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j) {
				DUMPREG32(st_space[j], fpxregs, old_fpxregs);
				printf("\t");
			}
			printf("\n");
		}
		printf("xmm_space:\n");
		for (uint32_t i = 0; i < 32/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j)
				printf(REGFMT32 "\t", fpxregs->st_space[j]);
			printf("\n");
		}
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
