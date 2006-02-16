
/************************************************************************
 *
 *  setenv.c
 *
 *  Shell setenv and unsetenv commands
 *
 *  TBD : Clear env variables if flash is erased.
 *
 *  setenv    [<variable> [<value>]]
 *  unsetenv  <variable> | (-u|-s)
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
#include <env_api.h>
#include <shell_api.h>
#include <ctype.h>
#include <string.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{
    { "u", "Delete all user variables." },
    { "s", "Reset all read/write (R/W) system variables to default values." }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))


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
    char   **name,
    char   **value,
    bool   *user,
    bool   *system );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          setenv
 ************************************************************************/
static MON_FUNC(setenv)
{
    /* Options */
    char           *name;
    char	   *value;
    UINT32	   rc;
 
    rc = get_options( argc, argv, &name, &value, NULL, NULL );

    if( rc != OK )
        return rc;
    else
    {
        if( !name )
	{   
	    env_print_all();
	}
	else
	{
	    if( !value )
	        value = "";

	    rc = env_set( name, value, ENV_ATTR_USER, NULL, NULL );

	    if( rc != OK )
	        return rc;
	}

	return OK;
    }
}


/************************************************************************
 *                          unsetenv
 ************************************************************************/
static MON_FUNC(unsetenv)
{
    /* Options */
    char           *name;
    bool	   user, system;
    UINT32	   rc;

    rc = get_options( argc, argv, &name, NULL, &user, &system );

    if( rc != OK )
        return rc;
    else
    {	
        if(user || system)
	{
	    rc = env_remove( user, system );
	}
        else if( !name )
	{
	    rc = SHELL_ERROR_SYNTAX;
        }
	else
	{
	    rc = env_unset( name );
        }

	return rc;
    }
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    char   **name,
    char   **value,
    bool   *user,
    bool   *system )
{
    bool   ok = TRUE;
    char   *token;
    UINT32 arg;
    UINT32 error = SHELL_ERROR_SYNTAX;

    /* Defaults */
    *name  = NULL;
    if( value )
        *value  = NULL;
    if( user )
        *user   = FALSE;
    if( system )
        *system = FALSE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
		  (token = argv[arg]);
         arg++ )
    {
        if( strcmp( token, "-u" ) == 0 )
	{
	    if( user )
	        *user = TRUE;
	    else
	    {
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = token;
	        ok		 = FALSE;
            }
        }
        else if( strcmp( token, "-s" ) == 0 )
	{
	    if( system )
	        *system = TRUE;
	    else
	    {
    	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = token;
	        ok		 = FALSE;
	    }
        }
	else if ( *token == '-' )
	{
	    error	     = SHELL_ERROR_OPTION;
	    shell_error_data = token;
	    ok		     = FALSE;
        }
        else if( !(*name) )
        {
	    *name  = token;

	    do {
		if (!isalnum(*token) && *token != '_') {
		    ok = FALSE;
		    break;
		}
	    } while (*++token);
        }
	else if( value && !(*value) )
	    *value = token;
	else
	    ok = FALSE;
    }   

    if( ((user && *user) || (system && *system)) && *name )
        ok = FALSE;

    return ok ? OK : error;
}


/* setenv command definition for help */
static t_cmd cmd_def_setenv =
{
    "setenv",
     setenv,
    "setenv [<variable> [<value>]]",

    "Set the specified environment variable.\n"
    "If no variable is specified, all environment variables are displayed.\n"
    "If no value is specified, the variable is set to the NULL value.\n"
    "When setting a R/W system variable, the value is first validated.",

    NULL,
    0,
    FALSE
};

/* unsetenv command definition for help */
static t_cmd cmd_def_unsetenv =
{
    "unsetenv",
     unsetenv,

    "unsetenv  <variable> | (-u|-s)",

    "Unset specified environment variable.\n"
    "If a user created variable is specified, it will be removed from the\n"
    "environment. A system variable will not be removed, but will instead\n"
    "be set to the default value.\n"
    "\n"
    "By using appropriate options, all user and/or system variables can be\n"
    "unset using a single command.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell_setenv_init
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
shell_setenv_init( void )
{
    return &cmd_def_setenv;
}

/************************************************************************
 *
 *                          shell_unsetenv_init
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
shell_unsetenv_init( void )
{
    return &cmd_def_unsetenv;
}


