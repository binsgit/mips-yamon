/************************************************************************
 *
 *      sysenv_api.h
 *
 *      The 'sysenv' module implements the system environment variable
 *      write and read functions, which operate on a RAM and FLASH
 *      storage media. The functions are normally called through the
 *      SYSCON api.
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


#ifndef SYSENV_API_H
#define SYSENV_API_H


/************************************************************************
 *    Include files
 ************************************************************************/

#include "sysdefs.h"
#include "syserror.h"




/************************************************************************
 *   SYSENV, ERROR completion codes
*************************************************************************/

#define ERROR_SYSENV              0x0000c000 /* for compile time check       */
#define ERROR_SYSENV_OUT_OF_MEMORY    0xc000 /* No more dynamic memory       */
#define ERROR_SYSENV_ENV_VAR_TOO_BIG  0xc001 /* environment var size too big */
#define ERROR_SYSENV_NO_VARIABLE      0xc002 /* environment var not created  */
#define ERROR_SYSENV_INVALID_INDEX    0xc003 /* environment var index invalid */
#define ERROR_SYSENV_UPDATE_READ_REFS 0xc004 /* all references for env 
                                                variables must be re-read    */
#define ERROR_SYSENV_FLASH_INVALID    0xc005 /* FLASH has been corrupted     */


/************************************************************************
 *  System Definitions
*************************************************************************/

/* User environment. */
#define SYS_USER_ENVIRONMENT_MAX_INDEX  99
#define SYS_USER_ENVIRONMENT_DATA_SIZE  124
typedef struct sys_user_environment_var
{
    UINT32 index ;      /* index of this env. variable                  */
    UINT32 size  ;      /* size of user environment data                */
    void   *data_inout ;/* input/output pointer for user env. data      */
} t_user_environment_var ;


/************************************************************************
 *    Public variables
 ************************************************************************/

/************************************************************************
 *   Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          SYSENV_check_state
 *  Description :
 *  -------------
 *
 *  Check the 'sysenv' state.
 *
 *
 *
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  ERROR_SYSENV_FLASH_INVALID, sysenv is corrupted.
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
INT32 SYSENV_check_state( void ) ;

/************************************************************************
 *
 *                          SYSENV_init
 *  Description :
 *  -------------
 *
 *  Initializes the 'sysenv' module.
 *  
 *  
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
INT32 SYSENV_init( void ) ;


/************************************************************************
 *
 *                          SYSENV_read
 *  Description :
 *  -------------
 *
 *  Read a system environment variable. Unformatted data are read from
 *  a data array, accessed via an 'index'. Data are not copied, just
 *  a pointer reference is being returned to the data array and the
 *  actual size of the data, which have been stored.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'var',          INOUT, data are not copied
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *
 *
 ************************************************************************/
INT32 SYSENV_read( t_user_environment_var *var ) ;


/************************************************************************
 *
 *                          SYSENV_write
 *  Description :
 *  -------------
 *
 *  Write data to a system environment variable. Unformatted data are 
 *  written to a data array, accessed via an 'index'. Data are copied, 
 *  into non-volatile memory (FLASH) and a pointer reference is
 *  returned to the data-array of this variable in FLASH. The actual
 *  size is stored too, to be returned in a 'read' for this variable.
 *  A system variable is being created with the first 'write' operation
 *  to this variable and deleted, if any succeeding 'write' to
 *  this 'index' contains a 'size' parameter of '0'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'var',          INOUT, data are copied into non-volatile memory
 *                         and the storage-pointer for this memory
 *                         is being returned in the 'data_inout'
 *                         parameter of the parameter block.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *
 *
 ************************************************************************/
INT32 SYSENV_write( t_user_environment_var *var ) ;


#endif /* #ifndef SYSENV_API_H */
