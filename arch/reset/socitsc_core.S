/************************************************************************
 *
 *  socitsc_core.S
 *
 *  SOCitSC (Mips System Controller) specific functions
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
#include <spd.h>
#include <pci_api.h>
#include <pci.h>
#include <product.h>
#include <core_sys.h>

#include <socitsc_dendefine.h>
#include <socitsc_reginit.h>
#include <socitsc_reginit_sdr.h>
#include <socitsc_reginit_ddr.h>
#include <socitsc.h>

/************************************************************************
 *  Definitions
 ************************************************************************/
#define SYSCTRL_DDR_SUPPORT
	
/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

	.set	noreorder

/************************************************************************	
 * access_socitsc
 *	Common function calling conventions, see init_core.S
 ************************************************************************/

LEAF(access_socitsc)
	
#define X(this, last, type) \
	xori	t9, FUNC_##last^FUNC_##this; \
	beqz	t9, socitsc_##type
#define FUNC_0	0

	X(0,			INIT,			init)
	X(INIT,			GET_PCIMEM_BASE,	get_pcimem_base)
	X(GET_PCIMEM_BASE,	CONFIG_WRITE,		config_write)
	X(CONFIG_WRITE,		CONFIGURE_SDRAM,	configure_sdram)
	X(CONFIGURE_SDRAM,	SETUP_DECODE,		setup_decode)
	X(SETUP_DECODE,		REMAP_PCI_IO,		remap_pci_io)
	nop
#undef X
#undef FUNC_0
	
	/* Unsupported function */
	j	ra
	 nop
END(access_socitsc)

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/



/************************************************************************
 *
 *                          socitsc_init
 *  Description :
 *  -------------
 *  Initialise socitsc just enough so that we can access PCI.
 *
 *  Parameters :
 *  ------------
 *  a0 = Base address to be used for access to North Bridge registers.
 *  (a0 is ignored since this is hard defined by core_sys.h)
 *
 *  Return values :
 *  ---------------
 *  v0 = error code (0 = OK)
 *  v1 = 1 -> Request software reset
 *
 ************************************************************************/
SLEAF(socitsc_init)
	
        /*  We need to determine if we arrived here due to either :
	 *
	 *  1) A hardware or software reset.
	 *  2) Due to a "go bfc00000" command or under EJTAG control.
	 *
	 *  In case 2, we issue a software reset. We need to do
	 *  this since some devices will not otherwise be properly
	 *  reset.
	 *
	 *  We detect case 2 by seeing if AHB2 region 1 is enabled
	 */

#define AHB_REGS t9
	li	AHB_REGS, KSEG1(SOCITSC_AHB2_BC_BASE)
	lw	t0, AHBMREGOFFS(1,CTRL)(AHB_REGS)
	and	t0, AHBM_CTRL_ENABLE_MSK
	bnez	t0, init_done
	 li	v1, 1		/* Already enabled... Reset needed */

/* bringup version of SocitSC does not have an AHB0 */	
#ifndef SOCITSC_BRINGUP
	li	AHB_REGS, KSEG1(SOCITSC_AHB0_BC_BASE)

	/* Map PCI bridge registers - AHB 0 Master 0 */
	li	t0, SOCITSC_MIPS_PCI_BASE
	li	t1, -SOCITSC_MIPS_PCI_SIZE
	li	t2, 0
	sw	t0, AHBMREGOFFS(0,BASE)(AHB_REGS)
	sw	t1, AHBMREGOFFS(0,MASK)(AHB_REGS)
	sw	t2, AHBMREGOFFS(0,TRANS)(AHB_REGS)
	lw	t0, AHBMREGOFFS(0,CTRL)(AHB_REGS)
	or	t0, AHBM_CTRL_ENABLE_MSK
	sw	t0, AHBMREGOFFS(0,CTRL)(AHB_REGS)

	/* Map outgoing PCI accesses - AHB 0 Master 1 */
	/* Local:10000000-17ffffff -> PCI:10000000-17ffffff */
	li	t0, MALTA_PCIMEM1_BASE
	li	t1, -MALTA_SC_PCIMEM1_SIZE
	li	t2, 0
	sw	t0, AHBMREGOFFS(1,BASE)(AHB_REGS)
	sw	t1, AHBMREGOFFS(1,MASK)(AHB_REGS)
	sw	t2, AHBMREGOFFS(1,TRANS)(AHB_REGS)
	lw	t0, AHBMREGOFFS(1,CTRL)(AHB_REGS)
	or	t0, AHBM_CTRL_ENABLE_MSK
	sw	t0, AHBMREGOFFS(1,CTRL)(AHB_REGS)
	
	/* Map incoming PCI accesses - AHB 0 Slave 0 */
	/* PCI:00000000-0fffffff -> LOCAL:00000000-0fffffff */
	li	t0, MALTA_SYSTEMRAM_BASE
	li	t1, -MALTA_SYSTEMRAM_SIZE
	li	t2, 0
	sw	t0, AHBSREGOFFS(1,BASE)(AHB_REGS)
	sw	t1, AHBSREGOFFS(1,MASK)(AHB_REGS)
	sw	t2, AHBSREGOFFS(1,TRANS)(AHB_REGS)
	lw	t0, AHBSREGOFFS(1,CTRL)(AHB_REGS)
	or	t0, AHBS_CTRL_ENABLE_MSK
	sw	t0, AHBSREGOFFS(1,CTRL)(AHB_REGS)

	/* FIXME: not quite sure why we do this */
	/* Map incoming PCI accesses - AHB 0 Slave 1 */
	/* PCI:80000000-ffffffff -> LOCAL:80000000-ffffffff */
	li	t0, 0x80000000
	li	t1, -0x80000000
	li	t2, 0
	sw	t0, AHBSREGOFFS(1,BASE)(AHB_REGS)
	sw	t1, AHBSREGOFFS(1,MASK)(AHB_REGS)
	sw	t2, AHBSREGOFFS(1,TRANS)(AHB_REGS)
	lw	t0, AHBSREGOFFS(1,CTRL)(AHB_REGS)
	or	t0, AHBS_CTRL_ENABLE_MSK
	sw	t0, AHBSREGOFFS(1,CTRL)(AHB_REGS)
#endif
		
	/* Setup AHB2 */
	li	AHB_REGS, KSEG1(SOCITSC_AHB2_BC_BASE)
	
	/* Map MIPS IP - AHB2 Master 1*/
	li	t0, SOCITSC_MIPS_IP_BASE
	li	t1, -SOCITSC_MIPS_IP_SIZE
	move	t2, t0					/* Untranslated */
	sw	t0, AHBMREGOFFS(1,BASE)(AHB_REGS)
	sw	t1, AHBMREGOFFS(1,MASK)(AHB_REGS)
	sw	t2, AHBMREGOFFS(1,TRANS)(AHB_REGS)
	lw	t0, AHBMREGOFFS(1,CTRL)(AHB_REGS)
	or	t0, AHBM_CTRL_ENABLE_MSK
	sw	t0, AHBMREGOFFS(1,CTRL)(AHB_REGS)

	/* Map PBC memory - AHB 2 Master 2 */
	li	t0, SOCITSC_MIPS_CBUS_BASE
	li	t1, -SOCITSC_MIPS_CBUS_SIZE
	move	t2, t0					/* Untranslated */
	sw	t0, AHBMREGOFFS(2,BASE)(AHB_REGS)
	sw	t1, AHBMREGOFFS(2,MASK)(AHB_REGS)
	sw	t2, AHBMREGOFFS(2,TRANS)(AHB_REGS)
	lw	t0, AHBMREGOFFS(2,CTRL)(AHB_REGS)
	or	t0, AHBM_CTRL_ENABLE_MSK
	sw	t0, AHBMREGOFFS(2,CTRL)(AHB_REGS)
#undef AHB_REGS

#define PBC_REGS t9
	/* Speed up peripheral bus controller with configuration: */
	/*       CDIV=1, WS=16, WSIDLE=3, ADM=0,                  */
	/*       CAT=1,  CDT=1, RAT=2,    RDT=0, WAT=2, WDT=0     */
	li	PBC_REGS, KSEG1(SOCITSC_MIPS_PBC_REG_BASE)
	li	t0, (   1<<MSC01_PBC_CLKCFG_SHF)
	sw	t0, MSC01_PBC_CLKCFG_OFS(PBC_REGS)
	li	t0, (   1<<MSC01_PBC_CS0TIM_CDT_SHF) | \
		    (   1<<MSC01_PBC_CS0TIM_CAT_SHF)
	sw	t0, MSC01_PBC_CS0TIM_OFS(PBC_REGS)
	li	t0, (   0<<MSC01_PBC_CS0RW_RDT_SHF) | \
		    (   2<<MSC01_PBC_CS0RW_RAT_SHF) | \
		    (   0<<MSC01_PBC_CS0RW_WDT_SHF) | \
		    (   2<<MSC01_PBC_CS0RW_WAT_SHF)
	sw	t0, MSC01_PBC_CS0RW_OFS(PBC_REGS)
	lw	t0, MSC01_PBC_CS0CFG_OFS(PBC_REGS) /* preserve DTYP */
	li	t1, MSC01_PBC_CS0CFG_DTYP_MSK
	and	t0, t1
	ori	t0, (   0<<MSC01_PBC_CS0CFG_ADM_SHF) | \
		    (   3<<MSC01_PBC_CS0CFG_WSIDLE_SHF) | \
		    (0x10<<MSC01_PBC_CS0CFG_WS_SHF)
	sw	t0, MSC01_PBC_CS0CFG_OFS(PBC_REGS) /* WS=16, WSIDLE=3, ADM=0 */
#undef PBC_REGS

#define PCI_REGS t9
	li	PCI_REGS, KSEG1(SOCITSC_MIPS_PCI_REG_BASE)

        /* Setup PCI_SC2PMEM (0x10000000-0x17ffffff transparent) */
	li	t0, SOCITSC_PCIMEM_BASE
	sw	t0, MSC01_PCI_SC2PMBASL_OFS(PCI_REGS)
	sw	t0, MSC01_PCI_SC2PMMAPL_OFS(PCI_REGS)
	li	t0, (-SOCITSC_PCIMEM_SIZE) & MSC01_PCI_SC2PMMSKL_MSK_MSK
	sw	t0, MSC01_PCI_SC2PMMSKL_OFS(PCI_REGS)

	/* Setup PCI_SC2PIO (0x1780000-0x17ffffff -> 0x00000000-0x007fffff) */
	li	t0, SOCITSC_PCIIO_BASE
	sw	t0, MSC01_PCI_SC2PIOBASL_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_SC2PIOMAPL_OFS(PCI_REGS)
	li	t0, (-SOCITSC_PCIIO_SIZE) & MSC01_PCI_SC2PIOMSKL_MSK_MSK
	sw	t0, MSC01_PCI_SC2PIOMSKL_OFS(PCI_REGS)

	/* Setup PCI_BAR0 to 256Mbyte memory window */
	li	t0, -CORE_SYS_MEMORY_SIZE
	sw	t0, MSC01_PCI_BAR0_OFS(PCI_REGS)

	/* Setup PCI_P2SC (0x00000000-0x0fffffff transparent) */
	sw	t0, MSC01_PCI_P2SCMSKL_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_P2SCMAPL_OFS(PCI_REGS)

	/* Set up Vendor and device ID */
	li	t1, (PCI_VENDID_MIPS << PCI_ID_VENDORID_SHF) | \
	            (PCI_DEVID_MSC01 << PCI_ID_DEVID_SHF)
	sw	t1, MSC01_PCI_HEAD0_OFS(PCI_REGS)

	/* Set up Sub system ID and Sub system Vendor ID */
	sw	t1, MSC01_PCI_HEAD11_OFS(PCI_REGS)

	/* Set up Class code and Revision ID */
	li	t0, 0x06000001
	sw	t0, MSC01_PCI_HEAD2_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD3_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD4_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD5_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD6_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD7_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD8_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD9_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD10_OFS(PCI_REGS)

	sw	zero, MSC01_PCI_HEAD12_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD13_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD14_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_HEAD15_OFS(PCI_REGS)

	/* Set up Command Register */
	li	t0, PCI_SC_CMD_FBB_BIT  | \
		    PCI_SC_CMD_SERR_BIT | \
		    PCI_SC_CMD_PERR_BIT | \
		    PCI_SC_CMD_MS_BIT   | \
		    PCI_SC_CMD_BM_BIT
	sw	t0, MSC01_PCI_HEAD1_OFS(PCI_REGS)

	/* Set up byte swap properties */
#ifdef EB
	li	t0, (MSC01_PCI_SWAP_IO_NOSWAP    <<MSC01_PCI_SWAP_IO_SHF) | \
		    (MSC01_PCI_SWAP_MEM_NOSWAP   <<MSC01_PCI_SWAP_MEM_SHF)| \
		    (MSC01_PCI_SWAP_BAR0_BYTESWAP<<MSC01_PCI_SWAP_BAR0_SHF);
#else
	li	t0, (MSC01_PCI_SWAP_IO_NOSWAP    <<MSC01_PCI_SWAP_IO_SHF) | \
		    (MSC01_PCI_SWAP_MEM_NOSWAP   <<MSC01_PCI_SWAP_MEM_SHF)| \
		    (MSC01_PCI_SWAP_BAR0_NOSWAP  <<MSC01_PCI_SWAP_BAR0_SHF);
#endif
	sw	t0, MSC01_PCI_SWAP_OFS(PCI_REGS)

	/* Enable PCI host bridge to respond to configuration cycles */
	lw	t0, MSC01_PCI_CFG_OFS(PCI_REGS)	/* preserve retry count */
	li	t1, MSC01_PCI_CFG_RA_BIT |	/* slave read ahead  */     \
		    MSC01_PCI_CFG_G_BIT  |	/* master gathering  */     \
		    MSC01_PCI_CFG_EN_BIT  	/* configuration cycles */
	or	t0, t1
#ifdef SOCITSC_BRINGUP
	# set retry count to 100 attempts
	and	t0, ~(MSC01_PCI_CFG_MRTRY_MSK << MSC01_PCI_CFG_MRTRY_SHF)
	or	t0, 100 << MSC01_PCI_CFG_MRTRY_SHF
#endif		
	sw	t0, MSC01_PCI_CFG_OFS(PCI_REGS)
	sync

	move	v1, zero	/* No reset needed */

init_done:
	j	ra
	 move	v0, zero
END(socitsc_init)

/************************************************************************
 *
 *                          socitsc_get_pcimem_base
 *  Description :
 *  -------------
 *  Return base address for PCI memory cycles.
 *
 ************************************************************************/
SLEAF(socitsc_get_pcimem_base)
	li	v0, SOCITSC_PCIMEM_BASE
	j	ra
	 nop
END(socitsc_get_pcimem_base)

/************************************************************************
 *
 *                          socitsc_config_write
 *  Description :
 *  -------------
 *  Perform 32 bit PCI configuration write cycle on local bus.
 *
 *  Parameters :
 *  ------------
 *  a0 = device number (function 0 assumed)
 *  a1 = register
 *  a2 = data
 *  a3 = Base address to be used for access to North Bridge registers.
 *  (a0 is ignored since this is hard defined by core_sys.h)
 *
 *  Return values :
 *  ---------------
 *  v0 = 0 if OK
 *  v0 = ERROR_NB_CONFIG_WRITE if not OK
 *
 ************************************************************************/
SLEAF(socitsc_config_write)

#define PCI_REGS t9
	li	PCI_REGS, KSEG1(SOCITSC_MIPS_PCI_REG_BASE)

        /* Clear cause register bits */
	li	t0, MSC01_PCI_INTSTAT_MA_BIT | MSC01_PCI_INTSTAT_TA_BIT
	sw	t0, MSC01_PCI_INTSTAT_OFS(PCI_REGS)

        /* Setup address */
	sll	a0, MSC01_PCI_CFGADDR_DNUM_SHF
	and	a0, MSC01_PCI_CFGADDR_DNUM_MSK
	and	a1, MSC01_PCI_CFGADDR_RNUM_MSK
	or	a0, a1
	sw	a0, MSC01_PCI_CFGADDR_OFS(PCI_REGS)
	sync

	/* Write data */
        sw	a2, MSC01_PCI_CFGDATA_OFS(PCI_REGS)

	/* Detect master/slave abort */
	sync
	lw	t1, MSC01_PCI_INTSTAT_OFS(PCI_REGS)
	and	t1, t0
	beq	t1, zero, 1f
	 move	v0, zero
	li	v0, ERROR_NB_CONFIG_WRITE
1:
	jr	ra
	 nop
END(socitsc_config_write)

/************************************************************************
 *
 *                          socitsc_configure_sdram
 *  Description :
 *  -------------
 *  Setup CoreSYS SDRAM configuration
 *
 *  Parameters :
 *  ------------
 *  a0 = Worst case (lowest) bus freq. (MHz) for setting timing parms.
 *  a1 = Base address to be used for access to North Bridge registers.
 *  a2 = Max SDRAM size supported by platform.
 *  (a1, a2 are ignored since this is not configured for CoreSYS)
 *       Max SDRAM size is defined by CORE_SYS_PCILO_BASE (256 Mbyte)
 *
 *  Return values :
 *  ---------------
 *  v0 = error code (0 = OK)
 *  v1 = RAM size
 *
 ************************************************************************/
SLEAF(socitsc_configure_sdram)

#define RA		s0
#define FREQ		s1
#define SC_REGS		s2
#define RAMSIZE		s3
#define CSNUM           s4
#define ROWS		s5
#define COLUMNS		s6
#define RA2		s7
#define ROWSB		s8
#define COLUMNSB	a3

	move	RA, ra
	move	FREQ, a0

/* Worst case setup based on frequency assumptions */
#define GCLK_SCALE_WORST_CASE 200

	/* Read and check basic RAM type */
	jal	my_read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
#ifdef SYSCTRL_DDR_SUPPORT
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	bne	v0, t0, 2f
	 nop
	DISP_STR(msg_socitsc_ddr) /* destroys t5, t6, t7, t8, t9, ra */
	b	3f
	 nop
2:
#endif
	li	t0, SPD_FUNDAMENTAL_TYPE_SDR
	bne	v0, t0, error_sdram
	 li	v0, ERROR_SPD
	DISP_STR(msg_socitsc_sdr) /* destroys t5, t6, t7, t8, t9, ra */

3:
	/* Read basic data width - must be 0x0040 or 0x0048 (parity byte) */
	jal	my_read_eeprom
	 li      a0, SPD_MODULE_WIDTH_LO
	or	v0, 8
	li	a0, 64 | 8
	bne	v0, a0, error_sdram
	 li	v0, ERROR_SDRAM_WIDTH

	jal	my_read_eeprom
	 li      a0, SPD_MODULE_WIDTH_HI
	bnez	v0, error_sdram
	 li	v0, ERROR_SDRAM_WIDTH

	/* Read number of DIMM banks (CSNUM) */
	jal	my_read_eeprom
	 li	a0, SPD_MODULE_BANKS
	move	CSNUM, v0

	/* Read number of SDRAM banks */
	jal	my_read_eeprom
	 li	a0, SPD_DEVICE_BANKS
	li	t1, 4
	bne	t1, v0, error_sdram
	 li	v0, ERROR_SDRAM_DEV_BANKS

	/****************************************
	** Compute RAM size
	** and values for physical bank config
	*/
	/* Read number of row address bits */
	jal	my_read_eeprom
	 li	a0, SPD_ROWS

	andi	ROWS, v0, SPD_ROWS_A_MSK
	srl	ROWS, SPD_ROWS_A_SHF
	andi	ROWSB, v0, SPD_ROWS_B_MSK
	srl	ROWSB, SPD_ROWS_B_SHF

	/* Read number of column address bits */
	jal	my_read_eeprom
	 li	a0, SPD_COL
	andi	COLUMNS, v0, SPD_COL_A_MSK
	srl	COLUMNS, SPD_COL_A_SHF
	andi	COLUMNSB, v0, SPD_COL_B_MSK
	srl	COLUMNSB, SPD_COL_B_SHF

	/* Configure registers for physical bank 0 */
	/* Check and adjust rows, columns if necessary */
        /* The following defines do not survive my_read_eeprom! */
#define CONFIG0 a1
#define CONFIG1 v1

	jal	checkrows
	 nop

	move	RAMSIZE, v0

#define INIT_DENALI_SDR_REG(reg)		\
	li	t0, SC_DENALI_SDR_REG_##reg##_VAL;	\
	sw	t0, (4 * reg)(SC_REGS)

#ifdef SYSCTRL_DDR_SUPPORT
#define INIT_DENALI_DDR_REG(reg)		\
	li	t0, SC_DENALI_DDR_REG_##reg##_VAL;	\
	sw	t0, (4 * reg)(SC_REGS)
#endif

#undef  SC_GET_REG_FIELD				/* result in t0 */
#define SC_GET_REG_FIELD(reg, width, offset)	\
	lw	t0, (4 * reg)(SC_REGS); 	\
	srl	t0, offset; 			\
	li	t1, ((1 << width) - 1);		\
	and	t0, t1

#undef  SC_SET_REG_FIELD				/* value in t0 */
#define SC_SET_REG_FIELD(reg, width, offset)	\
	lw	t1, (4 * reg)(SC_REGS);		\
	sll	t0, offset;			\
	li	t2, (((1 << width) -1) << offset);\
	and	t0, t2;				\
	not	t2, t2;				\
	and	t1, t2;				\
	or	t1, t0;				\
	sw	t1, 4 * reg(SC_REGS)


	/* Adjust ROWS downwards if necessary */
1:	li	t0, CORE_SYS_MEMORY_SIZE
	bleu	RAMSIZE, t0, 2f
	 nop
	sub	ROWS, 1
	jal	checkrows
	 nop
	b	1b
	 move	RAMSIZE, v0
2:
	/* Get memory controller base address */
	li	SC_REGS, KSEG1(SOCITSC_DRAM_REG_BASE)

#ifdef SYSCTRL_DDR_SUPPORT
	/* ... read and check basic RAM type ... */
	jal	my_read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	beq	v0, t0, 1f
	 nop
#endif

	/* Do basic initialisation of all relevant registers */
#if SC_DENALI_SDR_REG_0_USED == 1
	INIT_DENALI_SDR_REG(0)
#endif
#if SC_DENALI_SDR_REG_1_USED == 1
	INIT_DENALI_SDR_REG(1)
#endif
#if SC_DENALI_SDR_REG_2_USED == 1
	INIT_DENALI_SDR_REG(2)
#endif
#if SC_DENALI_SDR_REG_3_USED == 1
	INIT_DENALI_SDR_REG(3)
#endif
#if SC_DENALI_SDR_REG_4_USED == 1
	INIT_DENALI_SDR_REG(4)
#endif
#if SC_DENALI_SDR_REG_5_USED == 1
	INIT_DENALI_SDR_REG(5)
#endif
#if SC_DENALI_SDR_REG_6_USED == 1
	INIT_DENALI_SDR_REG(6)
#endif
#if SC_DENALI_SDR_REG_7_USED == 1
	INIT_DENALI_SDR_REG(7)
#endif
#if SC_DENALI_SDR_REG_8_USED == 1
	INIT_DENALI_SDR_REG(8)
#endif
#if SC_DENALI_SDR_REG_9_USED == 1
	INIT_DENALI_SDR_REG(9)
#endif
#if SC_DENALI_SDR_REG_10_USED == 1
	INIT_DENALI_SDR_REG(10)
#endif
#if SC_DENALI_SDR_REG_11_USED == 1
	INIT_DENALI_SDR_REG(11)
#endif
#if SC_DENALI_SDR_REG_12_USED == 1
	INIT_DENALI_SDR_REG(12)
#endif
#if SC_DENALI_SDR_REG_13_USED == 1
	INIT_DENALI_SDR_REG(13)
#endif
#if SC_DENALI_SDR_REG_14_USED == 1
	INIT_DENALI_SDR_REG(14)
#endif
#if SC_DENALI_SDR_REG_15_USED == 1
	INIT_DENALI_SDR_REG(15)
#endif
#if SC_DENALI_SDR_REG_16_USED == 1
	INIT_DENALI_SDR_REG(16)
#endif
#if SC_DENALI_SDR_REG_17_USED == 1
	INIT_DENALI_SDR_REG(17)
#endif
#if SC_DENALI_SDR_REG_18_USED == 1
	INIT_DENALI_SDR_REG(18)
#endif
#if SC_DENALI_SDR_REG_19_USED == 1
	INIT_DENALI_SDR_REG(19)
#endif
#if SC_DENALI_SDR_REG_20_USED == 1
	INIT_DENALI_SDR_REG(20)
#endif
#if SC_DENALI_SDR_REG_21_USED == 1
	INIT_DENALI_SDR_REG(21)
#endif
#if SC_DENALI_SDR_REG_22_USED == 1
	INIT_DENALI_SDR_REG(22)
#endif
#if SC_DENALI_SDR_REG_23_USED == 1
	INIT_DENALI_SDR_REG(23)
#endif
#if SC_DENALI_SDR_REG_24_USED == 1
	INIT_DENALI_SDR_REG(24)
#endif
#if SC_DENALI_SDR_REG_25_USED == 1
	INIT_DENALI_SDR_REG(25)
#endif
#if SC_DENALI_SDR_REG_26_USED == 1
	INIT_DENALI_SDR_REG(26)
#endif
#if SC_DENALI_SDR_REG_27_USED == 1
	INIT_DENALI_SDR_REG(27)
#endif
#if SC_DENALI_SDR_REG_28_USED == 1
	INIT_DENALI_SDR_REG(28)
#endif
#if SC_DENALI_SDR_REG_29_USED == 1
	INIT_DENALI_SDR_REG(29)
#endif
#if SC_DENALI_SDR_REG_30_USED == 1
	INIT_DENALI_SDR_REG(30)
#endif
#if SC_DENALI_SDR_REG_31_USED == 1
	INIT_DENALI_SDR_REG(31)
#endif

#ifdef SYSCTRL_DDR_SUPPORT
	b	2f
	 nop
1:
#if SC_DENALI_DDR_REG_0_USED == 1
	INIT_DENALI_DDR_REG(0)
#endif
#if SC_DENALI_DDR_REG_1_USED == 1
	INIT_DENALI_DDR_REG(1)
#endif
#if SC_DENALI_DDR_REG_2_USED == 1
	INIT_DENALI_DDR_REG(2)
#endif
#if SC_DENALI_DDR_REG_3_USED == 1
	INIT_DENALI_DDR_REG(3)
#endif
#if SC_DENALI_DDR_REG_4_USED == 1
	INIT_DENALI_DDR_REG(4)
#endif
#if SC_DENALI_DDR_REG_5_USED == 1
	INIT_DENALI_DDR_REG(5)
#endif
#if SC_DENALI_DDR_REG_6_USED == 1
	INIT_DENALI_DDR_REG(6)
#endif
#if SC_DENALI_DDR_REG_7_USED == 1
	INIT_DENALI_DDR_REG(7)
#endif
#if SC_DENALI_DDR_REG_8_USED == 1
	INIT_DENALI_DDR_REG(8)
#endif
#if SC_DENALI_DDR_REG_9_USED == 1
	INIT_DENALI_DDR_REG(9)
#endif
#if SC_DENALI_DDR_REG_10_USED == 1
	INIT_DENALI_DDR_REG(10)
#endif
#if SC_DENALI_DDR_REG_11_USED == 1
	INIT_DENALI_DDR_REG(11)
#endif
#if SC_DENALI_DDR_REG_12_USED == 1
	INIT_DENALI_DDR_REG(12)
#endif
#if SC_DENALI_DDR_REG_13_USED == 1
	INIT_DENALI_DDR_REG(13)
#endif
#if SC_DENALI_DDR_REG_14_USED == 1
	INIT_DENALI_DDR_REG(14)
#endif
#if SC_DENALI_DDR_REG_15_USED == 1
	INIT_DENALI_DDR_REG(15)
#endif
#if SC_DENALI_DDR_REG_16_USED == 1
	INIT_DENALI_DDR_REG(16)
#endif
#if SC_DENALI_DDR_REG_17_USED == 1
	INIT_DENALI_DDR_REG(17)
#endif
#if SC_DENALI_DDR_REG_18_USED == 1
	INIT_DENALI_DDR_REG(18)
#endif
#if SC_DENALI_DDR_REG_19_USED == 1
	INIT_DENALI_DDR_REG(19)
#endif
#if SC_DENALI_DDR_REG_20_USED == 1
	INIT_DENALI_DDR_REG(20)
#endif
#if SC_DENALI_DDR_REG_21_USED == 1
	INIT_DENALI_DDR_REG(21)
#endif
#if SC_DENALI_DDR_REG_22_USED == 1
	INIT_DENALI_DDR_REG(22)
#endif
#if SC_DENALI_DDR_REG_23_USED == 1
	INIT_DENALI_DDR_REG(23)
#endif
#if SC_DENALI_DDR_REG_24_USED == 1
	INIT_DENALI_DDR_REG(24)
#endif
#if SC_DENALI_DDR_REG_25_USED == 1
	INIT_DENALI_DDR_REG(25)
#endif
#if SC_DENALI_DDR_REG_26_USED == 1
	INIT_DENALI_DDR_REG(26)
#endif
#if SC_DENALI_DDR_REG_27_USED == 1
	INIT_DENALI_DDR_REG(27)
#endif
#if SC_DENALI_DDR_REG_28_USED == 1
	INIT_DENALI_DDR_REG(28)
#endif
#if SC_DENALI_DDR_REG_29_USED == 1
	INIT_DENALI_DDR_REG(29)
#endif
#if SC_DENALI_DDR_REG_30_USED == 1
	INIT_DENALI_DDR_REG(30)
#endif
#if SC_DENALI_DDR_REG_31_USED == 1
	INIT_DENALI_DDR_REG(31)
#endif

2:
#endif
	/* Mask off external interrupt */
	li	t0, 0xf
	SC_SET_REG_FIELD(SC_INT_MASK_ADDR, SC_INT_MASK_WIDTH, SC_INT_MASK_OFFSET)
 
	/* Registered DIMM? */
	jal	my_read_eeprom
	 li	a0, SPD_MODATTR
	and	t0, v0, SPD_MODATTR_REG_MSK
	srl	t0, SPD_MODATTR_REG_SHF
	SC_SET_REG_FIELD(SC_REG_DIMM_ENABLE_ADDR,
			SC_REG_DIMM_ENABLE_WIDTH, SC_REG_DIMM_ENABLE_OFFSET)

	/********************************************
	** Use SPD values obtained earlier (ROWS, COLS and CSNUM) to fix up
	** some of the defaults above.
	*/

	/* Set addrpins = addrbits_max - addrbits */
	SC_GET_REG_FIELD(SC_MAX_ROW_REG_ADDR, SC_MAX_ROW_REG_WIDTH, SC_MAX_ROW_REG_OFFSET)
	sub	t0, ROWS
	SC_SET_REG_FIELD(SC_ADDR_PINS_ADDR, SC_ADDR_PINS_WIDTH, SC_ADDR_PINS_OFFSET)

	/* Set colsize = colbits_max - colbits */
	SC_GET_REG_FIELD(SC_MAX_COL_REG_ADDR, SC_MAX_COL_REG_WIDTH, SC_MAX_COL_REG_OFFSET)
	sub	t0, COLUMNS
	SC_SET_REG_FIELD(SC_COLUMN_SIZE_ADDR, SC_COLUMN_SIZE_WIDTH, SC_COLUMN_SIZE_OFFSET)

	/* Set csmap = (1 << csnum) - 1 */
	li	t0, 1
	sll	t0, CSNUM
	subu	t0, 1
	SC_SET_REG_FIELD(SC_CS_MAP_ADDR, SC_CS_MAP_WIDTH, SC_CS_MAP_OFFSET)

	/********************************************
	** Get SPD timing values to fix up some of the defaults above.
	** We should do this on the basis of the clock frequency, but we
	** don't have that yet so use some default min and max values.
	*/
#define BROKEN_LIKE_SOCIT	1	/* Make timings as wrong as SOC-it! */
#ifdef BROKEN_LIKE_SOCIT
#define MIN_PERIOD	8	/* Default to minimum cycle time of 6nS */
#else
#define MIN_PERIOD	6	/* Default to minimum cycle time of 6nS */
#endif
#define MAX_PERIOD	60	/* Default to maximum cycle time of 60nS */

#define PERIOD	a2
#define TRC_CLAMP a3

#ifdef SYSCTRL_DDR_SUPPORT
	/* For DDR, don't mess with any timings but refresh */
	jal	my_read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	beq	v0, t0, 9f
	 nop
#endif
	li	PERIOD, MIN_PERIOD

	/* CAS Latency */
	jal	my_read_eeprom
	 li	a0, SPD_CASLAT

	andi	a0, v0, SPD_CASLAT_2_BIT
	bne	a0, zero, 2f
	 li	v0, 2		/* If CAS latency 2 supported, use 2 */
	li	v0, 3		/* Otherwise use 3 */
2:
	move	t0, v0
	SC_SET_REG_FIELD(SC_CASLAT_ADDR, SC_CASLAT_WIDTH, SC_CASLAT_OFFSET)

	sll	v0, 1		/* CASLAT_LIN params defined in half cycles */
	move	t0, v0
	SC_SET_REG_FIELD(SC_CASLAT_LIN_ADDR, SC_CASLAT_LIN_WIDTH, SC_CASLAT_LIN_OFFSET)

	move	t0, v0
	SC_SET_REG_FIELD(SC_CASLAT_LIN_GATE_ADDR,
			SC_CASLAT_LIN_GATE_WIDTH, SC_CASLAT_LIN_GATE_OFFSET)

	/* RAS Precharge */
	jal	my_read_eeprom
	 li	a0, SPD_TRP

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRP_CLAMP 2
	sltiu	t1, t0, TRP_CLAMP	/* Clamp TRP >= 2 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRP_CLAMP
1:
	move	TRC_CLAMP, t0		/* TRC_CLAMP = TRP; updated later */
	SC_SET_REG_FIELD(SC_TRP_ADDR, SC_TRP_WIDTH, SC_TRP_OFFSET)

	/* RAS -> CAS delay */
	jal	my_read_eeprom
	 li	a0, SPD_TRCD

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRCD_CLAMP 2
	sltiu	t1, t0, TRCD_CLAMP	/* Clamp TRCD >= 2 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRCD_CLAMP
1:
	SC_SET_REG_FIELD(SC_TRCD_INT_ADDR, SC_TRCD_INT_WIDTH, SC_TRCD_INT_OFFSET)

	/* RAS -> RAS delay (tRRD) different banks */
	jal	my_read_eeprom
	 li	a0, SPD_TRRD

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0
	SC_SET_REG_FIELD(SC_TRRD_ADDR, SC_TRRD_WIDTH, SC_TRRD_OFFSET)

	/* RAS min */
	jal	my_read_eeprom
	 li	a0, SPD_TRAS

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRAS_MIN_CLAMP 6
	sltiu	t1, t0, TRAS_MIN_CLAMP	/* Clamp TRAS_MIN >= 6 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRAS_MIN_CLAMP
1:
	add	TRC_CLAMP, t0		/* Now TRC_CLAMP = TRP + TRAS_MIN */
	SC_SET_REG_FIELD(SC_TRAS_MIN_ADDR, SC_TRAS_MIN_WIDTH, SC_TRAS_MIN_OFFSET)

	/* RAS -> RAS same bank */
	jal	my_read_eeprom
	 li	a0, SPD_TRC

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

	sltu	t1, t0, TRC_CLAMP	/* Clamp TRC >= TRP + TRAS_MIN */
	beq	t1, zero, 1f
	 nop
	move	t0, TRC_CLAMP
1:
	SC_SET_REG_FIELD(SC_TRC_ADDR, SC_TRC_WIDTH, SC_TRC_OFFSET)

	/* Max RAS time */
	/* No SPD entry for this, so use standard PC133 val of 100000nS */
	li	t0, ((TRAS_MAX_DEFAULT + MAX_PERIOD - 1) / MAX_PERIOD)
	SC_SET_REG_FIELD(SC_TRAS_MAX_ADDR, SC_TRAS_MAX_WIDTH, SC_TRAS_MAX_OFFSET)

	/* Write Recovery */
	/* No SPD entry for this, so use standard PC133 val of 15nS */
	li	t0, ((TWR_DEFAULT + MIN_PERIOD - 1) / MIN_PERIOD)
	SC_SET_REG_FIELD(SC_TWR_INT_ADDR, SC_TWR_INT_WIDTH, SC_TWR_INT_OFFSET)

#ifdef SYSCTRL_DDR_SUPPORT
9:
#endif
	/* Refresh */
	jal	my_read_eeprom
	 li	a0, SPD_RFSH_RT
	and	v0, SPD_RFSH_RT_RATE_MSK

	li	t0, SPD_RFSH_RT_RATE_125	/* 125 us */
	beq	t0, v0, 1f
	 li	t0, 125000>>1
	li	t0, SPD_RFSH_RT_RATE_62_5	/* 62.5 us */
	beq	t0, v0, 1f
	 li	t0, 62500>>1
	li	t0, SPD_RFSH_RT_RATE_31_3	/* 31.3 us */
	beq	t0, v0, 1f
	 li	t0, 31300>>1
	li	t0, SPD_RFSH_RT_RATE_15_625	/* 15.6 us */
	beq	t0, v0, 1f
	 li	t0, 15625>>1
	li	t0, SPD_RFSH_RT_RATE_7_8	/* 7.8 us */
	beq	t0, v0, 1f
	 li	t0, 7800>>1
	li	t0, 3900>>1			/* assume 3.9 us */
1:
	sll	t0, 1
	li	PERIOD, MAX_PERIOD

	divu	t0, PERIOD		/* t0 /= PERIOD */
	mflo	t0
	SC_SET_REG_FIELD(SC_TREF_ADDR, SC_TREF_WIDTH, SC_TREF_OFFSET)

	li	t0, 1
	SC_SET_REG_FIELD(SC_TREF_ENABLE_ADDR, SC_TREF_ENABLE_WIDTH, SC_TREF_ENABLE_OFFSET)
#under TRC_CLAMP
#undef PERIOD

	/********************************************
	** Start the controller
	*/
	li	t0, 1
	SC_SET_REG_FIELD(SC_START_ADDR, SC_START_WIDTH, SC_START_OFFSET)

	/* for (cnt = 100000; (int_stat = intstat()) != 0 && --cnt != 0; ); */
#define CNT t4
	li	CNT, 100000
1:
	SC_GET_REG_FIELD(SC_INT_STATUS_ADDR, SC_INT_STATUS_WIDTH, SC_INT_STATUS_OFFSET)
	bne	t0, zero, 2f
	 subu	CNT, 1
	bne	CNT, zero, 1b
	 nop

	/* SDRAM: Timed out waiting for initialisation complete interrupt */
	b	config_error
	 nop
#undef CNT

2:
	/* Clear the interrupt */
	li	t0, 0xf
	SC_SET_REG_FIELD (SC_INT_ACK_ADDR, SC_INT_ACK_WIDTH, SC_INT_ACK_OFFSET)

	/* Re-enable external interrupt */
	move	t0, zero
	SC_SET_REG_FIELD(SC_INT_MASK_ADDR, SC_INT_MASK_WIDTH, SC_INT_MASK_OFFSET)

#ifdef SYSCTRL_DDR_SUPPORT
	/* We're running too slowly for a DDR SDRAM's DLL, so disable it */
	jal	my_read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	bne	v0, t0, 9f
	 nop
	li	t0, 1
	SC_SET_REG_FIELD(SC_EMRS1_DATA_ADDR, SC_EMRS1_DATA_WIDTH, SC_EMRS1_DATA_OFFSET)
	li	t0, 1
	SC_SET_REG_FIELD(SC_WRITE_MODEREG_ADDR,
			SC_WRITE_MODEREG_WIDTH, SC_WRITE_MODEREG_OFFSET)

	/********************************************
	** Try to determine the DDR CAS latency value
	*/
	/* Write 16 unique and easily-identifiable words */
	li	t0, KSEG1BASE	/* start addr */
	li	t1, 0xaa55aa00	/* start pattern */
	li	t2, 0xaa55aa15	/* final pattern */
1:	sw	t1, 0(t0)
	addu	t1, 1
	bleu	t1, t2, 1b
	 addu	t0, 4

	/* for (i = 2; i < 11;  i++) { set_caslat(i); check_pattern() } */
	li	a0, KSEG1BASE
	li	a2, 0xaa55aa00
	li	v0, 2
	li	t4, 11
1:	move	t0, v0
	SC_SET_REG_FIELD(SC_CASLAT_LIN_ADDR, SC_CASLAT_LIN_WIDTH, SC_CASLAT_LIN_OFFSET)
	move	t0, v0
	SC_SET_REG_FIELD(SC_CASLAT_LIN_GATE_ADDR,
			SC_CASLAT_LIN_GATE_WIDTH, SC_CASLAT_LIN_GATE_OFFSET)
	lw	t0, 0(a0)
	beq	t0, a2, 9f
	 nop
	addu	v0, 1
	bltu	v0, t4, 1b
	 nop

	/* CASLAT_LIN not found */
	DISP_STR( msg_socitsc_cl ) /* destroys t5, t6, t7, t8, t9, ra */
99:	b	99b
	 nop
9:
#endif

#ifdef SOCITSC_BRINGUP
	/* memory width... */
	srl	RAMSIZE,1	
#endif
#if 0	
	/* This is only required to initialise parity */

	/* Clear memory */
	DISP_STR( msg_pram_clr ) /* destroys t5, t6, t7, t8, t9, ra */

	li	t0, KSEG1BASE	/* start addr */
	or	t1, t0, RAMSIZE /* limit */

	/* The processor is assumed to be MIPS32/64 compatible */
	MFC0(   v0, C0_Config )
	and	v0, M_ConfigAT
	sub	v0, K_ConfigAT_MIPS32<<S_ConfigAT
	bne	v0, zero, 2f
	 nop
	
	/* 32 bit CPU */
1:
	sw	zero, 0(t0)
	sw	zero, 4(t0)
	sw	zero, 8(t0)
	sw	zero, 12(t0)
	sw	zero, 16(t0)
	sw	zero, 20(t0)
	sw	zero, 24(t0)
	addu	t0, 32
	bnel	t0, t1, 1b
	 sw	zero, -4(t0)	# Store nullified when t0==t1

	b	8f
	 nop
	
	/* 64 bit CPU */
2:
SET_MIPS3()
	sd	zero, 0(t0)
	sd	zero, 8(t0)
	sd	zero, 16(t0)
	sd	zero, 24(t0)
	sd	zero, 32(t0)
	sd	zero, 40(t0)
	sd	zero, 48(t0)
	addu	t0, 64		
	bnel	t0, t1, 2b
	 sd	zero, -8(t0)	# Store nullified when t0==t1
SET_MIPS0()

8:
#endif

	/********************************************
	** Return
	*/
	move	v1, RAMSIZE
	jr	RA
	 li	v0, 0

config_error:
	li	v0, ERROR_SDRAM_CONFIG
error_sdram:
	jr	RA
	nop

/************************************************************
** local subroutine - read from spd eeprom.
** makes a master return in case of error.
** Returns byte read in v0.
*/

my_read_eeprom:
	/* Use FPGA IIC */
	/* Changes a0, a1, at, t0, t1, t2, t3, t5, t6, t7, t8, t9, v0, v1, RA2 */
	move	RA2, ra
	jal	sys_spd_read
	 nop
	bnez	v0, error_msc01_eeprom
	 nop
	jr	RA2
	 move	v0, v1

error_msc01_eeprom:
	jr	RA
	 nop


/************************************************************
** local subroutine - check and adjust rows and columns
** Makes a master return in case of error.
** Changes t0 and v0
** Uses t2	
** Returns memory size in v0
*/
checkrows:
	/* ROWS below 11 is not supported */
	sltiu	t0, ROWS, 11
	bne	t0, zero, config_error
	nop

	/* COLUMNS below 8 is not supported */
	sltiu	t0, COLUMNS, 8
	bne	t0, zero, config_error
	nop

	/* ROWS above 13 is limited to 13 */
	sltiu	t0, ROWS, 13+1
	bne	t0, zero, 1f
	nop
	li	ROWS, 13
1:

	/* COLUMNS is limited to less than ROWS */
	sltu	t0, COLUMNS, ROWS
	bne	t0, zero, 1f
	nop
	addiu	COLUMNS, ROWS, -1
1:

	/* COLUMNS + ROWS must not exceed 25 */
	add	t0, COLUMNS, ROWS
	sltiu	t0, t0, 26
	bne	t0, zero, 1f
	nop
2:
	addiu	COLUMNS, COLUMNS, -1
1:
	add	t0, ROWS, COLUMNS
	add	t0, 2+3		/* 4 banks * 8 bytes */

	/* ROC-it - don't mess with MC, use CSNUM as size multiplier */
	move	v0, CSNUM

	sll	v0, t0	/* return size in bytes */

	/* CoreSYS - limit ram size to max 256 Mbyte */
	li	t0, CORE_SYS_MEMORY_SIZE
	slt	t0, t0, v0
	bnez	t0, 2b
	 nop

	jr	ra
	 nop

#undef RA
#undef MC_REGS
#undef RAMSIZE
#undef CSNUM
#undef ROW
#undef ROWSB
#undef COLUMNS
#undef COLUMNSB
#undef FREQ
#undef RA2

END(socitsc_configure_sdram)

/************************************************************************
 *
 *                          socitsc_setup_decode
 *  Description :
 *  -------------
 *  Setup SOCitSC memory decoding (except for SDRAM)
 *   - already done for SOCitSC
 *
 *  Parameters :
 *  ------------
 *  a0 = PCI memory space base
 *  a1 = PCI memory space size
 *  a2 = PCI I/O space base
 *  a3 = PCI I/O space size
 *  t0 = CBUS base
 *  t1 = CBUS size
 *  t2 = Base address to be used for access to North Bridge registers.
 *
 *  Return values :
 *  ---------------
 *  Always 0
 *
 ************************************************************************/
SLEAF(socitsc_setup_decode)
	j	ra
	 move	v0, zero
END(socitsc_setup_decode)

/************************************************************************
 *
 *                          socitsc_remap_pci_io
 *  Description :
 *  -------------
 *  Remap PCI IO range so that PCI IO range starts at address 0 on
 *  PCI - already does for SOCitSC.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
SLEAF(socitsc_remap_pci_io)
	j	ra
	 move	v0, zero
END(socitsc_remap_pci_io)
	
MSG(msg_pram_clr,    "PRAM_CLR" )
MSG(msg_socitsc_sdr, "SDR DIMM")
MSG(msg_socitsc_ddr, "DDR DIMM")
MSG(msg_socitsc_cl,  "CL: ERR" )



