/************************************************************************
 *
 *      NET_TFTPC.c
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
 *      Include files
 ************************************************************************/

//#define NET_DEBUG 1

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>
#include <io_api.h>
#include <flash_api.h>

/* net stuff */
#include <net_api.h>
#include "net.h"
#include "net_mac.h"
#include "net_ip.h"
#include "net_udp.h"
#include "net_tftpc.h"


/************************************************************************
 *      Definitions
 ************************************************************************/



/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

/* Global state of TFTPC */
static UINT32 NET_TFTPC_state = TFTPC_STATE_CLOSED ;

/* SAP */
static t_tftpc_sap_context sap_context ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

#ifdef NET_DEBUG
static
void NET_TFTPC_dumpcontext( void ) ;
#endif


/************************************************************************
 *
 *                          NET_TFTPC_receive_ack
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call
 *  us back with an reference to the received frame,
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'OK'
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_ack( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     ); /* start of frame */


/************************************************************************
 *
 *                          NET_TFTPC_receive_data_block
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call
 *  us back with an reference to the received frame,
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'OK'
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_data_block( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     ); /* start of frame */



/************************************************************************
 *
 *                          NET_TFTPC_receive_data_byte
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a 
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call 
 *  us back with an reference to the received frame, 
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'ERROR_NET_TFTPC_CONGESTION'
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_data_byte( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     ); /* start of frame */


/************************************************************************
 *
 *                          NET_TFTPC_init_sap
 *  Description :
 *  -------------
 *  Initialize the TFTPC SAP context.
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
static
UINT32 NET_TFTPC_init_sap( t_tftpc_sap_context *context ) ;


/************************************************************************
 *
 *                          NET_TFTPC_reset_sap
 *  Description :
 *  -------------
 *  Reset the TFTPC SAP context.
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
 I
************************************************************************/
static
UINT32 NET_TFTPC_reset_sap( t_tftpc_sap_context *context,
                            t_udp_usr_receive   rec ) ;


/************************************************************************
 *
 *                          NET_TFTPC_sendDATA
 *  Description :
 *  -------------
 *  Send 'DATA' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendDATA( t_tftpc_sap_context *context ) ;


/************************************************************************
 *
 *                          NET_TFTPC_sendRRQ
 *  Description :
 *  -------------
 *  Send 'RRQ' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendRRQ( t_tftpc_sap_context *context ) ;


/************************************************************************
 *
 *                          NET_TFTPC_sendWRQ
 *  Description :
 *  -------------
 *  Send 'WRQ' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendWRQ( t_tftpc_sap_context *context ) ;


/************************************************************************
 *
 *                          NET_TFTPC_sendACK
 *  Description :
 *  -------------
 *  Send 'ACK' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendACK( t_tftpc_sap_context *context ) ;


/************************************************************************
 *
 *                          NET_TFTPC_unknownTID
 *  Description :
 *  -------------
 *  Send 'ERROR' packet, cause = 'unknown TID'
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
************************************************************************/
static
UINT32 NET_TFTPC_unknownTID(
                     UINT32     dst_ip_adr, /* dest. ip address (BE) */
                     UINT16     dst_port,   /* dest. port (BE)       */
                     UINT32     sp_hd   ) ; /* handle from 'open'    */


#if 0 /* currently not called */
/************************************************************************
 *
 *                          NET_TFTPC_sendERROR
 *  Description :
 *  -------------
 *  Send 'ERROR' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendERROR( t_tftpc_sap_context *context, UINT16 error ) ;
#endif


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_TFTPC_init
 *  Description :
 *  -------------
 *  Initialize the TFTPC module, which includes make a UDP-'open' to
 *  create a UDP-SAP.
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
UINT32 NET_TFTPC_init(  void )
{
    UINT32 i, rcode ;
    t_sys_malloc  mem ;
    void          *ptmp ;

    /* allocate buffers */
    mem.size     = TFTP_HEADER_DATA_MAX_SIZE ;
    mem.boundary = TFTPC_SAP_BUFFER_ALIGNMENT_SIZE ;
    mem.memory   = &(ptmp) ;
    rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                         &mem,
                         sizeof(t_sys_malloc) ) ;
    if (rcode != OK)
    {
       return( rcode ) ;
    }
    sap_context.data = (UINT8*) KSEG0( (UINT32)ptmp ) ;

    for (i=0; i<TFTPC_SAP_BUFFER_COUNT; i++)
    {
        /* allocate SAP buffer */
        mem.size     = TFTP_HEADER_DATA_MAX_SIZE ;
        mem.boundary = TFTPC_SAP_BUFFER_ALIGNMENT_SIZE ;
        mem.memory   = &(ptmp) ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
           return( rcode ) ;
        }
        sap_context.buf[i].blink = (UINT8*) KSEG0( (UINT32)ptmp ) ;
    }

    /* initialize SAP */
    rcode = NET_TFTPC_init_sap( &sap_context ) ;

    /* TFTPC-module has now been initialized */
    NET_TFTPC_state = TFTPC_STATE_OPEN ;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_open
 *  Description :
 *  -------------
 *  Allocate a TFTPC-SAP, register user context, 
 *  and request UDP to send 'RRQ'.
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
 *  'ERROR_NET_TFTPC_NOT_INITIALIZED'
 *  'ERROR_NET_TFTPC_INVALID_FILENAME'
 *  'ERROR_NET_TFTPC_FATAL_STATE'
 *  'OK'(=0),
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_open( UINT32     ip_adr,    /* destination ip address (BE) */
                       UINT8      *filename, /* filename to read            */
                       UINT32     *sp_hd )   /* file session handle         */
{
    UINT32  old_ie, time_now ;
    UINT32  rcode = OK ;
    int     len ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_open\n") ;
#endif

    old_ie = sys_disable_int() ;
    switch (NET_TFTPC_state)
    {
        case TFTPC_STATE_CLOSED:
                rcode = ERROR_NET_TFTPC_NOT_INITIALIZED ;
            break;

        case TFTPC_STATE_OPEN:
                /* Reset the one and only SAP */
                *sp_hd = 0 ; /* actually, this is not used */
                rcode = NET_TFTPC_reset_sap( &sap_context,
                                             NET_TFTPC_receive_data_byte ) ;

                /* validate filename length */
                len = strlen(filename) ;
                if (len > TFTP_HEADER_FILENAME_MAX_LENGTH)
                {
                    rcode = ERROR_NET_TFTPC_INVALID_FILENAME ;
                    break;
                }

                /* Save user context to this SAP */
                sap_context.dst_ip_adr = ip_adr ;
                strcpy( sap_context.filename, filename ) ;
                strcpy( &sap_context.filename[len+1], TFTP_HEADER_MODE_OCTET ) ;

                /* Set retry count */
                sap_context.retry_left = TFTPC_SAP_OPEN_RETRY_COUNT ;

                /* get time now (unit is seconds since 1.1.1970) */
                NET_gettime( &time_now ) ;
                sap_context.timeout = time_now + TFTPC_SAP_OPEN_TIMEOUT ;

                /* New state = 'WAIT_FIRST_BLOCK' */
                sap_context.sap_state = TFTPC_SAP_STATE_WAIT_FIRST_BLOCK ;

                /* Send 'RRQ' */
                rcode = NET_TFTPC_sendRRQ( &sap_context ) ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_TFTPC_FATAL_STATE ;
            break;

    }
    if(old_ie) sys_enable_int();
    return( rcode ) ;
}



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
 *  'OK'(=0),              TFTPC SAP has been closed
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_close( UINT32 sp_hd )
{
    UINT32 rcode, old_ie ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_close\n") ;
#endif

    /* Close SAP */

    /* Reset context */
    old_ie = sys_disable_int() ;
    rcode = NET_TFTPC_reset_sap( &sap_context,
                                 NET_TFTPC_receive_data_byte ) ;
    if(old_ie) sys_enable_int();

    return( rcode ) ;
}


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
 *  'ERROR_NET_TFTPC_NO_DATA'
 *  'ERROR_NET_TFTPC_EOF'
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'OK'(=0)
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_readbyte( UINT32 sp_hd,
                           UINT8  *byte )
{
    UINT32 old_ie, rcode, time_now, index ;

    if ( sap_context.bytes_left )
    {
        /* Normal case1: We seem to have data */
        sap_context.bytes_left-- ; 
        *byte = *sap_context.pread++ ;
        rcode = OK ;
    }
    else
    {
        /* No data, re-evaluate buffer status */
        old_ie = sys_disable_int() ;
        rcode = ERROR_NET_TFTPC_NO_DATA ;
        index = sap_context.read_idx ;
        if ( sap_context.buf[ index ].ctrl == BUFFER_STATE_READING )
        {
            /* we have just finished another buffer, proceed to next buffer */
            sap_context.buf[ index ].count = 0 ;
            sap_context.buf[ index ].ctrl  = BUFFER_STATE_EMPTY ;
            sap_context.read_idx++ ;
            if (sap_context.read_idx == TFTPC_SAP_BUFFER_COUNT)
            {
                sap_context.read_idx = 0 ;
            }
            index = sap_context.read_idx ;
        }
        
        /* check for data in next buffer */
        if ( sap_context.buf[ index ].ctrl == BUFFER_STATE_FULL )
        {
            /* we just received another buffer, proceed */
            sap_context.pread = sap_context.buf[ index ].blink ;
            sap_context.bytes_left = sap_context.buf[ index ].count ;
            sap_context.buf[ index ].ctrl  = BUFFER_STATE_READING ;

            if (sap_context.bytes_left)
            {
                /* read first byte in next buffer */
                sap_context.bytes_left-- ;
                *byte = *sap_context.pread++ ;
                rcode = OK ;
            }
            else
            {
                /* next buffer contains headers only: no data left */
                *byte = EOF_SREC ;
                rcode = ERROR_NET_TFTPC_EOF ;
            }
        }

        /* Last case: no data, evaluate why */
        if ( sap_context.buf[ index ].ctrl == BUFFER_STATE_EMPTY )
        {
            if ( sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_READ )
            {
                *byte = EOF_SREC ;
                rcode = ERROR_NET_TFTPC_EOF ;
            }

            if ( sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
            {
                *byte = EOF_SREC ;
                rcode = ERROR_NET_TFTPC_SAP_ERROR_STATE ;
            }
        }

        /* If we have been in 'CONGESTION' state, we need to ACK */
        if (sap_context.sap_state == TFTPC_SAP_STATE_CONGESTION)
        {
            /* Set retry count */
            sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

            /* get time now (unit is seconds since 1.1.1970) */
            NET_gettime( &time_now ) ;
            sap_context.timeout = time_now + TFTPC_SAP_TIMEOUT ;

            sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_BLOCK ;
            NET_TFTPC_sendACK( &sap_context ) ;
        }
        if(old_ie) sys_enable_int();
    }
    return( rcode ) ;
}


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
 *  'ERROR_NET_TFTPC_NOT_INITIALIZED'
 *  'ERROR_NET_TFTPC_INVALID_FILENAME'
 *  'ERROR_NET_TFTPC_FATAL_STATE'
 *  'OK'(=0)
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_file_read( UINT32 ipadr,
                            UINT8  *filename,
                            UINT8  *buffer,
                            UINT32 *size,
                            UINT32 (*poll)(void) )
{
    UINT32  old_ie, time_now ;
    UINT32  rcode = OK ;
    int     len ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_file_read\n") ;
#endif

    /* clear error context */
    net_last_error  = OK ;
    net_diag_msg[0] = 0 ;

    old_ie = sys_disable_int() ;
    switch (NET_TFTPC_state)
    {
        case TFTPC_STATE_CLOSED:
                rcode = ERROR_NET_TFTPC_NOT_INITIALIZED ;
            break;

        case TFTPC_STATE_OPEN:
                /* Reset the one and only SAP */
                rcode = NET_TFTPC_reset_sap( &sap_context,
                                             NET_TFTPC_receive_data_block ) ;

                /* validate filename length */
                len = strlen(filename) ;
                if (len > TFTP_HEADER_FILENAME_MAX_LENGTH)
                {
                    rcode = ERROR_NET_TFTPC_INVALID_FILENAME ;
                    break;
                }

                /* Save user context to this SAP */
                sap_context.dst_ip_adr = ipadr ;
                sap_context.puser      = buffer ;
                sap_context.user_size  = *size ;
                sap_context.bytes_left = *size ;
                strcpy( sap_context.filename, filename ) ;
                strcpy( &sap_context.filename[len+1], TFTP_HEADER_MODE_OCTET ) ;

                /* Set retry count */
                sap_context.retry_left = TFTPC_SAP_OPEN_RETRY_COUNT ;

                /* get time now (unit is seconds since 1.1.1970) */
                NET_gettime( &time_now ) ;
                sap_context.timeout = time_now + TFTPC_SAP_OPEN_TIMEOUT ;

                /* New state = 'WAIT_FIRST_BLOCK' */
                sap_context.sap_state = TFTPC_SAP_STATE_WAIT_FIRST_BLOCK ;

                /* Send 'RRQ' */
                rcode = NET_TFTPC_sendRRQ( &sap_context ) ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_TFTPC_FATAL_STATE ;
            break;

    }
    if(old_ie) sys_enable_int();

    while (1)
    {
        if ((rcode !=OK) && (rcode != ERROR_NET_ARP_MAC_NOT_RESOLUTED))
        {
            break ;
        }

        if ( sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_READ )
        {
            rcode = OK ;
            break ;
        }

        if ( sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
        {
            rcode = sap_context.error_cause ;
            break ;
        }

        (*poll)() ;
    }

    *size = sap_context.user_size - sap_context.bytes_left ;
    NET_TFTPC_close( 0 ) ;
    net_last_error = rcode ;
    return( rcode ) ;
}


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
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_NOT_INITIALIZED'
 *  'ERROR_NET_TFTPC_INVALID_FILENAME'
 *  'ERROR_NET_TFTPC_FATAL_STATE'
 *  'OK'(=0)
 *
 *
 ************************************************************************/
UINT32 NET_TFTPC_file_write( UINT32 ipadr,
                             UINT8  *filename,
                             UINT8  *buffer,
                             UINT32 *size,
                             UINT32 (*poll)(void) )
{
    UINT32  old_ie, time_now ;
    UINT32  rcode = OK ;
    int     len ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_file_write\n") ;
    printf("ipadr = %x\n", (unsigned)(BE32_TO_CPU(ipadr)) ) ;
    printf("filename = %s\n", filename ) ;
    printf("size = %d\n", *size ) ;
#endif

    /* clear error context */
    net_last_error  = OK ;
    net_diag_msg[0] = 0 ;

    if (*size > TFTP_FILE_MAX_SIZE)
    {
        rcode = ERROR_NET_TFTP_FILE_TOO_BIG ;
        net_last_error = rcode ;
        return( rcode ) ;
    }

    old_ie = sys_disable_int() ;
    switch (NET_TFTPC_state)
    {
        case TFTPC_STATE_CLOSED:
                rcode = ERROR_NET_TFTPC_NOT_INITIALIZED ;
            break;

        case TFTPC_STATE_OPEN:
                /* Reset the one and only SAP */
                rcode = NET_TFTPC_reset_sap( &sap_context,
                                             NET_TFTPC_receive_ack ) ;

                /* validate filename length */
                len = strlen(filename) ;
                if (len > TFTP_HEADER_FILENAME_MAX_LENGTH)
                {
                    rcode = ERROR_NET_TFTPC_INVALID_FILENAME ;
                    break;
                }

                /* Save user context to this SAP */
                sap_context.dst_ip_adr = ipadr ;
                sap_context.puser      = buffer ;
                sap_context.user_size  = *size ;
                sap_context.bytes_left = *size ;
                strcpy( sap_context.filename, filename ) ;
                strcpy( &sap_context.filename[len+1], TFTP_HEADER_MODE_OCTET ) ;

                /* Set retry count */
                sap_context.retry_left = TFTPC_SAP_OPEN_RETRY_COUNT ;

                /* get time now (unit is seconds since 1.1.1970) */
                NET_gettime( &time_now ) ;
                sap_context.timeout = time_now + TFTPC_SAP_OPEN_TIMEOUT ;

                /* New state = 'WAIT_FIRST_ACK' */
                sap_context.sap_state = TFTPC_SAP_STATE_WAIT_FIRST_ACK ;

                /* Send 'WRQ' */
                rcode = NET_TFTPC_sendWRQ( &sap_context ) ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_TFTPC_FATAL_STATE ;
            break;

    }
    if(old_ie) sys_enable_int();

    while (1)
    {
        if ((rcode !=OK) && (rcode != ERROR_NET_ARP_MAC_NOT_RESOLUTED))
        {
            break ;
        }

        if ( sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_WRITE )
        {
            rcode = OK ;
            break ;
        }

        if ( sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
        {
            rcode = sap_context.error_cause ;
            break ;
        }

        (*poll)() ;
    }

    *size = sap_context.user_size - sap_context.bytes_left ;
    NET_TFTPC_close( 0 ) ;
    net_last_error = rcode ;
    return( rcode ) ;
}


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
UINT32 NET_TFTPC_poll( UINT32 *state, UINT32 *last_io_compl, UINT32 *cause )
{
    UINT32 time_now ;
    UINT32 old_ie ;

    if ( sap_context.sap_state == TFTPC_SAP_STATE_BOUND_TO_UDP )
    {
        /* No TFTP-client session active; do nothing */
        return( OK ) ;
    }

    /* Check for inquire */
    if ( state != NULL )
    {
        old_ie = sys_disable_int() ;
        *state         = sap_context.sap_state ;
        *last_io_compl = sap_context.send_rc ;
        *cause         = sap_context.error_cause ;
        if(old_ie) sys_enable_int();
        return( OK ) ;
    }

    /* get time now (unit is seconds since 1.1.1970) */
    NET_gettime( &time_now ) ;

    old_ie = sys_disable_int() ;

    switch (sap_context.sap_state)
    {
        case TFTPC_SAP_STATE_WAIT_FIRST_BLOCK:
        {
            /* Check for 'RRQ' retry */
    
            if ( time_now >= sap_context.timeout )
            {
                /* Check for any retries */
                if (sap_context.retry_left)
                {
                    /* Send 'RRQ' */
                    sap_context.timeout = time_now + TFTPC_SAP_OPEN_TIMEOUT ;
                    sap_context.retry_left-- ;
                    NET_TFTPC_sendRRQ( &sap_context ) ;
                }
                else
                {
                    sap_context.sap_state   = TFTPC_SAP_STATE_ERROR ;
                    if (sap_context.send_rc == ERROR_NET_ARP_MAC_NOT_RESOLUTED)
                    {
                        sap_context.error_cause = ERROR_NET_ARP_TIME_OUT ;
                    }
                    else
                    {
                        sap_context.error_cause = ERROR_NET_TFTP_READ_TIMEOUT_ERROR ;
                    }
                }
            }
        }
        break ;


        case TFTPC_SAP_STATE_WAIT_FIRST_ACK:
        {
            /* Check for 'WRQ' retry */
    
            if ( time_now >= sap_context.timeout )
            {
                /* Check for any retries */
                if (sap_context.retry_left)
                {
                    /* Send 'WRQ' */
                    sap_context.timeout = time_now + TFTPC_SAP_OPEN_TIMEOUT ;
                    sap_context.retry_left-- ;
                    NET_TFTPC_sendWRQ( &sap_context ) ;
                }
                else
                {
                    sap_context.sap_state   = TFTPC_SAP_STATE_ERROR ;
                    if (sap_context.send_rc == ERROR_NET_ARP_MAC_NOT_RESOLUTED)
                    {
                        sap_context.error_cause = ERROR_NET_ARP_TIME_OUT ;
                    }
                    else
                    {
                        sap_context.error_cause = ERROR_NET_TFTP_WRITE_TIMEOUT_ERROR ;
                    }
                }
            }
        }
        break ;


        case TFTPC_SAP_STATE_WAIT_NEXT_BLOCK:
        {
            /* Check for 'ACK' retry */
    
            if ( time_now >= sap_context.timeout )
            {
                /* Check for any retries */
                if (sap_context.retry_left)
                {
                    if (sap_context.retry_left == 1)
                    {
                        /* Set 'CLOSE' timeout */
                        sap_context.timeout = time_now + TFTPC_SAP_CLOSE_TIMEOUT ;
                        sap_context.retry_left-- ;
                    }
                    else
                    {
                        /* Send 'ACK' */
                        sap_context.timeout = time_now + TFTPC_SAP_TIMEOUT ;
                        sap_context.retry_left-- ;
                        NET_TFTPC_sendACK( &sap_context ) ;
                    }
                }
                else
                {
                    sap_context.sap_state   = TFTPC_SAP_STATE_ERROR ;
                    sap_context.error_cause = ERROR_NET_TFTP_DATA_TIMEOUT_ERROR ;
                }
            }
        }
        break ;
    
        case TFTPC_SAP_STATE_WAIT_NEXT_ACK:
        {
            /* Check for 'DATA' retry */

            if ( time_now >= sap_context.timeout )
            {
                /* Check for any retries */
                if (sap_context.retry_left)
                {
                    /* Send 'DATA' */
                    sap_context.timeout = time_now + TFTPC_SAP_TIMEOUT ;
                    sap_context.retry_left-- ;
                    NET_TFTPC_sendDATA( &sap_context ) ;
                }
                else
                {
                    sap_context.sap_state   = TFTPC_SAP_STATE_ERROR ;
                    sap_context.error_cause = ERROR_NET_TFTP_DATA_TIMEOUT_ERROR ;
                }
            }
        }

        default:
                /* we should never arrive here */
            break;
    }

    if(old_ie) sys_enable_int();
    return( OK ) ;
}

/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_TFTPC_receive_ack
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call 
 *  us back with an reference to the received frame, 
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_ack( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     )  /* start of frame */
{
    UINT32            rcode = OK ;
    UINT8             *p ;
    UINT16            opcode ;
    UINT16            tmp2 ;

    /* We already registered an ERROR */
    if (sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
    {
        /* do nothing */
        return(ERROR_NET_TFTPC_SAP_ERROR_STATE) ;
    }

    /* get TFTP-'opcode'-field of this received frame */
    p = (data + TFTP_HEADER_BASE + TFTP_HEADER_OPCODE) ;
    get2( p, opcode) ;
    opcode = BE16_TO_CPU( opcode ) ;
    get2( p, tmp2) ;

    /* NOTE: 'tmp2' now keeps block-number or host-error */
    tmp2 = BE16_TO_CPU( tmp2 ) ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_receive_ack\n") ;
    printf("src_ip_adr = %x\n", (unsigned)BE32_TO_CPU(src_ip_adr) ) ;
    printf("src_port   = %d\n", (unsigned)BE16_TO_CPU(src_port) ) ;
    printf("length = %d\n", length ) ;
    printf("src MAC = " ) ;
    p = (UINT8*) src_mac_adr ;
    {
        int  i ;
        for (i=0; i<6; i++)
        {
            printf(" %02x",*p++ ) ;
        }
    }
    printf("\n" ) ;
    printf("opcode     = %d\n", (unsigned)opcode) ;
    printf("block nr.  = %d\n", (unsigned)tmp2 ) ;
    printf("bytes_left = %d\n", (unsigned)sap_context.bytes_left ) ;
#endif

    if (opcode != TFTP_HEADER_OPCODE_ACK)
    {
        /* We expect nothing else than 'ACK' or 'ERROR' */

        /* check for HOST error */
        if (opcode == TFTP_HEADER_OPCODE_ERROR)
        {
            if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_ACK)
            {
                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );
                sap_context.error_cause = ERROR_NET_TFTP_WRITE_ERROR ;
            }

            if ( (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_NEXT_ACK) ||
                 (sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_WRITE) )
            {
                /* validate 'src_port' */
                if (sap_context.dst_port != src_port)
                {
                    /* Ignore packet, and reply 'unknown TID' */
                    rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                                  src_port,
                                                  sap_context.udp_sp_hd ) ;
        
                    /* do nothing */
                    return( ERROR_NET_TFTPC_INVALID_TID ) ;
                }

                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );

                sap_context.error_cause = ERROR_NET_TFTP_DATA_ERROR ;
            }

            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
        }

        /* return, despite if it is not ACK */
        return( sap_context.error_cause ) ;
    }


    /* We have 'ACK' */

    /* Check for 1. 'ACK' and save 'src_port', and src-MAC */
    if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_ACK)
    {
        sap_context.dst_port = src_port ;
        memcpy( &sap_context.dst_mac_adr, 
                src_mac_adr,
                SYS_MAC_ADDR_SIZE ) ;
    }
    else
    {
        /* validate 'src_port' */
        if (sap_context.dst_port != src_port)
        {
            /* Ignore packet, and reply 'unknown TID' */
            rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                          src_port,
                                          sap_context.udp_sp_hd ) ;

            /* do nothing */
            return( ERROR_NET_TFTPC_INVALID_TID ) ;
        }
    }

    /* 'TID' is valid */

    /* Validate block number */
    if (tmp2 != sap_context.block_number)
    {
        if (tmp2 == TFTP_HEADER_MAX_BLOCKNUMBER)
        {
            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
            sap_context.error_cause = ERROR_NET_TFTP_FILE_TOO_BIG ;

            return( ERROR_NET_TFTP_FILE_TOO_BIG ) ;
        }

        /* To avoid the "Sorcerer's Apprentice: do nothing and let timer 
           retransmit if host doesn't */
        return( ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER ) ;
    }


    if ( sap_context.bytes_left < TFTP_HEADER_DATA_MAX_SIZE )
    {
        if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_ACK)
        {
            /* OK, proceed to send first (and last) block */
            sap_context.block_number++ ;

            /* Set timer and retry count */

            /* get time now (unit is seconds since 1.1.1970) */
            NET_gettime( &sap_context.timeout ) ;
            sap_context.timeout    = sap_context.timeout + TFTPC_SAP_TIMEOUT ;
            sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

            /* Set state 'WAIT_NEXT_ACK' */
            sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_ACK ;

            /* Send DATA */
            rcode = NET_TFTPC_sendDATA( &sap_context ) ;
        }
        else
        {
            /* Last block has been acknowledged */


            /* update context */
            sap_context.puser = sap_context.puser + sap_context.bytes_left ;
            sap_context.bytes_left = 0 ;

            /* Set state 'LAST_BLOCK_WRITE' */
            sap_context.sap_state = TFTPC_SAP_STATE_LAST_BLOCK_WRITE ;

        }
    }
    else
    {
        if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_ACK)
        {
            /* OK, proceed to send first (full) block */
            sap_context.block_number++ ;

            /* Set timer and retry count */

            /* get time now (unit is seconds since 1.1.1970) */
            NET_gettime( &sap_context.timeout ) ;
            sap_context.timeout    = sap_context.timeout + TFTPC_SAP_TIMEOUT ;
            sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

            /* Set state 'WAIT_NEXT_ACK' */
            sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_ACK ;

            /* Send DATA */
            rcode = NET_TFTPC_sendDATA( &sap_context ) ;
        }
        else
        {
            /* OK, we have just sent a full block, update context */
            /* indicate address in alpha-display */
            DISP( (UINT32)sap_context.puser ) ;
    
            sap_context.bytes_left = sap_context.bytes_left - TFTP_HEADER_DATA_MAX_SIZE ;
            sap_context.puser = sap_context.puser + TFTP_HEADER_DATA_MAX_SIZE ;

            /* OK, proceed to send next block */
            sap_context.block_number++ ;

            /* Set timer and retry count */

            /* get time now (unit is seconds since 1.1.1970) */
            NET_gettime( &sap_context.timeout ) ;
            sap_context.timeout    = sap_context.timeout + TFTPC_SAP_TIMEOUT ;
            sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

            /* Set state 'WAIT_NEXT_ACK' */
            sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_ACK ;

            /* Send DATA */
            rcode = NET_TFTPC_sendDATA( &sap_context ) ;
        }
    }


    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_receive_data_block
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call 
 *  us back with an reference to the received frame, 
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_data_block( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     )  /* start of frame */
{
    UINT32            rcode = OK ;
    UINT8             *p ;
    UINT16            opcode ;
    UINT16            tmp2 ;
    UINT32            tftp_data_length ;
    t_FLASH_write_descriptor flash_write ;

#ifdef NET_DEBUG
    int               i ;
    printf("NET_TFTPC_receive_data_block\n") ;
    printf("src_ip_adr = %x\n", (unsigned)BE32_TO_CPU(src_ip_adr) ) ;
    printf("src_port   = %d\n", BE16_TO_CPU(src_port) ) ;
    printf("length = %d\n", length ) ;
    printf("src MAC = " ) ;
    p = (UINT8*) src_mac_adr ;
    for (i=0; i<6; i++)
    {
        printf(" %02x",*p++ ) ;
    }
    printf("\n" ) ;
#endif


    /* We already registered an ERROR */
    if (sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
    {
        /* do nothing */
        return(ERROR_NET_TFTPC_SAP_ERROR_STATE) ;
    }

    /* get TFTP-'opcode'-field of this received frame */
    p = (data + TFTP_HEADER_BASE + TFTP_HEADER_OPCODE) ;
    get2( p, opcode) ;
    opcode = BE16_TO_CPU( opcode ) ;
    get2( p, tmp2) ;

    /* NOTE: 'tmp2' now keeps block-number or host-error */
    tmp2 = BE16_TO_CPU( tmp2 ) ;

    if (opcode != TFTP_HEADER_OPCODE_DATA)
    {
        /* We expect nothing else than 'DATA' or 'ERROR' */

        /* check for HOST error */
        if (opcode == TFTP_HEADER_OPCODE_ERROR)
        {
            if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_BLOCK)
            {
                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );
                sap_context.error_cause = ERROR_NET_TFTP_READ_ERROR ;
            }

            if ( (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_NEXT_BLOCK) ||
                 (sap_context.sap_state == TFTPC_SAP_STATE_CONGESTION)      ||
                 (sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_READ) )
            {
                /* validate 'src_port' */
                if (sap_context.dst_port != src_port)
                {
                    /* Ignore packet, and reply 'unknown TID' */
                    rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                                  src_port,
                                                  sap_context.udp_sp_hd ) ;
        
                    /* do nothing */
                    return( ERROR_NET_TFTPC_INVALID_TID ) ;
                }

                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );

                sap_context.error_cause = ERROR_NET_TFTP_DATA_ERROR ;
            }

            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
        }

        /* return, despite if it is not DATA */
        return( sap_context.error_cause ) ;
    }

    /* We have 'DATA' */

    /* Check for 1. Block data and save 'src_port', and src-MAC */
    if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_BLOCK)
    {
        sap_context.dst_port = src_port ;
        memcpy( &sap_context.dst_mac_adr, 
                src_mac_adr,
                SYS_MAC_ADDR_SIZE ) ;
    }
    else
    {
        /* validate 'src_port' */
        if (sap_context.dst_port != src_port)
        {
            /* Ignore packet, and reply 'unknown TID' */
            rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                          src_port,
                                          sap_context.udp_sp_hd ) ;

            /* do nothing */
            return( ERROR_NET_TFTPC_INVALID_TID ) ;
        }
    }

    /* 'TID' is valid */

    /* Validate block number */
    if (tmp2 != (sap_context.block_number + 1) )
    {
        if (tmp2 == TFTP_HEADER_MAX_BLOCKNUMBER)
        {
            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
            sap_context.error_cause = ERROR_NET_TFTP_FILE_TOO_BIG ;

            return( ERROR_NET_TFTP_FILE_TOO_BIG ) ;
        }

        /* To avoid the "Sorcerer's Apprentice: do nothing and let timer 
           retransmit if host doesn't */
        return( ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER ) ;
    }

    /* Save data into user buffer */
    tftp_data_length = length - (MAC_HEADER_SIZE + IP_HEADER_SIZE +
                                 UDP_HEADER_SIZE + TFTP_HEADER_DATA) ;
    if ( tftp_data_length <= sap_context.bytes_left )
    {
        /* we have space for this block */

        /* OK, send ACK now */
        sap_context.block_number = tmp2 ;
        rcode = NET_TFTPC_sendACK( &sap_context ) ;


        /* save data in user buffer */
	if(tftp_data_length)
	{
	    /* Write data to RAM/FLASH
	     *   
	     * dst addr  = (UINT32)sap_context.puser
	     * src addr  = &data[TFTP_HEADER_BASE + TFTP_HEADER_DATA]
	     */

	    /* Validate address */
	    rcode = sys_validate_range( 
	             (UINT32)sap_context.puser,
		     tftp_data_length,
		     sizeof(UINT8),
		     TRUE );
            if (rcode != OK) goto error_out;

            flash_write.adr     = (UINT32)sap_context.puser;
            flash_write.length  = tftp_data_length ;
            flash_write.buffer  = &data[TFTP_HEADER_BASE + TFTP_HEADER_DATA] ;
            rcode = IO_write( SYS_MAJOR_FLASH_STRATA, 0, &flash_write ) ;
            if (rcode != OK) goto error_out;

            /* indicate address in alpha-display */
            DISP( (UINT32)sap_context.puser ) ;
        }

        /* update context */
        sap_context.bytes_left = sap_context.bytes_left - tftp_data_length ;
        sap_context.puser = sap_context.puser + tftp_data_length ;

        if (tftp_data_length < TFTP_HEADER_DATA_MAX_SIZE)
        {
            /* Set state 'LAST_BLOCK_READ' */
            sap_context.sap_state = TFTPC_SAP_STATE_LAST_BLOCK_READ ;
        }
        else
        {
            /* Set timer and retry count */

            /* get time now (unit is seconds since 1.1.1970) */
            NET_gettime( &sap_context.timeout ) ;
            sap_context.timeout    = sap_context.timeout + TFTPC_SAP_TIMEOUT ;
            sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

            /* Set state 'WAIT_NEXT_BLOCK' */
            sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_BLOCK ;
        }
        rcode = OK ;
    }
    else
    {
        rcode = ERROR_NET_TFTP_FILE_TOO_BIG ;
error_out:
        /* New state: 'ERROR' */
        sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
        sap_context.error_cause = rcode;
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_receive_data_byte
 *  Description :
 *  -------------
 *  This function is registered in the UDP-module and linked with a
 *  unique UDP-'port', idnetifying our SAP. This allows the UDP to call 
 *  us back with an reference to the received frame, 
 *  containing a TFTP-frame.
 *
 *
 *
 *  Parameters :
 *  ------------
 *  'src_ip_adr':  senders IP-address in BE-format
 *  'src_port':    senders UDP-port in BE-format
 *  'src_mac_adr': senders MAC-address
 *  'udp_sp_hd':   UDP defined handle, identifying our SAP (i.e. port)
 *  'length':      length of received ethernet frame
 *  'data':        pointer for received ethernet frame (in driver's space)
 *
 *
 *  Return values :
 *  ---------------
 *  'ERROR_NET_TFTPC_SAP_ERROR_STATE'
 *  'ERROR_NET_TFTPC_INVALID_TID'
 *  'ERROR_NET_TFTP_FILE_TOO_BIG'
 *  'ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER'
 *  'ERROR_NET_TFTPC_CONGESTION'
 *  'OK' 
 *
 ************************************************************************/
static
UINT32 NET_TFTPC_receive_data_byte( 
                          UINT32     src_ip_adr,  /* IP BE-format   */
                          UINT16     src_port,    /* port BE-format */
                          t_mac_addr *src_mac_adr,/* MAC            */
                          UINT32     udp_sp_hd,   /* handle of open */
                          UINT32     length,      /* total length   */
                          UINT8      *data     )  /* start of frame */
{
    UINT32            rcode = OK ;
    UINT8             *p ;
    UINT16            opcode ;
    UINT16            tmp2 ;
    UINT32            tftp_data_length ;
    int               index ;

#ifdef NET_DEBUG
    int               i ;
    printf("NET_TFTPC_receive_data_byte\n") ;
    printf("src_ip_adr = %x\n", (unsigned)BE32_TO_CPU(src_ip_adr) ) ;
    printf("src_port   = %d\n", BE16_TO_CPU(src_port) ) ;
    printf("length = %d\n", length ) ;
    printf("src MAC = " ) ;
    p = (UINT8*) src_mac_adr ;
    for (i=0; i<6; i++)
    {
        printf(" %02x",*p++ ) ;
    }
    printf("\n" ) ;
#endif


    /* We already registered an ERROR */
    if (sap_context.sap_state == TFTPC_SAP_STATE_ERROR)
    {
        /* do nothing */
        return(ERROR_NET_TFTPC_SAP_ERROR_STATE) ;
    }

    /* get TFTP-'opcode'-field of this received frame */
    p = (data + TFTP_HEADER_BASE + TFTP_HEADER_OPCODE) ;
    get2( p, opcode) ;
    opcode = BE16_TO_CPU( opcode ) ;
    get2( p, tmp2) ;

    /* NOTE: 'tmp2' now keeps block-number or host-error */
    tmp2 = BE16_TO_CPU( tmp2 ) ;

    if (opcode != TFTP_HEADER_OPCODE_DATA)
    {
        /* We expect nothing else than 'DATA' or 'ERROR' */

        /* check for HOST error */
        if (opcode == TFTP_HEADER_OPCODE_ERROR)
        {
            if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_BLOCK)
            {
                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );
                sap_context.error_cause = ERROR_NET_TFTP_READ_ERROR ;
            }

            if ( (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_NEXT_BLOCK) ||
                 (sap_context.sap_state == TFTPC_SAP_STATE_CONGESTION)      ||
                 (sap_context.sap_state == TFTPC_SAP_STATE_LAST_BLOCK_READ) )
            {
                /* validate 'src_port' */
                if (sap_context.dst_port != src_port)
                {
                    /* Ignore packet, and reply 'unknown TID' */
                    rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                                  src_port,
                                                  sap_context.udp_sp_hd ) ;

                    /* do nothing */
                    return( ERROR_NET_TFTPC_INVALID_TID ) ;
                }

                /* save HOST error */
                sap_context.host_error = tmp2 ;
                data[length] = 0 ;
                strcpy( sap_context.host_error_msg,
                        &data[TFTP_HEADER_BASE + TFTP_HEADER_ERRORMSG] ) ;
                sprintf( net_diag_msg, "Host returned: ErrorCode = %d, ErrorMsg = %s",
                         sap_context.host_error, 
                         sap_context.host_error_msg  );

                sap_context.error_cause = ERROR_NET_TFTP_DATA_ERROR ;
            }

            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
        }

        /* return, despite if it is not DATA */
        return( sap_context.error_cause ) ;
    }

    /* We have 'DATA' */

    /* Check for 1. Block data and save 'src_port', and src-MAC */
    if (sap_context.sap_state == TFTPC_SAP_STATE_WAIT_FIRST_BLOCK)
    {
        sap_context.dst_port = src_port ;
        memcpy( &sap_context.dst_mac_adr, 
                src_mac_adr,
                SYS_MAC_ADDR_SIZE ) ;
    }
    else
    {
        /* validate 'src_port' */
        if (sap_context.dst_port != src_port)
        {
            /* Ignore packet, and reply 'unknown TID' */
            rcode = NET_TFTPC_unknownTID( src_ip_adr,
                                          src_port,
                                          sap_context.udp_sp_hd ) ;

            /* do nothing */
            return( ERROR_NET_TFTPC_INVALID_TID ) ;
        }
    }

    /* 'TID' is valid */

    /* Validate block number */
    if (tmp2 != (sap_context.block_number + 1) )
    {
        if (tmp2 == TFTP_HEADER_MAX_BLOCKNUMBER)
        {
            /* New state: 'ERROR' */
            sap_context.sap_state = TFTPC_SAP_STATE_ERROR ;
            sap_context.error_cause = ERROR_NET_TFTP_FILE_TOO_BIG ;

            return( ERROR_NET_TFTP_FILE_TOO_BIG ) ;
        }

        /* To avoid the "Sorcerer's Apprentice: do nothing and let timer 
           retransmit if host doesn't */
        return( ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER ) ;

#ifdef NET_ENABLE_TFTPC_INVALID_BLOCK_NUMBER
        if (tmp2 == sap_context.block_number)
        {
            /* This is the case, when an 'ACK' has got lost */

            /* Send 'ACK' */
            rcode = NET_TFTPC_sendACK( &sap_context ) ;
            return( rcode ) ;
        }
        else
        {
            /* New state: 'ERROR' */
            sap_context.sap_state   = TFTPC_SAP_STATE_ERROR ;
            sap_context.error_cause = ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER ;

            return( ERROR_NET_TFTPC_INVALID_BLOCK_NUMBER ) ;
        }
#endif

    }

    /* Try to save data into our buffer */
    tftp_data_length = length - (MAC_HEADER_SIZE + IP_HEADER_SIZE +
                                 UDP_HEADER_SIZE + TFTP_HEADER_DATA) ;

    /* No data, re-evaluate buffer status */
    index = sap_context.write_idx ;
    if ( sap_context.buf[ index ].ctrl == BUFFER_STATE_EMPTY )
    {
        /* we seem to have an empty buffer */

        /* OK, update block number */
        sap_context.block_number = tmp2 ;

        /* save data */
        memcpy( sap_context.buf[ index ].blink,
                &data[ TFTP_HEADER_BASE + TFTP_HEADER_DATA ],
                tftp_data_length ) ;

        /* update context */
        sap_context.buf[ index ].count = tftp_data_length ;
        sap_context.buf[ index ].ctrl  = BUFFER_STATE_FULL ;

        /* proceed to next buffer */
        sap_context.write_idx++ ;
        if (sap_context.write_idx == TFTPC_SAP_BUFFER_COUNT)
        {
            sap_context.write_idx = 0 ;
        }
        index = sap_context.write_idx ;

        if (tftp_data_length < TFTP_HEADER_DATA_MAX_SIZE)
        {
            /* Set state 'LAST_BLOCK_READ' */
            sap_context.sap_state = TFTPC_SAP_STATE_LAST_BLOCK_READ ;
        }
        else
        {
            
            if ( sap_context.buf[ index ].ctrl == BUFFER_STATE_EMPTY )
            {
                /* We have space for the next block, send ACK now */
                rcode = NET_TFTPC_sendACK( &sap_context ) ;
            
                /* Set timer and retry count */

                /* get time now (unit is seconds since 1.1.1970) */
                NET_gettime( &sap_context.timeout ) ;
                sap_context.timeout    = sap_context.timeout + TFTPC_SAP_TIMEOUT ;
                sap_context.retry_left = TFTPC_SAP_MAX_RETRY_COUNT ;

                /* Set state 'WAIT_NEXT_BLOCK' */
                sap_context.sap_state = TFTPC_SAP_STATE_WAIT_NEXT_BLOCK ;
            }
            else
            {
                /* We need to delay our ACK */

                /* Set state 'CONGESTION' */
                sap_context.sap_state = TFTPC_SAP_STATE_CONGESTION ;
            }
        }
        rcode = OK ;
    }
    else
    {
        /* Set state 'CONGESTION' */
        sap_context.sap_state = TFTPC_SAP_STATE_CONGESTION ;
        rcode = ERROR_NET_TFTPC_CONGESTION ;
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_init_sap
 *  Description :
 *  -------------
 *  Initialize the TFTPC SAP context.
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
static
UINT32 NET_TFTPC_init_sap( t_tftpc_sap_context *context )
{
    UINT32 rcode ;

    /* initialize SAP */


    /* bind this SAP to UDP and let UDP allocate a unique port */
    rcode = NET_UDP_open( UDP_SAP_UNDEFINED,
                          NET_TFTPC_receive_data_byte,
                          &(context->udp_sp_hd) ) ;

    if (rcode == OK)
    {
        /* New state 'BOUND_TO_UDP' */
        context->sap_state  = TFTPC_SAP_STATE_BOUND_TO_UDP ;

        /* Reset context */
        rcode = NET_TFTPC_reset_sap( context, NET_TFTPC_receive_data_byte ) ;
    }
    else
    {
        /* New state 'IDLE' */
        context->sap_state  = TFTPC_SAP_STATE_IDLE ;
    }

    return( rcode ) ;
}

/************************************************************************
 *
 *                          NET_TFTPC_reset_sap
 *  Description :
 *  -------------
 *  Reset the TFTPC SAP context.
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
static
UINT32 NET_TFTPC_reset_sap( t_tftpc_sap_context *context,
                            t_udp_usr_receive   rec )
{
    UINT32 rcode = OK ;
    int    i ;

    /* Reset SAP */
    if (context->sap_state != TFTPC_SAP_STATE_IDLE)
    {
        /* re-allocate a new UDP SAP */
        IF_ERROR( (rcode),
                  (NET_UDP_close( context->udp_sp_hd ) ) )
        IF_ERROR( (rcode),
                  (NET_UDP_open( UDP_SAP_UNDEFINED,
                                 rec,
                                 &(context->udp_sp_hd) ) ) )

        /* clean up context */
        context->dst_ip_adr     = IP_ADDR_UNDEFINED ;
        context->dst_port       = CPU_TO_BE16( UDP_SAP_TFTP ) ;
        memcpy( context->dst_mac_adr, &mac_undefined_adr, SYS_MAC_ADDR_SIZE) ;
        context->block_number   = 0 ;
        context->timeout        = 0 ;
        context->retry_left     = 0 ;
        context->error_cause    = OK ;
        context->send_rc        = OK ;
        context->host_error     = 0 ;
        context->host_error_msg = context->data ;
        context->filename       = context->data ;
        context->pread          = NULL ;
        context->puser          = NULL ;
        context->user_size      = 0 ;
        context->bytes_left     = 0 ;
        context->read_idx       = 0 ;
        context->write_idx      = 0 ;
        context->sap_state      = TFTPC_SAP_STATE_BOUND_TO_UDP ;
    }

    for (i=0; i<TFTPC_SAP_BUFFER_COUNT; i++)
    {
        context->buf[i].ctrl  = BUFFER_STATE_EMPTY ;
        context->buf[i].count = 0 ;
    }

    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_sendDATA
 *  Description :
 *  -------------
 *  Send 'DATA' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendDATA( t_tftpc_sap_context *context )
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_DATA_MAX_HEADER_SIZE] ;
    UINT8   *p ;
    UINT32  length ;

    /* build the 'TFTP-DATA' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    putbe2( TFTP_HEADER_OPCODE_DATA , p )

    putbe2( context->block_number, p )

    if ( context->bytes_left > TFTP_HEADER_DATA_MAX_SIZE )
    {
        length = TFTP_HEADER_DATA_MAX_SIZE ;
    }
    else
    {
        length = context->bytes_left ;
    }

    if (length > 0)
    {
        memcpy( &data[TFTP_HEADER_BASE + TFTP_HEADER_DATA], context->puser, length ) ;
    }
    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_DATA + length ;

#ifdef NET_DEBUG
    printf("NET_TFTPC_sendDATA\n") ;
    printf("dst_ip_adr = %x\n", (unsigned)(BE32_TO_CPU(context->dst_ip_adr)) ) ;
    printf("dst_port   = %d\n", (unsigned)(BE16_TO_CPU(context->dst_port)) ) ;
    printf("block nr. = %d\n", (unsigned) context->block_number ) ;
    printf("bytes_left = %d\n", (unsigned) context->bytes_left ) ;
    printf("length = %d\n", (unsigned) length ) ;
#endif

    /* request UDP to 'send' TFTP DATA */
    context->send_rc = NET_UDP_send( context->dst_ip_adr, /* destination ip address (BE) */
                                     context->dst_port,   /* destination (TFTP) port (BE)*/
                                     NULL,                /* optional MAC destination    */
                                     context->udp_sp_hd,  /* handle returned of 'open'   */
                                     length,              /* total length of frame to send */
                                     data  );             /* pointer to start of frame   */

#ifdef NET_DEBUG
    {
        int i;
        p = data ;
        printf("NET_MAC_send:") ;
        for (i=0; i<length; i++)
        {
            if (i%16 == 0)
            {
                printf("\n %02x",*p++ ) ;
            }
            else
            {
                printf(" %02x",*p++ ) ;
            }
        }
        printf("\n" ) ;
    }
#endif

    return( context->send_rc ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_sendRRQ
 *  Description :
 *  -------------
 *  Send 'RRQ' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendRRQ( t_tftpc_sap_context *context )
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_RRQ_MAX_HEADER_SIZE] ;
    UINT8   *p ;
    UINT32  length ;

    /* build the 'TFTP-RRQ' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    putbe2( TFTP_HEADER_OPCODE_RRQ , p )

    /* copy filename + mode */
    length = strlen(context->filename) + TFTP_HEADER_MODE_OCTET_SIZE ;
    memcpy( &data[TFTP_HEADER_BASE + TFTP_HEADER_FILENAME], context->filename, length ) ;
    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_FILENAME + length ;

    /* request UDP to 'send' TFTP RRQ */
    context->send_rc = NET_UDP_send( context->dst_ip_adr, /* destination ip address (BE) */
                                     context->dst_port,   /* destination (TFTP) port (BE)*/
                                     NULL,                /* optional MAC destination    */
                                     context->udp_sp_hd,  /* handle returned of 'open'   */
                                     length,              /* total length of frame to send */
                                     data  );             /* pointer to start of frame   */

    return( context->send_rc ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_sendWRQ
 *  Description :
 *  -------------
 *  Send 'WRQ' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendWRQ( t_tftpc_sap_context *context )
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_WRQ_MAX_HEADER_SIZE] ;
    UINT8   *p ;
    UINT32  length ;

    /* build the 'TFTP-WRQ' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    putbe2( TFTP_HEADER_OPCODE_WRQ , p )

    /* copy filename + mode */
    length = strlen(context->filename) + TFTP_HEADER_MODE_OCTET_SIZE ;
    memcpy( &data[TFTP_HEADER_BASE + TFTP_HEADER_FILENAME], context->filename, length ) ;
    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_FILENAME + length ;

    /* request UDP to 'send' TFTP WRQ */
    context->send_rc = NET_UDP_send( context->dst_ip_adr, /* destination ip address (BE) */
                                     context->dst_port,   /* destination (TFTP) port (BE)*/
                                     NULL,                /* optional MAC destination    */
                                     context->udp_sp_hd,  /* handle returned of 'open'   */
                                     length,              /* total length of frame to send */
                                     data  );             /* pointer to start of frame   */

    return( context->send_rc ) ;
}


/************************************************************************
 *
 *                          NET_TFTPC_sendACK
 *  Description :
 *  -------------
 *  Send 'ACK' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendACK( t_tftpc_sap_context *context )
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_ACK_HEADER_SIZE] ;
    UINT8   *p ;
    UINT32  length ;

    /* build the 'TFTP-ACK' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    putbe2( TFTP_HEADER_OPCODE_ACK , p )

    putbe2( context->block_number , p )

    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_ACK_HEADER_SIZE ;

    /* request UDP to 'send' TFTP ACK */
    context->send_rc = NET_UDP_send( context->dst_ip_adr, /* destination ip address (BE) */
                                     context->dst_port,   /* destination (TFTP) port (BE)*/
                                     &(context->dst_mac_adr),/* optional MAC destination    */
                                     context->udp_sp_hd,  /* handle returned of 'open'   */
                                     length,              /* total length of frame to send */
                                     data  );             /* pointer to start of frame   */


    return( context->send_rc ) ;
}

/************************************************************************
 *
 *                          NET_TFTPC_unknownTID
 *  Description :
 *  -------------
 *  Send 'ERROR' packet, cause = 'unknown TID'
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
************************************************************************/
static
UINT32 NET_TFTPC_unknownTID(
                     UINT32     dst_ip_adr, /* dest. ip address (BE) */
                     UINT16     dst_port,   /* dest. port (BE)       */
                     UINT32     sp_hd   )   /* handle from 'open'    */
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_ERROR_HEADER_SIZE] ;
    UINT8   *p ;
    UINT16  tmp2 ;
    UINT32  length ;
    UINT32  rc ;

    /* build the 'TFTP-ERROR' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    tmp2 = CPU_TO_BE16( TFTP_HEADER_OPCODE_ERROR ) ;
    put2( tmp2, p )

    /* set error code = 'unknown transfer ID' */
    tmp2 = CPU_TO_BE16( TFTP_HEADER_ERROR_UNKNOWN_TID ) ;
    put2( tmp2, p )

    data[ (TFTP_HEADER_BASE + TFTP_HEADER_ERROR_HEADER_SIZE) - 1 ] = 0 ;

    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_ERROR_HEADER_SIZE ;

    /* request UDP to 'send' TFTP ERROR */
    rc = NET_UDP_send( dst_ip_adr, /* destination ip address (BE) */
                       dst_port,   /* destination (TFTP) port (BE)*/
                       NULL,       /* optional MAC destination    */
                       sp_hd,      /* handle returned of 'open'   */
                       length,     /* total length of frame to send */
                       data  );    /* pointer to start of frame   */

    return( rc ) ;
}


#if 0 /* currently not called */
/************************************************************************
 *
 *                          NET_TFTPC_sendERROR
 *  Description :
 *  -------------
 *  Send 'ERROR' of this SAP context.
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
************************************************************************/
static
UINT32 NET_TFTPC_sendERROR( t_tftpc_sap_context *context, UINT16 error )
{
    UINT8   data[TFTP_HEADER_BASE+TFTP_HEADER_ERROR_HEADER_SIZE] ;
    UINT8   *p ;
    UINT32  length ;

    /* build the 'TFTP-ERROR' header */
    p = &data[ TFTP_HEADER_BASE + TFTP_HEADER_OPCODE ] ;
    putbe2( TFTP_HEADER_OPCODE_ERROR , p )

    putbe2( error , p )

    data[ (TFTP_HEADER_BASE + TFTP_HEADER_ERROR_HEADER_SIZE) - 1 ] = 0 ;

    length = MAC_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + TFTP_HEADER_ERROR_HEADER_SIZE ;

    /* request UDP to 'send' TFTP ERROR */
    context->send_rc = NET_UDP_send( context->dst_ip_adr, /* destination ip address (BE) */
                                     context->dst_port,   /* destination (TFTP) port (BE)*/
                                     NULL,                /* optional MAC destination    */
                                     context->udp_sp_hd,  /* handle returned of 'open'   */
                                     length,              /* total length of frame to send */
                                     data  );             /* pointer to start of frame   */


    return( context->send_rc ) ;
}
#endif

#ifdef NET_DEBUG
static
void NET_TFTPC_dumpcontext( void )
{

    int i ;
    printf("sap_state    = %x\n", sap_context.sap_state ) ;
    printf("udp_sp_hd    = %x\n", sap_context.udp_sp_hd  ) ;
    printf("dst_ip_adr   = %x\n", sap_context.dst_ip_adr ) ;
    printf("dst_port     = %d\n", BE16_TO_CPU(sap_context.dst_port)   ) ;
    printf("block_number = %d\n", sap_context.block_number ) ;
    printf("timeout      = %x\n", sap_context.timeout ) ;
    printf("retry_left   = %x\n", sap_context.retry_left ) ;
    printf("error_cause  = %x\n", sap_context.error_cause  ) ;
    printf("send_rc      = %x\n", sap_context.send_rc ) ;
    printf("pread      = %x\n", (unsigned)sap_context.pread ) ;
    printf("puser      = %x\n", (unsigned)sap_context.puser ) ;
    printf("user_size  = %x\n", sap_context.user_size ) ;
    printf("bytes_left = %x\n", sap_context.bytes_left ) ;
    for (i=0; i<TFTPC_SAP_BUFFER_COUNT; i++)
    {
        printf(" %d  %x", i, (unsigned)sap_context.buf[i].ctrl ) ;
        printf(" %x", (unsigned)sap_context.buf[i].count ) ;
        printf(" %x\n", (unsigned)sap_context.buf[i].blink ) ;
    }
}
#endif
