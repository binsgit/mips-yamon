
/************************************************************************
 *
 *  search.c
 *
 *  Monitor command to search for ascii or hex string in memory.
 *
 *  search [-asc|-hex] <address> <length> <string>
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <sys_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define	STRTYPE_ASC		1	/* ascii string */
#define	STRTYPE_HEX		2	/* hex string */
#define SEARCH_DEFAULT_STRTYPE	STRTYPE_ASC

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_ASC	0
  { "asc",  "Search for ascii string" },
#define OPTION_HEX	1
  { "hex",  "Search for hex string" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *strtype,
    UINT32 *addr,
    UINT32 *length,
    char   **pstr);

static int hexd(                        /* Get hex digit        */
    char ch);

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          search
 ************************************************************************/
static MON_FUNC(search)
{
    UINT32  strtype;
    UINT32  addr;
    UINT32  length;
    char    *str;
    char    sdata[81];
    char    ch;
    UINT32  slen, out, i;
    int	    d1, d2;
    bool    found, ctrlc;
    UINT32  rc;
    char    msg[80];

    rc = get_options( argc, argv, &strtype, &addr, &length, &str );

    if( rc != OK )
        return rc;

    if (!length)
	return OK;

    slen  = 0;
    out   = 0;
    found = FALSE;
    ctrlc = FALSE;

    if (strtype == STRTYPE_HEX)
    {
	/* skip prefix 0x, if present */
	if (str[0] == '0'  &&  tolower(str[1]) == 'x')
	    str += 2;

    	while ((ch = *str++)  &&  (slen < sizeof(sdata)-1))
    	{
	    d1 = hexd(ch);
	    d2 = hexd(*str++);

	    if ((d1 < 0) || (d2 < 0))
	    {
	        return SHELL_ERROR_SYNTAX;
	    }

	    sdata[slen++] = (d1<<4) | d2;
	}
    }
    else
    {
    	while ((ch = *str++)  &&  (slen < sizeof(sdata)))
    	{
	    sdata[slen++] = ch;
	}
    }

    sdata[slen] = 0;

    SHELL_PUTS("Searching after ");

    if (strtype == STRTYPE_ASC)
    {
	if (slen < 30)
	{
	    SHELL_PUTC( '"' );
	    SHELL_PUTS( sdata );
	    SHELL_PUTC( '"' );
        }
	else
	{
	    SHELL_PUTS( "ascii-string" );
	}
    }
    else
    {
	if (slen < 9)
	{
            SHELL_PUTS( "0x" );

	    for (i = 0;  i < slen;  i++)
	    {
	        sprintf( msg, "%02x", (UINT8)sdata[i] );
		SHELL_PUTS( msg );
	    }
	}
	else
	{
	    SHELL_PUTS( "hex-string" );
	}
    }

    sprintf( msg, " in area 0x%08X..0x%08x.\n",
	     addr, addr + length - 1);

    SHELL_PUTS( msg );

    SHELL_PUTS(shell_msg_ctrl_c);

    /* Now do the search */
    for (i = 0;  i <= length - slen;  i++)
    {
	if (memcmp((void *) (addr+i), sdata, slen) == 0)
	{
	    found = TRUE;
	    if ((out++ % 6) == 0)
	    {
		if( SHELL_PUTC( '\n' ) ) 
		{
		    ctrlc = TRUE;
		    break;
		}
	    }

	    sprintf( msg, "0x%08X  ", addr+i);
	    if( SHELL_PUTS( msg ) )
	    {
	        ctrlc = TRUE;
		break;
	    }
	}

	if ((i & 0x0FFF) == 0x0000)
	{
	    if (GETCHAR_CTRLC(DEFAULT_PORT))
	    {
		ctrlc = TRUE;
		break;
	    }
	}
    }

    if (!found)
    {
	SHELL_PUTS("\nNo match found.");
    }

    SHELL_PUTC( '\n' );

    return ctrlc ? SHELL_ERROR_CONTROL_C_DETECTED : OK;
}

/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *strtype,
    UINT32 *addr,
    UINT32 *length,
    char   **pstr)
{
    char	   *token;
    t_shell_option decode;
    UINT32	   type;
    bool	   ok 		= TRUE;
    bool	   addr_valid	= FALSE;
    bool	   length_valid	= FALSE;
    bool	   str_valid	= FALSE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    *strtype = SEARCH_DEFAULT_STRTYPE;

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
	      case OPTION_ASC   :
		*strtype = STRTYPE_ASC; break;
	      case OPTION_HEX   :
		*strtype = STRTYPE_HEX; break;
	      default :
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = token;
		ok		 = FALSE;
	        break;		      
	    }
	    break;

	  case SHELL_TOKEN_NUMBER :

	    if (!addr_valid)
	    {
		*addr      = decode.number;
		addr_valid = TRUE;
		break;
	    }
	    else if (!length_valid)
	    {
		*length      = decode.number;
		length_valid = TRUE;
		break;
	    }
	    /* else fall-through */

	  case SHELL_TOKEN_STRING :

	    /* Use raw token as search string */
	    if (!str_valid)
	    {
		*pstr     = token;
		str_valid = TRUE;
	    }
	    else
	        ok = FALSE;
	    break;

	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( !length_valid || !str_valid )
        ok = FALSE;

    return ok ?
        sys_validate_range( *addr, *length, sizeof(UINT8), FALSE ) :
	error;
}

/************************************************************************
 *                          hexd
 ************************************************************************/

static int hexd(                        /* Get hex digit		*/
    char ch )				/* Char				*/
{
        if (!isxdigit(ch)) return -1;
        if (islower(ch)) ch = toupper(ch);
        if (isalpha(ch)) ch -= 7;
  
        return ch - 48;
}


/************************************************************************
 *                          cmd stuff
 ************************************************************************/

/* Command definition for search */
static t_cmd cmd_def =
{
    "search",
    search,
    "search [-asc|-hex] <address> <size> <string>",

    "Search for string in the memory area specified by <address> and <size>.\n"
    "Default string type is ASCII. If the search string contains spaces,\n"
    "remember to use quotes around the string.\n"
    "\n"
    "If searching for a hex string, the search pattern must be entered as\n"
    "a number of two-digit hexcodes without spaces inbetween.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_search_init
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
shell_search_init( void )
{
    return &cmd_def;
}


