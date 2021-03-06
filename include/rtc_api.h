
/************************************************************************
 *
 *      RTC_api.h
 *
 *      The 'RTC' module implements a device driver 
 *      for MC146818A/DS1685/DS1687 compatible RTCs.
 *
 *        1) init  serial device:  configure and initialize RTC driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  get RTC
 *        5) write serial device:  set RTC
 *        6) ctrl  serial device:  not used
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

#ifndef RTC_API_H
#define RTC_API_H

/************************************************************************
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>

/************************************************************************
 *  Definitions
*************************************************************************/

/************************************************************************
 *   RTC device driver, ERROR completion codes
*************************************************************************/

#define ERROR_RTC         0x00007000 /* for compile time check       */
#define ERROR_RTC_NO_FRAME    0x7000 /* No receive frame present     */
#define ERROR_RTC_COMM_ERROR  0x7001 /* Communication error detected */


/************************************************************************
 *  Parameter definitions
*************************************************************************/

/* RTC value */
typedef struct RTC_calendar
{
    UINT16  year;
    UINT16  month;
    UINT16  dayofmonth;
    UINT16  dayofweek;
    UINT16  hour;
    UINT16  minute;
    UINT16  second;
} 
t_RTC_calendar;


/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          RTC_install
 *  Description :
 *  -------------
 *
 *  Installs the RTC device drivers services in the IO system 
 *  at the reserved device slot, found in the 'sys_dev.h' file, 
 *  which defines all major device numbers.
 *
 *  Note:
 *  This service is the only public declared interface function; all
 *  provided device driver services are static declared, but this
 *  function installs the function pointers in the io-system to
 *  enable the provided public driver services.
 *
 *  Parameters :
 *  ------------
 *
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 
RTC_install( void );



#endif /* #ifndef RTC_API_H */
