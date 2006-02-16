#ifndef NET_IP_H
#define NET_IP_H

/************************************************************************
 *
 *      NET_IP.h
 *
 *      The 'NET_IP' module implements the IP-layer services to provide
 *      service access points, which are linked with the 'protocol'-field
 *      of the IP-header.
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
 *    Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Parameter definitions
*************************************************************************/


#define IP_ADDR_UNDEFINED 0x00000000

/* IP header definitions */
#define IP_HEADER_SIZE     20    /* total IP header size                   */
#define IP_HEADER_BASE     14    /* header start address in ethernet frame */

/* IP header: Relative offset addresses for each protocol element     */
#define IP_HEADER_VERSION_LENGTH           0
#define IP_HEADER_TYPE_OF_SERVICE          1
#define IP_HEADER_TOTAL_LENGTH             2
#define IP_HEADER_IDENTIFICATION           4
#define IP_HEADER_FRAGMENT                 6
#define IP_HEADER_TIME_TO_LIVE             8
#define IP_HEADER_PROTOCOL                 9
#define IP_HEADER_CHECKSUM                 10
#define IP_HEADER_SOURCE_IP_ADDRESS        12
#define IP_HEADER_DESTINATION_IP_ADDRESS   16

/* IP header: Protocol element values */
#define IP_HEADER_VERSION_LENGTH_IPV4      0x45
#define IP_HEADER_TYPE_OF_SERVICE_NORMAL   0x00
#define IP_HEADER_FRAGMENT_NO              0x4000
#define IP_HEADER_TIME_TO_LIVE_NORMAL      0x40


/* Reserved service access points: (value of 'IP_HEADER_PROTOCOL') */
#define IP_SAP_ICMP         1   /* ICMP protocol */
#define IP_SAP_UDP          17  /* UDP  protocol */
#define IP_SAP_TCP          6   /* TCP  protocol */
#define IP_SAP_UNDEFINED    255

#define IP_SAP_COUNT        2    /* Maximum number of IP SAP's        */

#define IP_SAP_STATE_CLOSED 0x42 /* SAP is closed for service          */
#define IP_SAP_STATE_OPEN   0x43 /* SAP is opened for service          */

#define IP_STATE_CLOSED     0x44 /* IP is closed for service           */
#define IP_STATE_INITED     0x45 /* IP 'init' has been called          */
#define IP_STATE_OPEN       0x46 /* IP 'open' called min. one time:
                                         i.e. IP bound to MAC          */

/* prototype for user defined receive handler for a IP SAP */
typedef UINT32 (*t_ip_usr_receive)( UINT32     src_ip_adr,
                                    t_mac_addr *src_mac_adr,
                                    UINT32     length,
                                    UINT8      *data      ) ;

typedef struct ip_sap_context
{
    UINT16           ip_sap_state ;   /* state of this SAP            */
    UINT8            ip_sap ;         /* SAP: 'protocol'              */
    t_ip_usr_receive ip_usr_receive ; /* user defined receive handler,
                                           linked to this SAP         */
} t_ip_sap_context ;


/************************************************************************
 *  NET_IP services: 
*************************************************************************/

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
UINT32 NET_IP_init( void ) ;


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
                    UINT32           *ip_sp_hd ) ;


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
UINT32 NET_IP_close( UINT32  ip_sp_hd ) ;


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
 *  'dst_mac_adr',        IN,   destination MAC address (may be undefined)
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
                    UINT8       *data  ); /* pointer to start of frame       */

#endif /* #ifndef NET_IP_H */
