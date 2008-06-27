
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
#include <sys_api.h>
#include <mips.h>
#include <init.h>
#include <product.h>
#include <atlas.h>
#include <malta.h>
#include <sead.h>
#include <core_sys.h>
#include <socitsc.h>

/************************************************************************
 *  Definitions
 ************************************************************************/
/* Is the system controller SOCitSC? */
#define ISSC(reg)							\
	li	reg, HIKSEG1(MIPS_REVISION);				\
	lw	reg, LO_OFFS(MIPS_REVISION)(reg);			\
	srl	reg, MALTA_REVISION_SCON_SHF;				\
	andi	reg, MALTA_REVISION_SCON_MSK >> MALTA_REVISION_SCON_SHF

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
	beq	k1, k0, atlas_malta_early
	 li	k1, PRODUCT_MALTA_ID
	beq	k1, k0, atlas_malta_early
	 li	k1, PRODUCT_SEAD_ID
	beq	k1, k0, sead_early
	 li	k1, PRODUCT_SEAD2_ID
	beq	k1, k0, sead_early
	 nop

done_unknown:	/* Finished - Unknown platform */
	b	done
	 li	v0, ERROR_PLATFORM_UNKNOWN
	
done_ok:	/* Finished - Successful */
	b	done
	 move	v0, zero
					
done:		/* Finished - status in v0 */
	li	k0, HIKSEG1(MIPS_REVISION)		/* Reload k0 with the product ID */
	lw	k0, LO_OFFS(MIPS_REVISION)(k0)
	srl	k0, MIPS_REVISION_PROID_SHF
	j       sys_platform_early_done
	 andi	k0, MIPS_REVISION_PROID_MSK >> MIPS_REVISION_PROID_SHF

sead_early:
	/**** SEAD. We support only Basic RTL and MSC01 ****/

	li	k1, HIKSEG1(SEAD_REVISION)
	lw	k1, LO_OFFS(SEAD_REVISION)(k1)
	srl	k1, SEAD_REVISION_RTLID_SHF
	and	k1, SEAD_REVISION_RTLID_MSK >> SEAD_REVISION_RTLID_SHF
	xori	k1, SEAD_REVISION_RTLID_BASIC
	beqz	k1, sead_nmi_detect
	 xori	k1, SEAD_REVISION_RTLID_SOCIT101^SEAD_REVISION_RTLID_BASIC
	bnez	k1, done_unknown
	 nop

	/* MSC01: check register block position */
	li	k1, MSC01_REGADDR_RESET + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	k1, MSC01_BIU_RBBASL_OFS(k1)
	srl	k1, 16
	xor	k1, PHYS(MSC01_REGADDR_RESET) >> 16  /* 0x1f80  reset value */
	bnez	k1, sead_nmi_detect
	 nop

sead_init_msc01:
	jal	sys_init_sead2_msc01
	 nop

	b	done
	 move	v1, zero
	
sead_nmi_detect:
	/* Detect NMI (MIPS32/64) */
	MFC0(   k1, C0_Status )
	sll	k1, 31-S_StatusNMI
	bltz	k1, sead_nmi			/* Branch if bit set */
	 nop

	/* Detect NMI */
	li	k1, HIKSEG1(SEAD_NMISTATUS)
	lw	k1, LO_OFFS(SEAD_NMISTATUS)(k1)
	andi	k1, SEAD_NMISTATUS_FLAG_MSK
	beqz	k1, done_ok
	 move	v1, zero

sead_nmi:
	li	k1, HIKSEG1(SEAD_ASCIIPOS0)
	b	nmi
	 add	k1, LO_OFFS(SEAD_ASCIIPOS0)
	
atlas_malta_early:

	/**** Atlas/Malta ****/

	/* Determine system controller used on the core board */
	ISSC(k1)
	bnez	k1, atlas_malta_sc_socitsc
	 nop
	
	li	k1, HIKSEG1(MIPS_REVISION)
	lw	k1, LO_OFFS(MIPS_REVISION)(k1)
	srl	k1, MIPS_REVISION_CORID_SHF
	and	k1, MIPS_REVISION_CORID_MSK >> MIPS_REVISION_CORID_SHF
	
	xor	k1, MIPS_REVISION_CORID_CORE_SYS
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_SYS
	
	xor	k1, MIPS_REVISION_CORID_CORE_FPGA2
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA2
	
	xor	k1, MIPS_REVISION_CORID_CORE_EMUL
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_EMUL
	
	xor	k1, MIPS_REVISION_CORID_CORE_FPGA3
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA3
	
	xor	k1, MIPS_REVISION_CORID_CORE_24K
	beqz	k1, atlas_malta_nsc_socit
	 xor	k1, MIPS_REVISION_CORID_CORE_24K

	xor	k1, MIPS_REVISION_CORID_CORE_FPGA4
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA4

	xor	k1, MIPS_REVISION_CORID_CORE_FPGA5
	beqz	k1, atlas_malta_nsc_rocit
	 xor	k1, MIPS_REVISION_CORID_CORE_FPGA5

	b	atlas_malta_nsc_nmi_detect
	 nop
	
atlas_malta_nsc_rocit:
	/* If ROC-it System controller, check SC_CFG.CDONE to determine */
	/* whether initialised (at reset, NMISTATUS is not accessible) */
	
	li	k1, MSC01_BIU_REG_BASE
	lw	k1, MSC01_SC_ID_OFS(k1)
	srl	k1, MSC01_SC_ID_ID_SHF
	and	k1, MSC01_SC_ID_ID_MSK >> MSC01_SC_ID_ID_SHF
	xor	k1, MSC01_ID_SC_ROCIT
	bnez	k1, atlas_malta_nsc_socit		/* not ROC-it, try SOC-it */
	 nop

	li	k1, MSC01_BIU_REG_BASE
	lw	k1, MSC01_SC_CFG_OFS(k1)
	srl	k1, MSC01_SC_CFG_CDONE_SHF
	xor	k1, MSC01_SC_CFG_CDONE_BIT >> MSC01_SC_CFG_CDONE_SHF
	beqz	k1, atlas_malta_nsc_nmi_detect		/* initialised: go check for NMI */
	 nop
	b	atlas_malta_nsc_init
	 nop

atlas_malta_sc_socitsc:
	li	k1, KSEG1(SOCITSC_AHB2_BC_BASE)
	lw	k1, AHBMREGOFFS(1,CTRL)(k1)
	andi	k1, AHBM_CTRL_ENABLE_MSK
	beqz	k1, atlas_malta_sc_init
	 nop
	
atlas_malta_sc_nmi_detect:
	/* Detect NMI (MIPS32/64) */
	MFC0(   k1, C0_Status )
	sll	k1, (31-S_StatusNMI)
	bltz	k1, atlas_malta_nmi
	 nop

	/* Detect NMI (older cpus) */
	li	k1, HIKSEG1(MALTA_NMISTATUS)
	lw	k1, LO_OFFS(MALTA_NMISTATUS)(k1)
	andi	k1, MALTA_NMISTATUS_SB_MSK | MALTA_NMISTATUS_ONNMI_MSK
	bnez	k1, atlas_malta_nmi
	 nop
	
	b	atlas_malta_sc_init
	 nop

atlas_malta_nsc_socit:
	/* SOC-it 101 System controller: check register block position */
	/*                     (at reset, NMISTATUS is not accessible) */
	li	k1, MSC01_REGADDR_RESET + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	k1, MSC01_BIU_RBBASL_OFS(k1)
	srl	k1, 16
	xor	k1, PHYS(MSC01_REGADDR_RESET) >> 16  /* 0x1f80  reset value */
	beqz	k1, atlas_malta_nsc_init
	 nop

atlas_malta_nsc_nmi_detect:
	/* Detect NMI (MIPS32/64) */
	MFC0(   k1, C0_Status )
	sll	k1, (31-S_StatusNMI)
	bltz	k1, atlas_malta_nmi
	 nop

	/* Detect NMI (older cpus) */
	li	k1, HIKSEG1(MALTA_NMISTATUS)
	lw	k1, LO_OFFS(MALTA_NMISTATUS)(k1)
	andi	k1, MALTA_NMISTATUS_SB_MSK | MALTA_NMISTATUS_ONNMI_MSK
	bnez	k1, atlas_malta_nmi
	 nop

atlas_malta_sc_init:
atlas_malta_nsc_init:
	/*
	 * MIPSCMP
	 * At this point we know that there was no NMI
	 * Only the master CPU should carry on initialisation from here
	 */

	/*
	 * The first assumption is that only MIPSR2 processors are involved...
	 */
	MFC0(	a0, C0_Config)
	srl	a0, S_ConfigAR
	andi	a0, M_ConfigAR>>S_ConfigAR
	beqz	a0, 9f
	 nop

	/* Check the CPU number */
	MFC0_SEL_OPCODE( R_v1, R_C0_EBase, R_C0_SelEBase )
	srl	v1,S_EBaseCPUNum
	andi	v1,M_EBaseCPUNum>>S_EBaseCPUNum
	bnez	v1,done_ok		/* secondary CPUs don't do any initialisaion */
	 nop

	/* Init controller */
	/* k0 still holds Product ID */
9:	li	a0, MALTA_GT64120_BASE
	jal	sys_core_init
	 nop

	beqz	v1, done	/* No hard reset requested.. status in v0 */
	 nop

atlas_malta_hard_reset:
	/* Request software reset (common to Atlas and Malta) */
	li	t1, ATLAS_SOFTRES_RESET_GORESET
	li	t0, HIKSEG1(ATLAS_SOFTRES)
	sw	t1, LO_OFFS(ATLAS_SOFTRES)(t0)
1:
	b	1b
	 nop

atlas_malta_nmi:
	li	 k1, KSEG1(MALTA_ASCIIPOS0)

nmi:
	/*  NMI occurred.
	 *  Display message 'NMI'. Can't use DISP_STR since
	 *  only k0/k1 may be used.
	 *  k1 points at the LED
	 */
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
	 li    t7, PRODUCT_MALTA_ID
	beq   k0, t7, disp_string_atlas_malta
	 li    t7, PRODUCT_SEAD_ID
	beq   k0, t7, disp_string_sead
	 li    t7, PRODUCT_SEAD2_ID
	beq   k0, t7, disp_string_sead
	 nop
	/* Unknown platform, so ignore */
	b     disp_string_end
	 nop

disp_string_atlas_malta:
	/* Identical code for Atlas and Malta */
	li	t7,KSEG1(MALTA_ASCIIPOS0)
	b     disp_string_loop
	 addu	t6,t7,(MALTA_ASCIIPOS7-MALTA_ASCIIPOS0)

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
	li    t7,KSEG1(MALTA_ASCIIWORD)
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

