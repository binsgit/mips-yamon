/************************************************************************
 *
 *      lan.c
 *
 *      The 'lan_shared' module implements the common utility functions, 
 *      needed by the LAN-drivers.
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
 *      Include files
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <sys_api.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>

/* lan stuff */
#include <lan_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

static char* lan_error_string[] =
{
    /* ERROR_LAN_NO_FRAME                   */ "RX, No frame received",
    /* ERROR_LAN_COMM_ERROR                 */ "Communication error detected",
    /* ERROR_LAN_NO_TXM_RESOURCES           */ "No TX-driver buffers available",
    /* ERROR_LAN_TXM_ERROR                  */ "TX, frame could not be sent",
    /* ERROR_LAN_INIT_FAILED                */ "Initialization failed"
} ;

static char* lan_error_hint_string[] =
{
    /* ERROR_LAN_NO_FRAME                   */ NULL,
    /* ERROR_LAN_COMM_ERROR                 */ NULL,
    /* ERROR_LAN_NO_TXM_RESOURCES           */ NULL,
    /* ERROR_LAN_TXM_ERROR                  */ NULL,
    /* ERROR_LAN_INIT_FAILED                */ NULL
} ;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static INT32 LAN_error_lookup( t_sys_error_string *p_param ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          LAN_init
 *  Description :
 *  -------------
 *  Initialize the lan modules' error lookup function
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0) 
 ************************************************************************/
UINT32 LAN_error_lookup_init( void )
{
    t_sys_error_lookup_registration registration ;

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_LAN ) ;
    registration.lookup = LAN_error_lookup ;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    return( OK ) ;
}


/************************************************************************
 *
 *                          LAN_error_lookup
 *  Description :
 *  -------------
 *  Convert error code to strings.
 *
 *
 *  Parameters :
 *  ------------
 *  p_param->syserror:   unique LAN specific error code
 *
 *  Return values :
 *  ---------------
 *  p_param->strings[]:  references for error strings 
 *
 ************************************************************************/
static INT32 LAN_error_lookup( t_sys_error_string *p_param )
{
    UINT32 t, i ;

    i = 0 ;
    p_param->count = 0 ;
    t = SYSERROR_ID( p_param->syserror ) ;
     
    /* check for recognized error code */
    if ( (t < sizeof(lan_error_string)/sizeof(char*)) &&
         (lan_error_string[t] != NULL ) )
    {
        /* fill in mandatory error message string */
        p_param->strings[SYSCON_ERRORMSG_IDX] = lan_error_string[t] ;
        i++ ;

        /* check for hint message */
        if ( lan_error_hint_string[t] != NULL)
        {
            /* fill in optional hint message string */
            p_param->strings[SYSCON_HINTMSG_IDX] = lan_error_hint_string[t] ;
            i++ ;
        }
    }
    p_param->count      = i ;
    return(OK) ;
}
