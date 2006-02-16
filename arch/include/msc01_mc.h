
/* ########################################################################## 
 *
 * Description:	Memory Controller Header File
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

#ifndef MSC01_MC_H
#define MSC01_MC_H

/*****************************************************************************
 * Register offset addresses
 ****************************************************************************/

#define MSC01_MC_CFGPB0_OFS	0x0110	/* SDRAM config, phbank0 (1) */
#define MSC01_MC_CFGPB1_OFS	0x0118	/* SDRAM config, phbank1 (1) */
#define MSC01_MC_CFGPB2_OFS	0x0120	/* SDRAM config, phbank2 (1) */
#define MSC01_MC_CFGPB3_OFS	0x0128	/* SDRAM config, phbank3 (1) */
#define MSC01_MC_LATENCY_OFS	0x0130	/* SDRAM latency params  (1) */
#define MSC01_MC_IOCTRL_OFS	0x0140	/* I/O region ctrl reg   (1) */
#define MSC01_MC_TIMPAR_OFS	0x0150	/* SDRAM command timing  (2) */
#define MSC01_MC_TREFRESH_OFS	0x0160	/* SDRAM refresh time    (3) */
#define MSC01_MC_INITCMD_OFS	0x0200	/* SDRAM init cmd reg    (4) */
#define MSC01_MC_CTRLENA_OFS	0x0300	/* Memory controller enabl   */
#define MSC01_MC_CMD_PD_OFS	0x0400	/* SDRAM power down ctrl     */
#define MSC01_MC_ARB_CFG_OFS	0x0600	/* 16 GP bits to arbiter     */
#define MSC01_MC_ARB_STAT_OFS	0x0608	/* arbitration unit status   */
#define MSC01_MC_HC_DDR_OFS	0x0710	/* DDR SDRAM type config (5) */
#define MSC01_MC_HC_FMDW_OFS	0x0718	/* Full memory data width(5) */
#define MSC01_MC_HC_PARITY_OFS	0x0720	/* Parity check enable   (5) */
#define MSC01_MC_HC_CLKRAT_OFS	0x0728	/* Clk ratio sysctl/SDRAM(5) */
#define MSC01_MC_HC_RDDEL_OFS	0x0730	/* Read Data delay       (5) */
#define MSC01_MC_SPD_CFG_OFS	0x0800	/* Presence Detect Config    */
#define MSC01_MC_SPD_ADR_OFS	0x0808	/* Presence Detect Read Addr */
#define MSC01_MC_SPD_DAT_OFS	0x0810	/* Presence Detect Read Data */

/* Note 1: must be set and frozen before initialization of SDRAM */
/* Note 2: must be set and frozen before MC_CTRLENA is set       */
/* Note 3: must be set before MC_CTRLENA is set                  */
/* Note 4: must NOT be accesses when MC_CTRLENA is set.          */
/* Note 5: Hidden Control,  note 1 applies if used anyway.       */

/*****************************************************************************
 * Register encodings
 ****************************************************************************/

/* Row Width Fields of MC_CFGPX */
#define MSC01_MC_CFGPBx_ROWW_SHF	4
#define MSC01_MC_CFGPBx_ROWW_MSK	0x000000f0
#define MSC01_MC_CFGPB0_ROWW_SHF	MSC01_MC_CFGPBx_ROWW_SHF
#define MSC01_MC_CFGPB0_ROWW_MSK	MSC01_MC_CFGPBx_ROWW_MSK
#define MSC01_MC_CFGPB1_ROWW_SHF	MSC01_MC_CFGPBx_ROWW_SHF
#define MSC01_MC_CFGPB1_ROWW_MSK	MSC01_MC_CFGPBx_ROWW_MSK
#define MSC01_MC_CFGPB2_ROWW_SHF	MSC01_MC_CFGPBx_ROWW_SHF
#define MSC01_MC_CFGPB2_ROWW_MSK	MSC01_MC_CFGPBx_ROWW_MSK
#define MSC01_MC_CFGPB3_ROWW_SHF	MSC01_MC_CFGPBx_ROWW_SHF
#define MSC01_MC_CFGPB3_ROWW_MSK	MSC01_MC_CFGPBx_ROWW_MSK

/* Column Width Fields of MC_CFGPX */
#define MSC01_MC_CFGPBx_COLW_SHF	0
#define MSC01_MC_CFGPBx_COLW_MSK	0x0000000f
#define MSC01_MC_CFGPB0_COLW_SHF	MSC01_MC_CFGPBx_COLW_SHF
#define MSC01_MC_CFGPB0_COLW_MSK	MSC01_MC_CFGPBx_COLW_MSK
#define MSC01_MC_CFGPB1_COLW_SHF	MSC01_MC_CFGPBx_COLW_SHF
#define MSC01_MC_CFGPB1_COLW_MSK	MSC01_MC_CFGPBx_COLW_MSK
#define MSC01_MC_CFGPB2_COLW_SHF	MSC01_MC_CFGPBx_COLW_SHF
#define MSC01_MC_CFGPB2_COLW_MSK	MSC01_MC_CFGPBx_COLW_MSK
#define MSC01_MC_CFGPB3_COLW_SHF	MSC01_MC_CFGPBx_COLW_SHF
#define MSC01_MC_CFGPB3_COLW_MSK	MSC01_MC_CFGPBx_COLW_MSK

/* Fields of MC_LATENCY */
#define MSC01_MC_LATENCY_CSL_SHF	8
#define MSC01_MC_LATENCY_CSL_MSK	0x00000f00
#define MSC01_MC_LATENCY_WL_SHF		4
#define MSC01_MC_LATENCY_WL_MSK		0x000000f0
#define MSC01_MC_LATENCY_CL_SHF		0
#define MSC01_MC_LATENCY_CL_MSK		0x0000000f

/* Field of MC_IOCTRL */
#define MSC01_MC_IOCTRL_IOGP_SHF	0
#define MSC01_MC_IOCTRL_IOGP_MSK	0xffffffff

/* Fields of MC_TIMPAR */
#define MSC01_MC_TIMPAR_TMRAS_SHF	16
#define MSC01_MC_TIMPAR_TMRAS_MIN	1
#define MSC01_MC_TIMPAR_TMRAS_MAX	255
#define MSC01_MC_TIMPAR_TMRAS_MSK	0x00ff0000
#define MSC01_MC_TIMPAR_TRP_SHF		12
#define MSC01_MC_TIMPAR_TRP_MIN		2
#define MSC01_MC_TIMPAR_TRP_MAX		4
#define MSC01_MC_TIMPAR_TRP_MSK		0x0000f000
#define MSC01_MC_TIMPAR_TRAS_SHF	8
#define MSC01_MC_TIMPAR_TRAS_MIN	2
#define MSC01_MC_TIMPAR_TRAS_MAX	8
#define MSC01_MC_TIMPAR_TRAS_MSK	0x00000f00
#define MSC01_MC_TIMPAR_TRCD_SHF	4
#define MSC01_MC_TIMPAR_TRCD_MIN	2
#define MSC01_MC_TIMPAR_TRCD_MAX	3
#define MSC01_MC_TIMPAR_TRCD_MSK	0x000000f0
#define MSC01_MC_TIMPAR_TDPL_SHF	0
#define MSC01_MC_TIMPAR_TDPL_MIN	1
#define MSC01_MC_TIMPAR_TDPL_MAX	3
#define MSC01_MC_TIMPAR_TDPL_MSK	0x0000000f

/* Fields of MC_TREFRESH */
#define MSC01_MC_TREFRESH_TREF_SHF	0
#define MSC01_MC_TREFRESH_TREF_MSK	0x00001fff

/* Fields of MC_INITCMD */
#define MSC01_MC_INITCMD_WAIT_SHF	20
#define MSC01_MC_INITCMD_WAIT_MSK	0x00f00000	
#define MSC01_MC_INITCMD_ICMD_SHF	16
#define MSC01_MC_INITCMD_ICMD_MSK	0x00070000	
#define MSC01_MC_INITCMD_BA_SHF		12
#define MSC01_MC_INITCMD_BA_MSK		0x00003000
#define MSC01_MC_INITCMD_A_SHF		0
#define MSC01_MC_INITCMD_A_MSK		0x00000fff
/* init command field-codes */
#define MSC01_MC_INITCMD_ICMD_NOP	0x7
#define MSC01_MC_INITCMD_ICMD_PALL	0x2
#define MSC01_MC_INITCMD_ICMD_CBR	0x1
#define MSC01_MC_INITCMD_ICMD_MRS	0x0
#define MSC01_MC_INITCMD_BA_MRS		0x0
#define MSC01_MC_INITCMD_BA_EMRS	0x1
#define MSC01_MC_INITCMD_BA_PALL	0x0
#define MSC01_MC_INITCMD_BA_CBR		0x0
#define MSC01_MC_INITCMD_A_PALL		0x400
#define MSC01_MC_INITCMD_A_CBR		0x0

/* Fields of MC_CTRLENA */
#define MSC01_MC_CTRLENA_ENA_SHF	0	
#define MSC01_MC_CTRLENA_ENA_MSK	0x00000001
#define MSC01_MC_CTRLENA_ENA_BIT	MSC01_MC_CTRLENA_ENA_MSK

/* Fields of MC_CMD_PD */
#define MSC01_MC_CMD_PD_PWDS_SHF	1
#define MSC01_MC_CMD_PD_PWDS_MSK	0x00000002
#define MSC01_MC_CMD_PD_PWDS_BIT	MSC01_MC_CMD_PD_PWDS_MSK
#define MSC01_MC_CMD_PD_PWD_SHF		0
#define MSC01_MC_CMD_PD_PWD_MSK		0x00000001
#define MSC01_MC_CMD_PD_PWD_BIT		MSC01_MC_CMD_PD_PWD_MSK

/* Fields of MC_ARB_CFG */
#define MSC01_MC_ARB_CFG_ARB_CFG_SHF	0	
#define MSC01_MC_ARB_CFG_ARB_CFG_MSK	0x0000ffff

/* Fields of MC_ARB_STAT */
#define MSC01_MC_ARB_STAT_ARB_STAT_SHF	0	
#define MSC01_MC_ARB_STAT_ARB_STAT_MSK	0x0000ffff

/* Hidden register MC_HC_DDR */
#define MSC01_MC_HC_DDR_DDR_SHF		0
#define MSC01_MC_HC_DDR_DDR_MSK		0x00000001
#define MSC01_MC_HC_DDR_DDR_BIT		MSC01_MC_HC_DDR_DDR_MSK

/* Hidden register MC_HC_FMDW */
#define MSC01_MC_HC_FMDW_FMDW_SHF	0	
#define MSC01_MC_HC_FMDW_FMDW_MSK	0x00000001
#define MSC01_MC_HC_FMDW_FMDW_BIT	MSC01_MC_HC_FMDW_FMDW_MSK

/* Hidden register MC_HC_PARITY */
#define MSC01_MC_HC_PARITY_PARITY_SHF	0	
#define MSC01_MC_HC_PARITY_PARITY_MSK	0x00000001
#define MSC01_MC_HC_PARITY_PARITY_BIT	MSC01_MC_HC_PARITY_PARITY_MSK

/* Hidden register MC_HC_CLKRAT */
#define MSC01_MC_HC_CLKRAT_CLKRAT_SHF	0	
#define MSC01_MC_HC_CLKRAT_CLKRAT_MSK	0x00000007
#define MSC01_MC_HC_CLKRAT_CLKRAT_1_1	1
#define MSC01_MC_HC_CLKRAT_CLKRAT_3_2	2
#define MSC01_MC_HC_CLKRAT_CLKRAT_2_1	3
#define MSC01_MC_HC_CLKRAT_CLKRAT_3_1	4
#define MSC01_MC_HC_CLKRAT_CLKRAT_4_1	5

/* Hidden register MC_HC_RDDEL */
#define MSC01_MC_HC_RDDEL_RDDEL_SHF	0	
#define MSC01_MC_HC_RDDEL_RDDEL_MSK	0x0000000f

/* Fields of MC_SPD_CFG */
#define MSC01_MC_SPD_CFG_CPDIV_SHF	0
#define MSC01_MC_SPD_CFG_CPDIV_MSK	0x000001ff

/* Fields of MC_SPD_ADR */
#define MSC01_MC_SPD_ADR_DADR_SHF	8
#define MSC01_MC_SPD_ADR_DADR_MSK	0x00000700
#define MSC01_MC_SPD_ADR_BADR_SHF	0
#define MSC01_MC_SPD_ADR_BADR_MSK	0x000000ff

/* Fields of MC_SPD_DAT */
#define MSC01_MC_SPD_DAT_RDERR_SHF	9	
#define MSC01_MC_SPD_DAT_RDERR_MSK	0x00000200
#define MSC01_MC_SPD_DAT_RDERR_BIT	MSC01_MC_SPD_DAT_RDERR_MSK
#define MSC01_MC_SPD_DAT_BUSY_SHF	8	
#define MSC01_MC_SPD_DAT_BUSY_MSK	0x00000100
#define MSC01_MC_SPD_DAT_BUSY_BIT	MSC01_MC_SPD_DAT_BUSY_MSK
#define MSC01_MC_SPD_DAT_RDATA_SHF	0
#define MSC01_MC_SPD_DAT_RDATA_MSK	0x000000ff


/*****************************************************************************
 * Registers absolute addresses
 ****************************************************************************/

#define MSC01_MC_CFGPB0		(MSC01_MC_REG_BASE + MSC01_MC_CFGPB0_OFS)
#define MSC01_MC_CFGPB1		(MSC01_MC_REG_BASE + MSC01_MC_CFGPB1_OFS)
#define MSC01_MC_CFGPB2		(MSC01_MC_REG_BASE + MSC01_MC_CFGPB2_OFS)
#define MSC01_MC_CFGPB3		(MSC01_MC_REG_BASE + MSC01_MC_CFGPB3_OFS)
#define MSC01_MC_LATENCY	(MSC01_MC_REG_BASE + MSC01_MC_LATENCY_OFS)
#define MSC01_MC_IOCTRL		(MSC01_MC_REG_BASE + MSC01_MC_IOCTRL_OFS)
#define MSC01_MC_TIMPAR		(MSC01_MC_REG_BASE + MSC01_MC_TIMPAR_OFS)
#define MSC01_MC_TREFRESH	(MSC01_MC_REG_BASE + MSC01_MC_TREFRESH_OFS)
#define MSC01_MC_INITCMD	(MSC01_MC_REG_BASE + MSC01_MC_INITCMD_OFS)
#define MSC01_MC_CTRLENA	(MSC01_MC_REG_BASE + MSC01_MC_CTRLENA_OFS)
#define MSC01_MC_CMD_PD		(MSC01_MC_REG_BASE + MSC01_MC_CMD_PD_OFS)
#define MSC01_MC_ARB_CFG	(MSC01_MC_REG_BASE + MSC01_MC_ARB_CFG_OFS)
#define MSC01_MC_ARB_STAT	(MSC01_MC_REG_BASE + MSC01_MC_ARB_STAT_OFS)
#define MSC01_MC_HC_DDR		(MSC01_MC_REG_BASE + MSC01_MC_HC_DDR_OFS)
#define MSC01_MC_HC_FMDW	(MSC01_MC_REG_BASE + MSC01_MC_HC_FMDW_OFS)
#define MSC01_MC_HC_PARITY	(MSC01_MC_REG_BASE + MSC01_MC_HC_PARITY_OFS)
#define MSC01_MC_HC_CLKRAT	(MSC01_MC_REG_BASE + MSC01_MC_HC_CLKRAT_OFS)
#define MSC01_MC_HC_RDDEL	(MSC01_MC_REG_BASE + MSC01_MC_HC_RDDEL_OFS)
#define MSC01_MC_SPD_CFG	(MSC01_MC_REG_BASE + MSC01_MC_SPD_CFG_OFS)
#define MSC01_MC_SPD_ADR	(MSC01_MC_REG_BASE + MSC01_MC_SPD_ADR_OFS)
#define MSC01_MC_SPD_DAT	(MSC01_MC_REG_BASE + MSC01_MC_SPD_DAT_OFS)


#endif
/*****************************************************************************
 *  End of msc01_mc.h
 *****************************************************************************/
