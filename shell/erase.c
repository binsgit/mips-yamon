
/************************************************************************
 *
 *  erase.c
 *
 *  Shell erase command
 *
 *  erase [-s|-e|<address> <size>]
 *
 *  We differentiate between "monitor flash", "environment flash"  and 
 *  "system flash".
 *
 *  Monitor flash is the flash memory used for holding YAMON.
 *  Environment flash is the flash memory used for environment variables.
 *  System flash is the flash available for the user.
 *
 *  -s ( "Erase system flash" ) option is only available on boards 
 *  having system flash.
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
#include <shell_api.h>
#include <syscon_api.h>
#include <flash_api.h>
#include <sysdev.h>
#include <io_api.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sysenv_api.h>
#include <sys_api.h>
#include <env_api.h>
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
#define OPTION_ENV	0
  { "e",  "Erase and reinitialise entire environment area." },
#define OPTION_SYSTEM	1
  { "s",  "Erase entire system flash (default option)." }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

static char erase_msg[]         = "Erasing...";
static char *syntax_sysflash    = "erase [-s|-e|<address> <size>]";
static char *syntax_no_sysflash = "erase -e | <address> <size>";

static UINT32 default_start;
static UINT32 default_size;
static UINT32 option_count;
static bool   sysflash_avail;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   *system,
    bool   *env,
    UINT32 *start,
    UINT32 *size );

static bool
get_default_range(
    UINT32 *start,
    UINT32 *size );

static UINT32
do_erase(
    bool   system,
    bool   env,
    UINT32 start,
    UINT32 size );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          erase
 ************************************************************************/
static MON_FUNC(erase)
{
    UINT32 start;
    UINT32 size;
    bool   system, env;
    UINT32 rc;
    
    rc = get_options( argc, argv, &system, &env, &start, &size );

    if( rc == OK )
        rc = do_erase( system, env, start, size );

    return rc;
}


/************************************************************************
 *                          erase_sdb
 ************************************************************************/
static MON_FUNC(erase_sdb)
{
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

    return do_erase( FALSE, FALSE, default_start, default_size );
}
   

/************************************************************************
 *                          get_default_range
 ************************************************************************/
static bool
get_default_range(
    UINT32 *start,
    UINT32 *size )
{
    if( 
        (SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BASE_ID,
		     (void *)start,
		     sizeof(UINT32) ) == OK) &&

        (SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,
		     (void *)size,
		     sizeof(UINT32) ) == OK) )
    {
        /* System flash available */
	return TRUE;
    }
    else
    {	
        /* System flash not available */
        return FALSE;
    }
}


/************************************************************************
 *                          do_erase
 ************************************************************************/
static UINT32
do_erase(
    bool   system,
    bool   env,
    UINT32 start,
    UINT32 size )
{
    UINT32 rc = OK;
    t_FLASH_ctrl_descriptor   flash_ctrl;
    char		      msg[120];
    char		      ch;
    bool		      cancelled = FALSE;

    if( system || env )
    {
	if(SHELL_PUTS( erase_msg )) return OK;

        if( system )
        {
            /* Erase entire system flash */

            flash_ctrl.command = FLASH_CTRL_ERASE_SYSTEMFLASH;
            rc = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl) );
            if(rc != OK )
	    {
	        SHELL_PUTC( '\n' );
                return rc;
	    }
        }

        if( env )
        {
            /* Erase entire file flash */

            flash_ctrl.command = FLASH_CTRL_ERASE_FILEFLASH;
            rc = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl) );
            if(rc != OK)
	    {
	        SHELL_PUTC( '\n' );
                return rc;
	    }

	    /* Reinit sysenv (handling of environment records in flash) */
	    SYSENV_init();
        }
    }
    else
    {
        /* Inquire what will actually be erased */

        flash_ctrl.command      = FLASH_CTRL_INQUIRE_FLASH_AREA;
        flash_ctrl.user_physadr = PHYS(start);
        flash_ctrl.user_length  = size;
        rc = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl) );
        if(rc != OK)
	{
            return rc;
	}

        flash_ctrl.user_physadr = flash_ctrl.driver_physadr;
        flash_ctrl.user_length  = flash_ctrl.driver_length;

        sprintf( msg,
	    "The following area will be erased:\n"
	    "Start address = 0x%08x\n"
	    "Size          = 0x%08x\n"
	    "Confirm ? (y/n) ",
            flash_ctrl.user_physadr, flash_ctrl.user_length);

        if(SHELL_PUTS( msg )) 
	{
	    SHELL_PUTC( '\n' );
	    return OK;
	}

	/* Wait for user to type any key */
	do
	{
            while( !GETCHAR( DEFAULT_PORT, &ch ) );
        }
	while( (tolower(ch) != 'y') &&
	       (tolower(ch) != 'n') &&
	       (ch	    != CTRL_C ) );

	if( ch == CTRL_C )
	    rc = SHELL_ERROR_CONTROL_C_DETECTED;
	else
	{
            if(SHELL_PUTC( ch )) 
	    {
	        SHELL_PUTC( '\n' );
	        return OK;
            }
        }

        if( tolower(ch) == 'y' )
        {
	    SHELL_PUTC( '\n' );
	  
	    if(SHELL_PUTS( erase_msg )) 
	    {
	        SHELL_PUTC( '\n' );
		return OK;
	    }

	    flash_ctrl.command = FLASH_CTRL_ERASE_FLASH_AREA;
            rc = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl) );
            if (rc != OK)
	    {
	        SHELL_PUTC( '\n' );
                return rc;
	    }
        }
        else
	    cancelled = TRUE;
    }

    SHELL_PUTS( cancelled ? "\nCancelled\n" : "Done\n" );

    if( !cancelled )
    {
        /* Reinit environment variables */
	env_init();
	if( env_check() )
	{
	    SHELL_PUTC( '\n' );
	}
    }

    return rc;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   *system,
    bool   *env,
    UINT32 *start,
    UINT32 *size )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   int_count = 0;
    bool	   ok = TRUE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup default */
    *system = FALSE;
    *env    = FALSE;
    *start  = default_start;
    *size   = default_size;

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
	    {
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
	        ok		 = FALSE;
	        break;		      
	    }
	    else
	    {
	        switch(i)
	        {
	          case OPTION_SYSTEM :
	            *system = TRUE;
		    *env    = FALSE;
		    break;
	          case OPTION_ENV :
	            *env    = TRUE;
		    *system = FALSE;
		    break;
	          default : /* Should not happen */
		}
	    }

	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( int_count == 0 )
	        *start = decode.number;
	    else if( int_count == 1 )
	        *size  = decode.number;
	    else
	        ok = FALSE;

	    int_count++;
	    break;
	 default :
	    ok = FALSE;
	    break;
       }
    }

    if( *system || *env )
    {
        if( int_count != 0 )
            ok = FALSE;
    }
    else
    {
        /* 0 or 2 numbers is OK unless there is no system flash, in
         * which case 0 numbers is NOT ok (since there is not default
         * range) 
         */

	if( ! ( (int_count == 2) ||
	        ((int_count == 0) && sysflash_avail)
	      ) )
        {
	    ok = FALSE;
        }
    }

    return ok ? OK : error;
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "erase",
     erase,
     NULL,

    "Erase flash memory.\n"
    "\n"
    "An option may be applied specifying which flash region to erase.\n"
    "If no such option is applied, the address range to be erased\n"
    "is specified by the <address> and <size> parameters.\n"
    "If no such range is specified either, the range corresponding to the\n"
    "default option is assumed (if there is a default option, this is\n"
    "platform specific).\n"
    "\n"
    "If (and only if) the -e option (erase environment flash) is\n"
    "applied, the system environment variables are reinitialised to\n"
    "factory default values.\n"
    "\n"
    "If a range is specified, all flash blocks touched by the range\n"
    "are cleared. The block size depends on the flash memory type used by the\n"
    "board. The blocks to be cleared are displayed, and the user is asked\n"
    "for confirmation before the operation is performed.\n"
    "\n"
    "Erasing a large flash area takes time. It can easily take several\n"
    "minutes to erase a 32 MByte area.\n"
    "\n"
    "Any set flash sector lock bits will be cleared before the sector is\n"
    "erased. If they cannot be cleared (e.g. due to hardware protection of the\n"
    "lock bits), the command will fail.",

    options,
    0,
    FALSE
};

/* Command definitions for SDB 'e' command (secret command) */

static t_cmd cmd_def_sdb_lower =
{
    "e",
    erase_sdb,

    "e                        (Microsoft SDB command)",

    "Completely erase system flash. The command is equivalent to 'erase',\n"
    "see 'help erase' for more details.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_erase_init
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
shell_erase_init( void )
{
    /* Determine default range (system flash if available) */
    sysflash_avail = get_default_range( &default_start, &default_size );

    if( sysflash_avail )
    {
        cmd_def.syntax = syntax_sysflash;
	option_count   = OPTION_COUNT;
    }
    else	
    {
        cmd_def.syntax = syntax_no_sysflash;
	option_count   = OPTION_COUNT - 1;
    }

    cmd_def.option_count = option_count;

    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_erase_sdb_init
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
shell_erase_sdb_init( void )
{
    sysflash_avail = get_default_range( &default_start, &default_size );

    return &cmd_def_sdb_lower;
}
