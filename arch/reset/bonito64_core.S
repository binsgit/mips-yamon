	
/************************************************************************
 *
 *  bonito64_core.S
 *
 *  Bonito64 specific functions
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
#include <spd.h>
#include <pci_api.h>
#include <pci.h>     
#include <core_bonito64.h>
#include <bonito64.h>

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
 *                          bonito64_write_sdcfg
 *  Description :
 *  -------------
 *  Write data to SDCFG register.
 *
 *  Note :
 *  ------
 *  This routine is called also via sys_func_noram() in module sys.c
 *  and must obey c calling conventions.
 *
 *  Parameters :
 *  ------------
 *  a0 = data to be written.
 *
 *  Return values :
 *  ---------------
 *  0
 *
 ************************************************************************/
LEAF(bonito64_write_sdcfg)

	/* Flush all RAM accesses */	
	sync
	lw	t0,   KSEG1(0)
	move	zero, t0

	/* SDCFG register address */
	li	t3, KSEG1(BONITO_REG_BASE)

	/* Prepare data to write */
	li	t1, BONITO_SDCFG_DRAMMODESET;
	or	t1, a0

	/* Write data to SDCFG */
	sw	t1, BONITO_SDCFG(t3)

	/* Poll for done */
	li	t2, BONITO_SDCFG_DRAMMODESET_DONE
1:		
	lw	t1, BONITO_SDCFG(t3)
	and	t1, t2
	beq	t1, zero, 1b
	nop

	/* Write the data again, but without setting the MODESET request */
	sw	a0, BONITO_SDCFG(t3)

	/* Flush */
	lw	t1, BONITO_SDCFG(t3)
	sync

	/* Done */
	jr	ra
	move    v0, zero

END(bonito64_write_sdcfg)


/************************************************************************	
 *
 *                          bonito64_write_iodevcfg
 *  Description :
 *  -------------
 *  Write data to IODEVCFG register.
 *
 *  Note :
 *  ------
 *  This routine is called only via sys_func_noram() in module sys.c
 *  and must obey c calling conventions.
 *
 *  Parameters :
 *  ------------
 *  a0 = data to be written.
 *
 *  Return values :
 *  ---------------
 *  0
 *
 ************************************************************************/
LEAF(bonito64_write_iodevcfg)

	/* Flush all RAM accesses */	
	sync
	lw	t0,   KSEG1(0)
	move	zero, t0

	/* IODEVCFG register address */
	li	t3, KSEG1(BONITO_REG_BASE)

	/* Write data to SDCFG */
	sw	a0, BONITO_IODEVCFG(t3)

	/* Done */
	jr	ra
	move    v0, zero

END(bonito64_write_iodevcfg)




/************************************************************************	
 *		access_bonito64
 *	Common function calling conventions, see init_core.S
 ************************************************************************/


LEAF(access_bonito64)
	
        li      t8, FUNC_INIT
	beql    t9, t8, bonito64_init
	move	a1, zero /* 0 => CoreBonito64, 1 => Core20K */

        li      t8, FUNC_GET_PCIMEM_BASE
	beq     t9, t8, bonito64_get_pcimem_base
	nop

        li      t8, FUNC_CONFIG_WRITE
	beq     t9, t8, bonito64_config_write
	nop

        li      t8, FUNC_CONFIGURE_SDRAM
	beql    t9, t8, bonito64_configure_sdram
	move	a3, zero /* 0 => CoreBonito64, 1 => Core20K */	

        li      t8, FUNC_SETUP_DECODE
	beq     t9, t8, bonito64_setup_decode
	nop

        /* FUNC_REMAP_PCI_IO */
	j	bonito64_remap_pci_io
	nop
END(access_bonito64)


/************************************************************************	
 *		access_core_20k
 *	Common function calling conventions, see init_core.S
 ************************************************************************/

LEAF(access_core_20k)
	
        li      t8, FUNC_INIT
	beql    t9, t8, bonito64_init
	li      a1, 1 /* 0 => CoreBonito64, 1 => Core20K */

        li      t8, FUNC_GET_PCIMEM_BASE
	beq     t9, t8, bonito64_get_pcimem_base
	nop

        li      t8, FUNC_CONFIG_WRITE
	beq     t9, t8, bonito64_config_write
	nop

        li      t8, FUNC_CONFIGURE_SDRAM
	beql    t9, t8, bonito64_configure_sdram
	li      a3, 1 /* 0 => CoreBonito64, 1 => Core20K */

        li      t8, FUNC_SETUP_DECODE
	beq     t9, t8, bonito64_setup_decode
	nop

        /* FUNC_REMAP_PCI_IO */
	j	bonito64_remap_pci_io
	nop
END(access_core_20k)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/




/************************************************************************	
 *
 *                          bonito64_init
 *  Description :
 *  -------------
 *
 *  Initialise Bonito64 just enough so that we can access PCI.
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = Base address to be used for access to North Bridge registers.
 *  (a0 is ignored since this is not configurable for Bonito64)
 *
 *  a1 = 0 => CoreBonito64
 *  a1 = 1 => Core20K
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 = OK)
 *  v1 = 1 -> Request software reset
 *
 ************************************************************************/
SLEAF(bonito64_init)

        /*  We need to determine if we arrived here due to either :
	 *
	 *  1) A hardware or software reset.
	 *  2) Due to a "go bfc00000" command or under EJTAG control.
	 *
	 *  In case 2, we issue a software reset. We need to do
	 *  this since some devices will not otherwise be properly
	 *  reset.
	 *
	 *  We detect case 2 by checking if the Bonito64 PCI configuration
	 *  space COMMAND register has the "Memory Space Control" bit
	 *  set. This is NOT the case after a hardware or software reset, 
	 *  but IS otherwise the case since it is set below.
	 */
	
	li      t3, KSEG1(BONITO_REG_BASE)
	lw      t1, BONITO_PCICMD(t3)
	and	t1, PCI_SC_CMD_MS_BIT
	beq	t1, zero, 1f
	nop

	/* Request software reset */
	li	v1, 1
	jr	ra
	nop

1:

        /* Setup Bonito64 to have Master capability */
	li	t1, (PCI_SC_CMD_MS_BIT   | PCI_SC_CMD_BM_BIT     | \
		     PCI_SC_CMD_PERR_BIT | PCI_SC_CMD_SERR_BIT )
	sw	t1, BONITO_PCICMD(t3)

        /* Setup Bonito64 PCI latency timer */
	li	t1, PCI_LATTIM_FIXED << BONITO_PCILTIMER_BUSLATENCY_SHIFT
	sw	t1, BONITO_PCILTIMER(t3)

	/* Setup PCI_Lo0,1,2 to map transparently */
	li      t1,\
          BONITO_PCIMAP_WIN(0, BONITO_PCILO_BASE)			   |\
	  BONITO_PCIMAP_WIN(1, BONITO_PCILO_BASE + BONITO_PCIMAP_WINSIZE)  |\
	  BONITO_PCIMAP_WIN(2, BONITO_PCILO_BASE + 2*BONITO_PCIMAP_WINSIZE)
	sw	t1, BONITO_PCIMAP(t3)

	/* Core20K (ie 20Kc CPU) vs CoreBonito64 (ie QED CPU) */
        beq     a1, zero, 1f
	nop

	/* Core20K, setup sequential burstorder */
	lw	t1, BONITO_BONPONCFG(t3)
	li	t2, BONITO_BONPONCFG_BURSTORDER
	or	t1, t2
	sw	t1, BONITO_BONPONCFG(t3)
	b	2f
	nop
1:
	/* CoreBonito64, setup subblock transfer */
	lw	t1, BONITO_BONPONCFG(t3)
	li	t2, ~BONITO_BONPONCFG_BURSTORDER
	and	t1, t2
	sw	t1, BONITO_BONPONCFG(t3)
2:
	/* Done */
	move	v0, zero
	jr	ra
	move	v1, zero

END(bonito64_init)


/************************************************************************	
 *
 *                          bonito64_get_pcimem_base
 *  Description :
 *  -------------
 *
 *  Return base address for PCI memory cycles.
 *
 *  Parameters :
 *  ------------
 * 
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = Base address for PCI memory cycles
 *
 ************************************************************************/
SLEAF(bonito64_get_pcimem_base)

	li	v0, BONITO_PCILO_BASE
	jr	ra
	nop

END(bonito64_get_pcimem_base)


/************************************************************************	
 *
 *                          bonito64_config_write
 *  Description :
 *  -------------
 *
 *  Perform 32 bit PCI configuration write cycle on local bus.
 *
 *  Parameters :
 *  ------------
 * 
 *  a0 = device number (function 0 assumed)
 *  a1 = register
 *  a2 = data
 *  a3 = Base address to be used for access to North Bridge registers.
 *  (a3 is ignored since this is not configurable for Bonito64)
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = 0 if OK
 *  v0 = ERROR_NB_CONFIG_WRITE if not OK
 *
 ************************************************************************/
SLEAF(bonito64_config_write)

	/* Clear master/slave aborts */
	li      t3, KSEG1(BONITO_REG_BASE)
	lw      t1, BONITO_PCICMD(t3)
	li	t2, BONITO_PCICMD_MABORT_CLR | BONITO_PCICMD_MTABORT_CLR
	or	t1, t2
	sw      t1, BONITO_PCICMD(t3)

	/* Setup pattern to be used as PCI "address" for Type 0	cycle */

	/* IDSEL */
	addu	t2, a0, PCI_DEVNUM2IDSEL_OFFSET
	li	t1, 1
	sllv	t1, t1, t2

	/* Register number */
	or	t1, a1	

	/* t1 now holds pattern */

	/* Setup pcimap_cfg */
	srl	t2, t1, 16
	sw	t2, BONITO_PCIMAP_CFG(t3)

	/* Flush Bonito register block */
	lw	t2, BONITO_PCIMAP_CFG(t3)
	sync

	/* Perform access */
	and	t2, t1, 0xFFFF
	li	t1, KSEG1(BONITO_PCICFG_BASE)
	addu	t1, t2
	sw	a2, 0(t1)

	/* Wait till done */
1:		
	lw	t1, BONITO_PCIMSTAT(t3)
	andi	t1, 0xF
	bne	t1, zero, 1b
	nop

	/* Detect master/slave abort */
	lw      t1, BONITO_PCICMD(t3)
	li	t2, BONITO_PCICMD_MABORT_CLR | BONITO_PCICMD_MTABORT_CLR
	and	t1, t2
	beq	t1, zero, 1f
	move	v0, zero
	li	v0, ERROR_NB_CONFIG_WRITE
1:
	jr	ra
	nop

END(bonito64_config_write)


/************************************************************************	
 *
 *                          bonito64_configure_sdram
 *  Description :
 *  -------------
 *
 *  Setup Bonito64 SDRAM configuration
 *
 *  Parameters :
 *  ------------
 * 
 *  a0 = Worst case (lowest) bus freq. (MHz) for setting timing parms.
 *  a1 = Base address to be used for access to North Bridge registers.
 *  a2 = Max SDRAM size supported by platform.
 *  (a1, a2 are ignored since this is not configurable for Bonito64)
 *
 *  a3 = 0 => CoreBonito64
 *  a3 = 1 => Core20K
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 = OK)
 *  v1 = RAM size
 *
 ************************************************************************/
SLEAF(bonito64_configure_sdram)

#define RA	    s0
#define MOD_BANKS   s1
#define BSIZE0	    s2
#define BSIZE1	    s3
#define ACCUM	    s4
		
	move	RA, ra

	/* Check module width */

	li      a0, SPD_MODULE_WIDTH_LO
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	li	a0, 64
	bne	v1, a0, error_bonito64_configure_sdram
	li	v0, ERROR_SDRAM_WIDTH

	li      a0, SPD_MODULE_WIDTH_HI
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	li	a0, 0
	bne	v1, a0, error_bonito64_configure_sdram
	li	v0, ERROR_SDRAM_WIDTH

	
	/**** Setup SDRAM configuration ****/

	/* Misc parameters */
	li	ACCUM, BONITO_SDCFG_AWIDTH64 | BONITO_SDCFG_BABSENT |\
		       BONITO_SDCFG_EXTPRECH			 |\
		       BONITO_SDCFG_EXTRASCAS

	/* Rows */	
	li	a0, SPD_ROWS
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	and	v1, SPD_ROWS_A_MSK
	srl	v1, SPD_ROWS_A_SHF
	subu    v1, 11
	sll	v1, BONITO_SDCFG_AROWBITS_SHIFT
	or	ACCUM, v1

	/* Columns */
	li	a0, SPD_COL
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	and	v1, SPD_COL_A_MSK
	srl	v1, SPD_COL_A_SHF
	subu	v1, 8
	sll	v1, v1, BONITO_SDCFG_ACOLBITS_SHIFT
	or	ACCUM, v1

	/* Device banks */
	li	a0, SPD_DEVICE_BANKS
	jal	sys_spd_read		
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	li	a0, 1
	beq	a0, v1, 1f
	nop
	/* 2 device banks */
	or	ACCUM, BONITO_SDCFG_ABANKBIT
1:
	/* Module banks (aka sides) */
	li	a0, SPD_MODULE_BANKS
	jal	sys_spd_read	
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	move	MOD_BANKS, v1
	li	a0, 1
	beq	a0, MOD_BANKS, 1f
	nop
	/* 2 sides */
	or      ACCUM, BONITO_SDCFG_ASIDES
1:
	/* Size of Bank(s) */
	li	a0, SPD_ROW_DENSITY
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop

	/* bank 0 */
	li	BSIZE0, 512*1024*1024
1:	
	and	t0, v1, 1 << 7
	bne	t0, zero, 2f
	sll	v1, 1
	b	1b
	srl	BSIZE0, 1
2:
	/* bank 1 (if available) */
	li	t0, 1
	beq	MOD_BANKS, t0, 2f		/* 1 bank only		*/
	move	BSIZE1, zero
	and	v1, 0xFF
	bne	v1, zero, error_bonito64_configure_sdram
	li	v0, ERROR_SDRAM_MOD_BANKS	/*  2 asymmetrical banks. 
						    Not supported by Bonito64 */

	move	BSIZE1, BSIZE0			/* 2 symmetrical banks	*/
2:

	/* Validate that memory size <= BONITO_PCILO_BASE
	 * (BONITO_PCILO_BASE is first address not used for RAM)
	 */
	addu	t0, BSIZE1, BSIZE0
	li	t1, BONITO_PCILO_BASE
	sltu	t0, t1, t0
	bne	t0, zero, error_bonito64_configure_sdram
	li	v0, ERROR_SDRAM_SIZE
1:

	/* CAS latency */
	li	a0, SPD_CASLAT
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	and	t0, v1, SPD_CASLAT_2_BIT
	bne	t0, zero, 1f
	nop
	/* CAS latency 2 not supported, so use CAS latency 3 */
	li	t0,    BONITO_SDCFG_EXTRDDATA
	or	ACCUM, t0
1:	

	/* Refresh rate */
	li	a0, SPD_RFSH_RT
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	and	v1, SPD_RFSH_RT_RATE_MSK
	srl	v1, SPD_RFSH_RT_RATE_SHF
	
	li	t0, SPD_RFSH_RT_RATE_3_9	/* 3.9 us    */
	beq	t0, v1, set_rfsh
	li	t0, 0				/* x1        */

	li	t0, SPD_RFSH_RT_RATE_7_8	/* 7.8 us    */
	beq	t0, v1, set_rfsh
	li	t0, 1				/* x2        */

	li	t0, SPD_RFSH_RT_RATE_15_625	/* 15.625 us */
	beq	t0, v1, set_rfsh
	li	t0, 2				/* x4        */

	/* assume 31.3us */
	li	t0, 3				/* x8	     */

set_rfsh :
	sll     t0,    BONITO_SDCFG_DRAMRFSHMULT_SHIFT
	or	ACCUM, t0

	
	/* Determine Parity support */
	li	a0, SPD_CONFIG_TYPE
	jal	sys_spd_read
	nop
	bne	v0, zero, error_bonito64_configure_sdram
	nop
	li	t0, SPD_CONFIG_TYPE_PARITY
	beq	t0, v1, parity_support
	nop
	li	t0, SPD_CONFIG_TYPE_ECC
	beq	t0, v1, error_bonito64_configure_sdram
	li	v0, ERROR_SDRAM_ERRORCHECK
	b	no_parity_support
	nop
parity_support:
	li	t0, BONITO_SDCFG_DRAMPARITY
	or	ACCUM, t0
no_parity_support:		
	
	/* Core20K (ie 20Kc CPU) vs CoreBonito64 (ie QED CPU) */
        beq     a3, zero, 1f
	nop
	
	/* Core20K */
	li      t0, (2 << BONITO_SDCFG_DRAMBURSTLEN_SHIFT)
	b	2f
	nop
1:
	/* CoreBonito64 */
	li      t0, (3 << BONITO_SDCFG_DRAMBURSTLEN_SHIFT)
2:
	or	ACCUM, t0
		
	/* Store setup in bonito sdram configuration register */
	jal	bonito64_write_sdcfg
	move	a0, ACCUM

	/* Set return parameters */
	move	v0, zero		/* No error    */
	addu	v1, BSIZE0, BSIZE1	/* memory size */
		
error_bonito64_configure_sdram:
	
	/* Done */	
	jr	RA
	nop

#undef RA
#undef MOD_BANKS
#undef BSIZE0
#undef BSIZE1
#undef ACCUM
	
END(bonito64_configure_sdram)

		
/************************************************************************	
 *
 *                          bonito64_setup_decode
 *  Description :
 *  -------------
 *
 *  Setup Bonito64 memory decoding (except for SDRAM).
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = PCI memory space base
 *  a1 = PCI memory space size	     
 *  a2 = PCI I/O space base
 *  a3 = PCI I/O space size
 *  t0 = CBUS base
 *  t1 = CBUS size
 *  t2 = Base address to be used for access to North Bridge registers.
 *  (t2 is ignored since this is not configurable for Bonito64)
 *
 *  Return values :
 *  ---------------
 *
 *  Always 0
 *
 ************************************************************************/
SLEAF(bonito64_setup_decode)

	jr	ra
	move    v0, zero

END(bonito64_setup_decode)


/************************************************************************	
 *
 *                          bonito64_remap_pci_io
 *  Description :
 *  -------------
 *
 *  Remap PCI IO range so that PCI IO range starts at address 0 on
 *  PCI (NB no longer transparent for PCI IO).
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = Base address to be used for access to North Bridge registers.	
 *  (a0 is ignored since this is not configurable for Bonito64)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
SLEAF(bonito64_remap_pci_io)

	jr	ra
	move    v0, zero

END(bonito64_remap_pci_io)

