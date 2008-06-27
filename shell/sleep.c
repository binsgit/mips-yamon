
/************************************************************************
 *
 *  sleep.c
 *
 *  Monitor "sleep" command
 *
 *  sleep <ms>
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


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <shell_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *ms );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          sleep
 ************************************************************************/
static MON_FUNC(sleep)
{
    UINT32 rc;
    UINT32 ms;

    rc = get_options( argc, argv, &ms );

    if( rc != OK )
        return rc;
    else
    {
        do
	{
	    if( ms < 10 )
	    {
                sys_wait_ms( ms );
		ms = 0;
	    }
	    else
	    {
		sys_wait_ms( 10 );
		ms -= 10;

		if( GETCHAR_CTRLC( DEFAULT_PORT ) )
 	            return SHELL_ERROR_CONTROL_C_DETECTED;
	    }
	}
	while( ms );

	return OK;
    }
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *ms )
{
    t_shell_option decode;
    UINT32	   type;

    if( argc != 2 )
        return SHELL_ERROR_SYNTAX;

    shell_decode_token( argv[1], &type, &decode );

    switch( type )
    {
      case SHELL_TOKEN_OPTION :
	shell_error_data = argv[1];
	return SHELL_ERROR_OPTION;
      case SHELL_TOKEN_NUMBER :
	*ms = decode.number;
	return OK;
      default :
	return SHELL_ERROR_SYNTAX;
    }
}

/* Command definition for dump */
static t_cmd cmd_def =
{
    "sleep",
    sleep,
    "sleep <ms>",

    "Halt shell for the specified number of milliseconds.\n"
    "Note that the default number format is hexadecimal.",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_sleep_init
 *  Description :
 *  -------------
 *
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
t_cmd *
shell_sleep_init( void )
{
    return &cmd_def;
}
