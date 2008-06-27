/* 
 * GIC register definitions
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
 */

#include <sysdefs.h>
#include <mips.h>

#undef  GICISBYTELITTLEENDIAN

#define GIC_BASE_ADDR			0x1bdc0000

/* Accessors */
#define GIC_REG(segment, offset) \
	REG32(KSEG1(GIC_BASE_ADDR) + segment##_SECTION_OFS + offset##_OFS)
#define GIC_REG_ADDR(segment, offset) \
	REG32(KSEG1(GIC_BASE_ADDR) + segment##_SECTION_OFS + offset)


#ifdef  GICISBYTELITTLEENDIAN
#define GICREAD(reg, data)	(data) = (reg), (data) = LE32_TO_CPU(data)
#define GICWRITE(reg, data)	(reg) = CPU_TO_LE32(data)
#else
#define GICREAD(reg, data)	(data) = (reg)
#define GICWRITE(reg, data)	(reg) = (data)
#endif

/* GIC Address Space */
#define SHARED_SECTION_OFS		0x0000
#define SHARED_SECTION_SIZE		0x8000
#define VPE_LOCAL_SECTION_OFS		0x8000
#define VPE_LOCAL_SECTION_SIZE		0x4000
#define VPE_OTHER_SECTION_OFS		0xc000
#define VPE_OTHER_SECTION_SIZE		0x4000
#define USM_VISIBLE_SECTION_OFS		0x10000
#define USM_VISIBLE_SECTION_SIZE	0x10000

/* Register Map for Shared Section */

#define	GIC_SH_CONFIG_OFS		0x0000

/* Shared Global Counter */
#define GIC_SH_COUNTER_31_00_OFS	0x0010 
#define GIC_SH_COUNTER_63_32_OFS	0x0014

#define GIC_SH_REVISIONID_OFS		0x0020

/* Interrupt Polarity */
#define GIC_SH_POL_31_0_OFS		0x0100 
#define GIC_SH_POL_63_32_OFS		0x0104
#define GIC_SH_POL_95_64_OFS		0x0108
#define GIC_SH_POL_127_96_OFS		0x010c
#define GIC_SH_POL_159_128_OFS		0x0110
#define GIC_SH_POL_191_160_OFS		0x0114
#define GIC_SH_POL_223_192_OFS		0x0118
#define GIC_SH_POL_255_224_OFS		0x011c

/* Edge/Level Triggering */
#define GIC_SH_TRIG_31_0_OFS		0x0180 
#define GIC_SH_TRIG_63_32_OFS		0x0184
#define GIC_SH_TRIG_95_64_OFS		0x0188
#define GIC_SH_TRIG_127_96_OFS		0x018c
#define GIC_SH_TRIG_159_128_OFS		0x0190
#define GIC_SH_TRIG_191_160_OFS		0x0194
#define GIC_SH_TRIG_223_192_OFS		0x0198
#define GIC_SH_TRIG_255_224_OFS		0x019c

/* Dual Edge Triggering */
#define GIC_SH_DUAL_31_0_OFS		0x0200 
#define GIC_SH_DUAL_63_32_OFS		0x0204
#define GIC_SH_DUAL_95_64_OFS		0x0208
#define GIC_SH_DUAL_127_96_OFS		0x020c
#define GIC_SH_DUAL_159_128_OFS		0x0210
#define GIC_SH_DUAL_191_160_OFS		0x0214
#define GIC_SH_DUAL_223_192_OFS		0x0218
#define GIC_SH_DUAL_255_224_OFS		0x021c

/* Set/Clear corresponding bit in Edge Detect Register */
#define GIC_SH_WEDGE_OFS		0x0280 

/* Reset Mask - Disables Interrupt */
#define GIC_SH_RMASK_31_0_OFS		0x0300 
#define GIC_SH_RMASK_63_32_OFS		0x0304
#define GIC_SH_RMASK_95_64_OFS		0x0308
#define GIC_SH_RMASK_127_96_OFS		0x030c
#define GIC_SH_RMASK_159_128_OFS	0x0310
#define GIC_SH_RMASK_191_160_OFS	0x0314
#define GIC_SH_RMASK_223_192_OFS	0x0318
#define GIC_SH_RMASK_255_224_OFS	0x031c

/* Set Mask (WO) - Enables Interrupt */
#define GIC_SH_SMASK_31_0_OFS		0x0380 
#define GIC_SH_SMASK_63_32_OFS		0x0384
#define GIC_SH_SMASK_95_64_OFS		0x0388
#define GIC_SH_SMASK_127_96_OFS		0x038c
#define GIC_SH_SMASK_159_128_OFS	0x0390
#define GIC_SH_SMASK_191_160_OFS	0x0394
#define GIC_SH_SMASK_223_192_OFS	0x0398
#define GIC_SH_SMASK_255_224_OFS	0x039c

/* Global Interrupt Mask Register (RO) - Bit Set == Interrupt enabled */
#define GIC_SH_MASK_31_0_OFS		0x0400 
#define GIC_SH_MASK_63_32_OFS		0x0404
#define GIC_SH_MASK_95_64_OFS		0x0408
#define GIC_SH_MASK_127_96_OFS		0x040c
#define GIC_SH_MASK_159_128_OFS		0x0410
#define GIC_SH_MASK_191_160_OFS		0x0414
#define GIC_SH_MASK_223_192_OFS		0x0418
#define GIC_SH_MASK_255_224_OFS		0x041c

/* Pending Global Interrupts (RO) */
#define GIC_SH_PEND_31_0_OFS		0x0480 
#define GIC_SH_PEND_63_32_OFS		0x0484
#define GIC_SH_PEND_95_64_OFS		0x0488
#define GIC_SH_PEND_127_96_OFS		0x048c
#define GIC_SH_PEND_159_128_OFS		0x0490
#define GIC_SH_PEND_191_160_OFS		0x0494
#define GIC_SH_PEND_223_192_OFS		0x0498
#define GIC_SH_PEND_255_224_OFS		0x049c

#define GIC_SH_INTR_MAP_TO_PIN_BASE_OFS	0x0500

/* Maps Interrupt X to a Pin */
#define GIC_SH_MAP_TO_PIN(intr) \
	(GIC_SH_INTR_MAP_TO_PIN_BASE_OFS + (4 * intr)) 

#define GIC_SH_INTR_MAP_TO_VPE_BASE_OFS	0x2000

/* Maps Interrupt X to a VPE */
#define GIC_SH_MAP_TO_VPE_REG_OFF(intr, vpe) \
	(GIC_SH_INTR_MAP_TO_VPE_BASE_OFS + (32 * (intr)) + (((vpe) / 32) * 4))
#define GIC_SH_MAP_TO_VPE_REG_BIT(vpe)	(1 << ((vpe) % 32))

/* Convert an interrupt number to a byte offset/bit for multi-word registers */
#define GIC_INTR_OFS(intr) (((intr) / 32)*4)
#define GIC_INTR_BIT(intr) ((intr) % 32)

/* Register Map for Local Section */
#define GIC_VPE_CTL_OFS			0x0000
#define GIC_VPE_PEND_OFS		0x0004
#define GIC_VPE_MASK_OFS		0x0008
#define GIC_VPE_RMASK_OFS		0x000c
#define GIC_VPE_SMASK_OFS		0x0010
#define GIC_VPE_WD_MAP_OFS		0x0040
#define GIC_VPE_COMPARE_MAP_OFS		0x0044
#define GIC_VPE_TIMER_MAP_OFS		0x0048
#define GIC_VPE_PERFCTR_MAP_OFS		0x0050
#define GIC_VPE_SWINT0_MAP_OFS		0x0054
#define GIC_VPE_SWINT1_MAP_OFS		0x0058
#define GIC_VPE_OTHER_ADDR_OFS		0x0080
#define GIC_VPE_WD_CONFIG0_OFS		0x0090
#define GIC_VPE_WD_COUNT0_OFS		0x0094
#define GIC_VPE_WD_INITIAL0_OFS		0x0098
#define GIC_VPE_COMPARE_LO_OFS		0x00a0
#define GIC_VPE_COMPARE_HI		0x00a4

#define GIC_VPE_EIC_SHADOW_SET_BASE	0x0100
#define GIC_VPE_EIC_SS(intr) \
	(GIC_EIC_SHADOW_SET_BASE + (4 * intr))

#define GIC_VPE_EIC_VEC_BASE		0x0800
#define GIC_VPE_EIC_VEC(intr) \
	(GIC_VPE_EIC_VEC_BASE + (4 * intr))

#define GIC_VPE_TENABLE_NMI_OFS		0x1000
#define GIC_VPE_TENABLE_YQ_OFS		0x1004
#define GIC_VPE_TENABLE_INT_31_0_OFS	0x1080
#define GIC_VPE_TENABLE_INT_63_32_OFS	0x1084

/* User Mode Visible Section Register Map */
#define GIC_UMV_SH_COUNTER_31_00_OFS	0x0000
#define GIC_UMV_SH_COUNTER_63_32_OFS	0x0004

/* Masks */
#define GIC_SH_CONFIG_COUNTSTOP_SHF	28
#define GIC_SH_CONFIG_COUNTSTOP_MSK	(MSK(1) << GIC_SH_CONFIG_COUNTSTOP_SHF)

#define GIC_SH_CONFIG_COUNTBITS_SHF	24
#define GIC_SH_CONFIG_COUNTBITS_MSK	(MSK(4) << GIC_SH_CONFIG_COUNTBITS_SHF)

#define GIC_SH_CONFIG_NUMINTRS_SHF	16
#define GIC_SH_CONFIG_NUMINTRS_MSK	(MSK(8) << GIC_SH_CONFIG_NUMINTRS_SHF)

#define GIC_SH_CONFIG_NUMVPES_SHF	0
#define GIC_SH_CONFIG_NUMVPES_MSK	(MSK(8) << GIC_SH_CONFIG_NUMVPES_SHF)

#define GIC_SH_WEDGE_SET(intr)		(intr | (0x1 << 31))
#define GIC_SH_WEDGE_CLR(intr)		(intr & ~(0x1 << 31))

#define GIC_MAP_TO_PIN_SHF		31
#define GIC_MAP_TO_PIN_MSK		(MSK(1) << GIC_MAP_TO_PIN_SHF)
#define GIC_MAP_TO_NMI_SHF		30
#define GIC_MAP_TO_NMI_MSK		(MSK(1) << GIC_MAP_TO_NMI_SHF)
#define GIC_MAP_TO_YQ_SHF		29
#define GIC_MAP_TO_YQ_MSK		(MSK(1) << GIC_MAP_TO_YQ_SHF)
#define GIC_MAP_SHF			0
#define GIC_MAP_MSK			(MSK(6) << GIC_MAP_SHF)

/* GIC_VPE_CTL Masks */
#define GIC_VPE_CTL_PERFCNT_RTBL_SHF	2
#define GIC_VPE_CTL_PERFCNT_RTBL_MSK	(MSK(1) << GIC_VPE_CTL_PERFCNT_RTBL_SHF)
#define GIC_VPE_CTL_TIMER_RTBL_SHF	1
#define GIC_VPE_CTL_TIMER_RTBL_MSK	(MSK(1) << GIC_VPE_CTL_TIMER_RTBL_SHF)
#define GIC_VPE_CTL_EIC_MODE_SHF	0
#define GIC_VPE_CTL_EIC_MODE_MSK	(MSK(1) << GIC_VPE_CTL_EIC_MODE_SHF)

/* GIC_VPE_PEND Masks */
#define GIC_VPE_PEND_WD_SHF		0
#define GIC_VPE_PEND_WD_MSK		(MSK(1) << GIC_VPE_PEND_WD_SHF)
#define GIC_VPE_PEND_CMP_SHF		1
#define GIC_VPE_PEND_CMP_MSK		(MSK(1) << GIC_VPE_PEND_CMP_SHF)
#define GIC_VPE_PEND_TIMER_SHF		2
#define GIC_VPE_PEND_TIMER_MSK		(MSK(1) << GIC_VPE_PEND_TIMER_SHF)
#define GIC_VPE_PEND_PERFCOUNT_SHF	3
#define GIC_VPE_PEND_PERFCOUNT_MSK	(MSK(1) << GIC_VPE_PEND_PERFCOUNT_SHF)
#define GIC_VPE_PEND_SWINT0_SHF		4
#define GIC_VPE_PEND_SWINT0_MSK		(MSK(1) << GIC_VPE_PEND_SWINT0_SHF)
#define GIC_VPE_PEND_SWINT1_SHF		5
#define GIC_VPE_PEND_SWINT1_MSK		(MSK(1) << GIC_VPE_PEND_SWINT1_SHF)

/* GIC_VPE_RMASK Masks */
#define GIC_VPE_RMASK_WD_SHF		0
#define GIC_VPE_RMASK_WD_MSK		(MSK(1) << GIC_VPE_RMASK_WD_SHF)
#define GIC_VPE_RMASK_CMP_SHF		1
#define GIC_VPE_RMASK_CMP_MSK		(MSK(1) << GIC_VPE_RMASK_CMP_SHF)
#define GIC_VPE_RMASK_TIMER_SHF		2
#define GIC_VPE_RMASK_TIMER_MSK		(MSK(1) << GIC_VPE_RMASK_TIMER_SHF)
#define GIC_VPE_RMASK_PERFCNT_SHF	3
#define GIC_VPE_RMASK_PERFCNT_MSK	(MSK(1) << GIC_VPE_RMASK_PERFCNT_SHF)
#define GIC_VPE_RMASK_SWINT0_SHF	4
#define GIC_VPE_RMASK_SWINT0_MSK	(MSK(1) << GIC_VPE_RMASK_SWINT0_SHF)
#define GIC_VPE_RMASK_SWINT1_SHF	5
#define GIC_VPE_RMASK_SWINT1_MSK	(MSK(1) << GIC_VPE_RMASK_SWINT1_SHF)

/* GIC_VPE_SMASK Masks */
#define GIC_VPE_SMASK_WD_SHF		0
#define GIC_VPE_SMASK_WD_MSK		(MSK(1) << GIC_VPE_SMASK_WD_SHF)
#define GIC_VPE_SMASK_CMP_SHF		1
#define GIC_VPE_SMASK_CMP_MSK		(MSK(1) << GIC_VPE_SMASK_CMP_SHF)
#define GIC_VPE_SMASK_TIMER_SHF		2
#define GIC_VPE_SMASK_TIMER_MSK		(MSK(1) << GIC_VPE_SMASK_TIMER_SHF)
#define GIC_VPE_SMASK_PERFCNT_SHF	3
#define GIC_VPE_SMASK_PERFCNT_MSK	(MSK(1) << GIC_VPE_SMASK_PERFCNT_SHF)
#define GIC_VPE_SMASK_SWINT0_SHF	4
#define GIC_VPE_SMASK_SWINT0_MSK	(MSK(1) << GIC_VPE_SMASK_SWINT0_SHF)
#define GIC_VPE_SMASK_SWINT1_SHF	5
#define GIC_VPE_SMASK_SWINT1_MSK	(MSK(1) << GIC_VPE_SMASK_SWINT1_SHF)

/*
 * Prototypes of functions used externally
 */
int gic_init(void);
int gic_probe(void);
