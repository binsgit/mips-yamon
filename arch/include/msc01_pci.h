/* ##########################################################################
 *
 *  PCI bridge header file
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

#ifndef MSC01_PCI_H
#define MSC01_PCI_H

/*****************************************************************************
 * Register offset addresses
 ****************************************************************************/

/* PCI bridge ID in MSC01_PCI_ID */
#define MSC01_ID_PCI                    0x82

#define MSC01_PCI_ID_OFS		0x0000
#define MSC01_PCI_SC2PMBASL_OFS		0x0208
#define MSC01_PCI_SC2PMMSKL_OFS		0x0218
#define MSC01_PCI_SC2PMMAPL_OFS		0x0228
#define MSC01_PCI_SC2PIOBASL_OFS	0x0248
#define MSC01_PCI_SC2PIOMSKL_OFS	0x0258
#define MSC01_PCI_SC2PIOMAPL_OFS	0x0268
#define MSC01_PCI_P2SCMSKL_OFS		0x0308
#define MSC01_PCI_P2SCMAPL_OFS		0x0318
#define MSC01_PCI_INTCFG_OFS		0x0600
#define MSC01_PCI_INTSTAT_OFS		0x0608
#define MSC01_PCI_CFGADDR_OFS		0x0610
#define MSC01_PCI_CFGDATA_OFS		0x0618
#define MSC01_PCI_IACK_OFS		0x0620
#define MSC01_PCI_HEAD0_OFS		0x2000  /* DevID, VendorID */
#define MSC01_PCI_HEAD1_OFS		0x2008  /* Status, Command */
#define MSC01_PCI_HEAD2_OFS		0x2010  /* Class code, RevID */
#define MSC01_PCI_HEAD3_OFS		0x2018  /* bist, header, latency */
#define MSC01_PCI_HEAD4_OFS		0x2020  /* BAR 0 */
#define MSC01_PCI_HEAD5_OFS		0x2028  /* BAR 1 */
#define MSC01_PCI_HEAD6_OFS		0x2030  /* BAR 2 */
#define MSC01_PCI_HEAD7_OFS		0x2038  /* BAR 3 */
#define MSC01_PCI_HEAD8_OFS		0x2040  /* BAR 4 */
#define MSC01_PCI_HEAD9_OFS		0x2048  /* BAR 5 */
#define MSC01_PCI_HEAD10_OFS		0x2050  /* CardBus CIS Ptr */
#define MSC01_PCI_HEAD11_OFS		0x2058  /* SubSystem ID, -VendorID */
#define MSC01_PCI_HEAD12_OFS		0x2060  /* ROM BAR */
#define MSC01_PCI_HEAD13_OFS		0x2068  /* Capabilities ptr */
#define MSC01_PCI_HEAD14_OFS		0x2070  /* reserved */
#define MSC01_PCI_HEAD15_OFS		0x2078  /* Maxl, ming, intpin, int */
#define MSC01_PCI_BAR0_OFS		0x2220
#define MSC01_PCI_CFG_OFS		0x2380
#define MSC01_PCI_SWAP_OFS		0x2388


/*****************************************************************************
 * Register encodings
 ****************************************************************************/

#define MSC01_PCI_ID_ID_SHF		16
#define MSC01_PCI_ID_ID_MSK		0x00ff0000
#define MSC01_PCI_ID_MAR_SHF		8
#define MSC01_PCI_ID_MAR_MSK		0x0000ff00
#define MSC01_PCI_ID_MIR_SHF		0
#define MSC01_PCI_ID_MIR_MSK		0x000000ff

#define MSC01_PCI_SC2PMBASL_BAS_SHF	24
#define MSC01_PCI_SC2PMBASL_BAS_MSK	0xff000000

#define MSC01_PCI_SC2PMMSKL_MSK_SHF	24
#define MSC01_PCI_SC2PMMSKL_MSK_MSK	0xff000000

#define MSC01_PCI_SC2PMMAPL_MAP_SHF	24
#define MSC01_PCI_SC2PMMAPL_MAP_MSK	0xff000000

#define MSC01_PCI_SC2PIOBASL_BAS_SHF	24
#define MSC01_PCI_SC2PIOBASL_BAS_MSK	0xff000000

#define MSC01_PCI_SC2PIOMSKL_MSK_SHF	24
#define MSC01_PCI_SC2PIOMSKL_MSK_MSK	0xff000000

#define MSC01_PCI_SC2PIOMAPL_MAP_SHF	24
#define MSC01_PCI_SC2PIOMAPL_MAP_MSK	0xff000000

#define MSC01_PCI_P2SCMSKL_MSK_SHF	20
#define MSC01_PCI_P2SCMSKL_MSK_MSK	0xfff00000

#define MSC01_PCI_P2SCMAPL_MAP_SHF	20
#define MSC01_PCI_P2SCMAPL_MAP_MSK	0xfff00000

#define MSC01_PCI_INTCFG_RST_SHF        10
#define MSC01_PCI_INTCFG_RST_MSK        0x00000400
#define MSC01_PCI_INTCFG_RST_BIT        0x00000400
#define MSC01_PCI_INTCFG_MWE_SHF        9
#define MSC01_PCI_INTCFG_MWE_MSK        0x00000200
#define MSC01_PCI_INTCFG_MWE_BIT        0x00000200
#define MSC01_PCI_INTCFG_DTO_SHF        8
#define MSC01_PCI_INTCFG_DTO_MSK        0x00000100
#define MSC01_PCI_INTCFG_DTO_BIT        0x00000100
#define MSC01_PCI_INTCFG_MA_SHF         7
#define MSC01_PCI_INTCFG_MA_MSK         0x00000080
#define MSC01_PCI_INTCFG_MA_BIT         0x00000080
#define MSC01_PCI_INTCFG_TA_SHF         6
#define MSC01_PCI_INTCFG_TA_MSK         0x00000040
#define MSC01_PCI_INTCFG_TA_BIT         0x00000040
#define MSC01_PCI_INTCFG_RTY_SHF        5
#define MSC01_PCI_INTCFG_RTY_MSK        0x00000020
#define MSC01_PCI_INTCFG_RTY_BIT        0x00000020
#define MSC01_PCI_INTCFG_MWP_SHF        4
#define MSC01_PCI_INTCFG_MWP_MSK        0x00000010
#define MSC01_PCI_INTCFG_MWP_BIT        0x00000010
#define MSC01_PCI_INTCFG_MRP_SHF        3
#define MSC01_PCI_INTCFG_MRP_MSK        0x00000008
#define MSC01_PCI_INTCFG_MRP_BIT        0x00000008
#define MSC01_PCI_INTCFG_SWP_SHF        2
#define MSC01_PCI_INTCFG_SWP_MSK        0x00000004
#define MSC01_PCI_INTCFG_SWP_BIT        0x00000004
#define MSC01_PCI_INTCFG_SRP_SHF        1
#define MSC01_PCI_INTCFG_SRP_MSK        0x00000002
#define MSC01_PCI_INTCFG_SRP_BIT        0x00000002
#define MSC01_PCI_INTCFG_SE_SHF         0
#define MSC01_PCI_INTCFG_SE_MSK         0x00000001
#define MSC01_PCI_INTCFG_SE_BIT         0x00000001

#define MSC01_PCI_INTSTAT_RST_SHF       10
#define MSC01_PCI_INTSTAT_RST_MSK       0x00000400
#define MSC01_PCI_INTSTAT_RST_BIT       0x00000400
#define MSC01_PCI_INTSTAT_MWE_SHF       9
#define MSC01_PCI_INTSTAT_MWE_MSK       0x00000200
#define MSC01_PCI_INTSTAT_MWE_BIT       0x00000200
#define MSC01_PCI_INTSTAT_DTO_SHF       8
#define MSC01_PCI_INTSTAT_DTO_MSK       0x00000100
#define MSC01_PCI_INTSTAT_DTO_BIT       0x00000100
#define MSC01_PCI_INTSTAT_MA_SHF        7
#define MSC01_PCI_INTSTAT_MA_MSK        0x00000080
#define MSC01_PCI_INTSTAT_MA_BIT        0x00000080
#define MSC01_PCI_INTSTAT_TA_SHF        6
#define MSC01_PCI_INTSTAT_TA_MSK        0x00000040
#define MSC01_PCI_INTSTAT_TA_BIT        0x00000040
#define MSC01_PCI_INTSTAT_RTY_SHF       5
#define MSC01_PCI_INTSTAT_RTY_MSK       0x00000020
#define MSC01_PCI_INTSTAT_RTY_BIT       0x00000020
#define MSC01_PCI_INTSTAT_MWP_SHF       4
#define MSC01_PCI_INTSTAT_MWP_MSK       0x00000010
#define MSC01_PCI_INTSTAT_MWP_BIT       0x00000010
#define MSC01_PCI_INTSTAT_MRP_SHF       3
#define MSC01_PCI_INTSTAT_MRP_MSK       0x00000008
#define MSC01_PCI_INTSTAT_MRP_BIT       0x00000008
#define MSC01_PCI_INTSTAT_SWP_SHF       2
#define MSC01_PCI_INTSTAT_SWP_MSK       0x00000004
#define MSC01_PCI_INTSTAT_SWP_BIT       0x00000004
#define MSC01_PCI_INTSTAT_SRP_SHF       1
#define MSC01_PCI_INTSTAT_SRP_MSK       0x00000002
#define MSC01_PCI_INTSTAT_SRP_BIT       0x00000002
#define MSC01_PCI_INTSTAT_SE_SHF        0
#define MSC01_PCI_INTSTAT_SE_MSK        0x00000001
#define MSC01_PCI_INTSTAT_SE_BIT        0x00000001

#define MSC01_PCI_CFGADDR_BNUM_SHF	16
#define MSC01_PCI_CFGADDR_BNUM_MSK	0x00ff0000
#define MSC01_PCI_CFGADDR_DNUM_SHF	11
#define MSC01_PCI_CFGADDR_DNUM_MSK	0x0000f800
#define MSC01_PCI_CFGADDR_FNUM_SHF	8
#define MSC01_PCI_CFGADDR_FNUM_MSK	0x00000700
#define MSC01_PCI_CFGADDR_RNUM_SHF	2
#define MSC01_PCI_CFGADDR_RNUM_MSK	0x000000fc

#define MSC01_PCI_CFGDATA_DATA_SHF	0
#define MSC01_PCI_CFGDATA_DATA_MSK	0xffffffff


#define MSC01_PCI_HEAD0_DID_SHF		16
#define MSC01_PCI_HEAD0_DID_MSK		0xffff0000
#define MSC01_PCI_HEAD0_VID_SHF         0
#define MSC01_PCI_HEAD0_VID_MSK         0x0000ffff
#define MSC01_PCI_HEAD1_PED_SHF         31
#define MSC01_PCI_HEAD1_PED_MSK         0x80000000
#define MSC01_PCI_HEAD1_PED_BIT         0x80000000
#define MSC01_PCI_HEAD1_SSE_SHF         30
#define MSC01_PCI_HEAD1_SSE_MSK         0x40000000
#define MSC01_PCI_HEAD1_SSE_BIT         0x40000000
#define MSC01_PCI_HEAD1_MAR_SHF         29
#define MSC01_PCI_HEAD1_MAR_MSK         0x20000000
#define MSC01_PCI_HEAD1_MAR_BIT         0x20000000
#define MSC01_PCI_HEAD1_TAR_SHF         28
#define MSC01_PCI_HEAD1_TAR_MSK         0x10000000
#define MSC01_PCI_HEAD1_TAR_BIT         0x10000000
#define MSC01_PCI_HEAD1_TAS_SHF         27
#define MSC01_PCI_HEAD1_TAS_MSK         0x08000000
#define MSC01_PCI_HEAD1_TAS_BIT         0x08000000
#define MSC01_PCI_HEAD1_DST_SHF         25
#define MSC01_PCI_HEAD1_DST_MSK         0x06000000
#define MSC01_PCI_HEAD1_DST_BIT         0x06000000
#define MSC01_PCI_HEAD1_MPE_SHF         24
#define MSC01_PCI_HEAD1_MPE_BIT         0x01000000
#define MSC01_PCI_HEAD1_MPE_MSK         0x01000000
#define MSC01_PCI_HEAD1_BB_SHF          23
#define MSC01_PCI_HEAD1_BB_BIT          0x00800000
#define MSC01_PCI_HEAD1_BB_MSK          0x00800000
#define MSC01_PCI_HEAD1_UDF_SHF         22
#define MSC01_PCI_HEAD1_UDF_BIT         0x00400000
#define MSC01_PCI_HEAD1_UDF_MSK         0x00400000
#define MSC01_PCI_HEAD1_66M_SHF         21
#define MSC01_PCI_HEAD1_66M_BIT         0x00200000
#define MSC01_PCI_HEAD1_66M_MSK         0x00200000
#define MSC01_PCI_HEAD1_CAP_SHF         20
#define MSC01_PCI_HEAD1_CAP_BIT         0x00100000
#define MSC01_PCI_HEAD1_CAP_MSK         0x00100000
#define MSC01_PCI_HEAD1_EBB_SHF         9
#define MSC01_PCI_HEAD1_EBB_BIT         0x00000200
#define MSC01_PCI_HEAD1_EBB_MSK         0x00000200
#define MSC01_PCI_HEAD1_SE_SHF          8
#define MSC01_PCI_HEAD1_SE_BIT          0x00000100
#define MSC01_PCI_HEAD1_SE_MSK          0x00000100
#define MSC01_PCI_HEAD1_SC_SHF          7
#define MSC01_PCI_HEAD1_SC_BIT          0x00000080
#define MSC01_PCI_HEAD1_SC_MSK          0x00000080
#define MSC01_PCI_HEAD1_PER_SHF         6
#define MSC01_PCI_HEAD1_PER_BIT         0x00000040
#define MSC01_PCI_HEAD1_PER_MSK         0x00000040
#define MSC01_PCI_HEAD1_VGA_SHF         5
#define MSC01_PCI_HEAD1_VGA_BIT         0x00000020
#define MSC01_PCI_HEAD1_VGA_MSK         0x00000020
#define MSC01_PCI_HEAD1_MWI_SHF         4
#define MSC01_PCI_HEAD1_MWI_BIT         0x00000010
#define MSC01_PCI_HEAD1_MWI_MSK         0x00000010
#define MSC01_PCI_HEAD1_MSC_SHF         3
#define MSC01_PCI_HEAD1_MSC_BIT         0x00000008
#define MSC01_PCI_HEAD1_MSC_MSK         0x00000008
#define MSC01_PCI_HEAD1_EMA_SHF         2
#define MSC01_PCI_HEAD1_EMA_BIT         0x00000004
#define MSC01_PCI_HEAD1_EMA_MSK         0x00000004
#define MSC01_PCI_HEAD1_EMS_SHF         1
#define MSC01_PCI_HEAD1_EMS_MSK         0x00000002
#define MSC01_PCI_HEAD1_EMS_BIT         0x00000002
#define MSC01_PCI_HEAD1_EIO_SHF         0
#define MSC01_PCI_HEAD1_EIO_MSK         0x00000001
#define MSC01_PCI_HEAD1_EIO_BIT         0x00000001
#define MSC01_PCI_HEAD2_CLASS_SHF       8
#define MSC01_PCI_HEAD2_CLASS_MSK       0xffffff00
#define MSC01_PCI_HEAD2_RID_SHF         0
#define MSC01_PCI_HEAD2_RID_MSK         0x000000ff
#define MSC01_PCI_HEAD3_BIST_SHF        24
#define MSC01_PCI_HEAD3_BIST_MSK        0xff000000
#define MSC01_PCI_HEAD3_HTYPE_SHF       16
#define MSC01_PCI_HEAD3_HTYPE_MSK       0x00ff0000
#define MSC01_PCI_HEAD3_LAT_SHF         8
#define MSC01_PCI_HEAD3_LAT_MSK         0x0000ff00
#define MSC01_PCI_HEAD3_CLS_SHF         0
#define MSC01_PCI_HEAD3_CLS_MSK         0x000000ff
#define MSC01_PCI_HEAD4_BAR_SHF         0
#define MSC01_PCI_HEAD4_BAR_MSK         0xffffffff
//#define MSC01_PCI_HEAD5_
//#define MSC01_PCI_HEAD6_
//#define MSC01_PCI_HEAD7_
//#define MSC01_PCI_HEAD8_
//#define MSC01_PCI_HEAD9_
#define MSC01_PCI_HEAD10_CIS_SHF        0
#define MSC01_PCI_HEAD10_CIS_MSK        0xffffffff
#define MSC01_PCI_HEAD11_SID_SHF        16
#define MSC01_PCI_HEAD11_SID_MSK        0xffff0000
#define MSC01_PCI_HEAD11_SVID_SHF       0
#define MSC01_PCI_HEAD11_SVID_MSK       0x0000ffff
#define MSC01_PCI_HEAD12_EROM_SHF       0
#define MSC01_PCI_HEAD12_EROM_MSK       0xffffffff
#define MSC01_PCI_HEAD13_CAPP_SHF       0
#define MSC01_PCI_HEAD13_CAPP_MSK       0x000000ff
//#define MSC01_PCI_HEAD14
#define MSC01_PCI_HEAD15_MAXL_SHF       24
#define MSC01_PCI_HEAD15_MAXL_MSK       0xff000000
#define MSC01_PCI_HEAD15_MING_SHF       16
#define MSC01_PCI_HEAD15_MING_MSK       0x00ff0000
#define MSC01_PCI_HEAD15_IPIN_SHF       8
#define MSC01_PCI_HEAD15_IPIN_MSK       0x0000ff00
#define MSC01_PCI_HEAD15_ILINE_SHF      0
#define MSC01_PCI_HEAD15_ILINE_MSK      0x000000ff

/* The defines below are ONLY valid for a MEM bar! */
#define MSC01_PCI_BAR0_SIZE_SHF	        4
#define MSC01_PCI_BAR0_SIZE_MSK	        0xfffffff0
#define MSC01_PCI_BAR0_P_SHF	        3
#define MSC01_PCI_BAR0_P_MSK	        0x00000008
#define MSC01_PCI_BAR0_P_BIT	        MSC01_PCI_BAR0_P_MSK
#define MSC01_PCI_BAR0_D_SHF	        1
#define MSC01_PCI_BAR0_D_MSK	        0x00000006
#define MSC01_PCI_BAR0_T_SHF	        0
#define MSC01_PCI_BAR0_T_MSK	        0x00000001
#define MSC01_PCI_BAR0_T_BIT	        MSC01_PCI_BAR0_T_MSK


#define MSC01_PCI_CFG_RA_SHF	        17
#define MSC01_PCI_CFG_RA_MSK	        0x00020000
#define MSC01_PCI_CFG_RA_BIT	        MSC01_PCI_CFG_RA_MSK
#define MSC01_PCI_CFG_G_SHF	        16
#define MSC01_PCI_CFG_G_MSK	        0x00010000
#define MSC01_PCI_CFG_G_BIT	        MSC01_PCI_CFG_G_MSK
#define MSC01_PCI_CFG_EN_SHF	        15
#define MSC01_PCI_CFG_EN_MSK	        0x00008000
#define MSC01_PCI_CFG_EN_BIT	        MSC01_PCI_CFG_EN_MSK
#define MSC01_PCI_CFG_MRTRY_SHF         0
#define MSC01_PCI_CFG_MRTRY_MSK         0x00000fff

#define MSC01_PCI_SWAP_IO_SHF		18
#define MSC01_PCI_SWAP_IO_MSK		0x000c0000
#define MSC01_PCI_SWAP_IO_NOSWAP	0
#define MSC01_PCI_SWAP_IO_BYTESWAP	1
#define MSC01_PCI_SWAP_MEM_SHF		16
#define MSC01_PCI_SWAP_MEM_MSK		0x00030000
#define MSC01_PCI_SWAP_MEM_NOSWAP	0
#define MSC01_PCI_SWAP_MEM_BYTESWAP	1
#define MSC01_PCI_SWAP_BAR0_SHF		0
#define MSC01_PCI_SWAP_BAR0_MSK		0x00000003
#define MSC01_PCI_SWAP_BAR0_NOSWAP	0
#define MSC01_PCI_SWAP_BAR0_BYTESWAP	1

/*****************************************************************************
 * Registers absolute addresses
 ****************************************************************************/

#define MSC01_PCI_ID            (MSC01_PCI_REG_BASE + MSC01_PCI_ID_OFS)
#define MSC01_PCI_SC2PMBASL     (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PMBASL_OFS)
#define MSC01_PCI_SC2PMMSKL     (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PMMSKL_OFS)
#define MSC01_PCI_SC2PMMAPL     (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PMMAPL_OFS)
#define MSC01_PCI_SC2PIOBASL    (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PIOBASL_OFS)
#define MSC01_PCI_SC2PIOMSKL    (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PIOMSKL_OFS)
#define MSC01_PCI_SC2PIOMAPL    (MSC01_PCI_REG_BASE + MSC01_PCI_SC2PIOMAPL_OFS)
#define MSC01_PCI_P2SCMSKL      (MSC01_PCI_REG_BASE + MSC01_PCI_P2SCMSKL_OFS)
#define MSC01_PCI_P2SCMAPL      (MSC01_PCI_REG_BASE + MSC01_PCI_P2SCMAPL_OFS)
#define MSC01_PCI_INTCFG        (MSC01_PCI_REG_BASE + MSC01_PCI_INTCFG_OFS)
#define MSC01_PCI_INTSTAT       (MSC01_PCI_REG_BASE + MSC01_PCI_INTSTAT_OFS)
#define MSC01_PCI_CFGADDR       (MSC01_PCI_REG_BASE + MSC01_PCI_CFGADDR_OFS)
#define MSC01_PCI_CFGDATA       (MSC01_PCI_REG_BASE + MSC01_PCI_CFGDATA_OFS)
#define MSC01_PCI_IACK		(MSC01_PCI_REG_BASE + MSC01_PCI_IACK_OFS)
#define MSC01_PCI_HEAD0		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD0_OFS)
#define MSC01_PCI_HEAD1		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD1_OFS)
#define MSC01_PCI_HEAD2		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD2_OFS)
#define MSC01_PCI_HEAD3		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD3_OFS)
#define MSC01_PCI_HEAD4		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD4_OFS)
#define MSC01_PCI_HEAD5		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD5_OFS)
#define MSC01_PCI_HEAD6		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD6_OFS)
#define MSC01_PCI_HEAD7		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD7_OFS)
#define MSC01_PCI_HEAD8		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD8_OFS)
#define MSC01_PCI_HEAD9		(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD9_OFS)
#define MSC01_PCI_HEAD10	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD10_OFS)
#define MSC01_PCI_HEAD11	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD11_OFS)
#define MSC01_PCI_HEAD12	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD12_OFS)
#define MSC01_PCI_HEAD13	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD13_OFS)
#define MSC01_PCI_HEAD14	(MSC01_PCI_REG_BASE + MSC01_PCI_HEAD14_OFS)
#define MSC01_PCI_HEAD15        (MSC01_PCI_REG_BASE + MSC01_PCI_HEAD15_OFS)
#define MSC01_PCI_BAR0		(MSC01_PCI_REG_BASE + MSC01_PCI_BAR0_OFS)
#define MSC01_PCI_CFG		(MSC01_PCI_REG_BASE + MSC01_PCI_CFG_OFS)
#define MSC01_PCI_SWAP          (MSC01_PCI_REG_BASE + MSC01_PCI_SWAP_OFS)

#endif
/*****************************************************************************
 *  End of msc01_pci.h
 *****************************************************************************/
