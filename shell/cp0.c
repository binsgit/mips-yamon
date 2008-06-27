
/************************************************************************
 *
 *  cp0.c
 *
 *  Read/Write CP0 register
 *
 *  cp0					|
 *  cp0 <name> [<value>]		|
 *  cp0 [-t n] [-<0..7>] [-32|-64] <regnum> [<value>]
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <shell_api.h>
#include <sys_api.h>
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
#define OPTION_SEL0	0
  { "0",  "Select = 0 (default option)"		       },
#define OPTION_SEL1	1
  { "1",  "Select = 1"				       },
#define OPTION_SEL2	2
  { "2",  "Select = 2"			     	       },
#define OPTION_SEL3	3
  { "3",  "Select = 3"				       },
#define OPTION_SEL4	4
  { "4",  "Select = 4"				       },
#define OPTION_SEL5	5
  { "5",  "Select = 5"				       },
#define OPTION_SEL6	6
  { "6",  "Select = 6"				       },
#define OPTION_SEL7	7
  { "7",  "Select = 7"				       },
#define OPTION_TC       8
  { "t", "Select TC"				       },
#define OPTION_W32	9
  { "32", "Access 32 bit of register (default option)" },
#define OPTION_W64	10
  { "64", "Access 64 bit of register"		       }
};
#define OPTION_COUNT64	(sizeof(options)/sizeof(t_cmd_option))
#define OPTION_COUNT32	(sizeof(options)/sizeof(t_cmd_option)-2)

static UINT32 option_count;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
get_options(
    UINT32 argc,
    char   **argv, 
    char   **reg_name,
    UINT32 *tc_number,
    UINT32 *reg_number,
    UINT32 *sel_number,
    UINT32 *width,
    UINT64 *value,
    bool   *read );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          cp0
 ************************************************************************/
static MON_FUNC(cp0)
{
    char   *reg_name;
    UINT32 tc_number, reg_number, sel_number, width;
    UINT64 value;
    bool   read;
    char   msg[80];
    UINT32 rc;

    /* Options */
    rc = get_options( argc, argv, 
		      &reg_name, &tc_number, &reg_number, &sel_number, &width,
		      &value, &read );

    if( rc != OK )
        return rc;

    /* If no register is selected, display all */
    if( (reg_name == NULL) && (reg_number == 0xffffffff) )
    {
	SHELL_PUTC( '\n' );

	rc = sys_cp0_printreg_all(tc_number, NULL );

        if( rc != OK )
            return rc;

	SHELL_PUTC( '\n' );
    }
    else
    {
        /* A register was selected, either by name or number */

	if( reg_name )
	{
	    if( read )
	        return sys_cp0_printreg( tc_number, reg_name );
	    else
	        return sys_cp0_writereg( tc_number, reg_name, value );
        }
	else
	{
	    /* Register selected by number and possibly select field */

	    if( (reg_number > 31) || (sel_number > 7) )
	        return SHELL_ERROR_UNKNOWN_CP0_REG;

	    if( read )
	    {
	        if( width == 32 )
		{
		    if (sys_mt) 
			value = (UINT64)sys_cp0_mtread32(reg_number, sel_number, tc_number );
		    else
			value = (UINT64)sys_cp0_read32(reg_number, sel_number );

		    sprintf( msg, "CP0(%d,%d) = 0x%08x\n", 
			     reg_number,
			     sel_number,
			     (UINT32)value );
		}
		else
		{
		    value = sys_cp0_read64(reg_number, sel_number );

		    sprintf( msg, "CP0(%d,%d) = 0x%08x%08x\n",
			     reg_number,
			     sel_number,
			     HI32(value), LO32(value) );
		}

   	        SHELL_PUTS( msg );
	    }
	    else
	    {
	        if( width == 32 ) {
		    if (sys_mt)
			sys_cp0_mtwrite32(reg_number, sel_number, (UINT32)value , tc_number);
		    else
			sys_cp0_write32(reg_number, sel_number, (UINT32)value );
		}
		else {
		    sys_cp0_write64(reg_number, sel_number, value );
		}
		/* The write may have changed the L2 cache setup */
		if (sys_l2cache)
		    sys_l2cache_enabled = sys_cpu_l2_enabled();
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
    UINT32 *tc_number,
    UINT32 *reg_number,
    UINT32 *sel_number,
    UINT32 *width,
    UINT64 *value,
    bool   *read )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    bool	   ok    = TRUE;
    UINT32	   error = SHELL_ERROR_SYNTAX;
    UINT32	   i;
    UINT64	   number[2];
    UINT32	   number_count = 0;
    UINT32	   option_arg   = 0xff;
    char	   *endp;

    /* Defaults */
    *tc_number  = 0;
    *reg_name   = NULL;
    *reg_number = 0xffffffff;
    *sel_number = 0;
    *width	= 32;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
        switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    /* Find match */
	    for(i=0; 
		(i<option_count) &&
		(strcmp(decode.option, options[i].option) != 0);
		i++) ;

	    if( i == option_count )
	        i = 0xff; /* Make sure we hit default in below switch */
	     
	    option_arg = arg;

	    switch(i)
	    {
	      case OPTION_SEL0 : *sel_number = 0;  break;
	      case OPTION_SEL1 : *sel_number = 1;  break;
	      case OPTION_SEL2 : *sel_number = 2;  break;
	      case OPTION_SEL3 : *sel_number = 3;  break;
	      case OPTION_SEL4 : *sel_number = 4;  break;
	      case OPTION_SEL5 : *sel_number = 5;  break;
	      case OPTION_SEL6 : *sel_number = 6;  break;
	      case OPTION_SEL7 : *sel_number = 7;  break;
	      case OPTION_W32  : *width	     = 32; break;
	      case OPTION_W64  : *width	     = 64; break;
	      case OPTION_TC   :
		  if (arg+1 < argc) {
		      *tc_number = strtoul( argv[arg+1], &endp, 0);
		      if (*endp == '\0') {
			  option_arg = 0xff; /* Botch */
			  arg++;
			  break;
		      }
		  }
		  /* fall through */

	      default :
		error	         = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }

	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( number_count == 2 )
	        ok = FALSE;
	    else
	        number[number_count++] = (UINT64)decode.number;
	    break;
	  case SHELL_TOKEN_NUMBER64 :
	    if( (number_count == 2) || !sys_64bit )
	        ok = FALSE;
	    else
	        number[number_count++] = decode.number64;
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
        if( option_arg != 0xff )
        {
	    error	     = SHELL_ERROR_OPTION;
	    shell_error_data = argv[option_arg];
	    ok		     = FALSE;
        }
	else
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
    }
    else
    {
        switch( number_count )
	{
	  case 0 :
            if( option_arg != 0xff )
            {
	        error	         = SHELL_ERROR_OPTION;
	        shell_error_data = argv[option_arg];
	        ok		 = FALSE;
            }
	    break;
	  case 1 :
	    *read       = TRUE;
	    *reg_number = number[0] >> 32 ? 0xffffffff : (UINT32)number[0];
	    break;
	  case 2 :
	    *read       = FALSE;
	    *reg_number = number[0] >> 32 ? 0xffffffff : (UINT32)number[0];
	    *value	= number[1];
	    break;
          default :
	    ok = FALSE;
	}
    }

    return ok ? OK : error;
}

static char *syntax_32 = "cp0 [ (<name> | ([-<0..7>] <regnum>)) [<value>] ]";
static char *syntax_64 = "cp0 [ (<name> | ([-<0..7>] [-32|-64] <regnum>)) [<value>] ]";
static char *syntax_mt32 = "cp0 [ [-t <tc>] (<name> | ([-<0..7>] <regnum>)) [<value>] ]";
static char *syntax_mt64 = "cp0 [ [-t <tc>] (<name> | ([-<0..7>] [-32|-64] <regnum>)) [<value>] ]";

/* Command definition for help */
static t_cmd cmd_def =
{
    "cp0",
    cp0,

    NULL,

    "Read/write CP0 register(s).\n"
    "\n"
    "If no arguments are applied, all CP0 registers are read.\n"
    "A register may be selected by name (case insensitive) or number (hex)\n"
    "(and possibly select value using -0,-1,-2,...,-7 option).\n"
    "Select 0 is default.\n"    
    "If a value is given, this value is written to the register, otherwise\n"
    "the register is read.\n"
    "Writing a CP0 register takes effect immediately and should be done\n"
    "with care since it may crash YAMON.\n"
    "Some of the CP0 registers are optional and not available for all CPUs.\n"
    "\n"
    "Settings of CP0 registers are also applied to user applications\n"
    "(started with 'go' or 'gdb') except for STATUS, for which the\n"
    "IE field is cleared.\n"
    "TLB related registers as well as COUNT and COMPARE are\n"
    "undefined when user application is first started.",

    options,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_cp0_init
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
shell_cp0_init( void )
{
    if (sys_mt) {
	if (sys_64bit) {
	    option_count         = OPTION_COUNT64;
	    cmd_def.syntax       = syntax_mt64;
	}
	else {
	    option_count         = OPTION_COUNT32;
	    cmd_def.syntax       = syntax_mt32;
	}
    }
    else {
	if (sys_64bit) {
	    option_count         = OPTION_COUNT64-1;
	    cmd_def.syntax       = syntax_64;
	}
	else {
	    option_count         = OPTION_COUNT32-1;
	    cmd_def.syntax       = syntax_32;
	}
    }

    cmd_def.option_count = option_count;

    return &cmd_def;
}

