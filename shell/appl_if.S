
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
 * Copyright (c) 2004 MIPS Technologies, Inc. All rights reserved.
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
 * This code shall not be exported, reexported, transferred, or released,
 * directly or indirectly, in violation of the law of any country or
 * international law, regulation, treaty, Executive Order, statute,
 * amendments or supplements thereto. Should a conflict arise regarding the
 * export, reexport, transfer, or release of this code, the laws of the
 * United States of America shall be the governing law.
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

	/* Clean up CP0 STATUS
	 * Clear KSU bits so that we remain in kernel mode when we
	 * afterwards clear ERL and EXL.
	 * Clear IE bit in case user appl. has reenabled interrupts.
	 */
        MFC0(	t1, C0_Status )
	or	v0, t1, (M_StatusKSU | M_StatusIE)
	xor	v0, (M_StatusKSU | M_StatusIE)
	MTC0(	v0, C0_Status )

	/* Clear ERL and EXL */
	or	v0, (M_StatusERL | M_StatusEXL)
	xor	v0, (M_StatusERL | M_StatusEXL)
	MTC0(	v0, C0_Status )

        /* Running disabled - Store original CP0 STATUS */
	la	v0, save_cp0_status
	sw	t1, 0(v0)

	la	t1, appl_shell_func_code
	sw	t0, 0(t1)

	/**** Store all cp0 general registers (to keep bit63-32) ****/
	la	t1, appl_context
	la	t0, sys_64bit
	lb	t0, 0(t0)
	bne	t0, zero, 64f
	nop
	b	32f
	sw	ra, (GDB_FR_REG31 + OFS_END)(t1)
64:
SET_MIPS3()
        sd	ra, GDB_FR_REG31(t1)
SET_MIPS0()
32:
#define SAVE_K0 t0
#define SAVE_K1 t1
	move	SAVE_K0, k0
	move	SAVE_K1, k1
	la	k1, appl_context
	jal	EXCEP_store_gpr_but_k0k1ra
	nop

	/**** set up minimal YAMON context ****/
	la    sp, EXCEP_shell_sp
	lw    sp, 0(sp)

	/**** make the call ****/
	jal   appl_shell_func
	addiu sp, -4*4

	/**** the call returned, so restore application context ****/
	la	k0, appl_context	/* context to be restored */
	la	k1, back2user		/* k1 holds return address */
	j	EXCEP_restore_gpr_but_k1
	nop
back2user:
	move	k0, SAVE_K0
	move	k1, SAVE_K1

	/* Restore CP0 STATUS */
	la	t0, save_cp0_status
	lw	t0, 0(t0)
	la	t1, appl_shell_func_code
	lw	t1, 0(t1)
	xor	t1, SHELL_FUNC_REGISTER_CPU_ISR_CODE
	beq	t1, zero, set_imask
	xor	t1, SHELL_FUNC_REGISTER_CPU_ISR_CODE
	xor	t1, SHELL_FUNC_DEREGISTER_CPU_ISR_CODE
	bne	t1, zero, set_status
	nop

set_imask :
	/*  Modify IM field of saved STATUS register
	 *  to comply with YAMON return condition.
	 *  v0 was set to current CP0 STATUS value
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


