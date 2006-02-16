/************************************************************************
 *
 *  copy.c
 *
 *  Shell copy command
 *
 *  copy [-f] <src> <dst> <size>
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
#include <sysdev.h>
#include <io_api.h>
#include <flash_api.h>
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
#define OPTION_FLUSH	0 
    { "f",  "Do not flush caches" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *src,
    UINT32 *dst,
    UINT32 *size,
    bool   *flush );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          copy
 ************************************************************************/
static MON_FUNC(copy)
{
    /* Options */
    UINT32                    rc ;
    UINT32		      src, dst, size;
    bool		      flush;
    t_FLASH_write_descriptor  flash_write ;

    rc = get_options( argc, argv, &src, &dst, &size, &flush );
    
    if( rc != OK )
        return rc;
    else
    {
        if( size != 0 )
	{
            if(SHELL_PUTS( "Copying..." ))
	    {
	        SHELL_PUTC( '\n' );
		return OK;
            }

            flash_write.adr =    dst;
            flash_write.length = size;
            flash_write.buffer = (UINT8 *)src;

	    /* Possibly flush caches */
	    if( flush )
	        sys_flush_caches();

	    /* This flash copy function can also handle RAM */
            if( (rc = IO_write( SYS_MAJOR_FLASH_STRATA, 0, &flash_write )) != OK )
            {
	        SHELL_PUTC( '\n' );
	        return rc;
            }
            else
            {
                if(SHELL_PUTS( "Done\n" )) return OK;
            }

	    /* Possibly flush caches */
	    if( flush )
	        sys_flush_caches();
        }

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
    UINT32 *src,
    UINT32 *dst,
    UINT32 *size,
    bool   *flush )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   count = 0;
    bool           ok    = TRUE;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Defaults */
    *flush = TRUE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
        switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    if( strcmp(decode.option, options[OPTION_FLUSH].option) == 0 )
	    {
	        *flush = FALSE;
	    }
	    else
	    {
	        error	         = SHELL_ERROR_OPTION;
	        shell_error_data = argv[arg];
	        ok		 = FALSE;
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    switch( count )
	    {
	      case 0  : *src  = decode.number; break;
	      case 1  : *dst  = decode.number; break;
	      case 2  : *size = decode.number; break;
	      default : ok    = FALSE; break;
	    }    
	    count++;
	    break; 
	  default :
	    ok = FALSE;
	    break;
	}
    }

    if( count != 3 )
        ok = FALSE;

    if( !ok )
        return error;
    else
    {
        error = sys_validate_range( *src, *size, sizeof(UINT8), FALSE );

	return 
	    ( error != OK ) ?
	        error :
                sys_validate_range( *dst, *size, sizeof(UINT8), TRUE );
    }
}

/* Command definition for help */
static t_cmd cmd_def =
{
    "copy",
     copy,
    "copy [-f] <src> <dst> <size>",

    "The number of bytes specified by <size> are copied from <src> to <dst>.\n"
    "Both the source and destination can be located anywhere in the\n"
    "address space. The copy command knows the address areas for the\n"
    "flash memories in the system and is able to program them.\n"
    "\n" 
    "If the destination is flash, the destination area must be cleared\n"
    "using the 'erase' command prior to the copy operation.\n"
    "\n"
    "Note that the copy command prevents the user from overwriting the\n"
    "environment flash area.\n"
    "\n"
    "Unless the -f option is applied, caches are flushed before and after\n"
    "the copy operations (D-cache writeback and invalidate,\n"
    "I-cache invalidate).",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_copy_init
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
shell_copy_init( void )
{
    return &cmd_def;
}
