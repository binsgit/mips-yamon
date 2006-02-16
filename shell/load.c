
/************************************************************************
 *
 *  load.c
 *
 *  Shell load command
 * 
 *  load [-r]
 *       ([tftp:][//<ipaddr>][/<filename>]) |
 *       ([asc:] [//(tty0|tty1)])
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
#include <loader_api.h>
#include <shell_api.h>
#include <env_api.h>
#include <net_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define MSG_LOAD "Start = 0x%08x, range = (0x%08x,0x%08x), format = %s"

/* State event machine states */
#define ST_START		0
#define ST_GET_PROTOCOL		1
#define ST_GOT_PROTOCOL		2
#define ST_GET_SRC_OR_NAME	3
#define ST_GET_SRC		4
#define ST_GET_NAME		5
#define ST_DONE			6
#define ST_ERROR		7

/************************************************************************
 *  Public variables
 ************************************************************************/

void *shell_addr_go;
bool shell_addr_go_valid = FALSE;

/************************************************************************
 *  Static variables
 ************************************************************************/

/* load, min, max address */
static     void*  load_adr_context[3] ;

/* OPTIONS */
static t_cmd_option options[] =
{ 
  { "r",  "Retry on ARP timeout (until load succeeds or Ctrl-c is typed)" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

static char arg_copy[SHELL_MAX_TOKEN_LEN+1];

static char *msg_hint_filename = "Filename must be preceeded with '/'";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   sdb,
    char   **s,
    bool   *retry );

static UINT32
parse(
    char   *s,
    UINT32 *protocol,
    UINT32 *src,
    char   **filename );

static UINT32
do_load(
    UINT32 port,
    UINT32 ip,
    char   *filename,
    bool   retry );

static void
load_message(
    UINT32 protocol,
    UINT32 src,
    char   *filename );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          load
 ************************************************************************/
static MON_FUNC(load)
{
    /* Options */
    char   *s;
    UINT32 protocol;
    UINT32 src;
    char   *filename;
    bool   retry;
    UINT32 rc;

    SHELL_DISABLE_MORE;
    
    rc = get_options( argc, argv, FALSE, &s, &retry );

    if( rc != OK )
        return rc;

    rc = parse( s, &protocol, &src, &filename );
    
    if( rc != OK )
        return rc;

    /* All OK */
    load_message( protocol, src, filename );

    rc = do_load( (protocol == PROTOCOL_TFTP) ?
		   PORT_NET : src,
		   src, filename, retry );

    printf( "\n" );

    return rc;
}


/************************************************************************
 *                          load_sdb
 ************************************************************************/
static MON_FUNC(load_sdb)
{
    UINT32 rc;

    rc = get_options( argc, argv, TRUE, NULL, NULL );

    if( rc != OK )
        return rc;

    load_message( PROTOCOL_ASC, DEFAULT_PORT, NULL );

    rc = do_load( DEFAULT_PORT, 0, NULL, FALSE );

    printf( "\n" );
    return rc;
}



/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    bool   sdb,
    char   **s,
    bool   *retry )
{
    UINT32 i;

    if( !sdb )
    {
        *s     = NULL;
        *retry = FALSE;
    }

    for( i=1; i<argc; i++ )
    {
        if( *(argv[i]) == '-' )
	{
	    if( !sdb && (strcmp( argv[i], "-r" ) == 0) )
	    { 
	        *retry = TRUE;
	    }
	    else
	    {
		shell_error_data = argv[i];
		return SHELL_ERROR_OPTION;
	    }
	}
	else
	{
	    if( !sdb && !(*s) )
	    {
	        *s = arg_copy;
		strcpy( *s, argv[i] );
	    }
	    else
	        return SHELL_ERROR_SYNTAX;
	}
    }

    if( !sdb && !(*s) )
        *s = "";

    return OK;
}


/************************************************************************
 *                          load_message
 ************************************************************************/
static void
load_message(
    UINT32 protocol,
    UINT32 src,
    char   *filename )
{
    /* Protocol */
    printf( "About to load %s://", (protocol == PROTOCOL_TFTP) ?
				       "tftp" : "asc" );

    if( protocol == PROTOCOL_TFTP )
    {
        /* IP address */
        printf( "%d.%d.%d.%d/%s\n", 
                    (UINT32)(CPU_TO_BE32(src) >> 24),
                    (UINT32)(CPU_TO_BE32(src) >> 16) & 0xFF,
                    (UINT32)(CPU_TO_BE32(src) >> 8)  & 0xFF,
                    (UINT32)CPU_TO_BE32(src)         & 0xFF,
		    filename );
    }
    else
    {
        printf( (src == PORT_TTY0) ? "tty0\n" : "tty1\n" );
    }
}


/************************************************************************
 *                          do_load
 ************************************************************************/
static UINT32
do_load(
    UINT32 port,
    UINT32 ip,
    char   *filename,
    bool   retry )
{
    UINT32	   error_pos;
    UINT32         raw_error;
    t_image_format format;
    bool	   first_error = TRUE;
    UINT32	   rc;

    if(SHELL_PUTS( shell_msg_ctrl_c )) return OK;

    if(port != PORT_NET)
    {
	if(SHELL_PUTS( "\nStart dump from terminal program\n" )) 
	    return OK;	
    }

    do
    {    
        if( GETCHAR_CTRLC( DEFAULT_PORT ) )
	    return SHELL_ERROR_CONTROL_C_DETECTED;

	/* Previous load address is no longer valid */
	shell_addr_go_valid = FALSE;

        rc = loader_image(
	         port,
	         ip,
	         filename,
	         load_adr_context,
	         &format,
                 &error_pos,
                 &raw_error );

        switch( rc )
        {
          case OK :
            /* Get new load address */
            shell_addr_go       = load_adr_context[0];
	    shell_addr_go_valid = TRUE;
	    break;
          case ERROR_LOAD_BREAK :
	    return SHELL_ERROR_CONTROL_C_DETECTED;
          default :
            if( !retry )
            {
	        return raw_error;
            }

            if( raw_error != ERROR_NET_ARP_TIME_OUT )
            {
	        return raw_error;
            }
	    else
	    {
	        if( first_error )
		{
                    printf( "\n" );
		    first_error = FALSE;
		}

	        shell_command_error( NULL, raw_error );
	    }

    	    break;
        }

	if( rc != OK )
	{
	    /* Pause 1 sec */
	    sys_wait_ms( 1000 );
	}
    }
    while( rc != OK );

    /* print load status message */
    printf(MSG_LOAD, (UINT32)load_adr_context[0],
                     (UINT32)load_adr_context[1],
                     (UINT32)load_adr_context[2],
                     (format == MOTOROLA_S3 ? "SREC" :
                     (format == MICROSOFT_BIN ? "WINCEBIN" :
	              "UNKNOWN") ) );

    return OK;
}


/************************************************************************
 *                          parse
 ************************************************************************/
static UINT32
parse(
    char   *s,
    UINT32 *protocol,
    UINT32 *src,
    char   **filename )
{
    UINT32 state;
    char   *proto_start;
    char   *src_start;
    char   *name_start;

    proto_start = NULL;
    src_start   = NULL;
    name_start  = NULL;

    state = ST_START;

    while( (state != ST_DONE) && (state != ST_ERROR) )
    {
        switch( state )
	{
	  case ST_START :
	    switch( *s )
	    {
	      case ':' :
		shell_error_hint = msg_hint_filename;
		state = ST_ERROR;
		break;
	      case '/' :
		state = ST_GET_SRC_OR_NAME;
		break;
	      case ' ' :
		state = ST_ERROR;
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
		shell_error_hint = msg_hint_filename;
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
	      case ':' :
	        state = ST_ERROR;
	        break;
	      case '/' :
	        state = ST_GET_SRC_OR_NAME;
	        break;
	      case ' ' :
	        state = ST_ERROR;
	        break;
	      case '\0' :
	        state = ST_DONE;
		break;
	      default :
	        state = ST_ERROR;
	        break;
	    }
	    break;
	  case ST_GET_SRC_OR_NAME :
	    switch( *s )
	    {
	      case '/' :
	        src_start = s + 1;
	        state     = ST_GET_SRC;
	        break;
	      case ' ' :
	        state = ST_ERROR;
	        break;
	      case '\0' :
	        state = ST_ERROR;
		break;
	      case ':' :
	      default  :
	        name_start = s;
	        state      = ST_GET_NAME;
	        break;
	    }
	    break;

	  case ST_GET_SRC :
	    switch( *s )
	    {
	      case ':' :
	        state = ST_ERROR;
	        break;
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
	        state = ST_GET_SRC;
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
	      case ':' :
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
        /* Get protocol */
        if( !proto_start )
	{
            if( !env_get( "bootprot",
		          &proto_start, NULL, 0 ) )
	    {
	        /* Only fails on platforms not supporting Ethernet */
	        proto_start = "asc";
	    }
	}

	if( !env_decode_bootprot(
	        proto_start,
		protocol,
		sizeof(UINT32) ) )
        {
	    shell_error_hint = msg_hint_filename;
	    return SHELL_ERROR_BOOTPROT;
        }

	/* Get source */
	if( !src_start )
	{
	    if( !env_get( (*protocol == PROTOCOL_ASC) ?
		         "bootserport" : "bootserver",
		    &src_start, NULL, 0 ) )
	    {
	        src_start = "";
	    }
        }

        if( *protocol == PROTOCOL_ASC )
        {
	    if( !env_decode_bootserport(
	            src_start,
		    src,
		    sizeof(UINT32) ) )
            {
	        return SHELL_ERROR_PORT;
            }
	}
	else
	{
	    /* Must be IP address */
	    if( !env_ip_s2num( src_start, src, sizeof(UINT32) ) )
	        return SHELL_ERROR_IP;
	}

	/* Filename */
        if( *protocol == PROTOCOL_TFTP )
	{
	    if( name_start )
		*filename = name_start;
	    else
	    {
	        if( !env_get( "bootfile", filename, NULL, 0 ) )
		    *filename = "";
	    }

	    if( strspn( *filename, " " ) == strlen( *filename ) )
	    {
		/* All whitespace filename */
		return SHELL_ERROR_FILENAME;
	    }
	}
	else
	{
  	    if( name_start )
	        return SHELL_ERROR_SYNTAX;
        }

        return OK;
    }
    else
        return SHELL_ERROR_SYNTAX;
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "load",
     load,

    "load [-r]\n"
    "     ([tftp:][//<ipaddr>][/<filename>]) |\n"
    "     ([asc:] [//(tty0|tty1)])",

    "Load image from serial port or Ethernet to RAM or flash (depending\n"
    "on address). The only image type currently supported is SREC.\n"
    "\n"
    "On platforms supporting both Ethernet and serial port, the default\n"
    "protocol is taken from the environment variable 'bootprot'. On\n"
    "platforms without Ethernet, the only (and default) protocol is 'asc'.\n"
    "\n"
    "If loading from serial port, the default port is taken from the\n"
    "environment variable 'bootserport'.\n"
    "\n"
    "If loading from Ethernet, the IP address of the TFTP server and\n"
    "the filename may be specified. If an IP address is not specified,\n"
    "it is taken from the environment variable 'bootserver'. If a\n"
    "filename is not specified, it is taken from the environment\n"
    "variable 'bootfile'.\n"
    "Note that the exact limitation on the filesize in the TFTP protocol\n"
    "is 33553919 bytes (appr. 32 Mbytes). Any file larger than this size\n"
    "cannot be transferred.\n"
    "\n"
    "For the currently supported formats, the execution entrypoint of the\n"
    "image is embedded in the image. This address is saved such that the\n"
    "'go' command can use it as the default entrypoint.\n"
    "\n"
    "During the load operation, the current load address will be shown on\n"
    "the 8-position hex display (if present).\n"
    "\n"
    "Note that the load command prevents the user from overwriting the\n"
    "environment flash area.",

    options,
    OPTION_COUNT,

    FALSE
};

/* Command definitions for SDB 'l' command (secret command) */
static t_cmd cmd_def_sdb_lower =
{
    "l",
    load_sdb,
    "l                        (Microsoft SDB command)",
    "Load image from serial port (console tty0) to RAM or Flash (depending\n"
    "on destination addresses).\n"
    "\n"
    "The only image type currently supported is SREC.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_load_init
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
shell_load_init( void )
{
    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_load_sdb_init
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
shell_load_sdb_init( void )
{
    return &cmd_def_sdb_lower;
}
