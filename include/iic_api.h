#ifndef IIC_API_H
#define IIC_API_H

/************************************************************************
 *
 *      IIC_api.h
 *
 *      The 'IIC_api' module defines the IIC device driver
 *      interface to be used via 'IIC' device driver services:
 *
 *        1) init  serial device:  configure and initialize IIC driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  read IIC device
 *        5) write serial device:  write IIC device
 *        6) ctrl  serial device:  a) write+read IIC device
 *                                 b) test IIC device
 *
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
 *    Include files
 ************************************************************************/

#include "sysdefs.h"
#include "syserror.h"


/************************************************************************
 *   IIC device driver, ERROR completion codes
*************************************************************************/

#define ERROR_IIC              0x00005000 /* for compile time check       */
#define ERROR_IIC_COMM_ERROR       0x5000 /* Communication error detected */
#define ERROR_IIC_ADDRESS_ERROR    0x5001 /* Address error                */
#define ERROR_IIC_DATA_ERROR       0x5002 /* Data error                   */
#define ERROR_IIC_TIMEOUT          0x5003 /* Timeout                      */
#define ERROR_IIC_UNKNOWN_COMMAND  0x5004 /* Unknown ctrl command         */


/************************************************************************
 *   IIC 'ctrl' service, command codes
*************************************************************************/


typedef enum IIC_ctrl_command_ids
{

    IIC_CTRL_WRITE_READ = 0,                  /* Write Read command     */
    IIC_CTRL_TEST,                            /* Test command           */

} t_IIC_ctrl_command_ids;



/************************************************************************
 *  Parameter definitions
*************************************************************************/

/* 
    IIC read descriptor, to be used with IIC-'read' service
                                                                        */
typedef struct IIC_read_descriptor
{
    UINT8  IICaddress ;    /* 7-bit IIC slave address */
    UINT8  length ;        /* number of bytes to read */
    UINT8  *buffer ;       /* pointer for buffer to store read data */
} t_IIC_read_descriptor ;


/* 
    IIC write descriptor, to be used with IIC-'write' service
                                                                        */
typedef struct IIC_write_descriptor
{
    UINT8  IICaddress ;    /* 7-bit IIC slave address */
    UINT8  length ;        /* number of bytes to write */
    UINT8  *buffer ;       /* pointer for buffer to be written */
} t_IIC_write_descriptor ;


/* 
    IIC write-read descriptor, to be used with IIC-'write_read' service
                                                                        */
typedef struct IIC_write_read_descriptor
{
    UINT8  write_length ;  /* number of bytes to write */
    UINT8  *write_buffer ; /* pointer for buffer to be written */
    UINT8  read_length ;   /* number of bytes to read */
    UINT8  *read_buffer ;  /* pointer for buffer to store read data */
} t_IIC_write_read_descriptor ;

/* 
    IIC ctrl descriptor, to be used with aggregated IIC services
                                                                        */
typedef struct IIC_ctrl_descriptor
{
    UINT8  IICaddress ;    /* 7-bit IIC slave address */
    UINT8  command ;       /* 'ctrl' command */
    void   *data ;         /* specific data per command */
} t_IIC_ctrl_descriptor ;


#endif /* #ifndef IIC_API_H */
