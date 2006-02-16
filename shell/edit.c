
/************************************************************************
 *
 *  edit.c
 *
 *  Monitor command for editing memory (or memory mapped registers).
 *
 *  edit [-8|-16|-32] <address>
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <sys_api.h>
#include <errno.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define EDIT_DEFAULT_WIDTH      sizeof(UINT8)

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_W8	0
  { "8",  "Edit data in units of bytes" },
#define OPTION_W16	1
  { "16", "Edit data in units of halfwords" },
#define OPTION_W32	2
  { "32", "Edit data in units of words" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *width,
    UINT32 *address);

bool
get_value(
    UINT32 width,  
    UINT32 *value );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          edit
 ************************************************************************/
static MON_FUNC(edit)
{
    UINT32 addr;
    UINT32 width;
    UINT32 rc;        
    UINT32 value;
    char   line[ 2*sizeof(UINT32) + 1];
    UINT32 len;
    char   *endp;
    bool   ctrlc;
 
    rc = get_options( argc, argv, &width, &addr );

    if( rc != OK )
        return rc;

    for (;;) 
    {
        rc = sys_validate_range( addr, width, width, TRUE );

	if( rc != OK )
	    return rc;

	switch (width) 
	{
	  case sizeof(UINT8):
	    printf("0x%08X:  (%02X)  ", addr, REG8(addr)); 
	    break;
	  case sizeof(UINT16):
	    printf("0x%08X:  (%04X)  ", addr, REG16(addr)); 
	    break;
	  case sizeof(UINT32):
	    printf("0x%08X:  (%08X)  ", addr, REG32(addr));
	    break;
	  default:  /* Should not happen */
	    return SHELL_ERROR_STRUCTURE;
	}

        ctrlc = !shell_get_line( line,
				 width * 2,
				 TRUE,
				 FALSE,
				 NULL );

        printf( "\n" );

	if( ctrlc )
	    return SHELL_ERROR_CONTROL_C_DETECTED;

	len = strlen( line );

	if( len != 0 )
	{
	    if( (len == 1) && (line[0] == '.') )	
	    {
	        /* '.' typed */
	        return OK;
	    }

	    if( (len ==1) && (line[0] == '-') )	
	    {
	        /* '-' typed */
		addr -= 2 * width;
	    }
	    else
	    {
                errno = 0;
                value = (UINT32)strtoul( line, &endp, 16 );

                if( (*endp == '\0') && (errno == 0) )
	        {
                    switch (width) 
                    {
                      case sizeof(UINT8)  : 
	                REG8(addr)  = (UINT8)value;
	                break;
                      case sizeof(UINT16) : 
	                REG16(addr) = (UINT16)value;
	                break;
                      case sizeof(UINT32) : 
	                REG32(addr) = value;
	                break;
                      default : /* Should not happen */
	                return SHELL_ERROR_STRUCTURE;
                    }
	        }
	        else
	        {
	            printf( "Illegal number\n" );
		    addr -= width;
	        }
	    }
        }

	addr += width;
    }
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *width,
    UINT32 *address)
{
    t_shell_option decode;
    UINT32	   type;
    bool	   ok		 = TRUE;
    bool	   address_valid = FALSE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    *width = EDIT_DEFAULT_WIDTH;

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

	    switch(i)
	    {	
	      case OPTION_W8   :
		*width = sizeof(UINT8);  break;
	      case OPTION_W16  :
		*width = sizeof(UINT16); break;
	      case OPTION_W32  :
		*width = sizeof(UINT32); break;
	      default :
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if(!address_valid)
	    {
		address_valid = TRUE;
		*address      = decode.number;
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
 *                          cmd definition
 ************************************************************************/

/* Command definition for edit */
static t_cmd cmd_def =
{
    "edit",
    edit,
    "edit [-8|-16|-32] <address>",

    "Edit memory contents starting at <address>. The default data width\n"
    "is 8 bits. Edit mode is exited by typing '.' or Ctrl-C.\n"
    "\n"
    "During the edit, a data element can be left untouched by just\n"
    "pushing Enter. The edit will then continue with the next data element.\n"
    "\n"
    "Typing '-' will cause the address to be decremented.\n"
    "\n"
    "Data is entered using hexadecimal number format (with no leading \"0x\").",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_edit_init
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
shell_edit_init( void )
{
    return &cmd_def;
}
