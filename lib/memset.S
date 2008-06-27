
/************************************************************************
 *
 *  memset.S
 *
 *  Memory set
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

/*  This routine could be optimized for MIPS64. The current code only
 *  uses MIPS32 instructions.
 */	

#ifdef EB
#  define SWHI	swl		/* high part is left in big-endian	*/
#else
#  define SWHI	swr		/* high part is right in little-endian	*/
#endif

LEAF( memset )

	.set	noreorder
	.set	noat

	andi	a1, 0xff		# To avoid problems when extending
	slti	AT,a2,8
	bne	AT,zero,last8
	move	v0,a0
	beq	a1,zero,uneven
	subu	v1,zero,a0
	sll	t0,a1,8
	or	a1,a1,t0
	sll	t0,a1,16
	or	a1,a1,t0
uneven:	andi	v1,v1,0x3		# unaligned address?
	beq	v1,zero,chkw
	subu	a2,a2,v1
	SWHI	a1,0(a0)
	addu	a0,a0,v1
chkw:	andi	t0,a2,0x7
	beq	a2,t0,chkl
	subu	a3,a2,t0
	addu	a3,a3,a0
loopw:	addiu	a0,a0,8
	sw	a1,-8(a0)
	bne	a0,a3,loopw
	sw	a1,-4(a0)
	move	a2,t0
chkl:	andi	t0,a2,0x4
	beq	t0,zero,last8
	subu	a2,a2,t0
	sw	a1,0(a0)
	addiu	a0,a0,4
last8:	blez	a2,last8e
	addu	a3,a2,a0
last8l:	addiu	a0,a0,1
	bne	a0,a3,last8l
	sb	a1,-1(a0)
last8e:	j	ra
	nop

	.set	at
	.set	reorder

END( memset )


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

