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
 *      Include files
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <shell_api.h>

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_arp.h"
#include "net_ip.h"
#include "net_icmp.h"
#include "net_tftpc.h"


/************************************************************************
 *      Definitions
 ************************************************************************/



/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

static bool       appl_reply_received ;
static UINT16     appl_reply_sequence ;
static UINT16     appl_reply_length ;
static UINT32     appl_reply_ip ;
static t_mac_addr appl_reply_mac_adr ;

static UINT16     appl_sequence ;
static UINT16     appl_length ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

/************************************************************************
 *
 *                          NET_APPL_echo_reply
 *  Description :
 *  -------------
 *  Initialize the net applications
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
static
UINT32 NET_APPL_echo_reply( UINT32     src_ip_adr,  /* BE-format */
                            t_mac_addr *src_mac_adr,/* MAC */
                            UINT16     sequence,    /* CPU-format */
                            UINT16     usr_length ) ;

/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_APPL_init
 *  Description :
 *  -------------
 *  Initialize the net applications
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
UINT32 NET_APPL_init( void )
{
    UINT32          rcode ;

    /* register ECHO-REPLY handler */
    IF_ERROR( (rcode),
               (NET_ICMP_ECHO_open( NET_APPL_echo_reply )) )

    return( OK ) ; 
}


/************************************************************************
 *
 *                          NET_ping
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
UINT32
NET_ping( UINT32 ipadr, UINT32 data_size )
{
    UINT32 dot_count = 0;
    UINT32 compl, rcode, time_start, time_now ;
    UINT32 retry = 3 ;

#ifdef NET_DEBUG
    printf("net_ping\n") ;
#endif

    /* clear error context */
    net_last_error  = OK ;
    net_diag_msg[0] = 0 ;

    /* check data_size */
    if ( data_size > PING_MAX_DATAGRAM_SIZE )
    {
        data_size = PING_DEFAULT_DATAGRAM_SIZE ;
    }

    /* Set context before request is called */
    appl_reply_received = FALSE ;
    appl_sequence = data_size ;
    appl_length   = data_size ;

    /* Request ICMP to 'send' */
    compl = NET_ICMP_ECHO_send( ipadr, NULL, appl_sequence, appl_length ) ;
    if ( (compl != OK) &&
         (compl != ERROR_NET_ARP_MAC_NOT_RESOLUTED) )
    {
        return( compl ) ;
    }

    /* Get time right now */
    IF_ERROR( (rcode),
               (NET_gettime( &time_start)) )

    while (1)
    {
        /* Poll for any reply */
        NET_poll() ;

        if ( appl_reply_received )
        {
            if ( ( appl_sequence == appl_reply_sequence ) &&
                 ( appl_length   == appl_reply_length   ) &&
                 ( ipadr         == appl_reply_ip       ) )
            {
                if( dot_count )
                {
                    printf("\n") ;
                }
                printf("%d bytes ICMP-ECHO-REPLY user data received from %d.%d.%d.%d\n",
                                                         appl_length,
                                                        ((UINT8*)&ipadr)[0],
                                                        ((UINT8*)&ipadr)[1],
                                                        ((UINT8*)&ipadr)[2],
                                                        ((UINT8*)&ipadr)[3] );

                rcode = OK ;
                break ;
            }
        }
 
        /* Get time right now */
        IF_ERROR( (rcode),
                   (NET_gettime( &time_now)) )

        if ((time_now - time_start) >= 1)
        {
            if(shell_print_dot( &dot_count ))
            {
              rcode = SHELL_ERROR_CONTROL_C_DETECTED ;
              break ;
            }

            /* Do we have any retries left */
            if (!retry)
            {
                rcode = compl ;
                if ( compl == ERROR_NET_ARP_MAC_NOT_RESOLUTED )
                {
                    net_last_error = ERROR_NET_ARP_TIME_OUT ;
                    rcode = net_last_error ;
                }
                if ( compl == OK )
                {
                    net_last_error = ERROR_NET_PING_TIME_OUT ;
                    rcode = ERROR_NET_PING_TIME_OUT ;
                }
                if( dot_count )
                {
                    printf("\n") ;
                }
                break ;
            }

            /* retransmit */
            time_start = time_now ;

            /* Request ICMP to 'send' */
            compl = NET_ICMP_ECHO_send( ipadr, NULL, appl_sequence, appl_length ) ;
            retry-- ;
        }

    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          TEST_ping
 *  Description :
 *  -------------
 *  Special version of ping for production test:
 *      non verbose
 *      without retries
 *      with variable timeout in milliseconds
 *
 *  The lack of retries makes it important to avoid false timeouts.
 *  The time algorithm depends on the fact, that the timeout value
 *   plus the maximum possible timer value will not wrap.
 *  Timeout is measured in milliseconds.
 *  The raw timer wraps every 16777216 microsecs ~ 16777 millisec
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
UINT32
TEST_ping( UINT32 ipadr, UINT32 data_size, UINT32 ping_ms_timeout)
{
    UINT32 rcode, latest, previous, mstimout ;

    /* check data_size */
    if ( data_size > PING_MAX_DATAGRAM_SIZE )
    {
        data_size = PING_DEFAULT_DATAGRAM_SIZE ;
    }

    /* Set context before request is called */
    appl_reply_received = FALSE ;
    appl_sequence = data_size ;
    appl_length   = data_size ;

    /* Request ICMP to 'send' */
    NET_ICMP_ECHO_send( ipadr,
                        0,
                        appl_sequence,
                        appl_length );

    /* Get millisecond count right now */
    SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                 &latest,
                 sizeof(latest) ) ;
    mstimout = latest + ping_ms_timeout ;  /* typically 1000 = one second */

    do
    {
        /* Poll for any reply */
        NET_poll() ;

        if ( appl_reply_received )
        {
            if ( ( appl_sequence == appl_reply_sequence ) &&
                 ( appl_length   == appl_reply_length   ) &&
                 ( ipadr         == appl_reply_ip       ) )
            {
                rcode = OK ;
                break ;
            }
        }
 
        previous = latest;
        /* get millisecond count right now */
        rcode = SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                             &latest,
                             sizeof(latest) ) ;
        if ( latest < previous )
        {
            mstimout -= previous;  /* may cause a few millisec extra delay */
        }
        if (latest > mstimout) 
        {
            rcode = ERROR_NET_PING_TIME_OUT ;
        }
    } while (rcode == OK);

    return( rcode ) ;
}



/************************************************************************
 *
 *                          NET_APPL_echo_reply
 *  Description :
 *  -------------
 *  Receive 'ECHO'-reply
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
static
UINT32 NET_APPL_echo_reply( UINT32     src_ip_adr,  /* BE-format */
                            t_mac_addr *src_mac_adr,/* MAC */
                            UINT16     sequence,    /* CPU-format */
                            UINT16     usr_length )
{

#ifdef NET_DEBUG
    int i ;
    UINT8 *p ;
    printf("NET_APPL_echo_reply\n") ;
    printf("src_ip_adr = %x\n", (UINT32)BE32_TO_CPU(src_ip_adr) ) ;
    printf("sequence   = %d\n", sequence ) ;
    printf("usr_length = %d\n", usr_length ) ;
    printf("src MAC = " ) ;
    p = (UINT8*) src_mac_adr ;
    for (i=0; i<6; i++)
    {
        printf(" %02x",*p++ ) ;
    }
    printf("\n" ) ;
#endif

    /* save reply context */
    if ( !appl_reply_received )
    {
        appl_reply_received = TRUE ;
        appl_reply_sequence = sequence ;
        appl_reply_length   = usr_length ;
        appl_reply_ip       = src_ip_adr ;
        memcpy( &appl_reply_mac_adr, src_mac_adr, SYS_MAC_ADDR_SIZE ) ;
    }

    return( OK ) ; 
}


UINT32
net_open( UINT32 ipadr, char *filename )
{
    UINT32 compl, state, cause, sp_hd ;

    /* clear error context */
    net_last_error  = OK ;
    net_diag_msg[0] = 0 ;

    compl = NET_TFTPC_open( ipadr, filename, &sp_hd ) ;
    if ( (compl != OK) &&
         (compl != ERROR_NET_ARP_MAC_NOT_RESOLUTED) )
    {
        NET_TFTPC_close( 0 ) ;
        return( compl ) ;
    }

    while (1)
    {
        /* Poll for any reply */
        NET_poll() ;

        if (GETCHAR_CTRLC( DEFAULT_PORT ))
        {
            NET_TFTPC_close( 0 ) ;
            net_last_error = ERROR_NET_USER_BREAK ;
            compl = net_last_error ;
            break ;
        }

        NET_TFTPC_poll( &state, &compl, &cause ) ;
        if (state != TFTPC_SAP_STATE_WAIT_FIRST_BLOCK)
        {
            if (state == TFTPC_SAP_STATE_ERROR)
            {
               net_last_error = cause ;
               compl = net_last_error ;
               NET_TFTPC_close( 0 ) ;
            }
            else
            {
                compl = OK ;
            }
            break ;
        }
    }


    /* We need to reply OK, as loader will terminate by any error */
    return( compl ) ;
}

char net_getchar( void )
{
    char   byte ;
    UINT32 rcode, state, compl, cause ;

    rcode = NET_TFTPC_readbyte( 0,
                                &byte ) ;
    if (!rcode)
    {
        return( byte ) ;
    }

    while (1)
    {
        /* Poll for any reply */
        NET_poll() ;

        if (GETCHAR_CTRLC( DEFAULT_PORT ))
        {
            NET_TFTPC_close( 0 ) ;
            net_last_error  = ERROR_NET_USER_BREAK ;
            return( EOF_SREC ) ;
        }

        rcode = NET_TFTPC_readbyte( 0,
                                    &byte ) ;
        if (!rcode)
        {
            return( byte ) ;
        }

        if (rcode == ERROR_NET_TFTPC_NO_DATA)
        {
            continue ;
        }

        if (rcode == ERROR_NET_TFTPC_EOF)
        {
            NET_TFTPC_close( 0 ) ;
            return( EOF_SREC ) ;
        }

        if (rcode == ERROR_NET_TFTPC_SAP_ERROR_STATE)
        {
            NET_TFTPC_poll( &state, &compl, &cause ) ;
            net_last_error = cause ;
            NET_TFTPC_close( 0 ) ;
            return( EOF_SREC ) ;
        }
    }
}


UINT32
net_getbyte( UINT8 *byte )
{
    UINT32 rcode, state, compl, cause ;

    rcode = NET_TFTPC_readbyte( 0,
                                byte ) ;
    if (!rcode)
    {
        return( rcode ) ;
    }

    while (1)
    {
        /* Poll for any reply */
        NET_poll() ;

        if (GETCHAR_CTRLC( DEFAULT_PORT ))
        {
            NET_TFTPC_close( 0 ) ;
            net_last_error  = ERROR_NET_USER_BREAK ;
            return( ERROR_NET_USER_BREAK ) ;
        }

        rcode = NET_TFTPC_readbyte( 0,
                                    byte ) ;
        if (!rcode)
        {
            return( rcode ) ;
        }

        if (rcode == ERROR_NET_TFTPC_NO_DATA)
        {
            continue ;
        }

        if (rcode == ERROR_NET_TFTPC_EOF)
        {
            NET_TFTPC_close( 0 ) ;
            return( rcode ) ;
        }

        if (rcode == ERROR_NET_TFTPC_SAP_ERROR_STATE)
        {
            NET_TFTPC_poll( &state, &compl, &cause ) ;
            net_last_error = cause ;
            NET_TFTPC_close( 0 ) ;
            return( rcode ) ;
        }
    }
}

UINT32
NET_file_read( UINT32 ipadr,
               char   *filename,
               UINT8  *buffer,
               UINT32 *size )
{
    return( NET_TFTPC_file_read( ipadr,
                                 filename,
                                 buffer,
                                 size,
                                 NET_poll ) ) ;
}

UINT32
NET_file_write( UINT32 ipadr,
                char   *filename,
                UINT8  *buffer,
                UINT32 *size )
{
    return( NET_TFTPC_file_write( ipadr,
                                  filename,
                                  buffer,
                                  size,
                                  NET_poll ) ) ;
}
