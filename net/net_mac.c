/************************************************************************
 *
 *      net_mac.c
 *
 *      The 'NET_MAC' module implements the MAC-layer services to provide
 *      service access points, which are linked with the 'type'-field
 *      of the MAC-header.
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

/* generic device driver interface */
#include <io_api.h>

/* drivers */
#include <lan_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

#define MAC_BUFFER_SIZE      0x600 
#define MAC_CACHE_LINE_SIZE  0x20


/************************************************************************
 *      Public variables
 ************************************************************************/

/* MAC undefined address */
t_mac_addr mac_undefined_adr = { 0, 0, 0, 0, 0, 0 } ;

/* MAC broadcast address */
t_mac_addr mac_broadcast_adr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } ;

/* Our stations MAC-address */
t_mac_addr mac_station_adr ;


/************************************************************************
 *      Static variables
 ************************************************************************/

/* Global state of MAC */
static UINT32 NET_MAC_state = MAC_STATE_CLOSED ;

/* SAP table */
static t_mac_sap_context sap_context[MAC_SAP_COUNT] ;

/* Major device number of EN0 */
static UINT32 en0_major_device = 0 ;

/* Ref. for loopback buffer */
static void *lbbuf ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          NET_MAC_receive
 *  Description :
 *  -------------
 *  In "RX-frame receive int. mode", this function is called from 
 *  the LAN-driver ISR, whenever it receives an Ethernet frame. 
 *
 *  In "RX-frame receive polled mode", this function will still
 *  be called from the LAN-driver, but in this case, the call
 *  gets started when NET_MAC_poll() calls the LAN-driver 'read'
 *  entry.
 *
 *  It must check for any SAP's registered to handle the addressed
 *  SAP and if one found, the registered receive handler is called.
 *
 *
 *
 *  Parameters :
 *  ------------
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
UINT32 NET_MAC_receive( UINT32 length,
                        UINT8  *data    ) ;


/************************************************************************
 *
 *                          NET_MAC_alloc_sap
 *  Description :
 *  -------------
 *  Allocate a MAC SAP, register user context and return a sp-handle.
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'mac_sap_id',          IN,    value of MAC-'type' to bind for
 *  'mac_usr_receive',     IN,    user-receive function to be registered
 *  'mac_sp_hd',           OUT,   handle of MAC to be used by user by call
 *                                of 'send'
 *
 *
 *  Return values :
 *  ---------------
 * 'ERROR_NET_MAC_NO_FREE_SAP', No SAP entry could be allocated
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
UINT32 NET_MAC_alloc_sap(  
                      UINT16            mac_sap_id,      /* 'type'-field of mac-header
                                                            always in 'cpu' format          */
                      t_mac_usr_receive mac_usr_receive, /* user defined receive handler    */ 
                      UINT32            *mac_sp_hd ) ;   /* service provider defined handle */

/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          NET_MAC_init
 *  Description :
 *  -------------
 *  Initialize the MAC module.
 *
 *
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
UINT32 NET_MAC_init(  void )
{
    t_sys_malloc  mem ;
    UINT32        rcode ;
    int           i ;

    /* initialize MAC SAP table */
    for ( i = 0; i <MAC_SAP_COUNT ; i++)
    {
        sap_context[i].mac_sap_state   = MAC_SAP_STATE_CLOSED ;
        sap_context[i].mac_sap         = MAC_SAP_UNDEFINED ;
        sap_context[i].mac_usr_receive = NULL ;
    }

    /* get a local copy of this stations MAC address */
    IF_ERROR( (rcode),
              (SYSCON_read( SYSCON_COM_EN0_MAC_ADDR_ID,
                            &(mac_station_adr),
                            sizeof(mac_station_adr)) ) )

    /* get a EN0 major device number */
    IF_ERROR( (rcode),
              (SYSCON_read( SYSCON_COM_EN0_MAJOR_DEVICE_ID,
                            &(en0_major_device),
                            sizeof(en0_major_device)) ) )

    /* allocate data for loopback buffer */
    mem.size     = MAC_BUFFER_SIZE ;
    mem.boundary = MAC_CACHE_LINE_SIZE ;
    mem.memory   = &(lbbuf) ;
    rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
    if (rcode != OK)
    {
            return( rcode ) ;
    }
    lbbuf = (void*) KSEG0( (UINT32)lbbuf ) ;

    /* MAC-module has now been initialized */
    NET_MAC_state = MAC_STATE_INITED ;

    return(OK) ;
}


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
                      UINT32            *mac_sp_hd )     /* service provider defined handle */
{
    t_LAN_OPEN_desc desc ;
    UINT32          rcode = OK ;

    switch (NET_MAC_state)
    {
        case MAC_STATE_CLOSED:
                rcode = ERROR_NET_MAC_NOT_INITIALIZED ;
            break;

        case MAC_STATE_INITED:
                /* try allocating a SAP */
                rcode = NET_MAC_alloc_sap( mac_sap_id,
                                           mac_usr_receive,
                                           mac_sp_hd ) ;
                if ( rcode == OK ) 
                {
                    /* By first 'open', we need to open driver */
                    desc.receive = NET_MAC_receive ;
                    rcode = IO_open( en0_major_device, 0, &desc )  ;
                    if ( rcode == OK )
                    {
                        NET_MAC_state = MAC_STATE_OPEN ;
                    }
                }
            break;

        case MAC_STATE_OPEN:
                /* try allocating a SAP */
                rcode = NET_MAC_alloc_sap( mac_sap_id,
                                           mac_usr_receive,
                                           mac_sp_hd ) ;
            break;

        default:
                /* we should never arrive here */
                rcode = ERROR_NET_MAC_FATAL_STATE ;
            break;

    }

    return( rcode ) ;
}



/************************************************************************
 *
 *                          NET_MAC_close
 *  Description :
 *  -------------
 *
 *  Close a MAC-SAP.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  mac_sp_hd     service provider defined handle
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'ERROR_NET_MAC_INVALID_HANDLE', invalid handle
 *  'OK'(=0),                       SAP has been closed 
 *
 *
 ************************************************************************/
UINT32 NET_MAC_close( UINT32 mac_sp_hd ) /* service provider defined handle */
{
    /* validate handle */
    IF_UPPER( ERROR_NET_MAC_INVALID_HANDLE, mac_sp_hd, MAC_SAP_COUNT)

    /* close SAP */
    sap_context[mac_sp_hd].mac_sap_state   = MAC_SAP_STATE_CLOSED ;
    sap_context[mac_sp_hd].mac_sap         = MAC_SAP_UNDEFINED ;
    sap_context[mac_sp_hd].mac_usr_receive = NULL ;

    return(OK) ;
}


/************************************************************************
 *
 *                          NET_MAC_send
 *  Description :
 *  -------------
 *
 *  Request the MAC module to send a frame, linked
 *  to a certain 'SAP' to a specified MAC-destination address.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'mac_sp_hd',           OUT,   handle to lookup a registered SAP context
 *  'dst_adr',             OUT,   destination mac address
 *  'length',              OUT,   length of frame to send
 *  'data',                OUT,   address to start of frame to be send
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
                      UINT8             *data  )         /* pointer to start of frame       */
{
    UINT32  old_ie ;
    t_LAN_IO_desc desc ;
    UINT32        rcode ;
    UINT8         *p ;
    UINT16        sap ;

    /* validate handle */
    IF_UPPER( ERROR_NET_MAC_INVALID_HANDLE, mac_sp_hd, MAC_SAP_COUNT)

    /* validate state */
    if ( sap_context[mac_sp_hd].mac_sap_state != MAC_SAP_STATE_OPEN )
    {
        return( ERROR_NET_MAC_SAP_CLOSED ) ;
    }

    /* we have a registered SAP context */
    sap = sap_context[mac_sp_hd].mac_sap ; /* get SAP (in 'BE' format) */

    /* Fill-in MAC header of frame */
    p = (data + MAC_HEADER_BASE) ;
    put6( dst_adr[0], p ) ;         /* fill-in destination MAC-address */
    put6( mac_station_adr[0], p ) ; /* fill-in our MAC-address         */
    put2( sap, p ) ;                /* fill-in SAP                     */

#ifdef NET_DEBUG
    {
        int i ;
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

    /* Check for broadcast */
    if ( !memcmp( data, mac_broadcast_adr, SYS_MAC_ADDR_SIZE ) )
    {
        /* This is for our station; i.e. loopback */
        old_ie = sys_disable_int() ;
        memcpy( lbbuf, data, length ) ;
        rcode = NET_MAC_receive( length, lbbuf ) ;
        if(old_ie) sys_enable_int();
    }

    /* Check for loopback */
    if ( !memcmp( data, mac_station_adr,   SYS_MAC_ADDR_SIZE ) )
    {
        /* This is for our station; i.e. loopback */
        old_ie = sys_disable_int() ;
        memcpy( lbbuf, data, length ) ;
        rcode = NET_MAC_receive( length, lbbuf ) ;
        if(old_ie) sys_enable_int();
    }
    else
    {
        /* Call driver to send this frame */
        desc.length = length ;
        desc.data   = data ;
        rcode = IO_write( en0_major_device, 0, &desc )  ;
    }

    /* return completion */
    return( rcode ) ;
}


/************************************************************************
 *
 *                          NET_MAC_poll
 *  Description :
 *  -------------
 *  Poll the MAC module to receive any frame from driver.
 *  (Note: is not used by ISR-handled reception.)
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'
 *
 ************************************************************************/
UINT32 NET_MAC_poll(  void )
{
    UINT32 rcode ;
    t_LAN_IO_desc desc ;

    /* we just call the driver to poll and let the driver
       call 'NET_MAC_receive' in case of any received frame             */
    desc.length = 0 ;
    desc.data   = NULL ;
    rcode = IO_read( en0_major_device, 0, &desc )  ;
    return( rcode ) ;
}


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          NET_MAC_receive
 *  Description :
 *  -------------
 *  In "RX-frame receive int. mode", this function is called from 
 *  the LAN-driver ISR, whenever it receives an Ethernet frame. 
 *
 *  In "RX-frame receive polled mode", this function will still
 *  be called from the LAN-driver, but in this case, the call
 *  gets started when NET_MAC_poll() calls the LAN-driver 'read'
 *  entry.
 *
 *  It must check for any SAP's registered to handle the addressed
 *  SAP and if one found, the registered receive handler is called.
 *
 *
 *
 *  Parameters :
 *  ------------
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
UINT32 NET_MAC_receive( UINT32 length,
                        UINT8  *data    )
{
    UINT32            rcode ;
    t_mac_usr_receive receive ;
    UINT8             *p ;
    UINT16            sap ;
    int               i ;

#ifdef NET_DEBUG
    p = data ;
    printf("NET_MAC_receive:") ;
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
#endif

    /* validate frame size */
    if (length > MAC_MAX_FRAME_SIZE)
    {
        return( ERROR_NET_MAC_INVALID_LENGTH ) ;
    }

    /* get sap of this received frame, and keep it in 'BE'-format */
    p = (data + MAC_HEADER_BASE + MAC_HEADER_TYPE) ;
    get2( p, sap) ;


    /* search the SAP table to match this 'sap' */
    for ( i = 0; i <MAC_SAP_COUNT ; i++ )
    {
        if ( sap_context[i].mac_sap == sap )
        {
            if ( sap_context[i].mac_sap_state == MAC_SAP_STATE_OPEN )
            {
                /* we have a registered SAP context, call user */
                receive    = sap_context[i].mac_usr_receive ;
                p = (data + MAC_HEADER_BASE + MAC_HEADER_SRCADR) ;
                rcode = (*receive)( (t_mac_addr*) p,
                                    length,
                                    data ) ;
#ifdef NET_DEBUG
                printf("NET_MAC_receive, rcode = %x\n", rcode ) ;
#endif

                return( rcode ) ;
            }
        }
    }
    return( ERROR_NET_MAC_NO_SAP ) ;
}


/************************************************************************
 *
 *                          NET_MAC_alloc_sap
 *  Description :
 *  -------------
 *  Allocate a MAC SAP, register user context and return a sp-handle.
 * 
 *
 *  Parameters :
 *  ------------
 *  'mac_sap_id',          IN,    value of MAC-'type' to bind for
 *  'mac_usr_receive',     IN,    user-receive function to be registered
 *  'mac_sp_hd',           OUT,   handle of MAC to be used by user by call
 *                                of 'send'
 *
 *
 *  Return values :
 *  ---------------
 * 'ERROR_NET_MAC_NO_FREE_SAP', No SAP entry could be allocated
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
UINT32 NET_MAC_alloc_sap(
                      UINT16            mac_sap_id,      /* 'type'-field of mac-header
                                                            always in 'cpu' format          */
                      t_mac_usr_receive mac_usr_receive, /* user defined receive handler    */ 
                      UINT32            *mac_sp_hd )     /* service provider defined handle */
{
    int i ;

    /* allocate a free MAC SAP table entry */
    for ( i = 0; i <MAC_SAP_COUNT ; i++)
    {
        if ( sap_context[i].mac_sap_state == MAC_SAP_STATE_CLOSED )
        {
            /* save SAP context in allocated entry */
            sap_context[i].mac_sap_state   = MAC_SAP_STATE_OPEN ;
            sap_context[i].mac_sap         = CPU_TO_BE16( mac_sap_id ) ;
            sap_context[i].mac_usr_receive = mac_usr_receive ;
            *mac_sp_hd                     = i ;
            return( OK ) ;
        }
    }

    /* we should not arrive here */
    return( ERROR_NET_MAC_NO_FREE_SAP ) ;
}
