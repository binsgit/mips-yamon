
/************************************************************************
 *
 *  init_core_s.S
 *
 *  Core card specific startup code (assembler)
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
#include <product.h>
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
 *
 *                          sys_core_init
 *  Description :
 *  -------------
 *
 *  Initialise North bridge just enough so that we can access PCI.
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
 *  None
 *
 ************************************************************************/
LEAF(sys_core_init)

	b	access_core
	li	t9, FUNC_INIT
	
END(sys_core_init)	


/************************************************************************	
 *
 *                          sys_core_get_pcimem_base
 *  Description :
 *  -------------
 *  Return base address for PCI memory cycles.
 *
 *  Parameters :
 *  ------------
 *  None
 *
 *  Return values :
 *  ---------------
 *  v0 = Base address for PCI memory cycles
 *
 ************************************************************************/
LEAF(sys_core_get_pcimem_base)

	b	access_core
	li	t9, FUNC_GET_PCIMEM_BASE
	
END(sys_core_get_pcimem_base)	


/************************************************************************	
 *
 *                          sys_core_config_write
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
 *
 *  Return values :
 *  ---------------
 *  v0 = 0 if OK
 *  v0 = ERROR_NB_CONFIG if not OK
 *
 ************************************************************************/
LEAF(sys_core_config_write)

	b	access_core
	li	t9, FUNC_CONFIG_WRITE

END(sys_core_config_write)	
	

/************************************************************************	
 *
 *                          sys_core_configure_sdram
 *  Description :
 *  -------------
 *  Setup North bridge SDRAM configuration
 *
 *  Parameters :
 *  ------------
 *  a0 = Worst case (lowest) bus freq. (MHz) for setting timing parms.
 *  a1 = Base address to be used for access to North Bridge registers.
 *  a2 = Max SDRAM size supported by platform (may be ignored by some
 *       North Bridges).
 *
 *  Return values :
 *  ---------------
 *  v0 = error code (0 = OK)
 *  v1 = RAM size
 *
 ************************************************************************/
LEAF(sys_core_configure_sdram)

	b	access_core
	li	t9, FUNC_CONFIGURE_SDRAM
	
END(sys_core_configure_sdram)	


/************************************************************************	
 *
 *                          sys_core_setup_decode
 *  Description :
 *  -------------
 *  Setup North Bridge memory decoding (except for SDRAM).
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
 *  0 :				No error.
 *  ERROR_NB_DECODE :		Illegal ranges requested.
 *
 ************************************************************************/
LEAF(sys_core_setup_decode)

	b	access_core
	li	t9, FUNC_SETUP_DECODE
	
END(sys_core_setup_decode)	


/************************************************************************	
 *
 *                          sys_core_remap_pci_io
 *  Description :
 *  -------------
 *  Remap PCI IO range so that PCI IO range starts at address 0 on
 *  PCI (North Bridge no longer transparent for PCI IO).
 *
 *  Parameters :
 *  ------------
 *  a0 = Base address to be used for access to North Bridge registers.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF(sys_core_remap_pci_io)

	b	access_core
	li	t9, FUNC_REMAP_PCI_IO
	
END(sys_core_remap_pci_io)	
	
	
/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************	
 *		access_core
 ************************************************************************/
SLEAF(access_core)

	/* Determine core board */
	li	t7, HIKSEG1(MIPS_REVISION)
	lw	t7, LO_OFFS(MIPS_REVISION)(t7)
	
	.set	push
	.set	noat
	/* Check for SocitSC */
	srl	AT, t7, MIPS_REVISION_SCON_SHF
	andi	AT, MALTA_REVISION_SCON_MSK >> MALTA_REVISION_SCON_SHF
	beqz	AT, 1f
	 srl	t7, MIPS_REVISION_CORID_SHF
	.set	pop

	j	access_socitsc
	 nop
	
1:
	and	t7, MIPS_REVISION_CORID_MSK >> MIPS_REVISION_CORID_SHF

#define X(this, last, type) \
	xori	t7, MIPS_REVISION_CORID_##last^MIPS_REVISION_CORID_##this; \
	beqz	t7, goto_##type
#define MIPS_REVISION_CORID_0	0
	X(0,		QED_RM5261,	gt64120)
	X(QED_RM5261,	CORE_LV,	gt64120)
	X(CORE_LV,	BONITO64,	bonito64)
	X(BONITO64,	CORE_20K,	core_20k) /* QED/20K with Bonito */
	X(CORE_20K,	CORE_FPGA,	gt64120)
	X(CORE_FPGA,	CORE_SYS,	msc01)
	X(CORE_SYS,	CORE_EMUL,	coreemul)
	X(CORE_EMUL,	CORE_FPGA2,	msc01)
	X(CORE_FPGA2,	CORE_FPGAr2,	gt64120)
	X(CORE_FPGAr2,	CORE_FPGA3,	msc01)
	X(CORE_FPGA3,	CORE_24K,	msc01)
	X(CORE_24K,	CORE_FPGA4,	msc01)
	X(CORE_FPGA4,	CORE_FPGA5,	msc01)
	 nop
#undef X
#undef MIPS_REVISION_CORID_0

	/* Unknown core card */
	jr	ra
	li	v0, ERROR_CORE_UNKNOWN

goto_gt64120:
	j	access_gt64120
	 nop
	
goto_bonito64:
	j	access_bonito64
	 nop
	
goto_core_20k:
	j	access_core_20k
	 nop
	
goto_msc01:
	j	access_msc01
	 nop

goto_coreemul:
	/* CoreEmul requires another check for presence of */
	/* SOC-it 101, ROC-it or Bonito System Controller   */
	li	t8, MSC01_REGADDR_RESET + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	t8, MSC01_BIU_RBBASL_OFS(t8)
	srl	t8, 16
	xor	t8, PHYS(MSC01_REGADDR_RESET) >> 16  /* 0x1f80  reset value */
	bnez	t8, 3f
	 nop
	/* CoreEMUL with SOC-it 101 in reset state */
	j	access_msc01
	 nop
3:
	/* CoreEMUL with Bonito System Controller will fall through !  */
	li	t8, MSC01_REGADDR_BASE + \
		         MSC01_REGSPACE_BIU * MSC01_REGADDR_SIZE
	lw	t8, MSC01_BIU_RBBASL_OFS(t8)
	srl	t8, 16
	xor	t8, PHYS(MSC01_REGADDR_BASE) >> 16  /* 0x1bc0  work value */
	bnez	t8, 2f
	 nop
	/* CoreEMUL with SOC-it 101 in work state, or ROC-it in either */
	/* reset or work state.                                        */
	j	access_msc01
	 nop
2:
	/* CoreEMUL with Bonito System Controller */
	j	access_core_20k
	 nop
	
END(access_core)

