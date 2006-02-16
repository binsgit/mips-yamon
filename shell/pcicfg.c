
/************************************************************************
 *
 *  pcicfg.c
 *
 *  Shell pci configuration space read/write command (one 32 bit word)
 *
 *  pcicfg [-8|-16|-32] ([-r] <bus> <dev> <func> <addr> [<range>]) |
 *                      (-w   <bus> <dev> <func> <addr> <val>)
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
#include <syserror.h>
#include <sys_api.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <pci_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static MON_FUNC(pcicfg);

static UINT32 
get_options(
    UINT32  argc,
    char    **argv,
    UINT32  *action,
#define ACTION_READ	0x00
#define ACTION_WRITE	0x01
    UINT32  *width,
    UINT32  *bus,
    UINT32  *dev,
    UINT32  *func,
    UINT32  *addr,
    UINT32  *value);

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_READ 	0
  { "r",     "Read PCI configuration space" },
#define OPTION_WRITE	1
  { "w",     "Write to PCI configuration space" },
#define OPTION_W8	2
  { "8",  "See data in units of bytes" },
#define OPTION_W16	3
  { "16", "See data in units of halfwords" },
#define OPTION_W32	4
  { "32", "See data in units of words" },
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))


/* Command definition for help */
static t_cmd cmd_def =
{
    "pcicfg",
     pcicfg,
     "pcicfg [-8|-16|-32] ([-r] <bus> <dev> <func> <addr> [<range>]) |\n"
     "                    (-w   <bus> <dev> <func> <addr> <val>)",

    "Read a value/range or write a value to PCI configuration space.\n"
    "All arguments are hexadecimal.\n"
    "Range parameter indicates the number of bytes to read.\n"
    "Default action is read.\n"
    "Default width is 32 bit.\n",

    options,
    OPTION_COUNT,
    FALSE
};

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          pcicfg
 ************************************************************************/
static MON_FUNC(pcicfg)
{
    /* Options */
    UINT32  action, width, bus, dev, func, addr, value, limit;
    UINT32  i;
    UINT32  rc;

    rc = get_options( argc, argv, &action, &width, &bus, &dev, &func, &addr, &value);

    if( rc == OK )
    {
        if(action == ACTION_READ)
        {
            limit = addr + value;

	    if( limit > 0x100 )
	        return SHELL_ERROR_VAR_VALUE;

            printf("Read PCI (bus 0x%02x, dev 0x%02x, func %d)",
			bus, dev, func);

            for(i=0; addr < limit; addr+= width, i+= width)
	    {
		switch( width )
		{
		  case sizeof(UINT8) :
		    rc = pci_config_read8(bus, dev, func, addr, (UINT8 *)&value);
		    break;
		  case sizeof(UINT16) :
		    rc = pci_config_read16(bus, dev, func, addr, (UINT16 *)&value );
		    break;
		  case sizeof(UINT32) :
		    rc = pci_config_read32(bus, dev, func, addr, &value);		    
		    break;
		  default : /* Should not happen */
		    break;
		}

		if( rc != OK )
		{
		    printf( "\n" );
		    return rc;
		}

                if( (i % 8) == 0 ) 
		    printf("\n addr 0x%02X :", addr);

		switch( width )
		{
		  case sizeof(UINT8) :
		    printf(" 0x%02X", *(UINT8 *)(&value) );
		    break;
		  case sizeof(UINT16) :
		    printf(" 0x%04X", *(UINT16 *)(&value) );
		    break;
		  case sizeof(UINT32) :
		    printf(" 0x%08X", value );
		    break;
		  default : /* Should not happen */
		    break;
		}
            }

	    printf( "\n" );
	}

	if(action == ACTION_WRITE)
	{
            printf("Write PCI (bus 0x%02x, dev 0x%02x, func %d, addr 0x%02x)\n"
		   "data = ",
		   bus, dev, func, addr);

	    switch( width )
	    {
	      case sizeof(UINT8) :
                printf("0x%02X\n", (UINT8)(value) );
	        rc = pci_config_write8(bus, dev, func, addr, (UINT8)(value) );
		break;
	      case sizeof(UINT16) :
                printf("0x%04X\n", (UINT16)(value) );
	        rc = pci_config_write16(bus, dev, func, addr, (UINT16)(value) );
		break;
	      case sizeof(UINT32) :
                printf("0x%08X\n", value);
	        rc = pci_config_write32(bus, dev, func, addr, value);
		break;
   	      default : /* Should not happen */
		break;
	    }
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
    UINT32 *action,
    UINT32 *width,
    UINT32 *bus,
    UINT32 *dev,
    UINT32 *func,
    UINT32 *addr,
    UINT32 *value )  /* value or range */
{
    t_shell_option decode;
    UINT32	   type;

    bool   ok         = TRUE;
    int    error      = SHELL_ERROR_SYNTAX;
    int    arg_count  = 0;
    int    i;
    UINT32 arg;

    /* Setup defaults */
    *action = ACTION_READ;
    *width  = 4;
    *value  = 1;

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
	      case OPTION_READ :
		*action = ACTION_READ;
		break;
	      case OPTION_WRITE:
		*action = ACTION_WRITE;
		break;
	      case OPTION_W8:
		*width = sizeof(UINT8);
		break;
	      case OPTION_W16:
		*width = sizeof(UINT16);
		break;
	      case OPTION_W32:
		*width = sizeof(UINT32);
		break;
	      default :
		error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
            switch (arg_count)
            {
              case 0:
                if(decode.number > 0xfe)
		{
	            error = SHELL_ERROR_VAR_VALUE;
		    ok    = FALSE;
		}
		else
		    *bus = decode.number;
		break;
              case 1:
                if(decode.number > 0x1f)
		{
	            error = SHELL_ERROR_VAR_VALUE;
		    ok    = FALSE;
		}
		else
		    *dev = decode.number;
		break;
              case 2:
                if(decode.number > 0x07)
		{
	            error = SHELL_ERROR_VAR_VALUE;
		    ok    = FALSE;
		}
		else
		    *func = decode.number;
		break;
              case 3:
                if(decode.number > 0xff)
		{
	            error = SHELL_ERROR_VAR_VALUE;
		    ok    = FALSE;
		}
		else
		    *addr = decode.number;
		break;
              case 4:
		*value = decode.number;
		break; 
              default:
		ok = FALSE;
                break;
 	    }

	    arg_count++;
	    break;
	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( ( arg_count > 5 )				 ||
        ( (*action == ACTION_READ)  && (arg_count < 4) ) ||
        ( (*action == ACTION_WRITE) && (arg_count < 5) ) )
    {
        ok = FALSE;
    }

    if( ok && !sys_legal_align( *addr, *width ) )
    {
	if(error != SHELL_ERROR_OPTION ) error = SHELL_ERROR_ALIGN ;
        ok = FALSE;
    }

    if( ok && (*action == ACTION_WRITE) && *value > MAXUINT(*width) )
    {
	error = SHELL_ERROR_OVERFLOW;
	ok = FALSE;
    }

    return ok ? OK : error;
}



/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_pcicfg_init
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
shell_pcicfg_init( void )
{
    return &cmd_def;
}
