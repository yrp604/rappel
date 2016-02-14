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

	if (options.allregs) printf("GP Regs:\n");

	printf("eax:" REGFMT32 "\tebx:" REGFMT32 "\tecx:" REGFMT32 "\tedx:" REGFMT32 "\n",
			regs->eax, regs->ebx, regs->ecx, regs->edx);
	printf("esi:" REGFMT32 "\tedi:" REGFMT32 "\n",
			regs->esi, regs->edi);
	printf("eip:" REGFMT32 "\tesp:" REGFMT32 "\tebp:" REGFMT32 "\n",
			regs->eip, regs->esp, regs->ebp);

	if (options.allregs) {
		printf("cs:" REGFMT32 "\tss:" REGFMT32 "\tds:" REGFMT32 "\n",
				regs->xcs, regs->xss, regs->xds);
		printf("es:" REGFMT32 "\tfs:" REGFMT32 "\tgs:" REGFMT32 "\n",
				regs->xss, regs->xfs, regs->xgs);
	}

	uint8_t zf,cf,sf,pf,af,of;
	of = (regs->eflags & 1024) >> 11;
	sf = (regs->eflags & 128) >> 7;
	zf = (regs->eflags & 64) >> 6;
	af = (regs->eflags & 16) >> 4;
	pf = (regs->eflags & 4) >> 3;
	cf = regs->eflags & 1;
	printf("flags:" REGFMT32 " [CF: %d, ZF: %d, OF: %d, SF: %d, PF: %d, AF: %d]\n", regs->eflags, cf, zf, of, sf, pf, af);

	regs = NULL; // Make sure we dont copy/paste...

	if (options.allregs) {
		printf("FP Regs:\n");
		printf("cwd:" REGFMT32 "\tswd:" REGFMT32 "\ttwd:" REGFMT32 "\tfip:" REGFMT32 "\n",
				fpregs->cwd, fpregs->swd, fpregs->twd, fpregs->fip);
		printf("fcs:" REGFMT32 "\tfoo:" REGFMT32 "\tfos:" REGFMT32 "\n",
				fpregs->fcs, fpregs->foo, fpregs->fos);

		printf("st_space:\n");
		for (uint32_t i = 0; i < 20/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j)
				printf(REGFMT32 "\t", fpregs->st_space[j]);
			printf("\n");
		}

		fpregs = NULL;

		printf("FPX Regs:\n");
		printf("cwd:" REGFMT32 "\tswd:" REGFMT32 "\ttwd:" REGFMT32 "\tfop:" REGFMT32 "\n",
				fpxregs->cwd, fpxregs->swd, fpxregs->twd, fpxregs->fop);
		printf("fip:" REGFMT32 "\tfcs:" REGFMT32 "\tfoo:" REGFMT32 "\tfos:" REGFMT32 "\n",
				fpxregs->fip, fpxregs->fcs, fpxregs->foo, fpxregs->fos);
		printf("mxcsr:" REGFMT32 "\n", fpxregs->mxcsr);
		printf("st_space:\n");
		for (uint32_t i = 0; i < 32/4; ++i) {
			printf("0x%02x:\t", i * 0x10);
			for (uint32_t j = i*4; j < i*4 + 4; ++j)
				printf(REGFMT32 "\t", fpxregs->st_space[j]);
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
