/************************************************************************
 *
 *  eeprom.c
 *
 *  Monitor command for reading from eeprom device.
 *
 *  eeprom [-m] <dev> <address> [<size>]
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
#include <io_api.h>
#include <sysdev.h>
#include <eeprom_api.h>
#include <syscon_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define DEFAULT_LENGTH   256
#define BYTES_PER_LINE	 16

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Command description */
static char cmd_descr[300];

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_MORE	0
  { "m",  "Prompt user for keypress after each screen of data" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/* MINOR deviced IDs for EEPROM devices */
static t_syscon_eeprom_def  *eeprom_dev;
static UINT8	            dev_count;	

static UINT32 dev, address, length;
static bool   more;
static UINT32 rc = OK;
    

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv );

static UINT32
do_eeprom_dump( void );

static void
int2str( 
    char  *val_string,
    UINT32 val,
    UINT32 width );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          eeprom
 ************************************************************************/
static MON_FUNC(eeprom)
{
    if( !shell_dot )
        rc = get_options( argc, argv );

    if( rc != OK )
        return rc;
    else
    {
    	if(!length)
	    return OK;

	return do_eeprom_dump();
    }
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv )
{
    t_shell_option decode;
    UINT32	   type;

    bool	   address_valid = FALSE;
    bool	   length_valid  = FALSE;
    bool	   dev_valid	 = FALSE;
    bool	   ok		 = TRUE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    more   = FALSE;
    length = DEFAULT_LENGTH;

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
		    strcmp(decode.option, options[i].option);
		i++) ;

	    switch(i)
	    {
	      case OPTION_MORE :
		more  = TRUE; 
		break;
	      default :
		error	         = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( !address_valid )
	    {
		address_valid = TRUE;
		address       = decode.number;
	    }
	    else if( !length_valid )
	    {
	        length_valid = TRUE;
		length	     = decode.number;
	    }
	    else
		ok = FALSE;
	    break;
	  case SHELL_TOKEN_STRING :
	    /* Find match */
	    for( i=0; 
	             (i<dev_count) &&
		     strcmp(decode.string,eeprom_dev[i].name);
		 i++ ) ;

	    if( i == dev_count )
	        ok = FALSE;
	    else
	    {
	        dev_valid = TRUE;
	        dev       = eeprom_dev[i].minor;
	    }
	    break;
	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( !address_valid || !dev_valid )
        ok = FALSE;

    return ok ? OK : error;
}


/************************************************************************
 *                          do_eeprom_dump
 ************************************************************************/
static UINT32
do_eeprom_dump( void )
{
    UINT32		     i;
    char		     line[8 + BYTES_PER_LINE * 3 + 1 ];
    char		     val_string[sizeof(UINT32) * 2 + 1];
    char		     ascii[BYTES_PER_LINE + 2];
    UINT32		     char_count = 0;
    UINT32		     rc;
    UINT8		     ch;
    t_EEPROM_read_descriptor readbuf;
    
    if( !more )
        SHELL_DISABLE_MORE;

    int2str( line, address, sizeof(UINT32) );
    strcat( line, ":" );
    
    readbuf.length = 1;
    readbuf.buffer = &ch;

    for(i=0; i<length; i++)
    {	
        readbuf.offset = address;

        rc = IO_read( SYS_MAJOR_EEPROM_IIC, dev, &readbuf );

        if( rc != OK )
            return rc;

	if( i==0 )
        if(SHELL_PUTC( '\n' )) return SHELL_ERROR_CONTROL_C_DETECTED;

	address++;
	    	    
	int2str( val_string, ch, sizeof(UINT8) );
        strcat(  line, " " );
  	strcat(  line, val_string );   

        ascii[char_count++] = isgraph(ch) ? ch : '.';

	if( (((i+1) % BYTES_PER_LINE) == 0) || ((i+1) == length) )
	{
	    ascii[char_count]   = '\n';
	    ascii[char_count+1] = '\0';

	    if(SHELL_PUTS( line )) 
	    {
	        SHELL_PUTC( '\n' );
		return SHELL_ERROR_CONTROL_C_DETECTED;
	    }
		
	    if(SHELL_PUTS_INDENT( ascii, 59 ))
	        return SHELL_ERROR_CONTROL_C_DETECTED;

	    int2str( line, address, sizeof(UINT32) );
	    strcat( line, ":" );

	    char_count = 0;
	} 
    }

    SHELL_PUTC( '\n' );

    return OK;
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
      case sizeof(UINT32) :
        sprintf( val_string, "%08X", val );
	break;
    }
}


/* Command definition for eeprom */
static t_cmd cmd_def =
{
    "eeprom",
    eeprom,

    "eeprom [-m] <dev> <address> [<size>]",

    cmd_descr,

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell_eeprom_init
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
shell_eeprom_init( void )
{
    UINT32 i;

    SYSCON_read(
        SYSCON_BOARD_EEPROM_COUNT_ID,
	(void *)&dev_count,
	sizeof(dev_count) );
    
    SYSCON_read(
	SYSCON_BOARD_EEPROM_DEF_ID,
	(void *)&eeprom_dev,
	sizeof(t_syscon_eeprom_def *) );

    strcpy( cmd_descr, 
	      "Dumps 'size' bytes "
	      "(default " DEF2STR(DEFAULT_LENGTH) ") "
	      "from eeprom device 'dev'\n"
	      "starting at 'address'.\n"
	      "\n"
	      "The continuation command '.' works together with 'eeprom'.\n"
	      "\n"
	      "Available settings of 'dev' parameter are :"
	      "\n" );
	     
    for( i=0; i<dev_count; i++ )
    {
        sprintf( &cmd_descr[strlen(cmd_descr)],
		 "\n%-4s : %s.",
		 eeprom_dev[i].name,
		 eeprom_dev[i].descr );
    }

    return &cmd_def;
}


