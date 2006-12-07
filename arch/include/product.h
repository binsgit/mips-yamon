
/************************************************************************
 *
 *  product.h
 *
 *  Product IDs
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


#ifndef PRODUCT_H
#define PRODUCT_H


/************************************************************************
 *  Include files
 ************************************************************************/

/************************************************************************
 *  Definitions
*************************************************************************/

/************************************************************************
 *  REVISION: MUST BE FIXED TO  0x1FC00010 on any baseboard
 *  This register is reserved for storing the product ID of the
 *  baseboard. Below fields are the generic fields available on all
 *  boards. For board specific fields, see the corresponding board
 *  header file (e.g. atlas.h)
 *
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define MIPS_REVISION              0x1FC00010 /* REVISION              */

/* field: PROID */
#define MIPS_REVISION_PROID_SHF	   4
#define MIPS_REVISION_PROID_MSK	   (MSK(4) << MIPS_REVISION_PROID_SHF)

/* field: PRORV */
#define MIPS_REVISION_PRORV_SHF	   0
#define MIPS_REVISION_PRORV_MSK	   (MSK(4) << MIPS_REVISION_PRORV_SHF)

/* field: MANID (in case of 3rd party product) */
#define MIPS_REVISION_MANID_SHF	   8
#define MIPS_REVISION_MANID_MSK    (MSK(8) << MIPS_REVISION_MANID_SHF)

/* field: MANPD (in case of 3rd party product) */
#define MIPS_REVISION_MANPD_SHF	   16
#define MIPS_REVISION_MANPD_MSK    (MSK(8) << MIPS_REVISION_MANPD_SHF)

/* field: CORID (Atlas/Malta) */
#define MIPS_REVISION_CORID_SHF    10
#define MIPS_REVISION_CORID_MSK	   (MSK(6) << MIPS_REVISION_CORID_SHF)

/* field: CORRV (Atlas/Malta) */
#define MIPS_REVISION_CORRV_SHF    8
#define MIPS_REVISION_CORRV_MSK    (MSK(2) << MIPS_REVISION_CORRV_SHF)

/**** Encoding of corid field ****/

#define MIPS_REVISION_CORID_QED_RM5261     0
#define MIPS_REVISION_CORID_CORE_LV        1
#define MIPS_REVISION_CORID_BONITO64	   2
#define MIPS_REVISION_CORID_CORE_20K	   3
#define MIPS_REVISION_CORID_CORE_FPGA      4
#define MIPS_REVISION_CORID_CORE_SYS       5
#define MIPS_REVISION_CORID_CORE_EMUL      6
#define MIPS_REVISION_CORID_CORE_FPGA2     7
#define MIPS_REVISION_CORID_CORE_FPGAr2    8
#define MIPS_REVISION_CORID_CORE_FPGA3     9
#define MIPS_REVISION_CORID_CORE_24K       10

/**** Artificial corid defines ****/
/*
 *  CoreEMUL with   Bonito   System Controller is treated like a Core20K
 *  CoreEMUL with SOC-it 101 System Controller is treated like a CoreSYS
 *  CoreEMUL with ROC-it System Controller is treated like a CoreSYS
 */
#define MIPS_REVISION_CORID_CORE_EMUL_20K  0x63
#define MIPS_REVISION_CORID_CORE_EMUL_SYS  0x65
/*
 *  To ease code sharing with CoreSYS, this value is stored to
 *  variable sys_corecard when MIPS SOC-it 101 is used on SEAD:
 */
#define MIPS_REVISION_CORID_SEAD_MSC01 0x305

/* Following value used if no core card is available */
#define MIPS_REVISION_CORID_NA	   0xffffffff

/* Encoding of proid field */
#define PRODUCT_ATLASA_ID		 0x0		/* Atlas board   */
#define PRODUCT_SEAD_ID			 0x1		/* SEAD board    */
#define PRODUCT_MALTA_ID		 0x2		/* Malta board   */
#define PRODUCT_SEAD2_ID		 0x3		/* SEAD-2	 */
#define PRODUCT_THIRD_PARTY_ID		 0xe		/* Third party	 */
#define PRODUCT_UNKNOWN			 0xffffffff

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/


#endif /* #ifndef PRODUCT_H */

