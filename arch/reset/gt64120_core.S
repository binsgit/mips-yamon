
/************************************************************************
 *
 *  gt64120_core.S
 *
 *  Galileo specific functions
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2004 MIPS Technologies, Inc. All rights reserved.
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
#include <gt64120.h>

/************************************************************************
 *  Definitions
 ************************************************************************/
	
/* Parameters for setup_cpu_decode/setup_cpu_decode */
#define LO	a0
#define HI	a1
#define REGLO	a2
#define REGHI	a3

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
 *                          gt64120_write_reg
 *  Description :
 *  -------------
 *  Write data to GT64120 register.
 *  
 *  Note :
 *  ------
 *  This routine is called only via sys_func_noram() in module sys.c
 *  and must obey c calling conventions.
 *
 *  Parameters :
 *  ------------
 *  a0 = address of register
 *  a1 = data to be written
 *
 *  Return values :
 *  ---------------
 *  0
 *
 ************************************************************************/
LEAF(gt64120_write_reg)

	/* Flush all RAM accesses */	
	sync
	lw	t0,   KSEG1(0)
	move	zero, t0

	/* Perform the write */
	GT_SW(	a1, 0, a0 )
		
	/* Done */
	jr	ra
	move    v0, zero

END(gt64120_write_reg)	

	
/************************************************************************	
 *		access_gt64120
 *	Common function calling conventions, see init_core.S
 ************************************************************************/

LEAF(access_gt64120)
	
        li      t8, FUNC_INIT
	beq     t9, t8, gt64120_init
	nop

        li      t8, FUNC_GET_PCIMEM_BASE
	beq     t9, t8, gt64120_get_pcimem_base
	nop

        li      t8, FUNC_CONFIG_WRITE
	beq     t9, t8, gt64120_config_write
	nop

        li      t8, FUNC_CONFIGURE_SDRAM
	beq     t9, t8, gt64120_configure_sdram
	nop

        li      t8, FUNC_SETUP_DECODE
	beq     t9, t8, gt64120_setup_decode
	nop

        /* FUNC_REMAP_PCI_IO */
	j	gt64120_remap_pci_io
	nop

END(access_gt64120)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************	
 *
 *                          gt64120_init
 *  Description :
 *  -------------
 *
 *  Initialise GT64120 just enough so that we can access PCI.
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = Base address to be used for access to North Bridge registers.
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 = OK)
 *  v1 = 1 -> Request software reset
 *
 ************************************************************************/
SLEAF(gt64120_init)

	/* Default return values */
	move	v0, zero
	move	v1, zero
	
        /*  We need to determine if we arrived here due to either :
	 *
	 *  1) A hardware or software reset.
	 *  2) Due to a "go bfc00000" command or under EJTAG control.
	 *
	 *  In case 2, we issue a software reset. We need to do
	 *  this since some devices will not otherwise be properly
	 *  reset.
	 *
	 *  We detect case 2 by checking if the GT64120 
	 *  "Internal Space Decode" register is accessible at the
	 *  default address. This is the case after a hardware or 
	 *  software reset, but is NOT otherwise the case since 
	 *  the mapping of the system controller is changed below.
	 */

	li	t0, KSEG1(GT_DEF_BASE)  /* Base address after a reset	*/
	GT_LW(  t1, GT_ISD_OFS, t0 )
	li	t2, GT_ISD_DEFAULT
	beq	t1, t2, 1f
	nop

	/* Request software reset */
	li	v1, 1
	jr	ra
	nop
1:		
	/* Setup where we want the Galileo registers to be addressed */
	srl	t1, a0, 21
	GT_SW(	t1, GT_ISD_OFS, t0 )
	move	t0, a0
	KSEG1A(t0)
	
	/* Setup GT64120 CPU interface */
	GT_LW(  t1, GT_CPU_OFS, t0 )
	li	t2, ~GT_CPU_WR_MSK
	and	t1, t2
	li	t2, GT_CPU_WR_DDDD << GT_CPU_WR_SHF
	or	t1, t2
	GT_SW(  t1, GT_CPU_OFS, t0 )

        /*  HW bug workaround: Extend BootCS mapping area to access FPGA without
	 *		       4 x access on CBUS per 1 x access from SysAD.
	 */
	li	t1, 0
	GT_SW(	t1, GT_CS3HD_OFS, t0 )
	li	t1, 0xf0
        GT_SW(  t1, GT_BOOTLD_OFS, t0 )
	li	t1, 0xff
        GT_SW(  t1, GT_BOOTHD_OFS, t0 )

	/* Setup byte/word swap */

#ifdef EB	
	li t1, 0x00401
#else
	li t1, 0x10001
#endif
	GT_SW( t1, GT_PCI0_CMD_OFS, t0 )

        /* Change retrycount to a value, which is not 0 */
	li      t1, 0x00ffffff
	GT_SW(  t1, GT_PCI0_TOR_OFS, t0 )

        /* Setup GT64120 to have Master capability */
	li	t1,  GT_CFGADDR_CFGEN_BIT			    |\
		    (PCI_BUS_LOCAL   << GT_CFGADDR_BUSNUM_SHF)      |\
		    (GT_DEV          << GT_CFGADDR_DEVNUM_SHF)      |\
		    (0               << GT_CFGADDR_FUNCNUM_SHF)     |\
		    ((PCI_SC >>2)    << GT_CFGADDR_REGNUM_SHF)
	GT_SW(  t1, GT_PCI0_CFGADDR_OFS, t0 )
	GT_LW(  t2, GT_PCI0_CFGDATA_OFS, t0 )

	li	t1, (PCI_SC_CMD_MS_BIT   | PCI_SC_CMD_BM_BIT     | \
                     PCI_SC_CMD_PERR_BIT | PCI_SC_CMD_SERR_BIT )

        or      t2, t1

	li	t1,  GT_CFGADDR_CFGEN_BIT			    |\
		    (PCI_BUS_LOCAL   << GT_CFGADDR_BUSNUM_SHF)      |\
		    (GT_DEV          << GT_CFGADDR_DEVNUM_SHF)      |\
		    (0               << GT_CFGADDR_FUNCNUM_SHF)     |\
		    ((PCI_SC >> 2)   << GT_CFGADDR_REGNUM_SHF)
	
	GT_SW(	t1, GT_PCI0_CFGADDR_OFS, t0 )
	GT_SW(	t2, GT_PCI0_CFGDATA_OFS, t0 )

        /* Setup GT64120 PCI latency timer */
	li	t1,  GT_CFGADDR_CFGEN_BIT			    |\
		    (PCI_BUS_LOCAL   << GT_CFGADDR_BUSNUM_SHF)      |\
		    (GT_DEV          << GT_CFGADDR_DEVNUM_SHF)      |\
		    (0               << GT_CFGADDR_FUNCNUM_SHF)     |\
		    ((PCI_BHLC >> 2) << GT_CFGADDR_REGNUM_SHF)
	GT_SW(	t1, GT_PCI0_CFGADDR_OFS, t0 )
	GT_LW(	t2, GT_PCI0_CFGDATA_OFS, t0 )

	li	t1, (GT_LATTIM_MIN << PCI_BHLC_LT_SHF)
        or      t2, t1

	li	t1,  GT_CFGADDR_CFGEN_BIT			    |\
		    (PCI_BUS_LOCAL   << GT_CFGADDR_BUSNUM_SHF)      |\
		    (GT_DEV          << GT_CFGADDR_DEVNUM_SHF)      |\
		    (0               << GT_CFGADDR_FUNCNUM_SHF)     |\
		    ((PCI_BHLC >> 2) << GT_CFGADDR_REGNUM_SHF)
	
	GT_SW(	t1, GT_PCI0_CFGADDR_OFS, t0 )
	GT_SW(	t2, GT_PCI0_CFGDATA_OFS, t0 )

	jr	ra
	nop
	
END(gt64120_init)


/************************************************************************	
 *
 *                          gt64120_get_pcimem_base
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
SLEAF(gt64120_get_pcimem_base)

        li      v0, GT_DEF_PCI0_MEM0_BASE
	jr	ra
	nop

END(gt64120_get_pcimem_base)


/************************************************************************	
 *
 *                          gt64120_config_write
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
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = 0 if OK
 *  v0 = ERROR_NB_CONFIG_WRITE if not OK
 *
 ************************************************************************/
SLEAF(gt64120_config_write)

        /* Clear cause register bits */
        GT_SW(  zero, GT_INTRCAUSE_OFS, a3 )

	/* Write to requested register */
	sll	t0, a0, GT_CFGADDR_DEVNUM_SHF
	srl	t1, a1, 2
	sll	t1, GT_CFGADDR_REGNUM_SHF
	or	t0, t1
	li	t1, GT_CFGADDR_CFGEN_BIT			     |\
		    (PCI_BUS_LOCAL        << GT_CFGADDR_BUSNUM_SHF)  |\
		    (0                    << GT_CFGADDR_FUNCNUM_SHF)
	or	t0, t1

	GT_SW(	t0, GT_PCI0_CFGADDR_OFS, a3 )
	GT_SW(	a2, GT_PCI0_CFGDATA_OFS, a3 )

        /* Check for master abort */
        GT_LW(  t0, GT_INTRCAUSE_OFS, a3 )
	li	t1, GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT
	and	t0, t1
	beq	t0, zero, 1f
	move	v0, zero
	li	v0, ERROR_NB_CONFIG_WRITE
1:
	jr	ra
	nop
	
END(gt64120_config_write)


/************************************************************************	
 *
 *                          gt64120_configure_sdram
 *  Description :
 *  -------------
 *
 *  Setup GT64120 SDRAM configuration
 *
 *  Parameters :
 *  ------------
 * 
 *  a0 = Worst case (lowest) bus freq. (MHz) for setting timing parms.
 *  a1 = Base address to be used for access to North Bridge registers.
 *  a2 = Max SDRAM size supported by platform.
 *
 *  Banks are mapped to GT64120 ranges the following way :	
 *
 *  bank0 -> SCSO
 *  bank1 -> SCS2
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 = OK)
 *  v1 = RAM size
 *
 ************************************************************************/
SLEAF(gt64120_configure_sdram)

#define RA		s0
#define FREQ		s1
#define GTBASE		s2
#define BSIZE0		s3
#define BSIZE1		s4
#define RAMSIZE_MAX	s5
#define MOD_BANKS	s6
	
	/* s0..s6 used */	

	move	RA,	     ra
	move	FREQ,	     a0
	move	GTBASE,      a1
	move	RAMSIZE_MAX, a2

	/* Detect DIMM (if present check module width) */
	li      a0, SPD_MODULE_WIDTH_LO
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	li	a0, 64
	bne	v1, a0, error_sdram
	li	v0, ERROR_SDRAM_WIDTH

	li      a0, SPD_MODULE_WIDTH_HI
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	li	a0, 0
	bne	v1, a0, error_sdram
	li	v0, ERROR_SDRAM_WIDTH

	/* Number of module banks */
	li	a0, SPD_MODULE_BANKS
	jal	sys_spd_read	
	nop
	bne	v0, zero, error_sdram
	nop
	move	MOD_BANKS, v1

	/**** Determine size of bank(s) ****/
	li	a0, SPD_ROW_DENSITY
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
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
	beq	v1, zero, 2f
	move	BSIZE1, BSIZE0			/* 2 symmetrical banks	*/

	/*  2 asymmetrical banks. We assume that bank0 is largest.
	 *  (TBD : Is this a fair assumption ?)
	 */
1:
	srl	BSIZE1, 1
	and	t0, v1, 1 << 7
	beq	t0, zero, 1b
	sll	v1, 1
2:

#undef MOD_BANKS

	/* s0..s5 used */

	/*  Validate SDRAM parameters.
	 *
	 *  The following data is available :
	 *
	 *  RAMSIZE_MAX = Max allowed RAM size.
	 *  BSIZE0      = size(bank0) (in bytes).
	 *  BSIZE1      = size(bank1) (in bytes) (if available).
	 *
	 *  Checks performed :	
	 *
	 *  BSIZE<n>          <= GT_MAX_BANKSIZE
	 *  Total memory size <= RAMSIZE_MAX
	 */

	/*  Check and possibly adjust bank0 size :
         *  BSIZE0 <= MIN(GT_MAX_BANKSIZE, RAMSIZE_MAX)
         */

	/* Calc t0 = MIN(GT_MAX_BANKSIZE, RAMSIZE_MAX) */
	li	t0, GT_MAX_BANKSIZE
	sltu    t1, t0, RAMSIZE_MAX
	bne	t1, zero, 1f
	nop
	move	t0, RAMSIZE_MAX
1:
	/* Perform check on BSIZE0 */
	sltu	t1, t0, BSIZE0
	beq	t1, zero, 1f
	nop
	/* bank 0 too large, so adjust size */
	move	BSIZE0, t0
1:

	/*  Check and possibly adjust bank1 size.
	 *  BSIZE1 <= MIN(GT_MAX_BANKSIZE, RAMSIZE_MAX - BSIZE0)
         */

	/* Calc t0 = MIN(GT_MAX_BANKSIZE, RAMSIZE_MAX - BSIZE0) */
	li	t0, GT_MAX_BANKSIZE
	subu    t1, RAMSIZE_MAX, BSIZE0    
	sltu	t2, t0, t1
	bne	t2, zero, 1f
	nop
	move	t0, t1
1:
	/* Perform check on BSIZE1 */
	sltu	t1, t0, BSIZE1
	beq	t1, zero, 1f
	nop
	/* bank 1 too large, so adjust size */
	move	BSIZE1, t0
1:

#undef  RAMSIZE_MAX

	/* s0..s4 used */

	/*  SDRAM Burst Mode register.
	 *  Leave all bits except "Burst Order" unchanged.
	 */
	GT_LW(  t0, GT_SDRAM_BM_OFS, GTBASE )
	li      t1, ~GT_SDRAM_BM_ORDER_MSK
	and	t0, t1
	li	t1, (GT_SDRAM_BM_ORDER_SUB << GT_SDRAM_BM_ORDER_SHF)
	or	t0, t1
	GT_SW(	t0, GT_SDRAM_BM_OFS, GTBASE )
	
	/* SDRAM Address Decode register */
	li	t0, GT_SDRAM_ADDRDECODE_ADDR_2
	GT_SW(	t0, GT_SDRAM_ADDRDECODE_OFS, GTBASE )

#define ACCUM		s5

	/* s0..s5 used */

	/*  SDRAM Bank0 + Bank2 Parameters registers.
	 *  SRAS precharge and SRAS to SCAS delay are
	 *  set conservatively. They may later be optimised based
	 *  on RAM parameters and system frequency.
	 */

	li     ACCUM, (GT_SDRAM_B0_BW_64       << GT_SDRAM_B0_BW_SHF)	     |\
		      (GT_SDRAM_B0_SRASPRCHG_3 << GT_SDRAM_B0_SRASPRCHG_SHF) |\
		      (GT_SDRAM_B0_SRAS2SCAS_3 << GT_SDRAM_B0_SRAS2SCAS_SHF)
	
	/*  CAS latency field : Galileo only supports CAS latency = 2 */
	li	a0, SPD_CASLAT	/* CAS latency field in SPD device    */
	jal	sys_spd_read	/* v1 = CAS latencies supported byte  */
	nop
	bne	v0, zero, error_sdram
	nop
	and	t0, v1, SPD_CASLAT_2_BIT
	beq	t0, zero, error_sdram
	li	v0, ERROR_SDRAM_CASLAT

	ori	ACCUM, (GT_SDRAM_B0_CASLAT_2 << GT_SDRAM_B0_CASLAT_SHF)

#define DEV_BANKS       s6
#define ROWS	        s7
#define COLS	        s8

	/* s0..s8 used */

	/* Number of Rows bits */
	li	a0, SPD_ROWS
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	nop
	move	ROWS, v1
	and	ROWS, SPD_ROWS_A_MSK
	srl	ROWS, SPD_ROWS_A_SHF

	/* Number of Column bits */
	li	a0, SPD_COL
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	nop
	move	COLS, v1
	and	COLS, COLS, SPD_COL_A_MSK
	srl	COLS, SPD_COL_A_SHF

	/*  SDRAM device banks.
	 *  Note : This is the internal bank count of the SDRAM devices, not to
	 *         be confused with the module banks of the SDRAM module.
	 */
	li	a0, SPD_DEVICE_BANKS
	jal	sys_spd_read		
	nop
	bne	v0, zero, error_sdram
	nop
	move	DEV_BANKS, v1

	/* SDRAM device width */
	li	a0, SPD_SDRAM_WIDTH
	jal	sys_spd_read	
	nop
	bne	v0, zero, error_sdram
	nop
	and	v1, SPD_SDRAM_WIDTH_W_MSK
	srl	v1, SPD_SDRAM_WIDTH_W_SHF

	/* Calc SDRAM device size = 
	 * dev_width * dev_banks * 2 ^ (rows + columns) 
	 */
	addu	t0, ROWS, COLS

#undef ROWS
#undef COLS

       /* s0..s6 used */

#define DEV_SIZE        s7

       /* s0..s7 used */

	sll	DEV_SIZE, DEV_BANKS, t0
	multu	DEV_SIZE, v1
	mflo    DEV_SIZE

	/* Setup SDRAM device size (16MBit or 64/128Mbit) */
	li	t0, 16 * 1024 * 1024	/* 16 Mbit */
	beq	DEV_SIZE, t0, Mbit16
	nop
	ori	ACCUM, (GT_SDRAM_B0_SIZE_64M << GT_SDRAM_B0_SIZE_SHF)
	b	burstlen
	nop
Mbit16:	
	ori	ACCUM, (GT_SDRAM_B0_SIZE_16M << GT_SDRAM_B0_SIZE_SHF)

burstlen:	
		
	/* Burst length field : Galileo only supports burstlen = 8 */
	li	a0, SPD_BURSTLEN
	jal	sys_spd_read	     /* v1 = burstlen byte */
	nop
	bne	v0, zero, error_sdram
	nop
	and	t0, v1, SPD_BURSTLEN_8_MSK
	beq	t0, zero, error_sdram
	li	v0, ERROR_SDRAM_BURSTLEN
	
	ori	ACCUM, (GT_SDRAM_B0_BLEN_8 << GT_SDRAM_B0_BLEN_SHF)

	/*  Parity field :
	 *  If SDRAM module supports parity, enable parity support.
	 *  else if SDRAM module supports ECC, indicate error.
	 *  If SDRAM module does not support any of those, disable 
	 *  parity support.
	 */

	/* Determine Parity support */

#define PARITY_SUPPORT	s8

	/* s0..s8 used */

	li	a0, SPD_CONFIG_TYPE
	jal	sys_spd_read	    /* v1 = config type byte */
	nop
	bne	v0, zero, error_sdram
	nop
	li	t0, SPD_CONFIG_TYPE_PARITY
	beq	t0, v1, parity_support
	nop
	li	t0, SPD_CONFIG_TYPE_ECC
	beq	t0, v1, error_sdram
	li	v0, ERROR_SDRAM_ERRORCHECK
	b	write_banks
	move	PARITY_SUPPORT, zero

parity_support:
	ori	ACCUM, GT_SDRAM_B0_PAR_BIT
	li	PARITY_SUPPORT, 1

write_banks:

	/* Number of device banks */
	li	t0, 2
	beq	DEV_BANKS, t0, dev_banks2
	li	t0, 4
	bne	DEV_BANKS, t0, error_sdram
	li	v0, ERROR_SDRAM_DEV_BANKS
dev_banks4:	
	ori	ACCUM, GT_SDRAM_B0_64BITINT_4 << GT_SDRAM_B0_64BITINT_SHF
	b	write_bank_parms
	nop
dev_banks2:			
	ori	ACCUM, GT_SDRAM_B0_64BITINT_2 << GT_SDRAM_B0_64BITINT_SHF

write_bank_parms:		

	move	t0, ACCUM		
	GT_SW(	ACCUM, GT_SDRAM_B0_OFS, GTBASE )
	GT_SW(	t0,    GT_SDRAM_B2_OFS, GTBASE )

	/* SDRAM configuration register */
	li	ACCUM, GT_SDRAM_CFG_NINTERLEAVE_BIT | GT_SDRAM_CFG_DUPBA_BIT

	/*  Set RefIntCnt based on SDRAM module requirements and
	 *  the slowest possible bus frequency.
	 *  This may later be changed when the bus freq. is known.
	 */

	/* Determine min refresh rate and type */
	li	a0, SPD_RFSH_RT
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	nop
	and	v1, SPD_RFSH_RT_RATE_MSK	
	srl	v1, SPD_RFSH_RT_RATE_SHF
	
	li	t0, SPD_RFSH_RT_RATE_125	/* 125 us */
	beq	t0, v1, calc_rfsh_count
	li	t0, 125

	li	t0, SPD_RFSH_RT_RATE_62_5	/* 62.5 us */
	beq	t0, v1, calc_rfsh_count
	li	t0, 62
	
	li	t0, SPD_RFSH_RT_RATE_31_3	/* 31.3 us */
	beq	t0, v1, calc_rfsh_count
	li	t0, 31
	
	li	t0, SPD_RFSH_RT_RATE_15_625	/* 15.625 us */
	beq	t0, v1, calc_rfsh_count
	li	t0, 15
	
	li	t0, SPD_RFSH_RT_RATE_7_8	/* 7.8 us */
	beq	t0, v1, calc_rfsh_count
	li	t0, 7
	
	/* assume 3.9us */
	li	t0, 3

calc_rfsh_count:
	/* RefIntCnt = rate (in us) * lowest frequency (in MHz) */
	
	multu	FREQ, t0
	mflo 	t0
	
	/* Make sure the result fits in the REFINT field */
	li	t1, (GT_SDRAM_CFG_REFINT_MSK >> GT_SDRAM_CFG_REFINT_SHF)
	sltu	t2, t1, t0
	beq	t2, zero, set_refint
	nop
	move	t0, t1	/* Set refintcnt = max value */

set_refint:		
	/* Add the result to ACCUM */
	sll	t0, GT_SDRAM_CFG_REFINT_SHF
	or	ACCUM, t0

	/*  Setup RMW bit :
	 *  If parity is enabled AND SDRAM module contains SDRAM
	 *  device(s) dedicated to parity (as opposed to SDRAM 
	 *  modules where parity bit is included in each SDRAM device)
	 *  then Enable else Disable.
	 */
	
	beq     PARITY_SUPPORT, zero, store_sdram_cfg
	
	/* Read Error Checking RAM width */
	li	a0, SPD_EC_SDRAM
	jal	sys_spd_read
	nop
	bne	v0, zero, error_sdram
	nop
	and	v1, SPD_EC_SDRAM_WIDTH_MSK
	beq	v1, zero, store_sdram_cfg
	nop

rmw:	
	ori	ACCUM, GT_SDRAM_CFG_RMW_BIT
	
store_sdram_cfg :
	
	/* Ready to store CALC in SDRAM configuration register */
	GT_SW(	ACCUM, GT_SDRAM_CFG_OFS, GTBASE )

        /**** Setup decoding ****/

	/* SCS[1:0] */
	move	LO,    zero
	add	HI,    LO, BSIZE0
	add	REGLO, GTBASE, GT_SCS10LD_OFS
	add	REGHI, GTBASE, GT_SCS10HD_OFS
	jal	setup_cpu_decode
	nop
	bne	v0,    zero, error_sdram
	nop

	/* SCS0 */
	add	REGLO, GTBASE, GT_SCS0LD_OFS
	add	REGHI, GTBASE, GT_SCS0HD_OFS
	jal	setup_dev_decode
	nop

	/* SCS1 (not used) */
	move	HI,    zero
	add	REGLO, GTBASE, GT_SCS1LD_OFS
	add	REGHI, GTBASE, GT_SCS1HD_OFS
	jal	setup_dev_decode
	nop

	/* SCS[3:2] */
	move	LO,    BSIZE0
	add	HI,    LO, BSIZE1
	add	REGLO, GTBASE, GT_SCS32LD_OFS
	add	REGHI, GTBASE, GT_SCS32HD_OFS
	jal	setup_cpu_decode
	nop
	bne	v0,    zero, error_sdram
	nop


	/* SCS2 */
	add	REGLO, GTBASE, GT_SCS2LD_OFS
	add	REGHI, GTBASE, GT_SCS2HD_OFS
	jal	setup_dev_decode
	nop

	/* SCS3 (not used) */
	move	HI, zero
	add	REGLO, GTBASE, GT_SCS3LD_OFS
	add	REGHI, GTBASE, GT_SCS3HD_OFS
	jal	setup_dev_decode
	nop

	move	v0, zero
	addu	v1, BSIZE0, BSIZE1

error_sdram:
	jr	RA
	nop

#undef RA
#undef FREQ
#undef BSIZE0
#undef BSIZE1
#undef GTBASE
#undef ACCUM
#undef DEV_BANKS
#undef DEV_SIZE
#undef PARITY_SUPPORT
	
END(gt64120_configure_sdram)

		
/************************************************************************	
 *
 *                          gt64120_setup_decode
 *  Description :
 *  -------------
 *
 *  Setup GT64120 memory decoding (except for SDRAM).
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
 *
 *  Return values :
 *  ---------------
 *
 *  0 :				No error.
 *  ERROR_NB_DECODE :		Illegal ranges requested.
 *
 ************************************************************************/
SLEAF(gt64120_setup_decode)

#define RA		s0
#define PCIMEM_BASE	s1
#define PCIMEM_SIZE	s2
#define PCIIO_BASE	s3
#define PCIIO_SIZE	s4
#define CBUS_BASE	s5
#define CBUS_SIZE	s6
#define GTBASE		s7

	move	RA,           ra
	move	PCIMEM_BASE,  a0
	move	PCIMEM_SIZE,  a1
	move	PCIIO_BASE,   a2
	move	PCIIO_SIZE,   a3
	move	CBUS_BASE,    t0
	move	CBUS_SIZE,    t1
	move	GTBASE,	      t2

	/**** PCI0_MEM0 ****/	

	/*  Adjust the banksize to that it is contained within 
	 *  a 256MB block (addr[31:28] constant).
	 */
	move	LO, PCIMEM_BASE
	srl	t0, LO, 28		/* t0 = LO[31:28]	    */

	addu	HI, LO, PCIMEM_SIZE	/* HI address before adjust */
	subu	t1, HI, 1
	srl	t1, 28			/* t1 = (HI-1)[31:28]  	    */

	beq	t0, t1, 1f
	nop

	/* We crossed a 256MB boundary. Limit HI to boundary address */
	addu    t0, 1
	sll	HI, t0, 28
1:

	/*  Adjust the banksize so that it allows PCI IO to
	 *  be present as a "red page" in the middle.
	 */
	sltu	t0, PCIIO_BASE, HI
	beq	t0, zero, 3f
	nop
	move	HI, PCIIO_BASE
3:

	/* Store HI since we will need it for PCI0_MEM1 */
	move	t3, HI

	/* Setup decode */
	add	REGLO, GTBASE, GT_PCI0M0LD_OFS
	add	REGHI, GTBASE, GT_PCI0M0HD_OFS
	jal	setup_cpu_decode  /* Modifies t0..t2 */
	nop
	bne	v0,    zero, error_decode
	nop

	/* Calc whether PCI memory was covered by PCI0_MEM0 */
	addu	HI, PCIMEM_BASE, PCIMEM_SIZE
	bne	HI, t3, setup_mem1
	move	LO, t3

	/* We don't need PCI0_MEM1, so disable it */
	move	HI, zero

setup_mem1:	
	/**** PCI0_MEM1 ****/
	/* Adjust the bankstart so that it does not interfeer with PCI IO */
	add	t1, PCIIO_BASE, PCIIO_SIZE
	sltu	t0, LO, t1
	beq	t0, zero, 1f
	nop
	move	LO, t1
1:

	add	REGLO, GTBASE, GT_PCI0M1LD_OFS
	add	REGHI, GTBASE, GT_PCI0M1HD_OFS
	jal	setup_cpu_decode
	nop
	bne	v0,    zero, error_decode
	nop

setup_io:	
	/**** PCI0_IO ****/

	move	LO, PCIIO_BASE
	addu	HI, LO, PCIIO_SIZE
	add	REGLO, GTBASE, GT_PCI0IOLD_OFS
	add	REGHI, GTBASE, GT_PCI0IOHD_OFS
	jal	setup_cpu_decode
	nop
	bne	v0,    zero, error_decode
	nop

	/**** PCI1 (not used) ****/

	move	HI, zero
	add	REGLO, GTBASE, GT_PCI1M0LD_OFS
	add	REGHI, GTBASE, GT_PCI1M0HD_OFS
	jal	setup_cpu_decode
	nop
	add	REGLO, GTBASE, GT_PCI1M1LD_OFS
	add	REGHI, GTBASE, GT_PCI1M1HD_OFS
	jal	setup_cpu_decode
	nop
	add	REGLO, GTBASE, GT_PCI1IOLD_OFS
	add	REGHI, GTBASE, GT_PCI1IOHD_OFS
	jal	setup_cpu_decode
	nop

	/**** CS[2:0] ****/

	move	LO, CBUS_BASE
	add     HI, LO, CBUS_SIZE
	add	REGLO, GTBASE, GT_CS20LD_OFS
	add	REGHI, GTBASE, GT_CS20HD_OFS
	jal	setup_cpu_decode
	nop
	bne	v0,    zero, error_decode
	nop

	/* CS0 */
	add	REGLO, GTBASE, GT_CS0LD_OFS
	add	REGHI, GTBASE, GT_CS0HD_OFS
	jal	setup_dev_decode
	nop
	
	/* CS1, CS2 (not used) */
	move	HI, zero
	add	REGLO, GTBASE, GT_CS1LD_OFS
	add	REGHI, GTBASE, GT_CS1HD_OFS
	jal	setup_dev_decode
	nop
	add	REGLO, GTBASE, GT_CS2LD_OFS
	add	REGHI, GTBASE, GT_CS2HD_OFS
	jal	setup_dev_decode
	nop
		
	/**** Done, return to caller ****/
done:
	move	v0, zero

error_decode :
	jr	RA
	nop

#undef RA
#undef PCIMEM_BASE
#undef PCIMEM_SIZE
#undef PCIIO_BASE
#undef PCIIO_SIZE
#undef CBUS_BASE
#undef CBUS_SIZE
#undef GTBASE

END(gt64120_setup_decode)


/************************************************************************	
 *
 *                          gt64120_remap_pci_io
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
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
SLEAF(gt64120_remap_pci_io)
	
	sw	zero,  GT_PCI0IORMP_OFS(a0)
	jr	ra
	nop

END(gt64120_remap_pci_io)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/*  Functions for setup of GT64120 cpu/device decoder
 *
 *  Input parameters :
 *  ------------------
 *  LO    (a0) = lo   address
 *  HI    (a1) = high address	
 *  REGLO (a2) = lo   register
 *  REGHI (a3) = high register
 */

/************************************************************************	
 *                          setup_cpu_decode
 ************************************************************************/
SLEAF(setup_cpu_decode)

	/* Check that LO and HI addresses are 2MB aligned (LO[20:0] == 0) */
	li	t0, MSK(21)
	and	t0, LO
	bne	t0, zero, error_cpu_decode
	nop

	/*  Check that LO and HI are in the same 256 MB block 
	 *  ((HI-1)[31:28] == LO[31:28])
	 *
	 *  Unless HI = 0 indicating that range should be disabled.
	 */
	beq	HI, zero, 1f
	nop
        srl	t0, LO, 28
	subu	t1, HI, 1
	srl	t1, 28
	bne	t0, t1, error_cpu_decode
	nop
1:
	b	setup_decode
	li	t0, 21

error_cpu_decode :

	li	v0, ERROR_NB_DECODE	
	jr      ra
	nop

END(setup_cpu_decode)


/************************************************************************	
 *                          setup_dev_decode
 ************************************************************************/
SLEAF(setup_dev_decode)		

	b	setup_decode
	li	t0, 20

END(setup_dev_decode)	
		

/************************************************************************	
 *                          setup_decode
 ************************************************************************/
SLEAF(setup_decode)

	bne	HI, zero, 1f
	nop
	li	t1, 0xFFFF
	b	2f
	move	t2, zero
	
1:	
	srlv	t1, LO, t0	
	subu	t2, HI, 1
	srlv	t2, t2, t0
2:
	GT_SW(	t1, 0, REGLO )
	GT_SW(	t2, 0, REGHI )
	jr	ra
	move	v0, zero

END(setup_decode)

