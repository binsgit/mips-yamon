/************************************************************************
 *
 *  handler.S
 *
 *  Exception handler for FPU emulator
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2008 MIPS Technologies, Inc. All rights reserved.
 *
 *
 * Unpublished rights (if any) reserved under the copyright laws of the
 * United States of America and other countries.
 *
 * This code is proprietary to MIPS Technologies, Inc. ("MIPS
 * Technologies"). Any copying, reproducing, modifying or use of this code
 * (in whole or in part) that is not expressly permitted in writing by MIPS
 * Technologies or an authorized third party is strictly prohibited. At a
 * minimum, this code is protected under unfair competition and copyright
 * laws. Violations thereof may result in criminal penalties and fines.
 *
 * MIPS Technologies reserves the right to change this code to improve
 * function, design or otherwise. MIPS Technologies does not assume any
 * liability arising out of the application or use of this code, or of any
 * error or omission in such code. Any warranties, whether express,
 * statutory, implied or otherwise, including but not limited to the implied
 * warranties of merchantability or fitness for a particular purpose, are
 * excluded. Except as expressly provided in any written license agreement
 * from MIPS Technologies or an authorized third party, the furnishing of
 * this code does not give recipient any license to any intellectual
 * property rights, including any patent rights, that cover this code.
 *
 * This code shall not be exported or transferred for the purpose of
 * reexporting in violation of any U.S. or non-U.S. regulation, treaty,
 * Executive Order, law, statute, amendment or supplement thereto.
 *
 * This code constitutes one or more of the following: commercial computer
 * software, commercial computer software documentation or other commercial
 * items. If the user of this code, or any related documentation of any
 * kind, including related technical data or manuals, is an agency,
 * department, or other entity of the United States government
 * ("Government"), the use, duplication, reproduction, release,
 * modification, disclosure, or transfer of this code, or any related
 * documentation of any kind, is restricted in accordance with Federal
 * Acquisition Regulation 12.212 for civilian agencies and Defense Federal
 * Acquisition Regulation Supplement 227.7202 for military agencies. The use
 * of this code by the Government is further restricted in accordance with
 * the terms of the license agreement(s) and/or applicable contract terms
 * and conditions covering this code from MIPS Technologies or an authorized
 * third party.
 *
 * 
 * mips_end_of_legal_notice
 * 
 *
 ************************************************************************/

#include <ArchDefs.h>

#define RSIZE 8
#ifdef EL
#define SAVELWr(n, r)	sw r, (RSIZE*n)(k1);
#define RESTLWr(n, r)	lw r, (RSIZE*n)(k1);
#else
#define SAVELWr(n, r)	sw r, (RSIZE*n+4)(k1);
#define RESTLWr(n, r)	lw r, (RSIZE*n+4)(k1);
#endif
#define SAVEDr(n, r)	sd r, (RSIZE*n)(k1)
#define RESTDr(n, r)	ld r, (RSIZE*n)(k1)
#define SAVEWr(n, r)	sw r, (RSIZE*n)(k1)
#define RESTWr(n, r)	lw r, (RSIZE*n)(k1)
#define SAVELW(n)	SAVELWr(n, $##n)
#define RESTLW(n)	RESTLWr(n, $##n)
#define SAVED(n)	SAVEDr(n, $##n)
#define RESTD(n)	RESTDr(n, $##n)
#define SAVEFP(n)	sdc1 $##n, (RSIZE*n)(k1)
#define RESTFP(n)	ldc1 $##n, (RSIZE*n)(k1)



	.text
	.set noreorder
	.set noat


	.globl	FPUEMUL_handler
	.ent	FPUEMUL_handler
FPUEMUL_handler:	
	/* Any exception while emulation is active causes restore */
	la	k0, emul_active
	lw	k0, (k0)
	bnezl	k0, restore_all
	li	v0, 1			# argument to restore_all

	mfc0	k0, C0_Status
	sll	k0, 31-S_StatusCU1
	bltz	k0, hardhandler		# Branch if bit set
	mfc0	k0, C0_Cause

softhandler:	
	and	k0, M_CauseExcCode
	xor	k0, EXC_CPU
	beqz	k0, handler
	nop
	jr	k1
	nop

hardhandler:	
	and	k0, M_CauseExcCode
	xor	k0, EXC_FPE
	beqz	k0, handler
	nop
	jr	k1
	nop

handler:
	la	k0, k1_save
	sw	k1, (k0)

	la	k0, sys_64bit
	lb	k0, 0(k0)
	bnez	k0, store_gpr_64bit
	nop

	/* Store 32 bit CPU Registers */
	la	k1, pt_regs
	SAVELW(0)
	SAVELW(1)
	SAVELW(2)
	SAVELW(3)
	SAVELW(4)
	SAVELW(5)
	SAVELW(6)
	SAVELW(7)
	SAVELW(8)
	SAVELW(9)
	SAVELW(10)
	SAVELW(11)
	SAVELW(12)
	SAVELW(13)
	SAVELW(14)
	SAVELW(15)
	SAVELW(16)
	SAVELW(17)
	SAVELW(18)
	SAVELW(19)
	SAVELW(20)
	SAVELW(21)
	SAVELW(22)
	SAVELW(23)
	SAVELW(24)
	SAVELW(25)
	/* Not k0, k1 = $26, $27 */
	SAVELW(28)
	SAVELW(29)
	SAVELW(30)
	SAVELW(31)

	mflo	t0
	mfhi	t1
	mfc0	t2, C0_EPC
	mfc0	t3, C0_BadVAddr		#  readonly    -> not restored
	mfc0	t4, C0_Status		#  not changed -> not restored
	mfc0	t5, C0_Cause		#  BD readonly -> not restored

	/* if (t4.CU1 == 0) t4.FR = FPUEMUL_soft_fr */
	sll	t7, t4, 31-S_StatusCU1
	bltz	t7, 1f			# branch if bit CU1 set
	li	t7, M_StatusFR
	la	t6, FPUEMUL_soft_fr	# CU1 == 0, get soft_fr
	lb	t6, 0(t6)
	bnez	t6, 1f
	or	t4, t7			# set   t4.FR
	xor	t4, t7			# clear t4.FR
1:

	SAVELWr(32, t0)
	SAVELWr(33, t1)
	SAVELWr(34, t2)
	SAVELWr(35, t3)
	SAVELWr(36, t4)
	SAVELWr(37, t5)

	b	store_fpu_gpr
	nop

store_gpr_64bit :
	/* Store 64 bit CPU Registers */
	la	k1, pt_regs
#ifndef _GHS_
.set push
.set mips3
#endif
	SAVED(0)
	SAVED(1)
	SAVED(2)
	SAVED(3)
	SAVED(4)
	SAVED(5)
	SAVED(6)
	SAVED(7)
	SAVED(8)
	SAVED(9)
	SAVED(10)
	SAVED(11)
	SAVED(12)
	SAVED(13)
	SAVED(14)
	SAVED(15)
	SAVED(16)
	SAVED(17)
	SAVED(18)
	SAVED(19)
	SAVED(20)
	SAVED(21)
	SAVED(22)
	SAVED(23)
	SAVED(24)
	SAVED(25)
	/* Not k0, k1 = $26, $27 */
	SAVED(28)
	SAVED(29)
	SAVED(30)
	SAVED(31)
#ifndef _GHS_
.set pop
#endif

	mflo	t0
	mfhi	t1
	mfc0	t2, C0_EPC
	mfc0	t3, C0_BadVAddr		#  readonly    -> not restored
	mfc0	t4, C0_Status		#  not changed -> not restored
	mfc0	t5, C0_Cause		#  BD readonly -> not restored

	/* if (t4.CU1 == 0) t4.FR = FPUEMUL_soft_fr */
	sll	t7, t4, 31-S_StatusCU1
	bltz	t7, 1f			# branch if bit CU1 set
	li	t7, M_StatusFR
	la	t6, FPUEMUL_soft_fr	# CU1 == 0, get soft_fr
	lb	t6, 0(t6)
	bnez	t6, 1f
	or	t4, t7			# set   t4.FR
	xor	t4, t7			# clear t4.FR
1:

#ifndef _GHS_
.set push
.set mips3
#endif
	SAVEDr(32, t0)
	SAVEDr(33, t1)
	SAVEDr(34, t2)
	SAVEDr(35, t3)
	SAVEDr(36, t4)
	SAVEDr(37, t5)
#ifndef _GHS_
.set pop
#endif

store_fpu_gpr :
	la	k1, fpu_regs
	sll	t0, t4, 31-S_StatusCU1	# C0_Status
	bgez	t0, call_emulator	# Branch if bit clear
	sll	t0, t4, 31-S_StatusFR	# C0_Status
	bgez	t0, store_fpu_gpr_even	# Branch if bit clear
	cfc1	t1, C1_FCSR

	/* Store odd 64 bit FPU Registers */
	SAVEFP(1)
	SAVEFP(3)
	SAVEFP(5)
	SAVEFP(7)
	SAVEFP(9)
	SAVEFP(11)
	SAVEFP(13)
	SAVEFP(15)
	SAVEFP(17)
	SAVEFP(19)
	SAVEFP(21)
	SAVEFP(23)
	SAVEFP(25)
	SAVEFP(27)
	SAVEFP(29)
	SAVEFP(31)


store_fpu_gpr_even:
	/* Store even 64 bit FPU Registers */
	SAVEFP(0)
	SAVEFP(2)
	SAVEFP(4)
	SAVEFP(6)
	SAVEFP(8)
	SAVEFP(10)
	SAVEFP(12)
	SAVEFP(14)
	SAVEFP(16)
	SAVEFP(18)
	SAVEFP(20)
	SAVEFP(22)
	SAVEFP(24)
	SAVEFP(26)
	SAVEFP(28)
	SAVEFP(30)

	SAVEWr(32, t1)			# C1_FCSR

call_emulator:
	la	sp, kstack
	subu	sp, 16			# sp - (arg save area)

	la	v0, emul_active		# mark start of emulation
	sw	v0, (v0)

	and	a0, t5, M_CauseExcCode	# C0_Cause
	la	a1, pt_regs_arg
	la	a2, fpu_regs
	jal	fpu_emulator_cop1Handler
	srl	a0, S_CauseExcCode

restore_all:
	la	a0, retval
	sw	v0, (a0)

	la	a0, emul_active		# mark end of emulation
	sw	zero, (a0)

	la	k0, sys_64bit
	lb	k0, 0(k0)
	bnez	k0, restore_cp0_64bit
	nop

restore_cp0:
	la	k1, pt_regs
	RESTLWr(32, t0)
	RESTLWr(33, t1)
	RESTLWr(34, t2)
	RESTLWr(36, t4)
	mtlo	t0
	mthi	t1
	mtc0	t2, C0_EPC
	b	restore_fpu_gpr
	nop

restore_cp0_64bit:
	la	k1, pt_regs
#ifndef _GHS_
.set push
.set mips3
#endif
	RESTDr(32, t0)
	RESTDr(33, t1)
	RESTDr(34, t2)
	RESTDr(36, t4)
	mtlo	t0
	mthi	t1
	mtc0	t2, C0_EPC
#ifndef _GHS_
.set pop
#endif

restore_fpu_gpr :
	la	k1, fpu_regs
	sll	t0, t4, 31-S_StatusCU1	# C0_Status
	bgez	t0, restore_gpr		# Branch if bit clear
	RESTWr(32, t1)			# C1_FCSR

	/* Restore even 64 bit FPU Registers */
	RESTFP(0)
	RESTFP(2)
	RESTFP(4)
	RESTFP(6)
	RESTFP(8)
	RESTFP(10)
	RESTFP(12)
	RESTFP(14)
	RESTFP(16)
	RESTFP(18)
	RESTFP(20)
	RESTFP(22)
	RESTFP(24)
	RESTFP(26)
	RESTFP(28)
	RESTFP(30)

	sll	t4, 31-S_StatusFR
	bgez	t4, restore_gpr		# Branch if bit clear
	ctc1	t1, C1_FCSR

	/* Restore odd 64 bit FPU Registers */
	RESTFP(1)
	RESTFP(3)
	RESTFP(5)
	RESTFP(7)
	RESTFP(9)
	RESTFP(11)
	RESTFP(13)
	RESTFP(15)
	RESTFP(17)
	RESTFP(19)
	RESTFP(21)
	RESTFP(23)
	RESTFP(25)
	RESTFP(27)
	RESTFP(29)
	RESTFP(31)

restore_gpr:
	la	k0, sys_64bit
	lb	k0, 0(k0)
	bnez	k0, restore_gpr_64bit
	nop

	/* Restore 32 bit CPU Registers */
	la	k1, pt_regs
	RESTLW(0)
	RESTLW(1)
	RESTLW(2)
	RESTLW(3)
	RESTLW(4)
	RESTLW(5)
	RESTLW(6)
	RESTLW(7)
	RESTLW(8)
	RESTLW(9)
	RESTLW(10)
	RESTLW(11)
	RESTLW(12)
	RESTLW(13)
	RESTLW(14)
	RESTLW(15)
	RESTLW(16)
	RESTLW(17)
	RESTLW(18)
	RESTLW(19)
	RESTLW(20)
	RESTLW(21)
	RESTLW(22)
	RESTLW(23)
	RESTLW(24)
	RESTLW(25)
	/* Not k0, k1 = $26, $27 */
	RESTLW(28)
	RESTLW(29)
	RESTLW(30)
	RESTLW(31)

	b	do_ret
	nop

restore_gpr_64bit :
	/* Store 64 bit CPU Registers */
	la	k1, pt_regs
#ifndef _GHS_
.set push
.set mips3
#endif
	RESTD(0)
	RESTD(1)
	RESTD(2)
	RESTD(3)
	RESTD(4)
	RESTD(5)
	RESTD(6)
	RESTD(7)
	RESTD(8)
	RESTD(9)
	RESTD(10)
	RESTD(11)
	RESTD(12)
	RESTD(13)
	RESTD(14)
	RESTD(15)
	RESTD(16)
	RESTD(17)
	RESTD(18)
	RESTD(19)
	RESTD(20)
	RESTD(21)
	RESTD(22)
	RESTD(23)
	RESTD(24)
	RESTD(25)
	/* Not k0, k1 = $26, $27 */
	RESTD(28)
	RESTD(29)
	RESTD(30)
	RESTD(31)
#ifndef _GHS_
.set pop
#endif

do_ret:
	la	k0, retval
	lw	k0, (k0)
	beqz	k0, no_error
	nop

in_error:
	la	k1, k1_save
	lw	k1, (k1)
	jr	k1
	nop

no_error:
#ifndef _GHS_
.set push
.set mips3
#endif
	eret
#ifndef _GHS_
.set pop
#endif
	nop
	.end	FPUEMUL_handler


	.section .sbss
	.space	0x400
	.globl kstack
	.align 4
kstack:
pt_regs_arg:
	.space	6*RSIZE
pt_regs:
	.space	(32+2+4)*RSIZE
fpu_regs:
	.space	32*RSIZE
	.globl	FPUEMUL_csr
FPUEMUL_csr:
	.space	RSIZE
retval:
	.space	4
k1_save:
	.space	4
emul_active:
	.space	4
