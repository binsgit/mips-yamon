
/************************************************************************
 *
 *  freadwrite.c
 *
 *  Shell fread and fwrite commands
 * 
 *  fread  tftp://<ipaddr>/<filename> <address>
 *  fwrite tftp://<ipaddr>/<filename> <address> <size>
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
#include <shell_api.h>
#include <env_api.h>
#include <net_api.h>
#include <sys_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* State event machine states */
#define ST_START		0
#define ST_GET_PROTOCOL		1
#define ST_GOT_PROTOCOL		2
#define ST_GET_IP_OR_NAME	3
#define ST_GET_IP		4
#define ST_GET_NAME		5
#define ST_DONE			6
#define ST_ERROR		7

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char arg_copy[SHELL_MAX_TOKEN_LEN+1];

static char *msg_hint = "Filename must be preceeded with 'tftp://.../'";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    char   **s,
    UINT32 *address,
    UINT32 *length );

static UINT32
parse_name(
    char   *s,
    UINT32 *protocol,
    UINT32 *ip,
    char   **filename );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          fread_yamon
 ************************************************************************/
static MON_FUNC(fread_yamon)
{
    /* Options */
    char   *s;
    UINT32 protocol, addr, size, ip;
    char   *filename;
    UINT32 rc;

    rc = get_options( argc, argv, &s, &addr, NULL );
    if( rc != OK )
        return rc;

    rc = parse_name( s, &protocol, &ip, &filename );
    if( rc != OK )
        return rc;

    size = TFTP_FILE_MAX_SIZE;

    printf( "About to binary read tftp://%d.%d.%d.%d/%s\n",  /* protocol */
                (UINT32)(CPU_TO_BE32(ip) >> 24),           /* IP address */
                (UINT32)(CPU_TO_BE32(ip) >> 16) & 0xFF,
                (UINT32)(CPU_TO_BE32(ip) >> 8)  & 0xFF,
                (UINT32)CPU_TO_BE32(ip)         & 0xFF,
	        filename );

    rc = NET_file_read( ip, filename, (UINT8 *)addr, &size );

    if (rc == OK)
	printf("Successfully transferred 0x%x (10'%u) bytes", size, size);

    printf( "\n" );

    /* flush away any ctrl/c during fwrite */
    GETCHAR_CTRLC( DEFAULT_PORT );
    return rc;
}

/************************************************************************
 *                          fwrite_yamon
 ************************************************************************/
static MON_FUNC(fwrite_yamon)
{
    /* Options */
    char   *s;
    UINT32 protocol, addr, size, ip;
    char   *filename;
    UINT32 rc;

    rc = get_options( argc, argv, &s, &addr, &size );
    if( rc != OK ) return rc;

    rc = parse_name( s, &protocol, &ip, &filename );
    if( rc != OK ) return rc;

    printf( "About to binary write tftp://%d.%d.%d.%d/%s\n",  /* protocol */
                (UINT32)(CPU_TO_BE32(ip) >> 24),            /* IP address */
                (UINT32)(CPU_TO_BE32(ip) >> 16) & 0xFF,
                (UINT32)(CPU_TO_BE32(ip) >> 8)  & 0xFF,
                (UINT32)CPU_TO_BE32(ip)         & 0xFF,
	        filename );

    rc = NET_file_write( ip, filename, (UINT8 *)addr, &size );

    if (rc == OK)
	printf("Successfully transferred 0x%x (10'%u) bytes", size, size);

    printf( "\n" );

    /* flush away any ctrl/c during fwrite */
    GETCHAR_CTRLC( DEFAULT_PORT );
    return rc;
}

/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    char   **s,
    UINT32 *address,
    UINT32 *length )
{
    bool	   action_fread  = (length == NULL);
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    bool	   address_valid = FALSE;
    bool	   length_valid  = action_fread;

    *s      = arg_copy;
    arg_copy[0] = '\0';
    
    for( arg = 1; 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    {
	        shell_error_data = argv[arg];
	        return SHELL_ERROR_OPTION;
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
		length_valid = TRUE;
		*length      = decode.number;
	    }
	    else
	        return SHELL_ERROR_SYNTAX;
            break;
	  case SHELL_TOKEN_STRING :
	    if(arg_copy[0])
	        return SHELL_ERROR_SYNTAX;
	    strcpy( arg_copy, decode.string );
	    break;
	}
    }

    if ( !(arg_copy[0] && address_valid && length_valid ))
        return SHELL_ERROR_SYNTAX;

    /* Validate address */
    return sys_validate_range( *address,
		               action_fread ? 1 : *length,
			       sizeof(UINT8),
			       action_fread );
}


/************************************************************************
 *                          parse_name
 ************************************************************************/
static UINT32
parse_name(
    char   *s,
    UINT32 *protocol,
    UINT32 *ip,
    char   **filename )
{
    UINT32 state       = ST_START;
    char   *proto_start= NULL;
    char   *ip_start   = NULL;
    char   *name_start = NULL;

    while( (state != ST_DONE) && (state != ST_ERROR) )
    {
        switch( state )
	{
	  case ST_START :
	    switch( *s )
	    {
	      case '/' :
		state = ST_GET_IP_OR_NAME;
		break;
	      case '\0' :
		state = ST_DONE;
		break;
	      default :
	        proto_start = s;
		state       = ST_GET_PROTOCOL;
		break;
	    }
	    break;
	  case ST_GET_PROTOCOL :
	    switch( *s )
	    {
	      case ':' :
	        *s    = '\0';
		state = ST_GOT_PROTOCOL;
		break;
	      case '/' :
	      case ' ' :
	      case '\0' :
		shell_error_hint = msg_hint;
		state = ST_ERROR;
		break;
	      default :
		state = ST_GET_PROTOCOL;
		break;
	    }
	    break;

	  case ST_GOT_PROTOCOL :
	    switch( *s )
	    {
	      case '/' :
	        state = ST_GET_IP_OR_NAME;
	        break;
	      case '\0' :
	        state = ST_DONE;
		break;
	      default :
	        state = ST_ERROR;
	        break;
	    }
	    break;
	  case ST_GET_IP_OR_NAME :
	    switch( *s )
	    {
	      case '/' :
	        ip_start = s + 1;
	        state    = ST_GET_IP;
	        break;
	      case ' ' :
	        state = ST_ERROR;
	        break;
	      case '\0' :
	        state = ST_ERROR;
		break;
	      default  :
	        name_start = s;
	        state      = ST_GET_NAME;
	        break;
	    }
	    break;
	  case ST_GET_IP :
	    switch( *s )
	    {
	      case '/' :
	        *s         = '\0';
		name_start = s + 1;
		state      = ST_GET_NAME;
	        break;
	      case ' ' :
	        state = ST_ERROR;
	        break;
	      case '\0' :
		state = ST_DONE;
		break;
	      default :
	        state = ST_GET_IP;
	        break;
	    }
	    break;
	  case ST_GET_NAME :
	    switch( *s )
	    {
	      case '/' :
		if( s[-1] == '/' )
		    state = ST_ERROR;
		else
	            state = ST_GET_NAME;
	        break;
	      case ' ' :
	        state = ST_ERROR;
	        break;
	      case '\0' :
		state = ST_DONE;
		break;
	      default :
	        state = ST_GET_NAME;
	        break;
	    }
	    break;
	  default :
	    break;
	}

        s++;
    }

    if( state == ST_DONE )
    {
        /* Check protocol */
        if( !proto_start ||
            strcmp( proto_start, "tftp" ) )
        {
	    shell_error_hint = msg_hint;
	    return SHELL_ERROR_BOOTPROT;
        }


	if( !ip_start ||
            !env_ip_s2num( ip_start, ip, sizeof(UINT32) ) )
	    return SHELL_ERROR_IP;

	if( !name_start ||
	    strspn( name_start, " " ) == strlen( name_start ) )
	{
	    /* All whitespace filename */
	    return SHELL_ERROR_FILENAME;
	}

	*filename = name_start;
        return OK;
    }
    else
        return SHELL_ERROR_SYNTAX;
}


/* Command definition for help */
static t_cmd cmd_def_read =
{
    "fread",
     fread_yamon,

#if 0 /* FUTURE */
    "fread  fat32://<hdxn></path/filename> <address>\n",
#endif
    "fread  tftp://<ipaddr>/<filename> <address>",
    "\n"
    "Load binary image to RAM or flash (depending on address) from TFTP server.\n"
    "\n"
    "Note that the exact limitation on the filesize in the TFTP protocol\n"
    "is 33553919 bytes (appr. 32 Mbytes). Any file larger than this size\n"
    "cannot be transferred.\n",

    NULL,
    0,

    FALSE
};


/* Command definition for help */
static t_cmd cmd_def_write =
{
    "fwrite",
     fwrite_yamon,

#if 0 /* FUTURE */
    "fwrite fat32://<hdxn></path/filename> <address> <size>\n",
#endif
    "fwrite tftp://<ipaddr>/<filename> <address> <size>",
    "\n"
    "Save binary image from RAM or flash (depending on address) to TFTP server.\n"
    "\n"
    "Note that the exact limitation on the filesize in the TFTP protocol\n"
    "is 0x1fffdff bytes (appr. 32 Mbytes). Any file larger than this size\n"
    "cannot be transferred.\n",

    NULL,
    0,

    FALSE
};

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_fread_init
 *  Description :
 *  -------------
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *  void
 ************************************************************************/
t_cmd *
shell_fread_init( void )
{
    return &cmd_def_read;
}

/************************************************************************
 *
 *                          shell_fwrite_init
 *  Description :
 *  -------------
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *  void
 ************************************************************************/
t_cmd *
shell_fwrite_init( void )
{
    return &cmd_def_write;
}

