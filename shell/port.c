
/************************************************************************
 *
 *  port.c
 *
 *  Read/Write operation to single address
 *
 *  port [-a] [-8|-16|-32] <address> [<value>]
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
#include <sys_api.h>
#include <string.h>
#include <stdio.h>
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

/* OPTIONS */
static t_cmd_option options[] =
{
#define OPTION_W8		0
  { "8",  "Access data byte" },
#define OPTION_W16	        1
  { "16", "Access data halfword" },
#define OPTION_W32		2
  { "32", "Access data word" },
#define OPTION_NO_VALIDATE	3
  { "a", "Allow invalid addresses" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
get_options(
    UINT32 argc,
    char   **argv, 
    UINT32 *width, 
    UINT32 *address, 
    UINT32 *data, 
    bool   *read );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          port
 ************************************************************************/
static MON_FUNC(port)
{
    UINT32 width;
    UINT32 address;
    UINT32 data;
    bool   read;
    char   str1[40], str2[40];
    UINT32 rc;

    /* Options */

    rc = get_options( argc, argv, &width, &address, &data, &read );

    if( rc != OK )
        return rc;
    else
    {
	if(read)
	{
	    switch(width)
	    {
	      case sizeof(UINT8) :
		data = REG8( address );
		break;
	      case sizeof(UINT16) :
	        data = REG16( address );
		break;
	      case sizeof(UINT32) :
	        data = REG32( address );
		break;
	      default :
		break;
	    }

	    sprintf( str1, "Value(0x%08x) = 0x", address );
	}
	else
	{
	    switch(width)
	    {
	      case sizeof(UINT8) :
		REG8( address ) = data;
		break;
	      case sizeof(UINT16) :
	        REG16( address ) = data;
		break;
	      case sizeof(UINT32) :
	        REG32( address ) = data;
		break;
	      default :
		break;
	    }

    	    sprintf( str1, "Address (0x%08x) set to 0x", address );
	}

	switch(width)
	{
	  case sizeof(UINT8) :
 	    sprintf( str2, "%02x\n", data );
	    break;
	  case sizeof(UINT16) :
	    sprintf( str2, "%04x\n", data );
	    break;
	  case sizeof(UINT32) :
	    sprintf( str2, "%08x\n", data );
	    break;
	  default :
	    break;
	}

	strcat( str1, str2 );
	
        SHELL_PUTS( str1 );

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
    UINT32 *width, 
    UINT32 *address, 
    UINT32 *data, 
    bool   *read )
{
    t_shell_option decode;
    UINT32	   type;
    bool	   address_valid  = FALSE;
    bool	   ok		  = TRUE;
    bool	   validate       = TRUE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Defaults */
    *width = sizeof(UINT32);
    *read  = TRUE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
        switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    for(i=0; 
	        (i<OPTION_COUNT) &&
		(strcmp(decode.option, options[i].option) != 0);
		i++) ;

	    switch(i)
	    {
	      case OPTION_W8 :
	        *width = sizeof(UINT8);  break;
	      case OPTION_W16 :
	        *width = sizeof(UINT16); break;
	      case OPTION_W32 :
	        *width = sizeof(UINT32); break;
	      case OPTION_NO_VALIDATE :
	        validate = FALSE;
		break;
	      default :
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
	        ok		 = FALSE;
		break;
	    }
	
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( !address_valid )
	    {
	        *address      = decode.number;
		address_valid = TRUE;
	    }
	    else if(*read)
	    {
		*read = FALSE;
		*data = decode.number;
	    }
	    else
	    {
		ok = FALSE;
	    }
	    break;
	  default :
	    ok = FALSE;
	    break;
	}
    }

    if( !address_valid || !ok )
        return error;
    else
    {
        /* Validate address */
        if( validate )
	{
	    return sys_validate_range(*address, *width, *width, !(*read));
	}
	else if(!(*read) && (*data > MAXUINT(*width)))
	{
	    error = SHELL_ERROR_DATA_WIDTH;
	    ok    = FALSE;
	}
    }

    return ok ? OK : error;
}

/* Command definition for help */
static t_cmd cmd_def =
{
    "port",
    port,
    "port [-a] [-8|-16|-32] <address> [<value>]",

    "Perform a read or write operation to the specified <address> with\n"
    "the specified data width (default 32 bits). If <value> is specified,\n"
    "this value is written, otherwise a read operation is performed and\n"
    "the result is displayed.\n"
    "\n"
    "The command checks the validity of the specified address.\n"
    "This check can be turned off using the '-a' option.\n"
    "\n"
    "The port command will result in exactly one read or write operation\n"
    "with the specified data width. This makes it useful for accessing\n"
    "registers in peripheral devices.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_port_init
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
shell_port_init( void )
{
    return &cmd_def;
}
