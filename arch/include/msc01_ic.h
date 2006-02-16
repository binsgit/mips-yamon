/* ##########################################################################
 *
 * Description: Header file with defines for IC test applications
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

#ifndef MSC01_IC_H
#define MSC01_IC_H

/*****************************************************************************
 * Register offset addresses
 *****************************************************************************/

#define MSC01_IC_RST_OFS     0x00008    /* Software reset              */
#define MSC01_IC_ENAL_OFS    0x00100    /* Int_in enable mask 31:0     */
#define MSC01_IC_ENAH_OFS    0x00108    /* Int_in enable mask 63:32    */
#define MSC01_IC_DISL_OFS    0x00120    /* Int_in disable mask 31:0    */
#define MSC01_IC_DISH_OFS    0x00128    /* Int_in disable mask 63:32   */
#define MSC01_IC_ISBL_OFS    0x00140    /* Raw int_in 31:0             */
#define MSC01_IC_ISBH_OFS    0x00148    /* Raw int_in 63:32            */
#define MSC01_IC_ISAL_OFS    0x00160    /* Masked int_in 31:0          */
#define MSC01_IC_ISAH_OFS    0x00168    /* Masked int_in 63:32         */
#define MSC01_IC_LVL_OFS     0x00180    /* Disable priority int_out    */
#define MSC01_IC_RAMW_OFS    0x00180    /* Shadow set RAM (EI)	       */
#define MSC01_IC_OSB_OFS     0x00188    /* Raw int_out                 */
#define MSC01_IC_OSA_OFS     0x00190    /* Masked int_out              */
#define MSC01_IC_GENA_OFS    0x00198    /* Global HW int enable        */
#define MSC01_IC_BASE_OFS    0x001a0    /* Base address of IC_VEC      */
#define MSC01_IC_VEC_OFS     0x001b0    /* Active int's vector address */
#define MSC01_IC_EOI_OFS     0x001c0    /* Enable lower level ints     */
#define MSC01_IC_CFG_OFS     0x001c8    /* Configuration register      */
#define MSC01_IC_TRLD_OFS    0x001d0    /* Interval timer reload val   */
#define MSC01_IC_TVAL_OFS    0x001e0    /* Interval timer current val  */
#define MSC01_IC_TCFG_OFS    0x001f0    /* Interval timer config       */
#define MSC01_IC_SUP_OFS     0x00200    /* Set up int_in line 0        */
#define MSC01_IC_ENA_OFS     0x00800    /* Int_in enable mask 63:0     */
#define MSC01_IC_DIS_OFS     0x00820    /* Int_in disable mask 63:0    */
#define MSC01_IC_ISB_OFS     0x00840    /* Raw int_in 63:0             */
#define MSC01_IC_ISA_OFS     0x00860    /* Masked int_in 63:0          */

/*****************************************************************************
 * Register field encodings
 *****************************************************************************/

#define MSC01_IC_RST_RST_SHF      0
#define MSC01_IC_RST_RST_MSK      0x00000001
#define MSC01_IC_RST_RST_BIT      MSC01_IC_RST_RST_MSK
#define MSC01_IC_LVL_LVL_SHF      0
#define MSC01_IC_LVL_LVL_MSK      0x000000ff
#define MSC01_IC_LVL_SPUR_SHF     16
#define MSC01_IC_LVL_SPUR_MSK     0x00010000
#define MSC01_IC_LVL_SPUR_BIT     MSC01_IC_LVL_SPUR_MSK
#define MSC01_IC_RAMW_RIPL_SHF	  0
#define MSC01_IC_RAMW_RIPL_MSK	  0x0000003f
#define MSC01_IC_RAMW_DATA_SHF	  6
#define MSC01_IC_RAMW_DATA_MSK	  0x00000fc0
#define MSC01_IC_RAMW_ADDR_SHF	  25
#define MSC01_IC_RAMW_ADDR_MSK	  0x7e000000
#define MSC01_IC_RAMW_READ_SHF	  31
#define MSC01_IC_RAMW_READ_MSK	  0x80000000
#define MSC01_IC_RAMW_READ_BIT	  MSC01_IC_RAMW_READ_MSK
#define MSC01_IC_OSB_OSB_SHF      0
#define MSC01_IC_OSB_OSB_MSK      0x000000ff
#define MSC01_IC_OSA_OSA_SHF      0
#define MSC01_IC_OSA_OSA_MSK      0x000000ff
#define MSC01_IC_GENA_GENA_SHF    0
#define MSC01_IC_GENA_GENA_MSK    0x00000001
#define MSC01_IC_GENA_GENA_BIT    MSC01_IC_GENA_GENA_MSK
#define MSC01_IC_CFG_DIS_SHF      0
#define MSC01_IC_CFG_DIS_MSK      0x00000001
#define MSC01_IC_CFG_DIS_BIT      MSC01_IC_CFG_DIS_MSK
#define MSC01_IC_CFG_SHFT_SHF     8
#define MSC01_IC_CFG_SHFT_MSK     0x00000f00
#define MSC01_IC_TCFG_ENA_SHF     0
#define MSC01_IC_TCFG_ENA_MSK     0x00000001
#define MSC01_IC_TCFG_ENA_BIT     MSC01_IC_TCFG_ENA_MSK
#define MSC01_IC_TCFG_INT_SHF     8
#define MSC01_IC_TCFG_INT_MSK     0x00000100
#define MSC01_IC_TCFG_INT_BIT     MSC01_IC_TCFG_INT_MSK
#define MSC01_IC_TCFG_EDGE_SHF    16
#define MSC01_IC_TCFG_EDGE_MSK    0x00010000
#define MSC01_IC_TCFG_EDGE_BIT    MSC01_IC_TCFG_EDGE_MSK
#define MSC01_IC_SUP_PRI_SHF      0
#define MSC01_IC_SUP_PRI_MSK      0x00000007
#define MSC01_IC_SUP_EDGE_SHF     8
#define MSC01_IC_SUP_EDGE_MSK     0x00000100
#define MSC01_IC_SUP_EDGE_BIT     MSC01_IC_SUP_EDGE_MSK
#define MSC01_IC_SUP_STEP         8

/*****************************************************************************
 * Absolute register addresses
 *****************************************************************************/

#define MSC01_IC_RST     (MSC01_IC_REG_BASE + MSC01_IC_RST_OFS)
#define MSC01_IC_ENAL    (MSC01_IC_REG_BASE + MSC01_IC_ENAL_OFS)
#define MSC01_IC_ENAH    (MSC01_IC_REG_BASE + MSC01_IC_ENAH_OFS)
#define MSC01_IC_DISL    (MSC01_IC_REG_BASE + MSC01_IC_DISL_OFS)
#define MSC01_IC_DISH    (MSC01_IC_REG_BASE + MSC01_IC_DISH_OFS)
#define MSC01_IC_ISBL    (MSC01_IC_REG_BASE + MSC01_IC_ISBL_OFS)
#define MSC01_IC_ISBH    (MSC01_IC_REG_BASE + MSC01_IC_ISBH_OFS)
#define MSC01_IC_ISAL    (MSC01_IC_REG_BASE + MSC01_IC_ISAL_OFS)
#define MSC01_IC_ISAH    (MSC01_IC_REG_BASE + MSC01_IC_ISAH_OFS)
#define MSC01_IC_LVL     (MSC01_IC_REG_BASE + MSC01_IC_LVL_OFS)
#define MSC01_IC_RAMW    (MSC01_IC_REG_BASE + MSC01_IC_RAMW_OFS)
#define MSC01_IC_OSB     (MSC01_IC_REG_BASE + MSC01_IC_OSB_OFS)
#define MSC01_IC_OSA     (MSC01_IC_REG_BASE + MSC01_IC_OSA_OFS)
#define MSC01_IC_GENA    (MSC01_IC_REG_BASE + MSC01_IC_GENA_OFS)
#define MSC01_IC_BASE    (MSC01_IC_REG_BASE + MSC01_IC_BASE_OFS)
#define MSC01_IC_VEC     (MSC01_IC_REG_BASE + MSC01_IC_VEC_OFS)
#define MSC01_IC_EOI     (MSC01_IC_REG_BASE + MSC01_IC_EOI_OFS)
#define MSC01_IC_CFG     (MSC01_IC_REG_BASE + MSC01_IC_CFG_OFS)
#define MSC01_IC_TRLD    (MSC01_IC_REG_BASE + MSC01_IC_TRLD_OFS)
#define MSC01_IC_TVAL    (MSC01_IC_REG_BASE + MSC01_IC_TVAL_OFS)
#define MSC01_IC_TCFG    (MSC01_IC_REG_BASE + MSC01_IC_TCFG_OFS)
#define MSC01_IC_SUP     (MSC01_IC_REG_BASE + MSC01_IC_SUP_OFS)
#define MSC01_IC_ENA     (MSC01_IC_REG_BASE + MSC01_IC_ENA_OFS)
#define MSC01_IC_DIS     (MSC01_IC_REG_BASE + MSC01_IC_DIS_OFS)
#define MSC01_IC_ISB     (MSC01_IC_REG_BASE + MSC01_IC_ISB_OFS)
#define MSC01_IC_ISA     (MSC01_IC_REG_BASE + MSC01_IC_ISA_OFS)

#endif /* #ifndef MSC01_IC_H */

/*****************************************************************************
 *  End of msc01_ic.h
 *****************************************************************************/
