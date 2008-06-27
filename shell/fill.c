
/************************************************************************
 *
 *  fill.c
 *
 *  Monitor command for filling memory.
 *
 *  fill [-8|-16|-32] <address> <size> <data>
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
#include <sys_api.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define FILL_DEFAULT_WIDTH      sizeof(UINT8)

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
  { "8",  "Fill data in units of bytes" },
#define OPTION_W16	1
  { "16", "Fill data in units of halfwords" },
#define OPTION_W32	2
  { "32", "Fill data in units of words" }
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
    UINT32 *address,
    UINT32 *length,
    UINT32 *data);

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          fill
 ************************************************************************/
static MON_FUNC(fill)
{
    UINT32	width;
    UINT32	addr;
    UINT32	length;
    UINT32	limit;
    UINT32	data;
    UINT16     *p16;
    UINT32     *p32;
    UINT32	rc;
        
    rc = get_options( argc, argv, &width, &addr, &length, &data );
  
    if( rc != OK )
        return rc;

    if (!length)
	return OK;

    limit = addr + length;
    switch (width) 
    {
      case sizeof(UINT8):
	data &= 0xff;
	printf("Filling from 0x%08X to 0x%08x with byte data 0x%02X.\n",
		addr, limit - 1, data);
	memset((void *) addr, data, length);
	break;

      case sizeof(UINT16):
	data &= 0xffff;
	printf("Filling from 0x%08X to 0x%08x with halfword data 0x%02X.\n",
		addr, limit - 1, data);
	for (p16 = (UINT16 *)addr;  p16 < (UINT16 *)limit;  p16++)
	{
	    *p16 = data;
	}
	break;

      case sizeof(UINT32):
	data &= 0xffffffff;
	printf("Filling from 0x%08X to 0x%08x with word data 0x%02X.\n",
		addr, limit - 1, data);
	for (p32 = (UINT32 *)addr;  p32 < (UINT32 *)limit;  p32++)
	{
	    *p32 = data;
	}
	break;

      default:
	return SHELL_ERROR_STRUCTURE;
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
    UINT32 *width,
    UINT32 *address,
    UINT32 *length,
    UINT32 *data)
{
    t_shell_option decode;
    UINT32	   type;
    bool	   ok 		 = TRUE;
    bool	   address_valid = FALSE;
    bool	   length_valid	 = FALSE;
    bool	   data_valid	 = FALSE;
    UINT32	   i;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    *width = FILL_DEFAULT_WIDTH;

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
		*width = sizeof(UINT8); break;
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
	    if (!address_valid)
	    {
		*address      = decode.number;
		address_valid = TRUE;
	    }
	    else if (!length_valid)
	    {
		*length      = decode.number;
		length_valid = TRUE;
	    }
	    else if (!data_valid)
	    {
		*data      = decode.number;
		data_valid = TRUE;
	    }
	    else
	        ok = FALSE;
	    break;

	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( !data_valid )
        ok = FALSE;

    if( ok && (*data > MAXUINT(*width)) )
    {
        ok    = FALSE;
	error = SHELL_ERROR_DATA_WIDTH;
    }

    return ok ?
        sys_validate_range( *address, *length, *width, TRUE ) :
        error;
}


/************************************************************************
 *                          cmd stuff
 ************************************************************************/

/* Command definition for fill */
static t_cmd cmd_def =
{
    "fill",
    fill,
    "fill [-8|-16|-32] <address> <size> <data>",

    "Fills the specified memory area starting at <address>. Default data\n"
    "width is 8 bits. <size> specifies the size of the area to fill\n"
    "(expressed in bytes).",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_fill_init
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
shell_fill_init( void )
{
    return &cmd_def;
}
