#ifndef NET_ICMP_H
#define NET_ICMP_H

/************************************************************************
 *
 *      NET_ICMP.h
 *
 *      The 'NET_ICMP' module currently implements the ECHO part of
 *      the ICMP layer. It provides services to send and receive
 *      ECHO events.
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


/* ICMP unreachable header definitions */
#define ICMP_UNREACHABLE_HEADER_SIZE     8     /* total ICMP UNREACHABLE header size  */
#define ICMP_UNREACHABLE_DATA_SIZE       28    /* total ICMP UNREACHABLE header size  */
#define ICMP_UNREACHABLE_HEADER_BASE     34    /* header start address
                                           in ethernet frame            */

/* ICMP UNREACHABLE header: Relative offset addresses for each protocol element*/
#define ICMP_UNREACHABLE_HEADER_TYPE                     0
#define ICMP_UNREACHABLE_HEADER_CODE                     1
#define ICMP_UNREACHABLE_HEADER_CHECKSUM                 2
#define ICMP_UNREACHABLE_HEADER_RESERVED                 4
#define ICMP_UNREACHABLE_HEADER_DATA                     8

/* ICMP UNREACHABLE header: Protocol element values */
#define ICMP_UNREACHABLE_HEADER_TYPE_UNREACHABLE         3
#define ICMP_UNREACHABLE_HEADER_CODE_NETWORK_UNREACHABLE 0
#define ICMP_UNREACHABLE_HEADER_CODE_HOST_UNREACHABLE    1
#define ICMP_UNREACHABLE_HEADER_CODE_PROT_UNREACHABLE    2
#define ICMP_UNREACHABLE_HEADER_CODE_PORT_UNREACHABLE    3



/* ICMP ECHO header definitions */
#define ICMP_ECHO_HEADER_SIZE     8     /* total ICMP ECHO header size  */
#define ICMP_ECHO_HEADER_BASE     34    /* header start address 
                                           in ethernet frame            */

/* ICMP ECHO header: Relative offset addresses for each protocol element*/
#define ICMP_ECHO_HEADER_TYPE                     0
#define ICMP_ECHO_HEADER_CODE                     1
#define ICMP_ECHO_HEADER_CHECKSUM                 2
#define ICMP_ECHO_HEADER_IDENTIFICATION           4
#define ICMP_ECHO_HEADER_SEQUENCE                 6
#define ICMP_ECHO_HEADER_DATA                     8

/* ICMP ECHO header: Protocol element values */
#define ICMP_ECHO_HEADER_TYPE_ECHOREQUEST         8
#define ICMP_ECHO_HEADER_TYPE_ECHOREPLY           0
#define ICMP_ECHO_HEADER_CODE_ECHO                0
#define ICMP_ECHO_HEADER_IDENTIFICATION_MIPS      ( ('M'<<8) | ('I') )

#define ICMP_ECHO_HEADER_DATA_MAX_SIZE            1472

#define ICMP_STATE_CLOSED     0x44 /* ICMP is closed for service        */
#define ICMP_STATE_OPEN       0x46 /* ICMP bound to IP                  */

/* prototype for user defined receive handler for a ICMP ECHO SAP */
typedef UINT32 (*t_icmp_echo_usr_receive)( UINT32     src_ip_adr,  /* BE-format */
                                           t_mac_addr *src_mac_adr,/* MAC */
                                           UINT16     sequence,    /* CPU-format */
                                           UINT16     usr_length ) ;

typedef struct icmp_echo_sap_context
{
    t_icmp_echo_usr_receive usr_receive;/* user defined receive handler,
                                           linked to this SAP      */
} t_icmp_echo_sap_context ;


/************************************************************************
 *  NET_ICMP services: 
*************************************************************************/

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
UINT32 NET_ICMP_init( void ) ;


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
 *  'ERROR_NET_ICMP_FATAL_STATE'      A fatal state has been detected in IP.
 *  'ERROR_NET_ICMP_NOT_INITIALIZED'  IP-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_ICMP_ECHO_open( t_icmp_echo_usr_receive usr_receive ) ;


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
UINT32 NET_ICMP_ECHO_close( void ) ;


/************************************************************************
 *
 *                          NET_ICMP_ECHO_send
 *  Description :
 *  -------------
 *  Request the ICMP ECHO module to send a 'ECHO' datagram
 *  to a specified destination IP-address with a specified
 *  sequence number and length of ECHO user data.
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
                           UINT16     length ) ;


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
                           UINT8      code ) ;

#endif /* #ifndef NET_ICMP_H */
