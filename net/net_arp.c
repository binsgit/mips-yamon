/************************************************************************
 *
 *      NET_ARP.c
 *
 *      The 'NET_ARP' module implements the ARP protocol and ARP
 *      services to provide access to the ARP cache, which contains
 *      entries of set of IP and ARP addresses. Only IP-addresses
 *      requested by the user are kept in the cache until they are
 *      timed-out after a certain period without being requested.
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
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_arp.h"
#include "net_ip.h"


/************************************************************************
 *      Definitions
 ************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/

/************************************************************************
 *      Static variables
 ************************************************************************/

/* System poll function pointer, initialized by 'init' */
static UINT32 (*syspoll)(void) = NULL ;

/* Global state of ARP */
static UINT32 NET_ARP_state = ARP_STATE_CLOSED ;

/* The MAC SP-handle, returned by 'MAC-open' */
static UINT32 mac_sp_hd ;

/* Next poll time (seconds) */
static UINT32 next_poll_time ;

/* SAP table */
static t_arp_sap_context sap_context[ARP_SAP_COUNT] ;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          NET_ARP_receive
 *  Description :
 *  -------------
 *  This function gets called from the MAC-module whenever the ARP
 *  registered SAP (0x806) are addressed in a received frame.
 *
 *
 *  Parameters :
 *  ------------
 *  'src_adr': senders MAC-address
 *  'length':  length of received ethernet frame
 *  'data':    pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_ARP_receive( t_mac_addr *src_adr,
                        UINT32     length,
                        UINT8      *data    ) ;


/************************************************************************
 *
 *                          NET_ARP_alloc_sap
 *  Description :
 *  -------------
 *  Allocate an ARP SAP and register user context.
 * 
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address, which MAC-address is to be
 *                             resoluted
 *  'mac_addr',         INOUT, MAC-address, which may be defined
 *                             by the caller. At return, the MAC-address
 *                             may be defined, if the cache contains
 *                             an entry in state 'resoluted' of the
 *                             specified IP-address.
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_alloc_sap( UINT32     ip_addr,
                          t_mac_addr *mac_addr ) ;



/************************************************************************
 *
 *                          NET_ARP_reply
 *  Description :
 *  -------------
 *  Send a 'reply' with this stations IP-and MAC-addresses to 'requester'.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address of 'requester'
 *  'mac_addr',         IN,    MAC-address of 'requester'
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_reply( UINT32     ip_addr,
                      t_mac_addr *mac_addr ) ;


/************************************************************************
 *
 *                          NET_ARP_request
 *  Description :
 *  -------------
 *  Broadcast a 'request' to resolute the MAC-address of the specified
 *  'ip_addr'.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address, which must be MAC-address
 *                             resoluted. 
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_request( UINT32 ip_addr ) ;




/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_ARP_init
 *  Description :
 *  -------------
 *  Initialize the ARP module.
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
UINT32 NET_ARP_init( UINT32 (*poll)(void) )
{
    UINT32 rcode ;
    int    i ;

    /* initialize system poll function */
    syspoll = poll ;

    /* initialize ARP SAP table */
    for ( i = 0; i < ARP_SAP_COUNT; i++ )
    {
        sap_context[i].state   = ARP_SAP_STATE_CLOSED ;
        sap_context[i].ip_addr = IP_ADDR_UNDEFINED ;
        sap_context[i].timeout = 0 ;
        memcpy( sap_context[i].mac_addr, 
                mac_undefined_adr,
                SYS_MAC_ADDR_SIZE ) ;
    }

    /* We need to create our ARP-SAP in the MAC module, as ARP must
       be operational to respond on 'request' */
    rcode = NET_MAC_open( MAC_SAP_ARP,
                          NET_ARP_receive,
                          &mac_sp_hd ) ;
    if ( rcode == OK ) 
    {
        /* ARP-module is now operational */
        NET_ARP_state = ARP_STATE_OPEN ;
    }

    /* define first poll time */
    next_poll_time = 0 ;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_ARP_open
 *  Description :
 *  -------------
 *  This service is called, whenever
 *    a) the MAC-address of the specified IP-address should be found
 *       (either in the cache or an ARP-request is being broadcasted) or
 *    b) a set of known (MAC,IP)-addresses are to be stored in the cache.
 *
 *  Any entries in the ARP-cache are situated in one of the states:
 *    a) 'closed', i.e. this entry is free and may be allocated, when a
 *                 new set of (MAC,IP) shall be resoluted
 *    b) 'allocated', i.e. the IP-address is stored in the entry and an
 *                    ARP-request has been broadcasted, but the MAC
 *                    address has not been resoluted by an ARP-reply
 *    c) 'resoluted', i.e. the MAC-address of the IP-address has been
 *                    resoluted.
 *
 *  In the states, 'allocated' and 'resoluted', an entry is being
 *  supervised by a timer. By each call of this service ('open'),
 *  a reference to an entry in one these two states, will reset
 *  the supversion timer. By expiration, the entry will be deleted
 *  and the state of the 'entry' will be set to 'closed'.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address (BE-format), which
 *                             MAC-address is to be resoluted
 *  'mac_addr',         INOUT, MAC-address, which may be defined
 *                             by the caller. At return, the MAC-address
 *                             may be defined, if the cache contains
 *                             an entry in state 'resoluted' of the
 *                             specified IP-address.
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_ARP_NOT_INITIALIZED'
 *  'ERROR_NET_ARP_FATAL_STATE'
 *  'OK'                              MAC-address has been resoluted
 *
 ************************************************************************/
UINT32 NET_ARP_open( UINT32     ip_addr,
                     t_mac_addr *mac_addr )
{
    UINT32 old_ie, loop ;
    UINT32 rcode = OK ;

    loop = 3 ;
    while (1)
    {
        switch (NET_ARP_state)
        {
            case ARP_STATE_CLOSED:
                    rcode = ERROR_NET_ARP_NOT_INITIALIZED ;
                break;

            case ARP_STATE_OPEN:
                    /* try allocating a SAP */
                    old_ie = sys_disable_int() ;
                    rcode = NET_ARP_alloc_sap( ip_addr, mac_addr ) ;
                    if(old_ie) sys_enable_int();

                break;

            default:
                    /* we should never arrive here */
                    rcode = ERROR_NET_ARP_FATAL_STATE ;
                break;

        }
        loop-- ;

        if ( (rcode    == ERROR_NET_ARP_MAC_NOT_RESOLUTED) && 
             (syspoll != NULL) &&
             (loop     >  0) )
        {
            /* Wait ~5 msec */
            sys_wait_ms(5);

            (*syspoll)() ;
        }
        else
        {
            break ;
        }
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_ARP_poll
 *  Description :
 *  -------------
 *  Scans the ARP-cache to check for timer-expired entries, which
 *  will be removed from the cache.
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
UINT32 NET_ARP_poll(  void )
{
    int    i ;
    UINT32 time_now ;
    UINT32 rcode ;
    UINT32 old_ie;

    /* get time now (unit is seconds since 1.1.1970) */
    rcode = NET_gettime( &time_now ) ;

    if (time_now < next_poll_time)
    {
        return( rcode ) ;
    }
    next_poll_time = time_now + ARP_POLL_TIMEOUT ;

    /* search the SAP table to detect any expired entries */
    for ( i = 0; i < ARP_SAP_COUNT; i++ )
    {
        if (sap_context[i].state  != ARP_SAP_STATE_CLOSED)
        {
            /* DISABLE INTERRUPT */
            old_ie = sys_disable_int() ;

            if (time_now >= sap_context[i].timeout)
            {
                sap_context[i].state   = ARP_SAP_STATE_CLOSED ;
                sap_context[i].ip_addr = IP_ADDR_UNDEFINED ;
                sap_context[i].timeout = 0 ;
                memcpy( sap_context[i].mac_addr, 
                        mac_undefined_adr,
                        SYS_MAC_ADDR_SIZE ) ;
            }

            /* Restore interrupt enable status */
            if(old_ie)
                sys_enable_int();
        }
    }

    return( rcode ) ;
}


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_ARP_receive
 *  Description :
 *  -------------
 *  In "RX-frame receive int. mode", this function is called from 
 *  the LAN-driver ISR, whenever it receives an Ethernet frame. 
 *
 *  In "RX-frame receive polled mode", this function will still
 *  be called from the LAN-driver, but in this case, the call
 *  gets started when NET_ARP_poll() calls the LAN-driver 'read'
 *  entry.
 *
 *  It must check for any SAP's registered to handle the addressed
 *  SAP and if one found, the registered receive handler is called.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_adr': senders MAC-address
 *  'length':  length of received ethernet frame
 *  'data':    pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' 
 *
 ************************************************************************/
static 
UINT32 NET_ARP_receive( t_mac_addr *src_adr,
                        UINT32     length,
                        UINT8      *data    )
{
    UINT32  rcode = OK ;
    UINT8   *p ;
    UINT16  op_code ;
    UINT32  src_ip_addr ;
    UINT32  dst_ip_addr ;
    int     i ;

    /* get 'ARP-op_code' and keep it in 'CPU'-format */
    p = (data + ARP_HEADER_BASE + ARP_HEADER_OPERATION_CODE) ;
    get2( p, op_code ) ;
    op_code = BE16_TO_CPU( op_code ) ;

    /* get 'ARP-sender-ip' and keep it in 'BE'-format */
    p = (data + ARP_HEADER_BASE + ARP_HEADER_SOURCE_PROTOCOL_ADDRESS) ;
    get4( p, src_ip_addr ) ;

    /* get 'ARP-destination-ip' and keep it in 'BE'-format */
    p = (data + ARP_HEADER_BASE + ARP_HEADER_DESTINATION_PROTOCOL_ADDRESS) ;
    get4( p, dst_ip_addr ) ;

    /* Check, that this ARP-frame actually contains our IP-address */
    if (dst_ip_addr != env_ipaddr)
    {
        return( ERROR_NET_ARP_INVALID_IP_ADDR ) ;
    }

    switch (op_code)
    {
        case ARP_HEADER_OPERATION_CODE_REQUEST:
                /* Return 'response' */
                p = (data + ARP_HEADER_BASE + ARP_HEADER_SOURCE_MEDIA_ADDRESS) ;
                rcode = NET_ARP_reply( src_ip_addr, (t_mac_addr*) p ) ;
            break;

        case ARP_HEADER_OPERATION_CODE_RESPONSE:
                /* search the cache to match any 'ip_addr' */
                for ( i = 0; i < ARP_SAP_COUNT; i++ )
                {
                    if ( (sap_context[i].state == ARP_SAP_STATE_ALLOCATED) &&
                         (src_ip_addr == sap_context[i].ip_addr) )
                    {
                        /* Match found, save the MAC-address of sender */
                        sap_context[i].state   = ARP_SAP_STATE_RESOLUTED ;
                        p = (data + ARP_HEADER_BASE + ARP_HEADER_SOURCE_MEDIA_ADDRESS) ;
                        memcpy( sap_context[i].mac_addr, 
                                p,
                                SYS_MAC_ADDR_SIZE ) ;
                    }
                }
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_ARP_INVALID_OPCODE ;
            break;
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_ARP_alloc_sap
 *  Description :
 *  -------------
 *  Allocate an ARP SAP and register user context.
 * 
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address, which MAC-address is to be
 *                             resoluted
 *  'mac_addr',         INOUT, MAC-address, which may be defined
 *                             by the caller. At return, the MAC-address
 *                             may be defined, if the cache contains
 *                             an entry in state 'resoluted' of the
 *                             specified IP-address.
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:  Entry found, MAC-address has been resoluted
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_alloc_sap( UINT32     ip_addr,
                          t_mac_addr *mac_addr )

{
    int i, j ;
    UINT32 rcode, oldest, time_now ;

#ifdef NET_DEBUG
    printf("NET_ARP_alloc_sap\n") ;
#endif

    /* search the cache to match any 'ip_addr' */
    for ( i = 0; i < ARP_SAP_COUNT; i++ )
    {
        if(ip_addr == sap_context[i].ip_addr)
        {
            /* Match found, check now the state to determine action */
            if (sap_context[i].state == ARP_SAP_STATE_RESOLUTED)
            {
                /* This entry contains the MAC-address */
                memcpy( mac_addr,
                        sap_context[i].mac_addr, 
                        SYS_MAC_ADDR_SIZE ) ;

                /* refresh timer */
                IF_ERROR( (rcode),
                          (NET_gettime( &time_now )) )
                sap_context[i].timeout = time_now + ARP_SAP_TIMEOUT ;
                return( OK ) ;
            }

            if (sap_context[i].state == ARP_SAP_STATE_ALLOCATED)
            {
                /* Broadcast ARP-request again */
                IF_ERROR( (rcode),
                          (NET_ARP_request( sap_context[i].ip_addr )) )

                /* refresh timer */
                IF_ERROR( (rcode),
                          (NET_gettime( &time_now )) )
                sap_context[i].timeout = time_now + ARP_SAP_TIMEOUT ;
                return( ERROR_NET_ARP_MAC_NOT_RESOLUTED ) ;
            }
        }
    }

    /* search the cache to create a new entry */
    j = 0 ;
    oldest = 0xffffffff ;
    for ( i = 0; i < ARP_SAP_COUNT; i++ )
    {
        if (sap_context[i].state == ARP_SAP_STATE_CLOSED)
        {
            j = i ;
            break ;
        }
        else
        {
            /* we must determine the oldest entry,
               in case the cache has no free entries */
            if (sap_context[i].timeout < oldest)
            {
                oldest = sap_context[i].timeout ;
                j = i ;
            }
        }
    }

    /* We have now determined where to put the new entry */
    i = j ;
    if ( memcmp(mac_addr, mac_undefined_adr, SYS_MAC_ADDR_SIZE) )
    {
        /* MAC is resoluted, create a new entry */

        /* Save (IP,MAC)-address */
        sap_context[i].ip_addr = ip_addr ;
        memcpy( sap_context[i].mac_addr, 
                mac_addr,
                SYS_MAC_ADDR_SIZE ) ;
        sap_context[i].state = ARP_SAP_STATE_RESOLUTED ;

        /* set timer */
        IF_ERROR( (rcode),
                  (NET_gettime( &time_now )) )
        sap_context[i].timeout = time_now + ARP_SAP_TIMEOUT ;
        return( OK ) ;
    }
    else
    {
        /* MAC must be resoluted, create a new entry */

        /* Save (IP)-address */
        sap_context[i].ip_addr = ip_addr ;
        memcpy( sap_context[i].mac_addr, 
                mac_undefined_adr,
                SYS_MAC_ADDR_SIZE ) ;
        sap_context[i].state = ARP_SAP_STATE_ALLOCATED ;

        /* Broadcast ARP-request */
        IF_ERROR( (rcode),
                  (NET_ARP_request( ip_addr )) )

        /* set timer */
        IF_ERROR( (rcode),
                  (NET_gettime( &time_now )) )
        sap_context[i].timeout = time_now + ARP_SAP_TIMEOUT ;
        return( ERROR_NET_ARP_MAC_NOT_RESOLUTED ) ;
    }
}


/************************************************************************
 *
 *                          NET_ARP_reply
 *  Description :
 *  -------------
 *  Send a 'reply' with this stations IP-and MAC-addresses to 'requester'.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address of 'requester' (BE-format)
 *  'mac_addr',         IN,    MAC-address of 'requester'
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_reply( UINT32     ip_addr,
                      t_mac_addr *mac_addr )
{
    UINT8   frame[MAC_HEADER_SIZE + ARP_HEADER_SIZE] ;
    UINT32  rcode = OK ;
    UINT8   *p ;

    /* build the ARP-reply header */
    p = &frame[ ARP_HEADER_BASE ] ;
    putbe2( ARP_HEADER_MEDIA_TYPE_ETHERNET , p ) 

    putbe2( ARP_HEADER_PROTOCOL_TYPE_IP , p ) 
    
    put1( ARP_HEADER_MEDIA_ADDRESS_SIZE_ETHERNET , p ) 

    put1( ARP_HEADER_PROTOCOL_ADDRESS_SIZE_IP , p ) 

    putbe2( ARP_HEADER_OPERATION_CODE_RESPONSE , p ) 
    
    put6( mac_station_adr, p )
    put4( env_ipaddr, p )

    put6( *mac_addr, p )
    put4( ip_addr, p )

    /* request MAC to send the reply */
    rcode = NET_MAC_send( mac_sp_hd,
                          mac_addr,
                          MAC_HEADER_SIZE + ARP_HEADER_SIZE,
                          frame ) ;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_ARP_request
 *  Description :
 *  -------------
 *  Broadcast a 'request' to resolute the MAC-address of the specified
 *  'ip_addr'.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_addr',          IN,    IP-address (BE-format), 
 *                             which is going to be MAC-address resoluted.
 *
 *
 *  Return values :
 *  ---------------
 * 'OK' = 0x00:
 *
 *
 ************************************************************************/
static
UINT32 NET_ARP_request( UINT32 ip_addr )
{
    UINT8   frame[MAC_HEADER_SIZE + ARP_HEADER_SIZE] ;
    UINT32  rcode = OK ;
    UINT8   *p ;

    /* build the ARP-reply header */
    p = &frame[ ARP_HEADER_BASE ] ;
    putbe2( ARP_HEADER_MEDIA_TYPE_ETHERNET , p )

    putbe2( ARP_HEADER_PROTOCOL_TYPE_IP , p )
   
    put1( ARP_HEADER_MEDIA_ADDRESS_SIZE_ETHERNET , p )

    put1( ARP_HEADER_PROTOCOL_ADDRESS_SIZE_IP , p )

    putbe2( ARP_HEADER_OPERATION_CODE_REQUEST , p )

    put6( mac_station_adr, p )
    put4( env_ipaddr, p )

    put6( mac_undefined_adr, p )
    put4( ip_addr, p )

    /* Request MAC to broadcast the request */
    rcode = NET_MAC_send( mac_sp_hd,
                          &mac_broadcast_adr,
                          MAC_HEADER_SIZE + ARP_HEADER_SIZE,
                          frame ) ;

    return( rcode ) ;
}
