/* Delay slot emulation code */
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2002 MIPS Technologies, Inc.
 */

typedef void *vaddr_t;

#define REG_TO_VA (vaddr_t)(long)
#define VA_TO_REG (gpreg_t)(long)

int mips_dsemul(struct pt_regs *regs, mips_instruction ir, gpreg_t cpc);
int do_dsemulret(struct pt_regs *xcp);

/* Instruction which will always cause an address error */
#define AdELOAD 0x8c000001	/* lw $0,1($0) */
/* Instruction which will plainly cause a CP1 exception when FPU is disabled */
#define CP1UNDEF 0x44400001    /* cfc1 $0,$0 undef  */

/* Instruction inserted following the badinst to further tag the sequence */
#define BD_COOKIE 0x0000bd36 /* tne $0,$0 with baggage */

/* Setup which instruction to use for trampoline */
#ifdef STANDALONE_EMULATOR
#define BADINST CP1UNDEF
#else
#define BADINST AdELOAD
#endif

#define EX_FPE 15

#define force_sig(x,y)

extern void flush_cache_sigtramp(long addr);

/* Enable emulation of MIPS64 instructions, even when compiled on mips32 */
#define __mips64 1

