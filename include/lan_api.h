#ifndef LAN_API_H
#define LAN_API_H

/************************************************************************
 *
 *      LAN_api.h
 *
 *      The 'LAN_api' module defines the lan device driver
 *      interface to be used via 'LAN' device driver services:
 *
 *        1) init  device:  configure and initialize LAN driver
 *        2) open  device:  register receive handler
 *        3) close device:  not used
 *        4) read  device:  read frame
 *        5) write device:  write frame
 *        6) ctrl  device:  control interfaces
 *
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




/************************************************************************
 *    Include files
 ************************************************************************/

#include "sysdefs.h"

/************************************************************************
 *   LAN device driver, ERROR completion codes
*************************************************************************/

#define ERROR_LAN              0x00004000 /* for compile time check       */
#define ERROR_LAN_NO_FRAME         0x4000 /* No receive frame present     */
#define ERROR_LAN_COMM_ERROR       0x4001 /* Communication error detected */
#define ERROR_LAN_NO_TXM_RESOURCES 0x4002 /* No buffer resources present  */
#define ERROR_LAN_TXM_ERROR        0x4003 /* Transmission error detected  */
#define ERROR_LAN_INIT_FAILED      0x4004 /* Initialization failed        */


typedef enum LAN_ctrl_command_ids
{
    LAN_CTRL_DISPLAY_STATISTICS = 0,    /* LAN, display statistics */
    LAN_CTRL_STOP_CONTROLLER,           /* LAN, stop any LAN controller
                                                activity, e.g. DMA */
    LAN_CTRL_START_CONTROLLER,          /* LAN, start LAN controller
                                                activity, e.g. DMA */
} t_LAN_ctrl_command_ids;


/************************************************************************
 *  Parameter definitions
*************************************************************************/

/* 
   LAN (Ethernet) frame buffer.
                                                                        */
#define LAN_MAX_FRAME_LENGTH  6+6+2+1500
typedef UINT8 t_LAN_frame_buffer[LAN_MAX_FRAME_LENGTH] ;

/* 
   IO (open) descriptor.
                                                                        */
typedef struct LAN_OPEN_desc
{
    UINT32 (*receive)( UINT32 length, UINT8 *data ) ;
} t_LAN_OPEN_desc ;


/* 
   IO (read and write) descriptor.
                                                                        */
typedef struct LAN_IO_desc
{
    UINT32 length ;
    UINT8  *data ;
} t_LAN_IO_desc ;

/* 
   CTRL  descriptor.
                                                                        */
typedef struct LAN_CTRL_desc
{
    UINT32 command ;
    void   *data ;
} t_LAN_CTRL_desc ;



/************************************************************************
 *  LAN drivers shared services, called by any LAN driver
*************************************************************************/

UINT32 LAN_error_lookup_init( void ) ;

#endif /* #ifndef LAN_API_H */
