
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
	sll	k1, k0, (31-S_StatusNMI)	/* Is this really from an NMI? */
	bltz	k1, 1f
	and	k1, M_StatusNMI >> S_StatusNMI
	li	k1, ~(M_StatusIE | M_StatusKSU)
	and	k0, k1
	MTC0(	k0, C0_Status)

	/* Determine endianness */
1:	li	k1, KSEG1BASE
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

setasciipos:
	/* Set k0 = Product ID (determined using REVISION register) */
	li	k0, HIKSEG1(MIPS_REVISION)
	lw	k0, LO_OFFS(MIPS_REVISION)(k0)
	srl	k0, MIPS_REVISION_PROID_SHF
	andi	k0, MIPS_REVISION_PROID_MSK >> MIPS_REVISION_PROID_SHF
	
	xori	k0, PRODUCT_ATLASA_ID
	beqz	k0, setasciipos_atlas
	 xori	k0, PRODUCT_MALTA_ID ^ PRODUCT_ATLASA_ID
	beqz	k0, setasciipos_malta
	 xori	k0, PRODUCT_SEAD_ID ^ PRODUCT_MALTA_ID
	beqz	k0, setasciipos_sead
	 xori	k0, PRODUCT_SEAD2_ID ^ PRODUCT_SEAD_ID
	beqz	k0, setasciipos_sead
	 nop

setasciipos_unknown:
	li	k0, KSEG1(0x1fc00000)
	j	k1
	 nop

setasciipos_atlas:
	li      k0, KSEG1(ATLAS_ASCIIPOS0)
	j	k1
	 nop

setasciipos_malta:
	li      k0, KSEG1(MALTA_ASCIIPOS0)
	j	k1
	 nop

setasciipos_sead:
	/* We support only Basic RTL and MSC01 on SEAD */
	li	k0, HIKSEG1(SEAD_REVISION)
	lw	k0, LO_OFFS(SEAD_REVISION)(k0)
	srl	k0, SEAD_REVISION_RTLID_SHF
	and	k0, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	xori	k0, SEAD_REVISION_RTLID_BASIC
	beqz	k0, 1f
	 xori	k0, SEAD_REVISION_RTLID_SOCIT101 ^ SEAD_REVISION_RTLID_BASIC
	bnez	k0, setasciipos_unknown
	 nop
1:	li	k1, KSEG1(SEAD_ASCIIPOS0)
	j	k1
	 nop
	
#define LLDISPLAY(c0,c1,c2,c3,c4,c5,c6,c7) \
	li      k0, KSEG1(MALTA_ASCIIWORD);	\
	sw	k0, 0(k0);			\
	lui	k0,0xbfc0; sw k0,(k0);		\
	la	k0,setasciipos;			\
	jalr	k1,k0;				\
	 nop;					\
	li	k1,c0; sw k1,0x00(k0);		\
	li	k1,c1; sw k1,0x08(k0);		\
	li	k1,c2; sw k1,0x10(k0);		\
	li	k1,c3; sw k1,0x18(k0);		\
	li	k1,c4; sw k1,0x20(k0);		\
	li	k1,c5; sw k1,0x28(k0);		\
	li	k1,c6; sw k1,0x30(k0);		\
	li	k1,c7; sw k1,0x38(k0);

.org 0x200
	/* 0xBFC00200 TLB refill, 32 bit task                    */
	/*********************************************************/
	LLDISPLAY('T','L','B','f','i','l','l',' ')
1:	b	1b	/* Stay here */
	nop
	

.org 0x280
	/* 0xBFC00280 XTLB refill, 64 bit task                   */
	/*********************************************************/
	LLDISPLAY('X','T','L','B','f','i','l','l')
1:	b	1b	/* Stay here */
	nop
	

.org 0x300
	/* 0xBFC00300 Cache error exception                      */
	/*********************************************************/
	LLDISPLAY('C','a','c','h','e','E','r','r')
1:	b	1b	/* Stay here */
	nop
	

.org 0x380
	/* 0xBFC00380 General exception                          */
	/*********************************************************/
	LLDISPLAY('G','e','n','E','x','c','e','p')
1:	b	1b	/* Stay here */
	nop


.org 0x400
	/* 0xBFC00400 Catch interrupt exceptions, some QEDs only */
	/*********************************************************/
	LLDISPLAY('I','n','t','E','x','c','e','p')
1:	b	1b	/* Stay here */
	nop
	

.org 0x480
	/* 0xBFC00480 EJTAG debug exception -- don't touch ANY GPR */
	/******************************************************************
	 * This exception cannot be moved from here, so below code
	 * transfers it to RAM entry SYS_EJTAG_RAM_VECTOR_OFS. The RAM entry will look
	 * similar, which may give this sequence of machine instructions:
	 *	bfc0049c:  jr    v0
	 *	bfc004a0:  mfc0  v0, C0_DESAVE
	 *	    (v0):  mtc0  xx, CO_DESAVE
	 * Notes on hazard:
	 * On CPUs without GPR interlocking, xx must be different from v0.
	 * The mfc0 and mtc0 placed back to back should not be a problem,
	 * as the read in CP0 takes place at an earlier stage than write,
	 * and DESAVE is a scratchpad register with no operational effects.
	 * Below: Two instructions between mtc0 and mfc0 is sufficient.
	 */
	.set	noat
	mtc0    v0, C0_DESAVE

	mfc0	v0, C0_PRId
	srl	v0, S_PRIdCoID
	and	v0, (M_PRIdCoID>>S_PRIdCoID)
	bnez	v0, ejtagmips3264
	 nop

	mfc0	v0, C0_PRId
	andi	v0, M_PRIdImp
	xori	v0, (QED_RM52XX & M_PRIdImp)
	beqz	v0, ejtag64
	 xori	v0, (QED_RM52XX & M_PRIdImp)
	xori	v0, (QED_RM70XX & M_PRIdImp)
	beqz	v0, ejtag64
	 nop

	/* fall through to ejtag32 */
	
ejtag32:
	li      v0, KSEG0(SYS_EJTAG_RAM_VECTOR_OFS)
	jr	v0
	 mfc0    v0, C0_DESAVE
	
ejtagmips3264:
	mfc0	v0, C0_Config
	srl	v0, S_ConfigAT
	and	v0, (M_ConfigAT >> S_ConfigAT)
	xori	v0, K_ConfigAT_MIPS32
	beqz	v0, ejtag32
	 nop
	
	/* fall through to ejtag64 */
	
ejtag64:
	li      v0, KSEG0(SYS_EJTAG_RAM_VECTOR_OFS)
	.set	mips64
	jr	v0
	 dmfc0    v0, C0_DESAVE
	.set	mips0
	
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

