/************************************************************************
 *
 *  Register definitions for SAA9730 device
 *
 * ######################################################################
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
 ************************************************************************/


#ifndef SAA9730_H
#define SAA9730_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Register offset addresses and access types
*************************************************************************/

#define SAA9730_I2CTFR_OFS		0x2400
#define SAA9730_I2CSC_OFS		0x2404

/************************************************************************
 *  Register encodings
*************************************************************************/

#define SAA9730_I2CSC_I2CCC_SHF		8
#define SAA9730_I2CSC_I2CCC_MSK		(MSK(3) << SAA9730_I2CSC_I2CCC_SHF)
#define SAA9730_I2CSC_I2CCC_120		0
#define SAA9730_I2CSC_I2CCC_3200	1
#define SAA9730_I2CSC_I2CCC_80		2
#define SAA9730_I2CSC_I2CCC_480		4
#define SAA9730_I2CSC_I2CCC_6400	5
#define SAA9730_I2CSC_I2CCC_320		6

#define SAA9730_I2CTFR_BYTE2_SHF	24
#define SAA9730_I2CTFR_BYTE2_MSK	(MSK(8) << SAA9730_I2CTFR_BYTE2_SHF)

#define SAA9730_I2CTFR_BYTE2ATTR_SHF	6
#define SAA9730_I2CTFR_BYTE2ATTR_MSK	(MSK(2) << SAA9730_I2CTFR_BYTE2ATTR_SHF)

#define SAA9730_I2CTFR_BYTE1_SHF	16
#define SAA9730_I2CTFR_BYTE1_MSK	(MSK(8) << SAA9730_I2CTFR_BYTE1_SHF)

#define SAA9730_I2CTFR_BYTE1ATTR_SHF	4
#define SAA9730_I2CTFR_BYTE1ATTR_MSK	(MSK(2) << SAA9730_I2CTFR_BYTE1ATTR_SHF)

#define SAA9730_I2CTFR_BYTE0_SHF	8
#define SAA9730_I2CTFR_BYTE0_MSK	(MSK(8) << SAA9730_I2CTFR_BYTE0_SHF)

#define SAA9730_I2CTFR_BYTE0ATTR_SHF	2
#define SAA9730_I2CTFR_BYTE0ATTR_MSK	(MSK(2) << SAA9730_I2CTFR_BYTE0ATTR_SHF)

#define SAA9730_I2CTFR_BUSY_SHF		0
#define SAA9730_I2CTFR_BUSY_MSK		(MSK(1) << SAA9730_I2CTFR_BUSY_SHF)

#define SAA9730_I2CTFR_ERR_SHF		1
#define SAA9730_I2CTFR_ERR_MSK		(MSK(1) << SAA9730_I2CTFR_ERR_SHF)

/* Common values for BYTE[2..0]ATTR fields */
#define SAA9730_I2CTFR_ATTR_START	0x3
#define SAA9730_I2CTFR_ATTR_CONT	0x2
#define SAA9730_I2CTFR_ATTR_STOP	0x1
#define SAA9730_I2CTFR_ATTR_NOP		0x0

#endif /* #ifndef SAA9730_H */
