# rappel

[![Build Status](https://dev.azure.com/yrp604/rappel/_apis/build/status/yrp604.rappel?branchName=master)](https://dev.azure.com/yrp604/rappel/_build/latest?definitionId=1&branchName=master)

Rappel is a pretty janky assembly REPL. It works by creating a shell ELF, starting it under ptrace, then continiously rewriting/running the `.text` section, while showing the register states. It's maybe half done right now, and supports Linux x86, amd64, armv7 (no thumb), and armv8 at the moment.

* If you're looking for a Windows version, please see [@zerosum0x0](https://twitter.com/zerosum0x0)'s [WinREPL](https://github.com/zerosum0x0/WinREPL)
* If you're looking for a macOS version, please see [@tyilol](https://twitter.com/tyilol)'s [asm_repl](https://github.com/Tyilo/asm_repl)
* If you're looking for a hacked together with gdb and python version, please see amtal's [rappel.py](https://gist.github.com/amtal/c457176af7f8770e0ad519aadc86013c/)

## Install

The only dependencies are `libedit` and an assembler (`nasm` on x86/amd64, `as` on ARM) , which on debian can be installed with the `libedit-dev` and `nasm`/`binutils` packages. Please note, as `rappel` require the ability to write to executable memory via `ptrace`, the program is broken under `PAX_MPROTECT` on grsec kernels (see [#2](https://github.com/yrp604/rappel/issues/2)).

```
$ CC=clang make
```

It should work fine with `gcc`, albeit with a few more warnings.

By default rappel is compiled with your native architecture. If you're on amd64 and want to target x86 you can do this with

```
$ ARCH=x86 CC=clang make
```

In theory you can also compile an armv7 binary this way, but I really doubt it will work. For rappel to function, the architecture of the main rappel binary must match that of the process it creates, and the host must be able to run binaries of this architecture.

## Running

Rappel has two modes it can operate in. A pipe mode for one off things, a la

```
$ echo "inc eax" | bin/rappel
rax=0000000000000001 rbx=0000000000000000 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400004 rsp=00007ffc73019c20 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
$
```

Or an interactive mode:

```
$ bin/rappel
rax=0000000000000000 rbx=0000000000000000 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400001 rsp=00007ffdedb264a0 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
> inc rax
rax=0000000000000001 rbx=0000000000000000 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400004 rsp=00007ffdedb264a0 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
> push rax
rax=0000000000000001 rbx=0000000000000000 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400002 rsp=00007ffdedb26498 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
> pop rbx
rax=0000000000000001 rbx=0000000000000001 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400002 rsp=00007ffdedb264a0 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
> cmp rax, rbx
rax=0000000000000001 rbx=0000000000000001 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400004 rsp=00007ffdedb264a0 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:1, of:0, sf:0, pf:1, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000246
> ^D
$
```

x86 looks like:
```
$ echo "nop" | bin/rappel
eax=00000000 ebx=00000000 ecx=00000000 edx=00000000 esi=00000000 edi=00000000
eip=00400002 esp=ffc67240 ebp=00000000 [cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0023  ss=002b  ds=002b  es=002b  fs=0000  gs=0000            efl=00000202
$
```

ARMv7 looks like:
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

ARMv8 looks like:
```
$ echo "nop" | bin/rappel
X0:  0x0000000000000000	X1:  0x0000000000000000	X2:  0x0000000000000000	X3:  0x0000000000000000
X4:  0x0000000000000000	X5:  0x0000000000000000	X6:  0x0000000000000000	X7:  0x0000000000000000
X8:  0x0000000000000000	X9:  0x0000000000000000	X10: 0x0000000000000000	X11: 0x0000000000000000
X12: 0x0000000000000000	X13: 0x0000000000000000	X14: 0x0000000000000000	X15: 0x0000000000000000
X16: 0x0000000000000000	X17: 0x0000000000000000	X18: 0x0000000000000000	X19: 0x0000000000000000
X20: 0x0000000000000000	X21: 0x0000000000000000	X22: 0x0000000000000000	X23: 0x0000000000000000
X24: 0x0000000000000000	X25: 0x0000000000000000	X26: 0x0000000000000000	X27: 0x0000000000000000
X28: 0x0000000000000000	X29: 0x0000000000000000	X30: 0x0000000000000000
PC:  0x0000000000400004	SP:  0x0000007fedb9be40	PS:  0x0000000000000000
```

## Notes
Someone asked about xmm registers. If you pass `-x` it will dump out quite a bit of info.

```
GP Regs:
rax=0000000000000000 rbx=0000000000000000 rcx=0000000000000000
rdx=0000000000000000 rsi=0000000000000000 rdi=0000000000000000
rip=0000000000400001 rsp=00007ffca03d9370 rbp=0000000000000000
 r8=0000000000000000  r9=0000000000000000 r10=0000000000000000
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
[cf:0, zf:0, of:0, sf:0, pf:0, af:0, df:0]
cs=0033  ss=002b  ds=0000  es=0000  fs=0000  gs=0000            efl=00000202
FP Regs:
rip: 0000000000000000   rdp: 0000000000000000   mxcsr: 00001f80 mxcsr_mask:0000ffff
cwd: 037f       swd: 0000       ftw: 0000       fop: 0000
st_space:
0x00:   00000000        00000000        00000000        00000000
0x10:   00000000        00000000        00000000        00000000
0x20:   00000000        00000000        00000000        00000000
0x30:   00000000        00000000        00000000        00000000
0x40:   00000000        00000000        00000000        00000000
0x50:   00000000        00000000        00000000        00000000
0x60:   00000000        00000000        00000000        00000000
0x70:   00000000        00000000        00000000        00000000
xmm_space:
0x00:   00000000        00000000        00000000        00000000
0x10:   00000000        00000000        00000000        00000000
0x20:   00000000        00000000        00000000        00000000
0x30:   00000000        00000000        00000000        00000000
0x40:   00000000        00000000        00000000        00000000
0x50:   00000000        00000000        00000000        00000000
0x60:   00000000        00000000        00000000        00000000
0x70:   00000000        00000000        00000000        00000000
0x80:   00000000        00000000        00000000        00000000
0x90:   00000000        00000000        00000000        00000000
0xa0:   00000000        00000000        00000000        00000000
0xb0:   00000000        00000000        00000000        00000000
0xc0:   00000000        00000000        00000000        00000000
0xd0:   00000000        00000000        00000000        00000000
0xe0:   00000000        00000000        00000000        00000000
0xf0:   00000000        00000000        00000000        00000000
```

There are some other regsets the kernel exports via ptrace(), but they're dependent on kernel version, and didn't want to try to detect and adjust at runtime. If you want them, you should just need to add the storage in `proc_info_t`, edit `ptrace_collect_regs_<arch>()`, then add the display in the relevant `display` function.

Right now supported platforms are determined by what hardware I own. Adding a new architecture shouldn't be too difficult, as most of the code can be adapted from existing archs.

## Docs

You can get pretty much all the documentation with either `-h` from the command line, or `.help` from the interactive bit.
