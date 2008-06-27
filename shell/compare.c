
/************************************************************************
 *
 *  compare.c
 *
 *  Monitor command to compare two memory areas.
 *
 *  compare <addr1> <addr2> <size>
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
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
    UINT32 *addr1,
    UINT32 *addr2,
    UINT32 *length);

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          compare
 ************************************************************************/
static MON_FUNC(compare)
{
    UINT32	addr1;
    UINT32	addr2;
    UINT32	length;
    char	*p1, *p2;
    UINT32      rc;
    

    rc = get_options( argc, argv, &addr1, &addr2, &length );

    if( rc != OK )
        return rc;

    if (!length)
	return OK;

    printf("Comparing memory area 0x%08X..0x%08x with 0x%08X..0x%08x.\n",
		addr1, addr1 + length - 1,
		addr2, addr2 + length - 1);

    p1 = (char *) addr1;
    p2 = (char *) addr2;
	
    while ((int)--length >= 0)
	if (*p1++ != *p2++)
	{
	    printf("The areas are not identical:\n");
	    printf("First difference found at addresses 0x%08X, 0x%08X.\n",
				(UINT32)--p1, (UINT32)--p2);
	    return OK;
	}

    printf("The memory areas are identical.\n");

    return OK;
}

/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *addr1,
    UINT32 *addr2,
    UINT32 *length)
{
    t_shell_option decode;
    UINT32	   type;
    bool	   ok 		 = TRUE;
    bool	   addr1_valid   = FALSE;
    bool	   addr2_valid   = FALSE;
    bool	   length_valid	 = FALSE;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    /* compare takes no options */
	    error	     = SHELL_ERROR_OPTION;
	    shell_error_data = argv[arg];
	    ok		     = FALSE;
	    break;

	  case SHELL_TOKEN_NUMBER :
	    if (!addr1_valid)
	    {
	        *addr1      = decode.number;
		addr1_valid = TRUE;
	    }
	    else if (!addr2_valid)
	    {
	        *addr2      = decode.number;
		addr2_valid = TRUE;
	    }
	    else if (!length_valid)
	    {
		*length      = decode.number;
		length_valid = TRUE;
	    }
	    else
		ok = FALSE;
	    break;

	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( !length_valid )
        ok = FALSE;

    if( !ok )
        return error;
    else
    {
        error = sys_validate_range( *addr1, *length, sizeof(UINT8), FALSE );

	return 
	    ( error != OK ) ?
	        error :
                sys_validate_range( *addr2, *length, sizeof(UINT8), FALSE );
    }
}



/************************************************************************
 *                          cmd stuff
 ************************************************************************/

/* Command definition for compare */

static t_cmd cmd_def =
{
    "compare",
    compare,

    "compare <address1> <address2> <size>",
    "Compares the two specified memory areas. If a difference is\n"
    "encountered during the compare, the address of the first mis-match\n"
    "will be reported.",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_compare_init
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
shell_compare_init( void )
{
    return &cmd_def;
}
