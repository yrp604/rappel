# rappel

Rappel is a pretty janky assembly REPL. It works by creating a shell ELF, starting it under ptrace, then continiously rewriting/running the `.text` section, while showing the register states. It's maybe half done right now, and supports Linux x86, amd64, and armv7 (no thumb) at the moment.

## Install

The only dependencies are libedit an assembler (nasm on x86/amd64, as on ARM), which on debian can be installed with the `libedit-dev` and `nasm`/`binutils` packages.  To use the LLVM assembler on amd64, the LLVM development headers are availble in the `llvm-3.8-dev` package on Ubuntu, and `nasm` is not required.  Please note, as `rappel` require the ability to write to executable memory via `ptrace`, the program is broken under `PAX_MPROTECT` on grsec kernels (see [#2](https://github.com/yrp604/rappel/issues/2)).

```
$ CC=clang make
```

It should work fine with `gcc`, albeit with a few more warnings.

By default rappel is compiled with your native architecture. If you're on amd64 and want to target x86 you can do this with

```
$ ARCH=x86 CC=clang make
```

In theory you can also compile an armv7 binary this way, but I really doubt it will work. For rappel to function, the architecture of the main rappel binary must match that of the process it creates, and the host must be able to run binaries of this architecture.

The LLVM assembler is also available for amd64 hosts.  To use it, you must pass `ARCH=amd64_llvm` when building rappel; by default, the nasm assembler will be used.  To specify a custom path for the LLVM 3.8 `llvm-config` utility in your build environment, the `LLVM_CONFIG` variable may also be specified.  Note: unlike nasm, the LLVM assembler uses AT&T syntax -- operations on x86 will differ from Intel's documentation.  See the [GNU as documentation](https://sourceware.org/binutils/docs/as/i386_002dVariations.html#i386_002dVariations) for details.

## Running

Rappel has two modes it can operate in. A pipe mode for one off things, a la

```
$ echo "inc eax" | bin/rappel
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400003  rsp:0x00007fffffffee80  rbp:0x0000000000000000
flags:0x0000000000000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
$
```

Or an interactive mode:

```
$ bin/rappel
rax:0x0000000000000000  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400001  rsp:0x00007fffffffee80  rbp:0x0000000000000000
flags:0x0000000000000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
> inc rax
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400004  rsp:0x00007fffffffee80  rbp:0x0000000000000000
flags:0x0000000000000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
> push rax
rax:0x0000000000000001  rbx:0x0000000000000000  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400002  rsp:0x00007fffffffee78  rbp:0x0000000000000000
flags:0x0000000000000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
> pop rbx
rax:0x0000000000000001  rbx:0x0000000000000001  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400002  rsp:0x00007fffffffee80  rbp:0x0000000000000000
flags:0x0000000000000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
> cmp rax, rbx
rax:0x0000000000000001  rbx:0x0000000000000001  rcx:0x0000000000000000  rdx:0x0000000000000000
rsi:0x0000000000000000  rdi:0x0000000000000000  r8 :0x0000000000000000  r9 :0x0000000000000000
r10:0x0000000000000000  r11:0x0000000000000000  r12:0x0000000000000000  r13:0x0000000000000000
r14:0x0000000000000000  r15:0x0000000000000000
rip:0x0000000000400004  rsp:0x00007fffffffee80  rbp:0x0000000000000000
flags:0x0000000000000246 [CF: 0, ZF: 1, OF: 0, SF: 0, PF: 0, AF: 0]
> ^D
$
```

x86 looks like:
```
$ echo "nop" | bin/rappel
eax:0x00000000  ebx:0x00000000  ecx:0x00000000  edx:0x00000000
esi:0x00000000  edi:0x00000000
eip:0x00400002  esp:0xffffdf10  ebp:0x00000000
flags:0x00000202 [CF: 0, ZF: 0, OF: 0, SF: 0, PF: 0, AF: 0]
$
```

ARM looks like:
```
$ echo "nop" | bin/rappel
R0 :0x00000000	R1 :0x00000000	R2 :0x00000000	R3 :0x00000000
R4 :0x00000000	R5 :0x00000000	R6 :0x00000000	R7 :0x00000000
R8 :0x00000000	R9 :0x00000000	R10:0x00000000
FP :0x00000000	IP :0x00000000
SP :0xbe927f30	LR :0x00000000	PC :0x00400004
APSR:0x00000010
$
```

## Notes
Someone asked about xmm registers. If you pass `-x` it will dump out quite a bit of info.

```
> inc rax
GP Regs:
rax: 0x0000000000000001 rbx: 0x0000000000000000 rcx: 0x0000000000000000 rdx: 0x0000000000000000
rsi: 0x0000000000000000 rdi: 0x0000000000000000 r8 : 0x0000000000000000 r9 : 0x0000000000000000
r10: 0x0000000000000000 r11: 0x0000000000000000 r12: 0x0000000000000000 r13: 0x0000000000000000
r14: 0x0000000000000000 r15: 0x0000000000000000
cs: 0x0000000000000033  ss: 0x000000000000002b  ds: 0x0000000000000000
es: 0x0000000000000000  fs: 0x0000000000000000  gs: 0x0000000000000000
rip: 0x0000000000400004 rsp: 0x00007fffffffee80 rbp: 0x0000000000000000
flags: 0x0000000000000202 [cf:0, zf:0, of:0, sf:0, pf:0, af:0]
FP Regs:
rip: 0x0000000000000000 rdp: 0x0000000000000000 mxcsr: 0x00001f80       mxcsr_mask:0x00000000
cwd: 0x037f     swd: 0x0000     ftw: 0x0000     fop: 0x0000
st_space:
0x00:   0x00000000      0x00000000      0x00000000      0x00000000
0x10:   0x00000000      0x00000000      0x00000000      0x00000000
0x20:   0x00000000      0x00000000      0x00000000      0x00000000
0x30:   0x00000000      0x00000000      0x00000000      0x00000000
0x40:   0x00000000      0x00000000      0x00000000      0x00000000
0x50:   0x00000000      0x00000000      0x00000000      0x00000000
0x60:   0x00000000      0x00000000      0x00000000      0x00000000
0x70:   0x00000000      0x00000000      0x00000000      0x00000000
xmm_space:
0x00:   0x00000000      0x00000000      0x00000000      0x00000000
0x10:   0x00000000      0x00000000      0x00000000      0x00000000
0x20:   0x00000000      0x00000000      0x00000000      0x00000000
0x30:   0x00000000      0x00000000      0x00000000      0x00000000
0x40:   0x00000000      0x00000000      0x00000000      0x00000000
0x50:   0x00000000      0x00000000      0x00000000      0x00000000
0x60:   0x00000000      0x00000000      0x00000000      0x00000000
0x70:   0x00000000      0x00000000      0x00000000      0x00000000
0x80:   0x00000000      0x00000000      0x00000000      0x00000000
0x90:   0x00000000      0x00000000      0x00000000      0x00000000
0xa0:   0x00000000      0x00000000      0x00000000      0x00000000
0xb0:   0x00000000      0x00000000      0x00000000      0x00000000
0xc0:   0x00000000      0x00000000      0x00000000      0x00000000
0xd0:   0x00000000      0x00000000      0x00000000      0x00000000
0xe0:   0x00000000      0x00000000      0x00000000      0x00000000
0xf0:   0x00000000      0x00000000      0x00000000      0x00000000
```

There are some other regsets the kernel exports via ptrace(), but they're dependent on kernel version, and didn't want to try to detect and adjust at runtime. If you want them, you should just need to add the storage in `proc_info_t`, edit `ptrace_collect_regs_<arch>()`, then add the display in the relevant `display` function.

Right now platforms are largely determined by what hardware I own. I plan on splitting it apart a bit more in the future to make adding new archs easier.

## Docs

You can get pretty much all the documentation with either `-h` from the command line, or `.help` from the interactive bit.
