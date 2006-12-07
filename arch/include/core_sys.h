/************************************************************************
 *
 *  core_sys.h
 *
 *  Definitions for Malta core card with MIPS SOC-it 101 or ROC-it
 *  system controller
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

#ifndef CORE_SYS_H
#define CORE_SYS_H

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <product.h>
#include <malta.h>

#include <msc01_memmap.h>
#include <msc01_biu.h>
#include <msc01_ic.h>
#include <msc01_pci.h>
#include <msc01_pbc.h>
#include <msc01_mc.h>

#include <mc_reginit.h>
#include <mc_dendefine.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* The following mapping matches the msc01_system1 testbench used in
 * the SOC-it deliverables.  In this testbench, the IP blocks are attached
 * to the kernel as follows:
 *
 *   IP1:  "Testbench Monitor", not used in a real system
 *   IP2:  AHB bridge, PCI bridge, or nothing, depending on testbench build
 *   IP3:  Peripheral bus bridge.
 *
 * This configuration is reflected by the defines below:
 */
#define MSC01_AHB_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP2)
#define MSC01_PCI_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP2)
#define MSC01_PBC_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP3)

/* Definitions used for configuration of misc. timing parameters
 */
#ifndef MSC01_MC_TIMPAR_TDPL_PC133
#define MSC01_MC_TIMPAR_TDPL_PC133	2
#endif

#ifndef MSC01_MC_TREFRESH_TREF_MIN
#define MSC01_MC_TREFRESH_TREF_MIN	100
#endif

/* Physical address setup for core-sys */

#define CORE_SYS_MEMORY_BASE	MALTA_SYSTEMRAM_BASE  /* MEM */
#define CORE_SYS_MEMORY_SIZE	MALTA_SYSTEMRAM_SIZE

#define CORE_SYS_PCIMEM_BASE	MALTA_PCIMEM1_BASE  /* == IP2 */
#define CORE_SYS_PCIMEM_SIZE10	0x08000000  /* effective size, msc01 v1.0 */
#define CORE_SYS_PCIMEM_SIZE2N	0x10000000  /* "aligned" size */
#define CORE_SYS_PCIIO_BASE	0x1b000000  /* 16Mbyte natural alignment (IP2) */
#define CORE_SYS_PCIIO_SIZE	0x00800000
#define CORE_SYS_IP2MEM_BASE1	0x10000000
#define CORE_SYS_IP2MEM_SIZE1	0x08000000
#define CORE_SYS_IP2MEM_BASE2	0x18000000
#define CORE_SYS_IP2MEM_SIZE2	0x04000000

#define CORE_SYS_IP1MEM_BASE	0x1bc00000 /* IP1 (unused - put behind regbase) */
#define CORE_SYS_IP1MEM_BASE10	0x1b800000 /* IP1 (msc01 v1.0) */
#define CORE_SYS_IP1MEM_SIZE	0x00400000
 
#define CORE_SYS_REG_BASE	0x1bc00000
#define CORE_SYS_REG_SIZE	0x00400000

#define CORE_SYS_PBCMEM_BASE	0x1c000000  /* IP3 */
#define CORE_SYS_PBCMEM_SIZE	0x04000000
#define CORE_SYS_IP3MEM_SIZE	0x00400000  /* temporary size during start up */


/* PCI Configuration  Registers */

#define CORE_SYS_PCI_REG(x)	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD0_OFS + (x) + (x))
#define CORE_SYS_PCIDID		CORE_SYS_PCI_REG(0x00)
#define CORE_SYS_PCICMD		CORE_SYS_PCI_REG(0x04)
#define CORE_SYS_PCICLASS	CORE_SYS_PCI_REG(0x08)
#define CORE_SYS_PCILTIMER	CORE_SYS_PCI_REG(0x0c)
#define CORE_SYS_PCIBASE0	CORE_SYS_PCI_REG(0x10)
#define CORE_SYS_PCIINT		CORE_SYS_PCI_REG(0x3c)


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/

#endif /* #ifndef CORE_SYS_H */
