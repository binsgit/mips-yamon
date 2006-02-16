/************************************************************************
 *
 *  memcpy.S
 *
 *  Memory copy
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2006 MIPS Technologies, Inc. All rights reserved.
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
#  define LWHI	lwl		/* high part is left in big-endian	*/
#  define SWHI	swl		/* high part is left in big-endian	*/
#  define LWLO	lwr		/* low part is right in big-endian	*/
#  define SWLO	swr		/* low part is right in big-endian	*/
#else
#  define LWHI	lwr		/* high part is right in little-endian	*/
#  define SWHI	swr		/* high part is right in little-endian	*/
#  define LWLO	lwl		/* low part is left in big-endian	*/
#  define SWLO	swl		/* low part is left in big-endian	*/
#endif


LEAF( memcpy )

	.set	noreorder
	.set	noat

	slti	AT,a2,8
	bne	AT,zero,last8
	move	v0,a0
	xor	t2,a1,a0
	andi	t2,t2,0x3		# a0/a1 displacement
	bne	t2,zero,shift
	subu	v1,zero,a1
	andi	v1,v1,0x3
	beq	v1,zero,chk8w
	subu	a2,a2,v1
	LWHI	t2,0(a1)
	addu	a1,a1,v1
	SWHI	t2,0(a0)
	addu	a0,a0,v1
chk8w:	andi	t2,a2,0x1f
	beq	a2,t2,chk1w
	subu	a3,a2,t2
	addu	a3,a3,a1
loop8w:	lw	t7,0(a1)
	lw	v1,4(a1)
	lw	t0,8(a1)
	lw	t1,12(a1)
	lw	t3,16(a1)
	lw	t4,20(a1)
	lw	t5,24(a1)
	lw	t6,28(a1)
	sw	t7,0(a0)
	sw	v1,4(a0)
	sw	t0,8(a0)
	sw	t1,12(a0)
	sw	t3,16(a0)
	addiu	a1,a1,32
	sw	t4,20(a0)
	sw	t5,24(a0)
	addiu	a0,a0,32
	bne	a1,a3,loop8w
	sw	t6,-4(a0)
	move	a2,t2
chk1w:	andi	t2,a2,0x3
	beq	a2,t2,last8
	subu	a3,a2,t2
	addu	a3,a3,a1
loop1w:	lw	t3,0(a1)
	addiu	a1,a1,4
	addiu	a0,a0,4
	bne	a1,a3,loop1w
	sw	t3,-4(a0)
	move	a2,t2
last8:	blez	a2,last8e
	addu	a3,a2,a1
last8l:	lb	t2,0(a1)
	addiu	a1,a1,1
	addiu	a0,a0,1
	bne	a1,a3,last8l
	sb	t2,-1(a0)
last8e:	j	ra
	nop
shift:	subu	a3,zero,a0
	andi	a3,a3,0x3
	beq	a3,zero,shift1
	subu	a2,a2,a3	# bytes left
	LWHI	t2,0(a1)
	LWLO	t2,3(a1)
	addu	a1,a1,a3
	SWHI	t2,0(a0)
	addu	a0,a0,a3
shift1:	andi	t2,a2,0x3
	subu	a3,a2,t2
	addu	a3,a3,a1
shifth:	LWHI	t3,0(a1)
	LWLO	t3,3(a1)
	addiu	a1,a1,4
	addiu	a0,a0,4
	bne	a1,a3,shifth
	sw	t3,-4(a0)
	b	last8
	move	a2,t2

	.set	at
	.set	reorder

END( memcpy )


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/
