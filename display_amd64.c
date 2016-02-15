#include <stdio.h>

#include "common.h"

#include "display.h"

extern struct options_t options;

void display_amd64(
		const struct proc_info_t *const info)
{
	const struct user_regs_struct_amd64   *regs   = &info->regs_struct;
	const struct user_fpregs_struct_amd64 *fpregs = &info->fpregs_struct;

	const struct user_regs_struct_amd64   *old_regs   = &info->old_regs_struct;
	const struct user_fpregs_struct_amd64 *old_fpregs = &info->old_fpregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	PRINTREG64(rax, regs, old_regs, "\t");
	PRINTREG64(rbx, regs, old_regs, "\t");
	PRINTREG64(rcx, regs, old_regs, "\t");
	PRINTREG64(rdx, regs, old_regs, "\n");

	PRINTREG64(rsi, regs, old_regs, "\t");
	PRINTREG64(rdi, regs, old_regs, "\t");
	PRINTREG64(r8 , regs, old_regs, "\t");
	PRINTREG64(r9 , regs, old_regs, "\n");

	PRINTREG64(r10, regs, old_regs, "\t");
	PRINTREG64(r11, regs, old_regs, "\t");
	PRINTREG64(r12, regs, old_regs, "\t");
	PRINTREG64(r13, regs, old_regs, "\n");

	PRINTREG64(r14, regs, old_regs, "\t");
	PRINTREG64(r15, regs, old_regs, "\n");

	if (options.allregs) {
		PRINTREG64(cs, regs, old_regs, "\t");
		PRINTREG64(ss, regs, old_regs, "\t");
		PRINTREG64(ds, regs, old_regs, "\n");

		PRINTREG64(es, regs, old_regs, "\t");
		PRINTREG64(fs, regs, old_regs, "\t");
		PRINTREG64(gs, regs, old_regs, "\n");
	}

	PRINTREG64(rip, regs, old_regs, "\t");
	PRINTREG64(rsp, regs, old_regs, "\t");
	PRINTREG64(rbp, regs, old_regs, "\n");

	PRINTREG64(eflags, regs, old_regs, " ");

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
		PRINTREG64(rip, fpregs, old_fpregs, "\t");
		PRINTREG64(rdp, fpregs, old_fpregs, "\t");
		PRINTREG32(mxcsr, fpregs, old_fpregs, "\t");
		PRINTREG32(mxcr_mask, fpregs, old_fpregs, "\n");

		PRINTREG16(cwd, fpregs, old_fpregs, "\t");
		PRINTREG16(swd, fpregs, old_fpregs, "\t");
		PRINTREG16(ftw, fpregs, old_fpregs, "\t");
		PRINTREG16(fop, fpregs, old_fpregs, "\n");

		printf("st_space:\n");
		for (uint32_t i = 0; i < 32/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j) {
				DUMPREG32(st_space[j], fpregs, old_fpregs);
				printf("\t");
			}
			printf("\n");
		}

		printf("xmm_space:\n");
		for (uint32_t i = 0; i < 64/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j) {
				DUMPREG32(xmm_space[j], fpregs, old_fpregs);
				printf("\t");
			}
			printf("\n");
		}
	}
	//
	// 5 is sigtrap, which is expected, -1 is initial value
	if (info->sig != 5 && info->sig != -1) {
		printf("Process died with signal: %d\n", info->sig);
		printf("Exited: %ld\n", info->exit_code);
	}
}
