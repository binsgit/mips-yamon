/************************************************************************
 *
 *      net.c
 *
 *      The 'NET' module implements the common utility functions, neede
 *      by the NET-modules.
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
 *      Include files
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <sys_api.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_arp.h"
#include "net_ip.h"
#include "net_icmp.h"
#include "net_udp.h"
#include "net_tftpc.h"

/* generic device driver interface */
#include <io_api.h>

/* drivers */
#include <rtc_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/

UINT32 net_last_error ;
char   net_diag_msg[160] ;



/************************************************************************
 *      Static variables
 ************************************************************************/

static char* net_error_string[] =
{
    /* ERROR_NET_ARP_TIME_OUT               */ "ARP-timeout, no reponse on ARP-request",
    /* ERROR_NET_NO_RESOURCES               */ "Internal, no available buffers or links",
    /* ERROR_NET_INVALID_HOST_IP            */ "Invalid remote host IP-address",
    /* ERROR_NET_INVALID_BOARD_IP           */ "Invalid board IP-address",
    /* ERROR_NET_TFTP_ERROR                 */ "TFTP time out or ERROR",
    /* ERROR_NET_TFTP_READ_ERROR            */ "TFTP READ-REQ ERROR",
    /* ERROR_NET_TFTP_READ_TIMEOUT_ERROR    */ "TFTP READ-REQ timeout ERROR",
    /* ERROR_NET_TFTP_WRITE_ERROR           */ "TFTP WRITE-REQ ERROR",
    /* ERROR_NET_TFTP_WRITE_TIMEOUT_ERROR   */ "TFTP WRITE-REQ timeout ERROR",
    /* ERROR_NET_TFTP_DATA_ERROR            */ "TFTP DATA ERROR",
    /* ERROR_NET_TFTP_DATA_TIMEOUT_ERROR    */ "TFTP DATA timeout ERROR",
    /* ERROR_NET_USER_BREAK                 */ NULL,
    /* ERROR_NET_INVALID_GATEWAY_IP         */ "Gateway IP-address is not set",
    /* ERROR_NET_NOT_ON_SAME_SUBNET_IP      */ "Our IP & gateway not on same subnet",
    /* ERROR_NET_UNKNOWN_GATEWAY_IP         */ "ARP timeout on Gateway IP",
    /* ERROR_NET_TFTP_FILE_TOO_BIG          */ "TFTP Protocol Error: File bigger than 32 MB",
    /* ERROR_NET_INVALID_SUBNET_MASK        */ "Subnetmask is not set",
    /* ERROR_NET_PING_TIME_OUT              */ "No response from ping server"
} ;

static char* net_error_hint_string[] =
{
    /* ERROR_NET_ARP_TIME_OUT               */ "Check cable and IP-address of remote host",
    /* ERROR_NET_NO_RESOURCES               */ NULL,
    /* ERROR_NET_INVALID_HOST_IP            */ "Check specified remote host IP or 'bootserver' env. var",
    /* ERROR_NET_INVALID_BOARD_IP           */ "Check environment variable 'ipaddr'",
    /* ERROR_NET_TFTP_ERROR                 */ NULL,
    /* ERROR_NET_TFTP_READ_ERROR            */ "Check TFTP-server: file-existence, directory/file-attributes",
    /* ERROR_NET_TFTP_READ_TIMEOUT_ERROR    */ "Check TFTP-server is up and running",
    /* ERROR_NET_TFTP_WRITE_ERROR           */ "Check TFTP-server: file-existence, directory/file-attributes",
    /* ERROR_NET_TFTP_WRITE_TIMEOUT_ERROR   */ "Check TFTP-server is up and running",
    /* ERROR_NET_TFTP_DATA_ERROR            */ NULL,
    /* ERROR_NET_TFTP_DATA_TIMEOUT_ERROR    */ "Check cable and TFTP-server is still running",
    /* ERROR_NET_USER_BREAK                 */ NULL,
    /* ERROR_NET_INVALID_GATEWAY_IP         */ "Check environment variable 'gateway'",
    /* ERROR_NET_NOT_ON_SAME_SUBNET_IP      */ "Check env. var's 'ipaddr', 'gateway' & 'subnetmask'",
    /* ERROR_NET_UNKNOWN_GATEWAY_IP         */ "Check gateway is up and running",
    /* ERROR_NET_TFTP_FILE_TOO_BIG          */ "Check file size, must be less than 32 MByte",
    /* ERROR_NET_INVALID_SUBNET_MASK        */ "Check environment variable 'subnetmask'",
    /* ERROR_NET_PING_TIME_OUT              */ "Check remote host IP or var's 'ipaddr', 'gateway' & 'subnetmask'"
} ;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static INT32 net_error_lookup( t_sys_error_string *p_param ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_init
 *  Description :
 *  -------------
 *  Initialize the net modules
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0) 
 *
 ************************************************************************/
UINT32 NET_init( void )
{
    UINT32                          rcode ;
    t_sys_error_lookup_registration registration ;


    /* Initialize MAC-module */
    IF_ERROR( (rcode),
              (NET_MAC_init()) )

    /* Initialize ARP-module */
    IF_ERROR( (rcode),
              (NET_ARP_init( NET_poll )) )

    /* Initialize IP-module */
    IF_ERROR( (rcode),
              (NET_IP_init()) )

    /* Initialize ICMP-module */
    IF_ERROR( (rcode),
              (NET_ICMP_init()) )

    /* Initialize UDP-module */
    IF_ERROR( (rcode),
              (NET_UDP_init()) )

    /* Initialize TFTPC-module */
    IF_ERROR( (rcode),
              (NET_TFTPC_init()) )

    /* Initialize APPL-module */
    IF_ERROR( (rcode),
              (NET_APPL_init()) )

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_NET ) ;
    registration.lookup = net_error_lookup ;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_poll
 *  Description :
 *  -------------
 *  Poll the net modules
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0) 
 *
 ************************************************************************/
UINT32 NET_poll( void )
{
    int    i ;
    UINT32 rcode ;

    for (i=0;i<10;i++)
    {
        /* Poll MAC-module */
        rcode = NET_MAC_poll() ;
    }

    /* Poll ARP-module */
    rcode = NET_ARP_poll() ;

    /* Poll TFTPC-module */
    rcode = NET_TFTPC_poll( NULL, NULL, NULL ) ;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_gettime
 *  Description :
 *  -------------
 *  Get number of seconds since 1.1.1970.
 *
 *
 *
 *
 *  Parameters :
 *  ------------
 *  *time, OUT, user defined variable to return the time
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0) 
 *
 ************************************************************************/
UINT32 NET_gettime( UINT32 *time )
{
    UINT32          rcode ;
    t_RTC_calendar  rtc ;

    rcode = IO_read( SYS_MAJOR_RTC, 0, &rtc )  ;
    if (rcode == OK)
    {
        *time =                      rtc.second      +
                              (60 *  rtc.minute)     +
                         (60 * 60 *  rtc.hour)       +
                    (24 * 60 * 60 *  rtc.dayofmonth) +
               (30 * 24 * 60 * 60 *  rtc.month)      +
              (365 * 24 * 60 * 60 * (rtc.year -1970)) ;
    }

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_checksum
 *  Description :
 *  -------------
 *  Return 16-bit one's complement of 16-bit one's complement sum.
 *
 *
 *  Parameters :
 *  ------------
 *  'pw',          IN,    Pointer for start of segment
 *  'words',       IN,    Number of 16-bit words in segment
 *
 *
 *  Return values :
 *  ---------------
 *  'IP-header checksum'
 *
 ************************************************************************/
UINT16 NET_checksum( UINT16 *pw, int words )
{
    UINT32 checksum ;

    for( checksum = 0; words > 0; words-- )
    {
        checksum += *pw++ ;
    }

    checksum  = (checksum >> 16) + (checksum & 0xffff) ;
    checksum += (checksum >> 16) ;

    return( ~checksum ) ;
}


static INT32 net_error_lookup( t_sys_error_string *p_param )
{
    UINT32 t, i ;

    i = 0 ;
    p_param->count = 0 ;
    t = SYSERROR_ID( p_param->syserror ) ;
    if ( net_last_error == p_param->syserror )
    {
        /* check for recognized error code */
        if ( (t < sizeof(net_error_string)/sizeof(char*)) &&
             (net_error_string[t] != NULL ) )
        {
            /* fill in mandatory error message string */
            p_param->strings[SYSCON_ERRORMSG_IDX] = net_error_string[t] ;
            i++ ;

            /* check for diagnose message */
            if ( net_diag_msg[0] != 0 )
            {
                /* fill in optional diagnose message string */
                p_param->strings[SYSCON_DIAGMSG_IDX] = net_diag_msg ;
                i++ ;
            }

            /* check for hint message */
            if ( net_error_hint_string[t] != NULL)
            {
                /* fill in optional hint message string */
                p_param->strings[SYSCON_HINTMSG_IDX] = net_error_hint_string[t] ;
                i++ ;
            }
        }
        p_param->count      = i ;
    }

    /* delete context */
    net_last_error  = OK ;
    return(OK) ;
}

