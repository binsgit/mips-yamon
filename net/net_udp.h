#ifndef NET_UDP_H
#define NET_UDP_H

/************************************************************************
 *
 *      NET_UDP.h
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
 *    Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Parameter definitions
*************************************************************************/

/* UDP header definitions */
#define UDP_HEADER_SIZE     8     /* total UDP ECHO header size  */
#define UDP_HEADER_BASE     34    /* header start address 
                                           in ethernet frame            */

/* UDP header: Relative offset addresses for each protocol element*/
#define UDP_HEADER_SOURCE_PORT              0
#define UDP_HEADER_DESTINATION_PORT         2
#define UDP_HEADER_LENGTH                   4
#define UDP_HEADER_CHECKSUM                 6
#define UDP_HEADER_DATA                     8
#define UDP_HEADER_DATA_MAX_SIZE            1472

/* UDP pseudo header: Relative offset addresses for each protocol element*/
#define UDP_PSEUDO_HEADER_BASE              22 
#define UDP_PSEUDO_HEADER_SIZE              12

#define UDP_PSEUDO_HEADER_SOURCE_IPADR      0
#define UDP_PSEUDO_HEADER_DESTINATION_IPADR 4
#define UDP_PSEUDO_HEADER_ZERO_BYTE         8
#define UDP_PSEUDO_HEADER_PROTOCOL          9
#define UDP_PSEUDO_HEADER_LENGTH            10


/* Reserved service access points: (value of 'PORT') */
#define UDP_SAP_TFTP         69             /* TFTP server   */
#define UDP_SAP_RESERVED_UPPER 1023         /* Port numbers reserved 
                                               between 0-1023 */
#define UDP_SAP_UNDEFINED    0

#define UDP_SAP_COUNT        4    /* Maximum number of UDP SAP's        */

#define UDP_SAP_STATE_CLOSED 0x42 /* SAP is closed for service          */
#define UDP_SAP_STATE_OPEN   0x43 /* SAP is opened for service          */

#define UDP_STATE_CLOSED     0x44 /* UDP is closed for service        */
#define UDP_STATE_OPEN       0x46 /* UDP bound to IP                  */

/* prototype for user defined receive handler for a UDP SAP */
typedef UINT32 (*t_udp_usr_receive)( UINT32     src_ip_adr,  /* IP BE-format   */
                                     UINT16     src_port,    /* port BE-format */
                                     t_mac_addr *src_mac_adr,/* MAC            */
                                     UINT32     udp_sp_hd,   /* handle of open */
                                     UINT32     length,      /* total length   */
                                     UINT8      *data     ); /* start of frame */

typedef struct udp_sap_context
{
    UINT16            sap_state ;   /* state of this SAP            */
    UINT16            sap ;         /* port BE-format               */
    t_udp_usr_receive usr_receive ; /* user defined receive handler,
                                      linked to this SAP            */
} t_udp_sap_context ;


/************************************************************************
 *  NET_UDP services: 
*************************************************************************/

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
UINT32 NET_UDP_init( void ) ;


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
                     UINT32            *sp_hd ) ;


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
UINT32 NET_UDP_close( UINT32 sp_hd ) ;


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
                     UINT8      *data  ); /* pointer to start of frame   */


#endif /* #ifndef NET_UDP_H */
