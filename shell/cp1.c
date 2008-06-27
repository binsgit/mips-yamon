
/************************************************************************
 *
 *  cp1.c
 *
 *  Read/Write CP1 register
 *
 *  cp1					|
 *  cp1 <name>   [<value>]		|
 *  cp1 <regnum> [<value>]
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
#include <shell_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <shell.h>
#include <mips.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    char   *name;
    UINT32 number;
}
t_cp1_reg;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static t_cp1_reg regs[] =
{
    { "fir",  R_C1_FIR  },
    { "fcsr", R_C1_FCSR }
};
#define REG_COUNT (sizeof(regs)/sizeof(t_cp1_reg))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
get_options(
    UINT32 argc,
    char   **argv, 
    char   **reg_name,
    UINT32 *reg_number,
    UINT32 *value,
    bool   *read );


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          cp1
 ************************************************************************/
static MON_FUNC(cp1)
{
    char   *reg_name;
    UINT32 reg_number;
    UINT32 value;
    bool   read;

    UINT32 i;
    char   msg[80];
    UINT32 rc;
    UINT32 printed = 0;

    /* Options */
    rc = get_options( argc, argv, &reg_name, &reg_number, &value, &read );

    if( rc != OK )
        return rc;

    /* If no register is selected, display all */
    if( (reg_name == NULL) && (reg_number == 0xffffffff) )
    {
	SHELL_PUTC( '\n' );

        for(i=0; i<REG_COUNT; i++ )
	{
	    value = sys_cp1_read( regs[i].number );
        
	    if( (printed % 2) == 0 )
	    {
	        if( SHELL_PUTS( regs[i].name ) )
		    return OK;
		sprintf( msg, "= 0x%08x", value );
		if( SHELL_PUTS_INDENT( msg, 4 ) )
		    return OK;
	    }
	    else
	    {
	        if( SHELL_PUTS_INDENT( regs[i].name, 19 ) )
		    return OK;
		sprintf( msg, "= 0x%08x\n", value );
		if( SHELL_PUTS_INDENT( msg, 24 ) )
		    return OK;
	    }

	    printed++;
	}

	SHELL_PUTS( (i % 2) ? "\n\n" : "\n" );
    }
    else
    {
        /* A register was selected, either by name or number */

	if( reg_name )
	{
	    /* Register selected by name */

            for( i=0; 
	             (i < REG_COUNT) && 
		     (strcmp(reg_name, regs[i].name) != 0);
	         i++ );

	    if( i == REG_COUNT )
	        return SHELL_ERROR_UNKNOWN_CP1_REG;

	    if(read)
	    {
	        value = sys_cp1_read( regs[i].number );

		sprintf( msg, "%s = 0x%08x\n", regs[i].name, value );
		    
   	        SHELL_PUTS( msg );
	    }
	    else
	        sys_cp1_write( regs[i].number, value );
        }
	else
	{
	    /* Register selected by number and possibly select field */

	    if( reg_number > 31 )
	        return SHELL_ERROR_UNKNOWN_CP1_REG;

	    if( read )
	    {
	        value = sys_cp1_read( reg_number );

		sprintf( msg, "CP1(%d) = 0x%08x\n", 
			 reg_number,
			 value );

		SHELL_PUTS( msg );
	    }
	    else
	    {
	        sys_cp1_write( reg_number, value );
	    }
	}
    }

    return OK;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv, 
    char   **reg_name,
    UINT32 *reg_number,
    UINT32 *value,
    bool   *read )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    bool	   ok    = TRUE;
    UINT32	   error = SHELL_ERROR_SYNTAX;
    UINT32	   number[2];
    UINT32	   number_count = 0;

    /* Defaults */
    *reg_name   = NULL;
    *reg_number = 0xffffffff;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
        switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    error	     = SHELL_ERROR_OPTION;
	    shell_error_data = argv[arg];
	    ok		     = FALSE;
	    break;		      
	  case SHELL_TOKEN_NUMBER :
	    if( number_count == 2 )
	        ok = FALSE;
	    else
	        number[number_count++] = decode.number;
	    break;
	  default :
	    if( *reg_name )
	        ok = FALSE;
	    else
	        *reg_name = argv[arg];
	    break;
	}
    }

    if( !ok ) return error;

    if( *reg_name )
    {
        switch( number_count )
        {
	  case 0 :
	    *read = TRUE; 
	    break;
	  case 1 :
	    *read  = FALSE;
	    *value = number[0];
	    break;
          default :
	    ok = FALSE;
        }
    }
    else
    {
        switch( number_count )
	{
	  case 0 :
	    break;
	  case 1 :
	    *read       = TRUE;
	    *reg_number = number[0];
	    break;
	  case 2 :
	    *read       = FALSE;
	    *reg_number = number[0];
	    *value	= number[1];
	    break;
          default :
	    ok = FALSE;
	}
    }

    return ok ? OK : error;
}

/* Command definition for help */
static t_cmd cmd_def =
{
    "cp1",
    cp1,

    "cp1 [ (<name> | <regnum>) [<value>] ]",

    "Read/write CP1 control register(s).\n"
    "\n"
    "If no arguments are applied, all CP1 control registers are read.\n"
    "A register may be selected by name or number.\n"
    "If a value is given, this value is written to the register, otherwise\n"
    "the register is read.\n"
    "Writing a CP1 control register takes effect immediately.\n"
    "\n"
    "Settings of CP1 control registers are also applied to user applications\n"
    "(started with 'go' or 'gdb').",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_cp1_init
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
shell_cp1_init( void )
{
    return &cmd_def;
}

