
/************************************************************************
 *
 *  dump.c
 *
 *  Monitor command for dumping address range
 *
 *  dump [-m][-8|-16|-32] <address> [<size>]
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
#include <sys_api.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define DUMP_DEFAULT_WIDTH      sizeof(UINT8)
#define DUMP_DEFAULT_LENGTH     0x100
#define DUMP_BYTES_PER_LINE	16

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_MORE	0
  { "m",  "Prompt user for keypress after each screen of data" },
#define OPTION_W8	1
  { "8",  "Dump data in units of bytes" },
#define OPTION_W16	2
  { "16", "Dump data in units of halfwords" },
#define OPTION_W32	3
  { "32", "Dump data in units of words" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/* Options */
static bool   more;
static UINT32 width, address, length;

static UINT32 rc = OK;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   sdb,
    UINT32 *address,
    bool   *more,
    UINT32 *width,
    UINT32 *length );

static void
do_dump( void );

static void
int2str( 
    char  *val_string,
    UINT32 val,
    UINT32 width );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          dump
 ************************************************************************/
static MON_FUNC(dump)
{
    if( !shell_dot )
        rc = get_options( argc, argv, FALSE, 
			  &address, &more, &width, &length );

    if( rc == OK )    
        rc = sys_validate_range( address, length, width, FALSE );

    if( (rc == OK) && length )
        do_dump();    

    return rc;
}


/************************************************************************
 *                          dump_sdb
 ************************************************************************/
static MON_FUNC(dump_sdb)
{
    UINT32 rc;

    rc = get_options( argc, argv, TRUE, &address, &more, &width, &length );

    if( rc != OK )
        return rc;
    else
    {
        do_dump();
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
    bool   sdb,
    UINT32 *address,
    bool   *more,
    UINT32 *width,
    UINT32 *length )
{
    t_shell_option decode;
    UINT32	   type;

    bool	   address_valid = FALSE;
    bool	   length_valid  = FALSE;
    bool	   ok		 = TRUE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    *more  = FALSE;

    if( sdb )
    {
        /* SDB requirements */

        *width = sizeof( UINT32 );
	*length = 8 * sizeof(UINT32);
    }
    else
    {
        *width  = DUMP_DEFAULT_WIDTH;
        *length = DUMP_DEFAULT_LENGTH;
    }

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    if(sdb)
	    {
		error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
	    }
	    else
	    {
	        /* Find match */
	        for(i=0; 
		    (i<OPTION_COUNT) &&
		    (strcmp(decode.option, options[i].option) != 0);
		    i++) ;

	        switch(i)
		{
		  case OPTION_MORE :
		    *more  = TRUE; break;
		  case OPTION_W8   :
		    *width = sizeof(UINT8); break;
		  case OPTION_W16  :
		    *width = sizeof(UINT16); break;
		  case OPTION_W32  :
		    *width = sizeof(UINT32); break;
		  default :
		    error	     = SHELL_ERROR_OPTION;
		    shell_error_data = argv[arg];
		    ok		     = FALSE;
		    break;		      
		}
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( !address_valid )
	    {
		address_valid = TRUE;
		*address      = decode.number;
	    }
	    else if( !length_valid )
	    {
		if( sdb )
		    ok = FALSE;
		else
		{
		    length_valid = TRUE;
		    *length	 = decode.number;
		}
	    }
	    else
		ok = FALSE;
	    break;
	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( !address_valid )
        ok = FALSE;

    return ok ? OK : error;
}


/************************************************************************
 *                          do_dump
 ************************************************************************/
static void
do_dump( void )
{
    UINT32  val, units_per_line;
    UINT32  i,t;
    UINT32  char_count;
    char    ch;
    char    line[8 + DUMP_BYTES_PER_LINE * 3 + 1 ];
    char    val_string[sizeof(UINT32) * 2 + 1];
    char    ascii[DUMP_BYTES_PER_LINE + 2];

    units_per_line = DUMP_BYTES_PER_LINE/width;

    int2str( line, address, sizeof(UINT32) );
    strcat( line, ":" );
    
    char_count = 0;

    if(SHELL_PUTS( "\n" )) return;

    for(i=0; i< (length+width-1)/width; i++)
    {	
	switch( width )
	{
	  case sizeof(UINT8) :
	    val = REG8(address);
	    break;
	  case sizeof(UINT16) :
	    val = REG16(address);
	    break;
	  case sizeof(UINT32) :
	    val = REG32(address);
	    break;
	}

	address += width;
	    	    
	int2str( val_string, val, width );

        strcat( line, " " );
  	strcat( line, val_string );   
	    	
	for( t=0; t<width; t++ )
	{
	   ch = val & 0xFF;
#ifdef EL
	   ascii[char_count++]
#else
	   ascii[((char_count++) & ~(width-1)) + (width-1) - t] 
#endif
			= isgraph(ch) ? ch : '.';
	   val >>= 8;
	}

	if( (((i+1) % units_per_line) == 0) || (i+1 == length/width) )
	{
	    ascii[char_count]   = '\n';
	    ascii[char_count+1] = '\0';

	    if( !more )
	        SHELL_DISABLE_MORE;

	    if(SHELL_PUTS( line )) 
	    {
	        SHELL_PUTC( '\n' );
		return;
	    }
		
	    if(SHELL_PUTS_INDENT( ascii, width == 4 ? 47 :
					 width == 2 ? 51 : 59 )) return;

	    int2str( line, address, sizeof(UINT32) );
	    strcat( line, ":" );

	    char_count = 0;
	} 
    }

    SHELL_PUTC( '\n' );
}


/************************************************************************
 *                          int2str
 ************************************************************************/
static void
int2str( 
    char  *val_string,
    UINT32 val,
    UINT32 width )
{
    switch( width )
    {
      case sizeof(UINT8) :
        sprintf( val_string, "%02X", val );
	break;
      case sizeof(UINT16) :
        sprintf( val_string, "%04X", val );
	break;
      case sizeof(UINT32) :
        sprintf( val_string, "%08X", val );
	break;
    }
}


/* Command definition for dump */
static t_cmd cmd_def =
{
    "dump",
    dump,
    "dump [-m][-8|-16|-32] <address> [<size>]",

    "Dumps data from address range starting at <address>.\n"
    "The default data display width is 8 bits. The <size> parameter\n"
    "specifies the number of bytes to dump (default is 256).\n"
    "\n"
    "The continuation command '.' works together with 'dump'.",

    options,
    OPTION_COUNT,
    FALSE
};

/* Command definitions for SDB 'd' command (secret command) */

static t_cmd cmd_def_sdb_lower =
{
    "d",
    dump_sdb,
    "d <address>              (Microsoft SDB command)",
    "Dumps 8 32 bit words starting from 'address'.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_dump_init
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
shell_dump_init( void )
{
    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_dump_sdb_init
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
shell_dump_sdb_init( void )
{ 
    return &cmd_def_sdb_lower;
}
