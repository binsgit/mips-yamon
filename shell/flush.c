
/************************************************************************
 *
 *  flush.c
 *
 *  Monitor command for flushing caches
 *
 *  flush [-i|-d]
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
#include <shell_api.h>
#include <sys_api.h>
#include <string.h>
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
#define OPTION_ICACHE	0
  { "i", "Invalidate I-cache" },
#define OPTION_DCACHE   1
  { "d", "Flush D-cache" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *option );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          flush
 ************************************************************************/
static MON_FUNC(flush)
{
    /* Options */
    UINT32 option;
    UINT32 rc;

    rc = get_options( argc, argv, &option );
    
    if( rc != OK )
        return rc;
    else    
    {
        switch( option )
	{
	  case OPTION_ICACHE :
	    sys_icache_invalidate_all();
	    break;
	  case OPTION_DCACHE :
	    sys_dcache_flush_all();
	    break;
	  case OPTION_COUNT :	/* No option	     */
	    sys_flush_caches();
	    break;
	  default :		/* Should not happen */
	    break;
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
    UINT32 *option )
{
    t_shell_option decode;
    UINT32	   type;
    bool	   ok = TRUE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup default */
    *option = OPTION_COUNT;

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
		(i<OPTION_COUNT) &&
		(strcmp(decode.option, options[i].option) != 0);
		i++) ;

	    if( i<OPTION_COUNT )
	        *option = i;
	    else
	    {
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
	        ok		 = FALSE;
            }
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
    "flush",
     flush,
    "flush [-i|-d]",

    "Flush cache(s). By default, the D-cache is flushed first, followed\n"
    "by an I-cache invalidate. This behaviour can be changed by the '-i'\n"
    "and '-d' options.\n"
    "\n"
    "The D-cache flush operation is in reality a write-back of dirty lines\n"
    "(write-back caches only) followed by an invalidate operation.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_flush_init
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
shell_flush_init( void )
{
    return &cmd_def;
}
