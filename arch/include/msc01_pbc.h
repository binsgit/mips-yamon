/* ##########################################################################
 *
 *  Peripheral Bus Controller Header File
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

#ifndef MSC01_PBC_H
#define MSC01_PBC_H

/* PBC ID in MSC01_PBC_ID */
#define MSC01_ID_PBC	                0x80

/*****************************************************************************
 * Register offset addresses and access types
 ****************************************************************************/

#define MSC01_PBC_ID_OFS		0x0000
#define MSC01_PBC_CLKCFG_OFS		0x0100
#define MSC01_PBC_CS0BASL_OFS		0x0200
#define MSC01_PBC_CS1BASL_OFS		0x0210
#define MSC01_PBC_CS2BASL_OFS		0x0220
#define MSC01_PBC_CS3BASL_OFS		0x0230
#define MSC01_PBC_CS0MSKL_OFS		0x0300
#define MSC01_PBC_CS1MSKL_OFS		0x0310
#define MSC01_PBC_CS2MSKL_OFS		0x0320
#define MSC01_PBC_CS3MSKL_OFS		0x0330
#define MSC01_PBC_CS0CFG_OFS		0x0400
#define MSC01_PBC_CS1CFG_OFS		0x0410
#define MSC01_PBC_CS2CFG_OFS		0x0420
#define MSC01_PBC_CS3CFG_OFS		0x0430
#define MSC01_PBC_CS0TIM_OFS		0x0500
#define MSC01_PBC_CS1TIM_OFS		0x0510
#define MSC01_PBC_CS2TIM_OFS		0x0520
#define MSC01_PBC_CS3TIM_OFS		0x0530
#define MSC01_PBC_CS0RW_OFS		0x0600
#define MSC01_PBC_CS1RW_OFS		0x0610
#define MSC01_PBC_CS2RW_OFS		0x0620
#define MSC01_PBC_CS3RW_OFS		0x0630

/*****************************************************************************
 * Register encodings
 ****************************************************************************/

#define MSC01_PBC_ID_ID_SHF		16
#define MSC01_PBC_ID_ID_MSK		0x00ff0000
#define MSC01_PBC_ID_MAR_SHF		8
#define MSC01_PBC_ID_MAR_MSK		0x0000ff00
#define MSC01_PBC_ID_MIR_SHF		0
#define MSC01_PBC_ID_MIR_MSK		0x000000ff

#define MSC01_PBC_CLKCFG_SHF		0
#define MSC01_PBC_CLKCFG_MSK		0x0000001f

#define MSC01_PBC_CSxBASL_BASL_SHF		22
#define MSC01_PBC_CSxBASL_BASL_MSK		0x3fc00000
#define MSC01_PBC_CS0BASL_BASL_SHF		MSC01_PBC_CSxBASL_BASL_SHF
#define MSC01_PBC_CS0BASL_BASL_MSK		MSC01_PBC_CSxBASL_BASL_MSK
#define MSC01_PBC_CS1BASL_BASL_SHF		MSC01_PBC_CSxBASL_BASL_SHF
#define MSC01_PBC_CS1BASL_BASL_MSK		MSC01_PBC_CSxBASL_BASL_MSK
#define MSC01_PBC_CS2BASL_BASL_SHF		MSC01_PBC_CSxBASL_BASL_SHF
#define MSC01_PBC_CS2BASL_BASL_MSK		MSC01_PBC_CSxBASL_BASL_MSK
#define MSC01_PBC_CS3BASL_BASL_SHF		MSC01_PBC_CSxBASL_BASL_SHF
#define MSC01_PBC_CS3BASL_BASL_MSK		MSC01_PBC_CSxBASL_BASL_MSK

#define MSC01_PBC_CSxMSKL_MSKL_SHF		22
#define MSC01_PBC_CSxMSKL_MSKL_MSK		0x0fc00000
#define MSC01_PBC_CS0MSKL_MSKL_SHF		MSC01_PBC_CSxMSKL_MSKL_SHF
#define MSC01_PBC_CS0MSKL_MSKL_MSK		MSC01_PBC_CSxMSKL_MSKL_MSK
#define MSC01_PBC_CS1MSKL_MSKL_SHF		MSC01_PBC_CSxMSKL_MSKL_SHF
#define MSC01_PBC_CS1MSKL_MSKL_MSK		MSC01_PBC_CSxMSKL_MSKL_MSK
#define MSC01_PBC_CS2MSKL_MSKL_SHF		MSC01_PBC_CSxMSKL_MSKL_SHF
#define MSC01_PBC_CS2MSKL_MSKL_MSK		MSC01_PBC_CSxMSKL_MSKL_MSK
#define MSC01_PBC_CS3MSKL_MSKL_SHF		MSC01_PBC_CSxMSKL_MSKL_SHF
#define MSC01_PBC_CS3MSKL_MSKL_MSK		MSC01_PBC_CSxMSKL_MSKL_MSK

#define MSC01_PBC_CSxCFG_ADM_SHF		20
#define MSC01_PBC_CSxCFG_ADM_MSK		0x00100000
#define MSC01_PBC_CSxCFG_ADM_BIT		0x00100000
#define MSC01_PBC_CSxCFG_DTYP_SHF		16
#define MSC01_PBC_CSxCFG_DTYP_MSK		0x00030000
#define MSC01_PBC_CSxCFG_WSIDLE_SHF		8
#define MSC01_PBC_CSxCFG_WSIDLE_MSK		0x00001f00
#define MSC01_PBC_CSxCFG_WS_SHF			0
#define MSC01_PBC_CSxCFG_WS_MSK			0x0000001f
#define MSC01_PBC_CS0CFG_ADM_SHF		MSC01_PBC_CSxCFG_ADM_SHF
#define MSC01_PBC_CS0CFG_ADM_MSK		MSC01_PBC_CSxCFG_ADM_MSK
#define MSC01_PBC_CS0CFG_ADM_BIT		MSC01_PBC_CSxCFG_ADM_BIT
#define MSC01_PBC_CS0CFG_DTYP_SHF		MSC01_PBC_CSxCFG_DTYP_SHF
#define MSC01_PBC_CS0CFG_DTYP_MSK		MSC01_PBC_CSxCFG_DTYP_MSK
#define MSC01_PBC_CS0CFG_WSIDLE_SHF		MSC01_PBC_CSxCFG_WSIDLE_SHF
#define MSC01_PBC_CS0CFG_WSIDLE_MSK		MSC01_PBC_CSxCFG_WSIDLE_MSK
#define MSC01_PBC_CS0CFG_WS_SHF			MSC01_PBC_CSxCFG_WS_SHF
#define MSC01_PBC_CS0CFG_WS_MSK			MSC01_PBC_CSxCFG_WS_MSK
#define MSC01_PBC_CS1CFG_ADM_SHF		MSC01_PBC_CSxCFG_ADM_SHF
#define MSC01_PBC_CS1CFG_ADM_MSK		MSC01_PBC_CSxCFG_ADM_MSK
#define MSC01_PBC_CS1CFG_ADM_BIT		MSC01_PBC_CSxCFG_ADM_BIT
#define MSC01_PBC_CS1CFG_DTYP_SHF		MSC01_PBC_CSxCFG_DTYP_SHF
#define MSC01_PBC_CS1CFG_DTYP_MSK		MSC01_PBC_CSxCFG_DTYP_MSK
#define MSC01_PBC_CS1CFG_WSIDLE_SHF		MSC01_PBC_CSxCFG_WSIDLE_SHF
#define MSC01_PBC_CS1CFG_WSIDLE_MSK		MSC01_PBC_CSxCFG_WSIDLE_MSK
#define MSC01_PBC_CS1CFG_WS_SHF			MSC01_PBC_CSxCFG_WS_SHF
#define MSC01_PBC_CS1CFG_WS_MSK			MSC01_PBC_CSxCFG_WS_MSK
#define MSC01_PBC_CS2CFG_ADM_SHF		MSC01_PBC_CSxCFG_ADM_SHF
#define MSC01_PBC_CS2CFG_ADM_MSK		MSC01_PBC_CSxCFG_ADM_MSK
#define MSC01_PBC_CS2CFG_ADM_BIT		MSC01_PBC_CSxCFG_ADM_BIT
#define MSC01_PBC_CS2CFG_DTYP_SHF		MSC01_PBC_CSxCFG_DTYP_SHF
#define MSC01_PBC_CS2CFG_DTYP_MSK		MSC01_PBC_CSxCFG_DTYP_MSK
#define MSC01_PBC_CS2CFG_WSIDLE_SHF		MSC01_PBC_CSxCFG_WSIDLE_SHF
#define MSC01_PBC_CS2CFG_WSIDLE_MSK		MSC01_PBC_CSxCFG_WSIDLE_MSK
#define MSC01_PBC_CS2CFG_WS_SHF			MSC01_PBC_CSxCFG_WS_SHF
#define MSC01_PBC_CS2CFG_WS_MSK			MSC01_PBC_CSxCFG_WS_MSK
#define MSC01_PBC_CS3CFG_ADM_SHF		MSC01_PBC_CSxCFG_ADM_SHF
#define MSC01_PBC_CS3CFG_ADM_MSK		MSC01_PBC_CSxCFG_ADM_MSK
#define MSC01_PBC_CS3CFG_ADM_BIT		MSC01_PBC_CSxCFG_ADM_BIT
#define MSC01_PBC_CS3CFG_DTYP_SHF		MSC01_PBC_CSxCFG_DTYP_SHF
#define MSC01_PBC_CS3CFG_DTYP_MSK		MSC01_PBC_CSxCFG_DTYP_MSK
#define MSC01_PBC_CS3CFG_WSIDLE_SHF		MSC01_PBC_CSxCFG_WSIDLE_SHF
#define MSC01_PBC_CS3CFG_WSIDLE_MSK		MSC01_PBC_CSxCFG_WSIDLE_MSK
#define MSC01_PBC_CS3CFG_WS_SHF			MSC01_PBC_CSxCFG_WS_SHF
#define MSC01_PBC_CS3CFG_WS_MSK			MSC01_PBC_CSxCFG_WS_MSK

#define MSC01_PBC_CSxTIM_CDT_SHF		8
#define MSC01_PBC_CSxTIM_CDT_MSK		0x00001f00
#define MSC01_PBC_CSxTIM_CAT_SHF		0
#define MSC01_PBC_CSxTIM_CAT_MSK		0x0000001f
#define MSC01_PBC_CS0TIM_CDT_SHF		MSC01_PBC_CSxTIM_CDT_SHF
#define MSC01_PBC_CS0TIM_CDT_MSK		MSC01_PBC_CSxTIM_CDT_MSK
#define MSC01_PBC_CS0TIM_CAT_SHF		MSC01_PBC_CSxTIM_CAT_SHF
#define MSC01_PBC_CS0TIM_CAT_MSK		MSC01_PBC_CSxTIM_CAT_MSK
#define MSC01_PBC_CS1TIM_CDT_SHF		MSC01_PBC_CSxTIM_CDT_SHF
#define MSC01_PBC_CS1TIM_CDT_MSK		MSC01_PBC_CSxTIM_CDT_MSK
#define MSC01_PBC_CS1TIM_CAT_SHF		MSC01_PBC_CSxTIM_CAT_SHF
#define MSC01_PBC_CS1TIM_CAT_MSK		MSC01_PBC_CSxTIM_CAT_MSK
#define MSC01_PBC_CS2TIM_CDT_SHF		MSC01_PBC_CSxTIM_CDT_SHF
#define MSC01_PBC_CS2TIM_CDT_MSK		MSC01_PBC_CSxTIM_CDT_MSK
#define MSC01_PBC_CS2TIM_CAT_SHF		MSC01_PBC_CSxTIM_CAT_SHF
#define MSC01_PBC_CS2TIM_CAT_MSK		MSC01_PBC_CSxTIM_CAT_MSK
#define MSC01_PBC_CS3TIM_CDT_SHF		MSC01_PBC_CSxTIM_CDT_SHF
#define MSC01_PBC_CS3TIM_CDT_MSK		MSC01_PBC_CSxTIM_CDT_MSK
#define MSC01_PBC_CS3TIM_CAT_SHF		MSC01_PBC_CSxTIM_CAT_SHF
#define MSC01_PBC_CS3TIM_CAT_MSK		MSC01_PBC_CSxTIM_CAT_MSK

#define MSC01_PBC_CSxRW_RDT_SHF		24
#define MSC01_PBC_CSxRW_RDT_MSK		0x1f000000
#define MSC01_PBC_CSxRW_RAT_SHF		16
#define MSC01_PBC_CSxRW_RAT_MSK		0x001f0000
#define MSC01_PBC_CSxRW_WDT_SHF		8
#define MSC01_PBC_CSxRW_WDT_MSK		0x00001f00
#define MSC01_PBC_CSxRW_WAT_SHF		0
#define MSC01_PBC_CSxRW_WAT_MSK		0x0000001f
#define MSC01_PBC_CS0RW_RDT_SHF		MSC01_PBC_CSxRW_RDT_SHF
#define MSC01_PBC_CS0RW_RDT_MSK		MSC01_PBC_CSxRW_RDT_MSK
#define MSC01_PBC_CS0RW_RAT_SHF		MSC01_PBC_CSxRW_RAT_SHF
#define MSC01_PBC_CS0RW_RAT_MSK		MSC01_PBC_CSxRW_RAT_MSK
#define MSC01_PBC_CS0RW_WDT_SHF		MSC01_PBC_CSxRW_WDT_SHF
#define MSC01_PBC_CS0RW_WDT_MSK		MSC01_PBC_CSxRW_WDT_MSK
#define MSC01_PBC_CS0RW_WAT_SHF		MSC01_PBC_CSxRW_WAT_SHF
#define MSC01_PBC_CS0RW_WAT_MSK		MSC01_PBC_CSxRW_WAT_MSK
#define MSC01_PBC_CS1RW_RDT_SHF		MSC01_PBC_CSxRW_RDT_SHF
#define MSC01_PBC_CS1RW_RDT_MSK		MSC01_PBC_CSxRW_RDT_MSK
#define MSC01_PBC_CS1RW_RAT_SHF		MSC01_PBC_CSxRW_RAT_SHF
#define MSC01_PBC_CS1RW_RAT_MSK		MSC01_PBC_CSxRW_RAT_MSK
#define MSC01_PBC_CS1RW_WDT_SHF		MSC01_PBC_CSxRW_WDT_SHF
#define MSC01_PBC_CS1RW_WDT_MSK		MSC01_PBC_CSxRW_WDT_MSK
#define MSC01_PBC_CS1RW_WAT_SHF		MSC01_PBC_CSxRW_WAT_SHF
#define MSC01_PBC_CS1RW_WAT_MSK		MSC01_PBC_CSxRW_WAT_MSK
#define MSC01_PBC_CS2RW_RDT_SHF		MSC01_PBC_CSxRW_RDT_SHF
#define MSC01_PBC_CS2RW_RDT_MSK		MSC01_PBC_CSxRW_RDT_MSK
#define MSC01_PBC_CS2RW_RAT_SHF		MSC01_PBC_CSxRW_RAT_SHF
#define MSC01_PBC_CS2RW_RAT_MSK		MSC01_PBC_CSxRW_RAT_MSK
#define MSC01_PBC_CS2RW_WDT_SHF		MSC01_PBC_CSxRW_WDT_SHF
#define MSC01_PBC_CS2RW_WDT_MSK		MSC01_PBC_CSxRW_WDT_MSK
#define MSC01_PBC_CS2RW_WAT_SHF		MSC01_PBC_CSxRW_WAT_SHF
#define MSC01_PBC_CS2RW_WAT_MSK		MSC01_PBC_CSxRW_WAT_MSK
#define MSC01_PBC_CS3RW_RDT_SHF		MSC01_PBC_CSxRW_RDT_SHF
#define MSC01_PBC_CS3RW_RDT_MSK		MSC01_PBC_CSxRW_RDT_MSK
#define MSC01_PBC_CS3RW_RAT_SHF		MSC01_PBC_CSxRW_RAT_SHF
#define MSC01_PBC_CS3RW_RAT_MSK		MSC01_PBC_CSxRW_RAT_MSK
#define MSC01_PBC_CS3RW_WDT_SHF		MSC01_PBC_CSxRW_WDT_SHF
#define MSC01_PBC_CS3RW_WDT_MSK		MSC01_PBC_CSxRW_WDT_MSK
#define MSC01_PBC_CS3RW_WAT_SHF		MSC01_PBC_CSxRW_WAT_SHF
#define MSC01_PBC_CS3RW_WAT_MSK		MSC01_PBC_CSxRW_WAT_MSK

/* PERIPHERAL BUS CONTROLLER
   Registers absolute addresses:
   MSC01_PBC_REG_BASE is defined in <msc01_memmap.h>*/

#define MSC01_PBC_ID            (MSC01_PBC_REG_BASE + MSC01_PBC_ID_OFS)
#define MSC01_PBC_CLKCFG        (MSC01_PBC_REG_BASE + MSC01_PBC_CLKCFG_OFS)

#define MSC01_PBC_CS0BASL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS0BASL_OFS)
#define MSC01_PBC_CS0MSKL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS0MSKL_OFS)
#define MSC01_PBC_CS1BASL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS1BASL_OFS)
#define MSC01_PBC_CS1MSKL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS1MSKL_OFS)
#define MSC01_PBC_CS2BASL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS2BASL_OFS)
#define MSC01_PBC_CS2MSKL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS2MSKL_OFS)
#define MSC01_PBC_CS3BASL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS3BASL_OFS)
#define MSC01_PBC_CS3MSKL       (MSC01_PBC_REG_BASE + MSC01_PBC_CS3MSKL_OFS)

#define MSC01_PBC_CS0CFG        (MSC01_PBC_REG_BASE + MSC01_PBC_CS0CFG_OFS)
#define MSC01_PBC_CS0TIM        (MSC01_PBC_REG_BASE + MSC01_PBC_CS0TIM_OFS)
#define MSC01_PBC_CS0RW         (MSC01_PBC_REG_BASE + MSC01_PBC_CS0RW_OFS)
#define MSC01_PBC_CS1CFG        (MSC01_PBC_REG_BASE + MSC01_PBC_CS1CFG_OFS)
#define MSC01_PBC_CS1TIM        (MSC01_PBC_REG_BASE + MSC01_PBC_CS1TIM_OFS)
#define MSC01_PBC_CS1RW         (MSC01_PBC_REG_BASE + MSC01_PBC_CS1RW_OFS)
#define MSC01_PBC_CS2CFG        (MSC01_PBC_REG_BASE + MSC01_PBC_CS2CFG_OFS)
#define MSC01_PBC_CS2TIM        (MSC01_PBC_REG_BASE + MSC01_PBC_CS2TIM_OFS)
#define MSC01_PBC_CS2RW         (MSC01_PBC_REG_BASE + MSC01_PBC_CS2RW_OFS)
#define MSC01_PBC_CS3CFG        (MSC01_PBC_REG_BASE + MSC01_PBC_CS3CFG_OFS)
#define MSC01_PBC_CS3TIM        (MSC01_PBC_REG_BASE + MSC01_PBC_CS3TIM_OFS)
#define MSC01_PBC_CS3RW         (MSC01_PBC_REG_BASE + MSC01_PBC_CS3RW_OFS)

#define MSC01_PBC_CSxCFG_DTYP_8BIT	0
#define MSC01_PBC_CSxCFG_DTYP_16BIT	1
#define MSC01_PBC_CSxCFG_DTYP_32BIT	2
#define MSC01_PBC_CS0CFG_DTYP_8BIT	MSC01_PBC_CSxCFG_DTYP_8BIT
#define MSC01_PBC_CS0CFG_DTYP_16BIT	MSC01_PBC_CSxCFG_DTYP_16BIT
#define MSC01_PBC_CS0CFG_DTYP_32BIT	MSC01_PBC_CSxCFG_DTYP_32BIT
#define MSC01_PBC_CS1CFG_DTYP_8BIT	MSC01_PBC_CSxCFG_DTYP_8BIT
#define MSC01_PBC_CS1CFG_DTYP_16BIT	MSC01_PBC_CSxCFG_DTYP_16BIT
#define MSC01_PBC_CS1CFG_DTYP_32BIT	MSC01_PBC_CSxCFG_DTYP_32BIT
#define MSC01_PBC_CS2CFG_DTYP_8BIT	MSC01_PBC_CSxCFG_DTYP_8BIT
#define MSC01_PBC_CS2CFG_DTYP_16BIT	MSC01_PBC_CSxCFG_DTYP_16BIT
#define MSC01_PBC_CS2CFG_DTYP_32BIT	MSC01_PBC_CSxCFG_DTYP_32BIT
#define MSC01_PBC_CS3CFG_DTYP_8BIT	MSC01_PBC_CSxCFG_DTYP_8BIT
#define MSC01_PBC_CS3CFG_DTYP_16BIT	MSC01_PBC_CSxCFG_DTYP_16BIT
#define MSC01_PBC_CS3CFG_DTYP_32BIT	MSC01_PBC_CSxCFG_DTYP_32BIT

#endif
