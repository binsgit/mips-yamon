/* ##########################################################################
 *
 *  Default memory map
 *
 * ##########################################################################
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
 * ###########################################################################
 */

#ifndef MSC01_MEMMAP_H
#define MSC01_MEMMAP_H

#ifndef KSEG1_BASE
# define KSEG0_BASE      	0x80000000	/* unmapped, cachable	*/
# define KSEG1_BASE      	0xa0000000	/* unmapped, uncached	*/
#endif

/*****************************************************************************
 * IPIF generic register space addresses, unmapped & uncached access via kseg1
 ****************************************************************************/

#define	MSC01_REGSPACE_MC	0
#define	MSC01_REGSPACE_IC	1
#define	MSC01_REGSPACE_BIU	2
#define	MSC01_REGSPACE_IP1	3
#define	MSC01_REGSPACE_IP2	4
#define	MSC01_REGSPACE_IP3	5

#define	MSC01_REGADDR_RESET	(KSEG1_BASE + 0x1f800000) /* power-up value */
#define	MSC01_REGADDR_SIZE	0x00040000		  /* 256 KByte	    */
#define	MSC01_REGADDR(regspace)	(MSC01_REGADDR_BASE	\
				 +(regspace)*MSC01_REGADDR_SIZE)



/*****************************************************************************
 * Default register base addresses (can be modified at run-time by software)
 ****************************************************************************/

#define	MSC01_REGADDR_BASE	(KSEG1_BASE + 0x1bc00000) /* programmed val */

#define MSC01_MC_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_MC)
#define MSC01_IC_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IC)
#define MSC01_BIU_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_BIU)
#define MSC01_SC_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_BIU)
#define MSC01_IP1_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP1)
#define MSC01_IP2_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP2)
#define MSC01_IP3_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP3)


/*****************************************************************************
 * Power-up memory space (can be modified at run-time by software)
 ****************************************************************************/

#define	MSC01_PHYSBASE_MC	0x00000000
#define	MSC01_PHYSBASE_IP1	0x10000000
#define	MSC01_PHYSBASE_IP2	0x14000000
#define	MSC01_PHYSBASE_IP3	0x18000000

#define	MSC01_KSEG0BASE_MC	(KSEG0_BASE + MSC01_PHYSBASE_MC)
#define	MSC01_KSEG0BASE_IP1	(KSEG0_BASE + MSC01_PHYSBASE_IP1)
#define	MSC01_KSEG0BASE_IP2	(KSEG0_BASE + MSC01_PHYSBASE_IP2)
#define	MSC01_KSEG0BASE_IP3	(KSEG0_BASE + MSC01_PHYSBASE_IP3)

#endif

