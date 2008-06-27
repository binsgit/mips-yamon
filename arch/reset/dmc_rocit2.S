/*
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
#include <init.h>
#include <spd.h>
#include <pci_api.h>
#include <pci.h>
#include <product.h>
#include <core_sys.h>

#include <socitsc_dendefine.h>
#include <socitsc_reginit.h>
#include <socitsc_reginit_sdr.h>
#include <socitsc_reginit_ddr.h>
#include <socitsc.h>

	.set	noreorder

#define SYSCTRL_DDR_SUPPORT /* FIXME :( */

#define RA		s0
#define FREQ		s1
#define DMC_REGS	s2
#define RAMSIZE		s3
#define CSNUM           s4
#define ROWS		s5
#define COLUMNS		s6
#define RA2		s7
#define ROWSB		s8
#define COLUMNSB	a3

/************************************************************
** ROC-it memory controller config
*/
#define INIT_DENALI_SDR_REG(reg)		\
	li	t0, SC_DENALI_SDR_REG_##reg##_VAL;	\
	sw	t0, (4 * reg)(DMC_REGS)

#ifdef SYSCTRL_DDR_SUPPORT
#define INIT_DENALI_DDR_REG(reg)		\
	li	t0, SC_DENALI_DDR_REG_##reg##_VAL;	\
	sw	t0, (4 * reg)(DMC_REGS)
#endif

#define DMC_GET_REG_FIELD(reg, width, offset)	\
	lw	t0, (4 * SC_##reg)(DMC_REGS); 	\
	srl	t0, SC_##offset; 		\
	li	t1, ((1 << SC_##width) - 1);	\
	and	t0, t1

#define DMC_SET_REG_FIELD(reg, width, offset)	\
	lw	t1, (4 * SC_##reg)(DMC_REGS);	\
	sll	t0, SC_##offset;		\
	li	t2, (((1 << SC_##width) -1) << SC_##offset);\
	and	t0, t2;				\
	not	t2, t2;				\
	and	t1, t2;				\
	or	t1, t0;				\
	sw	t1, 4 * SC_##reg(DMC_REGS)


LEAF(dmc_rocit2_config)
	/* Get memory controller base address */
	li	DMC_REGS, MSC01_MC_REG_BASE

#ifdef SYSCTRL_DDR_SUPPORT
	/* ... read and check basic RAM type ... */
	jal    read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	beq	v0, t0, 1f
	 nop
#endif

	/* Do basic initialisation of all relevant registers */
#if SC_DENALI_SDR_REG_0_USED == 1
	INIT_DENALI_SDR_REG(0)
#endif
#if SC_DENALI_SDR_REG_1_USED == 1
	INIT_DENALI_SDR_REG(1)
#endif
#if SC_DENALI_SDR_REG_2_USED == 1
	INIT_DENALI_SDR_REG(2)
#endif
#if SC_DENALI_SDR_REG_3_USED == 1
	INIT_DENALI_SDR_REG(3)
#endif
#if SC_DENALI_SDR_REG_4_USED == 1
	INIT_DENALI_SDR_REG(4)
#endif
#if SC_DENALI_SDR_REG_5_USED == 1
	INIT_DENALI_SDR_REG(5)
#endif
#if SC_DENALI_SDR_REG_6_USED == 1
	INIT_DENALI_SDR_REG(6)
#endif
#if SC_DENALI_SDR_REG_7_USED == 1
	INIT_DENALI_SDR_REG(7)
#endif
#if SC_DENALI_SDR_REG_8_USED == 1
	INIT_DENALI_SDR_REG(8)
#endif
#if SC_DENALI_SDR_REG_9_USED == 1
	INIT_DENALI_SDR_REG(9)
#endif
#if SC_DENALI_SDR_REG_10_USED == 1
	INIT_DENALI_SDR_REG(10)
#endif
#if SC_DENALI_SDR_REG_11_USED == 1
	INIT_DENALI_SDR_REG(11)
#endif
#if SC_DENALI_SDR_REG_12_USED == 1
	INIT_DENALI_SDR_REG(12)
#endif
#if SC_DENALI_SDR_REG_13_USED == 1
	INIT_DENALI_SDR_REG(13)
#endif
#if SC_DENALI_SDR_REG_14_USED == 1
	INIT_DENALI_SDR_REG(14)
#endif
#if SC_DENALI_SDR_REG_15_USED == 1
	INIT_DENALI_SDR_REG(15)
#endif
#if SC_DENALI_SDR_REG_16_USED == 1
	INIT_DENALI_SDR_REG(16)
#endif
#if SC_DENALI_SDR_REG_17_USED == 1
	INIT_DENALI_SDR_REG(17)
#endif
#if SC_DENALI_SDR_REG_18_USED == 1
	INIT_DENALI_SDR_REG(18)
#endif
#if SC_DENALI_SDR_REG_19_USED == 1
	INIT_DENALI_SDR_REG(19)
#endif
#if SC_DENALI_SDR_REG_20_USED == 1
	INIT_DENALI_SDR_REG(20)
#endif
#if SC_DENALI_SDR_REG_21_USED == 1
	INIT_DENALI_SDR_REG(21)
#endif
#if SC_DENALI_SDR_REG_22_USED == 1
	INIT_DENALI_SDR_REG(22)
#endif
#if SC_DENALI_SDR_REG_23_USED == 1
	INIT_DENALI_SDR_REG(23)
#endif
#if SC_DENALI_SDR_REG_24_USED == 1
	INIT_DENALI_SDR_REG(24)
#endif
#if SC_DENALI_SDR_REG_25_USED == 1
	INIT_DENALI_SDR_REG(25)
#endif
#if SC_DENALI_SDR_REG_26_USED == 1
	INIT_DENALI_SDR_REG(26)
#endif
#if SC_DENALI_SDR_REG_27_USED == 1
	INIT_DENALI_SDR_REG(27)
#endif
#if SC_DENALI_SDR_REG_28_USED == 1
	INIT_DENALI_SDR_REG(28)
#endif
#if SC_DENALI_SDR_REG_29_USED == 1
	INIT_DENALI_SDR_REG(29)
#endif
#if SC_DENALI_SDR_REG_30_USED == 1
	INIT_DENALI_SDR_REG(30)
#endif
#if SC_DENALI_SDR_REG_31_USED == 1
	INIT_DENALI_SDR_REG(31)
#endif

#ifdef SYSCTRL_DDR_SUPPORT
	b	2f
	 nop
1:
#if SC_DENALI_DDR_REG_0_USED == 1
	INIT_DENALI_DDR_REG(0)
#endif
#if SC_DENALI_DDR_REG_1_USED == 1
	INIT_DENALI_DDR_REG(1)
#endif
#if SC_DENALI_DDR_REG_2_USED == 1
	INIT_DENALI_DDR_REG(2)
#endif
#if SC_DENALI_DDR_REG_3_USED == 1
	INIT_DENALI_DDR_REG(3)
#endif
#if SC_DENALI_DDR_REG_4_USED == 1
	INIT_DENALI_DDR_REG(4)
#endif
#if SC_DENALI_DDR_REG_5_USED == 1
	INIT_DENALI_DDR_REG(5)
#endif
#if SC_DENALI_DDR_REG_6_USED == 1
	INIT_DENALI_DDR_REG(6)
#endif
#if SC_DENALI_DDR_REG_7_USED == 1
	INIT_DENALI_DDR_REG(7)
#endif
#if SC_DENALI_DDR_REG_8_USED == 1
	INIT_DENALI_DDR_REG(8)
#endif
#if SC_DENALI_DDR_REG_9_USED == 1
	INIT_DENALI_DDR_REG(9)
#endif
#if SC_DENALI_DDR_REG_10_USED == 1
	INIT_DENALI_DDR_REG(10)
#endif
#if SC_DENALI_DDR_REG_11_USED == 1
	INIT_DENALI_DDR_REG(11)
#endif
#if SC_DENALI_DDR_REG_12_USED == 1
	INIT_DENALI_DDR_REG(12)
#endif
#if SC_DENALI_DDR_REG_13_USED == 1
	INIT_DENALI_DDR_REG(13)
#endif
#if SC_DENALI_DDR_REG_14_USED == 1
	INIT_DENALI_DDR_REG(14)
#endif
#if SC_DENALI_DDR_REG_15_USED == 1
	INIT_DENALI_DDR_REG(15)
#endif
#if SC_DENALI_DDR_REG_16_USED == 1
	INIT_DENALI_DDR_REG(16)
#endif
#if SC_DENALI_DDR_REG_17_USED == 1
	INIT_DENALI_DDR_REG(17)
#endif
#if SC_DENALI_DDR_REG_18_USED == 1
	INIT_DENALI_DDR_REG(18)
#endif
#if SC_DENALI_DDR_REG_19_USED == 1
	INIT_DENALI_DDR_REG(19)
#endif
#if SC_DENALI_DDR_REG_20_USED == 1
	INIT_DENALI_DDR_REG(20)
#endif
#if SC_DENALI_DDR_REG_21_USED == 1
	INIT_DENALI_DDR_REG(21)
#endif
#if SC_DENALI_DDR_REG_22_USED == 1
	INIT_DENALI_DDR_REG(22)
#endif
#if SC_DENALI_DDR_REG_23_USED == 1
	INIT_DENALI_DDR_REG(23)
#endif
#if SC_DENALI_DDR_REG_24_USED == 1
	INIT_DENALI_DDR_REG(24)
#endif
#if SC_DENALI_DDR_REG_25_USED == 1
	INIT_DENALI_DDR_REG(25)
#endif
#if SC_DENALI_DDR_REG_26_USED == 1
	INIT_DENALI_DDR_REG(26)
#endif
#if SC_DENALI_DDR_REG_27_USED == 1
	INIT_DENALI_DDR_REG(27)
#endif
#if SC_DENALI_DDR_REG_28_USED == 1
	INIT_DENALI_DDR_REG(28)
#endif
#if SC_DENALI_DDR_REG_29_USED == 1
	INIT_DENALI_DDR_REG(29)
#endif
#if SC_DENALI_DDR_REG_30_USED == 1
	INIT_DENALI_DDR_REG(30)
#endif
#if SC_DENALI_DDR_REG_31_USED == 1
	INIT_DENALI_DDR_REG(31)
#endif

2:
#endif
	/* Mask off external interrupt */
	li	t0, 0xf
	DMC_SET_REG_FIELD(INT_MASK_ADDR, INT_MASK_WIDTH, INT_MASK_OFFSET)
 
	/* Registered DIMM? */
	jal	read_eeprom
	 li	a0, SPD_MODATTR
	and	t0, v0, SPD_MODATTR_REG_MSK
	srl	t0, SPD_MODATTR_REG_SHF
	DMC_SET_REG_FIELD(REG_DIMM_ENABLE_ADDR,
			REG_DIMM_ENABLE_WIDTH, REG_DIMM_ENABLE_OFFSET)

	/********************************************
	** Use SPD values obtained earlier (ROWS, COLS and CSNUM) to fix up
	** some of the defaults above.
	*/

	/* Set addrpins = addrbits_max - addrbits */
	DMC_GET_REG_FIELD(MAX_ROW_REG_ADDR, MAX_ROW_REG_WIDTH, MAX_ROW_REG_OFFSET)
	sub	t0, ROWS
	DMC_SET_REG_FIELD(ADDR_PINS_ADDR, ADDR_PINS_WIDTH, ADDR_PINS_OFFSET)

	/* Set colsize = colbits_max - colbits */
	DMC_GET_REG_FIELD(MAX_COL_REG_ADDR, MAX_COL_REG_WIDTH, MAX_COL_REG_OFFSET)
	sub	t0, COLUMNS
	DMC_SET_REG_FIELD(COLUMN_SIZE_ADDR, COLUMN_SIZE_WIDTH, COLUMN_SIZE_OFFSET)

	/* Set csmap = (1 << csnum) - 1 */
	li	t0, 1
	sll	t0, CSNUM
	subu	t0, 1
	DMC_SET_REG_FIELD(CS_MAP_ADDR, CS_MAP_WIDTH, CS_MAP_OFFSET)

	/********************************************
	** Get SPD timing values to fix up some of the defaults above.
	** We should do this on the basis of the clock frequency, but we
	** don't have that yet so use some default min and max values.
	*/
#define BROKEN_LIKE_SOCIT	1	/* Make timings as wrong as SOC-it! */
#ifdef BROKEN_LIKE_SOCIT
#define MIN_PERIOD	8	/* Default to minimum cycle time of 6nS */
#else
#define MIN_PERIOD	6	/* Default to minimum cycle time of 6nS */
#endif
#define MAX_PERIOD	60	/* Default to maximum cycle time of 60nS */

#define PERIOD	a2
#define TRC_CLAMP a3

#ifdef SYSCTRL_DDR_SUPPORT
	/* For DDR, don't mess with any timings but refresh */
	jal	read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	beq	v0, t0, 9f
	 nop
#endif
	li	PERIOD, MIN_PERIOD

	/* CAS Latency */
	jal	read_eeprom
	 li	a0, SPD_CASLAT

	andi	a0, v0, SPD_CASLAT_2_BIT
	bne	a0, zero, 2f
	 li	v0, 2		/* If CAS latency 2 supported, use 2 */
	li	v0, 3		/* Otherwise use 3 */
2:
	move	t0, v0
	DMC_SET_REG_FIELD(CASLAT_ADDR, CASLAT_WIDTH, CASLAT_OFFSET)

	sll	v0, 1		/* CASLAT_LIN params defined in half cycles */
	move	t0, v0
	DMC_SET_REG_FIELD(CASLAT_LIN_ADDR, CASLAT_LIN_WIDTH, CASLAT_LIN_OFFSET)

	move	t0, v0
	DMC_SET_REG_FIELD(CASLAT_LIN_GATE_ADDR,
			CASLAT_LIN_GATE_WIDTH, CASLAT_LIN_GATE_OFFSET)

	/* RAS Precharge */
	jal	read_eeprom
	 li	a0, SPD_TRP

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRP_CLAMP 2
	sltiu	t1, t0, TRP_CLAMP	/* Clamp TRP >= 2 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRP_CLAMP
1:
	move	TRC_CLAMP, t0		/* TRC_CLAMP = TRP; updated later */
	DMC_SET_REG_FIELD(TRP_ADDR, TRP_WIDTH, TRP_OFFSET)

	/* RAS -> CAS delay */
	jal	read_eeprom
	 li	a0, SPD_TRCD

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRCD_CLAMP 2
	sltiu	t1, t0, TRCD_CLAMP	/* Clamp TRCD >= 2 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRCD_CLAMP
1:
	DMC_SET_REG_FIELD(TRCD_INT_ADDR, TRCD_INT_WIDTH, TRCD_INT_OFFSET)

	/* RAS -> RAS delay (tRRD) different banks */
	jal	read_eeprom
	 li	a0, SPD_TRRD

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0
	DMC_SET_REG_FIELD(TRRD_ADDR, TRRD_WIDTH, TRRD_OFFSET)

	/* RAS min */
	jal	read_eeprom
	 li	a0, SPD_TRAS

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

#define TRAS_MIN_CLAMP 6
	sltiu	t1, t0, TRAS_MIN_CLAMP	/* Clamp TRAS_MIN >= 6 */
	beq	t1, zero, 1f
	 nop
	li	t0, TRAS_MIN_CLAMP
1:
	add	TRC_CLAMP, t0		/* Now TRC_CLAMP = TRP + TRAS_MIN */
	DMC_SET_REG_FIELD(TRAS_MIN_ADDR, TRAS_MIN_WIDTH, TRAS_MIN_OFFSET)

	/* RAS -> RAS same bank */
	jal	read_eeprom
	 li	a0, SPD_TRC

#ifdef BROKEN_LIKE_SOCIT
	move	t0, v0
#else
	add	t0, v0, PERIOD		/* t0 = (v0 + PERIOD - 1) / PERIOD */
	sub	t0, 1
#endif
	divu	t0, PERIOD
	mflo	t0

	sltu	t1, t0, TRC_CLAMP	/* Clamp TRC >= TRP + TRAS_MIN */
	beq	t1, zero, 1f
	 nop
	move	t0, TRC_CLAMP
1:
	DMC_SET_REG_FIELD(TRC_ADDR, TRC_WIDTH, TRC_OFFSET)

	/* Max RAS time */
	/* No SPD entry for this, so use standard PC133 val of 100000nS */
	li	t0, ((TRAS_MAX_DEFAULT + MAX_PERIOD - 1) / MAX_PERIOD)
	DMC_SET_REG_FIELD(TRAS_MAX_ADDR, TRAS_MAX_WIDTH, TRAS_MAX_OFFSET)

	/* Write Recovery */
	/* No SPD entry for this, so use standard PC133 val of 15nS */
	li	t0, ((TWR_DEFAULT + MIN_PERIOD - 1) / MIN_PERIOD)
	DMC_SET_REG_FIELD(TWR_INT_ADDR, TWR_INT_WIDTH, TWR_INT_OFFSET)

#ifdef SYSCTRL_DDR_SUPPORT
9:
#endif
	/* Refresh */
	jal	read_eeprom
	 li	a0, SPD_RFSH_RT
	and	v0, SPD_RFSH_RT_RATE_MSK

	li	t0, SPD_RFSH_RT_RATE_125	/* 125 us */
	beq	t0, v0, 1f
	 li	t0, 125000>>1
	li	t0, SPD_RFSH_RT_RATE_62_5	/* 62.5 us */
	beq	t0, v0, 1f
	 li	t0, 62500>>1
	li	t0, SPD_RFSH_RT_RATE_31_3	/* 31.3 us */
	beq	t0, v0, 1f
	 li	t0, 31300>>1
	li	t0, SPD_RFSH_RT_RATE_15_625	/* 15.6 us */
	beq	t0, v0, 1f
	 li	t0, 15625>>1
	li	t0, SPD_RFSH_RT_RATE_7_8	/* 7.8 us */
	beq	t0, v0, 1f
	 li	t0, 7800>>1
	li	t0, 3900>>1			/* assume 3.9 us */
1:
	sll	t0, 1
	li	PERIOD, MAX_PERIOD

	divu	t0, PERIOD		/* t0 /= PERIOD */
	mflo	t0
	DMC_SET_REG_FIELD(TREF_ADDR, TREF_WIDTH, TREF_OFFSET)

	li	t0, 1
	DMC_SET_REG_FIELD(TREF_ENABLE_ADDR, TREF_ENABLE_WIDTH, TREF_ENABLE_OFFSET)
#undef TRC_CLAMP
#undef PERIOD

	/********************************************
	** Start the controller
	*/
	li	t0, 1
	DMC_SET_REG_FIELD(START_ADDR, START_WIDTH, START_OFFSET)

	/* for (cnt = 100000; (int_stat = intstat()) != 0 && --cnt != 0; ); */
#define CNT t4
	li	CNT, 100000
1:
	DMC_GET_REG_FIELD(INT_STATUS_ADDR, INT_STATUS_WIDTH, INT_STATUS_OFFSET)
	bne	t0, zero, 2f
	 subu	CNT, 1
	bne	CNT, zero, 1b
	 nop

	/* SDRAM: Timed out waiting for initialisation complete interrupt */
	b	config_error
	 nop
#undef CNT

2:
	/* Clear the interrupt */
	li	t0, 0xf
	DMC_SET_REG_FIELD (INT_ACK_ADDR, INT_ACK_WIDTH, INT_ACK_OFFSET)

	/* Re-enable external interrupt */
	move	t0, zero
	DMC_SET_REG_FIELD(INT_MASK_ADDR, INT_MASK_WIDTH, INT_MASK_OFFSET)

#ifdef SYSCTRL_DDR_SUPPORT
	/* We're running too slowly for a DDR SDRAM's DLL, so disable it */
	jal	read_eeprom
	 li	a0, SPD_FUNDAMENTAL_TYPE
	li	t0, SPD_FUNDAMENTAL_TYPE_DDR
	bne	v0, t0, 9f
	 nop
	li	t0, 1
	DMC_SET_REG_FIELD(EMRS1_DATA_ADDR, EMRS1_DATA_WIDTH, EMRS1_DATA_OFFSET)
	li	t0, 1
	DMC_SET_REG_FIELD(WRITE_MODEREG_ADDR,
			WRITE_MODEREG_WIDTH, WRITE_MODEREG_OFFSET)

	/********************************************
	** Try to determine the DDR CAS latency value
	*/
	/* Write 16 unique and easily-identifiable words */
	li	t0, KSEG1BASE	/* start addr */
	li	t1, 0xaa55aa00	/* start pattern */
	li	t2, 0xaa55aa15	/* final pattern */
1:	sw	t1, 0(t0)
	addu	t1, 1
	bleu	t1, t2, 1b
	 addu	t0, 4

	/* for (i = 2; i < 11;  i++) { set_caslat(i); check_pattern() } */
	li	a0, KSEG1BASE
	li	a2, 0xaa55aa00
	li	v0, 2
	li	t4, 11
1:	move	t0, v0
	DMC_SET_REG_FIELD(CASLAT_LIN_ADDR, CASLAT_LIN_WIDTH, CASLAT_LIN_OFFSET)
	move	t0, v0
	DMC_SET_REG_FIELD(CASLAT_LIN_GATE_ADDR,
			CASLAT_LIN_GATE_WIDTH, CASLAT_LIN_GATE_OFFSET)
	lw	t0, 0(a0)
	beq	t0, a2, 9f
	 nop
	addu	v0, 1
	bltu	v0, t4, 1b
	 nop

	/* CASLAT_LIN not found */
	DISP_STR( msg_dmc_cl ) /* destroys t5, t6, t7, t8, t9, ra */
99:	b	99b
	 nop
9:
#endif

#if 0
	/* This is only required to initialise parity which isnt supported */

	/* Clear memory */
	DISP_STR( msg_pram_clr ) /* destroys t5, t6, t7, t8, t9, ra */

	li	t0, KSEG1BASE	/* start addr */
	or	t1, t0, RAMSIZE /* limit */

	/* The processor is assumed to be MIPS32/64 compatible */
	MFC0(   v0, C0_Config )
	and	v0, M_ConfigAT
	sub	v0, K_ConfigAT_MIPS32<<S_ConfigAT
	bne	v0, zero, 2f
	 nop
	
	/* 32 bit CPU */
1:
	sw	zero, 0(t0)
	sw	zero, 4(t0)
	sw	zero, 8(t0)
	sw	zero, 12(t0)
	sw	zero, 16(t0)
	sw	zero, 20(t0)
	sw	zero, 24(t0)
	addu	t0, 32
	bnel	t0, t1, 1b
	 sw	zero, -4(t0)	# Store nullified when t0==t1

	b	8f
	 nop
	
	/* 64 bit CPU */
2:
SET_MIPS3()
	sd	zero, 0(t0)
	sd	zero, 8(t0)
	sd	zero, 16(t0)
	sd	zero, 24(t0)
	sd	zero, 32(t0)
	sd	zero, 40(t0)
	sd	zero, 48(t0)
	addu	t0, 64		
	bnel	t0, t1, 2b
	 sd	zero, -8(t0)	# Store nullified when t0==t1
SET_MIPS0()

8:
#endif

	/********************************************
	** Return
	*/
	move	v1, RAMSIZE
	jr	RA
	 li	v0, 0

config_error:
	li	v0, ERROR_SDRAM_CONFIG
	jr	RA
	 nop
END(dmc_rocit2_config)

MSG( msg_dmc_cl,   "CL: ERR" )
		
