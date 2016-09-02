/*
Copyright (c) 2016, Matt Mullins
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

extern "C" {
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#include <linux/limits.h>

#include "common.h"
#include "arch.h"

#include "assemble.h"
}

#include <iomanip>
#include <iostream>

#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCELFStreamer.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCObjectFileInfo.h>
#include <llvm/MC/MCParser/MCAsmParser.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSection.h>
#include <llvm/MC/MCSectionELF.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/MCTargetAsmParser.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Support/ELF.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

extern struct options_t options;

template<typename T>
static const T* string_offset(
		const llvm::StringRef& s, size_t offset,
		size_t size = sizeof(T)) {
	if (offset + size > s.size()) {
		return nullptr;
	}
	return (const T*)(s.data() + offset);
}

extern "C"
const
size_t assemble(
		uint8_t *const bytecode,
		const size_t bytecode_sz,
		const char *const assembly,
		const size_t asm_sz)
{
	using std::unique_ptr;

	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmParser();

	/* TODO: this is only used for MCObjectFileInfo init.  Why? */
	const llvm::Triple triple{LLVM_HOST_TRIPLE};

	std::string error;
	const llvm::Target* target =
		llvm::TargetRegistry::lookupTarget(LLVM_HOST_TRIPLE, error);
	if (!target) {
		std::cerr << "Could not find target: " << error << std::endl;
		exit(EXIT_FAILURE);
	}

	unique_ptr<llvm::MCSubtargetInfo> subtarget(
		target->createMCSubtargetInfo(LLVM_HOST_TRIPLE, "", "")
	);

	llvm::SourceMgr source;
	source.AddNewSourceBuffer(
		llvm::MemoryBuffer::getMemBufferCopy(
			llvm::StringRef(assembly, asm_sz)
		),
		llvm::SMLoc()
	);

	unique_ptr<llvm::MCRegisterInfo> reginfo(
		target->createMCRegInfo(LLVM_HOST_TRIPLE)
	);
	unique_ptr<llvm::MCAsmInfo> asminfo(
		target->createMCAsmInfo(*reginfo, LLVM_HOST_TRIPLE)
	);
	llvm::MCObjectFileInfo mofi;
	llvm::MCContext ctx(asminfo.get(), reginfo.get(), &mofi, &source);
	mofi.InitMCObjectFileInfo(
		triple, llvm::Reloc::Static, llvm::CodeModel::Default, ctx
	);

	llvm::SmallVector<char, 0> output;
	llvm::raw_svector_ostream stream(output);
	llvm::MCAsmBackend* backend =
		target->createMCAsmBackend(*reginfo, LLVM_HOST_TRIPLE, "");
	unique_ptr<llvm::MCInstrInfo> instrinfo(target->createMCInstrInfo());
	llvm::MCCodeEmitter* emitter = target->createMCCodeEmitter(
		*instrinfo,
		*reginfo,
		ctx);
	/* the MCObjectStreamer deletes the MCAsmBackend and MCCodeEmitter */
	unique_ptr<llvm::MCStreamer> streamer(target->createMCObjectStreamer(
		triple,
		ctx,
		*backend,
		stream,
		emitter,
		*subtarget,
		false,
		false,
		false));

	unique_ptr<llvm::MCAsmParser> asmparser(llvm::createMCAsmParser(
		source,
		ctx,
		*streamer,
		*asminfo));

	llvm::MCTargetOptions options;
	unique_ptr<llvm::MCTargetAsmParser> targetasmparser(
		target->createMCAsmParser(
			*subtarget,
			*asmparser,
			*instrinfo,
			options
		)
	);

	asmparser->setTargetParser(*targetasmparser);

	mem_assign(bytecode, bytecode_sz, TRAP, TRAP_SZ);

	llvm::MCSection* section = ctx.getELFSection(
		"user_input",
		llvm::ELF::SHT_PROGBITS,
		llvm::ELF::SHF_ALLOC|llvm::ELF::SHF_EXECINSTR);
	streamer->SwitchSection(section);

	asmparser->Run(true, true);

	streamer->Finish();
	const llvm::StringRef buffer = stream.str();

	if (buffer.substr(0, 4) != llvm::ELF::ElfMagic) {
		std::cerr << "I should have gotten an ELF file\n";
		abort();
	}

	const struct llvm::ELF::Elf64_Ehdr* filehdr =
		string_offset<llvm::ELF::Elf64_Ehdr>(buffer, 0);
	if (!filehdr) return 0;

	/* Find the section with type PROGBITS */
	const struct llvm::ELF::Elf64_Shdr* shdr;
	size_t end_offset = filehdr->e_shoff +
		filehdr->e_shnum * sizeof(llvm::ELF::Elf64_Shdr);
	for (size_t offset = filehdr->e_shoff;
			offset < end_offset;
			offset += sizeof(llvm::ELF::Elf64_Shdr)) {
		shdr = string_offset<llvm::ELF::Elf64_Shdr>(buffer, offset);
		if (!shdr) return 0;
		if (shdr->sh_type == llvm::ELF::SHT_PROGBITS) break;
	}

	// Not found?
	if (shdr->sh_type != llvm::ELF::SHT_PROGBITS) return 0;

	size_t sz = shdr->sh_size;
	const char *from_elf = string_offset<char>(buffer, shdr->sh_offset, sz);
	if (!from_elf) return 0;

	if (sz >= bytecode_sz) {
		fprintf(stderr, "Too much bytecode to handle, exiting...\n");
		exit(EXIT_FAILURE);
	}

	memcpy(bytecode, from_elf, sz);

	return sz;
}
