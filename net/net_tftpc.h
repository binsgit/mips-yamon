#ifndef NET_TFTPC_H
#define NET_TFTPC_H

/************************************************************************
 *
 *      NET_TFTPC.h
 *
 *      The 'NET_TFTPC' module implements the TFTP client application,
 *      which provides services to read a file by use of the
 *      TFTP-protocol.
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

/* TFTP header definitions */
#define TFTP_HEADER_BASE     42    /* header start address 
                                      in ethernet frame            */

/* TFTP header: Relative offset addresses for each protocol element*/
#define TFTP_HEADER_OPCODE                0
#define TFTP_HEADER_BLOCKNUMBER           2
#define TFTP_HEADER_FILENAME              2
#define TFTP_HEADER_ERRORCODE             2
#define TFTP_HEADER_ERRORMSG              4
#define TFTP_HEADER_DATA                  4

#define TFTP_HEADER_MAX_BLOCKNUMBER       0xffff /* TFTP-protocol, maximum blocknumber  */

#define TFTP_HEADER_DATA_MAX_SIZE         512
#define TFTP_HEADER_FILENAME_MAX_LENGTH   128
#define TFTP_HEADER_DATA_MAX_HEADER_SIZE  TFTP_HEADER_DATA_MAX_SIZE+4
#define TFTP_HEADER_RRQ_MAX_HEADER_SIZE   TFTP_HEADER_FILENAME_MAX_LENGTH+9
#define TFTP_HEADER_WRQ_MAX_HEADER_SIZE   TFTP_HEADER_FILENAME_MAX_LENGTH+9
#define TFTP_HEADER_ACK_HEADER_SIZE       4
#define TFTP_HEADER_ERROR_HEADER_SIZE     5

#define TFTP_HEADER_OPCODE_RRQ            0x0001
#define TFTP_HEADER_OPCODE_WRQ            0x0002
#define TFTP_HEADER_OPCODE_DATA           0x0003
#define TFTP_HEADER_OPCODE_ACK            0x0004
#define TFTP_HEADER_OPCODE_ERROR          0x0005

#define TFTP_HEADER_ERROR_UNDEFINED              0x0000
#define TFTP_HEADER_ERROR_FILE_NOT_FOUND         0x0001
#define TFTP_HEADER_ERROR_ACCESS_VIOLATION       0x0002
#define TFTP_HEADER_ERROR_DISK_FULL              0x0003
#define TFTP_HEADER_ERROR_ILLEGAL_TFTP_OPERATION 0x0004
#define TFTP_HEADER_ERROR_UNKNOWN_TID            0x0005
#define TFTP_HEADER_ERROR_FILE_EXISTS            0x0006
#define TFTP_HEADER_ERROR_NO_SUCH_USER           0x0007

#define TFTP_HEADER_MODE_OCTET            "octet"
#define TFTP_HEADER_MODE_OCTET_SIZE       7


#define TFTPC_SAP_COUNT                   1    /* Maximum number of TFTPC SAP's      */
#define TFTPC_SAP_UNDEFINED               42   /* TFTPC SAP undefined definition     */

#define TFTPC_SAP_STATE_IDLE              0    /* SAP is IDLE for service                    */
#define TFTPC_SAP_STATE_BOUND_TO_UDP      1    /* SAP has been bound to UDP with UDP-'open   */
#define TFTPC_SAP_STATE_WAIT_FIRST_BLOCK  2    /* SAP is waiting to receive first DATA block */
#define TFTPC_SAP_STATE_WAIT_NEXT_BLOCK   3    /* SAP is in normal TFTP data read mode       */
#define TFTPC_SAP_STATE_CONGESTION        4    /* SAP has currently no space for more data   */
#define TFTPC_SAP_STATE_LAST_BLOCK_READ   5    /* SAP has received last block of file        */
#define TFTPC_SAP_STATE_WAIT_FIRST_ACK    6    /* SAP is waiting to receive first ACK of WRQ */
#define TFTPC_SAP_STATE_WAIT_NEXT_ACK     7    /* SAP is in normal TFTP data write mode      */
#define TFTPC_SAP_STATE_LAST_BLOCK_WRITE  8    /* SAP has written last block of file         */
#define TFTPC_SAP_STATE_ERROR             9    /* SAP detected a protocol error or 
                                                      received a ERROR opcode from host      */

#define TFTPC_SAP_OPEN_TIMEOUT            2    /* Timeout value for 'open' retransmission    */
#define TFTPC_SAP_OPEN_RETRY_COUNT        5    /* Maximum number of 'open' retries           */
#define TFTPC_SAP_CLOSE_TIMEOUT          10    /* Timeout value for after last retransmission */
#define TFTPC_SAP_TIMEOUT                 2    /* Timeout value for retransmission           */
#define TFTPC_SAP_MAX_RETRY_COUNT         4    /* Maximum number of retries                  */
#define TFTPC_SAP_BUFFER_COUNT           10    /* Data buffer count                          */
#define TFTPC_SAP_BUFFER_ALIGNMENT_SIZE  32    /* Data buffer alignment size                 */

#define TFTPC_STATE_CLOSED                0x44 /* TFTPC is closed for service                */
#define TFTPC_STATE_OPEN                  0x46 /* TFTPC bound to IP                          */

#define BUFFER_STATE_EMPTY                0    /* Data buffer is empty                       */
#define BUFFER_STATE_READING              1    /* Data buffer is being emptied               */
#define BUFFER_STATE_FULL                 2    /* Data buffer has been filled                */

typedef struct buffer_context
{
    UINT32            ctrl ;         /* buffer control                  */
    UINT32            count ;        /* buffer count                    */
    UINT8             *blink ;       /* buffer link                     */
} t_buffer_context ;

typedef struct tftpc_sap_context
{
    UINT16            sap_state ;    /* state of this SAP               */
    UINT32            udp_sp_hd ;    /* returned handle of UDP-'open'   */
    UINT32            dst_ip_adr ;   /* Destination IP adr (BE)         */
    UINT16            dst_port ;     /* Port number of host by this
                                        file-transfer; received in 
                                        1. data block(BE)               */
    t_mac_addr        dst_mac_adr ;  /* Cached MAC, derived by 1. rply  */
    UINT16            block_number ; /* Number of last block of data,
                                        which has been received and
                                        acknowledged (CPU-format) !     */
    UINT32            timeout ;      /* timeout value derived from RTC  */
    UINT32            retry_left ;   /* retries still left              */
    UINT32            error_cause ;  /* keeps the error completion, which
                                        caused the 'ERROR-STATE'        */
    UINT32            send_rc ;      /* last completion from UDP-'send' */
    UINT16            host_error ;   /* state of this SAP (CPU-format)  */
    UINT8             *host_error_msg ; /* host error string message is
                                        kept in 'data'-buffer           */
    UINT8             *filename ;    /* Filename string is kept in
                                        in 'data'-buffer during 'RRQ'   */
    UINT8             *pread ;       /* 'read' pointer for data buffer  */
    UINT8             *puser ;       /* 'read' or 'write' pointer for 
                                        user data buffer                */
    UINT32            user_size ;    /* user bytes read or written      */
    UINT32            bytes_left;    /* keeps a counter for number of
                                        bytes, still to be read before
                                        next re-evaluation of this var  */
    UINT8             *data ;        /* File data buffer                */
    UINT32            read_idx ;     /* Next read buffer index          */
    UINT32            write_idx ;    /* Next write buffer index         */
    t_buffer_context  buf[TFTPC_SAP_BUFFER_COUNT] ; /* Buffer context   */
} t_tftpc_sap_context ;


/************************************************************************
 *  NET_TFTPC services: 
*************************************************************************/

/************************************************************************
 *
 *                          NET_TFTPC_init
 *  Description :
 *  -------------
 *  Initialize the TFTPC module.
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
UINT32 NET_TFTPC_init( void ) ;


/************************************************************************
 *
 *                          NET_TFTPC_open
 *  Description :
 *  -------------
 *  Allocate a TFTPC-SAP and register user context.
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',          IN,    destination ip address (BE)
 *  'filename',        IN,    filename to read
 *  'sp_hd',           OUT,   handle of TFTPC to be used by user by call
 *                            of 'read', 'readbyte' or 'close'
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_FATAL_STATE'      A fatal state has been detected in TFTPC.
 *  'ERROR_NET_TFTPC_NOT_INITIALIZED'  TFTPC-'init' has not been called.
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_open( UINT32     ip_adr,    /* destination ip address (BE) */
                       UINT8      *filename, /* filename to read            */
                       UINT32     *sp_hd ) ; /* file session handle         */


/************************************************************************
 *
 *                          NET_TFTPC_close
 *  Description :
 *  -------------
 *  Close  TFTPC-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                TFTPC SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_close( UINT32 sp_hd ) ;


/************************************************************************
 *
 *                          NET_TFTPC_read
 *  Description :
 *  -------------
 *  Read block data.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),               Data has been read.
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_read( UINT32 sp_hd,
                       UINT32 *length,
                       UINT8  *data ) ;


/************************************************************************
 *
 *                          NET_TFTPC_readbyte
 *  Description :
 *  -------------
 *  Read one byte of received data.
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),               Byte has been read.
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_readbyte( UINT32 sp_hd,
                           UINT8  *byte ) ;


/************************************************************************
 *
 *                          NET_TFTPC_file_read
 *  Description :
 *  -------------
 *  Read file ('filename') of host('ipadr') into 'buffer'
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',          IN,    TFTP server host ip address (BE)
 *  'filename',        IN,    name of file to read
 *  'buffer',          IN,    pointer to buffer to write
 *  'size',            IN,    pointer to size of buffer
 *  'poll',            IN,    pointer to poll function to do be called
 *                            during file transfer
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                 File has been read.
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_file_read( UINT32 ipadr, 
                            UINT8  *filename,
                            UINT8  *buffer,
                            UINT32 *size,
                            UINT32 (*poll)(void) ) ;

/************************************************************************
 *
 *                          NET_TFTPC_file_write
 *  Description :
 *  -------------
 *  Write 'buffer' into file('filename') of host('ipadr')
 *
 *
 *  Parameters :
 *  ------------
 *  'ip_adr',          IN,    TFTP server host ip address (BE)
 *  'filename',        IN,    name of file to write
 *  'buffer',          IN,    pointer to buffer to read into file
 *  'size',            IN,    pointer to size of buffer
 *  'poll',            IN,    pointer to poll function to do be called
 *                            during file transfer
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                 Buffer has been written.
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_file_write( UINT32 ipadr, 
                             UINT8  *filename,
                             UINT8  *buffer,
                             UINT32 *size,
                             UINT32 (*poll)(void) ) ;


/************************************************************************
 *
 *                          NET_TFTPC_poll
 *  Description :
 *  -------------
 *  Do timer managemnt of TFTPC-SAP's. or inquire state
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0),                
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_poll( UINT32 *state, UINT32 *last_io_compl, UINT32 *cause ) ;



#endif /* #ifndef NET_TFTPC_H */
