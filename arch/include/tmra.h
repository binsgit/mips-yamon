#ifndef TMRA_H
#define TMRA_H
/************************************************************************
 *
 *  tmra.h
 *
 *  Register definition file for :
 *
 *    MIPS Timer Device, type A.
 *
 *  This timer device is first used on the ATLAS board, which
 *  has this device being part of the CBUS FPGA.
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




/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Timer: Properties
*************************************************************************/

#define TMRA_TICKS_PER_SECOND  1000000 /* timers: count per second      */



/************************************************************************
 *  Timer: Relative Register Addresses
*************************************************************************/


#define TMRA_TM0CNT_OFS         0x00 /* TIMER 0 count register          */
#define TMRA_TM0CMP_OFS         0x08 /* TIMER 0 compare register        */
#define TMRA_TM1CNT_OFS         0x10 /* TIMER 1 count register          */
#define TMRA_TM1CMP_OFS         0x18 /* TIMER 1 compare register        */
#define TMRA_TMINTACK_OFS       0x20 /* TIMER interrupt acknowledge reg */


/************************************************************************
 *  Timer: Register field encodings
*************************************************************************/


/******** reg: TMINTACK ********/

/* field: TM1ACK */
#define TMRA_TMINTACK_TM1ACK_SHF	1
#define TMRA_TMINTACK_TM1ACK_MSK	(MSK(1) << TMRA_TMINTACK_TM1ACK_SHF)
#define TMRA_TMINTACK_TM1ACK_SET	TMRA_TMINTACK_TM1ACK_MSK

/* field: TM0ACK */
#define TMRA_TMINTACK_TM0ACK_SHF	0
#define TMRA_TMINTACK_TM0ACK_MSK	(MSK(1) << TMRA_TMINTACK_TM0ACK_SHF)
#define TMRA_TMINTACK_TM0ACK_SET	TMRA_TMINTACK_TM0ACK_MSK


#endif /* #ifndef TMRA_H */
