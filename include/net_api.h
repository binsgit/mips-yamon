#ifndef NET_API_H
#define NET_API_H

/************************************************************************
 *
 *      NET_api.h
 *
 *      The 'NET_api' module defines the NET subsystem
 *      interface. 
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
#include <syserror.h>

/************************************************************************
 *   NET, ERROR completion codes
*************************************************************************/

#define ERROR_NET                      0x0000a000 /* for compile time check        */
	                              
#define ERROR_NET_ARP_TIME_OUT             0xa000 /* No ARP response               */
#define ERROR_NET_NO_RESOURCES             0xa001 /* No available buffers or links */
#define ERROR_NET_INVALID_HOST_IP          0xa002 /* Host IP-address is not valid  */
#define ERROR_NET_INVALID_BOARD_IP         0xa003 /* Board IP-address is not valid */
#define ERROR_NET_TFTP_ERROR               0xa004 /* TFTP time out or ERROR        */
#define ERROR_NET_TFTP_READ_ERROR          0xa005 /* TFTP READ-REQ ERROR           */
#define ERROR_NET_TFTP_READ_TIMEOUT_ERROR  0xa006 /* TFTP READ-REQ timeout ERROR   */
#define ERROR_NET_TFTP_WRITE_ERROR         0xa007 /* TFTP WRITE-REQ ERROR          */
#define ERROR_NET_TFTP_WRITE_TIMEOUT_ERROR 0xa008 /* TFTP WRITE-REQ timeout ERROR  */
#define ERROR_NET_TFTP_DATA_ERROR          0xa009 /* TFTP DATA ERROR               */
#define ERROR_NET_TFTP_DATA_TIMEOUT_ERROR  0xa00a /* TFTP DATA timeout ERROR       */
#define ERROR_NET_USER_BREAK               0xa00b /* User made ctrl C              */
#define ERROR_NET_INVALID_GATEWAY_IP       0xa00c /* Gateway IP-address is not set */
#define ERROR_NET_NOT_ON_SAME_SUBNET_IP    0xa00d /* Our IP & GW not on same subnet*/
#define ERROR_NET_UNKNOWN_GATEWAY_IP       0xa00e /* ARP timeout on Gateway IP     */
#define ERROR_NET_TFTP_FILE_TOO_BIG        0xa00f /* TFTP File bigger than 32 MB   */
#define ERROR_NET_INVALID_SUBNET_MASK      0xa010 /* Subnetmask is not set         */
#define ERROR_NET_PING_TIME_OUT            0xa011 /* No response from ping server  */

#define ERROR_NET_MAC_NOT_INITIALIZED      0xa012 /* MAC module has not been init  */
#define ERROR_NET_MAC_FATAL_STATE          0xa013 /* MAC module is in fatal state  */
#define ERROR_NET_MAC_INVALID_HANDLE       0xa014 /* MAC module detected inv. hand */
#define ERROR_NET_MAC_NO_FREE_SAP          0xa015 /* MAC module is out of SAP space*/
#define ERROR_NET_MAC_NO_SAP               0xa016 /* MAC module found no reg. SAP  */
#define ERROR_NET_MAC_SAP_CLOSED           0xa017 /* MAC module SAP closed         */
#define ERROR_NET_MAC_INVALID_LENGTH       0xa018 /* MAC module detected length error */

#define ERROR_NET_ARP_NOT_INITIALIZED      0xa019 /* ARP module has not been init  */
#define ERROR_NET_ARP_FATAL_STATE          0xa01a /* ARP module is in fatal state  */
#define ERROR_NET_ARP_INVALID_OPCODE       0xa01b /* ARP module detected inv. opcode */
#define ERROR_NET_ARP_INVALID_IP_ADDR      0xa01c /* ARP module detected inv. IPadr */
#define ERROR_NET_ARP_MAC_NOT_RESOLUTED    0xa01d /* ARP module not resoluted MACadr */

#define ERROR_NET_IP_NOT_INITIALIZED       0xa01e /* IP module has not been init   */
#define ERROR_NET_IP_FATAL_STATE           0xa01f /* IP module is in fatal state   */
#define ERROR_NET_IP_INVALID_HANDLE        0xa020 /* IP module detected inv. hand  */
#define ERROR_NET_IP_NO_FREE_SAP           0xa021 /* IP module is out of SAP space */
#define ERROR_NET_IP_NO_SAP                0xa022 /* IP module found no reg. SAP   */
#define ERROR_NET_IP_SAP_CLOSED            0xa023 /* IP module SAP closed          */
#define ERROR_NET_IP_INVALID_OWN_IP_ADDR   0xa024 /* IP module detected inv. own IP*/
#define ERROR_NET_IP_INVALID_DST_IP_ADDR   0xa025 /* IP module detected inv. dst IP*/
#define ERROR_NET_IP_INVALID_SUBNET        0xa026 /* IP module detected inv. subnet */
#define ERROR_NET_IP_INVALID_GATEWAY       0xa027 /* IP module detected inv. gw    */
#define ERROR_NET_IP_NO_PATH               0xa028 /* IP module, gateway and this 
                                                        station  not on same subnet */
#define ERROR_NET_IP_INVALID_CHECKSUM      0xa029 /* IP module detected checksum err */
#define ERROR_NET_IP_INVALID_LENGTH        0xa02a /* IP module detected length error */

#define ERROR_NET_ICMP_NOT_INITIALIZED     0xa02b /* ICMP module has not been init   */
#define ERROR_NET_ICMP_FATAL_STATE         0xa02c /* ICMP module is in fatal state   */
#define ERROR_NET_ICMP_INVALID_DATA_SIZE   0xa02d /* ICMP module detected data size
                                                          error          */
#define ERROR_NET_ICMP_INVALID_CHECKSUM    0xa02e /* ICMP module detected checksum err */
#define ERROR_NET_ICMP_INVALID_ID          0xa02f /* ICMP module detected id err   */
#define ERROR_NET_ICMP_NO_USER             0xa030 /* ICMP module detected no 
                                                          user for reply */
#define ERROR_NET_ICMP_INVALID_TYPE        0xa031 /* ICMP module detected type err */

#define ERROR_NET_UDP_NOT_INITIALIZED      0xa032 /* UDP module has not been init   */
#define ERROR_NET_UDP_FATAL_STATE          0xa033 /* UDP module is in fatal state   */
#define ERROR_NET_UDP_INVALID_HANDLE       0xa034 /* UDP module detected inv. hand  */
#define ERROR_NET_UDP_NO_FREE_SAP          0xa035 /* UDP module is out of SAP space */
#define ERROR_NET_UDP_INVALID_CHECKSUM     0xa036 /* UDP module detected checksum err */
#define ERROR_NET_UDP_SAP_INUSE            0xa037 /* UDP module is out of SAP space */
#define ERROR_NET_UDP_NO_SAP               0xa038 /* UDP module found no SAP       */
#define ERROR_NET_UDP_INVALID_LENGTH       0xa039 /* UDP module detected length error */

#define ERROR_NET_TFTPC_NOT_INITIALIZED    0xa03a /* TFTPC module has not been init   */
#define ERROR_NET_TFTPC_FATAL_STATE        0xa03b /* TFTPC module is in fatal state   */
#define ERROR_NET_TFTPC_INVALID_HANDLE     0xa03c /* TFTPC module detected inv. hand  */
#define ERROR_NET_TFTPC_NO_FREE_SAP        0xa03d /* TFTPC module is out of SAP space */
#define ERROR_NET_TFTPC_INVALID_FILENAME   0xa03e /* TFTPC module is out of SAP space */
#define ERROR_NET_TFTPC_SAP_ERROR_STATE    0xa03f /* TFTPC module this SAP is in ERR  */
#define ERROR_NET_TFTPC_INVALID_TID        0xa040 /* TFTPC module detected invalid TID */
#define ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER 0xa041 /* TFTPC module detected 
                                                     invalid block number      */
#define ERROR_NET_TFTPC_EOF                0xa042 /* TFTPC module End Of File         */
#define ERROR_NET_TFTPC_NO_DATA            0xa043 /* TFTPC module no data             */
#define ERROR_NET_TFTPC_CONGESTION         0xa044 /* TFTPC module SAP in congestion   */
#define ERROR_NET_TFTPC_RRQ_TIMEOUT        0xa045 /* TFTPC module SAP RRQ time out    */
#define ERROR_NET_TFTPC_DATA_TIMEOUT       0xa046 /* TFTPC module SAP DATA time out   */


/************************************************************************
 *  Parameter definitions
*************************************************************************/


/* Default ping datagram size */
#define PING_DEFAULT_DATAGRAM_SIZE  64

/* Maximum ping datagram size */
#define PING_MAX_DATAGRAM_SIZE      1472

/* Maximum TFTP file size */
#define TFTP_FILE_MAX_SIZE          (0xffff * 512 - 1)


/************************************************************************
 *  NET services: 
*************************************************************************/

UINT32
NET_init( void ) ;

UINT32
net_open( UINT32 ipadr, char *filename ) ;

char
net_getchar( void ) ;

UINT32
net_getbyte( UINT8 *byte ) ;

UINT32
NET_file_read( UINT32 ipadr, 
               char   *filename,
               UINT8  *buffer,
               UINT32 *size ) ;

UINT32
NET_file_write( UINT32 ipadr, 
                char   *filename,
                UINT8  *buffer,
                UINT32 *size ) ;

UINT32
NET_poll( void ) ;

UINT32
NET_ping( UINT32 ipadr, UINT32 data_size ) ;





#endif /* #ifndef NET_API_H */
