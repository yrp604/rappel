#include <linux/elf.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "arch.h"

#include "elf_gen.h"

const size_t gen_elf_armv7(
		uint8_t **out,
		const unsigned long start,
		const uint8_t *const code,
		const size_t code_sz)
{
	/* We give the elf header and phdr an entire page, because the elf loader can
	 * only map the file at PAGE_SIZE offsets. So our file will look like this 
	 * for an invocation with some code and 2 data segments.
	 *
	 * +----------+
	 * | 1st page |
	 * | ehdr     |
	 * | phdr     |
	 * | shdr     |
	 * | shdr     |
	 * |----------|
	 * | 2nd page |
	 * | code     |
	 * |----------|
	 * | 3rd page |
	 * | data 1   |
	 * |----------|
	 * | 4th page |
	 * | data 2   |
	 * +----------+
	 *
	 * TODO add data section, section headers
	*/

	const size_t pg_align_dist = start - (start & ~0xffff);
	const size_t pad_sz = ((code_sz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) - code_sz;
	const size_t sz = PAGE_SIZE + pg_align_dist + code_sz + pad_sz;

	uint8_t *const e = xmalloc(sz);
	mem_assign(e, sz, TRAP, TRAP_SZ);

	Elf32_Ehdr *const ehdr = (Elf32_Ehdr *) e;
	
	ehdr->e_ident[0] = ELFMAG0;
	ehdr->e_ident[1] = ELFMAG1;
	ehdr->e_ident[2] = ELFMAG2;
	ehdr->e_ident[3] = ELFMAG3;
	ehdr->e_ident[4] = ELFCLASS32;
	ehdr->e_ident[5] = ELFDATA2LSB;
	ehdr->e_ident[6] = EV_CURRENT;
	ehdr->e_ident[7] = ELFOSABI_NONE;
	ehdr->e_ident[9] = 0;
	// Padding
	ehdr->e_type = ET_EXEC;
	ehdr->e_machine = EM_ARM;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_entry = start;
	ehdr->e_phoff = sizeof(Elf32_Ehdr);
	ehdr->e_shoff = 0; // XXX
	ehdr->e_flags = 0;
	ehdr->e_ehsize = sizeof(Elf32_Ehdr);
	ehdr->e_phentsize = sizeof(Elf32_Phdr);
	ehdr->e_phnum = 1;
	ehdr->e_shentsize = 0;
	ehdr->e_shnum = 0;
	ehdr->e_shstrndx = 0;

	Elf32_Phdr *const phdr = (Elf32_Phdr *) ((uint8_t *) e + sizeof(Elf32_Ehdr));

	phdr->p_type = PT_LOAD;
	phdr->p_flags = PF_X | PF_R;
	phdr->p_offset = PAGE_SIZE;
	phdr->p_vaddr = start - pg_align_dist;
	phdr->p_paddr = 0;
	phdr->p_filesz = code_sz + pg_align_dist; 
	phdr->p_memsz = code_sz + pg_align_dist;
	phdr->p_align = 0x4;

	uint8_t *const data = (uint8_t *) e + PAGE_SIZE + pg_align_dist;
	memcpy(data, code, code_sz);

	*out = e;

	return sz;
}
