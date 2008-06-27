#ifndef NET_ARP_H
#define NET_ARP_H

/************************************************************************
 *
 *      NET_ARP.h
 *
 *      The 'NET_ARP' module implements the ARP protocol and ARP
 *      services to provide access to the ARP cache, which contains
 *      entries of set of IP and MAC addresses. Only IP-addresses
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
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <syscon_api.h>



/************************************************************************
 *  Parameter definitions
*************************************************************************/

/* ARP header definitions */
#define ARP_HEADER_SIZE     28    /* total ARP header size                  */
#define ARP_HEADER_BASE     14    /* header start address in ethernet frame */

/* ARP header: Relative offset addresses for each protocol element     */
#define ARP_HEADER_MEDIA_TYPE                   0
#define ARP_HEADER_PROTOCOL_TYPE                2
#define ARP_HEADER_MEDIA_ADDRESS_SIZE           4
#define ARP_HEADER_PROTOCOL_ADDRESS_SIZE        5
#define ARP_HEADER_OPERATION_CODE               6
#define ARP_HEADER_SOURCE_MEDIA_ADDRESS         8
#define ARP_HEADER_SOURCE_PROTOCOL_ADDRESS      14
#define ARP_HEADER_DESTINATION_MEDIA_ADDRESS    18
#define ARP_HEADER_DESTINATION_PROTOCOL_ADDRESS 24

/* ARP header: Values for IP, Ethernet ARP-application */
#define ARP_HEADER_MEDIA_TYPE_ETHERNET          0x0001
#define ARP_HEADER_PROTOCOL_TYPE_IP             0x0800
#define ARP_HEADER_MEDIA_ADDRESS_SIZE_ETHERNET  0x06
#define ARP_HEADER_PROTOCOL_ADDRESS_SIZE_IP     0x04
#define ARP_HEADER_OPERATION_CODE_REQUEST       0x0001
#define ARP_HEADER_OPERATION_CODE_RESPONSE      0x0002

#define ARP_SAP_COUNT           10   /* Maximum number of ARP SAP's, 
                                        i.e. cache size                    */

#define ARP_SAP_STATE_CLOSED    0x41 /* Entry is free for allocation       */
#define ARP_SAP_STATE_ALLOCATED 0x42 /* Entry has been allocated, 
                                        and awaits response with MAC adr   */
#define ARP_SAP_STATE_RESOLUTED 0x43 /* Entry is available with cached 
                                        set of (IP,MAC) addresses          */
#define ARP_SAP_TIMEOUT         30   /* Maximum number of seconds an ARP 
                                        SAP entry may stay unused in cache */
#define ARP_POLL_TIMEOUT        5    /* Seconds between timer management
                                        of ARP cache entries               */

#define ARP_STATE_CLOSED        0x44 /* ARP is closed for service          */
#define ARP_STATE_OPEN          0x45 /* ARP 'init' called:
                                         i.e. ARP bound to MAC layer       */


typedef struct arp_sap_context
{
    UINT16            state ;           /* state of this SAP            */
    UINT32            ip_addr ;         /* IP-address of 
                                           this SAP (BE-format)         */
    UINT32            timeout ;         /* time to remove this SAP 
                                           from cache                   */
    t_mac_addr        mac_addr ;        /* MAC-address of this SAP      */
} t_arp_sap_context ;


/************************************************************************
 *  NET_ARP services: 
*************************************************************************/

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
 *  'poll'    pointer for system poll function, to be used during 'open'
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
UINT32 NET_ARP_init( UINT32 (*poll)(void) ) ;


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
                     t_mac_addr *mac_addr ) ;


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
UINT32 NET_ARP_poll( void ) ;


#endif /* #ifndef NET_ARP_H */
