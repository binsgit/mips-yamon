/************************************************************************
 *
 *      NET_ICMP.c
 *
 *      The 'NET_ICMP' module currently implements the ECHO part of
 *      the ICMP layer. It provides services to send and receive
 *      ECHO events. Service, to send ICMP 'unreachable' added.
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

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_ip.h"
#include "net_icmp.h"


/************************************************************************
 *      Definitions
 ************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

/* Global state of ICMP */
static UINT32 NET_ICMP_state = ICMP_STATE_CLOSED ;

/* The IP SP-handle, returned by 'IP-open' */
static UINT32 ip_sp_hd ;

/* ICMP ECHO SAP */
static t_icmp_echo_usr_receive icmp_echo_sap ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          NET_ICMP_receive
 *  Description :
 *  -------------
 *  This function is registered in the IP-module and linked with the
 *  IP-protocol = '1', to let the IP call us back with a reference
 *  to the received frame, containing an ICMP-frame.
 *  In this function the ICMP-header will be validated and 
 *  the ICMP-ECHO-SAP will be checked to call a user registered
 *  ECHO-reply handler, which may check sender-IP-address, sequence
 *  number and length of user data received.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  sender's IP-address (BE-format)
 *  'src_mac_adr': sender's MAC-address
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_ICMP_receive( UINT32     src_ip_adr,
                         t_mac_addr *src_mac_adr,
                         UINT32     length,
                         UINT8      *data    ) ;



/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_ICMP_init
 *  Description :
 *  -------------
 *  Initialize the ICMP module.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
UINT32 NET_ICMP_init(  void )
{
    UINT32 rcode ;

    /* initialize ICMP ECHO SAP */
    icmp_echo_sap  = NULL ;

    /* register our receive handler */
    IF_ERROR( (rcode),
              (NET_IP_open( IP_SAP_ICMP,
                            NET_ICMP_receive,
                            &ip_sp_hd) ) )

    /* ICMP-module has now been initialized and opened for service */
    NET_ICMP_state = ICMP_STATE_OPEN ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_ICMP_ECHO_open
 *  Description :
 *  -------------
 *  Allocate a ICMP-ECHO-SAP and register user context.
 *
 *
 *  Parameters :
 *  ------------
 *  'usr_receive',     IN,    user-receive function to be registered
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_ICMP_FATAL_STATE'      A fatal state has been detected in ICMP.
 *  'ERROR_NET_ICMP_NOT_INITIALIZED'  ICMP-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_ICMP_ECHO_open( t_icmp_echo_usr_receive usr_receive )
{
    UINT32          rcode = OK ;

    switch (NET_ICMP_state)
    {
        case ICMP_STATE_CLOSED:
                rcode = ERROR_NET_ICMP_NOT_INITIALIZED ;
            break;

        case ICMP_STATE_OPEN:
                /* register receive handler */
                icmp_echo_sap = usr_receive ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_ICMP_FATAL_STATE ;
            break;

    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_ICMP_ECHO_close
 *  Description :
 *  -------------
 *  Close  ICMP-ECHO-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                ICMP ECHO SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_ICMP_ECHO_close( void )
{
    /* un-register receive handler */
    icmp_echo_sap = NULL ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_ICMP_ECHO_send
 *  Description :
 *  -------------
 *  Request the ICMP ECHO module to send a 'ECHO' datagram
 *  to a specified destination IP-address with a specified
 *  sequence number and length of ECHO user data.
 *  Optionally a MAC-address for destination may be supplied.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',           IN,   destination ip address (BE-format)
 *  'mac_adr',          IN,   optional MAC address
 *  'sequence',         IN,   sequence number to assign datagram
 *  'length',           IN,   length of ECHO-data to send
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0)
 *
 ************************************************************************/
UINT32 NET_ICMP_ECHO_send( UINT32     ip_adr,  /* destination ip address (BE) */
                           t_mac_addr *mac_adr,/* optional MAC destination    */
                           UINT16     sequence,
                           UINT16     length )
{
    UINT8      data[MAC_MAX_FRAME_SIZE] ;  /* WARNING: This function needs stack */
    UINT32     rcode ;
    UINT8      *p ;
    UINT16     tmp2 ;
    int        i ;


    /* Length validation */
    if ( length > ICMP_ECHO_HEADER_DATA_MAX_SIZE )
    {
        return( ERROR_NET_ICMP_INVALID_DATA_SIZE ) ;
    }

    /* Fill-in ICMP header of frame */
    p = &data[ICMP_ECHO_HEADER_BASE]  ;

    /* Fill-in 'type'(=REQUEST)       */
    put1( ICMP_ECHO_HEADER_TYPE_ECHOREQUEST , p ) ;

    /* Fill-in 'code'                 */
    put1( ICMP_ECHO_HEADER_CODE_ECHO , p ) ;

    /* Fill-in 'checksum' (temp = 0)  */
    put2( 0, p ) ;

    /* Fill-in 'identification'       */
    putbe2( ICMP_ECHO_HEADER_IDENTIFICATION_MIPS , p ) ;

    /* Fill-in 'sequence'             */
    putbe2( sequence , p ) ;

    /* Fill-in data */
    for (i=0; i<length; i++)
    {
        put1( (UINT8)i , p ) ;
    }

    /* Check odd case */
    tmp2 = ICMP_ECHO_HEADER_SIZE/2 + (length/2) ;
    if (length%2)
    {
        put1( 0, p ) ;
        tmp2 = tmp2 + 1 ;
    }

    /* complete ECHO-header with checksum */
    p = &data[ICMP_ECHO_HEADER_BASE] ;
    tmp2 = NET_checksum( (UINT16*)p, tmp2 ) ;
    p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_CHECKSUM) ;
    put2( tmp2, p ) ;

    /* request IP to send this frame */
    rcode = NET_IP_send( ip_sp_hd,
                         ip_adr,  
                         mac_adr,
                         length+ICMP_ECHO_HEADER_SIZE+IP_HEADER_SIZE+MAC_HEADER_SIZE,
                         data ) ;

    /* return completion */
    return( rcode ) ;
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_ICMP_receive
 *  Description :
 *  -------------
 *  This function is registered in the IP-module and linked with the
 *  IP-protocol = '1', to let the IP call us back with a reference
 *  to the received frame, containing an ICMP-frame.
 *  In this function the ICMP-header will be validated and
 *  the ICMP-ECHO-SAP will be checked to call a user registered
 *  ECHO-reply handler, which may check sender-IP-address, sequence
 *  number and length of user data received.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  sender's IP-address (BE-format)
 *  'src_mac_adr': sender's MAC-address
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'
 *
 ************************************************************************/
static
UINT32 NET_ICMP_receive( UINT32     src_ip_adr,
                         t_mac_addr *src_mac_adr,
                         UINT32     length,
                         UINT8      *data    )
{
    UINT32     rcode ;
    UINT8      *p ;
    UINT8      type ;
    UINT16     tmp2, checksum, sequence, identification, words, usr_length ;

#ifdef NET_DEBUG
    printf("NET_ICMP_receive:\n") ;
#endif

    /* validate ICMP checksum */
    p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_CHECKSUM) ;
    get2( p, checksum) ;

    p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_CHECKSUM) ;
    put2( 0, p ) ;

    /* odd case */
    p = &data[length] ;
    words = (length - (ICMP_ECHO_HEADER_BASE))/2 ;
    if (length%2)
    {
        put1( 0, p ) ;
        words = words + 1 ;
    }

    p = &data[ICMP_ECHO_HEADER_BASE] ;
    tmp2 = NET_checksum( (UINT16*)p, words ) ;
    if ( tmp2 != checksum )
    {
        return( ERROR_NET_ICMP_INVALID_CHECKSUM ) ;
    }

    /* get ICMP-'type'-field of this received frame */
    p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_TYPE) ;
    get1( p, type) ;

    switch(type)
    {
        case ICMP_ECHO_HEADER_TYPE_ECHOREQUEST:
                /* make a reply */

                /* Put-in reply */
                p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_TYPE) ;
                put1( ICMP_ECHO_HEADER_TYPE_ECHOREPLY , p) ;

                /* Calculate checksum */
                p = &data[ICMP_ECHO_HEADER_BASE] ;
                tmp2 = NET_checksum( (UINT16*)p, words ) ;

                /* Put in checksum */
                p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_CHECKSUM) ;
                put2( tmp2, p ) ;

                /* request IP to send this frame */
                rcode = NET_IP_send( ip_sp_hd,
                                     src_ip_adr,  
                                     src_mac_adr,
                                     length,
                                     data ) ;
            break ;

        case ICMP_ECHO_HEADER_TYPE_ECHOREPLY:
                if (icmp_echo_sap != NULL)
                {
                    /* validate reply and call user */
                    p = (data + ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_IDENTIFICATION) ;
                    get2( p, identification ) ;
                    tmp2 = BE16_TO_CPU( identification ) ;
                    if (tmp2 != ICMP_ECHO_HEADER_IDENTIFICATION_MIPS)
                    {
                        rcode = ERROR_NET_ICMP_INVALID_ID ; 
                    }
                    else
                    {
                        get2( p, tmp2 ) ;
                        sequence   = BE16_TO_CPU( tmp2 ) ;
                        usr_length = length - (ICMP_ECHO_HEADER_BASE + ICMP_ECHO_HEADER_SIZE) ;
                        rcode = (*icmp_echo_sap)( src_ip_adr, src_mac_adr, sequence, usr_length ) ;
                    }
                }
                else
                {
                    rcode = ERROR_NET_ICMP_NO_USER ; 
                }
            break ;


        default:
                    rcode = ERROR_NET_ICMP_INVALID_TYPE ; 
            break ;
    }
    return( rcode ) ; 
}


/************************************************************************
 *
 *                          NET_ICMP_UNREACHABLE_send
 *  Description :
 *  -------------
 *  Request the ICMP module to send a 'destination unreachable' datagram
 *  to a specified destination IP-address with a specified
 *  unreachable code along with the received IP-header.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',           IN,   destination ip address (BE-format)
 *  'mac_adr',          IN,   optional MAC address
 *  'ip_header',        IN,   ip-header of received datagram
 *  'code',             IN,   unreachable code
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0)
 *
 ************************************************************************/
UINT32 NET_ICMP_UNREACHABLE_send( 
                           UINT32     ip_adr,  /* destination ip address (BE) */
                           t_mac_addr *mac_adr,/* optional MAC destination    */
                           UINT8      *ip_header,
                           UINT8      code )
{
    UINT8      data[MAC_MAX_FRAME_SIZE] ;  /* WARNING: This function needs stack */
    UINT32     rcode ;
    UINT8      *p ;
    UINT32     tmp4 ;
    UINT16     tmp2 ;
    int        i ;


    /* Fill-in ICMP header of frame */
    p = &data[ICMP_UNREACHABLE_HEADER_BASE]  ;

    /* Fill-in 'type'(=UNREACHABLE)       */
    put1( ICMP_UNREACHABLE_HEADER_TYPE_UNREACHABLE, p ) ;

    /* Fill-in 'code'                 */
    put1( code, p ) ;

    /* Fill-in 'checksum' (temp = 0)  */
    put2( 0, p ) ;

    /* Fill-in 'reserved' (= 0)       */
    tmp4 = 0 ;
    put4( tmp4, p ) ;

    /* Fill-in received ip-header in data field */
    for (i=0; i<ICMP_UNREACHABLE_DATA_SIZE; i++)
    {
        put1( *ip_header++ , p ) ;
    }

    /* complete UNREACHABLE-header with checksum */
    p = &data[ICMP_UNREACHABLE_HEADER_BASE] ;
    tmp2 = (ICMP_UNREACHABLE_DATA_SIZE+ICMP_UNREACHABLE_HEADER_SIZE)/2 ;
    tmp2 = NET_checksum( (UINT16*)p, tmp2 ) ;
    p = (data + ICMP_UNREACHABLE_HEADER_BASE + ICMP_UNREACHABLE_HEADER_CHECKSUM) ;
    put2( tmp2, p ) ;

    /* request IP to send this frame */
    rcode = NET_IP_send( ip_sp_hd,
                         ip_adr,  
                         mac_adr,
                         ICMP_UNREACHABLE_DATA_SIZE+ICMP_UNREACHABLE_HEADER_SIZE+IP_HEADER_SIZE+MAC_HEADER_SIZE,
                         data ) ;

    /* return completion */
    return( rcode ) ;
}

