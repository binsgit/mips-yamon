/************************************************************************
 *
 *  msc01_core.S
 *
 *  MSC01 (Mips System Controller) specific functions
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
#include <product.h>
#include <core_sys.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define SYSCTRL_DDR_SUPPORT 1


/************************************************************************
 *  Compile time assertions
 ************************************************************************/

/* check that SIZES are power of two */

#define POW2CHECK(X) ((X) - (((((X)-1) ^ (X)) >> 1 ) + 1 ))

#if ( POW2CHECK(CORE_SYS_MEMORY_SIZE) )
.error "CORE_SYS_MEMORY_SIZE MUST BE A POWER OF 2"
#endif
#if ( POW2CHECK(CORE_SYS_PCIMEM_SIZE2N) )
.error "CORE_SYS_PCIMEM_SIZE2N MUST BE A POWER OF 2"
#endif
#if ( POW2CHECK(CORE_SYS_PCIIO_SIZE) )
.error "CORE_SYS_PCIIO_SIZE MUST BE A POWER OF 2"
#endif
#if ( POW2CHECK(CORE_SYS_IP1MEM_SIZE) )
.error "CORE_SYS_IP1MEM_SIZE MUST BE A POWER OF 2"
#endif
#if ( POW2CHECK(CORE_SYS_REG_SIZE) )
.error "CORE_SYS_REG_SIZE MUST BE A POWER OF 2"
#endif
#if ( POW2CHECK(CORE_SYS_PBCMEM_SIZE) )
.error "CORE_SYS_PBCMEM_SIZE MUST BE A POWER OF 2"
#endif

/* check that SIZES are aligned properly */

#if (  (CORE_SYS_MEMORY_SIZE-1) & CORE_SYS_MEMORY_BASE )
.error "CORE_SYS_MEMORY_SIZE NOT ALIGNED PROPERLY"
#endif
#if (  (CORE_SYS_PCIMEM_SIZE2N-1) & CORE_SYS_PCIMEM_BASE )
.error "CORE_SYS_PCIMEM_SIZE2N NOT ALIGNED PROPERLY"
#endif
#if (  (CORE_SYS_PCIIO_SIZE-1) & CORE_SYS_PCIIO_BASE )
.error "CORE_SYS_PCIIO_SIZE NOT ALIGNED PROPERLY"
#endif
#if (  (CORE_SYS_IP1MEM_SIZE-1) & CORE_SYS_IP1MEM_BASE )
.error "CORE_SYS_IP1MEM_SIZE NOT ALIGNED PROPERLY"
#endif
#if (  (CORE_SYS_REG_SIZE-1) & CORE_SYS_REG_BASE )
.error "CORE_SYS_REG_SIZE NOT ALIGNED PROPERLY"
#endif
#if (  (CORE_SYS_PBCMEM_SIZE-1) & CORE_SYS_PBCMEM_BASE )
.error "CORE_SYS_PBCMEM_SIZE NOT ALIGNED PROPERLY"
#endif

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
 *		access_msc01
 *	Common function calling conventions, see init_core.S
 ************************************************************************/

LEAF( access_msc01 )
	
        li      t8, FUNC_INIT
	beq     t9, t8, msc01_init
	nop

        li      t8, FUNC_GET_PCIMEM_BASE
	beq     t9, t8, msc01_get_pcimem_base
	nop

        li      t8, FUNC_CONFIG_WRITE
	beq     t9, t8, msc01_config_write
	nop

        li      t8, FUNC_CONFIGURE_SDRAM
	beq     t9, t8, msc01_configure_sdram
	nop

        li      t8, FUNC_SETUP_DECODE
	beq     t9, t8, msc01_setup_decode
	nop

        /* FUNC_REMAP_PCI_IO */
	j	msc01_remap_pci_io
	nop

END( access_msc01 )


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/



/************************************************************************
 *
 *                          msc01_init
 *  Description :
 *  -------------
 *  Initialise msc01 just enough so that we can access PCI.
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
SLEAF(msc01_init)

        /*  We need to determine if we arrived here due to either :
	 *
	 *  1) A hardware or software reset.
	 *  2) Due to a "go bfc00000" command or under EJTAG control.
	 *
	 *  In case 2, we issue a software reset. We need to do
	 *  this since some devices will not otherwise be properly
	 *  reset.
	 *
	 *  We detect case 2 by checking if the System Controller Basic
	 *  Register Base address has been moved away from its reset value.
	 *  This is NOT the case after a hardware or software reset,
	 *  but IS otherwise the case since it is set below.
	 */

#define BIU_REGS			t2
	/* Check msc01 reset value of basic address decode */
	li	BIU_REGS, MSC01_REGADDR_RESET + \
		          MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	t1, MSC01_BIU_RBBASL_OFS(BIU_REGS)
	li	t0, PHYS(MSC01_REGADDR_RESET )		/* reset value */
	beq	t0, t1, 1f
	nop

	/* Request software reset */
	jr	ra
	li	v1, 1

1:
	/* Disable external sync cycles on 24K */
	MFC0(v0, C0_PRId)
	and	v0, M_PRIdImp
	srl	v0, S_PRIdImp
	xor	v0, K_PRIdImp_24K
	bnez	v0, 1f
	 nop
	
	/* Reset CONFIG7:bit8 to disable external sync cycles */
	MFC0_SEL_OPCODE(R_v0, 16, 7)
	and	v0,~(1<<8)
	MTC0_SEL_OPCODE( R_v0, 16, 7)
	
1:
	/* Move register block base address to 0x1bc00000 */
	/* Decrease size of IP3 space to avoid clash with reg-space */
	/* IP3 temporary decrease         - reset 0x18000000 */
	li	t0, -CORE_SYS_IP3MEM_SIZE		     /*-0x18400000 */
	sw	t0, MSC01_BIU_IP3MSK1L_OFS(BIU_REGS)
	/* Register block move            - reset 0x1f800000 */
	li	t0, CORE_SYS_REG_BASE      		     /* 0x1bc00000 */
	sw	t0, MSC01_BIU_RBBASL_OFS(BIU_REGS)           /*-0x1c000000 */
	sync
#undef BIU_REGS

#define PBC_REGS			t2
	/* Speed up peripheral bus controller with configuration: */
	/*       CDIV=1, WS=16, WSIDLE=3, ADM=0,                  */
	/*       CAT=1,  CDT=1, RAT=2,    RDT=0, WAT=2, WDT=0     */
	li	PBC_REGS, MSC01_PBC_REG_BASE
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

#define BIU_REGS			t2
	/* Setup msc01 basic address decode */
	li	BIU_REGS, MSC01_BIU_REG_BASE
	li	t0, CORE_SYS_MEMORY_BASE		     /* 0x00000000 */
	li	t1, -CORE_SYS_MEMORY_SIZE		     /*-0x10000000 */
	sw	t0, MSC01_BIU_MCBAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_MCMSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_MCBAS2L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_MCMSK2L_OFS(BIU_REGS)

	/* IP3 Peripheral Bus Controller  - reset 0x18000000 */
	li	t0, CORE_SYS_PBCMEM_BASE		     /* 0x1c000000 */
	li	t1, -CORE_SYS_PBCMEM_SIZE		     /*-0x20000000 */
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
	li	t0, CORE_SYS_IP1MEM_BASE10		     /* 0x1b800000 */
	li	t1, -CORE_SYS_IP1MEM_SIZE		     /*-0x1bc00000 */
	sw	t1, MSC01_BIU_IP1MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP1MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS2L_OFS(BIU_REGS)

	/* IP2 PCI                        - reset 0x14000000 */
	li	t0, CORE_SYS_PCIMEM_BASE		     /* 0x10000000 */
	li	t1, -CORE_SYS_PCIMEM_SIZE10		     /*-0x18000000 */
	sw	t0, MSC01_BIU_IP2BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP2MSK1L_OFS(BIU_REGS)
	li	t0, CORE_SYS_PCIIO_BASE			     /* 0x1b000000 */
	li	t1, -CORE_SYS_PCIIO_SIZE		     /*-0x1b800000 */
	sw	t1, MSC01_BIU_IP2MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS2L_OFS(BIU_REGS)
#undef BIU_REGS

#define PCI_REGS		t2
	li	PCI_REGS, MSC01_PCI_REG_BASE

        /* Setup PCI_SC2PMEM (0x10000000-0x17ffffff transparent) */
	li	t0, CORE_SYS_PCIMEM_BASE
	sw	t0, MSC01_PCI_SC2PMBASL_OFS(PCI_REGS)
	sw	t0, MSC01_PCI_SC2PMMAPL_OFS(PCI_REGS)
	li	t0, (-CORE_SYS_PCIMEM_SIZE10) & MSC01_PCI_SC2PMMSKL_MSK_MSK
	sw	t0, MSC01_PCI_SC2PMMSKL_OFS(PCI_REGS)
#undef PCI_REGS

	b	200f
	nop

#define BIU_REGS		t2
101:
	/* MSC01 version 01.01 and later */
	/* no clashing - prioritized compare: REGS > MEM > IP1 > IP2 > IP3 */
	/*        and prioritized pci spaces: SC2PCIIO > SC2PCIMEM         */
	/* IP1 not used                   - reset 0x10000000 */
	li	t0, CORE_SYS_IP1MEM_BASE		     /* 0x1bc00000 */
	li	t1, -CORE_SYS_IP1MEM_SIZE		     /*-0x1c000000 */
	sw	t1, MSC01_BIU_IP1MSK1L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP1MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP1BAS2L_OFS(BIU_REGS)

	/* IP2 PCI                        - reset 0x14000000 */
	li	t0, CORE_SYS_IP2MEM_BASE1		     /* 0x10000000 */
	li	t1, -CORE_SYS_IP2MEM_SIZE1		     /*-0x18000000 */
	sw	t0, MSC01_BIU_IP2BAS1L_OFS(BIU_REGS)
	sw	t1, MSC01_BIU_IP2MSK1L_OFS(BIU_REGS)
	li	t0, CORE_SYS_IP2MEM_BASE2		     /* 0x18000000 */
	li	t1, -CORE_SYS_IP2MEM_SIZE2		     /*-0x1c000000 */
	sw	t1, MSC01_BIU_IP2MSK2L_OFS(BIU_REGS)
	sw	t0, MSC01_BIU_IP2BAS2L_OFS(BIU_REGS)
#undef BIU_REGS

#define PCI_REGS		t2
	li	PCI_REGS, MSC01_PCI_REG_BASE

        /* Setup PCI_SC2PMEM (0x10000000-0x1fffffff transparent) */
	li	t0, CORE_SYS_PCIMEM_BASE
	sw	t0, MSC01_PCI_SC2PMBASL_OFS(PCI_REGS)
	sw	t0, MSC01_PCI_SC2PMMAPL_OFS(PCI_REGS)
	li	t0, (-CORE_SYS_PCIMEM_SIZE2N) & MSC01_PCI_SC2PMMSKL_MSK_MSK
	sw	t0, MSC01_PCI_SC2PMMSKL_OFS(PCI_REGS)
#undef PCI_REGS

200:
#define PCI_REGS		t2
	/* Setup PCI_SC2PIO (0x1b000000-0x1b7fffff -> 0x00000000-0x007fffff) */
	li	t0, CORE_SYS_PCIIO_BASE
	sw	t0, MSC01_PCI_SC2PIOBASL_OFS(PCI_REGS)
	sw	zero, MSC01_PCI_SC2PIOMAPL_OFS(PCI_REGS)
	li	t0, (-CORE_SYS_PCIIO_SIZE) & MSC01_PCI_SC2PIOMSKL_MSK_MSK
	sw	t0, MSC01_PCI_SC2PIOMSKL_OFS(PCI_REGS)

	/* Setup msc01 PCI_BAR0 to 256Mbyte memory window */
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
	sw	t0, MSC01_PCI_CFG_OFS(PCI_REGS)
	sync

	/* Done */
	move	v0, zero
	jr	ra
	move	v1, zero
#undef PCI_REGS

END(msc01_init)


/************************************************************************
 *
 *                          msc01_get_pcimem_base
 *  Description :
 *  -------------
 *  Return base address for PCI memory cycles.
 *
 ************************************************************************/
SLEAF(msc01_get_pcimem_base)

	li	v0, CORE_SYS_PCIMEM_BASE
	jr	ra
	nop

END(msc01_get_pcimem_base)


/************************************************************************
 *
 *                          msc01_config_write
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
SLEAF(msc01_config_write)

#define PCI_REGS		t2
	li	PCI_REGS, MSC01_PCI_REG_BASE
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
#undef PCI_REGS

END(msc01_config_write)


/************************************************************************
 *
 *                          msc01_configure_sdram
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
SLEAF(msc01_configure_sdram)

#define RA		s0
#define FREQ		s1
#define MC_REGS		s2
#define RAMSIZE		s3
#define CSNUM           s4
#define ROWS		s5
#define COLUMNS		s6
#define RA2		s7
#define ROWSB		a2
#define COLUMNSB	a3

	move	RA,	     ra
	move	FREQ,	     a0
/* Worst case setup based on frequency assumptions */
#define GCLK_SCALE_WORST_CASE 200 /*		(CORESYS_MAX_FREQ_MHZ) */

	move	RA, ra

	/********************************************
	** Configure IIC controller
	*/
	li	MC_REGS, MSC01_MC_REG_BASE
	li	t1, GCLK_SCALE_WORST_CASE
	sw	t1, MSC01_MC_SPD_CFG_OFS(MC_REGS)

	/* Read and check basic RAM type */
	jal	my_read_eeprom
	li	a0, SPD_FUNDAMENTAL_TYPE
	lw	t1, MSC01_MC_HC_DDR_OFS(MC_REGS)
	/* This bit reflects JP3 */
	andi	t1, MSC01_MC_HC_DDR_DDR_BIT
#ifdef SYSCTRL_DDR_SUPPORT
	bne	t1, zero, 1f
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
#endif
	li	t0, SPD_FUNDAMENTAL_TYPE_SDR
1:
	bne	t0, v0, error_sdram
	li	v0, ERROR_SPD

	/* Read basic data width - must be 0x0040 or 0x0048 (parity byte) */
	jal	my_read_eeprom
	li      a0, SPD_MODULE_WIDTH_LO
	or	v0, 8
	li	a0, 64 | 8
	bne	v0, a0, error_sdram
	li	v0, ERROR_SDRAM_WIDTH

	jal	my_read_eeprom
	li      a0, SPD_MODULE_WIDTH_HI
	bne	v0, zero, error_sdram
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

	sll	CONFIG0, ROWS, MSC01_MC_CFGPBx_ROWW_SHF
	or	CONFIG0, COLUMNS

	li	t0, 1
	beq	t0, CSNUM, 3f
	li	CONFIG1, 0
	li	t0, CORE_SYS_MEMORY_SIZE
	beq	t0, RAMSIZE, 3f
	nop

	/* Check and adjust rows, columns if necessary */
	/* CoreSYS: This is the second and last bank */
	/*          and may be set up smaller        */
	beq	ROWSB, zero, 1f
	slt	v0, ROWSB, ROWS
	beq	v0, zero, 1f
	nop
	/* ROWSB exists and is less than ROWS */
	move	ROWS, ROWSB
1:
	beq	COLUMNSB, zero, 1f
	slt	v0, COLUMNSB, COLUMNS
	beq	v0, zero, 1f
	nop
	/* COLUMNSB exists and is less than COLUMNS */
	move	COLUMNS, COLUMNSB
1:
	jal	checkrows
	nop

	add	RAMSIZE, v0
	sll	CONFIG1, ROWS, MSC01_MC_CFGPBx_ROWW_SHF
	or	CONFIG1, COLUMNS
3:
	/********************************************
	** Here starts writing to mc registers
	** Disable SDRAM
	*/
	sw	zero, MSC01_MC_CTRLENA_OFS(MC_REGS)

	/********************************************
	** Configure registers for physical banks 0-3
	*/
	sw	CONFIG0, MSC01_MC_CFGPB0_OFS(MC_REGS)
	sw	CONFIG1, MSC01_MC_CFGPB1_OFS(MC_REGS)
	sw	zero,    MSC01_MC_CFGPB2_OFS(MC_REGS)
	sw	zero,    MSC01_MC_CFGPB3_OFS(MC_REGS)

#undef CONFIG0
#undef CONFIG1

	/********************************************
	** Set up LATENCY register
	*/
#define ACCUM a3
	/* Read CAS Latency */
	jal	my_read_eeprom
	li	a0, SPD_CASLAT
/*
 * Note:
 * The present implementation is meant for an FPGA solution running up to 30MHz.
 * In hardened silicon, running faster, two things must be considered:
 * - Cut off frequencies for the suported cache latencies.
 * - Half ddr cache latencies requiring CLKRAT of 1:2 or 1:4 to work properly
 */

#ifdef SYSCTRL_DDR_SUPPORT
	 /* If CAS latency 2 is supported, use 2 otherwise 3 */
	lw	t1, MSC01_MC_HC_DDR_OFS(MC_REGS)
	andi	t1, MSC01_MC_HC_DDR_DDR_BIT
	beq	t1, zero, 1f
	andi	a0, v0, SPD_DDRCASLAT_2_0_BIT
	bne	a0, zero, 2f
	li	ACCUM, (2 << MSC01_MC_LATENCY_CL_SHF)

	 /* Use cache latency 3 as default */
	b	2f
	li	ACCUM, (3 << MSC01_MC_LATENCY_CL_SHF)
1:
#endif
	 /* If CAS latency 2 is supported, use 2 otherwise 3 */
	andi	a0, v0, SPD_CASLAT_2_BIT
	bne	a0, zero, 2f
	li	ACCUM, (2 << MSC01_MC_LATENCY_CL_SHF)
	li	ACCUM, (3 << MSC01_MC_LATENCY_CL_SHF)
2:
	/* Read CS Latency */
	jal	my_read_eeprom
	li	a0, SPD_CSLAT
	/* We only support CS latencies 0 and 1 */
	and	v0, SPD_CSLAT_0_BIT | SPD_CSLAT_1_BIT
	beq	v0, zero, config_error
	srl	v0, 1
	sll	v0, MSC01_MC_LATENCY_CSL_SHF
	or	ACCUM, v0

	/* Read Write Latency */
	jal	my_read_eeprom
	li	a0, SPD_WRLAT
	/* We only support WR latencies 0 and 1 */
	and	v0, SPD_WRLAT_0_BIT | SPD_WRLAT_1_BIT
	beq	v0, zero, config_error
	srl	v0, 1
	sll	v0, MSC01_MC_LATENCY_WL_SHF
	or	ACCUM, v0

	sw	ACCUM, MSC01_MC_LATENCY_OFS(MC_REGS)

	/********************************************
	** Set up IOCTL register
	*/
	sw	zero, MSC01_MC_IOCTRL_OFS(MC_REGS)

	/********************************************
	** Set up SDRAM command timing register
	*/

	/* Read Precharge time */
	jal	my_read_eeprom
	li	a0, SPD_TRP
#ifdef SYSCTRL_DDR_SUPPORT
	lw	t1, MSC01_MC_HC_DDR_OFS(MC_REGS)
	andi	t1, MSC01_MC_HC_DDR_DDR_BIT
	bnel	t1, zero, 1f
	srl	v0, 2   /* DDR - quarters of nanoseconds */
1:
#endif
	srl	v0, 3   /* 8 ns ~ 1 cycle at 125 MHz */
	sltiu	t0, v0, MSC01_MC_TIMPAR_TRP_MIN
	bnel	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRP_MIN
	sltiu   t0, v0, MSC01_MC_TIMPAR_TRP_MAX+1
	beql	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRP_MAX
1:
	sll	v0, MSC01_MC_TIMPAR_TRP_SHF
	move	ACCUM, v0

	/* Read RowActive time */
	jal	my_read_eeprom
	li	a0, SPD_TRAS
#ifdef SYSCTRL_DDR_SUPPORT
	lw	t1, MSC01_MC_HC_DDR_OFS(MC_REGS)
	andi	t1, MSC01_MC_HC_DDR_DDR_BIT
	bnel	t1, zero, 1f
	srl	v0, 2   /* DDR - quaters of nanoseconds */
1:
#endif
	srl	v0, 3   /* 8 ns ~ 1 cycle at 125 MHz */
	sltiu	t0, v0, MSC01_MC_TIMPAR_TRAS_MIN
	bnel	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRAS_MIN
	sltiu   t0, v0, MSC01_MC_TIMPAR_TRAS_MAX+1
	beql	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRAS_MAX
1:
	sll	v0, MSC01_MC_TIMPAR_TRAS_SHF
	or	ACCUM, v0

	/* Read RowToColcmd time */
	jal	my_read_eeprom
	li	a0, SPD_TRCD
#ifdef SYSCTRL_DDR_SUPPORT
	lw	t1, MSC01_MC_HC_DDR_OFS(MC_REGS)
	andi	t1, MSC01_MC_HC_DDR_DDR_BIT
	bnel	t1, zero, 1f
	srl	v0, 2   /* DDR - quaters of nanoseconds */
1:
#endif
	srl	v0, 3   /* 8 ns ~ 1 cycle at 125 MHz */
	sltiu	t0, v0, MSC01_MC_TIMPAR_TRCD_MIN
	bnel	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRCD_MIN
	sltiu   t0, v0, MSC01_MC_TIMPAR_TRCD_MAX+1
	beql	t0, zero, 1f
	li	v0, MSC01_MC_TIMPAR_TRCD_MAX
1:
	sll	v0, MSC01_MC_TIMPAR_TRCD_SHF
	or	ACCUM, v0

	or	ACCUM, (MSC01_MC_TIMPAR_TDPL_PC133 << MSC01_MC_TIMPAR_TDPL_SHF)
	sw	ACCUM, MSC01_MC_TIMPAR_OFS(MC_REGS)

	/********************************************
	** Set up SDRAM refresh time register
	*/
	jal	my_read_eeprom
	li	a0, SPD_RFSH_RT
	and	v0, SPD_RFSH_RT_RATE_MSK

	li	t0, SPD_RFSH_RT_RATE_125	/* 125 us */
	beq	t0, v0, 1f
	li	t0, 125
	li	t0, SPD_RFSH_RT_RATE_62_5	/* 62.5 us */
	beq	t0, v0, 1f
	li	t0, 62
	li	t0, SPD_RFSH_RT_RATE_31_3	/* 31.3 us */
	beq	t0, v0, 1f
	li	t0, 31
	li	t0, SPD_RFSH_RT_RATE_15_625	/* 15.6 us */
	beq	t0, v0, 1f
	li	t0, 15
	li	t0, SPD_RFSH_RT_RATE_7_8	/* 7.8 us */
	beq	t0, v0, 1f
	li	t0, 7
	li	t0, 3			/* assume 3.9 us */
1:
	multu	FREQ, t0
	mflo	t0

	/* Adjust for clock ratio. Exploit the values: */
	/*                1=1/1,  2=2/3,  3=1/2,  4=1/3,  5=1/4 */
	/* Decremented:   0=1/1,  1=2/3,  2=1/2,  3=1/3,  4=1/4 */
	lw	v0, MSC01_MC_HC_CLKRAT_OFS(MC_REGS)
	li	t1, 1
	beq	v0, t1, 2f /* branch taken for ratio 1/1 */
	sub	v0, t1
	bne	v0, t1, 1f /* branch taken for ratios 1/2, 1/3, 1/4 */
	nop
	add	t0, t0     /* ratio 2/3,  multiply by 2 */
	li	v0, 3
1:
	divu	t0, v0     /*             divide by v0 */
	mflo	t0
2:
	/* never go below a certain minimum count */
	sltiu	v0, t0, MSC01_MC_TREFRESH_TREF_MIN
	beq	v0, zero, 3f
	nop
	li	t0, MSC01_MC_TREFRESH_TREF_MIN
3:
	sw	t0, MSC01_MC_TREFRESH_OFS(MC_REGS)

	/********************************************
	** Set up PARITY register
        **  If SDRAM module supports parity or ECC, enable parity support.
        **  If SDRAM module does not support any of those, disable
        **  parity support.
	** On SEAD-2 parity must remain disabled because
	**  the SDRAM parity data pins are not connected.
        */
	li	t0, PRODUCT_SEAD2_ID
	beq	t0, k0, 1f
	li	t0, 0

        /* Read parity support */
        jal	my_read_eeprom
        li	a0, SPD_CONFIG_TYPE
        li	t0, SPD_CONFIG_TYPE_ECC
        beq	t0, v0, 2f
        li	t0, SPD_CONFIG_TYPE_PARITY
        bne	t0, v0, 1f
	li	t0, 0
2:
        li	t0, MSC01_MC_HC_PARITY_PARITY_BIT
1:
	sw	t0, MSC01_MC_HC_PARITY_OFS(MC_REGS)

	/********************************************
	** Initialize RAM through MC_INITCMD register
	*/

	/* set up v1 with CL aligned to suit the MRS command below */
	lw	v1, MSC01_MC_LATENCY_OFS(MC_REGS)
	and	v1, MSC01_MC_LATENCY_CL_MSK
	sll	v1, 4 - MSC01_MC_LATENCY_CL_SHF

#ifdef SYSCTRL_DDR_SUPPORT
	lw	t0, MSC01_MC_HC_DDR_OFS(MC_REGS)
	andi	t0, MSC01_MC_HC_DDR_DDR_BIT
	beq	t0, zero, 1f
	nop
	/* Dual data rate RAM */
	li	t0, 0x00070000;
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* NOP */
	li	t0, 0x00320400;
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* PAL */
#if 0 /* 66 MHz RAM clock frequency is the limit to disable/enable DLL */
	li	t0, 0x00301000
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* EMRS; enable DLL */
#else
	li	t0, 0x00301001
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* EMRS; disable DLL */
#endif
	li	t0, 0x00300121
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* MRS; reset DLL, CL=2,WT=linear,BL=2 */
	li	t0, 0x00320400
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* PAL */
	li	t0, 0x00910000
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */

	li	t0, 0x00300001; /* MRS; WT=linear,BL=2, CL=? */
	or	t0, v1
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* MRS */

	/* Give DLL time to syncronize (200 ram clock cycles) */
	lw	v0, MSC01_MC_HC_CLKRAT_OFS(MC_REGS)
	sll	v0, 7
3:
	bne	v0, zero, 3b
	sub	v0, 1
	b	2f
	nop

1:
#endif
	/* Single data rate RAM */
	li	t0, 0x00070000;
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* NOP */
	li	t0, 0x00320400;
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* PAL */
	li	t0, 0x00910000;
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* CBR */

	li	t0, 0x00300000; /* MRS; WT=linear,BL=1, CL=? */
	or	t0, v1
	sw	t0, MSC01_MC_INITCMD_OFS(MC_REGS) /* MRS */
2:

	/********************************************
	** Enable SDRAM
	*/
	li	t0, MSC01_MC_CTRLENA_ENA_BIT
	sw	t0, MSC01_MC_CTRLENA_OFS(MC_REGS)

	/********************************************
	** Another look for parity
	*/
	lw	t0, MSC01_MC_HC_PARITY_OFS(MC_REGS)
	and	t0, MSC01_MC_HC_PARITY_PARITY_BIT
	beq	t0, zero, 9f
	nop

	/********************************************
	** Parity is enabled, so all parity bytes
	** must be initialized by full data width
	** uncached writes to the whole sdram memory.
	*/
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
	sub	t0, 4
1:
	add	t0, 4
	bnel	t0, t1, 1b	/* does not store on limit addr */
	sw	zero, 0(t0)
	b	9f
	nop

	/* 64 bit CPU */
SET_MIPS3()
2:	sub	t0, 8
1:	add	t0, 8
	bnel	t0, t1, 1b
	sd	zero, 0(t0)
SET_MIPS0()
9:

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

#define BIU_REGS t2
	/* correct size for msc01 configured buswidth */
	li	BIU_REGS, MSC01_BIU_REG_BASE
	lw	v0, MSC01_SC_ID_OFS(BIU_REGS)
	and	v0, MSC01_SC_ID_ID_MSK
	srl	v0, MSC01_SC_ID_ID_SHF
	/* explicitly check for configurations that use 64 bit data buses */
	xor	v0, MSC01_ID_SC_EC64
	beqz	v0, 1f
	 xor    v0, MSC01_ID_SC_EC64^MSC01_ID_SC_MGB
	beqz	v0, 1f
	 xor    v0, MSC01_ID_SC_MGB^MSC01_ID_SC_MGBIIA36D64C0IO
	beqz	v0, 1f
	 xor    v0, MSC01_ID_SC_MGBIIA36D64C0IO^MSC01_ID_SC_OCP
	beqz	v0,1f
	 nop
	
	sub	t0, 1	/* can only access half of the 64bit memory */
1:
#undef BIU_REGS
	
	/* correct size for data width */
	lw	v0, MSC01_MC_HC_FMDW_OFS(MC_REGS) /* check data with */
	and	v0, MSC01_MC_HC_FMDW_FMDW_BIT
	bne	v0, zero, 1f
	li	v0, 1
	sub	t0, 1		/* FMDW_BIT=0  =>  half data width */
1:
	sll	v0, t0	/* return size in bytes */

	/* CoreSYS - limit ram size to max 256 Mbyte */
	li	t0, CORE_SYS_MEMORY_SIZE
	slt	t0, t0, v0
	bne	t0, zero, 2b
	nop

	jr	ra
	nop

/************************************************************
** local subroutine - read from spd eeprom.
** makes a master return in case of error.
** Requires MC_REGS to be defined and set up.
** Returns byte read in v0.
*/

my_read_eeprom:
	li	v0, PRODUCT_ATLASA_ID
	bne	v0, k0, 1f
	nop

	/* Atlas - the SAA9730 must be used as master */
	/* Changes a0, a1, at, t0, t1, t2, t3, t5, t6, t7, t8, t9, v0, v1, RA2 */
	move	RA2, ra
	jal	sys_spd_read
	nop
	bne	v0, zero, error_msc01_eeprom
	nop
	jr	RA2
	move	v0, v1
error_msc01_eeprom:
	jr	RA
	nop
1:
	/* Malta */
	/* Changes a0, v0. */
	/* Setup address: on CoreSystem always use Device Serial Address 0 */
	sw	a0, MSC01_MC_SPD_ADR_OFS(MC_REGS)
1:
	/* Poll for ready */
	lw	v0, MSC01_MC_SPD_DAT_OFS(MC_REGS)
	andi	a0, v0, MSC01_MC_SPD_DAT_BUSY_MSK
	bne	a0, zero, 1b
	nop

	/* Check read err flag */
	andi	a0, v0, MSC01_MC_SPD_DAT_RDERR_MSK
	beq	a0, zero, eeprom_done
	nop

	/* Error */
	jr	RA
	li	v0, ERROR_SPD

eeprom_done:
	jr	ra
	andi	v0, MSC01_MC_SPD_DAT_RDATA_MSK

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

END(msc01_configure_sdram)


/************************************************************************
 *
 *                          msc01_setup_decode
 *  Description :
 *  -------------
 *  Setup msc01 memory decoding (except for SDRAM)
 *   - already done for MSC01.
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
SLEAF(msc01_setup_decode)

	jr	ra
	move    v0, zero

END(msc01_setup_decode)


/************************************************************************
 *
 *                          msc01_remap_pci_io
 *  Description :
 *  -------------
 *  Remap PCI IO range so that PCI IO range starts at address 0 on
 *  PCI - already does for MSC01.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
SLEAF(msc01_remap_pci_io)

	jr	ra
	move    v0, zero

END(msc01_remap_pci_io)




/* Messages */

MSG( msg_pram_clr,   "PRAM_CLR" )

