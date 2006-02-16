#ifndef FLASH_API_H
#define FLASH_API_H

/************************************************************************
 *
 *      FLASH_api.h
 *
 *      The 'FLASH_api' module defines the FLASH device driver
 *      interface to be used via 'FLASH' device driver services:
 *
 *        1) init  device:  configure and initialize FLASH driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  not used
 *        5) write device:  write FLASH device
 *        6) ctrl  device:  a) ERASE_SYSTEMFLASH
 *                          b) ERASE_FILEFLASH
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
#include "syserror.h"

/************************************************************************
 *   FLASH device driver, ERROR completion codes
*************************************************************************/

#define ERROR_FLASH                  0x00009000 /* for compile time check    */
#define ERROR_FLASH_PROGRAM_ERROR        0x9000 /* Flash device failure      */
#define ERROR_FLASH_INVALID_ADDRESS      0x9001 /* Physical address not impl.*/
#define ERROR_FLASH_INVALID_COMMAND      0x9002 /* Ctrl. Command not supported*/
#define ERROR_FLASH_TIME_OUT             0x9003 /* FLASH operation timed out */
#define ERROR_FLASH_VERIFY_ERROR         0x9004 /* Program error detected during verify */
#define ERROR_FLASH_LOCKED               0x9005 /* Some sector(s) locked     */
#define ERROR_FLASH_ERASE_ERROR          0x9006 /* Some sector(s) have erase error */
#define ERROR_FLASH_LOW_VOLTAGE          0x9007 /* Low programming voltage detected */
#define ERROR_FLASH_WRITE_PROTECTED      0x9008 /* Flash is write protected  */
#define ERROR_FLASH_FILE_FLASH_PROT      0x9009 /* Environment FLASH is write protected */
#define ERROR_FLASH_FILE_FLASH_LOCK      0x900a /* Environment FLASH is lock-bit protected */
#define ERROR_FLASH_MONITOR_FLASH_LOCK   0x900b /* MONITOR FLASH sector(s) locked */
#define ERROR_FLASH_QRY_NOT_FOUND        0x900c /* CFI Query-id string not found */
#define ERROR_FLASH_BOOT_WRITE_PROTECTED 0x900d /* Boot is write protected   */


/************************************************************************
 *   FLASH 'ctrl' service, command codes
*************************************************************************/


typedef enum FLASH_ctrl_command_ids
{
    FLASH_CTRL_ERASE_SYSTEMFLASH = 0,      /* FLASH 'ERASE' SYSTEMFLASH */
    FLASH_CTRL_ERASE_FILEFLASH,            /* FLASH 'ERASE' FILEFLASH   */
    FLASH_CTRL_ERASE_FLASH_AREA,           /* FLASH 'ERASE' (partial) 
                                                     FLASH AREA         */
    FLASH_CTRL_INQUIRE_FLASH_AREA,         /* FLASH 'INQUIRE' committed
                                                     FLASH BLOCK AREA   */
    FLASH_CTRL_TEST_SYSTEMFLASH,           /* FLASH 'TEST' SYSTEMFLASH  */
    FLASH_CTRL_WRITE_FILEFLASH,            /* FLASH 'WRITE' FILEFLASH   */
    FLASH_CTRL_TEST_MONITORFLASH           /* FLASH 'TEST' MONITORFLASH */
} t_FLASH_ctrl_command_ids;


/************************************************************************
 *  Parameter definitions
*************************************************************************/


/* 
    FLASH write descriptor, to be used with FLASH-'write' service
                                                                        */
typedef struct FLASH_write_descriptor
{
    UINT32 adr ;           /* physical address to start programming;
                              this address can be in FLASH or RAM
                              space.                                   */ 
    UINT32 length ;        /* number of bytes to write                 */
    UINT8  *buffer ;       /* pointer for buffer of data to be written */
} t_FLASH_write_descriptor ;


/*
    FLASH ctrl descriptor, to be used with specific FLASH services
                                                                        */
typedef struct FLASH_ctrl_descriptor
{
    UINT32 command ;       /* IN: 'ctrl' command (ERASE_SYSTEMFLASH,..)     */
    UINT32 user_physadr ;  /* IN:  users request of physical address 
                                                    to start erase          */ 
    UINT32 user_length ;   /* IN:  users request of number of bytes 
                                                    to erase                */
    UINT32 driver_physadr ;/* OUT: drivers committed physical address 
                                                    to start erase          */ 
    UINT32 driver_length ; /* OUT: drivers committed number of bytes 
                                                    to erase                */
    t_FLASH_write_descriptor
           *wr_param ;     /* IN:  pointer for buffer of data to be written */
} t_FLASH_ctrl_descriptor ;


#endif /* #ifndef FLASH_API_H */
