/************************************************************************
 *
 *      NET_UDP.c
 *
 *      The 'NET_UDP' module implements the UDP layer of the IP-protocols
 *      and and it provides services to send and receive
 *      UDP datagrams via service access points, which are linked
 *      with the 'port'-field of the UDP-header.
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
#include "net_udp.h"


/************************************************************************
 *      Definitions
 ************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

/* Global state of UDP */
static UINT32 NET_UDP_state = UDP_STATE_CLOSED ;

/* The IP SP-handle, returned by 'IP-open' */
static UINT32 ip_sp_hd ;

/* UDP SAP table */
static t_udp_sap_context sap_context[UDP_SAP_COUNT] ;

/* UDP SAP allocation variable, keeping a simple counter
   to be used for allocating unique 'port' numbers
   in UDP-'open'                                      */
static UINT16 udp_sap_next ;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/

/************************************************************************
 *
 *                          NET_UDP_receive
 *  Description :
 *  -------------
 *  This function is registered in the IP-module and linked with the
 *  IP-protocol = '17', to let the IP call us back with a reference
 *  to the received frame, containing an UDP-frame.
 *  In this function the UDP-header will be validated and
 *  the any opened UDP-SAP will be checked to call a user registered
 *  receive handler.
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
UINT32 NET_UDP_receive( UINT32     src_ip_adr,
                        t_mac_addr *src_mac_adr,
                        UINT32     length,
                        UINT8      *data    ) ;




/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_UDP_init
 *  Description :
 *  -------------
 *  Initialize the UDP module.
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
UINT32 NET_UDP_init(  void )
{
    UINT32 rcode ;
    int    i ;

    /* initialize UDP SAP table */
    for ( i = 0; i <IP_SAP_COUNT ; i++)
    {
        sap_context[i].sap_state   = UDP_SAP_STATE_CLOSED ;
        sap_context[i].sap         = UDP_SAP_UNDEFINED ;
        sap_context[i].usr_receive = NULL ;
    }

    /* first unique SAP to be used at 'open' */
    udp_sap_next = UDP_SAP_RESERVED_UPPER + 1 ;

    /* register our receive handler */
    IF_ERROR( (rcode),
              (NET_IP_open( IP_SAP_UDP,
                            NET_UDP_receive,
                            &ip_sp_hd) ) )

    /* UDP-module has now been initialized and opened for service */
    NET_UDP_state = UDP_STATE_OPEN ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_UDP_open
 *  Description :
 *  -------------
 *  Allocate a UDP-SAP and register user context.
 *
 *
 *  Parameters :
 *  ------------
 *  'sap_id',          IN,    value of UDP-'port' (our) to bind for.
 *                            If the value is 'undefined'(=0), the
 *                            UDP-module allocates a unique port.
 *  'usr_receive',     IN,    user-receive function to be registered
 *  'sp_hd',           OUT,   handle of UDP to be used by user by call
 *                            of 'send' or 'close'
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_UDP_FATAL_STATE'      A fatal state has been detected in UDP.
 *  'ERROR_NET_UDP_NOT_INITIALIZED'  UDP-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_UDP_open( UINT16            sap_id,     /* port in BE-format */
                     t_udp_usr_receive usr_receive,
                     UINT32            *sp_hd )
{
    UINT32  rcode = OK ;
    int     i ;

    switch (NET_UDP_state)
    {
        case UDP_STATE_CLOSED:
                rcode = ERROR_NET_UDP_NOT_INITIALIZED ;
            break;

        case UDP_STATE_OPEN:
                /* Validate or find unique sap */
                if ( sap_id == UDP_SAP_UNDEFINED )
                {
                    sap_id = CPU_TO_BE16( udp_sap_next ) ;
                    udp_sap_next++ ;
                    if (udp_sap_next == 0) 
                    {
                        udp_sap_next = UDP_SAP_RESERVED_UPPER + 1 ;
                    }
                }
                for ( i = 0; i <UDP_SAP_COUNT ; i++)
                {
                    if ( (sap_context[i].sap_state == UDP_STATE_OPEN) &&
                         (sap_context[i].sap       == sap_id) )
                    {
                        /* we have found an unuseable SAP */
                        return( ERROR_NET_UDP_SAP_INUSE ) ;
                    }
                }

                /* allocate a free UDP SAP table entry */
                for ( i = 0; i <UDP_SAP_COUNT ; i++)
                {
                    if ( sap_context[i].sap_state == UDP_SAP_STATE_CLOSED )
                    {
                        /* save SAP context in allocated entry */
                        sap_context[i].sap_state   = UDP_SAP_STATE_OPEN ;
                        sap_context[i].sap         = sap_id ;
                        sap_context[i].usr_receive = usr_receive ;
                        *sp_hd                     = i ;
                        return( OK ) ;
                    }
                }
                rcode = ERROR_NET_UDP_NO_FREE_SAP ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_UDP_FATAL_STATE ;
            break;

    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_UDP_close
 *  Description :
 *  -------------
 *  Close  UDP-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                UDP SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_UDP_close( UINT32 sp_hd )
{
    /* validate handle */
    IF_UPPER( ERROR_NET_UDP_INVALID_HANDLE, sp_hd, UDP_SAP_COUNT)

    /* close SAP */
    sap_context[sp_hd].sap_state   = UDP_SAP_STATE_CLOSED ;
    sap_context[sp_hd].sap         = UDP_SAP_UNDEFINED ;
    sap_context[sp_hd].usr_receive = NULL ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_UDP_send
 *  Description :
 *  -------------
 *  Request the UDP module to send a 'UDP' datagram
 *  to a specified destination IP-address and UDP-port.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',           IN,   destination ip address (BE-format)
 *  'port',             IN,   destination udp port (BE-format)
 *  'mac_adr',          IN,   optional MAC address
 *  'sp_hd',            IN,   handle to select our SAP-context
 *  'length',           IN,   length of frame to send
 *  'data',             IN,   address to start of frame to be send
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0)
 *
 ************************************************************************/
UINT32 NET_UDP_send( UINT32     ip_adr,   /* destination ip address (BE) */
                     UINT16     port,     /* port (BE-format)            */
                     t_mac_addr *mac_adr, /* optional MAC destination    */
                     UINT32     sp_hd,    /* handle returned of 'open'   */
                     UINT32     length,   /* total length of frame to send */
                     UINT8      *data  )  /* pointer to start of frame   */
{
    UINT32     rcode ;
    UINT8      *p ;
    UINT16     tmp2 ;


    /* validate handle */
    IF_UPPER( ERROR_NET_UDP_INVALID_HANDLE, sp_hd, UDP_SAP_COUNT)

    /* Fill-in UDP header of frame */
    p = &data[UDP_HEADER_BASE + UDP_HEADER_SOURCE_PORT]  ;

    tmp2 = sap_context[sp_hd].sap ;
    put2( tmp2, p ) ;               /* Fill-in 'source_port'          */
    put2( port, p ) ;               /* Fill-in 'destination_port'     */

    tmp2 = length - (IP_HEADER_SIZE + MAC_HEADER_SIZE) ;
    putbe2( tmp2, p ) ;             /* Fill-in 'length'               */

    put2( 0, p ) ;                  /* Fill-in 'checksum' (temp = 0)  */

    /* Calculate UDP checksum, which include the pseudo header */

    /* Fill-in the pseudo-header to prepare for checksum calculation */
    p = &data[UDP_PSEUDO_HEADER_BASE] ;
    put4( env_ipaddr, p ) ;         /* Fill-in 'source-ip'           */
    put4( ip_adr, p ) ;             /* Fill-in 'dest-ip'             */
    put1( 0, p ) ;                  /* Fill-in 'zero'                */
    put1( IP_SAP_UDP , p ) ;        /* Fill-in 'protocol'            */
    tmp2 = length - (IP_HEADER_SIZE + MAC_HEADER_SIZE) ;
    putbe2( tmp2, p ) ;             /* Fill-in 'length'              */

    /* complete UDP-header with checksum */
    tmp2 = (UDP_PSEUDO_HEADER_SIZE + (length - (IP_HEADER_SIZE + MAC_HEADER_SIZE)))/2 ;
    if (length%2)
    {
        data[length] = 0 ;
        tmp2 = tmp2 + 1 ;
    }

    p = &data[UDP_PSEUDO_HEADER_BASE] ;
    tmp2 = NET_checksum( (UINT16*)p, tmp2 ) ;
    if ( tmp2 == 0 )
    {
        tmp2 = ~tmp2 ;
    }
    p = (data + UDP_HEADER_BASE + UDP_HEADER_CHECKSUM) ;
    put2( tmp2, p ) ;               /* Fill-in calculated 'checksum'  */

    /* request IP to send this frame */
    rcode = NET_IP_send( ip_sp_hd,
                         ip_adr,  
                         mac_adr,
                         length,
                         data ) ;

    /* return completion */
    return( rcode ) ;
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_UDP_receive
 *  Description :
 *  -------------
 *  This function is registered in the IP-module and linked with the
 *  IP-protocol = '17', to let the IP call us back with a reference
 *  to the received frame, containing an UDP-frame.
 *  In this function the UDP-header will be validated and
 *  the any opened UDP-SAP will be checked to call a user registered
 *  receive handler.
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
UINT32 NET_UDP_receive( UINT32     src_ip_adr,
                        t_mac_addr *src_mac_adr,
                        UINT32     length,
                        UINT8      *data    )
{
    UINT8             ip_header[IP_HEADER_SIZE+UDP_HEADER_SIZE] ;
    UINT32            rcode ;
    UINT8             *p ;
    UINT16            tmp2, checksum, src_port, dst_port, udp_length ;
    t_udp_usr_receive receive ;
    int               i ;

#ifdef NET_DEBUG
    printf("NET_UDP_receive:\n") ;
#endif

    /* save ip_header to use, if port is unreachable */
    p = (data + IP_HEADER_BASE) ;
    memcpy( ip_header, p, IP_HEADER_SIZE+UDP_HEADER_SIZE) ;

    /* validate UDP checksum */
    p = (data + UDP_HEADER_BASE + UDP_HEADER_CHECKSUM) ;
    get2( p, checksum) ;

    /* Do checksum verification when requested */
    if (checksum != 0)
    {
        p = (data + UDP_HEADER_BASE + UDP_HEADER_CHECKSUM) ;
        put2( 0, p ) ;

        /* prepend pseudo header */
        /* Fill-in the pseudo-header to prepare for checksum calculation */
        p = &data[UDP_PSEUDO_HEADER_BASE] ;
        put4( src_ip_adr, p ) ;       /* Fill-in 'source-ip'           */
        put4( env_ipaddr, p ) ;       /* Fill-in 'dest-ip'             */
        put1( 0 , p ) ;               /* Fill-in 'zero'                */
        put1( IP_SAP_UDP , p ) ;      /* Fill-in 'protocol'            */
        tmp2 = length - (IP_HEADER_SIZE + MAC_HEADER_SIZE) ;
        putbe2( tmp2, p ) ;           /* Fill-in 'length'              */

        tmp2 = (UDP_PSEUDO_HEADER_SIZE + (length - (IP_HEADER_SIZE + MAC_HEADER_SIZE)))/2 ;
        if (length%2)
        {
            data[length] = 0 ;
            tmp2 = tmp2 + 1 ;
        }

        p = &data[UDP_PSEUDO_HEADER_BASE] ;
        tmp2 = NET_checksum( (UINT16*)p, tmp2 ) ;
        if (tmp2 == 0)
        {
            tmp2 = ~tmp2 ;
        }
        if ( tmp2 != checksum )
        {
            return( ERROR_NET_UDP_INVALID_CHECKSUM ) ;
        }
    }

    /* validate UDP length */
    p = (data + UDP_HEADER_BASE + UDP_HEADER_LENGTH) ;
    get2( p, udp_length) ;
    udp_length = BE16_TO_CPU( udp_length ) ;
    if (udp_length != (length - (IP_HEADER_SIZE + MAC_HEADER_SIZE)) )
    {
        return( ERROR_NET_UDP_INVALID_LENGTH) ;
    }

    /* get UDP-'port'-field's of this received frame */
    p = (data + UDP_HEADER_BASE) ;
    get2( p, src_port) ;
    get2( p, dst_port) ;

    /* search SAP table to match any registered user */
    for ( i = 0; i <UDP_SAP_COUNT ; i++ )
    {
        if ( sap_context[i].sap == dst_port )
        {
            if ( sap_context[i].sap_state == UDP_SAP_STATE_OPEN )
            {
                /* we have a registered SAP context, call user */
                receive    = sap_context[i].usr_receive ;
                rcode = (*receive)( src_ip_adr,
                                    src_port,
                                    src_mac_adr,
                                    i,
                                    length,
                                    data ) ;
#ifdef NET_DEBUG
                if (rcode) printf("UDP-receive, rcode = %x\n", rcode ) ;
#endif
                return( rcode ) ;
            }
        }
    }
    /* No SAP, reply 'PORT is UNREACHABLE' */
    rcode = NET_ICMP_UNREACHABLE_send( src_ip_adr,
                                       NULL,
                                       ip_header,
                                       ICMP_UNREACHABLE_HEADER_CODE_PORT_UNREACHABLE ) ;
    return( ERROR_NET_UDP_NO_SAP ) ;
}
