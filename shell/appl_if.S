
/************************************************************************
 *
 *  appl_if.S
 *
 *  Assembler functions used for shell/user application interface
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


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <mips.h>
#include <shell_api.h>
#include <excep_api.h>
#include <sys_api.h>	
#include <gdb_stub.h>	
		
/************************************************************************
 *  Definitions
 ************************************************************************/

/* Offset to 32 bit word in 64 bit storage area */
#ifdef EB
#define OFS_END		4
#else
#define OFS_END		0
#endif

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

	BSS
	/* 32 bit align */
	ALIGN(2)
	
save_cp0_status:
	.space	4
	
/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *                          shell_api_vector_setup
 *  Description :
 *  -------------
 *  Install jump instructions to api handler
 *
 *  Parameters :
 *  ------------
 *  a0 :	vector_addr     - must be in KSEG0
 *  a1 :	shell_func_code - must be less than 0xffff
 *
 *  Return values :
 *  ---------------
 *  None
 ************************************************************************/

     .set noreorder
     .set noat
LEAF( shell_api_vector_setup )

	/* Copy ram vector code to shell api vector location */

#define ADDR		a0  /* also used as arg to sys_cache..() */
#define SRC		t2
#define RA		t3
	
	move	RA, ra

	la	SRC, api_vector_code	/* source */
	lw	t1, 0(SRC)
	sw	t1, 0(a0)
	lw	t1, 4(SRC)		/* li t0, 0x0000  */
	or	t1, a1			/* li t0, shell_func_code */
	sw	t1, 4(ADDR)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 *     sys_dcache_flush_addr modifies only t1.
	 *     sys_icache_invalidate_addr modifies only t1.
	 */
	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	move	a1, zero /* No need to flush pipeline */

	move	ra, RA
	jr	ra
	nop


api_vector_code:
	/*  Vector code to copy */
	j	appl2yamon
	li	t0, 0		/* when copied: "li t0, shell_func_code" */
END( shell_api_vector_setup )

	

LEAF( shell_return )
     /* Move return value to a0, so that 'return' acts like exit */
	move	a0, v0
	b	appl2yamon
	li   	t0, SHELL_FUNC_EXIT_CODE
END( shell_return )	

	
/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

SLEAF( appl2yamon )

	/* Preserves all regs except v0, t0-t1 */

	/*
	 * Disable interrupts in case user application has enabled them
	 */
        MFC0(	t1, C0_Status )
	or	v0, t1, M_StatusIE
	xor	v0, M_StatusIE
	MTC0(	v0, C0_Status )

	/* save the shell function code */
	la	v0, appl_shell_func_code
	sw	t0, 0(v0)
	
        /* save application status register */
	la	v0, appl_context
	sw	t1, GDB_REG32_OFS(GDB_FR_STATUS)(v0)
	
	/**** Store general registers ****/
	la	t1, sys_64bit
	lb	t1, 0(t1)
	bnez	t1, 64f
	 nop
	
	sw	$0,   GDB_REG32_OFS(GDB_FR_REG0 )(v0)
	sw	$1,   GDB_REG32_OFS(GDB_FR_REG1 )(v0)
	/* not v0 = $2 */
	sw	$3,   GDB_REG32_OFS(GDB_FR_REG3 )(v0)
	sw	$4,   GDB_REG32_OFS(GDB_FR_REG4 )(v0)
	sw	$5,   GDB_REG32_OFS(GDB_FR_REG5 )(v0)
	sw	$6,   GDB_REG32_OFS(GDB_FR_REG6 )(v0)
	sw	$7,   GDB_REG32_OFS(GDB_FR_REG7 )(v0)
	/* not t0,t1 = $8, $9 */
	sw	$10,  GDB_REG32_OFS(GDB_FR_REG10)(v0)
	sw	$11,  GDB_REG32_OFS(GDB_FR_REG11)(v0)
	sw	$12,  GDB_REG32_OFS(GDB_FR_REG12)(v0)
	sw	$13,  GDB_REG32_OFS(GDB_FR_REG13)(v0)
	sw	$14,  GDB_REG32_OFS(GDB_FR_REG14)(v0)
	sw	$15,  GDB_REG32_OFS(GDB_FR_REG15)(v0)
	sw	$16,  GDB_REG32_OFS(GDB_FR_REG16)(v0)
	sw	$17,  GDB_REG32_OFS(GDB_FR_REG17)(v0)
	sw	$18,  GDB_REG32_OFS(GDB_FR_REG18)(v0)
	sw	$19,  GDB_REG32_OFS(GDB_FR_REG19)(v0)
	sw	$20,  GDB_REG32_OFS(GDB_FR_REG20)(v0)
	sw	$21,  GDB_REG32_OFS(GDB_FR_REG21)(v0)
	sw	$22,  GDB_REG32_OFS(GDB_FR_REG22)(v0)
	sw	$23,  GDB_REG32_OFS(GDB_FR_REG23)(v0)
	sw	$24,  GDB_REG32_OFS(GDB_FR_REG24)(v0)
	sw	$25,  GDB_REG32_OFS(GDB_FR_REG25)(v0)
	/* Not k0, k1 = $26, $27 */
	sw	$28,  GDB_REG32_OFS(GDB_FR_REG28)(v0)
	sw	$29,  GDB_REG32_OFS(GDB_FR_REG29)(v0)
	sw	$30,  GDB_REG32_OFS(GDB_FR_REG30)(v0)
	sw	$31,  GDB_REG32_OFS(GDB_FR_REG31)(v0)

	b	1f	
	 nop
64:
	/* Store 64 bit CPU Registers */
SET_MIPS3()
        sd	$0,   GDB_REG64_OFS(GDB_FR_REG0 )(v0)
        sd	$1,   GDB_REG64_OFS(GDB_FR_REG1 )(v0)
        sd	$2,   GDB_REG64_OFS(GDB_FR_REG2 )(v0)
        sd	$3,   GDB_REG64_OFS(GDB_FR_REG3 )(v0)
        sd	$4,   GDB_REG64_OFS(GDB_FR_REG4 )(v0)
        sd	$5,   GDB_REG64_OFS(GDB_FR_REG5 )(v0)
        sd	$6,   GDB_REG64_OFS(GDB_FR_REG6 )(v0)
        sd	$7,   GDB_REG64_OFS(GDB_FR_REG7 )(v0)
	/* not t0,t1 = $8, $9 */
        sd	$10,  GDB_REG64_OFS(GDB_FR_REG10)(v0)
        sd	$11,  GDB_REG64_OFS(GDB_FR_REG11)(v0)
        sd	$12,  GDB_REG64_OFS(GDB_FR_REG12)(v0)
        sd	$13,  GDB_REG64_OFS(GDB_FR_REG13)(v0)
        sd	$14,  GDB_REG64_OFS(GDB_FR_REG14)(v0)
        sd	$15,  GDB_REG64_OFS(GDB_FR_REG15)(v0)
        sd	$16,  GDB_REG64_OFS(GDB_FR_REG16)(v0)
        sd	$17,  GDB_REG64_OFS(GDB_FR_REG17)(v0)
        sd	$18,  GDB_REG64_OFS(GDB_FR_REG18)(v0)
        sd	$19,  GDB_REG64_OFS(GDB_FR_REG19)(v0)
        sd	$20,  GDB_REG64_OFS(GDB_FR_REG20)(v0)
        sd	$21,  GDB_REG64_OFS(GDB_FR_REG21)(v0)
        sd	$22,  GDB_REG64_OFS(GDB_FR_REG22)(v0)
        sd	$23,  GDB_REG64_OFS(GDB_FR_REG23)(v0)
        sd	$24,  GDB_REG64_OFS(GDB_FR_REG24)(v0)
        sd	$25,  GDB_REG64_OFS(GDB_FR_REG25)(v0)
	/* Not k0, k1 = $26, $27 */
        sd	$28,  GDB_REG64_OFS(GDB_FR_REG28)(v0)
        sd	$29,  GDB_REG64_OFS(GDB_FR_REG29)(v0)
        sd	$30,  GDB_REG64_OFS(GDB_FR_REG30)(v0)
        sd	$31,  GDB_REG64_OFS(GDB_FR_REG31)(v0)
SET_MIPS0()

1:
	/**** set up minimal YAMON context ****/
	la    sp, EXCEP_shell_sp
	lw    sp, 0(sp)

	/**** make the call ****/
	jal   appl_shell_func
	 addiu sp, -4*4

	/**** the call returned, so restore application context ****/

	la	t0, appl_context
	la	t1, sys_64bit
	lb	t1, 0(t1)
	bnez	t1, 64f
	 nop
	
	lw	$0,   GDB_REG32_OFS(GDB_FR_REG0 )(t0)
	lw	$1,   GDB_REG32_OFS(GDB_FR_REG1 )(t0)
	lw	$2,   GDB_REG64_OFS(GDB_FR_REG2 )(t0)
	lw	$3,   GDB_REG32_OFS(GDB_FR_REG3 )(t0)
	lw	$4,   GDB_REG32_OFS(GDB_FR_REG4 )(t0)
	lw	$5,   GDB_REG32_OFS(GDB_FR_REG5 )(t0)
	lw	$6,   GDB_REG32_OFS(GDB_FR_REG6 )(t0)
	lw	$7,   GDB_REG32_OFS(GDB_FR_REG7 )(t0)
	/* not t0,t1 = $8, $9 */
	lw	$10,  GDB_REG32_OFS(GDB_FR_REG10)(t0)
	lw	$11,  GDB_REG32_OFS(GDB_FR_REG11)(t0)
	lw	$12,  GDB_REG32_OFS(GDB_FR_REG12)(t0)
	lw	$13,  GDB_REG32_OFS(GDB_FR_REG13)(t0)
	lw	$14,  GDB_REG32_OFS(GDB_FR_REG14)(t0)
	lw	$15,  GDB_REG32_OFS(GDB_FR_REG15)(t0)
	lw	$16,  GDB_REG32_OFS(GDB_FR_REG16)(t0)
	lw	$17,  GDB_REG32_OFS(GDB_FR_REG17)(t0)
	lw	$18,  GDB_REG32_OFS(GDB_FR_REG18)(t0)
	lw	$19,  GDB_REG32_OFS(GDB_FR_REG19)(t0)
	lw	$20,  GDB_REG32_OFS(GDB_FR_REG20)(t0)
	lw	$21,  GDB_REG32_OFS(GDB_FR_REG21)(t0)
	lw	$22,  GDB_REG32_OFS(GDB_FR_REG22)(t0)
	lw	$23,  GDB_REG32_OFS(GDB_FR_REG23)(t0)
	lw	$24,  GDB_REG32_OFS(GDB_FR_REG24)(t0)
	lw	$25,  GDB_REG32_OFS(GDB_FR_REG25)(t0)
	/* Not k0, k1 = $26, $27 */
	lw	$28,  GDB_REG32_OFS(GDB_FR_REG28)(t0)
	lw	$29,  GDB_REG32_OFS(GDB_FR_REG29)(t0)
	lw	$30,  GDB_REG32_OFS(GDB_FR_REG30)(t0)
	lw	$31,  GDB_REG32_OFS(GDB_FR_REG31)(t0)

	b	1f
	 nop

64:
SET_MIPS3()
	ld	$0,   GDB_REG64_OFS(GDB_FR_REG0 )(t0)
	ld	$1,   GDB_REG64_OFS(GDB_FR_REG1 )(t0)
	ld	$2,   GDB_REG64_OFS(GDB_FR_REG2 )(t0)
	ld	$3,   GDB_REG64_OFS(GDB_FR_REG3 )(t0)
	ld	$4,   GDB_REG64_OFS(GDB_FR_REG4 )(t0)
	ld	$5,   GDB_REG64_OFS(GDB_FR_REG5 )(t0)
	ld	$6,   GDB_REG64_OFS(GDB_FR_REG6 )(t0)
	ld	$7,   GDB_REG64_OFS(GDB_FR_REG7 )(t0)
	/* not t0,t1 = $8, $9 */
	ld	$10,  GDB_REG64_OFS(GDB_FR_REG10)(t0)
	ld	$11,  GDB_REG64_OFS(GDB_FR_REG11)(t0)
	ld	$12,  GDB_REG64_OFS(GDB_FR_REG12)(t0)
	ld	$13,  GDB_REG64_OFS(GDB_FR_REG13)(t0)
	ld	$14,  GDB_REG64_OFS(GDB_FR_REG14)(t0)
	ld	$15,  GDB_REG64_OFS(GDB_FR_REG15)(t0)
	ld	$16,  GDB_REG64_OFS(GDB_FR_REG16)(t0)
	ld	$17,  GDB_REG64_OFS(GDB_FR_REG17)(t0)
	ld	$18,  GDB_REG64_OFS(GDB_FR_REG18)(t0)
	ld	$19,  GDB_REG64_OFS(GDB_FR_REG19)(t0)
	ld	$20,  GDB_REG64_OFS(GDB_FR_REG20)(t0)
	ld	$21,  GDB_REG64_OFS(GDB_FR_REG21)(t0)
	ld	$22,  GDB_REG64_OFS(GDB_FR_REG22)(t0)
	ld	$23,  GDB_REG64_OFS(GDB_FR_REG23)(t0)
	ld	$24,  GDB_REG64_OFS(GDB_FR_REG24)(t0)
	ld	$25,  GDB_REG64_OFS(GDB_FR_REG25)(t0)
	/* Not k0, k1 = $26, $27 */
	ld	$28,  GDB_REG64_OFS(GDB_FR_REG28)(t0)
	ld	$29,  GDB_REG64_OFS(GDB_FR_REG29)(t0)
	ld	$30,  GDB_REG64_OFS(GDB_FR_REG30)(t0)
	ld	$31,  GDB_REG64_OFS(GDB_FR_REG31)(t0)
SET_MIPS0()

1:
	/* Get the application status register */
	lw	t0, GDB_REG32_OFS(GDB_FR_STATUS)(t0)

	/* Some functions may have changed the interrupt mask */	
	la	t1, appl_shell_func_code
	lw	t1, 0(t1)
	xor	t1, SHELL_FUNC_REGISTER_CPU_ISR_CODE
	beqz	t1, set_imask
	 xor	t1, SHELL_FUNC_REGISTER_CPU_ISR_CODE^SHELL_FUNC_DEREGISTER_CPU_ISR_CODE
	bnez	t1, set_status
	 nop

set_imask :
	/*  Modify IM field of saved STATUS register
	 *  to comply with YAMON return condition.
	 *  t0 was set to current CP0 STATUS value
	 */
	/* Update CP0 STATUS */
	or	t0, M_StatusIM
	xor	t0, M_StatusIM
	and	v0, M_StatusIM
	or	t0, v0

set_status :
	MTC0(	t0, C0_Status )

	/**** Return to caller ****/
	jr	ra
	 nop

END( appl2yamon )


