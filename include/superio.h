
/************************************************************************
 *
 *  Register definitions for SMSC SuperIO device
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2004 MIPS Technologies, Inc. All rights reserved.
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


#ifndef SUPERIO_H
#define SUPERIO_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Register offset addresses and access types
*************************************************************************/

#define SMSC_CONFIG_OFS		0x3f0
#define SMSC_DATA_OFS		0x3f1

#define SMSC_CONFIG_DEVNUM	0x7
#define SMSC_CONFIG_ACTIVATE	0x30
#define SMSC_CONFIG_BASEHI	0x60
#define SMSC_CONFIG_BASELO	0x61
#define SMSC_CONFIG_IRQ		0x70
#define SMSC_CONFIG_IRQ2	0x72
#define SMSC_CONFIG_MODE	0xF0

/************************************************************************
 *  Register encodings
*************************************************************************/

#define SMSC_CONFIG_KEY_START		  0x55
#define SMSC_CONFIG_KEY_EXIT		  0xaa

#define SMSC_CONFIG_DEVNUM_FDD	  	  0
#define SMSC_CONFIG_DEVNUM_COM1		  4
#define SMSC_CONFIG_DEVNUM_COM2		  5
#define SMSC_CONFIG_DEVNUM_PARALLEL	  3
#define SMSC_CONFIG_DEVNUM_KYBD		  7

#define SMSC_CONFIG_ACTIVATE_ENABLE_SHF	  0
#define SMSC_CONFIG_ACTIVATE_ENABLE_MSK   (MSK(1) << SMSC_CONFIG_ACTIVATE_ENABLE_SHF)
#define SMSC_CONFIG_ACTIVATE_ENABLE_BIT   SMSC_CONFIG_ACTIVATE_ENABLE_MSK

#define SMSC_CONFIG_MODE_HIGHSPEED_SHF	  1
#define SMSC_CONFIG_MODE_HIGHSPEED_MSK	  (MSK(1) << SMSC_CONFIG_MODE_HIGHSPEED_SHF)
#define SMSC_CONFIG_MODE_HIGHSPEED_BIT	  SMSC_CONFIG_MODE_HIGHSPEED_MSK

#endif /* #ifndef SUPERIO_H */

