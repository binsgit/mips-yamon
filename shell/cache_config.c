
/************************************************************************
 *
 *  cache_config.c
 *
 *  Shell command for configuring KSEG0 use of caches by setting K0 field
 *  of CONFIG register.
 *
 *  Second level cache may be enabled/disabled if CPU supports this.
 *
 *  cache [ <value> | list of options ]
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
#include <syscon_api.h>
#include <mips.h>
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

static UINT32				config_count;
static t_shell_cache_config_name_val	*mapping;

char   syntax[100] = "cache [ <value>";

char   descr[1000] = 
    "Configure KSEG0 cache algorithm by setting K0 field of CPU CONFIG register.\n"
    "Second level cache may be enabled/disabled as well if CPU supports this.\n"
    "If no parameter is supplied, the current cache setting is displayed.\n"
    "The command will cause caches to be flushed.\n"
    "\n"
    "K0 settings are CPU specific. The mappings are shown below\n"
    "(a specific value may be given instead of these symbols).\n";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   *display,
    UINT8  *value );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          cache
 ************************************************************************/
static MON_FUNC(cache)
{
    UINT32 rc;
    bool   display;
    UINT8  value;
    UINT32 config;
    UINT32 i;
    char   msg[80];

    rc = get_options( argc, argv, &display, &value );

    if( rc != OK )
        return rc;
    else
    {
        if( display )
	{
	    /* Get K0 setting */
	    SYSCON_read(
	        SYSCON_CPU_CP0_CONFIG_ID,
		(void *)&config,
		sizeof(UINT32) );

	    config = (config & M_ConfigK0) >> S_ConfigK0;

	    /* Find match */
	    for(i=0; 
		(i<config_count) &&
		(config != mapping[i].val);
		i++) ;

	    if( i==config_count )
	    {
	        /* No match, simply display value */
	        sprintf( msg, "K0 = %d", config );
	    }
	    else
	    {
	        sprintf( msg, "K0 = %d (%s)", config, mapping[i].descr );
	    }

	    if( sys_l2cache )
	        strcat( msg, sys_l2cache_enabled ? 
			        ", L2 enabled" :
			        ", L2 disabled" );

	    strcat( msg, "\n" );

	    SHELL_PUTS( msg );
        }
	else
	{
	    switch( value )
	    {
	      case SHELL_CACHE_CONFIG_L2_ENABLE :
	        sys_cpu_l2_enable( TRUE );
		sys_l2cache_enabled = TRUE;
		break;
	      case SHELL_CACHE_CONFIG_L2_DISABLE :
	        sys_cpu_l2_enable( FALSE );
		sys_l2cache_enabled = FALSE;
		break;
	      default :
	        sys_cpu_k0_config( value );
		break;
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
    bool   *display,
    UINT8  *value )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   i;

    if( argc == 1 )
    {
        *display = TRUE;
	return OK;
    }
    else
        *display = FALSE;

    if( argc != 2 ) 
        return SHELL_ERROR_SYNTAX;
    else
    {
        shell_decode_token( argv[1], &type, &decode );

	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    shell_error_data = argv[1];
	    return SHELL_ERROR_OPTION;
	  case SHELL_TOKEN_NUMBER :
	    *value = decode.number;
	    return
	        ( *value >  (M_ConfigK0 >> S_ConfigK0) ) ?
	            SHELL_ERROR_ILLEGAL_CACHE_CFG :
		    OK;
	  case SHELL_TOKEN_STRING :
	    /* Find match */
	    for(i=0; 
		(i<config_count) &&
		(strcmp(decode.option, mapping[i].name) != 0);
		i++) ;

	    if( i == config_count )	
	        return SHELL_ERROR_SYNTAX;
	    else
	    {
	        *value = mapping[i].val;
	        return OK;
	    }
	  default :
	    return SHELL_ERROR_SYNTAX;
        }
    }
}


/* Command definition for cache */
static t_cmd cmd_def =
{
    "cache",
    cache,

    NULL,
    NULL,

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_cache_init
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
shell_cache_init( void )
{
    UINT32 i;
    char   msg[100];

    shell_arch_cache_config( &mapping, &config_count );

    for( i=0; i<config_count; i++ )
    {
        /* Syntax */
        strcat( syntax, " | " );
        strcat( syntax, mapping[i].name );

	/* Description */
        sprintf( msg, "\n%8s : %d (%s)",
		 mapping[i].name,
		 mapping[i].val,
		 mapping[i].descr );
        strcat( descr, msg );
    }
    strcat( syntax, " ]" );

    cmd_def.syntax = syntax;
    cmd_def.descr  = descr;
    
    return config_count ? &cmd_def : NULL;
}
