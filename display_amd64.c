#include <stdio.h>

#include "common.h"

#include "display.h"

extern struct options_t options;

void display_amd64(
		const struct proc_info_t *const info)
{
	const struct user_regs_struct_amd64   *regs   = &info->regs_struct;
	const struct user_fpregs_struct_amd64 *fpregs = &info->fpregs_struct;

	if (options.allregs) printf("GP Regs:\n");

	printf("rax:" REGFMT64 "\trbx:" REGFMT64 "\trcx:" REGFMT64 "\trdx:" REGFMT64 "\n",
			regs->rax, regs->rbx, regs->rcx, regs->rdx);
	printf("rsi:" REGFMT64 "\trdi:" REGFMT64 "\tr8 :" REGFMT64 "\tr9 :" REGFMT64 "\n",
			regs->rsi, regs->rdi, regs->r8, regs->r9);
	printf("r10:" REGFMT64 "\tr11:" REGFMT64 "\tr12:" REGFMT64 "\tr13:" REGFMT64 "\n",
			regs->r10, regs->r11, regs->r12, regs->r13);
	printf("r14:" REGFMT64 "\tr15:" REGFMT64 "\n", regs->r14, regs->r15);
	printf("rip:" REGFMT64 "\trsp:" REGFMT64 "\trbp:" REGFMT64 "\n",
			regs->rip, regs->rsp, regs->rbp);

	if (options.allregs) {
		printf("cs:" REGFMT64 "\tss:" REGFMT64 "\tds:" REGFMT64 "\n",
				regs->cs, regs->ss, regs->ds);
		printf("es:" REGFMT64 "\tfs:" REGFMT64 "\tgs:" REGFMT64 "\n",
				regs->ss, regs->fs, regs->gs);
	}

	uint8_t zf,cf,sf,pf,af,of;
	of = (regs->eflags & 1024) >> 11;
	sf = (regs->eflags & 128) >> 7;
	zf = (regs->eflags & 64) >> 6;
	af = (regs->eflags & 16) >> 4;
	pf = (regs->eflags & 4) >> 3;
	cf = regs->eflags & 1;
	printf("flags:" REGFMT64 " [CF: %d, ZF: %d, OF: %d, SF: %d, PF: %d, AF: %d]\n", regs->eflags, cf, zf, of, sf, pf, af);

	if (options.allregs) {
		printf("FP Regs:\n");
		printf("rip:" REGFMT64 "\trdp:" REGFMT64 "\tmxcsr:" REGFMT32 "\tmxcr_mask:" REGFMT32" \n",
				fpregs->rip, fpregs->rdp, fpregs->mxcsr, fpregs->mxcr_mask);
		printf("cwd:" REGFMT16 "\tswd:" REGFMT16 "\tftw:" REGFMT16 "\tfop:" REGFMT16 "\n",
				fpregs->cwd, fpregs->swd, fpregs->ftw, fpregs->fop);

		printf("st_space:\n");
		for (uint32_t i = 0; i < 32/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j)
				printf(REGFMT32 "\t", fpregs->st_space[j]);
			printf("\n");
		}

		printf("xmm_space:\n");
		for (uint32_t i = 0; i < 64/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j)
				printf(REGFMT32 "\t", fpregs->xmm_space[j]);
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
