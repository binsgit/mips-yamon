
/************************************************************************
 *
 *  icta.h
 *
 *  Register definition file for :
 *
 *    MIPS Interrupt Controller Device, type A.
 *
 *  This interrupt controller is first used on the ATLAS board, which
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

#ifndef ICTA_H
#define ICTA_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Interrupt Controller : Relative Register Addresses
*************************************************************************/


#define ICTA_INTRAW_OFS         0x00 /* RAW value on external int lines */
#define ICTA_INTSETEN_OFS       0x08 /* SET enable int ("1") per bit    */
#define ICTA_INTRSTEN_OFS       0x10 /* RESET enable int ("1") per bit  */
#define ICTA_INTENABLE_OFS      0x18 /* INT enable mask status          */
#define ICTA_INTSTATUS_OFS      0x20 /* value = INTRAW & INTENABLE      */


/************************************************************************
 *  Interrupt Controller : Register field encodings
*************************************************************************/


/******** reg: INTRAW ********/


/* bit 19: PCISERRN */
#define ICTA_INTRAW_PCISERRN_SHF	19
#define ICTA_INTRAW_PCISERRN_MSK	(MSK(1) << ICTA_INTRAW_PCISERRN_SHF)
#define ICTA_INTRAW_PCISERRN_SET	ICTA_INTRAW_PCISERRN_MSK

/* bit 18: CONINTDN */
#define ICTA_INTRAW_CONINTDN_SHF	18
#define ICTA_INTRAW_CONINTDN_MSK	(MSK(1) << ICTA_INTRAW_CONINTDN_SHF)
#define ICTA_INTRAW_CONINTDN_SET	ICTA_INTRAW_CONINTDN_MSK

/* bit 17: CONINTCN */
#define ICTA_INTRAW_CONINTCN_SHF	17
#define ICTA_INTRAW_CONINTCN_MSK	(MSK(1) << ICTA_INTRAW_CONINTCN_SHF)
#define ICTA_INTRAW_CONINTCN_SET	ICTA_INTRAW_CONINTCN_MSK

/* bit 16: CONINTBN */
#define ICTA_INTRAW_CONINTBN_SHF	16
#define ICTA_INTRAW_CONINTBN_MSK	(MSK(1) << ICTA_INTRAW_CONINTBN_SHF)
#define ICTA_INTRAW_CONINTBN_SET	ICTA_INTRAW_CONINTBN_MSK

/* bit 15: CONINTAN */
#define ICTA_INTRAW_CONINTAN_SHF	15
#define ICTA_INTRAW_CONINTAN_MSK	(MSK(1) << ICTA_INTRAW_CONINTAN_SHF)
#define ICTA_INTRAW_CONINTAN_SET	ICTA_INTRAW_CONINTAN_MSK

/* bit 14: ATXOKN */
#define ICTA_INTRAW_ATXOKN_SHF	        14
#define ICTA_INTRAW_ATXOKN_MSK	        (MSK(1) << ICTA_INTRAW_ATXOKN_SHF)
#define ICTA_INTRAW_ATXOKN_SET	        ICTA_INTRAW_ATXOKN_MSK

/* bit 13: DEG */
#define ICTA_INTRAW_DEG_SHF	        13
#define ICTA_INTRAW_DEG_MSK	        (MSK(1) << ICTA_INTRAW_DEG_SHF)
#define ICTA_INTRAW_DEG_SET	        ICTA_INTRAW_DEG_MSK

/* bit 12: ENUM */
#define ICTA_INTRAW_ENUM_SHF	        12
#define ICTA_INTRAW_ENUM_MSK	        (MSK(1) << ICTA_INTRAW_ENUM_SHF)
#define ICTA_INTRAW_ENUM_SET	        ICTA_INTRAW_ENUM_MSK

/* bit 11: PCID */
#define ICTA_INTRAW_PCID_SHF	        11
#define ICTA_INTRAW_PCID_MSK	        (MSK(1) << ICTA_INTRAW_PCID_SHF)
#define ICTA_INTRAW_PCID_SET	        ICTA_INTRAW_PCID_MSK

/* bit 10: PCIC */
#define ICTA_INTRAW_PCIC_SHF	        10
#define ICTA_INTRAW_PCIC_MSK	        (MSK(1) << ICTA_INTRAW_PCIC_SHF)
#define ICTA_INTRAW_PCIC_SET	        ICTA_INTRAW_PCIC_MSK

/* bit 9: PCIB */
#define ICTA_INTRAW_PCIB_SHF	        9
#define ICTA_INTRAW_PCIB_MSK	        (MSK(1) << ICTA_INTRAW_PCIB_SHF)
#define ICTA_INTRAW_PCIB_SET	        ICTA_INTRAW_PCIB_MSK

/* bit 8: PCIA */
#define ICTA_INTRAW_PCIA_SHF	        8
#define ICTA_INTRAW_PCIA_MSK	        (MSK(1) << ICTA_INTRAW_PCIA_SHF)
#define ICTA_INTRAW_PCIA_SET	        ICTA_INTRAW_PCIA_MSK

/* bit 7: NMI */
#define ICTA_INTRAW_NMI_SHF	        7
#define ICTA_INTRAW_NMI_MSK	        (MSK(1) << ICTA_INTRAW_NMI_SHF)
#define ICTA_INTRAW_NMI_SET	        ICTA_INTRAW_NMI_MSK

/* bit 6: CORELO */
#define ICTA_INTRAW_CORELO_SHF	        6
#define ICTA_INTRAW_CORELO_MSK	        (MSK(1) << ICTA_INTRAW_CORELO_SHF)
#define ICTA_INTRAW_CORELO_SET	        ICTA_INTRAW_CORELO_MSK

/* bit 5: COREHI */
#define ICTA_INTRAW_COREHI_SHF	        5
#define ICTA_INTRAW_COREHI_MSK	        (MSK(1) << ICTA_INTRAW_COREHI_SHF)
#define ICTA_INTRAW_COREHI_SET	        ICTA_INTRAW_COREHI_MSK

/* bit 4: RTC */
#define ICTA_INTRAW_RTC_SHF	        4
#define ICTA_INTRAW_RTC_MSK	        (MSK(1) << ICTA_INTRAW_RTC_SHF)
#define ICTA_INTRAW_RTC_SET	        ICTA_INTRAW_RTC_MSK

/* bit 2: TIM1 */
#define ICTA_INTRAW_TIM1_SHF	        2
#define ICTA_INTRAW_TIM1_MSK	        (MSK(1) << ICTA_INTRAW_TIM1_SHF)
#define ICTA_INTRAW_TIM1_SET	        ICTA_INTRAW_TIM1_MSK

/* bit 1: TIM0 */
#define ICTA_INTRAW_TIM0_SHF	        1
#define ICTA_INTRAW_TIM0_MSK	        (MSK(1) << ICTA_INTRAW_TIM0_SHF)
#define ICTA_INTRAW_TIM0_SET	        ICTA_INTRAW_TIM0_MSK

/* bit 0: SER */
#define ICTA_INTRAW_SER_SHF	        0
#define ICTA_INTRAW_SER_MSK	        (MSK(1) << ICTA_INTRAW_SER_SHF)
#define ICTA_INTRAW_SER_SET	        ICTA_INTRAW_SER_MSK



/******** reg: INTSETEN ********/


/* bit 19: PCISERRN */
#define ICTA_INTSETEN_PCISERRN_SHF	19
#define ICTA_INTSETEN_PCISERRN_MSK	(MSK(1) << ICTA_INTSETEN_PCISERRN_SHF)
#define ICTA_INTSETEN_PCISERRN_SET	ICTA_INTSETEN_PCISERRN_MSK

/* bit 18: CONINTDN */
#define ICTA_INTSETEN_CONINTDN_SHF	18
#define ICTA_INTSETEN_CONINTDN_MSK	(MSK(1) << ICTA_INTSETEN_CONINTDN_SHF)
#define ICTA_INTSETEN_CONINTDN_SET	ICTA_INTSETEN_CONINTDN_MSK

/* bit 17: CONINTCN */
#define ICTA_INTSETEN_CONINTCN_SHF	17
#define ICTA_INTSETEN_CONINTCN_MSK	(MSK(1) << ICTA_INTSETEN_CONINTCN_SHF)
#define ICTA_INTSETEN_CONINTCN_SET	ICTA_INTSETEN_CONINTCN_MSK

/* bit 16: CONINTBN */
#define ICTA_INTSETEN_CONINTBN_SHF	16
#define ICTA_INTSETEN_CONINTBN_MSK	(MSK(1) << ICTA_INTSETEN_CONINTBN_SHF)
#define ICTA_INTSETEN_CONINTBN_SET	ICTA_INTSETEN_CONINTBN_MSK

/* bit 15: CONINTAN */
#define ICTA_INTSETEN_CONINTAN_SHF	15
#define ICTA_INTSETEN_CONINTAN_MSK	(MSK(1) << ICTA_INTSETEN_CONINTAN_SHF)
#define ICTA_INTSETEN_CONINTAN_SET	ICTA_INTSETEN_CONINTAN_MSK

/* bit 14: ATXOKN */
#define ICTA_INTSETEN_ATXOKN_SHF	14
#define ICTA_INTSETEN_ATXOKN_MSK	(MSK(1) << ICTA_INTSETEN_ATXOKN_SHF)
#define ICTA_INTSETEN_ATXOKN_SET	ICTA_INTSETEN_ATXOKN_MSK

/* bit 13: DEG */
#define ICTA_INTSETEN_DEG_SHF	        13
#define ICTA_INTSETEN_DEG_MSK	        (MSK(1) << ICTA_INTSETEN_DEG_SHF)
#define ICTA_INTSETEN_DEG_SET	        ICTA_INTSETEN_DEG_MSK

/* bit 12: ENUM */
#define ICTA_INTSETEN_ENUM_SHF	        12
#define ICTA_INTSETEN_ENUM_MSK	        (MSK(1) << ICTA_INTSETEN_ENUM_SHF)
#define ICTA_INTSETEN_ENUM_SET	        ICTA_INTSETEN_ENUM_MSK

/* bit 11: PCID */
#define ICTA_INTSETEN_PCID_SHF	        11
#define ICTA_INTSETEN_PCID_MSK	        (MSK(1) << ICTA_INTSETEN_PCID_SHF)
#define ICTA_INTSETEN_PCID_SET	        ICTA_INTSETEN_PCID_MSK

/* bit 10: PCIC */
#define ICTA_INTSETEN_PCIC_SHF	        10
#define ICTA_INTSETEN_PCIC_MSK	        (MSK(1) << ICTA_INTSETEN_PCIC_SHF)
#define ICTA_INTSETEN_PCIC_SET	        ICTA_INTSETEN_PCIC_MSK

/* bit 9: PCIB */
#define ICTA_INTSETEN_PCIB_SHF	        9
#define ICTA_INTSETEN_PCIB_MSK	        (MSK(1) << ICTA_INTSETEN_PCIB_SHF)
#define ICTA_INTSETEN_PCIB_SET	        ICTA_INTSETEN_PCIB_MSK

/* bit 8: PCIA */
#define ICTA_INTSETEN_PCIA_SHF	        8
#define ICTA_INTSETEN_PCIA_MSK	        (MSK(1) << ICTA_INTSETEN_PCIA_SHF)
#define ICTA_INTSETEN_PCIA_SET	        ICTA_INTSETEN_PCIA_MSK

/* bit 7: NMI */
#define ICTA_INTSETEN_NMI_SHF	        7
#define ICTA_INTSETEN_NMI_MSK	        (MSK(1) << ICTA_INTSETEN_NMI_SHF)
#define ICTA_INTSETEN_NMI_SET	        ICTA_INTSETEN_NMI_MSK

/* bit 6: CORELO */
#define ICTA_INTSETEN_CORELO_SHF	6
#define ICTA_INTSETEN_CORELO_MSK	(MSK(1) << ICTA_INTSETEN_CORELO_SHF)
#define ICTA_INTSETEN_CORELO_SET	ICTA_INTSETEN_CORELO_MSK

/* bit 5: COREHI */
#define ICTA_INTSETEN_COREHI_SHF	5
#define ICTA_INTSETEN_COREHI_MSK	(MSK(1) << ICTA_INTSETEN_COREHI_SHF)
#define ICTA_INTSETEN_COREHI_SET	ICTA_INTSETEN_COREHI_MSK

/* bit 4: RTC */
#define ICTA_INTSETEN_RTC_SHF	        4
#define ICTA_INTSETEN_RTC_MSK	        (MSK(1) << ICTA_INTSETEN_RTC_SHF)
#define ICTA_INTSETEN_RTC_SET	        ICTA_INTSETEN_RTC_MSK

/* bit 2: TIM1 */
#define ICTA_INTSETEN_TIM1_SHF	        2
#define ICTA_INTSETEN_TIM1_MSK	        (MSK(1) << ICTA_INTSETEN_TIM1_SHF)
#define ICTA_INTSETEN_TIM1_SET	        ICTA_INTSETEN_TIM1_MSK

/* bit 1: TIM0 */
#define ICTA_INTSETEN_TIM0_SHF	        1
#define ICTA_INTSETEN_TIM0_MSK	        (MSK(1) << ICTA_INTSETEN_TIM0_SHF)
#define ICTA_INTSETEN_TIM0_SET	        ICTA_INTSETEN_TIM0_MSK

/* bit 0: SER */
#define ICTA_INTSETEN_SER_SHF	        0
#define ICTA_INTSETEN_SER_MSK	        (MSK(1) << ICTA_INTSETEN_SER_SHF)
#define ICTA_INTSETEN_SER_SET	        ICTA_INTSETEN_SER_MSK



/******** reg: INTRSTEN ********/


/* bit 19: PCISERRN */
#define ICTA_INTRSTEN_PCISERRN_SHF	19
#define ICTA_INTRSTEN_PCISERRN_MSK	(MSK(1) << ICTA_INTRSTEN_PCISERRN_SHF)
#define ICTA_INTRSTEN_PCISERRN_SET	ICTA_INTRSTEN_PCISERRN_MSK

/* bit 18: CONINTDN */
#define ICTA_INTRSTEN_CONINTDN_SHF	18
#define ICTA_INTRSTEN_CONINTDN_MSK	(MSK(1) << ICTA_INTRSTEN_CONINTDN_SHF)
#define ICTA_INTRSTEN_CONINTDN_SET	ICTA_INTRSTEN_CONINTDN_MSK

/* bit 17: CONINTCN */
#define ICTA_INTRSTEN_CONINTCN_SHF	17
#define ICTA_INTRSTEN_CONINTCN_MSK	(MSK(1) << ICTA_INTRSTEN_CONINTCN_SHF)
#define ICTA_INTRSTEN_CONINTCN_SET	ICTA_INTRSTEN_CONINTCN_MSK

/* bit 16: CONINTBN */
#define ICTA_INTRSTEN_CONINTBN_SHF	16
#define ICTA_INTRSTEN_CONINTBN_MSK	(MSK(1) << ICTA_INTRSTEN_CONINTBN_SHF)
#define ICTA_INTRSTEN_CONINTBN_SET	ICTA_INTRSTEN_CONINTBN_MSK

/* bit 15: CONINTAN */
#define ICTA_INTRSTEN_CONINTAN_SHF	15
#define ICTA_INTRSTEN_CONINTAN_MSK	(MSK(1) << ICTA_INTRSTEN_CONINTAN_SHF)
#define ICTA_INTRSTEN_CONINTAN_SET	ICTA_INTRSTEN_CONINTAN_MSK

/* bit 14: ATXOKN */
#define ICTA_INTRSTEN_ATXOKN_SHF	14
#define ICTA_INTRSTEN_ATXOKN_MSK	(MSK(1) << ICTA_INTRSTEN_ATXOKN_SHF)
#define ICTA_INTRSTEN_ATXOKN_SET	ICTA_INTRSTEN_ATXOKN_MSK

/* bit 13: DEG */
#define ICTA_INTRSTEN_DEG_SHF	        13
#define ICTA_INTRSTEN_DEG_MSK	        (MSK(1) << ICTA_INTRSTEN_DEG_SHF)
#define ICTA_INTRSTEN_DEG_SET	        ICTA_INTRSTEN_DEG_MSK

/* bit 12: ENUM */
#define ICTA_INTRSTEN_ENUM_SHF	        12
#define ICTA_INTRSTEN_ENUM_MSK	        (MSK(1) << ICTA_INTRSTEN_ENUM_SHF)
#define ICTA_INTRSTEN_ENUM_SET	        ICTA_INTRSTEN_ENUM_MSK

/* bit 11: PCID */
#define ICTA_INTRSTEN_PCID_SHF	        11
#define ICTA_INTRSTEN_PCID_MSK	        (MSK(1) << ICTA_INTRSTEN_PCID_SHF)
#define ICTA_INTRSTEN_PCID_SET	        ICTA_INTRSTEN_PCID_MSK

/* bit 10: PCIC */
#define ICTA_INTRSTEN_PCIC_SHF	        10
#define ICTA_INTRSTEN_PCIC_MSK	        (MSK(1) << ICTA_INTRSTEN_PCIC_SHF)
#define ICTA_INTRSTEN_PCIC_SET	        ICTA_INTRSTEN_PCIC_MSK

/* bit 9: PCIB */
#define ICTA_INTRSTEN_PCIB_SHF	        9
#define ICTA_INTRSTEN_PCIB_MSK	        (MSK(1) << ICTA_INTRSTEN_PCIB_SHF)
#define ICTA_INTRSTEN_PCIB_SET	        ICTA_INTRSTEN_PCIB_MSK

/* bit 8: PCIA */
#define ICTA_INTRSTEN_PCIA_SHF	        8
#define ICTA_INTRSTEN_PCIA_MSK	        (MSK(1) << ICTA_INTRSTEN_PCIA_SHF)
#define ICTA_INTRSTEN_PCIA_SET	        ICTA_INTRSTEN_PCIA_MSK

/* bit 7: NMI */
#define ICTA_INTRSTEN_NMI_SHF	        7
#define ICTA_INTRSTEN_NMI_MSK	        (MSK(1) << ICTA_INTRSTEN_NMI_SHF)
#define ICTA_INTRSTEN_NMI_SET	        ICTA_INTRSTEN_NMI_MSK

/* bit 6: CORELO */
#define ICTA_INTRSTEN_CORELO_SHF	6
#define ICTA_INTRSTEN_CORELO_MSK	(MSK(1) << ICTA_INTRSTEN_CORELO_SHF)
#define ICTA_INTRSTEN_CORELO_SET	ICTA_INTRSTEN_CORELO_MSK

/* bit 5: COREHI */
#define ICTA_INTRSTEN_COREHI_SHF	5
#define ICTA_INTRSTEN_COREHI_MSK	(MSK(1) << ICTA_INTRSTEN_COREHI_SHF)
#define ICTA_INTRSTEN_COREHI_SET	ICTA_INTRSTEN_COREHI_MSK

/* bit 4: RTC */
#define ICTA_INTRSTEN_RTC_SHF	        4
#define ICTA_INTRSTEN_RTC_MSK	        (MSK(1) << ICTA_INTRSTEN_RTC_SHF)
#define ICTA_INTRSTEN_RTC_SET	        ICTA_INTRSTEN_RTC_MSK

/* bit 2: TIM1 */
#define ICTA_INTRSTEN_TIM1_SHF	        2
#define ICTA_INTRSTEN_TIM1_MSK	        (MSK(1) << ICTA_INTRSTEN_TIM1_SHF)
#define ICTA_INTRSTEN_TIM1_SET	        ICTA_INTRSTEN_TIM1_MSK

/* bit 1: TIM0 */
#define ICTA_INTRSTEN_TIM0_SHF	        1
#define ICTA_INTRSTEN_TIM0_MSK	        (MSK(1) << ICTA_INTRSTEN_TIM0_SHF)
#define ICTA_INTRSTEN_TIM0_SET	        ICTA_INTRSTEN_TIM0_MSK

/* bit 0: SER */
#define ICTA_INTRSTEN_SER_SHF	        0
#define ICTA_INTRSTEN_SER_MSK	        (MSK(1) << ICTA_INTRSTEN_SER_SHF)
#define ICTA_INTRSTEN_SER_SET	        ICTA_INTRSTEN_SER_MSK



/******** reg: INTENABLE ********/


/* bit 19: PCISERRN */
#define ICTA_INTENABLE_PCISERRN_SHF	19
#define ICTA_INTENABLE_PCISERRN_MSK	(MSK(1) << ICTA_INTENABLE_PCISERRN_SHF)
#define ICTA_INTENABLE_PCISERRN_SET	ICTA_INTENABLE_PCISERRN_MSK

/* bit 18: CONINTDN */
#define ICTA_INTENABLE_CONINTDN_SHF	18
#define ICTA_INTENABLE_CONINTDN_MSK	(MSK(1) << ICTA_INTENABLE_CONINTDN_SHF)
#define ICTA_INTENABLE_CONINTDN_SET	ICTA_INTENABLE_CONINTDN_MSK

/* bit 17: CONINTCN */
#define ICTA_INTENABLE_CONINTCN_SHF	17
#define ICTA_INTENABLE_CONINTCN_MSK	(MSK(1) << ICTA_INTENABLE_CONINTCN_SHF)
#define ICTA_INTENABLE_CONINTCN_SET	ICTA_INTENABLE_CONINTCN_MSK

/* bit 16: CONINTBN */
#define ICTA_INTENABLE_CONINTBN_SHF	16
#define ICTA_INTENABLE_CONINTBN_MSK	(MSK(1) << ICTA_INTENABLE_CONINTBN_SHF)
#define ICTA_INTENABLE_CONINTBN_SET	ICTA_INTENABLE_CONINTBN_MSK

/* bit 15: CONINTAN */
#define ICTA_INTENABLE_CONINTAN_SHF	15
#define ICTA_INTENABLE_CONINTAN_MSK	(MSK(1) << ICTA_INTENABLE_CONINTAN_SHF)
#define ICTA_INTENABLE_CONINTAN_SET	ICTA_INTENABLE_CONINTAN_MSK

/* bit 14: ATXOKN */
#define ICTA_INTENABLE_ATXOKN_SHF	14
#define ICTA_INTENABLE_ATXOKN_MSK	(MSK(1) << ICTA_INTENABLE_ATXOKN_SHF)
#define ICTA_INTENABLE_ATXOKN_SET       ICTA_INTENABLE_ATXOKN_MSK

/* bit 13: DEG */
#define ICTA_INTENABLE_DEG_SHF	        13
#define ICTA_INTENABLE_DEG_MSK	        (MSK(1) << ICTA_INTENABLE_DEG_SHF)
#define ICTA_INTENABLE_DEG_SET	        ICTA_INTENABLE_DEG_MSK

/* bit 12: ENUM */
#define ICTA_INTENABLE_ENUM_SHF	        12
#define ICTA_INTENABLE_ENUM_MSK	        (MSK(1) << ICTA_INTENABLE_ENUM_SHF)
#define ICTA_INTENABLE_ENUM_SET	        ICTA_INTENABLE_ENUM_MSK

/* bit 11: PCID */
#define ICTA_INTENABLE_PCID_SHF	        11
#define ICTA_INTENABLE_PCID_MSK	        (MSK(1) << ICTA_INTENABLE_PCID_SHF)
#define ICTA_INTENABLE_PCID_SET	        ICTA_INTENABLE_PCID_MSK

/* bit 10: PCIC */
#define ICTA_INTENABLE_PCIC_SHF	        10
#define ICTA_INTENABLE_PCIC_MSK	        (MSK(1) << ICTA_INTENABLE_PCIC_SHF)
#define ICTA_INTENABLE_PCIC_SET	        ICTA_INTENABLE_PCIC_MSK

/* bit 9: PCIB */
#define ICTA_INTENABLE_PCIB_SHF	        9
#define ICTA_INTENABLE_PCIB_MSK	        (MSK(1) << ICTA_INTENABLE_PCIB_SHF)
#define ICTA_INTENABLE_PCIB_SET	        ICTA_INTENABLE_PCIB_MSK

/* bit 8: PCIA */
#define ICTA_INTENABLE_PCIA_SHF	        8
#define ICTA_INTENABLE_PCIA_MSK	        (MSK(1) << ICTA_INTENABLE_PCIA_SHF)
#define ICTA_INTENABLE_PCIA_SET	        ICTA_INTENABLE_PCIA_MSK

/* bit 7: NMI */
#define ICTA_INTENABLE_NMI_SHF	        7
#define ICTA_INTENABLE_NMI_MSK	        (MSK(1) << ICTA_INTENABLE_NMI_SHF)
#define ICTA_INTENABLE_NMI_SET	        ICTA_INTENABLE_NMI_MSK

/* bit 6: CORELO */
#define ICTA_INTENABLE_CORELO_SHF	6
#define ICTA_INTENABLE_CORELO_MSK	(MSK(1) << ICTA_INTENABLE_CORELO_SHF)
#define ICTA_INTENABLE_CORELO_SET	ICTA_INTENABLE_CORELO_MSK

/* bit 5: COREHI */
#define ICTA_INTENABLE_COREHI_SHF	5
#define ICTA_INTENABLE_COREHI_MSK	(MSK(1) << ICTA_INTENABLE_COREHI_SHF)
#define ICTA_INTENABLE_COREHI_SET	ICTA_INTENABLE_COREHI_MSK

/* bit 4: RTC */
#define ICTA_INTENABLE_RTC_SHF	        4
#define ICTA_INTENABLE_RTC_MSK	        (MSK(1) << ICTA_INTENABLE_RTC_SHF)
#define ICTA_INTENABLE_RTC_SET	        ICTA_INTENABLE_RTC_MSK

/* bit 2: TIM1 */
#define ICTA_INTENABLE_TIM1_SHF	        2
#define ICTA_INTENABLE_TIM1_MSK	        (MSK(1) << ICTA_INTENABLE_TIM1_SHF)
#define ICTA_INTENABLE_TIM1_SET	        ICTA_INTENABLE_TIM1_MSK

/* bit 1: TIM0 */
#define ICTA_INTENABLE_TIM0_SHF	        1
#define ICTA_INTENABLE_TIM0_MSK	        (MSK(1) << ICTA_INTENABLE_TIM0_SHF)
#define ICTA_INTENABLE_TIM0_SET	        ICTA_INTENABLE_TIM0_MSK

/* bit 0: SER */
#define ICTA_INTENABLE_SER_SHF	        0
#define ICTA_INTENABLE_SER_MSK	        (MSK(1) << ICTA_INTENABLE_SER_SHF)
#define ICTA_INTENABLE_SER_SET	        ICTA_INTENABLE_SER_MSK



/******** reg: INTSTATUS ********/


/* bit 19: PCISERRN */
#define ICTA_INTSTATUS_PCISERRN_SHF	19
#define ICTA_INTSTATUS_PCISERRN_MSK	(MSK(1) << ICTA_INTSTATUS_PCISERRN_SHF)
#define ICTA_INTSTATUS_PCISERRN_SET	ICTA_INTSTATUS_PCISERRN_MSK

/* bit 18: CONINTDN */
#define ICTA_INTSTATUS_CONINTDN_SHF	18
#define ICTA_INTSTATUS_CONINTDN_MSK	(MSK(1) << ICTA_INTSTATUS_CONINTDN_SHF)
#define ICTA_INTSTATUS_CONINTDN_SET	ICTA_INTSTATUS_CONINTDN_MSK

/* bit 17: CONINTCN */
#define ICTA_INTSTATUS_CONINTCN_SHF	17
#define ICTA_INTSTATUS_CONINTCN_MSK	(MSK(1) << ICTA_INTSTATUS_CONINTCN_SHF)
#define ICTA_INTSTATUS_CONINTCN_SET	ICTA_INTSTATUS_CONINTCN_MSK

/* bit 16: CONINTBN */
#define ICTA_INTSTATUS_CONINTBN_SHF	16
#define ICTA_INTSTATUS_CONINTBN_MSK	(MSK(1) << ICTA_INTSTATUS_CONINTBN_SHF)
#define ICTA_INTSTATUS_CONINTBN_SET	ICTA_INTSTATUS_CONINTBN_MSK

/* bit 15: CONINTAN */
#define ICTA_INTSTATUS_CONINTAN_SHF	15
#define ICTA_INTSTATUS_CONINTAN_MSK	(MSK(1) << ICTA_INTSTATUS_CONINTAN_SHF)
#define ICTA_INTSTATUS_CONINTAN_SET	ICTA_INTSTATUS_CONINTAN_MSK

/* bit 14: ATXOKN */
#define ICTA_INTSTATUS_ATXOKN_SHF	14
#define ICTA_INTSTATUS_ATXOKN_MSK	(MSK(1) << ICTA_INTSTATUS_ATXOKN_SHF)
#define ICTA_INTSTATUS_ATXOKN_SET	ICTA_INTSTATUS_ATXOKN_MSK

/* bit 13: DEG */
#define ICTA_INTSTATUS_DEG_SHF	        13
#define ICTA_INTSTATUS_DEG_MSK	        (MSK(1) << ICTA_INTSTATUS_DEG_SHF)
#define ICTA_INTSTATUS_DEG_SET	        ICTA_INTSTATUS_DEG_MSK

/* bit 12: ENUM */
#define ICTA_INTSTATUS_ENUM_SHF	        12
#define ICTA_INTSTATUS_ENUM_MSK	        (MSK(1) << ICTA_INTSTATUS_ENUM_SHF)
#define ICTA_INTSTATUS_ENUM_SET	        ICTA_INTSTATUS_ENUM_MSK

/* bit 11: PCID */
#define ICTA_INTSTATUS_PCID_SHF	        11
#define ICTA_INTSTATUS_PCID_MSK	        (MSK(1) << ICTA_INTSTATUS_PCID_SHF)
#define ICTA_INTSTATUS_PCID_SET	        ICTA_INTSTATUS_PCID_MSK

/* bit 10: PCIC */
#define ICTA_INTSTATUS_PCIC_SHF	        10
#define ICTA_INTSTATUS_PCIC_MSK	        (MSK(1) << ICTA_INTSTATUS_PCIC_SHF)
#define ICTA_INTSTATUS_PCIC_SET	        ICTA_INTSTATUS_PCIC_MSK

/* bit 9: PCIB */
#define ICTA_INTSTATUS_PCIB_SHF	        9
#define ICTA_INTSTATUS_PCIB_MSK	        (MSK(1) << ICTA_INTSTATUS_PCIB_SHF)
#define ICTA_INTSTATUS_PCIB_SET	        ICTA_INTSTATUS_PCIB_MSK

/* bit 8: PCIA */
#define ICTA_INTSTATUS_PCIA_SHF	        8
#define ICTA_INTSTATUS_PCIA_MSK	        (MSK(1) << ICTA_INTSTATUS_PCIA_SHF)
#define ICTA_INTSTATUS_PCIA_SET	        ICTA_INTSTATUS_PCIA_MSK

/* bit 7: NMI */
#define ICTA_INTSTATUS_NMI_SHF	        7
#define ICTA_INTSTATUS_NMI_MSK	        (MSK(1) << ICTA_INTSTATUS_NMI_SHF)
#define ICTA_INTSTATUS_NMI_SET	        ICTA_INTSTATUS_NMI_MSK

/* bit 6: CORELO */
#define ICTA_INTSTATUS_CORELO_SHF	6
#define ICTA_INTSTATUS_CORELO_MSK	(MSK(1) << ICTA_INTSTATUS_CORELO_SHF)
#define ICTA_INTSTATUS_CORELO_SET	ICTA_INTSTATUS_CORELO_MSK

/* bit 5: COREHI */
#define ICTA_INTSTATUS_COREHI_SHF	5
#define ICTA_INTSTATUS_COREHI_MSK	(MSK(1) << ICTA_INTSTATUS_COREHI_SHF)
#define ICTA_INTSTATUS_COREHI_SET	ICTA_INTSTATUS_COREHI_MSK

/* bit 4: RTC */
#define ICTA_INTSTATUS_RTC_SHF	        4
#define ICTA_INTSTATUS_RTC_MSK	        (MSK(1) << ICTA_INTSTATUS_RTC_SHF)
#define ICTA_INTSTATUS_RTC_SET	        ICTA_INTSTATUS_RTC_MSK

/* bit 2: TIM1 */
#define ICTA_INTSTATUS_TIM1_SHF	        2
#define ICTA_INTSTATUS_TIM1_MSK	        (MSK(1) << ICTA_INTSTATUS_TIM1_SHF)
#define ICTA_INTSTATUS_TIM1_SET	        ICTA_INTSTATUS_TIM1_MSK

/* bit 1: TIM0 */
#define ICTA_INTSTATUS_TIM0_SHF	        1
#define ICTA_INTSTATUS_TIM0_MSK	        (MSK(1) << ICTA_INTSTATUS_TIM0_SHF)
#define ICTA_INTSTATUS_TIM0_SET	        ICTA_INTSTATUS_TIM0_MSK

/* bit 0: SER */
#define ICTA_INTSTATUS_SER_SHF	        0
#define ICTA_INTSTATUS_SER_MSK	        (MSK(1) << ICTA_INTSTATUS_SER_SHF)
#define ICTA_INTSTATUS_SER_SET	        ICTA_INTSTATUS_SER_MSK


/**** Number of interrupt lines ****/
#define ICTA_IC_COUNT			20



#endif /* #ifndef ICTA_H */
