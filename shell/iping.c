
/************************************************************************
 *
 *  iping.c
 *
 *  Shell ip ping command
 *
 *  ping ipaddr [<datagramsize>]
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
#include <shell_api.h>
#include <env_api.h>
#include <net_api.h>
#include <sysdev.h>
#include <io_api.h>
#include <ctype.h>
#include <stdio.h>
#include <sysenv_api.h>
#include <sys_api.h>
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

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *ipaddr,
    UINT32 *size );

static UINT32
do_ping(
    UINT32 ipaddr,
    UINT32 size );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          ping
 ************************************************************************/
static MON_FUNC(ping)
{
    UINT32 ipaddr;
    UINT32 size;
    UINT32 rc;
    
    rc = get_options( argc, argv, &ipaddr, &size );

    if( rc == OK )
        rc = do_ping( ipaddr, size );

    return rc;
}


static UINT32
do_ping(
    UINT32 ipaddr,
    UINT32 size )
{
    return( NET_ping( ipaddr, size ) ) ;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT32 *ipaddr,
    UINT32 *size )
{
    t_shell_option decode;
    UINT32         type;
    bool	   ok    = FALSE ;
    UINT32	   error = SHELL_ERROR_SYNTAX;
    UINT32	   i;

    /* Check for options (not allowed) */
    for( i=0; i<argc; i++ )
    {
        if( *argv[i] == '-' )
	{
	    shell_error_data = argv[i];
	    return SHELL_ERROR_OPTION;
        }
    }

    if( argc == 2 )
    {
        if( env_ip_s2num( argv[1], ipaddr, sizeof(UINT32) ) )
        {
            *size = 64 ; /* set default size to 64 bytes */
            ok = TRUE ;
        }
    }

    if( argc == 3 )
    {
        if( env_ip_s2num( argv[1], ipaddr, sizeof(UINT32) ) )
        {
            shell_decode_token( argv[2], &type, &decode );
            if ( type == SHELL_TOKEN_NUMBER )
            {
                *size = decode.number;
                ok = TRUE ;
            }
        }
    }

    return ok ? OK : error;
}

/* Command definition for help */
static t_cmd cmd_def =
{
    "ping",
     ping,
    "ping ipaddr [<datagramsize>]",

    "ping - send ICMP ECHO_REQUEST packets to network host.\n"
    "\n"
    "This command is typically used to verify end-to-end network\n"
    "functionality & connectivity in a debug or bring-up situation.\n"
    "\n"
    "An ICMP ECHO_REQUEST packet must be replied to with an ICMP\n"
    "ECHO_REPLY packet from the remote host ('ipaddr'). The ICMP ECHO\n"
    "packet will contain data with the specified size. The default\n"
    "datagramsize is 64 bytes, minimum is 0 bytes and maximum is 1472\n"
    "bytes. The maximum size is constrained by the Ethernet upper\n"
    "frame size limit (IP segmentation is not supported). If the\n"
    "optional datagramsize parameter is not within the valid range,\n"
    "the default size of 64 bytes will be used.\n"
    "\n"
    "The ping command will stop when the first reply is received\n"
    "from the remote host. If no replies are received, depending on\n"
    "whether the MAC-address of the destination path has been resoluted\n"
    "and kept in a cache, ARP or ICMP_ECHO REQUEST packets are\n"
    "retransmitted up to 3 times before an appropriate error message\n"
    "is finally returned. The user may stop the ping command at any time\n"
    "using Ctrl-C.",

    NULL,
    0,
    FALSE
};

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_ping_init
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
shell_ping_init( void )
{
    return &cmd_def;
}
