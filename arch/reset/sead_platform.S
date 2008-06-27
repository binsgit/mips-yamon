
/************************************************************************
 *
 *  sead_platform.S
 *
 *  SEAD specific initialisation
 *
 *  TBD : Move definitions to sead.h
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
#include <init.h>
#include <sead.h>
#include <spd.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Assumptions (TBD) */
#define SEAD_MEMORY_SIZE		(32*1024*1024)
#define SEAD_MEMORY_BASE		SEAD_SYSTEMRAM_BASE

/* Worst case setup based on frequency assumptions */
#define GCLK_SCALE_WORST_CASE		(SEAD_MAX_FREQ_MHZ * 10)
#define TREFRESH_WORST_CASE		(SEAD_MIN_FREQ_MHZ * 1000000 / 64020)


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

LEAF(sys_init_sead2_msc01)

	/* Init controller */
	/* k0 still holds Product ID */
#define BIU_REGS			t2
	/* Move register block base address to 0x1bc00000 */
	/* Decrease size of IP3 space to avoid clash with reg-space */
	/* IP3 temporary decrease         - reset 0x18000000 */
	li	BIU_REGS, MSC01_REGADDR_RESET + \
		          MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	li	t0, -SEAD_MSC01_IP3MEM_SIZE		     /*-0x18400000 */
	sw	t0, MSC01_BIU_IP3MSK1L_OFS(BIU_REGS)
	/* Register block move            - reset 0x1f800000 */
	li	t0, SEAD_MSC01_REG_BASE      		     /* 0x1bc00000 */
	sw	t0, MSC01_BIU_RBBASL_OFS(BIU_REGS)           /*-0x1c000000 */
	sync
#undef BIU_REGS

#define PBC_REGS			t2
	/* Speed up peripheral bus controller with configuration: */
	/*       CDIV=1, WS=5,  WSIDLE=2, ADM=0,                  */
	/*       CAT=1,  CDT=1, RAT=2,    RDT=0, WAT=2, WDT=0     */
	li	PBC_REGS, MSC01_PBC_REG_BASE
	li	t0, (   1<<MSC01_PBC_CLKCFG_SHF)
	sw	t0, MSC01_PBC_CLKCFG_OFS(PBC_REGS)
	li	t0, (   1<<MSC01_PBC_CS0TIM_CDT_SHF) | \
		    (   1<<MSC01_PBC_CS0TIM_CAT_SHF)
	sw	t0, MSC01_PBC_CS0TIM_OFS(PBC_REGS)
	sw	t0, MSC01_PBC_CS1TIM_OFS(PBC_REGS)
	li	t0, (   0<<MSC01_PBC_CS0RW_RDT_SHF) | \
		    (   2<<MSC01_PBC_CS0RW_RAT_SHF) | \
		    (   0<<MSC01_PBC_CS0RW_WDT_SHF) | \
		    (   2<<MSC01_PBC_CS0RW_WAT_SHF)
	sw	t0, MSC01_PBC_CS0RW_OFS(PBC_REGS)
	sw	t0, MSC01_PBC_CS1RW_OFS(PBC_REGS)
	lw	t0, MSC01_PBC_CS0CFG_OFS(PBC_REGS) /* preserve DTYP */
	li	t1, MSC01_PBC_CS0CFG_DTYP_MSK
	and	t0, t1
	ori	t0, (   0<<MSC01_PBC_CS0CFG_ADM_SHF) | \
		    (   2<<MSC01_PBC_CS0CFG_WSIDLE_SHF) | \
		    (   5<<MSC01_PBC_CS0CFG_WS_SHF)
	sw	t0, MSC01_PBC_CS0CFG_OFS(PBC_REGS) /* WS=16, WSIDLE=3, ADM=0 */
	/* Set CS1 = CS0 (temporarily, CS1 is used during CS0 narrow down) */
	sw	t0, MSC01_PBC_CS1CFG_OFS(PBC_REGS) /* WS=16, WSIDLE=3, ADM=0 */

	/* Chip Select 0 (high prio) -> 8 bit monitor flash 0x1fc00000 */
	/* Chip Select 1 (low prio) -> 32 bit flash, sram.. 0x1c000000 */
	/* Narrow down chip CS0 - Caution: mask before base */
	li	t1, -0x00400000
	sw	t1, MSC01_PBC_CS0MSKL_OFS(PBC_REGS)
	li	t1, 0x1fc00000
	sw	t1, MSC01_PBC_CS0BASL_OFS(PBC_REGS)
	
	/* Set CS1 to 32 bit width */
	li	t1, ~MSC01_PBC_CS0CFG_DTYP_MSK
	and	t0, t1
	li	t1, MSC01_PBC_CS0CFG_DTYP_32BIT << MSC01_PBC_CS3CFG_DTYP_SHF
	or	t0, t1
	sw	t0, MSC01_PBC_CS1CFG_OFS(PBC_REGS) /* WS=16, WSIDLE=3, ADM=0 */
#undef PBC_REGS

#define BIU_REGS			t2
	/* Setup msc01 basic address decode */
	li	BIU_REGS, MSC01_BIU_REG_BASE
	li	t0, SEAD_MSC01_MEMORY_BASE		     /* 0x00000000 */
	li	t1, -SEAD_MSC01_MEMORY_SIZE		     /*-0x10000000 */
	sw	t0, MSC01_BIU_MCBAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_MCMSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_MCBAS2L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_MCMSK2L_OFS(BIU_REGS)

	/* IP3 Peripheral Bus Controller  - reset 0x18000000 */
	li	t0, SEAD_MSC01_PBCMEM_BASE		     /* 0x1c000000 */
	li	t1, -SEAD_MSC01_PBCMEM_SIZE		     /*-0x20000000 */
	sw	t0, MSC01_BIU_IP3BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP3MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP3BAS2L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP3MSK2L_OFS(BIU_REGS)

	/* Read msc01 version and compare t0 0x0100 */
	lw	t0, MSC01_SC_ID_OFS(BIU_REGS)
	and	t0, 0xffff
	xor	t0, 0x0100
	bne	t0, zero, 101f
	nop

	/* MSC01 version 01.00 */
	/* IP3 was done before IP2 to avoid clash 0x18000000 */
	/* IP1 not used                   - reset 0x10000000 */
	/* Must be done before IP2 to avoid clash 0x10000000 */
	li	t0, SEAD_MSC01_IP1MEM_BASE		     /* 0x1b800000 */
	li	t1, -SEAD_MSC01_IP1MEM_SIZE		     /*-0x1bc00000 */
	sw	t1, MSC01_BIU_IP1MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP1MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS2L_OFS(BIU_REGS)

	/* IP2 not used                   - reset 0x14000000 */
	li	t0, SEAD_MSC01_IP2MEM_BASE		     /* 0x1b400000 */
	li	t1, -SEAD_MSC01_IP2MEM_SIZE		     /*-0x1b800000 */
	sw	t1, MSC01_BIU_IP2MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP2MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS2L_OFS(BIU_REGS)
	sync
	j       ra
	move	v0, zero

101:
	/* MSC01 version 01.01 and later */
	/* no clashing - prioritized compare: REGS > MEM > IP1 > IP2 > IP3 */
	/* IP1 not used                   - reset 0x10000000 */
	/* Must be done before IP2 to avoid clash 0x10000000 */
	li	t0, SEAD_MSC01_REG_BASE      		     /* 0x1bc00000 */
	li	t1, -SEAD_MSC01_IP1MEM_SIZE		     /*-0x1c000000 */
	sw	t1, MSC01_BIU_IP1MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP1MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS2L_OFS(BIU_REGS)

	/* IP2 not used                   - reset 0x14000000 */
	li	t0, SEAD_MSC01_REG_BASE      		     /* 0x1bc00000 */
	li	t1, -SEAD_MSC01_IP2MEM_SIZE		     /*-0x1c000000 */
	sw	t1, MSC01_BIU_IP2MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP2MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS2L_OFS(BIU_REGS)
	sync
	j       ra
	move	v0, zero
#undef BIU_REGS

END(sys_init_sead2_msc01)



LEAF(sys_init_sead)

#define RA	t4

	/**** Store return address ****/
	move	RA, ra

	/**** Configure IIC controller ****/
	li	t0, KSEG1(SEAD_SD_SPDCNF)
#ifdef _SIMULATE_
	li	t1, 5
#else
	li	t1, GCLK_SCALE_WORST_CASE
#endif
	sw	t1, 0(t0)


	/**** Setup MRSCODE register based on SPD settings ****/

	/* Read CAS Latency */
	li	a0, SPD_CASLAT
	jal	read_eeprom
	nop

	/* v0 now contains CAS Latency support mask
	 * If CAS latency 2 is supported, use this otherwise
	 * use CAS latency 3
	 */

	li	t0, SPD_CASLAT_2_BIT
	and	t0, v0
	bne	t0, zero, 1f
	li	v0, 2
	/* 2 not supported, so use 3 */
	li	v0, 3
1:
	sll	v0, SEAD_SD_MRSCODE_CL_SHF

	/* Write MRSCODE */
	li	t0, KSEG1(SEAD_SD_MRSCODE)
	sw	v0, 0( t0 )


	/**** Setup CONFIG register based on SPD settings ****/

	/* Read number of DIMM banks */
	li	a0, SPD_MODULE_BANKS
	jal	read_eeprom
	nop
	/* CSNUM = 2*number of DIMM banks */
	sll	v1, v0, 1
	sll	v1, SEAD_SD_CONFIG_CSN_SHF

	/* Read number of SDRAM banks */
	li	a0, SPD_DEVICE_BANKS
	jal	read_eeprom
	nop
	sll	v0, SEAD_SD_CONFIG_BANKN_SHF
	or	v1, v0

	/* Read number of row address bits */
	li	a0, SPD_ROWS
	jal	read_eeprom
	nop
	li	t0, SPD_ROWS_B_MSK
	and	t1, v0, t0
	bne	t1, zero, config_error
	li	t0, SPD_ROWS_A_MSK
	and	v0, t0
	sll	v0, SEAD_SD_CONFIG_ROWW_SHF
	or	v1, v0

	/* Read number of column address bits */
	li	a0, SPD_COL
	jal	read_eeprom
	nop
	li	t0, SPD_COL_B_MSK
	and	t1, v0, t0
	bne	t1, zero, config_error
	li	t0, SPD_COL_A_MSK
	and	v0, t0
	sll	v0, SEAD_SD_CONFIG_COLW_SHF
	or	v1, v0

	/* Check that CONFIG is for a 32 MByte module */
	/* (BANKN,CSN) must equal (2,4) or (4,2) */
	andi	v0, v1, SEAD_SD_CONFIG_BANKN_MSK
	srl	v0, SEAD_SD_CONFIG_BANKN_SHF
	li	t1, 2
	beq	v0, t1, 4f
	nop
	li	t1, 4
	beq	v0, t1, 2f
	nop
	b	config_error
	nop

4:
	andi	v0, v1, SEAD_SD_CONFIG_CSN_MSK
	srl	v0, SEAD_SD_CONFIG_CSN_SHF
	li	t1, 4
	bne	v0, t1, config_error
	nop
	b	csnbankn_done
	nop

2:
	li	t0, ~SEAD_SD_CONFIG_CSN_MSK
	and	v1, t0
	ori	v1, 2 << SEAD_SD_CONFIG_CSN_SHF



csnbankn_done:

	/* (ROWW,COLW) must equal (11,9) or (12,8) after adjustment */
	andi	v0, v1, SEAD_SD_CONFIG_ROWW_MSK
	srl	v0, SEAD_SD_CONFIG_ROWW_SHF
	li	t1, 11
	beq	v0, t1, 9f
	nop
	li	t1, 12
	beq	v0, t1, 8f
	nop
	b	config_error
	nop

9:
	andi	v0, v1, SEAD_SD_CONFIG_COLW_MSK
	srl	v0, SEAD_SD_CONFIG_COLW_SHF
	li	t1, 9
	blt	v0, t1, config_error
	li	t0, ~SEAD_SD_CONFIG_COLW_MSK
	and	v1, t0
	b	colrow_done
	ori	v1, 9 << SEAD_SD_CONFIG_COLW_SHF

8:
	andi	v0, v1, SEAD_SD_CONFIG_COLW_MSK
	srl	v0, SEAD_SD_CONFIG_COLW_SHF
	li	t1, 8
	blt	v0, t1, config_error
	li	t0, ~SEAD_SD_CONFIG_COLW_MSK
	and	v1, t0
	ori	v1, 8 << SEAD_SD_CONFIG_COLW_SHF

colrow_done:


	/* Write CONFIG */
	li	t0, KSEG1(SEAD_SD_CONFIG)
	sw	v1, 0(t0)



	/**** Setup LATENCIES register based on SPD settings ****/

	/*  We assume 100MHz
	 *  Since we assume 100MHz, we must divide the values
	 *  read from the eeprom (number of ns) with 10.
	 *  We divide by 8 since this is easier (TBD). Result
	 *  is too conservative !
	 */

	/* Read Minimum Row Precarge time */
	li	a0, SPD_MRPT
	jal	read_eeprom
	nop
	srl	v0, 3	/* x/8 */
	/* Values 2..4 supported */
	li	t0, 2
	subu	t0, t0, v0
	bltz	t0, 1f
	nop
	li	v0, 2 /* Set to min value */
1:
	li	t0, 4
	subu	t0, v0, t0
	bltz	t0, 1f
	nop
	li	v0, 4 /* Set to max value */
1:
	sll	v1, v0, SEAD_SD_LATENCIES_TRP_SHF

	/* Read Minimum RAS Pulse Width */
	li	a0, SPD_MRPW
	jal	read_eeprom
	nop
	srl	v0, 3	/* x/8 */
	/* Values 2..8 supported */
	li	t0, 2
	subu	t0, t0, v0
	bltz	t0, 1f
	nop
	li	v0, 2 /* Set to min value */
1:
	li	t0, 8
	subu	t0, v0, t0
	bltz	t0, 1f
	nop
	li	v0, 8 /* Set to max value */
1:
	sll	v0, SEAD_SD_LATENCIES_TRAS_SHF
	or	v1, v0

	/* Read RAS to CAS Defay Min */
	li	a0, SPD_RCDM
	jal	read_eeprom
	nop
	srl	v0, 3	/* x/8 */
	/* Values 2..3 supported */
	li	t0, 2
	subu	t0, t0, v0
	bltz	t0, 1f
	nop
	li	v0, 2 /* Set to min value */
1:
	li	t0, 3
	subu	t0, v0, t0
	bltz	t0, 1f
	nop
	li	v0, 3 /* Set to max value */
1:
	sll	v0, SEAD_SD_LATENCIES_TRCD_SHF
	or	v1, v0

	/* TDPL is set to 2 */
	li	v0, 2 << SEAD_SD_LATENCIES_TDPL_SHF
	or	v1, v0

	/* Write LATENCIES */
	li	t0, KSEG1(SEAD_SD_LATENCIES)
	sw	v1, 0(t0)


	/**** Setup TREFRESH register ****/
	li	t0, KSEG1(SEAD_SD_TREFRESH)
	li	v0, TREFRESH_WORST_CASE
	sw	v0, 0(t0)


	/**** Activate the SDRAM controller ****/
	li	t0, KSEG1(SEAD_SD_REGSSET)
	li	t1, SEAD_SD_REGSSET_SET_BIT
	sw	t1, 0(t0)


	/**** Setup return parameters ****/
	li	v1, SEAD_MEMORY_SIZE

done:
	/**** Return ****/
	jr	RA
	move	v0, zero

read_eeprom:

	DISP_STR( msg_spd )

	/* Setup address */
	li	t9, KSEG1(SEAD_SD_SPDADR)
	sw	a0, 0(t9)

	/* Poll for ready */
	li	t9, KSEG1(SEAD_SD_SPDDAT)
	li	t8, SEAD_SD_SPDDAT_BUSY_BIT
1:
	lw	v0, 0(t9)
	and	a0, v0, t8
	bne	a0, zero, 1b
	nop

	/* Check read err flag */
	li	t8, SEAD_SD_SPDDAT_READ_ERR_BIT
	and	a0, v0, t8
	beq	a0, zero, eeprom_done
	nop

	/* Error */
	jr	RA
	li	v0, ERROR_SPD

	/* Done */

eeprom_done:

	li	t9, SEAD_SD_SPDDAT_RDATA_MSK
	jr	ra
	and	v0, t9


config_error:
	jr	RA
	li	v0, ERROR_SDRAM_CONFIG


END(sys_init_sead)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/* Messages */
	.text

MSG( msg_spd,   "SPD"   )

