
/************************************************************************
 *
 *  test.c
 *
 *  Shell test command
 *
 *  test [-l] | [-m] [ <module> [ <module parameters> ] ]
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
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <shell_api.h>
#include <syscon_api.h>
#include "shell.h"
#include <sysdev.h>
#include <io_api.h>
#include <flash_api.h>
#include <sys_api.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
 *  "Private" externals
 ************************************************************************/

/* From memtest.S */

extern UINT32 errordump[];
extern UINT32 mem_pattern_write(UINT32, UINT32, UINT32, UINT32);
extern UINT32 *mem_pattern_verify(UINT32, UINT32, UINT32, UINT32);

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Requirements for SDB (Atlas) set by Microsoft */
#define SDB_REQUIREMENT_RAMSIZE	(64 * 1024 * 1024)

typedef struct test_mod {
    char    *t_module_name;
    UINT32   t_module_index;
    char    *t_start_text;
    char    *t_help_text;
    UINT32   t_arg_count;
    bool   (*t_func)();
} t_test_mod;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   *help,
    bool   *until_break,
    t_test_mod **pmodule,
    UINT32 *parg1,
    UINT32 *parg2,
    UINT32 *parg3 );

static bool
do_test(
    t_test_mod *pmodule,
    bool       write_msg,
    UINT32     tstarg1,
    UINT32     tstarg2,
    UINT32     tstarg3 );

static void
get_default_ram(
    UINT32 *pstart,
    UINT32 *psize,
    UINT32 *ploops );

static bool
test_ram( 
    UINT32 start, 
    UINT32 size,
    UINT32 loops );

static bool
test_flash( void ); 

static bool
test_all( void );

static UINT32
ram_range_valid(
    UINT32 *pstart,
    UINT32 *psize );

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{
#define OPTION_HELP	0
  { "m", "List available test modules" },
#define OPTION_LOOP	1
  { "l", "Loop default tests until Ctrl-C is pressed" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

static t_test_mod modules[] =
{
#define MODULE_RAM   0
    { "ram", MODULE_RAM, "Testing RAM\n",

	"\nUsage: test ram [<address> <size> <loops>]\n"
	"Default start address is the first RAM address not used by YAMON.\n"
	"Default size is all RAM not used by YAMON.\n"
        "Default loop count is 10.\n"
	"Loop count of 0 will cause the test to loop indefinitely.\n"
	"Start address is 256 byte aligned (by adding between 0 and 255).\n"
	"Size is 256 byte aligned (by subtracting between 0 and 255 ).\n"
        "In case this causes the end address to increase, size is\n"
        "reduced by 256.\n"
        "Range is not restricted to RAM.\n\n",

	3, &test_ram },

#define MODULE_FLASH 1
    { "flash", MODULE_FLASH, "Testing Flash\n",

	"\nUsage: test flash\n"
	"Performs a non-destructive test of the system and environment flash memory\n"
	"areas (if present). The test communicates with the state-machine in the\n"
	"flash chips for every sector in the areas tested.\n\n",
	0, &test_flash },

#define MODULE_ALL   2
    { "all", MODULE_ALL, "Testing ALL\n",
	"\nUsage : test [-m] [ <module> ]\n\nAvailable modules :\n",
	0, &test_all },
};

static bool   ctrl_c;
static int    wix;
static UINT32 err;


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          test
 ************************************************************************/
static MON_FUNC(test)
{
    /* Options */
    bool        help;
    bool        until_break;
    t_test_mod *pmodule;
    UINT32     i;
    UINT32     tstarg1, tstarg2, tstarg3;
    UINT32     rc;

    rc = get_options( argc, argv, &help, &until_break, &pmodule, 
		      &tstarg1, &tstarg2, &tstarg3 );

    if( rc != OK )
        return rc;
    else
    {
	err = OK;
    
	if( help )
	{
	    if (SHELL_PUTS( pmodule->t_help_text )) 
	    {
	        return OK;
	    }

            if( pmodule->t_module_index == MODULE_ALL) {
	        for( i = 0; i < MODULE_ALL; i++ )
	        {
                    if( SHELL_PUTS( modules[i].t_module_name ) ||
		        SHELL_PUTC( '\n' ) )
                    {
                        break;
                    }
	        }
	            
		SHELL_PUTC( '\n' );
		return OK;
            }
	}
	else
	{
	    if (until_break)
	    {
		SHELL_DISABLE_MORE;
	    }

	    while (TRUE)
	    {
	        i = do_test( pmodule,	
		     (pmodule->t_module_index == MODULE_ALL) ? FALSE : TRUE,
		     tstarg1, tstarg2, tstarg3 );
		printf("\n");
		if (!i || !until_break)
		    break;
	    }
        }

	return err;
    }
}


/************************************************************************
 *                          test_sdb
 ************************************************************************/
static MON_FUNC(test_sdb)
{
    bool rc;
    UINT32 start, size, dummy;

    if( argc != 1 )
    {
        if( *(argv[1]) == '-' )
	{
	    shell_error_data = argv[1];
	    return SHELL_ERROR_OPTION;
	}
	else
	    return SHELL_ERROR_SYNTAX;
    }

    err = OK;

    if( sys_ramsize < SDB_REQUIREMENT_RAMSIZE )
    {	
        SHELL_PUTS( "Not enough RAM\n" );
    }
    else
    {
        get_default_ram( &start, &size, &dummy );

    	rc = do_test( &modules[ MODULE_RAM ], TRUE, start, size, 1 );

        SHELL_PUTC( '\n' );

	if( !rc )
            return err;

    	rc = do_test( &modules[ MODULE_FLASH ], TRUE, 0, 0, 0 );

        SHELL_PUTC( '\n' );

	if( !rc )
            return err;
    }

    return err;
}


/************************************************************************
 *                          do_test
 ************************************************************************/
static bool
do_test(
    t_test_mod *pmodule,
    bool       write_msg,
    UINT32     tstarg1,
    UINT32     tstarg2,
    UINT32     tstarg3 )
{
    bool pass;

    ctrl_c = FALSE;
    if( SHELL_PUTS( pmodule->t_start_text )) return FALSE;

    pass = pmodule->t_func( tstarg1, tstarg2, tstarg3 );

    if( write_msg )
    {
        if( ctrl_c )
	{
	    err = SHELL_ERROR_CONTROL_C_DETECTED;
	}
	else
	{
            SHELL_PUTS( 
	        pass ? 
	            "Test passed" : 
		    "Test failed" );
        }
    }

    return (pass && !ctrl_c) ? TRUE : FALSE;
}


/************************************************************************
 *                          get_default_ram
 ************************************************************************/
static void
get_default_ram(
    UINT32 *pstart,
    UINT32 *psize,
    UINT32 *ploops )
{
    /* Setup default RAM parameters */
    *pstart = (KSEG1(sys_freemem) + 0xff) & ~0xff;  
    *psize  = KSEG1(sys_ramsize) - *pstart;
    *ploops = 10;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    bool   *help,
    bool   *until_break,
    t_test_mod **pmodule,
    UINT32 *parg1,
    UINT32 *parg2,
    UINT32 *parg3 )
{
    t_shell_option decode;
    UINT32	   type;
    char	   *token;
    UINT32	   i;
    UINT32	   modix;
    bool	   ok    = TRUE;
    UINT32	   count = 0;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;
    UINT32	   rc;

    /* Defaults */
    *help        = FALSE;
    *until_break = FALSE;
    modix        = MODULE_ALL;
 
    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
		  (token = argv[arg]) &&
                  shell_decode_token( token, &type, &decode );
         arg++ )
    {
        switch( type )
	{
	  case SHELL_TOKEN_OPTION :
            /* Find match */
	    for(i=0; 
	        (i<OPTION_COUNT) &&
		(strcmp(decode.option, options[i].option) != 0);
		i++) ;

	    switch(i)
	    {
	      case OPTION_HELP :
	        *help = TRUE;
		break;
	      case OPTION_LOOP :
	        *until_break = TRUE;
		break;
	      default :
	        error		 = SHELL_ERROR_OPTION;
	        shell_error_data = token;
		ok		 = FALSE;
		break;		      
	    }

	    break;

	  case SHELL_TOKEN_NUMBER :
	  
	     if( count == 0 )
	         *parg1 = decode.number;
	     else if( count == 1 )
	         *parg2 = decode.number;
	     else if( count == 2 )
	         *parg3 = decode.number;
	     else
	         ok = FALSE;

	     count++;

	     break;

	  case SHELL_TOKEN_STRING :
	    /* Use raw token to find match with module name */

	    if( modix != MODULE_ALL )
	        ok = FALSE;
	    else
	    {
	        for(modix=0; 
	            (modix < MODULE_ALL) &&
		    (strcmp(token, modules[modix].t_module_name) != 0);
		    (modix)++) ;
		
		if( modix == MODULE_ALL )
		    ok = FALSE;
	    }

	    break;
        }
    }

    if( !ok )
    {
        return error;
    }

    *pmodule = &modules[modix];

    if( *help )
    {
        return 
            ( *until_break || (count != 0) ) ?
	        SHELL_ERROR_SYNTAX :
		OK;
    }

    if( *until_break )
    {
        return
	    ( *help || (count != 0) || (modix != MODULE_ALL) ) ?
	        SHELL_ERROR_SYNTAX :
		OK;
    }

    /* --- argument special considerations --- */

#define pstart parg1
#define psize  parg2
#define ploops parg3

    if( modix == MODULE_ALL )
	get_default_ram(pstart, psize, ploops);

    if( modix == MODULE_RAM )
    {
        if( count == 0 )
        {
	    get_default_ram(pstart, psize, ploops);
            count = 3;
        }

        if( count == 3 )
	{ 
            /* Check validity of addresses, and adjust if possible */
            rc = ram_range_valid(pstart, psize);

	    if( rc != OK )
	        return rc;
        }
    }

#undef pstart
#undef psize
#undef ploops

    /* --- special considerations end --- */

    return
        ( count != modules[modix].t_arg_count ) ?
            SHELL_ERROR_SYNTAX :
	    OK;
}


/************************************************************************
 *                          test_all
 ************************************************************************/
static bool
test_all( void )
{	  
    UINT32 rambase, srambase, size, loops;

    srambase = 0;

    get_default_ram( &rambase, &size, &loops );
    SYSCON_read( SYSCON_BOARD_SRAM_BASE_ID, &srambase, sizeof(srambase) );

    if( !do_test(&modules[ MODULE_RAM ], TRUE, rambase, size, loops) )
        return FALSE;

    if( SHELL_PUTC( '\n' ) ) return FALSE;
    
    if (srambase)
    {
        SYSCON_read( SYSCON_BOARD_SRAM_SIZE_ID, &size, sizeof(size) );
    	if ( !do_test(&modules[ MODULE_RAM ], TRUE, KSEG1(srambase), size, loops) )
            return FALSE;

    	if ( SHELL_PUTC( '\n' ) ) return FALSE;
    }

    if (!do_test( &modules[ MODULE_FLASH ], TRUE, 0, 0, 0 ))
        return FALSE;

    return TRUE;
}


/************************************************************************
 *                          test_ram
 ************************************************************************/
static bool
test_ram( 
    UINT32 start, 
    UINT32 size,
    UINT32 loops )
{
#define	   TEST_BLOCK	0x80000
    UINT32 cnt;
    UINT32 dummy;
    UINT32 newstart, newend;
    UINT32 seed;
    UINT32 *erroradr;
    char   *wheel = "|/-\\";

    printf("Memory test from 0x%08X to 0x%08X, ",
			   start,
			   start + size - 4 );

    if( !loops )	   
        printf("loop indefinitely.\n" );
    else
        printf("%d loop%s\n", loops, loops == 1 ? "." : "s.");

    if (loops != 1 || (size > TEST_BLOCK))
    	printf( "%s\n", shell_msg_ctrl_c);

    erroradr = 0;

    for (cnt=0; ((loops == 0) || (cnt<loops)) && erroradr==0 && !ctrl_c; cnt++)
    {
	printf("\rNow running loop %d %c", 
		cnt+1, wheel[wix = ((wix+1) & 0x3)]);

        seed = cnt;
	for ( newstart=start; 
	      (newstart<start+size) && erroradr==0 && !ctrl_c; 
	      newstart += TEST_BLOCK)
	{
	    newend = newstart + TEST_BLOCK;
	    if (newend > start+size)
		newend = start+size;

	    dummy = newstart;
            mem_pattern_write(newstart, (UINT32)&dummy, newend, seed);
	    seed = dummy;
	    printf("\b%c", wheel[wix = ((wix+1) & 0x3)]);
	    if ( GETCHAR_CTRLC(DEFAULT_PORT) )
		ctrl_c = TRUE;
	}

        seed = cnt;
	for ( newstart=start; 
	      (newstart<start+size) && erroradr==0 && !ctrl_c; 
	      newstart += TEST_BLOCK)
	{
	    newend = newstart + TEST_BLOCK;
	    if (newend > start+size)
		newend = start+size;

	    dummy = newstart;
	    if ((erroradr = mem_pattern_verify(newstart, (UINT32)&dummy, newend, seed)))
	    {
	    	printf("\nFailed at address: 0x%08X\n", (UINT32)erroradr);
	    	printf("Written    Read       Difference Re-Read\n");
	    	for (cnt=0; cnt<8; cnt++)
		    printf("0x%08X 0x%08X 0x%08X 0x%08X\n",
			errordump[cnt*2],  errordump[cnt*2+1],
			errordump[cnt*2] ^ errordump[cnt*2+1],
			erroradr[cnt]);
	    }
	    seed = dummy;
	    
	    if (erroradr == 0)
	    {
	    	printf("\b%c", wheel[wix = ((wix+1) & 0x3)]);
	    	if ( GETCHAR_CTRLC(DEFAULT_PORT) )
	    	    ctrl_c = TRUE;
	    }
	}
    }

    if (!ctrl_c)
    	printf("\n");

    return erroradr == 0;
}


/************************************************************************
 *                          test_flash
 ************************************************************************/
static bool
test_flash( void )
{
    UINT32 rc ;
    t_FLASH_ctrl_descriptor   flash_ctrl;

    flash_ctrl.command = FLASH_CTRL_TEST_SYSTEMFLASH;

    
    rc = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl)) ;
    if (rc)
    {
        err = rc;
        return FALSE ;
    }
    return TRUE;
} 


/************************************************************************
 *                          ram_range_valid
 ************************************************************************/
static UINT32
ram_range_valid(
    UINT32 *pstart,
    UINT32 *psize )
{
    UINT32 adj_start;
    UINT32 adj_size;

    if( *pstart + *psize < MAX( *pstart, *psize ) )
    {
	/* Overflow */
	return SHELL_ERROR_RAM_RANGE;
    }

    /* Adjust start and size to be 0x100 aligned */
    adj_start = (*pstart + 0xff) & ~0xff;
    adj_size  = *psize & ~0xff;

    while  ( (adj_size != 0)  &&
	     (adj_start + adj_size) > (*pstart + *psize) )
    {
	adj_size -= 0x100;
    }

    if( adj_size == 0 )
        return SHELL_ERROR_RAM_RANGE;

    *pstart = adj_start;
    *psize  = adj_size;

    return
        sys_validate_range( adj_start,
			    adj_size,
			    1,
			    TRUE );
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "test",
     test,
    "test [-l] | [-m] [ <module> [ <module arguments> ] ]",

    "The test command can perform a number of self-tests on different\n"
    "modules. If no module is supplied, all available modules are tested\n"
    "and a final pass/fail status is indicated. If a module is specified,\n"
    "only this module is tested.\n"
    "\n"
    "If the option '-m' is applied and no module is specified, a list\n"
    "of the available modules is displayed.\n"
    "\n"
    "If the option '-m' is applied and a module is specified, additional\n"
    "information about the module test and the optional arguments is\n"
    "displayed.\n"
    "\n"
    "If the option '-l' is applied, all available modules are tested\n"
    "repetitively, until Ctrl-C is pressed or a test fails. The '-l'\n"
    "option cannot be specified together with other options or arguments.",

    options,
    OPTION_COUNT,
    FALSE
};

/* Command definitions for SDB 't' command (secret command) */
static t_cmd cmd_def_sdb_lower =
{
    "t",
    test_sdb,
    "t                        (Microsoft SDB command)",
    "Performs self-test as required by Microsoft SDB requirements.\n"
    "Detects and tests RAM and flash memories. A final pass/fail status\n"
    "is indicated.\n"
    "\n"
    "In order to run this test, a minimum of 64 MByte SDRAM and 32 MByte\n"
    "flash must be available. If these requirements are not met, the test\n"
    "will not start at all.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_test_init
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
shell_test_init( void )
{
    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_test_sdb_init
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
shell_test_sdb_init( void )
{
    return &cmd_def_sdb_lower;
}

