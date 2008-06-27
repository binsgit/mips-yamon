#ifndef NET_MAC_H
#define NET_MAC_H

/************************************************************************
 *
 *      NET_MAC.h
 *
 *      The 'NET_MAC' module implements the MAC-layer services to provide
 *      service access points, which are linked with the 'type'-field
 *      of the MAC-header.
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

/* Reserved service access points: */
#define MAC_SAP_ARP         0x806 /* ARP protocol */
#define MAC_SAP_IP          0x800 /* IP  protocol */
#define MAC_SAP_UNDEFINED   0

/* MAC header definitions */
#define MAC_MAX_FRAME_SIZE  6+6+2+1500 /* Maximum ethernet frame size       */
#define MAC_HEADER_SIZE     14    /* total MAC header size                  */
#define MAC_HEADER_BASE     0     /* header start address in ethernet frame */

/* MAC header: Relative offset addresses for each protocol element     */
#define MAC_HEADER_DSTADR   0     /* destination address   */
#define MAC_HEADER_SRCADR   6     /* source address        */
#define MAC_HEADER_TYPE     12    /* type                  */


#define MAC_SAP_COUNT        3    /* Maximum number of MAC SAP's        */

#define MAC_SAP_STATE_CLOSED 0x42 /* SAP is closed for service          */
#define MAC_SAP_STATE_OPEN   0x43 /* SAP is opened for service          */

#define MAC_STATE_CLOSED     0x44 /* MAC is closed for service          */
#define MAC_STATE_INITED     0x45 /* MAC 'init' has been called         */
#define MAC_STATE_OPEN       0x46 /* MAC 'open' called min. one time:
                                         i.e. MAC bound to LAN driver   */

/* prototype for user defined receive handler for a MAC SAP */
typedef UINT32 (*t_mac_usr_receive)( t_mac_addr *src_adr,
                                     UINT32     length, 
                                     UINT8      *data      ) ;

typedef struct mac_sap_context
{
    UINT16            mac_sap_state ;   /* state of this SAP            */
    UINT16            mac_sap ;         /* SAP: 'type',
                                           in network byte order (BE)   */
    t_mac_usr_receive mac_usr_receive ; /* user defined receive handler,
                                           linked to this SAP           */
} t_mac_sap_context ;

/************************************************************************
 *  NET_MAC public variables: 
*************************************************************************/

/* MAC undefined address */
extern       t_mac_addr mac_undefined_adr ; 

/* MAC broadcast address */
extern       t_mac_addr mac_broadcast_adr ;

/* Our stations MAC-address */
extern       t_mac_addr mac_station_adr ;


/************************************************************************
 *  NET_MAC services: 
*************************************************************************/


/************************************************************************
 *
 *                          NET_MAC_init
 *  Description :
 *  -------------
 *  Initialize the MAC module.
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
UINT32 NET_MAC_init(  void ) ;


/************************************************************************
 *
 *                          NET_MAC_open
 *  Description :
 *  -------------
 *  Allocate a MAC-SAP and register user context.
 *
 *
 *  Parameters :
 *  ------------
 *  'mac_sap_id',          IN,    value of MAC-'type' to bind for
 *  'mac_usr_receive',     IN,    user-receive function to be registered
 *  'mac_sp_hd',           OUT,   handle of MAC to be used by user by call
 *                                of 'send'
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_MAC_FATAL_STATE'      A fatal state has been detected in MAC.
 *  'ERROR_NET_MAC_NOT_INITIALIZED'  MAC-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_MAC_open(  UINT16            mac_sap_id,      /* 'type'-field of mac-header 
                                                            always in 'cpu' format          */
                      t_mac_usr_receive mac_usr_receive, /* user defined receive handler    */ 
                      UINT32            *mac_sp_hd ) ;   /* service provider defined handle */


/************************************************************************
 *
 *                          NET_MAC_close
 *  Description :
 *  -------------
 *  Close a MAC-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *  'mac_sp_hd'     service provider defined handle
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_MAC_INVALID_HANDLE', invalid handle
 *  'OK'(=0),                       SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_MAC_close( UINT32            mac_sp_hd ) ;    /* service provider defined handle */


/************************************************************************
 *
 *                          NET_MAC_send
 *  Description :
 *  -------------
 *  Request the MAC module to send a frame, linked
 *  to a certain 'SAP' to a specified MAC-destination address.
 *
 *
 *  Parameters :
 *  ------------
 *  'mac_sp_hd',           IN,   handle to lookup a registered SAP context
 *  'dst_adr',             IN,   destination mac address
 *  'length',              IN,   length of frame to send
 *  'data',                IN,   address to start of frame to be send
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
UINT32 NET_MAC_send(  UINT32            mac_sp_hd,       /* service provider defined handle */
                      t_mac_addr        *dst_adr,        /* destination mac address         */
                      UINT32            length,          /* total length of frame to send   */
                      UINT8             *data  ) ;       /* pointer to start of frame       */


/************************************************************************
 *
 *                          NET_MAC_poll
 *  Description :
 *  -------------
 *
 *  Poll the MAC module to receive any frame from driver.
 *  (Note: is not used by ISR-handled reception.)
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'
 *
 ************************************************************************/
UINT32 NET_MAC_poll( void ) ;



#endif /* #ifndef NET_MAC_H */
