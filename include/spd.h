/************************************************************************
 *
 *  spd.h
 *
 *  Register definitions for Serial Presence detect
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


#ifndef SPD_H
#define SPD_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Register offset addresses and access types
*************************************************************************/

#define SPD_FUNDAMENTAL_TYPE		2
#define SPD_ROWS			3
#define SPD_COL				4
#define SPD_MODULE_BANKS	       	5
#define SPD_MODULE_WIDTH_LO		6
#define SPD_MODULE_WIDTH_HI		7
#define SPD_CONFIG_TYPE			11
#define SPD_RFSH_RT			12
#define SPD_SDRAM_WIDTH			13
#define SPD_EC_SDRAM			14
#define SPD_BURSTLEN			16
#define SPD_DEVICE_BANKS		17
#define SPD_CASLAT			18
#define SPD_CSLAT			19
#define SPD_WRLAT			20
#define SPD_MRPT			27
#define SPD_TRP				27
#define SPD_RCDM			29
#define SPD_TRCD			29
#define SPD_MRPW			30
#define SPD_TRAS			30
#define SPD_ROW_DENSITY			31

/************************************************************************
 *  Register encodings
*************************************************************************/

#define SPD_FUNDAMENTAL_TYPE_SDR	4
#define SPD_FUNDAMENTAL_TYPE_DDR	7

#define SPD_ROWS_A_SHF			0
#define SPD_ROWS_A_MSK			(MSK(4) << SPD_ROWS_A_SHF)

#define SPD_ROWS_B_SHF			4
#define SPD_ROWS_B_MSK			(MSK(4) << SPD_ROWS_B_SHF)

#define SPD_COL_A_SHF			0
#define SPD_COL_A_MSK			(MSK(4) << SPD_COL_A_SHF)

#define SPD_COL_B_SHF			4
#define SPD_COL_B_MSK			(MSK(4) << SPD_COL_B_SHF)

#define SPD_CASLAT_1_SHF		0
#define SPD_CASLAT_1_MSK		(MSK(1) << SPD_CASLAT_1_SHF)
#define SPD_CASLAT_1_BIT		SPD_CASLAT_1_MSK

#define SPD_CASLAT_2_SHF		1
#define SPD_CASLAT_2_MSK		(MSK(1) << SPD_CASLAT_2_SHF)
#define SPD_CASLAT_2_BIT		SPD_CASLAT_2_MSK

#define SPD_CASLAT_3_SHF		2
#define SPD_CASLAT_3_MSK		(MSK(1) << SPD_CASLAT_3_SHF)
#define SPD_CASLAT_3_BIT		SPD_CASLAT_3_MSK

#define SPD_DDRCASLAT_1_0_SHF		0
#define SPD_DDRCASLAT_1_0_MSK		(MSK(1) << SPD_DDRCASLAT_1_0_SHF)
#define SPD_DDRCASLAT_1_0_BIT		SPD_DDRCASLAT_4_MSK

#define SPD_DDRCASLAT_1_5_SHF		1
#define SPD_DDRCASLAT_1_5_MSK		(MSK(1) << SPD_DDRCASLAT_1_5_SHF)
#define SPD_DDRCASLAT_1_5_BIT		SPD_DDRCASLAT_1_5_MSK

#define SPD_DDRCASLAT_2_0_SHF		2
#define SPD_DDRCASLAT_2_0_MSK		(MSK(1) << SPD_DDRCASLAT_2_0_SHF)
#define SPD_DDRCASLAT_2_0_BIT		SPD_DDRCASLAT_2_0_MSK

#define SPD_DDRCASLAT_2_5_SHF		3
#define SPD_DDRCASLAT_2_5_MSK		(MSK(1) << SPD_DDRCASLAT_2_5_SHF)
#define SPD_DDRCASLAT_2_5_BIT		SPD_DDRCASLAT_2_5_MSK

#define SPD_DDRCASLAT_3_0_SHF		4
#define SPD_DDRCASLAT_3_0_MSK		(MSK(1) << SPD_DDRCASLAT_3_0_SHF)
#define SPD_DDRCASLAT_3_0_BIT		SPD_DDRCASLAT_3_0_MSK

#define SPD_CSLAT_0_SHF			0
#define SPD_CSLAT_0_MSK			(MSK(1) << SPD_CASLAT_1_SHF)
#define SPD_CSLAT_0_BIT			SPD_CSLAT_0_MSK

#define SPD_CSLAT_1_SHF			1
#define SPD_CSLAT_1_MSK			(MSK(1) << SPD_CASLAT_2_SHF)
#define SPD_CSLAT_1_BIT			SPD_CSLAT_1_MSK

#define SPD_WRLAT_0_SHF			0
#define SPD_WRLAT_0_MSK			(MSK(1) << SPD_CASLAT_1_SHF)
#define SPD_WRLAT_0_BIT			SPD_WRLAT_0_MSK

#define SPD_WRLAT_1_SHF			1
#define SPD_WRLAT_1_MSK			(MSK(1) << SPD_CASLAT_2_SHF)
#define SPD_WRLAT_1_BIT			SPD_WRLAT_1_MSK

#define SPD_BURSTLEN_8_SHF		3
#define SPD_BURSTLEN_8_MSK		(MSK(1) << SPD_BURSTLEN_8_SHF)
#define SPD_BURSTLEN_8_BIT		SPD_BURSTLEN_8_MSK

#define SPD_BURSTLEN_4_SHF		2
#define SPD_BURSTLEN_4_MSK		(MSK(1) << SPD_BURSTLEN_4_SHF)
#define SPD_BURSTLEN_4_BIT		SPD_BURSTLEN_4_MSK

#define SPD_BURSTLEN_2_SHF		1
#define SPD_BURSTLEN_2_MSK		(MSK(1) << SPD_BURSTLEN_2_SHF)
#define SPD_BURSTLEN_2_BIT		SPD_BURSTLEN_2_MSK

#define SPD_BURSTLEN_1_SHF		0
#define SPD_BURSTLEN_1_MSK		(MSK(1) << SPD_BURSTLEN_1_SHF)
#define SPD_BURSTLEN_1_BIT		SPD_BURSTLEN_1_MSK

#define SPD_CONFIG_TYPE_NONE		0x0
#define SPD_CONFIG_TYPE_PARITY		0x1
#define SPD_CONFIG_TYPE_ECC		0x2

#define SPD_RFSH_RT_RATE_SHF		0
#define SPD_RFSH_RT_RATE_MSK		(MSK(7) << SPD_RFSH_RT_RATE_SHF)
#define SPD_RFSH_RT_RATE_125		5
#define SPD_RFSH_RT_RATE_62_5		4
#define SPD_RFSH_RT_RATE_31_3		3
#define SPD_RFSH_RT_RATE_15_625		0
#define SPD_RFSH_RT_RATE_7_8		2
#define SPD_RFSH_RT_RATE_3_9		1

#define SPD_SDRAM_WIDTH_W_SHF		0
#define SPD_SDRAM_WIDTH_W_MSK		(MSK(7) << SPD_SDRAM_WIDTH_W_SHF)
#define SPD_SDRAM_WIDTH_B2_SHF		7
#define SPD_SDRAM_WIDTH_B2_MSK		(MSK(1) << SPD_SDRAM_WIDTH_B2_SHF)
#define SPD_SDRAM_WIDTH_B2_BIT		SPD_SDRAM_WIDTH_B2_MSK

#define SPD_EC_SDRAM_WIDTH_SHF		0
#define SPD_EC_SDRAM_WIDTH_MSK		(MSK(7) << SPD_EC_SDRAM_WIDTH_SHF)


#endif /* #ifndef SPD_H */



