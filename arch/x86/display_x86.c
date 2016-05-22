#include <stdio.h>

#include "common.h"
#include "arch.h"

#include "display.h"
#include "printfmt.h"

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

	PRINTREG32("eax: ", eax, regs, old_regs, "\t");
	PRINTREG32("ebx: ", ebx, regs, old_regs, "\t");
	PRINTREG32("ecx: ", ecx, regs, old_regs, "\t");
	PRINTREG32("edx: ", edx, regs, old_regs, "\n");

	PRINTREG32("esi: ", esi, regs, old_regs, "\t");
	PRINTREG32("edi: ", edi, regs, old_regs, "\n");

	PRINTREG32("eip: ", eip, regs, old_regs, "\t");
	PRINTREG32("esp: ", esp, regs, old_regs, "\t");
	PRINTREG32("ebp: ", ebp, regs, old_regs, "\n");

	if (options.allregs) {
		PRINTREG32("cs : ", xcs, regs, old_regs, "\t");
		PRINTREG32("ss : ", xss, regs, old_regs, "\t");
		PRINTREG32("ds : ", xds, regs, old_regs, "\n");

		PRINTREG32("es : ", xss, regs, old_regs, "\t");
		PRINTREG32("fs : ", xfs, regs, old_regs, "\t");
		PRINTREG32("gs : ", xgs, regs, old_regs, "\n");
	}

    PRINTREG32("flags: ", eflags, regs, old_regs, " ");

	const uint8_t of = (regs->eflags & 0x800) >> 11;
	const uint8_t old_of = (old_regs->eflags & 0x800) >> 11;

	const uint8_t df = (regs->eflags & 0x400) >> 10;
	const uint8_t old_df = (old_regs->eflags & 0x400) >> 10;

	const uint8_t sf = (regs->eflags & 0x80) >> 7;
	const uint8_t old_sf = (regs->eflags & 0x80) >> 7;

	const uint8_t zf = (regs->eflags & 0x40) >> 6;
	const uint8_t old_zf = (old_regs->eflags & 0x40) >> 6;

	const uint8_t af = (regs->eflags & 0x10) >> 4;
	const uint8_t old_af = (old_regs->eflags & 0x10) >> 4;

	const uint8_t pf = (regs->eflags & 4) >> 2;
	const uint8_t old_pf = (old_regs->eflags & 4) >> 2;

	const uint8_t cf = regs->eflags & 1;
	const uint8_t old_cf = old_regs->eflags & 1;

	printf("[");
	PRINTBIT("cf:", cf, old_cf, ", ");
	PRINTBIT("zf:", zf, old_zf, ", ");
	PRINTBIT("of:", of, old_of, ", ");
	PRINTBIT("sf:", sf, old_sf, ", ");
	PRINTBIT("pf:", pf, old_pf, ", ");
	PRINTBIT("af:", af, old_af, ", ");
	PRINTBIT("df:", df, old_df, "");
	printf("]\n");


	if (options.allregs) {
		printf("FP Regs:\n");
		PRINTREG32("cwd: ", cwd, fpregs, old_fpregs, "\t");
		PRINTREG32("swd: ", swd, fpregs, old_fpregs, "\t");
		PRINTREG32("twd: ", twd, fpregs, old_fpregs, "\t");
		PRINTREG32("fip: ", fip, fpregs, old_fpregs, "\n");

		PRINTREG32("fcs: ", fcs, fpregs, old_fpregs, "\t");
		PRINTREG32("foo: ", foo, fpregs, old_fpregs, "\t");
		PRINTREG32("fos: ", fos, fpregs, old_fpregs, "\n");

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
		PRINTREG32("cwd: ", cwd, fpxregs, old_fpxregs, "\t");
		PRINTREG32("swd: ", swd, fpxregs, old_fpxregs, "\t");
		PRINTREG32("twd: ", twd, fpxregs, old_fpxregs, "\t");
		PRINTREG32("fop: ", fop, fpxregs, old_fpxregs, "\n");

		PRINTREG32("fip: ", fip, fpxregs, old_fpxregs, "\t");
		PRINTREG32("fcs: ", fcs, fpxregs, old_fpxregs, "\t");
		PRINTREG32("foo: ", foo, fpxregs, old_fpxregs, "\t");
		PRINTREG32("fos: ", fos, fpxregs, old_fpxregs, "\n");

		PRINTREG32("mxcsr: ", mxcsr, fpxregs, old_fpxregs, "\n");

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
			for (uint32_t j = i*4; j < i*4 + 4; ++j) {
				DUMPREG32(st_space[j], fpxregs, old_fpxregs);
				printf("\t");
			}
			printf("\n");
		}
	}

	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
