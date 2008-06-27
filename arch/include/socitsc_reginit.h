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

#define TRAS_MAX_DEFAULT 100000
#define TWR_DEFAULT 15
#include <socitsc_reginit_ddr.h>
#include <socitsc_reginit_sdr.h>

#define DENALI_REG_0_RO 0x0
#define DENALI_REG_1_RO 0x1 // DLL lock
#define DENALI_REG_2_RO 0x0
#define DENALI_REG_3_RO 0x0
#define DENALI_REG_4_RO 0x0
#define DENALI_REG_5_RO 0x200 // Max CS - read only
#define DENALI_REG_6_RO 0x0
#define DENALI_REG_7_RO 0x0
#define DENALI_REG_8_RO 0x0
#define DENALI_REG_9_RO 0x0e0c // Max rowaddr & coladdr pins
#define DENALI_REG_10_RO 0x0
#define DENALI_REG_11_RO 0x0
#define DENALI_REG_12_RO 0x0
#define DENALI_REG_13_RO 0x0
#define DENALI_REG_14_RO 0x0
#define DENALI_REG_15_RO 0x0
#define DENALI_REG_16_RO 0x0
#define DENALI_REG_17_RO 0x0
#define DENALI_REG_18_RO 0x0
#define DENALI_REG_19_RO 0x0
#define DENALI_REG_20_RO 0x0
#define DENALI_REG_21_RO 0x20400000 // Version
#define DENALI_REG_22_RO 0x0
#define DENALI_REG_23_RO 0x0
#define DENALI_REG_24_RO 0x0
#define DENALI_REG_25_RO 0x0
#define DENALI_REG_26_RO 0x0
#define DENALI_REG_27_RO 0x0
#define DENALI_REG_28_RO 0x0
#define DENALI_REG_29_RO 0x0
#define DENALI_REG_30_RO 0x0
#define DENALI_REG_31_RO 0x0

#define SC_GET_REG(addr)  (REG32(KSEG1(SOCITSC_DRAM_REG_BASE + 4 * addr)))
#define SC_SET_REG(addr,val)  ((REG32(KSEG1(SOCITSC_DRAM_REG_BASE + 4 * addr))) = val)

#define SC_SET_REG_FIELD(reg,val,width,offset) (reg & ~(((1 << width) - 1) << offset) | (val & ((1 << width) - 1)) << offset)
#define SC_GET_REG_FIELD(reg,width,offset) ((reg >> offset) & ((1 << width) - 1))

