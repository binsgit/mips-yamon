
/************************************************************************
 *
 *  reset.S
 *
 *  Startup code for bootprom
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
#include <product.h>
#include <atlas.h>
#include <malta.h>
#include <sead.h>
#include <shell_api.h>
#include <sys_api.h>
			
/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          __reset_vector
 *  Description :
 *  -------------
 *
 *  Exception handler entry points for bootprom.
 *  including reset vector (0xbfc00000).
 *
 *  Return values :
 *  ---------------
 *
 *  For interrupts, function simply returns
 *  For exceptions, function never returns
 *
 *  In case of the reset exception (or NMI), function jumps to the
 *  entry point for code corresponding to the endianness of the CPU.
 *
 ************************************************************************/
LEAF(__reset_vector)

	.set noreorder

	
	/**** linked cached (start address 0x9fc00000)
	 **** Processor is running code uncached (i.e. 0xbfc00000)
	 ****/
	
	
	/*  Address 0xBfc00010 is reserved for boardID on boards
	 *  from MIPS Technologies, so branch to 0xBfc00018 (registers
	 *  on CBUS are 64 bit aligned).
	 */
	b	1f      /* 0xBfc00000 */
	nop		/* 0xBfc00004 */
	nop		/* 0xBfc00008 */
	nop		/* 0xBfc0000C */
	.word 0xffffffff/* 0xBfc00010 - illegal board if not intercepted */
	.word 0xffffffff/* 0xBfc00014 -               expanded to 64 bit */
1:					
	/* Clear watch registers */
	MTC0(	zero, C0_WatchLo)
	MTC0(	zero, C0_WatchHi)

	/* Disable interrupts and KSU field (power up issue) */
	MFC0(   k0, C0_Status)
	li	k1, ~(M_StatusIE | M_StatusKSU)
	and	k0, k1
	MTC0(	k0, C0_Status)

	/* Determine endianness */
	li	k1, KSEG1BASE
	la	k0, digit
	or	k0, k1
	lb	k0, 0(k0)
	xor	k0, 0x78
	beq	k0, zero, little_endian
	nop
	la	k0, _reset_handler_be   /* Defined in linker script */
	or	k0, k1			/* Make sure it is uncached */
	jr	k0
	nop

little_endian:	
	la	k0, _reset_handler_le   /* Defined in linker script */
	or	k0, k1			/* Make sure it is uncached */
	jr	k0
	nop

digit:	/* Used for determining endianness */
	.word 0x12345678


	/*
	 * Display "CacheErr" and stay in endless loop.
	 *   contains no endianess dependencies
	 */
cacheerr:
	/* Set k0 = Product ID (determined using REVISION register) */
	li	k0, HIKSEG1(MIPS_REVISION)
	lw	k0, LO_OFFS(MIPS_REVISION)(k0)
	srl	k0, MIPS_REVISION_PROID_SHF
	andi	k0, MIPS_REVISION_PROID_MSK >> MIPS_REVISION_PROID_SHF

	li      k1, LO_OFFS(ATLAS_ASCIIPOS0)
	xori	k0, PRODUCT_ATLASA_ID
	beq	k0, zero, disp_cacheerr
	xori	k0, PRODUCT_MALTA_ID ^ PRODUCT_ATLASA_ID
	beq	k0, zero, disp_cacheerr
	xori	k0, PRODUCT_SEAD_ID ^ PRODUCT_MALTA_ID

	li	k1, LO_OFFS(SEAD_ASCIIPOS0)
	beq	k0, zero, disp_val_sead
	xori	k0, PRODUCT_SEAD2_ID ^ PRODUCT_SEAD_ID
1:	bne	k0, zero, 1b	/* Unknown platform, so stay here */
	nop

disp_val_sead:

	/* We support only Basic RTL and MSC01 */
	li	k0, HIKSEG1(SEAD_REVISION)
	lw	k0, LO_OFFS(SEAD_REVISION)(k0)
	srl	k0, SEAD_REVISION_RTLID_SHF
	and	k0, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	xori	k0, SEAD_REVISION_RTLID_BASIC
	beq	k0, zero, disp_cacheerr
	xori	k0, SEAD_REVISION_RTLID_SOCIT101 ^ SEAD_REVISION_RTLID_BASIC
1:	bne	k0, zero, 1b	/* Unknown platform, so stay here */
	nop

disp_cacheerr:
	/*  CacheError occurred,  Display message 'CacheErr'.
	 *  only k0/k1 are used.
	 */
	li	k0, HIKSEG1(ATLAS_ASCIIPOS0)
	or	k1, k0
	li	k0, 'C'
	sw	k0, 0x00(k1)
	li	k0, 'a'
	sw	k0, 0x08(k1)
	li	k0, 'c'
	sw	k0, 0x10(k1)
	li	k0, 'h'
	sw	k0, 0x18(k1)
	li	k0, 'e'
	sw	k0, 0x20(k1)
	li	k0, 'E'
	sw	k0, 0x28(k1)
	li	k0, 'r'
	sw	k0, 0x30(k1)
	sw	k0, 0x38(k1)

1:	b	1b	/* Stay here */
	nop



.org 0x200
	/* 0xBFC00200 TLB refill, 32 bit task                    */
	/*********************************************************/
1:	b	1b	/* Stay here */
	nop
	

.org 0x280
	/* 0xBFC00280 XTLB refill, 64 bit task                   */
	/*********************************************************/
1:	b	1b	/* Stay here */
	nop
	

.org 0x300
	/* 0xBFC00300 Cache error exception                      */
	/*********************************************************/
	b	cacheerr /* display [CacheErr] and stay */
	nop
	

.org 0x380
	/* 0xBFC00380 General exception                          */
	/*********************************************************/
1:	b	1b	/* Stay here */
	nop


.org 0x400
	/* 0xBFC00400 Catch interrupt exceptions, some QEDs only */
	/*********************************************************/
1:	b	1b	/* Stay here */
	nop
	

.org 0x480
       /* 0xBFC00480 EJTAG debug exception -- don't touch ANY GPR */
       /******************************************************************
	* This exception cannot be moved from here, so below code
	* transfers it to RAM entry 0x80000300. The RAM entry will look
	* similar, which may give this sequence of machine instructions:
	*	bfc0049c:  jr    v0
	*	bfc004a0:  mfc0  v0, C0_DESAVE
	*	    (v0):  mtc0  xx, CO-DESAVE
	* Notes on hazard:
	* On CPUs without GPR interlocking, xx must be different from v0.
	* The mfc0 and mtc0 placed back to back should not be a problem,
	* as the read in CP0 takes place at an earlier stage than write,
	* and DESAVE is a scratchpad register with no operational effects.
	* Below: Two instructions between mtc0 and mfc0 is sufficient.
	*/
       mtc0     v0, C0_DESAVE
       li       v0, KSEG0(SYS_EJTAG_RAM_VECTOR_OFS)
       jr	v0
       mfc0     v0, C0_DESAVE


.org 0x500
       /* 0xBFC00500 Vector table for shell functions */

#define ILLEGAL 0xffffffff

       .word    ILLEGAL
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_PRINT_COUNT_CODE * 8
       .word	ILLEGAL
       .word	ILLEGAL
       .word	ILLEGAL
       .word	ILLEGAL
       .word	ILLEGAL
       .word	ILLEGAL
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_EXIT_CODE * 8
       .word	ILLEGAL
       .word	ILLEGAL
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_FLUSH_CODE * 8
       .word	ILLEGAL
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_PRINT_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_REGISTER_CPU_ISR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_DEREGISTER_CPU_ISR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_REGISTER_IC_ISR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_DEREGISTER_IC_ISR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_REGISTER_ESR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_DEREGISTER_ESR_CODE * 8
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_GETCHAR_CODE * 8 
       .word	SHELL_VECTOR_ADDR + SHELL_FUNC_SYSCON_READ_CODE * 8
	
END(__reset_vector)



/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

