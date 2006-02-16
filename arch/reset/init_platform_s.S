
/************************************************************************
 *
 *  init_platform_s.S
 *
 *  Platform specific startup code (assembler)
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
#include <sys_api.h>
#include <mips.h>
#include <init.h>
#include <product.h>
#include <atlas.h>
#include <malta.h>
#include <sead.h>
#include <core_sys.h>

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
 *  sys_platform_early
 ************************************************************************/
LEAF(sys_platform_early)

	/*  Early platform specific initialisation.
	 *  Don't use registers other than k0/k1 until after
	 *  we have made sure an NMI has not occurred.
	 *  k0 must be set to the Product ID.
	 */

	/* Set k0 = Product ID (determined using REVISION register) */
	li	k0, HIKSEG1(MIPS_REVISION)
	lw	k0, LO_OFFS(MIPS_REVISION)(k0)
	srl	k0, MIPS_REVISION_PROID_SHF
	andi	k0, MIPS_REVISION_PROID_MSK >> MIPS_REVISION_PROID_SHF

	li	k1, PRODUCT_ATLASA_ID
	beq	k1, k0, early_atlas_malta
	li	k1, PRODUCT_MALTA_ID
	beq	k1, k0, early_atlas_malta
	li	k1, PRODUCT_SEAD_ID
	beq	k1, k0, early_sead
	li	k1, PRODUCT_SEAD2_ID
	bne	k1, k0, done_unknown

early_sead:
	/**** SEAD. We support only Basic RTL and MSC01 ****/

	li	k1, HIKSEG1(SEAD_REVISION)
	lw	k1, LO_OFFS(SEAD_REVISION)(k1)
	srl	k1, SEAD_REVISION_RTLID_SHF
	and	k1, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	sub	k1, SEAD_REVISION_RTLID_BASIC
	beq	k1, zero, sead_nmi_detect
	sub	k1, SEAD_REVISION_RTLID_SOCIT101 - SEAD_REVISION_RTLID_BASIC
	bne	k1, zero, done_unknown

	/* System controller: check register block position */
	li	k1, MSC01_REGADDR_RESET + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	k1, MSC01_BIU_RBBASL_OFS(k1)
	srl	k1, 16
	xor	k1, PHYS(MSC01_REGADDR_RESET) >> 16  /* 0x1f80  reset value */
	beq	k1, zero, 1f
	nop

	/* Detect NMI (MIPS32/64) */
	MFC0(   k1, C0_Status )
	sll	k1, 31-S_StatusNMI
	bltz	k1, nmi				/* Branch if bit set */
	li	k1, LO_OFFS(SEAD_ASCIIPOS0)
	j       sys_platform_early_done /* Return */
	move	v0, zero

1:
	jal	sys_init_sead2_msc01
	move	v0, zero
	j       sys_platform_early_done /* Return */
	nop

sead_nmi_detect:
	/* Detect NMI */
	li	k1, HIKSEG1(SEAD_NMISTATUS)
	lw	k1, LO_OFFS(SEAD_NMISTATUS)(k1)
	andi	k1, SEAD_NMISTATUS_FLAG_MSK
	bne	k1, zero, nmi
	li	k1, LO_OFFS(SEAD_ASCIIPOS0)

	j       sys_platform_early_done /* Return */
	move	v0, zero

done_unknown:
	/* Unknown platform */
	j       sys_platform_early_done /* Return */
	li	v0, ERROR_PLATFORM_UNKNOWN


early_atlas_malta:

	/**** Atlas/Malta ****/

	/* Determine system controller core board */
	/*   - NMISTATUS/DISPLAY not visible after reset */
	li	k1, HIKSEG1(MIPS_REVISION)
	lw	k1, LO_OFFS(MIPS_REVISION)(k1)
	srl	k1, MIPS_REVISION_CORID_SHF
	and	k1, MIPS_REVISION_CORID_MSK >> MIPS_REVISION_CORID_SHF
	xor	k1, MIPS_REVISION_CORID_CORE_SYS
	beq	k1,zero, 33f
	 xor	k1, MIPS_REVISION_CORID_CORE_SYS
	xor	k1, MIPS_REVISION_CORID_CORE_FPGA2
	beq	k1,zero, 33f
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA2
	xor	k1, MIPS_REVISION_CORID_CORE_EMUL
	beq	k1,zero, 33f
	 xor	k1, MIPS_REVISION_CORID_CORE_EMUL
	xor	k1, MIPS_REVISION_CORID_CORE_FPGA3
	bne	k1, zero, 1f
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA3
	
33:
	/* SOC-it 101 System controller: check register block position */
	/*                     (at reset, NMISTATUS is not accessible) */
	/* CoreEMUL with Bonito System Controller will fall through !  */
	li	k1, MSC01_REGADDR_RESET + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	k1, MSC01_BIU_RBBASL_OFS(k1)
	srl	k1, 16
	xor	k1, PHYS(MSC01_REGADDR_RESET) >> 16  /* 0x1f80  reset value */
	beq	k1, zero, early_core
	nop

1:
	/* Detect NMI (MIPS32/64) */
	MFC0(   k1, C0_Status )
	sll	k1, (31-S_StatusNMI)
	bltz	k1, nmi
	li      k1, LO_OFFS(ATLAS_ASCIIPOS0)

	/* Detect NMI (older cpus) */
	li	k1, HIKSEG1(MALTA_NMISTATUS)
	lw	k1, LO_OFFS(MALTA_NMISTATUS)(k1)
	andi	k1, MALTA_NMISTATUS_SB_MSK | MALTA_NMISTATUS_ONNMI_MSK
	bne	k1, zero, nmi
	li      k1, LO_OFFS(ATLAS_ASCIIPOS0)

early_core:
	/* Init controller */
	/* k0 still holds Product ID */
	li	a0, ATLAS_CORECTRL_BASE
	jal	sys_core_init
	nop
	bne	v0, zero, done_core	/* Error */
	nop
	bne	v1, zero, hard_reset_core  /* reset request */
	nop
done_core:
	j       sys_platform_early_done /* Return */
	nop

hard_reset_core:
	/* Request software reset (common to Atlas and Malta) */
	li	t1, ATLAS_SOFTRES_RESET_GORESET
	li	t0, HIKSEG1(ATLAS_SOFTRES)
	sw	t1, LO_OFFS(ATLAS_SOFTRES)(t0)
1:
	b	1b
	nop


nmi:
	/*  NMI occurred.
	 *  Display message 'NMI'. Can't use DISP_STR since
	 *  only k0/k1 may be used.
	 */
	li	k0, HIKSEG1(ATLAS_ASCIIPOS0)
	or	k1, k0
	li	k0, 'N'
	sw	k0, 0x00(k1)
	li	k0, 'M'
	sw	k0, 0x08(k1)
	li	k0, 'I'
	sw	k0, 0x10(k1)
	li	k0, ' '
	sw	k0, 0x18(k1)
	sw	k0, 0x20(k1)
	sw	k0, 0x28(k1)
	sw	k0, 0x30(k1)
	sw	k0, 0x38(k1)

	/*  Jump to NMI exception RAM handler
	 *  If the RAM handler has not been installed
	 *  yet (NMI very fast after RESET), the behaviour
	 *  is undefined.
	 */

	li      k0, 0x80000000 + SYS_NMI_RAM_VECTOR_OFS
	jr	k0
	nop



END(sys_platform_early)


/************************************************************************
 *  sys_init_platform
 ************************************************************************/
LEAF(sys_init_platform)

	move	v0, zero

	/* Atlas */
	li	t0, PRODUCT_ATLASA_ID
	bne	k0, t0, 2f
	nop
	DISP_STR( msg_atlas )
	j	sys_init_atlas_malta
	nop
2:
	/* Malta */
	li	t0, PRODUCT_MALTA_ID
	bne	k0, t0, 2f
	nop
	DISP_STR( msg_malta )
	j	sys_init_atlas_malta
	nop
2:
	/* SEAD */
	li	t0, PRODUCT_SEAD_ID
	bne	k0, t0, 2f
	nop
	DISP_STR( msg_sead )
	j	sys_init_sead
	nop
2:
	/* SEAD-2 */
	li	t0, PRODUCT_SEAD2_ID
	bne	k0, t0, 2f
	nop
	DISP_STR( msg_sead2 )
	li	t0, HIKSEG1(SEAD_REVISION)
	lw	t0, LO_OFFS(SEAD_REVISION)(t0)
	srl	t0, SEAD_REVISION_RTLID_SHF
	and	t0, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	sub	t0, SEAD_REVISION_RTLID_BASIC
	beq	t0, zero, 1f
	sub	t0, SEAD_REVISION_RTLID_SOCIT101 - SEAD_REVISION_RTLID_BASIC
	bne	t0, zero, 2f
	nop
	/* SEAD-2 with MSC01                                    */
	/* Share code with CoreSYS (below three code lines are  */
	/* "stolen" from atlas_malta_core.S and init_core_s.S)  */
	li      a0, SEAD_MIN_FREQ_MHZ
	j	access_msc01
        li      t9, FUNC_CONFIGURE_SDRAM
1:
	/* SEAD-2 with Basic RTL */
	j	sys_init_sead
	nop
2:
	/* Unknown platform */
	jr	ra
	li	v0, ERROR_PLATFORM_UNKNOWN

END(sys_init_platform)


/************************************************************************
 *  sys_disp_string
 ************************************************************************/
LEAF(sys_disp_string)

	li    t7, PRODUCT_ATLASA_ID
	beq   k0, t7, disp_string_atlas_malta
	nop
	li    t7, PRODUCT_MALTA_ID
	beq   k0, t7, disp_string_atlas_malta
	nop
	li    t7, PRODUCT_SEAD_ID
	beq   k0, t7, disp_string_sead
	nop
	li    t7, PRODUCT_SEAD2_ID
	beq   k0, t7, disp_string_sead
	nop
	/* Unknown platform, so ignore */
	b     disp_string_end
	nop

disp_string_atlas_malta:

	/* Identical code for Atlas and Malta */

	li    t7, KSEG1(ATLAS_ASCIIPOS0)
	li    t6, KSEG1(ATLAS_ASCIIPOS7)
	b     disp_string_loop
	nop

disp_string_sead:

	/* We support only Basic RTL and MSC01 */
	li    t7, HIKSEG1(SEAD_REVISION)
	lw    t7, LO_OFFS(SEAD_REVISION)(t7)
	srl   t7, SEAD_REVISION_RTLID_SHF
	and   t7, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	sub   t7, SEAD_REVISION_RTLID_BASIC
	beq   t7, zero, 1f
	sub   t7, SEAD_REVISION_RTLID_SOCIT101 - SEAD_REVISION_RTLID_BASIC
	bne   t7, zero, disp_string_end
	nop
1:
	li    t7, KSEG1(SEAD_ASCIIPOS0)
	li    t6, KSEG1(SEAD_ASCIIPOS7)

disp_string_loop:

	lb    t5, 0(t9)
	bne   t5, zero, 1f
	nop
	li    t5, ' '
	addiu t9, -1
1:
	addiu t9, 1
	sw    t5, 0(t7)
	bne   t7, t6, disp_string_loop
	addiu t7, 8

disp_string_end:

	jr    ra
	nop

END(sys_disp_string)


/************************************************************************
 *  sys_disp_val
 ************************************************************************/
LEAF(sys_disp_val)

	/* Set t6 = Product ID (determined using REVISION register) */
	li	t6, HIKSEG1(MIPS_REVISION)
	lw	t6, LO_OFFS(MIPS_REVISION)(t6)
	srl	t6, MIPS_REVISION_PROID_SHF
	andi	t6, MIPS_REVISION_PROID_MSK >> MIPS_REVISION_PROID_SHF

	li	t7, PRODUCT_ATLASA_ID
	beq	t6, t7, disp_val_atlas_malta
	li	t7, PRODUCT_MALTA_ID
	beq	t6, t7, disp_val_atlas_malta
	li	t7, PRODUCT_SEAD_ID
	beq	t6, t7, disp_val_sead
	li	t7, PRODUCT_SEAD2_ID
	beq	t6, t7, disp_val_sead
	nop
	/* Unknown platform, so ignore */
	b	disp_val_end
	nop

disp_val_atlas_malta:

	/* Identical code for Atlas and Malta */
	li    t7, KSEG1(ATLAS_ASCIIWORD)
	sw    a0, 0(t7)
	b     disp_val_end
	nop

disp_val_sead:

	/* We support only Basic RTL and MSC01 */
	li    t7, HIKSEG1(SEAD_REVISION)
	lw    t7, LO_OFFS(SEAD_REVISION)(t7)
	srl   t7, SEAD_REVISION_RTLID_SHF
	and   t7, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	sub   t7, SEAD_REVISION_RTLID_BASIC
	beq   t7, zero, 1f
	sub   t7, SEAD_REVISION_RTLID_SOCIT101 - SEAD_REVISION_RTLID_BASIC
	bne   t7, zero, disp_val_end
	nop
1:

	li    t7, KSEG1(SEAD_ASCIIPOS0)
	li    t6, 32

disp_val_loop:
	sub   t6, 4
	srl   t5, a0, t6
        and   t5, 0xf
        sub   t5, 9+1
	bltz  t5, 1f
	add   t5, '9'+1
	add   t5, 'A'-'9'-1
1:
	sw    t5, 0(t7)
	bne   t6, zero, disp_val_loop
	add   t7, 8

disp_val_end:
	jr    ra
	nop

END(sys_disp_val)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/* Messages */

	.text

MSG( msg_atlas, "ATLAS" )
MSG( msg_malta, "MALTA" )
MSG( msg_sead,  "SEAD" )
MSG( msg_sead2, "SEAD-2" )

