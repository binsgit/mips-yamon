
/************************************************************************
 *
 *  init_cpu_s.S
 *
 *  CPU specific startup code
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
 *  sys_init_processor
 ************************************************************************/
LEAF(sys_init_processor)

	/* Check if it is a MIPS 20Kc/25Kf processor */
	li	t0, MIPS_20Kc
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_25Kf
	bne	k1, t0, 2f
	nop
1:		
	/* Setup MIPS 20Kc/25Kf specifics (implementation dependent fields) */
	MFC0(   t0, C0_Config )
	li	t1, CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS20KC_25KF
	or	t0, t1
	MTC0(	t0, C0_Config )

	MFC0(   t0, C0_Status )
	li	t1, STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS20KC_25KF
	or	t0, t1
	MTC0(	t0, C0_Status )
2:
	/* Check if it is a MIPS 24K processor */	
	li	t0, MIPS_24K
	bne	k1, t0, 2f
	nop

	/* Setup MIPS24K specifics (implementation dependent fields) */
	MFC0(   t0, C0_Config )
	li	t1, CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS24K
	or	t0, t1
	MTC0(	t0, C0_Config )

	MFC0(   t0, C0_Status )
	li	t1, STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS24K
	or	t0, t1
	MTC0(	t0, C0_Status )
2:
	/* Check if it is a MIPS 34K processor */	
	li	t0, MIPS_34K
	bne	k1, t0, 2f
	nop

	/* Setup MIPS34K specifics (implementation dependent fields) */
	MFC0(   t0, C0_Config )
	li	t1, CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS34K
	or	t0, t1
	MTC0(	t0, C0_Config )

	MFC0(   t0, C0_Status )
	li	t1, STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS34K
	or	t0, t1
	MTC0(	t0, C0_Status )
2:
	/* Check if it is a MIPS 74K processor */	
	li	t0, MIPS_74K
	bne	k1, t0, 2f
	nop

	/* Setup MIPS74K specifics (implementation dependent fields) */
	MFC0(   t0, C0_Config )
	li	t1, CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS74K
	or	t0, t1
	MTC0(	t0, C0_Config )

	MFC0(   t0, C0_Status )
	li	t1, STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS74K
	or	t0, t1
	MTC0(	t0, C0_Status )
2:
	/* Check if it is a MIPS 4K/5K family processor */
	li	t0, MIPS_4Kc
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4Kmp
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KEc
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KEc_R2
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KEmp
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KEmp_R2
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KSc
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_4KSd
	beq	k1, t0, 1f
	nop
	li	t0, MIPS_5K
	beq	k1, t0, 1f
	nop	
	li	t0, MIPS_5KE
	beq	k1, t0, 1f
	nop	
	b	2f
	nop
1:
	/* Setup MIPS 4K/5K specifics (implementation dependent fields) */
	MFC0(   t0, C0_Config )
	li	t1, CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS4K5K
	or	t0, t1
	MTC0(	t0, C0_Config )

	MFC0(   t0, C0_Status )
	li	t1, STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS4K5K
	or	t0, t1
	MTC0(	t0, C0_Status )
2:	
	/* Check if it is a MIPS32/64 processor */	
	li      t0, M_PRIdCoID
	and	t0, k1
	srl	t0, S_PRIdCoID
	li	t1, C0_PRID_COMP_NOT_MIPS32_64
	bne	t0, t1, mips32_64
	nop
	/* Not a MIPS32/64 processor */
	li	t0, QED_RM52XX
	beq	k1, t0, qed_rm5261
	nop
	li	t0, QED_RM70XX
	beq	k1, t0, qed_rm7061a
	nop
	/* Unknown CPU */
	jr	ra
	li	v0, ERROR_PROCESSOR_UNKNOWN	


/************************************************************************
 *  MIPS32/MIPS64 specific cpu initialisation
 ************************************************************************/
mips32_64:	
	
	/* Setup generic MIPS32/MIPS64 fields of STATUS register */
	MFC0(   t0, C0_Status )
	li	t1, ~STATUS_MIPS32_64_MSK
	and	t0, t1
	li	t1, STATUS_MIPS32_64
	or	t0, t1
	MTC0(	t0, C0_Status )

	/* Setup generic MIPS32 fields of CONFIG0 register */
	MFC0(   t0, C0_Config )
	li	t1, ~CONFIG0_MIPS32_64_MSK
	and	t0, t1
	li	t1, CONFIG0_MIPS32_64
	or	t0, t1
	MTC0(	t0, C0_Config )

	jr	ra
	move	v0, zero
	

/************************************************************************
 *  QED RM5261 specific cpu initialisation.
 ************************************************************************/
qed_rm5261:		

	/* Setup STATUS and CONFIG registers */
	li	t0, STATUS_QED
	MTC0(	t0, C0_Status )
	li	t0, CONFIG_QED
	MTC0(	t0, C0_Config )

	jr   ra
	move v0, zero


/************************************************************************
 *  QED RM7061A specific cpu initialisation.
 ************************************************************************/
qed_rm7061a:		

	/* Setup STATUS and CONFIG registers */

	/*  Set DE bit so that cache parity checks are disabled.
	 *  Seems to be necessary for RM7061A, probably since
	 *  the YAMON cache initialisation functions only clear
	 *  the cache tag, but don't initialise the data parity
	 *  fields.
	 */
	li	t0, STATUS_QED | C0_STATUS_QED_DE_BIT
	MTC0(	t0, C0_Status )
	li	t0, CONFIG_QED
	MTC0(	t0, C0_Config )

	jr   ra
	move v0, zero

END(sys_init_processor)
