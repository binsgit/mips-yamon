/************************************************************************
 *
 *      NET_IP.c
 *
 *      The 'NET_IP' module implements the IP-layer services to provide
 *      service access points, which are linked with the 'protocol'-field
 *      of the IP-header.
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

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_arp.h"
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

/* Global state of IP */
static UINT32 NET_IP_state = IP_STATE_CLOSED ;

/* The MAC SP-handle, returned by 'MAC-open' */
static UINT32 mac_sp_hd ;

/* SAP table */
static t_ip_sap_context sap_context[IP_SAP_COUNT] ;

/* Global identification of IP datagrams; implemented as a counter */
static UINT32 NET_IP_identification = 0 ;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          NET_IP_receive
 *  Description :
 *  -------------
 *  This function is registered in the MAC-module and linked with the
 *  MAC-type = '0x800', to let the MAC call us back with an reference
 *  to the received frame, containing an IP-frame.
 *  In this function the IP-header will be validated and the IP-SAP
 *  table will be searched for any match between the actual value
 *  of the IP-protocol field and a registered user to handle upper
 *  protocol layer.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_mac_adr': senders MAC-address
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
UINT32 NET_IP_receive( t_mac_addr *src_mac_adr,
                       UINT32     length,
                       UINT8      *data    ) ;


/************************************************************************
 *
 *                          NET_IP_alloc_sap
 *  Description :
 *  -------------
 *  Allocate a IP SAP, register user context and return a sp-handle.
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'sap_id',             IN,    value of IP-'protocol' to bind for
 *  'ip_usr_receive',     IN,    user-receive function to be registered
 *  'ip_sp_hd',           OUT,   handle of IP to be used by user by call
 *                               of 'send' or 'close'
 *
 *
 *  Return values :
 *  ---------------
 * 'ERROR_NET_IP_NO_FREE_SAP', No SAP entry could be allocated
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
UINT32 NET_IP_alloc_sap( UINT8            sap_id,
                         t_ip_usr_receive ip_usr_receive,
                         UINT32           *ip_sp_hd ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_IP_init
 *  Description :
 *  -------------
 *  Initialize the IP module.
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
UINT32 NET_IP_init(  void )
{
    int    i ;

    /* initialize IP SAP table */
    for ( i = 0; i <IP_SAP_COUNT ; i++)
    {
        sap_context[i].ip_sap_state   = IP_SAP_STATE_CLOSED ;
        sap_context[i].ip_sap         = IP_SAP_UNDEFINED ;
        sap_context[i].ip_usr_receive = NULL ;
    }

    /* IP-module has now been initialized */
    NET_IP_state = IP_STATE_INITED ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_IP_open
 *  Description :
 *  -------------
 *  Allocate a IP-SAP and register user context.
 *
 *
 *  Parameters :
 *  ------------
 *  'sap_id',          IN,    value of IP-'protocol' to bind for
 *  'usr_receive',     IN,    user-receive function to be registered
 *  'ip_sp_hd',        OUT,   handle of IP to be used by user by call
 *                            of 'send' or 'close'
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_IP_FATAL_STATE'      A fatal state has been detected in IP.
 *  'ERROR_NET_IP_NOT_INITIALIZED'  IP-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_IP_open( UINT8            sap_id,
                    t_ip_usr_receive ip_usr_receive,
                    UINT32           *ip_sp_hd )
{
    UINT32          rcode = OK ;

    switch (NET_IP_state)
    {
        case IP_STATE_CLOSED:
                rcode = ERROR_NET_IP_NOT_INITIALIZED ;
            break;

        case IP_STATE_INITED:
                /* try allocating a SAP */
                rcode = NET_IP_alloc_sap( sap_id,
                                          ip_usr_receive,
                                          ip_sp_hd ) ;
                if ( rcode == OK ) 
                {
                    /* By first 'open', we need to create our SAP in MAC */
                    rcode = NET_MAC_open( MAC_SAP_IP,
                                          NET_IP_receive,
                                          &mac_sp_hd ) ;
                    if ( rcode == OK )
                    {
                        NET_IP_state = IP_STATE_OPEN ;
                    }
                }
            break;

        case IP_STATE_OPEN:
                /* try allocating a SAP */
                rcode = NET_IP_alloc_sap( sap_id,
                                          ip_usr_receive,
                                          ip_sp_hd ) ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_IP_FATAL_STATE ;
            break;

    }
    return( rcode ) ;
}



/************************************************************************
 *
 *                          NET_IP_close
 *  Description :
 *  -------------
 *  Close a IP-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_sp_hd'     service provider defined handle
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_IP_INVALID_HANDLE', invalid handle
 *  'OK'(=0),                      SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_IP_close( UINT32  ip_sp_hd )
{
    /* validate handle */
    IF_UPPER( ERROR_NET_IP_INVALID_HANDLE, ip_sp_hd, IP_SAP_COUNT)

    /* close SAP */
    sap_context[ip_sp_hd].ip_sap_state   = IP_SAP_STATE_CLOSED ;
    sap_context[ip_sp_hd].ip_sap         = IP_SAP_UNDEFINED ;
    sap_context[ip_sp_hd].ip_usr_receive = NULL ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_IP_send
 *  Description :
 *  -------------
 *  Request the IP module to send a frame, linked
 *  to a certain 'SAP' to a specified IP-destination address.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_sp_hd',           IN,   handle to lookup a registered SAP context
 *  'dst_adr',            IN,   destination ip address (BE-format)
 *  'dst_mac_adr',        IN,   destination MAC address (may be undefined =NULL)
 *  'length',             IN,   length of frame to send
 *  'data',               IN,   address to start of frame to be send
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0)
 *
 ************************************************************************/
UINT32 NET_IP_send( UINT32      ip_sp_hd, /* service provider defined handle */
                    UINT32      dst_adr,  /* destination ip address (BE)     */
                    t_mac_addr *dst_mac_adr, /* destination MAC-address      */
                    UINT32      length,   /* total length of frame to send   */
                    UINT8       *data  )  /* pointer to start of frame       */
{
    UINT32     rcode ;
    t_mac_addr mac_addr ;
    UINT8      *p ;
    UINT16     tmp2 ;

    /* validate handle */
    IF_UPPER( ERROR_NET_IP_INVALID_HANDLE, ip_sp_hd, IP_SAP_COUNT)

    /* validate state */
    if ( sap_context[ip_sp_hd].ip_sap_state != IP_SAP_STATE_OPEN )
    {
        return( ERROR_NET_IP_SAP_CLOSED ) ;
    }

    /* validate destination IP-address */
    if (dst_adr == IP_ADDR_UNDEFINED)
    {
        /* Destination host is undefined ! */
        sprintf( net_diag_msg,"Destination host: %d.%d.%d.%d", ((dst_adr>>24)&0xff),
                                                               ((dst_adr>>16)&0xff),
                                                               ((dst_adr>> 8)&0xff),
                                                               ((dst_adr>> 0)&0xff) );
        net_last_error = ERROR_NET_INVALID_HOST_IP ;
        return( ERROR_NET_INVALID_HOST_IP ) ;
    }

    /* get our IP address, and validate */
    if (env_ipaddr == IP_ADDR_UNDEFINED)
    {
        /* Our IP address of this board is undefined ! */
        sprintf( net_diag_msg,"Board IP-address: %d.%d.%d.%d",
                        ((UINT8*)&env_ipaddr)[0],
                        ((UINT8*)&env_ipaddr)[1],
                        ((UINT8*)&env_ipaddr)[2],
                        ((UINT8*)&env_ipaddr)[3] );
        net_last_error = ERROR_NET_INVALID_BOARD_IP ;
        return( ERROR_NET_INVALID_BOARD_IP ) ;
    }

    /* validate subnet mask */
    if (env_subnetmask == 0)
    {
        /* Subnet mask is undefined ! */
        sprintf( net_diag_msg,"subnetmask: %d.%d.%d.%d",
                        ((UINT8*)&env_subnetmask)[0],
                        ((UINT8*)&env_subnetmask)[1],
                        ((UINT8*)&env_subnetmask)[2],
                        ((UINT8*)&env_subnetmask)[3] );
        net_last_error = ERROR_NET_INVALID_SUBNET_MASK ;
        return( ERROR_NET_INVALID_SUBNET_MASK ) ;
    }

    /* check, if we have any MAC to use */
    if (dst_mac_adr == NULL)
    {
        /* clear mac */
        memcpy(mac_addr, mac_undefined_adr, SYS_MAC_ADDR_SIZE) ;

        /* Check for use of default gateway */
        if ( (env_subnetmask & env_ipaddr) != (env_subnetmask & dst_adr) )
        {
            /* Not on same subnet, we need to use default gateway */
            if (env_gateway == 0)
            {
                /* Gateway address is undefined ! */
                sprintf( net_diag_msg,"Gateway IP-address: %d.%d.%d.%d",
                                    ((UINT8*)&env_gateway)[0],
                                    ((UINT8*)&env_gateway)[1],
                                    ((UINT8*)&env_gateway)[2],
                                    ((UINT8*)&env_gateway)[3] );
                net_last_error = ERROR_NET_INVALID_GATEWAY_IP ;
                return( ERROR_NET_INVALID_GATEWAY_IP ) ;
            }

            /* We must be on same subnet as gateway */
            if ( (env_subnetmask & env_ipaddr) != (env_subnetmask & env_gateway) )
            {
                sprintf( net_diag_msg,"ip: %d.%d.%d.%d, gw: %d.%d.%d.%d, sub: %d.%d.%d.%d",
                                    ((UINT8*)&env_ipaddr)[0],
                                    ((UINT8*)&env_ipaddr)[1],
                                    ((UINT8*)&env_ipaddr)[2],
                                    ((UINT8*)&env_ipaddr)[3],
                                    ((UINT8*)&env_gateway)[0],
                                    ((UINT8*)&env_gateway)[1],
                                    ((UINT8*)&env_gateway)[2],
                                    ((UINT8*)&env_gateway)[3],
                                    ((UINT8*)&env_subnetmask)[0],
                                    ((UINT8*)&env_subnetmask)[1],
                                    ((UINT8*)&env_subnetmask)[2],
                                    ((UINT8*)&env_subnetmask)[3] );
                net_last_error = ERROR_NET_NOT_ON_SAME_SUBNET_IP ;
                return( ERROR_NET_NOT_ON_SAME_SUBNET_IP ) ;
            }

            /* We need to resolute the MAC-address of gateway */
            IF_ERROR( (rcode),
                      (NET_ARP_open( env_gateway,
                                    &mac_addr) ) )
    
        }
        else
        {
            /* On same subnet, we need to resolute the MAC-address of destination */
            IF_ERROR( (rcode),
                      (NET_ARP_open( dst_adr,
                                    &mac_addr) ) )
        }
    }
    

    /* Fill-in IP header of frame */
    p = (data + IP_HEADER_BASE) ;

    /* Fill-in version+header length  */
    put1( IP_HEADER_VERSION_LENGTH_IPV4, p ) ;

    /* Fill-in 'type-of-service'      */
    put1( IP_HEADER_TYPE_OF_SERVICE_NORMAL, p ) ;

    /* Fill-in 'total-length'         */
    tmp2 = length - MAC_HEADER_SIZE ;
    putbe2( tmp2, p ) ;

    /* Fill-in 'identification'       */
    putbe2( NET_IP_identification++, p ) ;

    /* Fill-in 'fragment' (not used)  */
    putbe2( IP_HEADER_FRAGMENT_NO, p ) ;

    /* Fill-in 'time-to-live'         */
    put1( IP_HEADER_TIME_TO_LIVE_NORMAL, p ) ;

    /* Fill-in 'protocol'             */
    put1( sap_context[ip_sp_hd].ip_sap, p ) ;

    /* Fill-in 'checksum' (temp = 0)  */
    put2( 0, p ) ;

    /* Fill-in 'source-IP-adr'        */
    put4( env_ipaddr, p ) ;

    /* Fill-in 'destination-IP-adr'   */
    put4( dst_adr, p ) ;

    /* Fill-in calculated 'checksum'  */
    tmp2 = NET_checksum( (UINT16*)((UINT32)data + IP_HEADER_BASE), IP_HEADER_SIZE/2 ) ;
    p = (data + IP_HEADER_BASE + IP_HEADER_CHECKSUM) ;
    put2( tmp2, p ) ;

    /* request MAC to send this frame */
    if (dst_mac_adr == NULL)
    {
        /* use resoluted MAC-address */
        rcode = NET_MAC_send( mac_sp_hd,
                              &mac_addr,
                              length,
                              data ) ;
    }
    else
    {
        /* use callers supplied MAC-address */
        rcode = NET_MAC_send( mac_sp_hd,
                              dst_mac_adr,
                              length,
                              data ) ;
    }

    /* return completion */
    return( rcode ) ;
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_IP_receive
 *  Description :
 *  -------------
 *  This function is registered in the MAC-module and linked with the
 *  MAC-type = '0x800', to let the MAC call us back with an reference
 *  to the received frame, containing an IP-frame.
 *  In this function the IP-header will be validated and the IP-SAP
 *  table will be searched for any match between the actual value
 *  of the IP-protocol field and a registered user to handle upper
 *  protocol layer.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_mac_adr': senders MAC-address
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
UINT32 NET_IP_receive( t_mac_addr *src_mac_adr,
                       UINT32     length,
                       UINT8      *data    )
{
    UINT32            rcode, src_ip_addr, dst_ip_addr;
    t_ip_usr_receive  receive ;
    UINT8             *p ;
    UINT8             sap ;
    UINT16            checksum, totlen ;
    UINT16            tmp2 ;
    int               i ;

#ifdef NET_DEBUG
    printf("NET_IP_receive\n") ;
#endif

    /* validate checksum */
    p = (data + IP_HEADER_BASE + IP_HEADER_CHECKSUM) ;
    get2( p, checksum) ;
    p = (data + IP_HEADER_BASE + IP_HEADER_CHECKSUM) ;
    put2( 0, p ) ;
    tmp2 = NET_checksum( (UINT16*)((UINT32)data + IP_HEADER_BASE), IP_HEADER_SIZE/2 ) ;
    if ( tmp2 != checksum )
    {
        return( ERROR_NET_IP_INVALID_CHECKSUM ) ;
    }

    /* Having verified checksum, let this frame be untouched */
    p = (data + IP_HEADER_BASE + IP_HEADER_CHECKSUM) ;
    put2( checksum, p ) ;

    /* get IP-'total_length'-field of this received frame and adjust length */
    p = (data + IP_HEADER_BASE + IP_HEADER_TOTAL_LENGTH) ;
    get2( p, totlen) ;
    length = BE16_TO_CPU(totlen) + MAC_HEADER_SIZE ;

    /* validate length */
    if (length > MAC_MAX_FRAME_SIZE)
    {
        return( ERROR_NET_IP_INVALID_LENGTH ) ;
    }

    /* get IP-'protocol'-field of this received frame */
    p = (data + IP_HEADER_BASE + IP_HEADER_PROTOCOL) ;
    get1( p, sap) ;

    /* get source IP address of this received frame */
    p = (data + IP_HEADER_BASE + IP_HEADER_SOURCE_IP_ADDRESS) ;
    get4( p, src_ip_addr) ;

    /* get destination IP address of this received frame, and validate
       destination is our address */
    get4( p, dst_ip_addr) ;
    if (env_ipaddr != dst_ip_addr)
    {
        return( ERROR_NET_IP_INVALID_DST_IP_ADDR ) ;
    }

    /* search the SAP table to match this 'sap' */
    for ( i = 0; i <IP_SAP_COUNT ; i++ )
    {
        if ( sap_context[i].ip_sap == sap )
        {
            if ( sap_context[i].ip_sap_state == IP_SAP_STATE_OPEN )
            {
                /* we have a registered SAP context, call user */
                receive    = sap_context[i].ip_usr_receive ;
                rcode = (*receive)( src_ip_addr,
                                    src_mac_adr,
                                    length,
                                    data ) ;
                return( rcode ) ;
            }
        }
    }

    /* No SAP, reply 'PROTOCOL is UNREACHABLE' */
    p = (data + IP_HEADER_BASE) ;
    rcode = NET_ICMP_UNREACHABLE_send( src_ip_addr,
                                       src_mac_adr,
                                       p,
                                       ICMP_UNREACHABLE_HEADER_CODE_PROT_UNREACHABLE ) ;

    return( ERROR_NET_IP_NO_SAP ) ;
}


/************************************************************************
 *
 *                          NET_IP_alloc_sap
 *  Description :
 *  -------------
 *  Allocate a IP SAP, register user context and return a sp-handle.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'sap_id',             IN,    value of IP-'protocol' to bind for
 *  'ip_usr_receive',     IN,    user-receive function to be registered
 *  'ip_sp_hd',           OUT,   handle of IP to be used by user by call
 *                               of 'send' or 'close'
 *
 *
 *  Return values :
 *  ---------------
 * 'ERROR_NET_IP_NO_FREE_SAP', No SAP entry could be allocated
 * 'OK' = 0x00:
 *
 *
 ************************************************************************/
static
UINT32 NET_IP_alloc_sap( UINT8            sap_id,
                         t_ip_usr_receive ip_usr_receive,
                         UINT32           *ip_sp_hd )
{
    int i ;

    /* allocate a free IP SAP table entry */
    for ( i = 0; i <IP_SAP_COUNT ; i++)
    {
        if ( sap_context[i].ip_sap_state == IP_SAP_STATE_CLOSED )
        {
            /* save SAP context in allocated entry */
            sap_context[i].ip_sap_state   = IP_SAP_STATE_OPEN ;
            sap_context[i].ip_sap         = sap_id ;
            sap_context[i].ip_usr_receive = ip_usr_receive ;
            *ip_sp_hd                     = i ;
            return( OK ) ;
        }
    }

    /* we should not arrive here */
    return( ERROR_NET_IP_NO_FREE_SAP ) ;
}


