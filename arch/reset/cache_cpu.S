
/************************************************************************
 *
 *  cache_cpu.S
 *
 *  Cache functions
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
#include <init.h>
		
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

	.set noreorder
	
/************************************************************************
 *
 *                          sys_init_cache
 *  Description :
 *  -------------
 *
 *  Invalidate I and D caches
 *
 *  input : k1 = processor ID
 *	
 *  Return values :
 *  ---------------
 *
 *  Always 0
 *
 ************************************************************************/
LEAF(sys_init_cache)

	/**** Determine cache settings ****/

	/* This code was linked cached, but is running uncached since
	 * caches have not been initialised yet.
	 */
	
#define RA			t4
#define l2cache_size		t1
#define l2cache_linesize	t0
#define icache_size		t3
#define icache_linesize		t2
#define dcache_size		t1
#define dcache_linesize		t0

	DISP_STR( msg_cache )
	
	move	RA, ra

	move	a0, k1

        /* L2 cache */
	bal     sys_determine_l2cache_avail_flash
	nop
	beq	v1, zero, 1f
	nop

	bal	sys_determine_l2cache_linesize_flash
	nop
	move	l2cache_linesize, v0

	bal	sys_determine_l2cache_lines_flash
	nop
	multu	l2cache_linesize, v0
	mflo	l2cache_size

	beq	l2cache_size, zero, 1f
	nop

	/* Initialise L2 */
	DISP_STR( msg_init_l2cache )
	move	a0, l2cache_size
	move	a1, l2cache_linesize
	move	a2, k1
	bal	sys_init_l2cache_flash
	nop
1:
	/* L1 cache */
	
	bal	sys_determine_icache_linesize_flash
	nop
	move	icache_linesize, v0

	bal	sys_determine_icache_lines_flash
	nop
	multu	icache_linesize, v0
	mflo	icache_size

	bal	sys_determine_dcache_linesize_flash
	nop
	move	dcache_linesize, v0

	bal	sys_determine_dcache_lines_flash
	nop
	multu	dcache_linesize, v0
	mflo	dcache_size

	beq	icache_size, zero, 1f
	nop
	beq	dcache_size, zero, 1f
	nop

	/* Initialise instruction cache */
	DISP_STR( msg_init_icache )
	move	a0, icache_size
	move	a1, icache_linesize
	bal	sys_init_icache
	nop
	
	/* Initialise data cache */
	DISP_STR( msg_init_dcache )
	move	a0, dcache_size
	move	a1, dcache_linesize
	bal	sys_init_dcache
	move	a2, k1

	/* Done */
	jr	RA
	move	v0, zero

	/* D-cache or I-cache has size 0,  set CPU uncached */
1:
	MFC0(   v0, C0_Config )
	and	v0, ~M_ConfigK0
	or	v0, K_CacheAttrU
	MTC0(   v0, C0_Config )

	/* Done */
	jr	RA
	move	v0, zero

END(sys_init_cache)


/************************************************************************
 *
 *                          sys_determine_l2cache_linesize_flash
 *
 *  Description :
 *  -------------
 *  Determine L2 CACHE linesize
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = L2 CACHE linesize in bytes
 *
 ************************************************************************/
LEAF(sys_determine_l2cache_linesize_flash)	

	move	v0, zero

	li	t9, QED_RM70XX
	beq	a0, t9, l2cache_linesize_qed_rm7061a
	nop
	li	t9, MIPS_25Kf
	beq	a0, t9, l2cache_linesize_25kf
	nop
	jr	ra
	nop

l2cache_linesize_qed_rm7061a:

	li	v0, QED_RM7061A_L2_CACHE_LSIZE
	jr	ra
	nop

l2cache_linesize_25kf:

	/* Read CONFIG2 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config2, R_C0_SelConfig2 )

	/* L2 cache line size */
	li	t8, M_Config2SL
	and	t9, t8
	beq	t9, zero, 1f
	nop

	li	t8, S_Config2SL
	srl	t9, t8
	li	t8, 0x2
	sll	v0, t8, t9
1:
	jr	ra
	nop

END(sys_determine_l2cache_linesize_flash)	


/************************************************************************
 *
 *                          sys_determine_l2cache_lines_flash
 *
 *  Description :
 *  -------------
 *  Determine number of L2 CACHE lines
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = number of L2 CACHE lines
 *
 ************************************************************************/
LEAF(sys_determine_l2cache_lines_flash)	

	li	t9, QED_RM70XX
	beq	a0, t9, l2cache_lines_qed_rm7061a
	nop
	li	t9, MIPS_25Kf
	beq	a0, t9, l2cache_lines_25kf
	nop
	jr	ra
	move	v0, zero

l2cache_lines_qed_rm7061a:

	li	v0, (QED_RM7061A_L2_CACHE_SIZE/QED_RM7061A_L2_CACHE_LSIZE)
	jr	ra
	nop

l2cache_lines_25kf:

	/* Read CONFIG2 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config2, R_C0_SelConfig2 )

	/* S-cache lines
	 * Calculated as associativity * sets per way
	 */
	li	t8, M_Config2SA
	and	t8, t9
	srl	t8, S_Config2SA
	addiu	t8, 1				/* t8 = associativity	*/

	li	t7, M_Config2SS
	and	t7, t9
	srl	t7, S_Config2SS
	li	t9, 0x40
	sll	t7, t9, t7			/* t7 = sets per way	*/

	multu	t8, t7
	mflo    v0

	jr	ra
	nop

END(sys_determine_l2cache_lines_flash)


/************************************************************************
 *
 *                          sys_determine_l2cache_assoc_flash
 *
 *  Description :
 *  -------------
 *  Determine L2 CACHE associativity
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = L2 CACHE associativity
 *
 ************************************************************************/
LEAF(sys_determine_l2cache_assoc_flash)	

	li	t9, QED_RM70XX
	beq	a0, t9, l2cache_assoc_qed_rm7061a
	nop
	li	t9, MIPS_25Kf
	beq	a0, t9, l2cache_assoc_25kf
	nop
	jr	ra
	move	v0, zero

l2cache_assoc_qed_rm7061a:

	li	v0, QED_RM7061A_L2_CACHE_ASSOC
	jr	ra
	nop

l2cache_assoc_25kf:

	/* Read CONFIG2 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config2, R_C0_SelConfig2 )

	/* L2-cache associativity */
	li	t8, M_Config2SA
	and	t9, t8
	srl	v0, t9, S_Config2SA
	jr	ra
	addiu	v0, 1

END(sys_determine_l2cache_assoc_flash)


/************************************************************************
 *
 *                          sys_determine_l2cache_avail_flash
 *
 *  Description :
 *  -------------
 *  Determine whether L2 CACHE is available and enabled
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = 1 if L2 CACHE is available, otherwise 0.
 *  v1 = 1 if L2 CACHE is enabled,   otherwise 0.
 *
 ************************************************************************/
LEAF(sys_determine_l2cache_avail_flash)	

	li	t9, QED_RM70XX
	beq	a0, t9, l2cache_avail_qed_rm7061a
	nop
	li	t9, MIPS_25Kf
	beq	a0, t9, l2cache_avail_25kf
	nop

	move	v0, zero
	jr	ra
	move	v1, zero

l2cache_avail_qed_rm7061a:

	li	v0, 1 /* L2 available        */
	li	v1, 0 /* Disabled by default */
	jr	ra
	nop

l2cache_avail_25kf:

	/* Read CONFIG2 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config2, R_C0_SelConfig2 )

	/* L2 cache line size */
	li	t8, M_Config2SL
	and	t9, t8
	beq	t9, zero, 1f
	move	v0, zero
	li	v0, 1
1:	
	jr	ra
	move	v1, v0 /* L2 can't be disabled */

END(sys_determine_l2cache_avail_flash)	


/************************************************************************
 *
 *                          sys_determine_icache_linesize_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine ICACHE linesize
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = ICACHE linesize in bytes
 *
 ************************************************************************/
LEAF(sys_determine_icache_linesize_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, icache_linesize_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, icache_linesize_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, icache_linesize_qed_rm7061a
	nop
	/* Unknown CPU */
icache_linesize_zero:
	jr	ra
	move	v0, zero

icache_linesize_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* I-cache line size */
	li	t8, M_Config1IL
	and	t8, config1
	beq	t8, zero, icache_linesize_zero
	li	t7, S_Config1IL
	srl	t8, t7
	li	t7, 0x2
	sll	v0, t7, t8

	jr	ra
	nop

icache_linesize_qed_rm5261:
	li	v0, QED_RM5261_ICACHE_LSIZE
	jr	ra
	nop

icache_linesize_qed_rm7061a:
	li	v0, QED_RM7061A_ICACHE_LSIZE
	jr	ra
	nop

END(sys_determine_icache_linesize_flash)	
	


/************************************************************************
 *
 *                          sys_determine_icache_lines_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine number of ICACHE lines
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = number of ICACHE lines
 *
 ************************************************************************/
LEAF(sys_determine_icache_lines_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, icache_lines_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, icache_lines_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, icache_lines_qed_rm7061a
	nop
	/* Unknown CPU */
icache_lines_zero:
	jr	ra
	move	v0, zero

icache_lines_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* I-cache lines
	 * Calculated as associativity * sets per way
	 */
	li	t8, M_Config1IA
	and	t8, config1
	li	t7, S_Config1IA
	srl	t8, t7
	addiu	t8,1				/* t8 = associativity	*/

	li	t7, M_Config1IS
	and	t7, config1
	li	t9, S_Config1IS
	srl	t7, t9
	li	t9, 0x40
	sll	t7, t9, t7			/* t7 = sets per way	*/

	multu	t8, t7
	mflo    v0

	jr	ra
	nop

icache_lines_qed_rm5261:
	li	v0, (QED_RM5261_ICACHE_SIZE / QED_RM5261_ICACHE_LSIZE)
	jr	ra
	nop

icache_lines_qed_rm7061a:
	li	v0, (QED_RM7061A_ICACHE_SIZE / QED_RM7061A_ICACHE_LSIZE)
	jr	ra
	nop

END(sys_determine_icache_lines_flash)


/************************************************************************
 *
 *                          sys_determine_icache_assoc_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine ICACHE associativity
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = ICACHE associativity
 *
 ************************************************************************/
LEAF(sys_determine_icache_assoc_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, icache_assoc_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, icache_assoc_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, icache_assoc_qed_rm7061a
	nop
	/* Unknown CPU */
icache_assoc_zero:
	jr	ra
	move	v0, zero

icache_assoc_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* I-cache associativity */
	li	t8, M_Config1IA
	and	t8, config1
	li	t7, S_Config1IA
	srl	t8, t7
	addiu	v0,t8,1				/* t8 = associativity	*/
	jr	ra
	nop

icache_assoc_qed_rm5261:
	li	v0, QED_RM5261_ICACHE_ASSOC
	jr	ra
	nop

icache_assoc_qed_rm7061a:
	li	v0, QED_RM7061A_ICACHE_ASSOC
	jr	ra
	nop

END(sys_determine_icache_assoc_flash)


/************************************************************************
 *
 *                          sys_determine_dcache_linesize_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine DCACHE linesize
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = DCACHE linesize in bytes
 *
 ************************************************************************/
LEAF(sys_determine_dcache_linesize_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, dcache_linesize_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, dcache_linesize_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, dcache_linesize_qed_rm7061a
	nop
	/* Unknown CPU */
dcache_linesize_zero:
	jr	ra
	move	v0, zero

dcache_linesize_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* D-cache line size */
	li	t8, M_Config1DL
	and	t8, config1
	beq	t8, zero, dcache_linesize_zero
	li	t7, S_Config1DL
	srl	t8, t7
	li	t7, 0x2
	sll	v0, t7, t8

	jr	ra
	nop

dcache_linesize_qed_rm5261:
	li	v0, QED_RM5261_DCACHE_LSIZE
	jr	ra
	nop

dcache_linesize_qed_rm7061a:
	li	v0, QED_RM7061A_DCACHE_LSIZE
	jr	ra
	nop

END(sys_determine_dcache_linesize_flash)	


/************************************************************************
 *
 *                          sys_determine_dcache_lines_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine number of DCACHE lines
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = number of DCACHE lines
 *
 ************************************************************************/
LEAF(sys_determine_dcache_lines_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, dcache_lines_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, dcache_lines_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, dcache_lines_qed_rm7061a
	nop
	/* Unknown CPU */
dcache_lines_zero:
	jr	ra
	move	v0, zero

dcache_lines_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* D-cache lines
	 * Calculated as associativity * sets per way
	 */
	li	t8, M_Config1DA
	and	t8, config1
	li	t7, S_Config1DA
	srl	t8, t7
	addiu	t8,1				/* t8 = associativity	*/

	li	t7, M_Config1DS
	and	t7, config1
	li	t9, S_Config1DS
	srl	t7, t9
	li	t9, 0x40
	sll	t7, t9, t7			/* t7 = sets per way	*/

	multu	t8, t7
	mflo    v0

	jr	ra
	nop

dcache_lines_qed_rm5261:
	li	v0, (QED_RM5261_DCACHE_SIZE / QED_RM5261_DCACHE_LSIZE)
	jr	ra
	nop

dcache_lines_qed_rm7061a:
	li	v0, (QED_RM7061A_DCACHE_SIZE / QED_RM7061A_DCACHE_LSIZE)
	jr	ra
	nop

END(sys_determine_dcache_lines_flash)


	
/************************************************************************
 *
 *                          sys_determine_dcache_assoc_flash
 *  Note :
 *  ------
 *  This routine is called also from syscon.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Determine DCACHE associativity
 *
 *  input : a0 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  v0 = DCACHE associativity
 *
 ************************************************************************/
LEAF(sys_determine_dcache_assoc_flash)	

	/* Check if it is a MIPS32/64 processor */	
	srl	t9, a0, S_PRIdCoID
	and     t9, M_PRIdCoID >> S_PRIdCoID
	li	t8, C0_PRID_COMP_NOT_MIPS32_64
	bne	t9, t8, dcache_assoc_mips32
	nop
	/* Not a MIPS32/64 processor */
	li	t9, QED_RM52XX
	beq	a0, t9, dcache_assoc_qed_rm5261
	nop
	li	t9, QED_RM70XX
	beq	a0, t9, dcache_assoc_qed_rm7061a
	nop
	/* Unknown CPU */
dcache_assoc_zero:
	jr	ra
	move	v0, zero

dcache_assoc_mips32:
	
	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )

#define config1	t9

	/* I-cache associativity */
	li	t8, M_Config1DA
	and	t8, config1
	li	t7, S_Config1DA
	srl	t8, t7
	addiu	v0,t8,1				/* t8 = associativity	*/
	jr	ra
	nop

dcache_assoc_qed_rm5261:
	li	v0, QED_RM5261_DCACHE_ASSOC
	jr	ra
	nop

dcache_assoc_qed_rm7061a:
	li	v0, QED_RM7061A_DCACHE_ASSOC
	jr	ra
	nop

END(sys_determine_dcache_assoc_flash)


/************************************************************************
 *
 *                          sys_init_l2cache_flash
 *
 *  Description :
 *  -------------
 *  Invalidate entire L2 CACHE
 *
 *  Inputs : a0 = cache size (bytes)
 *           a1 = line size  (bytes)
 *	     a2 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( sys_init_l2cache_flash )

	beq   a0, zero, 9f		 /* just in case cache size = 0 */
	 nop

	li	a3, MIPS_20Kc
	beq	a3, a2, 1f
	 nop
	li	 a3, MIPS_25Kf
	beq	a3, a2, 1f
	 nop
	li	 a3, MIPS_24K
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_24KE
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_34K
	beq	a3, a2, 2f
	 nop

	
	/* default: Clear TagLo/TagHi */
	MTC0(   zero, C0_TagLo )
	MTC0(   zero, C0_TagHi )
	b	0f
	 nop

1:
	/* 20Kc/25Kf : Clear L23TagLo/L23TagHi */
	MTC0_SEL_OPCODE( R_zero, R_C0_L23TagLo, R_C0_SelL23TagLo )
	MTC0_SEL_OPCODE( R_zero, R_C0_L23TagHi, R_C0_SelL23TagHi )
	b	0f
	 nop
	
2:
	/* 24K/24KE/34K : Clear L23TagLo */
	MTC0_SEL_OPCODE( R_zero, R_C0_L23TagLo, R_C0_SelL23TagLo )

0:
	/* Calc an address that will correspond to the first cache line */
	li	t0, KSEG0BASE
	addu	t1, t0, a0
	subu	t1, a1		/* t1 = last line */

	/* Loop through all lines, invalidating each of them */
1:	
SET_MIPS3()
	cache	SCACHE_INDEX_STORE_TAG, 0(t0)	/* clear tag */
SET_MIPS0()
	bne	t0, t1, 1b
	 addu	t0, a1
	
9:		
	jr	ra
	 nop
	
END( sys_init_l2cache_flash )

	
/************************************************************************
 *
 *                          sys_init_icache
 *  Note :
 *  ------
 *  This routine is called also from sys_cpu.c and syscon_cpu.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Invalidate entire ICACHE
 *
 *  Inputs : a0 = cache size (bytes)
 *           a1 = line size  (bytes)
 *	
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( sys_init_icache )
	
#ifdef NO_CACHE

	jr	ra
	nop

#else
		
	beq   a0, zero, 9f		 /* just in case cache size = 0 */
	 nop

	li	a3, MIPS_20Kc
	beq	a3, a2, 1f
	 nop
	li	 a3, MIPS_25Kf
	beq	a3, a2, 1f
	 nop
	li	 a3, MIPS_24K
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_24KE
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_34K
	beq	a3, a2, 2f
	 nop
	
1:
	/* 20Kc/25Kf : Clear ITagLo/ITagHi */
	/* Note: Use the default case because ITagLo==TagLo and ITagHI=TagHi */
	
	/* default: Clear TagLo/TagHi */
	MTC0( zero, C0_TagLo )
	MTC0( zero, C0_TagHi )
	b	0f
	 nop

2:
	/* 24K/24KE/34K : Clear ITagLo */
	MTC0_SEL_OPCODE( R_zero, R_C0_ITagLo, R_C0_SelITagLo )

0:
	/* Calc an address that will correspond to the first cache line */
	li	a2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	a3, a2, a0
	subu    a3, a1

	/* Loop through all lines, invalidating each of them */
1:	
SET_MIPS3()
	cache	ICACHE_INDEX_STORE_TAG, 0(a2)	/* clear tag */
SET_MIPS0()
	bne	a2, a3, 1b
	 addu	a2, a1

9:
	jr	ra
	nop

#endif
		
END( sys_init_icache )
		

/************************************************************************
 *
 *                          sys_init_dcache
 *  Note :
 *  ------
 *  This routine is called also from sys_cpu.c
 *  and must obey c calling conventions - and cannot use k0/k1
 *
 *  Description :
 *  -------------
 *  Invalidate entire DCACHE
 *
 *  Inputs : a0 = cache size (bytes)
 *           a1 = line size  (bytes)
 *           a2 = processor ID
 *	
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( sys_init_dcache )

#ifdef NO_CACHE

	jr	ra
	 nop

#else
		
	beq   a0, zero, 9f		 /* just in case cache size = 0 */
	 nop
	
	li	a3, MIPS_20Kc
	beq	a3, a2, 1f
  	 nop
	li	 a3, MIPS_25Kf
	beq	a3, a2, 1f
	 nop
	li	 a3, MIPS_24K
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_24KE
	beq	a3, a2, 2f
	 nop
	li	 a3, MIPS_34K
	beq	a3, a2, 2f
	 nop
	
	/* default: Clear TagLo/TagHi */
	MTC0( zero, C0_TagLo )
	MTC0( zero, C0_TagHi )
	b	0f
	 nop
	
1:	
	/* 20Kc/25Kf : Clear DTagLo and DTagHi */
	MTC0_SEL_OPCODE( R_zero, R_C0_DTagLo, R_C0_SelDTagLo )
	MTC0_SEL_OPCODE( R_zero, R_C0_DTagHi, R_C0_SelDTagHi )
	b	0f
	 nop	
2:	
	/* 24K/24KE/34K : Clear DTagLo */
	MTC0_SEL_OPCODE( R_zero, R_C0_DTagLo, R_C0_SelDTagLo )

0:	
	/* Calc an address that will correspond to the first cache line */
	li	a2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	a3, a2, a0
	subu    a3, a1

	/* Loop through all lines, invalidating each of them */
1:	
SET_MIPS3()
	cache	DCACHE_INDEX_STORE_TAG, 0(a2)	/* clear tag */
SET_MIPS0()
	bne	a2, a3, 1b
	 addu	a2, a1

9:
	jr	ra
	 nop

#endif
		
END( sys_init_dcache )
		
	

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

	
/* Messages */

	.text
	
MSG( msg_cache,        "CACHE"   )
MSG( msg_init_l2cache, "L2CACHE" )
MSG( msg_init_icache,  "ICACHE"  )
MSG( msg_init_dcache,  "DCACHE"  )		

