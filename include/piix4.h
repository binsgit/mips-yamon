
/************************************************************************
 *
 *  piix4.h
 *
 *  Register definitions for Intel PIIX4 South Bridge Device
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

#ifndef PIIX4_H
#define PIIX4_H

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <pci.h>

/************************************************************************
 *  PCI registers (word indexed)
 ************************************************************************/

/* Function 0 (32 bit registers) */
#define PIIX4_PCI_GENCFG	0xb0
#define PIIX4_PCI_PIRQRC	0x60
/* Function 0 (8 bit registers)  */
#define PIIX4_PCI_SERIRQC	0x64

/* Function 1 (16 bit registers) */
#define PIIX4_PCI_IDETIM_PRIM	0x40
#define PIIX4_PCI_IDETIM_SEC	0x42

/* Function 3 (8 bit registers) */
#define PIIX4_PCI_TOM		0x69
#define PIIX4_PCI_PMREGMISC	0x80
#define PIIX4_PCI_SMBHCFG	0xd2

/************************************************************************
 *  IO register offsets
 ************************************************************************/

/* Function 0 (Bridge) */
#define PIIX4_ELCR1_OFS		0x4d0
#define PIIX4_ELCR2_OFS		0x4d1
#define PIIX4_ICW1M_OFS		0x20
#define PIIX4_ICW2M_OFS		0x21
#define PIIX4_ICW3M_OFS		0x21
#define PIIX4_ICW4M_OFS		0x21
#define PIIX4_ICW1S_OFS		0xa0
#define PIIX4_ICW2S_OFS		0xa1
#define PIIX4_ICW3S_OFS		0xa1
#define PIIX4_ICW4S_OFS		0xa1
#define PIIX4_OCW1M_OFS		0x21
#define PIIX4_OCW1S_OFS		0xa1
#define PIIX4_OCW2M_OFS		0x20
#define PIIX4_OCW2S_OFS		0xa0
#define PIIX4_OCW3M_OFS		0x20
#define PIIX4_OCW3S_OFS		0xa0
#define PIIX4_TMRCNT0_OFS	0x40
#define PIIX4_TMRCNT1_OFS	0x41
#define PIIX4_TMRCNT2_OFS	0x42
#define PIIX4_TCW_OFS		0x43

/* Function 3 (Power Management) */
#define PIIX4_PMSTS_OFS		0x00
#define PIIX4_PMCNTRL_OFS	0x04
#define PIIX4_GPSTS_OFS		0x0c
#define PIIX4_GPEN_OFS		0x0e
#define PIIX4_GLBSTS_OFS	0x18
#define PIIX4_GLBEN_OFS		0x20
#define PIIX4_GLBCTL_OFS	0x28

/* Function 3 (SMB)		 */
#define PIIX4_SMBHSTSTS_OFS	0x0
#define PIIX4_SMBHSTCNT_OFS	0x2
#define PIIX4_SMBHSTCMD_OFS	0x3
#define PIIX4_SMBHSTADD_OFS	0x4
#define PIIX4_SMBHSTDAT0_OFS	0x5


/************************************************************************
 *  Register encodings (PCI is little always little endian)
 ************************************************************************/

#define PIIX4_SMBHCFG_IS_SHF		1
#define PIIX4_SMBHCFG_IS_MSK		(MSK(3) << PIIX4_SMBHCFG_IS_SHF)
#define PIIX4_SMBHCFG_IS_SMI		0
#define PIIX4_SMBHCFG_IS_IRQ9		4

#define PIIX4_SMBHCFG_EN_SHF		0
#define PIIX4_SMBHCFG_EN_MSK		(MSK(1) << PIIX4_SMBHCFG_EN_SHF)
#define PIIX4_SMBHCFG_EN_BIT		PIIX4_SMBHCFG_EN_MSK


#define PIIX4_TOM_TOM_SHF		4
#define PIIX4_TOM_TOM_MSK		(MSK(4) << PIIX4_TOM_TOM_SHF)
#define PIIX4_TOM_TOM_1MB		0
#define PIIX4_TOM_TOM_2MB		1
#define PIIX4_TOM_TOM_3MB		2
#define PIIX4_TOM_TOM_4MB		3
#define PIIX4_TOM_TOM_5MB		4
#define PIIX4_TOM_TOM_6MB		5
#define PIIX4_TOM_TOM_7MB		6
#define PIIX4_TOM_TOM_8MB		7
#define PIIX4_TOM_TOM_9MB		8
#define PIIX4_TOM_TOM_10MB		9
#define PIIX4_TOM_TOM_11MB		10
#define PIIX4_TOM_TOM_12MB		11
#define PIIX4_TOM_TOM_13MB		12
#define PIIX4_TOM_TOM_14MB		13
#define PIIX4_TOM_TOM_15MB		14
#define PIIX4_TOM_TOM_16MB		15



#define PIIX4_SERIRQC_ENABLE_SHF	7
#define PIIX4_SERIRQC_ENABLE_MSK	(MSK(1) << PIIX4_SERIRQC_ENABLE_SHF)
#define PIIX4_SERIRQC_ENABLE_BIT	PIIX4_SERIRQC_ENABLE_MSK
#define PIIX4_SERIRQC_CONT_SHF		6
#define PIIX4_SERIRQC_CONT_MSK		(MSK(1) << PIIX4_SERIRQC_CONT_SHF)
#define PIIX4_SERIRQC_CONT_BIT		PIIX4_SERIRQC_CONT_MSK
#define PIIX4_SERIRQC_FS_SHF		2
#define PIIX4_SERIRQC_FS_MSK		(MSK(4) << PIIX4_SERIRQC_FS_SHF)
#define PIIX4_SERIRQC_FPW_SHF		0
#define PIIX4_SERIRQC_FPW_MSK		(MSK(2) << PIIX4_SERIRQC_FPW_SHF)


#define PIIX4_GENCFG_SERIRQ_SHF		16
#define PIIX4_GENCFG_SERIRQ_MSK		(MSK(1) << PIIX4_GENCFG_SERIRQ_SHF)
#define PIIX4_GENCFG_SERIRQ_BIT		PIIX4_GENCFG_SERIRQ_MSK

#define PIIX4_GENCFG_ISA_SHF		0
#define PIIX4_GENCFG_ISA_MSK		(MSK(1) << PIIX4_GENCFG_ISA_SHF)
#define PIIX4_GENCFG_ISA_BIT		PIIX4_GENCFG_ISA_MSK


#define PIIX4_PMREGMISC_ENABLE_SHF	0
#define PIIX4_PMREGMISC_ENABLE_MSK	(MSK(1) << PIIX4_PMREGMISC_ENABLE_SHF)
#define PIIX4_PMREGMISC_ENABLE_BIT	PIIX4_PMREGMISC_ENABLE_MSK


#define PIIX4_PIRQRC_PCIA_ID_SHF	7
#define PIIX4_PIRQRC_PCIA_ID_MSK	(MSK(1) << PIIX4_PIRQRC_PCIA_ID_SHF)
#define PIIX4_PIRQRC_PCIA_ID_BIT	PIIX4_PIRQRC_PCIA_ID_MSK

#define PIIX4_PIRQRC_PCIA_IR_SHF	0
#define PIIX4_PIRQRC_PCIA_IR_MSK	(MSK(4) << PIIX4_PIRQRC_PCIA_IR_SHF)
#define PIIX4_PIRQRC_PCIA_IR_IRQ3	3
#define PIIX4_PIRQRC_PCIA_IR_IRQ4	4
#define PIIX4_PIRQRC_PCIA_IR_IRQ5	5
#define PIIX4_PIRQRC_PCIA_IR_IRQ6	6
#define PIIX4_PIRQRC_PCIA_IR_IRQ7	7
#define PIIX4_PIRQRC_PCIA_IR_IRQ9	9
#define PIIX4_PIRQRC_PCIA_IR_IRQ10	10
#define PIIX4_PIRQRC_PCIA_IR_IRQ11	11
#define PIIX4_PIRQRC_PCIA_IR_IRQ12	12
#define PIIX4_PIRQRC_PCIA_IR_IRQ14	14
#define PIIX4_PIRQRC_PCIA_IR_IRQ15	15

#define PIIX4_PIRQRC_PCIB_ID_SHF	(7 + 8)
#define PIIX4_PIRQRC_PCIB_ID_MSK	(MSK(1) << PIIX4_PIRQRC_PCIB_ID_SHF)
#define PIIX4_PIRQRC_PCIB_ID_BIT	PIIX4_PIRQRC_PCIB_ID_MSK

#define PIIX4_PIRQRC_PCIB_IR_SHF	(0 + 8)
#define PIIX4_PIRQRC_PCIB_IR_MSK	(MSK(4) << PIIX4_PIRQRC_PCIB_IR_SHF)
#define PIIX4_PIRQRC_PCIB_IR_IRQ3	3
#define PIIX4_PIRQRC_PCIB_IR_IRQ4	4
#define PIIX4_PIRQRC_PCIB_IR_IRQ5	5
#define PIIX4_PIRQRC_PCIB_IR_IRQ6	6
#define PIIX4_PIRQRC_PCIB_IR_IRQ7	7
#define PIIX4_PIRQRC_PCIB_IR_IRQ9	9
#define PIIX4_PIRQRC_PCIB_IR_IRQ10	10
#define PIIX4_PIRQRC_PCIB_IR_IRQ11	11
#define PIIX4_PIRQRC_PCIB_IR_IRQ12	12
#define PIIX4_PIRQRC_PCIB_IR_IRQ14	14
#define PIIX4_PIRQRC_PCIB_IR_IRQ15	15

#define PIIX4_PIRQRC_PCIC_ID_SHF	(7 + 16)
#define PIIX4_PIRQRC_PCIC_ID_MSK	(MSK(1) << PIIX4_PIRQRC_PCIC_ID_SHF)
#define PIIX4_PIRQRC_PCIC_ID_BIT	PIIX4_PIRQRC_PCIC_ID_MSK

#define PIIX4_PIRQRC_PCIC_IR_SHF	(0 + 16)
#define PIIX4_PIRQRC_PCIC_IR_MSK	(MSK(4) << PIIX4_PIRQRC_PCIC_IR_SHF)
#define PIIX4_PIRQRC_PCIC_IR_IRQ3	3
#define PIIX4_PIRQRC_PCIC_IR_IRQ4	4
#define PIIX4_PIRQRC_PCIC_IR_IRQ5	5
#define PIIX4_PIRQRC_PCIC_IR_IRQ6	6
#define PIIX4_PIRQRC_PCIC_IR_IRQ7	7
#define PIIX4_PIRQRC_PCIC_IR_IRQ9	9
#define PIIX4_PIRQRC_PCIC_IR_IRQ10	10
#define PIIX4_PIRQRC_PCIC_IR_IRQ11	11
#define PIIX4_PIRQRC_PCIC_IR_IRQ12	12
#define PIIX4_PIRQRC_PCIC_IR_IRQ14	14
#define PIIX4_PIRQRC_PCIC_IR_IRQ15	15

#define PIIX4_PIRQRC_PCID_ID_SHF	(7 + 24)
#define PIIX4_PIRQRC_PCID_ID_MSK	(MSK(1) << PIIX4_PIRQRC_PCID_ID_SHF)
#define PIIX4_PIRQRC_PCID_ID_BIT	PIIX4_PIRQRC_PCID_ID_MSK

#define PIIX4_PIRQRC_PCID_IR_SHF	(0 + 24)
#define PIIX4_PIRQRC_PCID_IR_MSK	(MSK(4) << PIIX4_PIRQRC_PCID_IR_SHF)
#define PIIX4_PIRQRC_PCID_IR_IRQ3	3
#define PIIX4_PIRQRC_PCID_IR_IRQ4	4
#define PIIX4_PIRQRC_PCID_IR_IRQ5	5
#define PIIX4_PIRQRC_PCID_IR_IRQ6	6
#define PIIX4_PIRQRC_PCID_IR_IRQ7	7
#define PIIX4_PIRQRC_PCID_IR_IRQ9	9
#define PIIX4_PIRQRC_PCID_IR_IRQ10	10
#define PIIX4_PIRQRC_PCID_IR_IRQ11	11
#define PIIX4_PIRQRC_PCID_IR_IRQ12	12
#define PIIX4_PIRQRC_PCID_IR_IRQ14	14
#define PIIX4_PIRQRC_PCID_IR_IRQ15	15



#define PIIX4_ICW1_ICWSEL75_SHF		5
#define PIIX4_ICW1_ICWSEL75_MSK		(MSK(3) << PIIX4_ICW1_ICWSEL75_SHF)
#define PIIX4_ICW1_ICWSEL_SHF		4
#define PIIX4_ICW1_ICWSEL_MSK		(MSK(1) << PIIX4_ICW1_ICWSEL_SHF)
#define PIIX4_ICW1_ICWSEL_BIT		PIIX4_ICW1_ICWSEL_MSK
#define PIIX4_ICW1_SNGL_SHF		1
#define PIIX4_ICW1_SNGL_MSK		(MSK(1) << PIIX4_ICW1_SNGL_SHF)
#define PIIX4_ICW1_SNGL_BIT		PIIX4_ICW1_SNGL_MSK
#define PIIX4_ICW1_ICW4WR_SHF		0
#define PIIX4_ICW1_ICW4WR_MSK		(MSK(1) << PIIX4_ICW1_ICW4WR_SHF)
#define PIIX4_ICW1_ICW4WR_BIT		PIIX4_ICW1_ICW4WR_MSK


#define PIIX4_ICW2_BASE_SHF		3
#define PIIX4_ICW2_BASE_MSK		(MSK(5) << PIIX4_ICW2_BASE_SHF)


#define PIIX4_ICW3M_CAS_SHF		2
#define PIIX4_ICW3M_CAS_MSK		(MSK(1) << PIIX4_ICW3M_CAS_SHF)
#define PIIX4_ICW3M_CAS_BIT		PIIX4_ICW3M_CAS_MSK


#define PIIX4_ICW3S_SID_SHF		0
#define PIIX4_ICW3S_SID_MSK		(MSK(3) << PIIX4_ICW3S_SID_SHF)
#define PIIX4_ICW3S_SID_DEF		0x2


#define PIIX4_ICW4_SFNM_SHF		4
#define PIIX4_ICW4_SFNM_MSK		(MSK(1) << PIIX4_ICW4_SFNM_SHF)
#define PIIX4_ICW4_SFNM_BIT		PIIX4_ICW4_SFNM_MSK
#define PIIX4_ICW4_AEOI_SHF		1
#define PIIX4_ICW4_AEOI_MSK		(MSK(1) << PIIX4_ICW4_AEOI_SHF)
#define PIIX4_ICW4_UPMODE_SHF		0
#define PIIX4_ICW4_UPMODE_MSK		(MSK(1) << PIIX4_ICW4_UPMODE_SHF)
#define PIIX4_ICW4_UPMODE_BIT		PIIX4_ICW4_UPMODE_MSK


#define PIIX4_OCW2_CODE_SHF		5
#define PIIX4_OCW2_CODE_MSK		(MSK(3) << PIIX4_OCW2_CODE_SHF)
#define PIIX4_OCW2_CODE_NSEOI		0x1
#define PIIX4_OCW2_CODE_SEOI		0x3
#define PIIX4_OCW2_CODE_RNSEOI		0x5
#define PIIX4_OCW2_CODE_RAEOIS		0x4
#define PIIX4_OCW2_CODE_RAEOIC		0x0
#define PIIX4_OCW2_CODE_RSEOI		0x7
#define PIIX4_OCW2_CODE_SP		0x6

#define PIIX4_OCW2_OCWSEL_SHF		3
#define PIIX4_OCW2_OCWSEL_MSK		(MSK(2) << PIIX4_OCW2_OCWSEL_SHF)
#define PIIX4_OCW2_OCWSEL_2		0

#define PIIX4_OCW2_ILS_SHF		0
#define PIIX4_OCW2_ILS_MSK		(MSK(3) << PIIX4_OCW2_ILS_SHF)
#define PIIX4_OCW2_ILS_0_8		0
#define PIIX4_OCW2_ILS_1_9		1
#define PIIX4_OCW2_ILS_2_10		2
#define PIIX4_OCW2_ILS_3_11		3
#define PIIX4_OCW2_ILS_4_12		4
#define PIIX4_OCW2_ISL_5_13		5
#define PIIX4_OCW2_ISL_6_14		6
#define PIIX4_OCW2_ISL_7_15		7


#define PIIX4_OCW3_SMM_SHF		6
#define PIIX4_OCW3_SMM_MSK		(MSK(1) << PIIX4_OCW3_SMM_SHF)
#define PIIX4_OCW3_SMM_BIT		PIIX4_OCW3_SMM_MSK

#define PIIX4_OCW3_ESMM_SHF		5
#define PIIX4_OCW3_ESMM_MSK		(MSK(1) << PIIX4_OCW3_ESMM_SHF)
#define PIIX4_OCW3_ESMM_BIT		PIIX4_OCW3_ESMM_MSK

#define PIIX4_OCW3_OCWSEL_SHF		3
#define PIIX4_OCW3_OCWSEL_MSK		(MSK(2) << PIIX4_OCW3_OCWSEL_SHF)
#define PIIX4_OCW3_OCWSEL_3		0x1

#define PIIX4_OCW3_PMC_SHF		2
#define PIIX4_OCW3_PMC_MSK		(MSK(1) << PIIX4_OCW3_PMC_SHF)
#define PIIX4_OCW3_PMC_BIT		PIIX4_OCW3_PMC_MSK

#define PIIX4_OCW3_RRC_SHF		0
#define PIIX4_OCW3_RRC			(MSK(2) << PIIX4_OCW3_RRC_SHF)
#define PIIX4_OCW3_RRC_IRR		0x2
#define PIIX4_OCW3_RRC_ISR		0x3


#define PIIX4_ELCR1_IRQ7LEVEL_SHF	7
#define PIIX4_ELCR1_IRQ7LEVEL_MSK	(MSK(1) << PIIX4_ELCR1_IRQ7LEVEL_SHF)
#define PIIX4_ELCR1_IRQ7LEVEL_BIT	PIIX4_ELCR1_IRQ7LEVEL_MSK

#define PIIX4_ELCR1_IRQ6LEVEL_SHF	6
#define PIIX4_ELCR1_IRQ6LEVEL_MSK	(MSK(1) << PIIX4_ELCR1_IRQ6LEVEL_SHF)
#define PIIX4_ELCR1_IRQ6LEVEL_BIT	PIIX4_ELCR1_IRQ6LEVEL_MSK

#define PIIX4_ELCR1_IRQ5LEVEL_SHF	5
#define PIIX4_ELCR1_IRQ5LEVEL_MSK	(MSK(1) << PIIX4_ELCR1_IRQ5LEVEL_SHF)
#define PIIX4_ELCR1_IRQ5LEVEL_BIT	PIIX4_ELCR1_IRQ5LEVEL_MSK

#define PIIX4_ELCR1_IRQ4LEVEL_SHF	4
#define PIIX4_ELCR1_IRQ4LEVEL_MSK	(MSK(1) << PIIX4_ELCR1_IRQ4LEVEL_SHF)
#define PIIX4_ELCR1_IRQ4LEVEL_BIT	PIIX4_ELCR1_IRQ4LEVEL_MSK

#define PIIX4_ELCR1_IRQ3LEVEL_SHF	3
#define PIIX4_ELCR1_IRQ3LEVEL_MSK	(MSK(1) << PIIX4_ELCR1_IRQ3LEVEL_SHF)
#define PIIX4_ELCR1_IRQ3LEVEL_BIT	PIIX4_ELCR1_IRQ3LEVEL_MSK


#define PIIX4_ELCR2_IRQ15LEVEL_SHF	7
#define PIIX4_ELCR2_IRQ15LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ15LEVEL_SHF)
#define PIIX4_ELCR2_IRQ15LEVEL_BIT	PIIX4_ELCR2_IRQ15LEVEL_MSK

#define PIIX4_ELCR2_IRQ14LEVEL_SHF	6
#define PIIX4_ELCR2_IRQ14LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ14LEVEL_SHF)
#define PIIX4_ELCR2_IRQ14LEVEL_BIT	PIIX4_ELCR2_IRQ14LEVEL_MSK

#define PIIX4_ELCR2_IRQ12LEVEL_SHF	4
#define PIIX4_ELCR2_IRQ12LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ12LEVEL_SHF)
#define PIIX4_ELCR2_IRQ12LEVEL_BIT	PIIX4_ELCR2_IRQ12LEVEL_MSK

#define PIIX4_ELCR2_IRQ11LEVEL_SHF	3
#define PIIX4_ELCR2_IRQ11LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ11LEVEL_SHF)
#define PIIX4_ELCR2_IRQ11LEVEL_BIT	PIIX4_ELCR2_IRQ11LEVEL_MSK

#define PIIX4_ELCR2_IRQ10LEVEL_SHF	2
#define PIIX4_ELCR2_IRQ10LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ10LEVEL_SHF)
#define PIIX4_ELCR2_IRQ10LEVEL_BIT	PIIX4_ELCR2_IRQ10LEVEL_MSK

#define PIIX4_ELCR2_IRQ9LEVEL_SHF	1
#define PIIX4_ELCR2_IRQ9LEVEL_MSK	(MSK(1) << PIIX4_ELCR2_IRQ9LEVEL_SHF)
#define PIIX4_ELCR2_IRQ9LEVEL_BIT	PIIX4_ELCR2_IRQ9LEVEL_MSK


/* Timer countrol word */

#define PIIX4_TCW_CS_SHF		6
#define PIIX4_TCW_CS_MSK		(MSK(2) << PIIX4_TCW_CS_SHF)
#define PIIX4_TCW_CS_0			0
#define PIIX4_TCW_CS_1			1
#define PIIX4_TCW_CS_2			2
#define PIIX4_TCW_CS_RB			3

#define PIIX4_TCW_RW_SHF		4
#define PIIX4_TCW_RW_MSK		(MSK(2) << PIIX4_TCW_RW_SHF)
#define PIIX4_TCW_RW_CL			0
#define PIIX4_TCW_RW_LSB		1
#define PIIX4_TCW_RW_MSB		2
#define PIIX4_TCW_RW_16			3

#define PIIX4_TCW_MODE_SHF		1
#define PIIX4_TCW_MODE_MSK		(MSK(3) << PIIX4_TCW_MODE_SHF)
#define PIIX4_TCW_MODE_0		0
#define PIIX4_TCW_MODE_1		1
#define PIIX4_TCW_MODE_2		2
#define PIIX4_TCW_MODE_3		3
#define PIIX4_TCW_MODE_4		4
#define PIIX4_TCW_MODE_5		5

#define PIIX4_TCW_BCD_SHF		0
#define PIIX4_TCW_BCD_MSK		(MSK(1) << PIIX4_TCW_BCD_SHF)
#define PIIX4_TCW_BCD_BIT		PIIX4_TCW_BCD_MSK


/* Power Management Control Register */
#define PIIX4_PMCNTRL_SE_SHF	        13
#define PIIX4_PMCNTRL_SE_MSK		(MSK(1) << PIIX4_PMCNTRL_SE_SHF)
#define PIIX4_PMCNTRL_SE_BIT		PIIX4_PMCNTRL_SE_MSK

#define PIIX4_PMCNTRL_TYPE_SHF		10
#define PIIX4_PMCNTRL_TYPE_MSK		(MSK(3) << PIIX4_PMCNTRL_TYPE_SHF)
#define PIIX4_PMCNTRL_TYPE_STD		0
#define PIIX4_PMCNTRL_TYPE_STR		1
#define PIIX4_PMCNTRL_TYPE_POSCL	2
#define PIIX4_PMCNTRL_TYPE_POSCCL	3
#define PIIX4_PMCNTRL_TYPE_POS		4
#define PIIX4_PMCNTRL_TYPE_WORK		5


/* Global Control Register */
#define PIIX4_GLBCTL_LID_POL_SHF	25
#define PIIX4_GLBCTL_LID_POL_MSK	(MSK(1) << PIIX4_GLBCTL_LID_POL_SHF)
#define PIIX4_GLBCTL_LID_POL_BIT	PIIX4_GLBCTL_LID_POL_MSK

#define PIIX4_GLBCTL_EOS_SHF		16
#define PIIX4_GLBCTL_EOS_MSK		(MSK(1) << PIIX4_GLBCTL_EOS_SHF)
#define PIIX4_GLBCTL_EOS_BIT		PIIX4_GLBCTL_EOS_MSK

#define PIIX4_GLBCTL_GSTBY_CNT_SHF	9
#define PIIX4_GLBCTL_GSTBY_CNT_MSK	(MSK(7) << PIIX4_GLBCTL_GSTBY_CNT_SHF)

#define PIIX4_GLBCTL_THRM_POL_SHF	2
#define PIIX4_GLBCTL_THRM_POL_MSK	(MSK(1) << PIIX4_GLBCTL_THRM_POL_SHF)
#define PIIX4_GLBCTL_THRM_POL_BIT	PIIX4_GLBCTL_THRM_POL_MSK

#define PIIX4_GLBCTL_SMI_EN_SHF		0
#define PIIX4_GLBCTL_SMI_EN_MSK		(MSK(1) << PIIX4_GLBCTL_SMI_EN_SHF)
#define PIIX4_GLBCTL_SMI_EN_BIT		PIIX4_GLBCTL_SMI_EN_MSK


/* SMB Host Control register */
#define PIIX4_SMBHSTCNT_START_SHF	6
#define PIIX4_SMBHSTCNT_START_MSK	(MSK(1) << PIIX4_SMBHSTCNT_START_SHF)
#define PIIX4_SMBHSTCNT_START_BIT	PIIX4_SMBHSTCNT_START_MSK

#define PIIX4_SMBHSTCNT_CP_SHF		2
#define PIIX4_SMBHSTCNT_CP_MSK		(MSK(3) << PIIX4_SMBHSTCNT_CP_SHF)
#define PIIX4_SMBHSTCNT_CP_QRW		0
#define PIIX4_SMBHSTCNT_CP_BRW		1
#define PIIX4_SMBHSTCNT_CP_BDRW		2
#define PIIX4_SMBHSTCNT_CP_WDRW		3
#define PIIX4_SMBHSTCNT_CP_BLRW		5

#define PIIX4_SMBHSTCNT_KILL_SHF	1
#define PIIX4_SMBHSTCNT_KILL_MSK	(MSK(1) << PIIX4_SMBHSTCNT_KILL_SHF)
#define PIIX4_SMBHSTCNT_KILL_BIT	PIIX4_SMBHSTCNT_KILL_MSK

#define PIIX4_SMBHSTCNT_IE_SHF		0
#define PIIX4_SMBHSTCNT_IE_MSK		(MSK(1) << PIIX4_SMBHSTCNT_IE_SHF)
#define PIIX4_SMBHSTCNT_IE_BIT		PIIX4_SMBHSTCNT_IE_MSK

/* SMB Host Status register */
#define PIIX4_SMBHSTSTS_FAILED_SHF	4
#define PIIX4_SMBHSTSTS_FAILED_MSK	(MSK(1) << PIIX4_SMBHSTSTS_FAILED_SHF)
#define PIIX4_SMBHSTSTS_FAILED_BIT	PIIX4_SMBHSTSTS_FAILED_MSK

#define PIIX4_SMBHSTSTS_COL_SHF		3
#define PIIX4_SMBHSTSTS_COL_MSK		(MSK(1) << PIIX4_SMBHSTSTS_COL_SHF)
#define PIIX4_SMBHSTSTS_COL_BIT		PIIX4_SMBHSTSTS_COL_MSK

#define PIIX4_SMBHSTSTS_DE_SHF		2
#define PIIX4_SMBHSTSTS_DE_MSK		(MSK(1) << PIIX4_SMBHSTSTS_DE_SHF)
#define PIIX4_SMBHSTSTS_DE_BIT		PIIX4_SMBHSTSTS_DE_MSK

#define PIIX4_SMBHSTSTS_INT_SHF		1
#define PIIX4_SMBHSTSTS_INT_MSK		(MSK(1) << PIIX4_SMBHSTSTS_INT_SHF)
#define PIIX4_SMBHSTSTS_INT_BIT		PIIX4_SMBHSTSTS_INT_MSK

#define PIIX4_SMBHSTSTS_BUSY_SHF	0
#define PIIX4_SMBHSTSTS_BUSY_MSK	(MSK(1) << PIIX4_SMBHSTSTS_BUSY_SHF)
#define PIIX4_SMBHSTSTS_BUSY_BIT	PIIX4_SMBHSTSTS_BUSY_MSK

/* Host Address register */
#define PIIX4_SMBHSTADD_ADDR_SHF	1
#define PIIX4_SMBHSTADD_ADDR_MSK	(MSK(7) << PIIX4_SMBHSTADD_ADDR_SHF)

#define PIIX4_SMBHSTADD_READ_SHF	0
#define PIIX4_SMBHSTADD_READ_MSK	(MSK(1) << PIIX4_SMBHSTADD_READ_SHF)
#define PIIX4_SMBHSTADD_READ_BIT	PIIX4_SMBHSTADD_READ_MSK


/* IDE Timing register */
#define PIIX4_IDETIM_IDE_SHF	 	15
#define PIIX4_IDETIM_IDE_MSK		(MSK(1) << PIIX4_IDETIM_IDE_SHF)
#define PIIX4_IDETIM_IDE_BIT		PIIX4_IDETIM_IDE_MSK


/* General Purpose Enable Register */

#define PIIX4_GPEN_GPIEN_SHF		9
#define PIIX4_GPEN_GPIEN_MSK		(MSK(1) << PIIX4_GPEN_GPIEN_SHF)
#define PIIX4_GPEN_GPIEN_BIT		PIIX4_GPEN_GPIEN_MSK



/************************************************************************
 *  Misc
 ************************************************************************/

/* PIIX4 is a 4-function PCI device */
#define PIIX4_PCI_FUNCTION_BRIDGE	0
#define PIIX4_PCI_FUNCTION_IDE		1
#define PIIX4_PCI_FUNCTION_USB		2
#define PIIX4_PCI_FUNCTION_POWER	3

/* BAR positions for fixed BARs */
#define PIIX4_PCI_BAR_BRIDGE_POS	PCI_BAR(0)
#define PIIX4_PCI_BAR_POWER_POS		0x40
#define PIIX4_PCI_BAR_SMB_POS		0x90

/* Fixed I/O range mapping */
#define PIIX4_IO_BRIDGE_START		0
#define PIIX4_IO_BRIDGE_SIZE		0x1000
#define PIIX4_IO_POWER_START		0x1000
#define PIIX4_IO_POWER_SIZE		0x100
#define PIIX4_IO_SMB_START		0x1100
#define PIIX4_IO_SMB_SIZE		0x100

/* Number of IRQs supported */
#define PIIX4_IRQ_COUNT			16

/* Timer period (ns) */
#define PIIX4_TIMER_PERIOD_NS		838


#endif /* #ifndef PIIX4_H */





