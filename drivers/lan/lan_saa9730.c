
/************************************************************************
 *
 *      LAN_SAA9730.c
 *
 *      The 'LAN_SAA9730' module implements the LAN_SAA9730 
 *      device driver as an IO device with following services:
 *
 *        1) init  serial device:  configure and initialize LAN 
 *                                 SAA9730 driver
 *        2) open  serial device:  register receive handler
 *        3) close serial device:  not used
 *        4) read  serial device:  poll for received frame
 *        5) write serial device:  request frame to be transmitted
 *        6) ctrl  serial device:  not used
 *
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
 *      Include files
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <io_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <lan_api.h>
#include <lan_saa9730_api.h>


/* #define  ETH_DEBUG 1 */
/************************************************************************
 *  Constant Definitions
*************************************************************************/

typedef enum
{
    LAN_MINOR_SAA9730_DEVICE_1 = 0,    /* The one and only SAA9730
		                                  LAN controller               */

	/******* ADD NEW MINOR DEVICES JUST BEFORE THIS LINE ONLY ********/

    LAN_MINOR_SAA9730_DEVICES
} t_LAN_MINOR_SAA9730_devices ;


/* PHY definitions for Basic registers QS6612 */

#define PHY_CONTROL                     0
#define PHY_STATUS                      1
#define PHY_REG31                       31

#define PHY_CONTROL_RESET               (1 << 15)
#define PHY_CONTROL_AUTO_NEG            (1 << 12)
#define PHY_CONTROL_RESTART_AUTO_NEG    (1 <<  9)

#define PHY_STATUS_LINK_UP              (1 << 2)

#define PHY_REG31_OPMODE_SHF             2
#define PHY_REG31_OPMODE_MSK             (7 << PHY_REG31_OPMODE_SHF)
#define OPMODE_AUTONEGOTIATE             0
#define OPMODE_10BASET_HALFDUPLEX        1
#define OPMODE_100BASEX_HALFDUPLEX       2
#define OPMODE_REPEATER_MODE             3
#define OPMODE_UNDEFINED                 4
#define OPMODE_10BASET_FULLDUPLEX        5
#define OPMODE_100BASEX_FULLDUPLEX       6
#define OPMODE_ISOLATE                   7

#define LAN_SAA9730_MD_CA_BUSY          (1 << 11)
#define LAN_SAA9730_MD_CA_WRITE         (1 << 10)


#define PHY_ADDRESS                     QS6612_PHY_ADDRESS
#define QS6612_PHY_ADDRESS              0


/* Number of 6-byte entries in the CAM */
#define LAN_SAA9730_CAM_ENTRIES                       10
#define	LAN_SAA9730_CAM_DWORDS                        15

/* TX and RX packet size: fixed to 2048 bytes,
   according to HW requirements                                        */
#define LAN_SAA9730_PACKET_SIZE                       2048

/* 
   Number of TX buffers = number of RX buffers = 2,
   which is fixed according to HW requirements                         */
#define LAN_SAA9730_BUFFERS                           2

/* 
   Number of RX packets per RX buffer                                  */
#define LAN_SAA9730_RCV_Q_SIZE                        16

/* 
   Number of TX packets per TX buffer                                  */
#define LAN_SAA9730_TXM_Q_SIZE                        1


/* Minimum pcket size */
#define LAN_SAA9730_MIN_PACKET_SIZE                   60

/* owner ship bit */
#define LAN_SAA9730_BLOCK_OWNED_BY_SYSTEM             0
#define LAN_SAA9730_BLOCK_OWNED_BY_HARDWARE           1

/* Default Rcv interrupt count */
#define LAN_SAA9730_DEFAULT_RCV_INTERRUPT_CNT         4

/* Default maxium transmit retry */
#define LAN_SAA9730_DEFAULT_MAX_TXM_RETRY	      16

/* Default time out value */
#define LAN_SAA9730_DEFAULT_TIME_OUT_CNT              200

/* MAX map registers */
#define LAN_SAA9730__MAX_MAP_REGISTERS		      64

/* Defines used by Interrupt code */
#define  LAN_SAA9730_DMA_PACKET_SIZE                  2048
#define  LAN_SAA9730_VALID_PACKET                     0xC0000000
#define  LAN_SAA9730_FRAME_TYPELEN_OFFSET             12
#define  LAN_SAA9730_ETH_MIN_FRAME_SIZE               60
#define  LAN_SAA9730_DEST_ADDR_SIZE                   6
#define  LAN_SAA9730_SRC_ADDR_SIZE                    6
#define  LAN_SAA9730_TYPE_LEN_SIZE                    2

/* MAC receive error */
#define  LAN_SAA9730_MAC_GOOD_RX                      (0x00004000) << 11
#define  LAN_SAA9730_MAC_RCV_ALIGN_ERROR              (0x00000100) << 11
#define  LAN_SAA9730_MAC_RCV_CRC_ERROR                (0x00000200) << 11
#define  LAN_SAA9730_MAC_RCV_OVERFLOW                 (0x00000400) << 11

/* This number is arbitrary and can be increased if needed */
#define LAN_SAA9730_MAX_MULTICAST_ADDRESSES           20 

/************************************************************************
 *  Macro Definitions
*************************************************************************/


#define IF_ERROR( completion, function )  \
{ \
  completion = function ; \
  if ( completion != OK )  \
{ \
    return( completion ) ; \
} \
}

// These macros are machine dependent.  
// They convert the big endian representation to the little endian
// format, used by the DMA-engine of the LAN-controller on the PCI-bus.
// Convert by BE:
#ifdef EL
#define cpu_to_le32( value ) (value)
#define le32_to_cpu( value ) (value)
#else
#define cpu_to_le32( value ) ( (                ((UINT32)value)  << 24) |   \
                               ((0x0000FF00UL & ((UINT32)value)) <<  8) |   \
                               ((0x00FF0000UL & ((UINT32)value)) >>  8) |   \
                               (                ((UINT32)value)  >> 24)   )
#define le32_to_cpu( value ) cpu_to_le32( value )
#endif
// Convert by LE:
#ifdef EB
#define cpu_to_be32( value ) (value)
#define be32_to_cpu( value ) (value)
#else
#define cpu_to_be32( value ) ( (                ((UINT32)value)  << 24) |   \
                               ((0x0000FF00UL & ((UINT32)value)) <<  8) |   \
                               ((0x00FF0000UL & ((UINT32)value)) >>  8) |   \
                               (                ((UINT32)value)  >> 24)   )
#define be32_to_cpu( value ) cpu_to_be32( value )
#endif

/************************************************************************
 *  Type Definitions
*************************************************************************/



/*
 *      Network device statistics. 
 */
typedef struct net_device_stats
{
        unsigned long   rx_packets;             /* total packets received       */
        unsigned long   tx_packets;             /* total packets transmitted    */
        unsigned long   rx_bytes;               /* total bytes received         */
        unsigned long   tx_bytes;               /* total bytes transmitted      */
        unsigned long   rx_errors;              /* bad packets received         */
        unsigned long   tx_errors;              /* packet transmit problems     */
        unsigned long   multicast;              /* multicast packets received   */
        unsigned long   collisions;

        /* detailed rx_errors: */
        unsigned long   rx_zero_length_errors;
        unsigned long   rx_buffer_length_errors;
        unsigned long   rx_length_errors;
        unsigned long   rx_crc_errors;          /* recved pkt with crc error    */
        unsigned long   rx_frame_errors;        /* recv'd frame alignment error */
        unsigned long   rx_fifo_errors;         /* recv'r fifo overrun          */

        /* detailed tx_errors */
        unsigned long   tx_aborted_errors;
        unsigned long   tx_carrier_errors;
        unsigned long   tx_fifo_errors;
        unsigned long   tx_heartbeat_errors;
        unsigned long   tx_window_errors;
        unsigned long   tx_timeout_errors;

} t_net_device_stats ;


/*
   Device context for a SAA9730 LAN controller 
                                                                        */
typedef struct LAN_SAA9730_device
{
	/* pointer for the SAA9730 LAN controller register set */
	UINT32                     *p9730Regs;

	/*  TRUE if the next buffer to write is RxBuffA,  FALSE if RxBuffB. */
	UINT8                      NextRcvToUseIsA;

	/* Rcv buffer Index */
	UINT8                      NextRcvPacketIndex;

	/* index of next packet to use in that buffer */
	UINT8                      NextTxmPacketIndex;	

	/* Tx buffer index */
	UINT8                      NextTxmBufferIndex;

	UINT8                      RcvAIndex; /* index into RcvBufferSpace[] for Blk A */
	UINT8                      RcvBIndex; /* index into RcvBufferSpace[] for Blk B */

	UINT32                     TxmBuffer[LAN_SAA9730_BUFFERS][LAN_SAA9730_TXM_Q_SIZE];
	UINT32                     RcvBuffer[LAN_SAA9730_BUFFERS][LAN_SAA9730_RCV_Q_SIZE];

	UINT16                     PhysicalAddress[LAN_SAA9730_CAM_ENTRIES][3];

        /* network statistics */
        t_net_device_stats         status ;

} t_LAN_SAA9730_device ;


/************************************************************************
 *  LAN SAA9730: Relative Register Addresses
*************************************************************************/


#define LAN_TXBUFA_OFS          0x20400 /* TX buffer A register         */
#define LAN_TXBUFB_OFS          0x20404 /* TX buffer B register         */
#define LAN_RXBUFA_OFS          0x20408 /* RX buffer A register         */
#define LAN_RXBUFB_OFS          0x2040C /* RX buffer B register         */
#define LAN_PCKCNT_OFS          0x20410 /* Packet count register        */
#define LAN_OK2USE_OFS          0x20414 /* OK-to-use register           */
#define LAN_DMACTL_OFS          0x20418 /* DMA control register         */
#define LAN_TIMOUT_OFS          0x2041C /* Time out register            */
#define LAN_DMASTA_OFS          0x20420 /* DMA status register          */
#define LAN_DMATST_OFS          0x20424 /* DMA loop back register       */
#define LAN_PAUSE_OFS           0x20430 /* Pause count register         */
#define LAN_REMPAUSE_OFS        0x20434 /* Remote Pause count register  */
#define LAN_MACCTL_OFS          0x20440 /* MAC control register         */
#define LAN_CAMCTL_OFS          0x20444 /* CAM control register         */
#define LAN_TXCTL_OFS           0x20448 /* TX control register          */
#define LAN_TXSTA_OFS           0x2044C /* TX status register           */
#define LAN_RXCTL_OFS           0x20450 /* RX control register          */
#define LAN_RXSTA_OFS           0x20454 /* RX status register           */
#define LAN_MDDATA_OFS          0x20458 /* PHY management data register */
#define LAN_MDCTL_OFS           0x2045C /* PHY management control reg.  */
#define LAN_CAMADR_OFS          0x20460 /* CAM address register         */
#define LAN_CAMDAT_OFS          0x20464 /* CAM data register            */
#define LAN_CAMENA_OFS          0x20468 /* CAM enable register          */

#define LAN_DBGRXS_OFS          0x20508 /* DEBUG: RX-state machine      */

/************************************************************************
 *  LAN SAA9730: Register field encodings
*************************************************************************/



/******** reg: TXBUFA ********/

/* field: BUF */
#define LAN_TXBUFA_BUF_SHF              11
#define LAN_TXBUFA_BUF_MSK              (MSK(21) << LAN_TXBUFA_BUF_SHF)


/******** reg: TXBUFB ********/

/* field: BUF */
#define LAN_TXBUFB_BUF_SHF              11
#define LAN_TXBUFB_BUF_MSK              (MSK(21) << LAN_TXBUFB_BUF_SHF)


/******** reg: RXBUFB ********/

/* field: BUF */
#define LAN_RXBUFB_BUF_SHF              11
#define LAN_RXBUFB_BUF_MSK              (MSK(21) << LAN_RXBUFB_BUF_SHF)


/******** reg: RXBUFB ********/

/* field: BUF */
#define LAN_RXBUFB_BUF_SHF              11
#define LAN_RXBUFB_BUF_MSK              (MSK(21) << LAN_RXBUFB_BUF_SHF)


/******** reg: PCKCNT ********/

/* field: TXA */
#define LAN_PCKCNT_TXA_SHF              24
#define LAN_PCKCNT_TXA_MSK              (MSK(8) << LAN_PCKCNT_TXA_SHF)

/* field: TXB */
#define LAN_PCKCNT_TXB_SHF              16
#define LAN_PCKCNT_TXB_MSK              (MSK(8) << LAN_PCKCNT_TXB_SHF)

/* field: RXA */
#define LAN_PCKCNT_RXA_SHF              8
#define LAN_PCKCNT_RXA_MSK              (MSK(8) << LAN_PCKCNT_RXA_SHF)

/* field: RXB */
#define LAN_PCKCNT_RXB_SHF              0
#define LAN_PCKCNT_RXB_MSK              (MSK(8) << LAN_PCKCNT_RXB_SHF)


/******** reg: OK2USE ********/

/* field: TXA */
#define LAN_OK2USE_TXA_SHF              3
#define LAN_OK2USE_TXA_MSK              (MSK(1) << LAN_OK2USE_TXA_SHF)
#define LAN_OK2USE_TXA_SET              LAN_OK2USE_TXA_MSK

/* field: TXB */
#define LAN_OK2USE_TXB_SHF              2
#define LAN_OK2USE_TXB_MSK              (MSK(1) << LAN_OK2USE_TXB_SHF)
#define LAN_OK2USE_TXB_SET              LAN_OK2USE_TXB_MSK

/* field: RXA */
#define LAN_OK2USE_RXA_SHF              1
#define LAN_OK2USE_RXA_MSK              (MSK(1) << LAN_OK2USE_RXA_SHF)
#define LAN_OK2USE_RXA_SET              LAN_OK2USE_RXA_MSK

/* field: RXB */
#define LAN_OK2USE_RXB_SHF              0
#define LAN_OK2USE_RXB_MSK              (MSK(1) << LAN_OK2USE_RXB_SHF)
#define LAN_OK2USE_RXB_SET              LAN_OK2USE_RXB_MSK


/******** reg: DMACTL ********/

/* field: BLKINT */
#define LAN_DMACTL_BLKINT_SHF           31
#define LAN_DMACTL_BLKINT_MSK           (MSK(1) << LAN_DMACTL_BLKINT_SHF)
#define LAN_DMACTL_BLKINT_SET           LAN_DMACTL_BLKINT_MSK

/* field: MAXXFER */
#define LAN_DMACTL_MAXXFER_SHF          18
#define LAN_DMACTL_MAXXFER_MSK          (MSK(2) << LAN_DMACTL_MAXXFER_SHF)
#define MAXXFER_ANY                     0
#define MAXXFER_8                       1
#define MAXXFER_32                      2
#define MAXXFER_64                      3

/* field: ENDIAN */
#define LAN_DMACTL_ENDIAN_SHF           16
#define LAN_DMACTL_ENDIAN_MSK           (MSK(2) << LAN_DMACTL_ENDIAN_SHF)
#define ENDIAN_LITTLE                   0
#define ENDIAN_2143                     1
#define ENDIAN_4321                     2

/* field: RXINTCNT */
#define LAN_DMACTL_RXINTCNT_SHF         8
#define LAN_DMACTL_RXINTCNT_MSK         (MSK(8) << LAN_DMACTL_RXINTCNT_SHF)

/* field: ENTX */
#define LAN_DMACTL_ENTX_SHF             7
#define LAN_DMACTL_ENTX_MSK             (MSK(1) << LAN_DMACTL_ENTX_SHF)
#define LAN_DMACTL_ENTX_SET             LAN_DMACTL_ENTX_MSK

/* field: ENRX */
#define LAN_DMACTL_ENRX_SHF             6
#define LAN_DMACTL_ENRX_MSK             (MSK(1) << LAN_DMACTL_ENRX_SHF)
#define LAN_DMACTL_ENRX_SET             LAN_DMACTL_ENRX_MSK

/* field: RXFULL */
#define LAN_DMACTL_RXFULL_SHF           5
#define LAN_DMACTL_RXFULL_MSK           (MSK(1) << LAN_DMACTL_RXFULL_SHF)
#define LAN_DMACTL_RXFULL_SET           LAN_DMACTL_RXFULL_MSK

/* field: RXTOINT */
#define LAN_DMACTL_RXTOINT_SHF          4
#define LAN_DMACTL_RXTOINT_MSK          (MSK(1) << LAN_DMACTL_RXTOINT_SHF)
#define LAN_DMACTL_RXTOINT_SET          LAN_DMACTL_RXTOINT_MSK

/* field: RXINT */
#define LAN_DMACTL_RXINT_SHF            3
#define LAN_DMACTL_RXINT_MSK            (MSK(1) << LAN_DMACTL_RXINT_SHF)
#define LAN_DMACTL_RXINT_SET            LAN_DMACTL_RXINT_MSK

/* field: TXINT */
#define LAN_DMACTL_TXINT_SHF            2
#define LAN_DMACTL_TXINT_MSK            (MSK(1) << LAN_DMACTL_TXINT_SHF)
#define LAN_DMACTL_TXINT_SET            LAN_DMACTL_TXINT_MSK

/* field: MACTXINT */
#define LAN_DMACTL_MACTXINT_SHF         1
#define LAN_DMACTL_MACTXINT_MSK         (MSK(1) << LAN_DMACTL_MACTXINT_SHF)
#define LAN_DMACTL_MACTXINT_SET         LAN_DMACTL_MACTXINT_MSK

/* field: MACRXINT */
#define LAN_DMACTL_MACRXINT_SHF         0
#define LAN_DMACTL_MACRXINT_MSK         (MSK(1) << LAN_DMACTL_MACRXINT_SHF)
#define LAN_DMACTL_MACRXINT_SET         LAN_DMACTL_MACRXINT_MSK

/******** reg: TIMOUT ********/

/* no fields; one 32 bit register */

/******** reg: DMASTA ********/


/* field: TXABADR */
#define LAN_DMASTA_TXABADR_SHF          19
#define LAN_DMASTA_TXABADR_MSK          (MSK(1) << LAN_DMASTA_TXABADR_SHF)
#define LAN_DMASTA_TXABADR_SET          LAN_DMASTA_TXABADR_MSK

/* field: TXBBADR */
#define LAN_DMASTA_TXBBADR_SHF          18
#define LAN_DMASTA_TXBBADR_MSK          (MSK(1) << LAN_DMASTA_TXBBADR_SHF)
#define LAN_DMASTA_TXBBADR_SET          LAN_DMASTA_TXBBADR_MSK

/* field: RXABADR */
#define LAN_DMASTA_RXABADR_SHF          17
#define LAN_DMASTA_RXABADR_MSK          (MSK(1) << LAN_DMASTA_RXABADR_SHF)
#define LAN_DMASTA_RXABADR_SET          LAN_DMASTA_RXABADR_MSK

/* field: RXBBADR */
#define LAN_DMASTA_RXBBADR_SHF          16
#define LAN_DMASTA_RXBBADR_MSK          (MSK(1) << LAN_DMASTA_RXBBADR_SHF)
#define LAN_DMASTA_RXBBADR_SET          LAN_DMASTA_RXBBADR_MSK

/* field: RXPCKCNT */
#define LAN_DMASTA_RXPCKCNT_SHF         8
#define LAN_DMASTA_RXPCKCNT_MSK         (MSK(8) << LAN_DMASTA_RXPCKCNT_SHF)

/* field: TXMACBUSY */
#define LAN_DMASTA_TXMACBUSY_SHF        7
#define LAN_DMASTA_TXMACBUSY_MSK        (MSK(1) << LAN_DMASTA_TXMACBUSY_SHF)
#define LAN_DMASTA_TXMACBUSY_SET        LAN_DMASTA_TXMACBUSY_MSK

/* field: RXAFULL */
#define LAN_DMASTA_RXAFULL_SHF          6
#define LAN_DMASTA_RXAFULL_MSK          (MSK(1) << LAN_DMASTA_RXAFULL_SHF)
#define LAN_DMASTA_RXAFULL_SET          LAN_DMASTA_RXAFULL_MSK

/* field: RXBFULL */
#define LAN_DMASTA_RXBFULL_SHF          5
#define LAN_DMASTA_RXBFULL_MSK          (MSK(1) << LAN_DMASTA_RXBFULL_SHF)
#define LAN_DMASTA_RXBFULL_SET          LAN_DMASTA_RXBFULL_MSK

/* field: RXTOINT */
#define LAN_DMASTA_RXTOINT_SHF          4
#define LAN_DMASTA_RXTOINT_MSK          (MSK(1) << LAN_DMASTA_RXTOINT_SHF)
#define LAN_DMASTA_RXTOINT_SET          LAN_DMASTA_RXTOINT_MSK

/* field: RXINT */
#define LAN_DMASTA_RXINT_SHF            3
#define LAN_DMASTA_RXINT_MSK            (MSK(1) << LAN_DMASTA_RXINT_SHF)
#define LAN_DMASTA_RXINT_SET            LAN_DMASTA_RXINT_MSK

/* field: TXINT */
#define LAN_DMASTA_TXINT_SHF            2
#define LAN_DMASTA_TXINT_MSK            (MSK(1) << LAN_DMASTA_TXINT_SHF)
#define LAN_DMASTA_TXINT_SET            LAN_DMASTA_TXINT_MSK

/* field: MACTXINT */
#define LAN_DMASTA_MACTXINT_SHF         1
#define LAN_DMASTA_MACTXINT_MSK         (MSK(1) << LAN_DMASTA_MACTXINT_SHF)
#define LAN_DMASTA_MACTXINT_SET         LAN_DMASTA_MACTXINT_MSK

/* field: MACRXINT */
#define LAN_DMASTA_MACRXINT_SHF         0
#define LAN_DMASTA_MACRXINT_MSK         (MSK(1) << LAN_DMASTA_MACRXINT_SHF)
#define LAN_DMASTA_MACRXINT_SET         LAN_DMASTA_MACRXINT_MSK

/******** reg: DMATST ********/

/* field: LPBACK */
#define LAN_DMATST_LPBACK_SHF           24
#define LAN_DMATST_LPBACK_MSK           (MSK(1) << LAN_DMATST_LPBACK_SHF)
#define LAN_DMATST_LPBACK_SET           LAN_DMATST_LPBACK_MSK

/* field: RESET */
#define LAN_DMATST_RESET_SHF            0
#define LAN_DMATST_RESET_MSK            (MSK(1) << LAN_DMATST_RESET_SHF)
#define LAN_DMATST_RESET_SET            LAN_DMATST_RESET_MSK

/******** reg: MACCTL ********/

/* field: MISSRINT */
#define LAN_MACCTL_MISSRINT_SHF         13
#define LAN_MACCTL_MISSRINT_MSK         (MSK(1) << LAN_MACCTL_MISSRINT_SHF)
#define LAN_MACCTL_MISSRINT_SET         LAN_MACCTL_MISSRINT_MSK

/* field: MISSROLL */
#define LAN_MACCTL_MISSROLL_SHF         10
#define LAN_MACCTL_MISSROLL_MSK         (MSK(1) << LAN_MACCTL_MISSROLL_SHF)
#define LAN_MACCTL_MISSROLL_SET         LAN_MACCTL_MISSROLL_MSK

/* field: LOOP10 */
#define LAN_MACCTL_LOOP10_SHF           7
#define LAN_MACCTL_LOOP10_MSK           (MSK(1) << LAN_MACCTL_LOOP10_SHF)
#define LAN_MACCTL_LOOP10_SET           LAN_MACCTL_LOOP10_MSK

/* field: CONMODE */
#define LAN_MACCTL_CONMODE_SHF          5
#define LAN_MACCTL_CONMODE_MSK          (MSK(2) << LAN_MACCTL_CONMODE_SHF)
#define CONMODE_AUTOMATIC               0
#define CONMODE_FORCE_10MB              1
#define CONMODE_FORCE_MII               2

/* field: LPBACK */
#define LAN_MACCTL_LPBACK_SHF           4
#define LAN_MACCTL_LPBACK_MSK           (MSK(1) << LAN_MACCTL_LPBACK_SHF)
#define LAN_MACCTL_LPBACK_SET           LAN_MACCTL_LPBACK_MSK

/* field: FULLDUP */
#define LAN_MACCTL_FULLDUP_SHF          3
#define LAN_MACCTL_FULLDUP_MSK          (MSK(1) << LAN_MACCTL_FULLDUP_SHF)
#define LAN_MACCTL_FULLDUP_SET          LAN_MACCTL_FULLDUP_MSK

/* field: RESET */
#define LAN_MACCTL_RESET_SHF            2
#define LAN_MACCTL_RESET_MSK            (MSK(1) << LAN_MACCTL_RESET_SHF)
#define LAN_MACCTL_RESET_SET            LAN_MACCTL_RESET_MSK

/* field: HALTNOW */
#define LAN_MACCTL_HALTNOW_SHF          1
#define LAN_MACCTL_HALTNOW_MSK          (MSK(1) << LAN_MACCTL_HALTNOW_SHF)
#define LAN_MACCTL_HALTNOW_SET          LAN_MACCTL_HALTNOW_MSK

/* field: HALTREQ */
#define LAN_MACCTL_HALTREQ_SHF          0
#define LAN_MACCTL_HALTREQ_MSK          (MSK(1) << LAN_MACCTL_HALTREQ_SHF)
#define LAN_MACCTL_HALTREQ_SET          LAN_MACCTL_HALTREQ_MSK

/******** reg: CAMCTL ********/

/* field: COMPARE */
#define LAN_CAMCTL_COMPARE_SHF          4
#define LAN_CAMCTL_COMPARE_MSK          (MSK(1) << LAN_CAMCTL_COMPARE_SHF)
#define LAN_CAMCTL_COMPARE_SET          LAN_CAMCTL_COMPARE_MSK

/* field: NEGATE */
#define LAN_CAMCTL_NEGATE_SHF           3
#define LAN_CAMCTL_NEGATE_MSK           (MSK(1) << LAN_CAMCTL_NEGATE_SHF)
#define LAN_CAMCTL_NEGATE_SET           LAN_CAMCTL_NEGATE_MSK

/* field: BROADCAST */
#define LAN_CAMCTL_BROADCAST_SHF        2
#define LAN_CAMCTL_BROADCAST_MSK        (MSK(1) << LAN_CAMCTL_BROADCAST_SHF)
#define LAN_CAMCTL_BROADCAST_SET        LAN_CAMCTL_BROADCAST_MSK

/* field: MULTICAST */
#define LAN_CAMCTL_MULTICAST_SHF        1
#define LAN_CAMCTL_MULTICAST_MSK        (MSK(1) << LAN_CAMCTL_MULTICAST_SHF)
#define LAN_CAMCTL_MULTICAST_SET        LAN_CAMCTL_MULTICAST_MSK

/* field: UNICAST */
#define LAN_CAMCTL_UNICAST_SHF          0
#define LAN_CAMCTL_UNICAST_MSK          (MSK(1) << LAN_CAMCTL_UNICAST_SHF)
#define LAN_CAMCTL_UNICAST_SET          LAN_CAMCTL_UNICAST_MSK

/******** reg: TXCTL  ********/

/* field: COMPLINT */
#define LAN_TXCTL_COMPLINT_SHF          14
#define LAN_TXCTL_COMPLINT_MSK          (MSK(1) << LAN_TXCTL_COMPLINT_SHF)
#define LAN_TXCTL_COMPLINT_SET          LAN_TXCTL_COMPLINT_MSK

/* field: TXPARINT */
#define LAN_TXCTL_TXPARINT_SHF          13
#define LAN_TXCTL_TXPARINT_MSK          (MSK(1) << LAN_TXCTL_TXPARINT_SHF)
#define LAN_TXCTL_TXPARINT_SET          LAN_TXCTL_TXPARINT_MSK

/* field: LATECOLLINT */
#define LAN_TXCTL_LATECOLLINT_SHF       12
#define LAN_TXCTL_LATECOLLINT_MSK       (MSK(1) << LAN_TXCTL_LATECOLLINT_SHF)
#define LAN_TXCTL_LATECOLLINT_SET       LAN_TXCTL_LATECOLLINT_MSK

/* field: EXCOLLINT */
#define LAN_TXCTL_EXCOLLINT_SHF         11
#define LAN_TXCTL_EXCOLLINT_MSK         (MSK(1) << LAN_TXCTL_EXCOLLINT_SHF)
#define LAN_TXCTL_EXCOLLINT_SET         LAN_TXCTL_EXCOLLINT_MSK

/* field: CARRIERINT */
#define LAN_TXCTL_CARRIERINT_SHF        10
#define LAN_TXCTL_CARRIERINT_MSK        (MSK(1) << LAN_TXCTL_CARRIERINT_SHF)
#define LAN_TXCTL_CARRIERINT_SET        LAN_TXCTL_CARRIERINT_MSK

/* field: DEFERINT */
#define LAN_TXCTL_DEFERINT_SHF          9
#define LAN_TXCTL_DEFERINT_MSK          (MSK(1) << LAN_TXCTL_DEFERINT_SHF)
#define LAN_TXCTL_DEFERINT_SET          LAN_TXCTL_DEFERINT_MSK

/* field: UNDERINT */
#define LAN_TXCTL_UNDERINT_SHF          8
#define LAN_TXCTL_UNDERINT_MSK          (MSK(1) << LAN_TXCTL_UNDERINT_SHF)
#define LAN_TXCTL_UNDERINT_SET          LAN_TXCTL_UNDERINT_MSK

/* field: MII10 */
#define LAN_TXCTL_MII10_SHF             7
#define LAN_TXCTL_MII10_MSK             (MSK(1) << LAN_TXCTL_MII10_SHF)
#define LAN_TXCTL_MII10_SET             LAN_TXCTL_MII10_MSK

/* field: SDPAUSE */
#define LAN_TXCTL_SDPAUSE_SHF           6
#define LAN_TXCTL_SDPAUSE_MSK           (MSK(1) << LAN_TXCTL_SDPAUSE_SHF)
#define LAN_TXCTL_SDPAUSE_SET           LAN_TXCTL_SDPAUSE_MSK

/* field: NOEXDEF */
#define LAN_TXCTL_NOEXDEF_SHF           5
#define LAN_TXCTL_NOEXDEF_MSK           (MSK(1) << LAN_TXCTL_NOEXDEF_SHF)
#define LAN_TXCTL_NOEXDEF_SET           LAN_TXCTL_NOEXDEF_MSK

/* field: FBACK */
#define LAN_TXCTL_FBACK_SHF             4
#define LAN_TXCTL_FBACK_MSK             (MSK(1) << LAN_TXCTL_FBACK_SHF)
#define LAN_TXCTL_FBACK_SET             LAN_TXCTL_FBACK_MSK

/* field: NOCRC */
#define LAN_TXCTL_NOCRC_SHF             3
#define LAN_TXCTL_NOCRC_MSK             (MSK(1) << LAN_TXCTL_NOCRC_SHF)
#define LAN_TXCTL_NOCRC_SET             LAN_TXCTL_NOCRC_MSK

/* field: NOPAD */
#define LAN_TXCTL_NOPAD_SHF             2
#define LAN_TXCTL_NOPAD_MSK             (MSK(1) << LAN_TXCTL_NOPAD_SHF)
#define LAN_TXCTL_NOPAD_SET             LAN_TXCTL_NOPAD_MSK

/* field: TXHALT */
#define LAN_TXCTL_TXHALT_SHF            1
#define LAN_TXCTL_TXHALT_MSK            (MSK(1) << LAN_TXCTL_TXHALT_SHF)
#define LAN_TXCTL_TXHALT_SET            LAN_TXCTL_TXHALT_MSK

/* field: ENTX */
#define LAN_TXCTL_ENTX_SHF              0
#define LAN_TXCTL_ENTX_MSK              (MSK(1) << LAN_TXCTL_ENTX_SHF)
#define LAN_TXCTL_ENTX_SET              LAN_TXCTL_ENTX_MSK

/******** reg: TXSTA  ********/

/* field: SQERR */
#define LAN_TXSTA_SQERR_SHF             16
#define LAN_TXSTA_SQERR_MSK             (MSK(1) << LAN_TXSTA_SQERR_SHF)
#define LAN_TXSTA_SQERR_SET             LAN_TXSTA_SQERR_MSK

/* field: TXHALTED */
#define LAN_TXSTA_TXHALTED_SHF          15
#define LAN_TXSTA_TXHALTED_MSK          (MSK(1) << LAN_TXSTA_TXHALTED_SHF)
#define LAN_TXSTA_TXHALTED_SET          LAN_TXSTA_TXHALTED_MSK

/* field: COMPLETION */
#define LAN_TXSTA_COMPLETION_SHF        14
#define LAN_TXSTA_COMPLETION_MSK        (MSK(1) << LAN_TXSTA_COMPLETION_SHF)
#define LAN_TXSTA_COMPLETION_SET        LAN_TXSTA_COMPLETION_MSK

/* field: PARITYERR */
#define LAN_TXSTA_PARITYERR_SHF         13
#define LAN_TXSTA_PARITYERR_MSK         (MSK(1) << LAN_TXSTA_PARITYERR_SHF)
#define LAN_TXSTA_PARITYERR_SET         LAN_TXSTA_PARITYERR_MSK

/* field: LATECOLLERR */
#define LAN_TXSTA_LATECOLLERR_SHF       12
#define LAN_TXSTA_LATECOLLERR_MSK       (MSK(1) << LAN_TXSTA_LATECOLLERR_SHF)
#define LAN_TXSTA_LATECOLLERR_SET       LAN_TXSTA_LATECOLLERR_MSK

/* field: WAS10MB */
#define LAN_TXSTA_WAS10MB_SHF           11
#define LAN_TXSTA_WAS10MB_MSK           (MSK(1) << LAN_TXSTA_WAS10MB_SHF)
#define LAN_TXSTA_WAS10MB_SET           LAN_TXSTA_WAS10MB_MSK

/* field: LOSTCARRIER */
#define LAN_TXSTA_LOSTCARRIER_SHF       10
#define LAN_TXSTA_LOSTCARRIER_MSK       (MSK(1) << LAN_TXSTA_LOSTCARRIER_SHF)
#define LAN_TXSTA_LOSTCARRIER_SET       LAN_TXSTA_LOSTCARRIER_MSK

/* field: EXDEFER */
#define LAN_TXSTA_EXDEFER_SHF           9
#define LAN_TXSTA_EXDEFER_MSK           (MSK(1) << LAN_TXSTA_EXDEFER_SHF)
#define LAN_TXSTA_EXDEFER_SET           LAN_TXSTA_EXDEFER_MSK

/* field: UNDERRUN */
#define LAN_TXSTA_UNDERRUN_SHF          8
#define LAN_TXSTA_UNDERRUN_MSK          (MSK(1) << LAN_TXSTA_UNDERRUN_SHF)
#define LAN_TXSTA_UNDERRUN_SET          LAN_TXSTA_UNDERRUN_MSK

/* field: INTERRUPT */
#define LAN_TXSTA_INTERRUPT_SHF         7
#define LAN_TXSTA_INTERRUPT_MSK         (MSK(1) << LAN_TXSTA_INTERRUPT_SHF)
#define LAN_TXSTA_INTERRUPT_SET         LAN_TXSTA_INTERRUPT_MSK

/* field: PAUSED */
#define LAN_TXSTA_PAUSED_SHF            6
#define LAN_TXSTA_PAUSED_MSK            (MSK(1) << LAN_TXSTA_PAUSED_SHF)
#define LAN_TXSTA_PAUSED_SET            LAN_TXSTA_PAUSED_MSK

/* field: DEFERRED */
#define LAN_TXSTA_DEFERRED_SHF          5
#define LAN_TXSTA_DEFERRED_MSK          (MSK(1) << LAN_TXSTA_DEFERRED_SHF)
#define LAN_TXSTA_DEFERRED_SET          LAN_TXSTA_DEFERRED_MSK

/* field: EXCOLL */
#define LAN_TXSTA_EXCOLL_SHF            4
#define LAN_TXSTA_EXCOLL_MSK            (MSK(1) << LAN_TXSTA_EXCOLL_SHF)
#define LAN_TXSTA_EXCOLL_SET            LAN_TXSTA_EXCOLL_MSK

/* field: COLLISIONS */
#define LAN_TXSTA_COLLISIONS_SHF        0
#define LAN_TXSTA_COLLISIONS_MSK        (MSK(4) << LAN_TXSTA_COLLISIONS_SHF)

/******** reg: RXCTL  ********/

/* field: ENGOOD */
#define LAN_RXCTL_ENGOOD_SHF            14
#define LAN_RXCTL_ENGOOD_MSK            (MSK(1) << LAN_RXCTL_ENGOOD_SHF)
#define LAN_RXCTL_ENGOOD_SET            LAN_RXCTL_ENGOOD_MSK

/* field: ENPARITY */
#define LAN_RXCTL_ENPARITY_SHF          13
#define LAN_RXCTL_ENPARITY_MSK          (MSK(1) << LAN_RXCTL_ENPARITY_SHF)
#define LAN_RXCTL_ENPARITY_SET          LAN_RXCTL_ENPARITY_MSK

/* field: ENLONGERR */
#define LAN_RXCTL_ENLONGERR_SHF         11
#define LAN_RXCTL_ENLONGERR_MSK         (MSK(1) << LAN_RXCTL_ENLONGERR_SHF)
#define LAN_RXCTL_ENLONGERR_SET         LAN_RXCTL_ENLONGERR_MSK

/* field: ENOVER */
#define LAN_RXCTL_ENOVER_SHF            10
#define LAN_RXCTL_ENOVER_MSK            (MSK(1) << LAN_RXCTL_ENOVER_SHF)
#define LAN_RXCTL_ENOVER_SET            LAN_RXCTL_ENOVER_MSK

/* field: ENCRCERR */
#define LAN_RXCTL_ENCRCERR_SHF          9
#define LAN_RXCTL_ENCRCERR_MSK          (MSK(1) << LAN_RXCTL_ENCRCERR_SHF)
#define LAN_RXCTL_ENCRCERR_SET          LAN_RXCTL_ENCRCERR_MSK

/* field: ENALIGN */
#define LAN_RXCTL_ENALIGN_SHF           8
#define LAN_RXCTL_ENALIGN_MSK           (MSK(1) << LAN_RXCTL_ENALIGN_SHF)
#define LAN_RXCTL_ENALIGN_SET           LAN_RXCTL_ENALIGN_MSK

/* field: IGNORECRC */
#define LAN_RXCTL_IGNORECRC_SHF         6
#define LAN_RXCTL_IGNORECRC_MSK         (MSK(1) << LAN_RXCTL_IGNORECRC_SHF)
#define LAN_RXCTL_IGNORECRC_SET         LAN_RXCTL_IGNORECRC_MSK

/* field: PASSCTL */
#define LAN_RXCTL_PASSCTL_SHF           5
#define LAN_RXCTL_PASSCTL_MSK           (MSK(1) << LAN_RXCTL_PASSCTL_SHF)
#define LAN_RXCTL_PASSCTL_SET           LAN_RXCTL_PASSCTL_MSK

/* field: STRIPCRC */
#define LAN_RXCTL_STRIPCRC_SHF          4
#define LAN_RXCTL_STRIPCRC_MSK          (MSK(1) << LAN_RXCTL_STRIPCRC_SHF)
#define LAN_RXCTL_STRIPCRC_SET          LAN_RXCTL_STRIPCRC_MSK

/* field: SHORTEN */
#define LAN_RXCTL_SHORTEN_SHF           3
#define LAN_RXCTL_SHORTEN_MSK           (MSK(1) << LAN_RXCTL_SHORTEN_SHF)
#define LAN_RXCTL_SHORTEN_SET           LAN_RXCTL_SHORTEN_MSK

/* field: LONGEN */
#define LAN_RXCTL_LONGEN_SHF            2
#define LAN_RXCTL_LONGEN_MSK            (MSK(1) << LAN_RXCTL_LONGEN_SHF)
#define LAN_RXCTL_LONGEN_SET            LAN_RXCTL_LONGEN_MSK

/* field: RXHALT */
#define LAN_RXCTL_RXHALT_SHF            1
#define LAN_RXCTL_RXHALT_MSK            (MSK(1) << LAN_RXCTL_RXHALT_SHF)
#define LAN_RXCTL_RXHALT_SET            LAN_RXCTL_RXHALT_MSK

/* field: RXEN */
#define LAN_RXCTL_RXEN_SHF              0
#define LAN_RXCTL_RXEN_MSK              (MSK(1) << LAN_RXCTL_RXEN_SHF)
#define LAN_RXCTL_RXEN_SET              LAN_RXCTL_RXEN_MSK

/******** reg: RXSTA  ********/

/* field: HALTED */
#define LAN_RXSTA_HALTED_SHF            15
#define LAN_RXSTA_HALTED_MSK            (MSK(1) << LAN_RXSTA_HALTED_SHF)
#define LAN_RXSTA_HALTED_SET            LAN_RXSTA_HALTED_MSK

/* field: GOOD */
#define LAN_RXSTA_GOOD_SHF              14
#define LAN_RXSTA_GOOD_MSK              (MSK(1) << LAN_RXSTA_GOOD_SHF)
#define LAN_RXSTA_GOOD_SET              LAN_RXSTA_GOOD_MSK

/* field: PARITY */
#define LAN_RXSTA_PARITY_SHF            13
#define LAN_RXSTA_PARITY_MSK            (MSK(1) << LAN_RXSTA_PARITY_SHF)
#define LAN_RXSTA_PARITY_SET            LAN_RXSTA_PARITY_MSK

/* field: LONGERR */
#define LAN_RXSTA_LONGERR_SHF           11
#define LAN_RXSTA_LONGERR_MSK           (MSK(1) << LAN_RXSTA_LONGERR_SHF)
#define LAN_RXSTA_LONGERR_SET           LAN_RXSTA_LONGERR_MSK

/* field: OVERFLOW */
#define LAN_RXSTA_OVERFLOW_SHF          10
#define LAN_RXSTA_OVERFLOW_MSK          (MSK(1) << LAN_RXSTA_OVERFLOW_SHF)
#define LAN_RXSTA_OVERFLOW_SET          LAN_RXSTA_OVERFLOW_MSK

/* field: CRCERR */
#define LAN_RXSTA_CRCERR_SHF            9
#define LAN_RXSTA_CRCERR_MSK            (MSK(1) << LAN_RXSTA_CRCERR_SHF)
#define LAN_RXSTA_CRCERR_SET            LAN_RXSTA_CRCERR_MSK

/* field: ALIGNERR */
#define LAN_RXSTA_ALIGNERR_SHF          8
#define LAN_RXSTA_ALIGNERR_MSK          (MSK(1) << LAN_RXSTA_ALIGNERR_SHF)
#define LAN_RXSTA_ALIGNERR_SET          LAN_RXSTA_ALIGNERR_MSK

/* field: WAS10MB */
#define LAN_RXSTA_WAS10MB_SHF           7
#define LAN_RXSTA_WAS10MB_MSK           (MSK(1) << LAN_RXSTA_WAS10MB_SHF)
#define LAN_RXSTA_WAS10MB_SET           LAN_RXSTA_WAS10MB_MSK

/* field: INTERRUPT */
#define LAN_RXSTA_INTERRUPT_SHF         6
#define LAN_RXSTA_INTERRUPT_MSK         (MSK(1) << LAN_RXSTA_INTERRUPT_SHF)
#define LAN_RXSTA_INTERRUPT_SET         LAN_RXSTA_INTERRUPT_MSK

/* field: CONTROLRCV */
#define LAN_RXSTA_CONTROLRCV_SHF        5
#define LAN_RXSTA_CONTROLRCV_MSK        (MSK(1) << LAN_RXSTA_CONTROLRCV_SHF)
#define LAN_RXSTA_CONTROLRCV_SET        LAN_RXSTA_CONTROLRCV_MSK

/******** reg: MDDATA ********/

/* field: DATA */
#define LAN_MDDATA_DATA_SHF             0
#define LAN_MDDATA_DATA_MSK             (MSK(16) << LAN_MDDATA_DATA_SHF)
#define LAN_MDDATA_DATA_SET             LAN_MDDATA_DATA_MSK

/******** reg: MDCTL  ********/

/* field: PRESUP */
#define LAN_MDCTL_PRESUP_SHF            12
#define LAN_MDCTL_PRESUP_MSK            (MSK(1) << LAN_MDCTL_PRESUP_SHF)
#define LAN_MDCTL_PRESUP_SET            LAN_MDCTL_PRESUP_MSK

/* field: BUSY */
#define LAN_MDCTL_BUSY_SHF              11
#define LAN_MDCTL_BUSY_MSK              (MSK(1) << LAN_MDCTL_BUSY_SHF)
#define LAN_MDCTL_BUSY_SET              LAN_MDCTL_BUSY_MSK

/* field: WR */
#define LAN_MDCTL_WR_SHF                10
#define LAN_MDCTL_WR_MSK                (MSK(1) << LAN_MDCTL_WR_SHF)
#define LAN_MDCTL_WR_SET                LAN_MDCTL_WR_MSK

/* field: PHY */
#define LAN_MDCTL_PHY_SHF               5
#define LAN_MDCTL_PHY_MSK               (MSK(5) << LAN_MDCTL_PHY_SHF)

/* field: ADDR */
#define LAN_MDCTL_ADDR_SHF              0
#define LAN_MDCTL_ADDR_MSK              (MSK(5) << LAN_MDCTL_ADDR_SHF)

/******** reg: CAMADR ********/

/* field: ADDRESS */
#define LAN_CAMADR_ADDRESS_SHF          0
#define LAN_CAMADR_ADDRESS_MSK          (MSK(9) << LAN_CAMADR_ADDRESS_SHF)

/******** reg: CAMDAT ********/

/* 32-bit transparent data register */


/******** reg: CAMENA ********/

/* field: ENABLE */
#define LAN_CAMENA_ENABLE_SHF          0
#define LAN_CAMENA_ENABLE_MSK          (MSK(22) << LAN_CAMENA_ENABLE_SHF)

/******** reg: DBGRXS ********/

/* field: RXPI */
#define LAN_DBGRXS_RXPI_SHF            16
#define LAN_DBGRXS_RXPI_MSK            (MSK(10) << LAN_DBGRXS_RXPI_SHF)
#define LAN_DBGRXS_RXPI_ERROR          1

/* field: RXDII */
#define LAN_DBGRXS_RXDII_SHF           0
#define LAN_DBGRXS_RXDII_MSK           (MSK(9) << LAN_DBGRXS_RXDII_SHF)
#define LAN_DBGRXS_RXDII_ERROR         8

/************************************************************************
 *  LAN SAA9730: Packet Control & Status Word, Relative Address
*************************************************************************/

#define TXPACKET_CONTROL_OFS    0x00000 /* TX-Packet, Control word      */
#define TXPACKET_STATUS_OFS     0x00000 /* TX-Packet, Status  word      */

#define RXPACKET_CONTROL_OFS    0x00000 /* RX-Packet, Control word      */
#define RXPACKET_STATUS_OFS     0x00000 /* RX-Packet, Status  word      */

/************************************************************************
 *  LAN SAA9730: Packet Control & Status Word, Field encodings
*************************************************************************/

/******** reg: TXPACKET_CONTROL ********/

/* field: FLAG */
#define TXPACKET_CONTROL_FLAG_SHF       30
#define TXPACKET_CONTROL_FLAG_MSK       (MSK(2) << TXPACKET_CONTROL_FLAG_SHF)
#define TX_EMPTY                        0
#define TX_READY                        2
#define TX_HWDONE                       3

/* field: COMPLETION */
#define TXPACKET_CONTROL_COMPLETION_SHF 29
#define TXPACKET_CONTROL_COMPLETION_MSK (MSK(1) << TXPACKET_CONTROL_COMPLETION_SHF)

/* field: NOCRC */
#define TXPACKET_CONTROL_NOCRC_SHF      28
#define TXPACKET_CONTROL_NOCRC_MSK      (MSK(1) << TXPACKET_CONTROL_NOCRC_SHF)

/* field: NOPAD */
#define TXPACKET_CONTROL_NOPAD_SHF      27
#define TXPACKET_CONTROL_NOPAD_MSK      (MSK(1) << TXPACKET_CONTROL_NOPAD_SHF)

/* field: SIZE */
#define TXPACKET_CONTROL_SIZE_SHF       0
#define TXPACKET_CONTROL_SIZE_MSK       (MSK(11) << TXPACKET_CONTROL_SIZE_SHF)


/******** reg: TXPACKET_STATUS  ********/

/* field: FLAG */
#define TXPACKET_STATUS_FLAG_SHF        30
#define TXPACKET_STATUS_FLAG_MSK        (MSK(2) << TXPACKET_STATUS_FLAG_SHF)

/* field: SQERR */
#define TXPACKET_STATUS_SQERR_SHF       27
#define TXPACKET_STATUS_SQERR_MSK       (MSK(1) << TXPACKET_STATUS_SQERR_SHF)

/* field: TXHALTED */
#define TXPACKET_STATUS_TXHALTED_SHF    26
#define TXPACKET_STATUS_TXHALTED_MSK    (MSK(1) << TXPACKET_STATUS_TXHALTED_SHF)

/* field: COMPLETION */
#define TXPACKET_STATUS_COMPLETION_SHF  25
#define TXPACKET_STATUS_COMPLETION_MSK  (MSK(1) << TXPACKET_STATUS_COMPLETION_SHF)

/* field: PARITYERR */
#define TXPACKET_STATUS_PARITYERR_SHF   24
#define TXPACKET_STATUS_PARITYERR_MSK   (MSK(1) << TXPACKET_STATUS_PARITYERR_SHF)

/* field: LATECOLLERR */
#define TXPACKET_STATUS_LATECOLLERR_SHF 23
#define TXPACKET_STATUS_LATECOLLERR_MSK (MSK(1) << TXPACKET_STATUS_LATECOLLERR_SHF)

/* field: WAS10MB */
#define TXPACKET_STATUS_WAS10MB_SHF     22
#define TXPACKET_STATUS_WAS10MB_MSK     (MSK(1) << TXPACKET_STATUS_WAS10MB_SHF)

/* field: LOSTCARRIER */
#define TXPACKET_STATUS_LOSTCARRIER_SHF 21
#define TXPACKET_STATUS_LOSTCARRIER_MSK (MSK(1) << TXPACKET_STATUS_LOSTCARRIER_SHF)

/* field: EXDEFER */
#define TXPACKET_STATUS_EXDEFER_SHF     20
#define TXPACKET_STATUS_EXDEFER_MSK     (MSK(1) << TXPACKET_STATUS_EXDEFER_SHF)

/* field: UNDERRUN */
#define TXPACKET_STATUS_UNDERRUN_SHF    19
#define TXPACKET_STATUS_UNDERRUN_MSK    (MSK(1) << TXPACKET_STATUS_UNDERRUN_SHF)

/* field: COLLISIONS */
#define TXPACKET_STATUS_COLLISIONS_SHF  11
#define TXPACKET_STATUS_COLLISIONS_MSK  (MSK(5) <<TXPACKET_STATUS_COLLISIONS_SHF)

/* field: SIZE */
#define TXPACKET_STATUS_SIZE_SHF        0
#define TXPACKET_STATUS_SIZE_MSK        (MSK(11) << TXPACKET_STATUS_SIZE_SHF)

#define TXPACKET_STATUS_ERROR_MSK       (TXPACKET_STATUS_EXDEFER_MSK     | \
                                         TXPACKET_STATUS_LATECOLLERR_MSK | \
                                         TXPACKET_STATUS_LOSTCARRIER_MSK | \
                                         TXPACKET_STATUS_UNDERRUN_MSK    | \
                                         TXPACKET_STATUS_SQERR_MSK       )

/******** reg: RXPACKET_CONTROL ********/


/******** reg: RXPACKET_STATUS  ********/

/* field: FLAG */
#define RXPACKET_STATUS_FLAG_SHF        30
#define RXPACKET_STATUS_FLAG_MSK        (MSK(2) << RXPACKET_STATUS_FLAG_SHF)
#define RX_NDIS                         0
#define RX_INVALID_STAT                 1
#define RX_READY                        2
#define RX_HWDONE                       3

/* field: GOOD */
#define RXPACKET_STATUS_GOOD_SHF        25
#define RXPACKET_STATUS_GOOD_MSK        (MSK(1) << RXPACKET_STATUS_GOOD_SHF)

/* field: PARITY */
#define RXPACKET_STATUS_PARITY_SHF      24
#define RXPACKET_STATUS_PARITY_MSK      (MSK(1) << RXPACKET_STATUS_PARITY_SHF)

/* field: LONGERR */
#define RXPACKET_STATUS_LONGERR_SHF     22
#define RXPACKET_STATUS_LONGERR_MSK     (MSK(1) << RXPACKET_STATUS_LONGERR_SHF)

/* field: OVERFLOW */
#define RXPACKET_STATUS_OVERFLOW_SHF    21
#define RXPACKET_STATUS_OVERFLOW_MSK    (MSK(1) << RXPACKET_STATUS_OVERFLOW_SHF)

/* field: CRCERR */
#define RXPACKET_STATUS_CRCERR_SHF      20
#define RXPACKET_STATUS_CRCERR_MSK      (MSK(1) << RXPACKET_STATUS_CRCERR_SHF)

/* field: ALIGNERR */
#define RXPACKET_STATUS_ALIGNERR_SHF    19
#define RXPACKET_STATUS_ALIGNERR_MSK    (MSK(1) << RXPACKET_STATUS_ALIGNERR_SHF)

/* field: WAS10MB */
#define RXPACKET_STATUS_WAS10MB_SHF     18
#define RXPACKET_STATUS_WAS10MB_MSK     (MSK(1) << RXPACKET_STATUS_WAS10MB_SHF)

/* field: SIZE */
#define RXPACKET_STATUS_SIZE_SHF        0
#define RXPACKET_STATUS_SIZE_MSK        (MSK(11) << RXPACKET_STATUS_SIZE_SHF)


/* Global driver states */
#define LAN_SAA9730_DRIVER_IS_STOPPED   0x42
#define LAN_SAA9730_DRIVER_IS_STARTED   0x44


/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/
static UINT32 poll_count = 0 ;
static char msg[160] ;

/* Global driver state */
static UINT32 LAN_SAA9730_state = LAN_SAA9730_DRIVER_IS_STOPPED ;

/* MAC broadcast address */
static t_mac_addr mac_broadcast_adr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } ;

/* User registered receive handler */
static UINT32 (*usr_receive)( UINT32 length, UINT8 *data ) = NULL ;

/* Device context per minor device for this driver */
static t_LAN_SAA9730_device   minor_device[LAN_MINOR_SAA9730_DEVICES] ;

/* Dynamic allocated memory */
static UINT32 lan_dma_buffers ;

static char* opmode_info[]= 
{
  /* OPMODE_AUTONEGOTIATE       */ "still in auto-negotiate",
  /* OPMODE_10BASET_HALFDUPLEX  */ "10 Mbit/s, half-duplex",
  /* OPMODE_100BASEX_HALFDUPLEX */ "100 Mbit/s, half-duplex",
  /* OPMODE_REPEATER_MODE       */ "repeater mode",
  /* OPMODE_UNDEFINED           */ "undefined",
  /* OPMODE_10BASET_FULLDUPLEX  */ "10 Mbit/s, full-duplex",
  /* OPMODE_100BASEX_FULLDUPLEX */ "100 Mbit/s, full-duplex",
  /* OPMODE_ISOLATE             */ "PHY/MII isolate, auto-negotiate disabled"
} ;

static bool opmode_is_fullduplex[]= 
{
  /* OPMODE_AUTONEGOTIATE       */ FALSE,
  /* OPMODE_10BASET_HALFDUPLEX  */ FALSE,
  /* OPMODE_100BASEX_HALFDUPLEX */ FALSE,
  /* OPMODE_REPEATER_MODE       */ FALSE,
  /* OPMODE_UNDEFINED           */ FALSE,
  /* OPMODE_10BASET_FULLDUPLEX  */ TRUE,
  /* OPMODE_100BASEX_FULLDUPLEX */ TRUE,
  /* OPMODE_ISOLATE             */ FALSE
} ;

/************************************************************************
 *      Static function prototypes, local helper functions
 ************************************************************************/

/************************************************************************
 *
 *                         LAN_SAA9730_enable_buffer
 *  Description :
 *  -------------
 * 
 *  Enable next receive buffer.
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  buffer_index                  Index for next buffer to be enabled
 *
 *  Return values :
 *  ---------------
 *
 *  -
 *
 ************************************************************************/
static
void LAN_SAA9730_enable_buffer( t_LAN_SAA9730_device *pdevice,
                                UINT32               buffer_index ) ;

/************************************************************************
 *
 *                          LAN_SAA9730_allocate_buffers
 *  Description :
 *  -------------
 *     This routine allocates memory for:
 *
 *     - Receive buffers
 *     - Transmit buffer
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_allocate_buffers( t_LAN_SAA9730_device *pdevice ) ; 

/************************************************************************
 *
 *                          LAN_SAA9730_stop
 *  Description :
 *  -------------
 *   This routine stops the SAA9730 LAN controller
 *   by stoping  DMA transfer and resetting the chip.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_stop( t_LAN_SAA9730_device *pdevice ) ;            

/************************************************************************
 *
 *                          LAN_SAA9730_CAM_init
 *  Description :
 *  -------------
 *    Initialize the SAA9730 LAN CAM
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_CAM_init( t_LAN_SAA9730_device *pdevice ) ;        

/************************************************************************
 *
 *                          LAN_SAA9730_MII_status
 *  Description :
 *  -------------
 *    Display MII status 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_MII_status( t_LAN_SAA9730_device *pdevice ) ;        

/************************************************************************
 *
 *                          LAN_SAA9730_check_MII_opmode
 *  Description :
 *  -------------
 *    Check MII opmode and align with MAC 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_check_MII_opmode( t_LAN_SAA9730_device *pdevice ) ;

/************************************************************************
 *
 *                          LAN_SAA9730_check_RX_controller_state
 *  Description :
 *  -------------
 *    Check LAN controllers RX state machine for any lock-up
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_check_RX_controller_state( t_LAN_SAA9730_device *pdevice ) ;

/************************************************************************
 *
 *                          LAN_SAA9730_CAM_load
 *  Description :
 *  -------------
 *    Load the Ethernet address into SAA9730 CAM registers
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_CAM_load( t_LAN_SAA9730_device *pdevice ) ;        

/************************************************************************
 *
 *                          LAN_SAA9730_DMA_init
 *  Description :
 *  -------------
 *    Setup the SAA9730 DMA registers
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_DMA_init( t_LAN_SAA9730_device *pdevice ) ;        


/************************************************************************
 *
 *                          LAN_SAA9730_start
 *  Description :
 *  -------------
 *    Start the SAA9730 LAN controller operation
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
#ifdef ETH_DEBUG
static
INT32 LAN_SAA9730_start( t_LAN_SAA9730_device *pdevice ) ;           
#endif


/************************************************************************
 *
 *                          LAN_SAA9730_dump_regs
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller registers
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
#ifdef  ETH_DEBUG
static
INT32 LAN_SAA9730_dump_regs( t_LAN_SAA9730_device *pdevice ) ;
#endif

/************************************************************************
 *
 *                          LAN_SAA9730_dump_status
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller statistics
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
void LAN_SAA9730_dump_status( t_LAN_SAA9730_device *pdevice ) ;

/************************************************************************
 *
 *                          LAN_SAA9730_dump_packetstatus
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller packet status
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
#ifdef ETH_DEBUG
static
void LAN_SAA9730_dump_packetstatus( t_LAN_SAA9730_device *pdevice ) ;
#endif

/************************************************************************
 *      Static function prototypes, device driver IO functions
 ************************************************************************/


/************************************************************************
 *
 *                          LAN_SAA9730_init
 *  Description :
 *  -------------
 *  This service initializes the lan driver and configures
 *  the MAC-address for the 'EN0' LAN interface.
 *  The MAC-address is read during 'init' via the 'syscon' parameter:
 *    -'SYSCON_COM_EN0_MAC_ADDR_ID'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   INOUT, not used
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *
 *                          LAN_SAA9730_open
 *  Description :
 *  -------------
 *  This service registers a mac-layer defined receive-handler in the
 *  LAN-drivers ISR-context to allow for interrupt controlled receive
 *  frame processing in the network stack. No external buffer
 *  administration is required as the protocol-layers are responsible for
 *  handling buffer-allocation and data copy-ing to the allocated buffer
 *  payload area. At return from 'receive' handler, the LAN-drivers
 *  local RX-buffer (packet) is released for re-use. After 'open'
 *  has been called, the LAN-driver's 'read' service will call the
 *  registered receive-handler by any frame reception with direct
 *  reference to the LAN-drivers packet space and no read data will be
 *  delivered in the IO-descriptor.
 *
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   IN,    LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_open(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_OPEN_desc *p_param ) ; /* IN: receive handler reference */


/************************************************************************
 *
 *                          LAN_SAA9730_read
 *  Description :
 *  -------------
 *  This service polls the specified LAN interface for any received frame.
 *  If any frame has been received, it will be read into the user allocated
 *  variable, *p_param; if none present, completion = 'ERROR_LAN_NO_FRAME'
 *  will be returned.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_LAN_NO_FRAME':     no frame present on this LAN interface
 *  'ERROR_LAN_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param ) ; /* INOUT: LAN frame           */


/************************************************************************
 *
 *                          LAN_SAA9730_write
 *  Description :
 *  -------------
 *  This service requests transmission of a frame on the LAN interface.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_LAN_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param ) ; /* OUT: frame to transmit     */

/************************************************************************
 *
 *                          LAN_SAA9730_ctrl
 *  Description :
 *  -------------
 *  This service requests special service via 'ctrl'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_CTRL_desc *p_param ) ; /* IN-OUT:                       */



/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          LAN_SAA9730_install
 *  Description :
 *  -------------
 *
 *  Installs the SAA9730 LAN device drivers services in 
 *  the IO system at the reserved device slot, found in the
 *  'sysdev.h' file, which defines all major device numbers.
 *
 *  Note:
 *  This service is the only public declared interface function; all
 *  provided device driver services are static declared, but this
 *  function installs the function pointers in the io-system to
 *  enable the provided public driver services.
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 LAN_SAA9730_install( void )
{
    /* pre-initialize local variables and install device services */
    memset( minor_device, sizeof(minor_device), 0) ;
    IO_install(   SYS_MAJOR_LAN_SAA9730, /* major device number */
           (t_io_service) LAN_SAA9730_init,      /* 'init'  service     */
           (t_io_service) LAN_SAA9730_open,      /* 'open'  service     */
                          NULL,                  /* 'close' service  na */
           (t_io_service) LAN_SAA9730_read,      /* 'read'  service     */
           (t_io_service) LAN_SAA9730_write,     /* 'write' service     */
           (t_io_service) LAN_SAA9730_ctrl ) ;   /* 'ctrl'  service     */

    /* Initialize SAA9730 LAN device driver */
    if( IO_init( SYS_MAJOR_LAN_SAA9730, 0, NULL ) != OK )
    {
        /* Should not happen unless board is defect */
        IO_deinstall( SYS_MAJOR_LAN_SAA9730 );
    }
    return OK;
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *      Implementation : Local helper functions
 ************************************************************************/

/************************************************************************
 *
 *                         LAN_SAA9730_enable_buffer
 *  Description :
 *  -------------
 * 
 *  Enable next receive buffer.
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  buffer_index                  Index for next buffer to be enabled
 *
 *  Return values :
 *  ---------------
 *
 *  -
 *
 ************************************************************************/
static
void LAN_SAA9730_enable_buffer( t_LAN_SAA9730_device *pdevice,
                                UINT32               buffer_index )
{
    UINT32  *pPacket;
    int     j ;

    /* Enable RX buffer */
    for ( j= 0; j < LAN_SAA9730_RCV_Q_SIZE; j++ )
    {
        pPacket     = (UINT32* )pdevice->RcvBuffer[buffer_index][j];
        REG( pPacket, RXPACKET_STATUS ) = cpu_to_le32(( RX_READY << RXPACKET_STATUS_FLAG_SHF )) ;
    }

    REG( pdevice->p9730Regs, LAN_OK2USE ) |= (buffer_index == 0)
                                              ? LAN_OK2USE_RXA_MSK
                                              : LAN_OK2USE_RXB_MSK ;
}

/************************************************************************
 *
 *                          LAN_SAA9730_allocate_buffers
 *  Description :
 *  -------------
 *     This routine allocates memory for:
 *
 *     - Receive buffers
 *     - Transmit buffer
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_allocate_buffers( t_LAN_SAA9730_device *pdevice )  
{
    INT32                rcode ;
    t_sys_malloc         mem ;
    UINT32               Pa;
    UINT32               i, j ;

    if (lan_dma_buffers == 0)
    {
        /* first time initialization */

        /* allocate all RX and TX packets in one chunk on a 2K boundary */
        mem.size     = (LAN_SAA9730_RCV_Q_SIZE  + LAN_SAA9730_TXM_Q_SIZE) *
                        LAN_SAA9730_PACKET_SIZE * LAN_SAA9730_BUFFERS ;
        mem.boundary = LAN_SAA9730_PACKET_SIZE ;
        mem.memory   = (void*)&Pa ;

        /* request RX and TX memory in one chunk */
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
    
        /* cache this reference */
        lan_dma_buffers = Pa ;
    }
    else
    {
        /* not first time initialization */
        Pa = lan_dma_buffers ;
    }

    /* Initialize buffer space */
    mem.size     = (LAN_SAA9730_RCV_Q_SIZE  + LAN_SAA9730_TXM_Q_SIZE) *
                    LAN_SAA9730_PACKET_SIZE * LAN_SAA9730_BUFFERS ;
    Pa = KSEG1(Pa) ;
    memset( (void *)Pa, 0, mem.size ) ;

    /* Init RX buffers */
    for ( i=0; i < 2; i++ )
    {
        for ( j= 0; j < LAN_SAA9730_RCV_Q_SIZE; j++ )
        {
            REG( Pa, RXPACKET_STATUS ) = 0 ;
            pdevice->RcvBuffer[i][j]   = Pa;
            Pa += LAN_SAA9730_PACKET_SIZE ;
        }
    }

    /* Init TX buffers */
    for ( i=0; i < 2; i++ )
    {
        for ( j= 0; j < LAN_SAA9730_TXM_Q_SIZE; j++ )
        {
            REG( Pa, TXPACKET_STATUS ) = cpu_to_le32(( TX_EMPTY << TXPACKET_STATUS_FLAG_SHF )) ;
            pdevice->TxmBuffer[i][j]   = Pa;
            Pa += LAN_SAA9730_PACKET_SIZE ;
        }
    }

    /* Initialize Buffer Index */
    pdevice->NextRcvPacketIndex = 0;
    pdevice->NextRcvToUseIsA    = TRUE;

    /* Set current buffer index & next availble packet index */
    pdevice->NextTxmPacketIndex = 0;
    pdevice->NextTxmBufferIndex = 0;

    return( OK );
}


/************************************************************************
 *
 *                          LAN_SAA9730_stop
 *  Description :
 *  -------------
 *   This routine stops the SAA9730 LAN controller
 *   by stoping  DMA transfer and resetting the chip.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_stop( t_LAN_SAA9730_device *pdevice )              
{
    /* Stop DMA first */
    REG( pdevice->p9730Regs, LAN_DMACTL ) &= ~LAN_DMACTL_ENTX_SET ;
    REG( pdevice->p9730Regs, LAN_DMACTL ) &= ~LAN_DMACTL_ENRX_SET ;

    /* Stop any transmit or receive activity */
    REG( pdevice->p9730Regs, LAN_TXCTL )  &= ~LAN_TXCTL_ENTX_SET ;
    REG( pdevice->p9730Regs, LAN_RXCTL )  &= ~LAN_RXCTL_RXEN_SET ;

    /* Set the SW Reset bits in DMA and MAC control registers */
    REG( pdevice->p9730Regs, LAN_DMATST ) |= LAN_DMATST_RESET_SET ;
    REG( pdevice->p9730Regs, LAN_MACCTL ) |= LAN_MACCTL_RESET_SET ;

    LAN_SAA9730_state = LAN_SAA9730_DRIVER_IS_STOPPED ;
    return( OK );
}


/************************************************************************
 *
 *                          LAN_SAA9730_CAM_init
 *  Description :
 *  -------------
 *    Initialize the SAA9730 LAN CAM
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_CAM_init( t_LAN_SAA9730_device *pdevice )          
{
    UINT32     rcode ;
    UINT32     i;
    t_mac_addr mac_addr ;

    /* get MAC address from board */
    IF_ERROR( (rcode),
              (SYSCON_read( SYSCON_COM_EN0_MAC_ADDR_ID,
                            &(mac_addr),
                            sizeof(mac_addr)) ) )

    /* Copy MAC-address into all entries */
    for (i = 0; i < LAN_SAA9730_CAM_ENTRIES; i++) 
    {
        memcpy( (UINT8*) pdevice->PhysicalAddress[i], 
                mac_addr, 
                SYS_MAC_ADDR_SIZE ) ;
    }
    return( OK );
}



/************************************************************************
 *
 *                          LAN_SAA9730_CAM_load
 *  Description :
 *  -------------
 *    Load the Ethernet address into SAA9730 CAM registers
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_CAM_load( t_LAN_SAA9730_device *pdevice )          
{
    UINT32 i ;
    UINT8  *NetworkAddress;

    /*
        UINT16 PhysicalAddress[0..LAN_SAA9730_CAM_ENTRIES-1][0..2] holds
        little endian copies of our physical address.

        The dwords we write to the CAM need to be big endian.
    */
    NetworkAddress = (UINT8*) &pdevice->PhysicalAddress[0][0];

    for ( i = 0; i < LAN_SAA9730_CAM_DWORDS; i++ )
    {
        REG( pdevice->p9730Regs, LAN_CAMADR ) = i ;
        REG( pdevice->p9730Regs, LAN_CAMDAT ) = cpu_to_be32( *(UINT32*)NetworkAddress ) ;
        NetworkAddress += 4;

#ifdef  ETH_DEBUG
        sprintf( msg, "\n\r LAN_SAA9730_CAM_load: %08x\n\r", REG( pdevice->p9730Regs, LAN_CAMDAT ) ) ;
        PUTS( DEFAULT_PORT, msg ) ;
#endif
    }
    return( OK );
}


/************************************************************************
 *
 *                          LAN_SAA9730_DMA_init
 *  Description :
 *  -------------
 *    Setup the SAA9730 DMA registers
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_DMA_init( t_LAN_SAA9730_device *pdevice )          
{
    /* Stop lan controller */
    LAN_SAA9730_stop( pdevice );

    /* Set the hardware    */

    /*   Reset DMA engine  */
    REG( pdevice->p9730Regs, LAN_DMATST ) |= LAN_DMATST_RESET_SET ;

    /*   Setup Buffers */
    REG( pdevice->p9730Regs, LAN_TXBUFA ) = PHYS( pdevice->TxmBuffer[0][0] ) ;
    REG( pdevice->p9730Regs, LAN_TXBUFB ) = PHYS( pdevice->TxmBuffer[1][0] ) ;
    REG( pdevice->p9730Regs, LAN_RXBUFA ) = PHYS( pdevice->RcvBuffer[0][0] ) ;
    REG( pdevice->p9730Regs, LAN_RXBUFB ) = PHYS( pdevice->RcvBuffer[1][0] ) ;

    REG( pdevice->p9730Regs, LAN_PCKCNT ) = ( (LAN_SAA9730_TXM_Q_SIZE << LAN_PCKCNT_TXA_SHF) |
                                              (LAN_SAA9730_TXM_Q_SIZE << LAN_PCKCNT_TXB_SHF) |
                                              (LAN_SAA9730_RCV_Q_SIZE << LAN_PCKCNT_RXA_SHF) |
                                              (LAN_SAA9730_RCV_Q_SIZE << LAN_PCKCNT_RXB_SHF) ) ;

    REG( pdevice->p9730Regs, LAN_OK2USE ) = ( (1 << LAN_OK2USE_RXA_SHF) |
                                              (1 << LAN_OK2USE_RXB_SHF) ) ;
    LAN_SAA9730_enable_buffer( pdevice, 0 ) ;

    /* initialize DMA control register
       disable all the interrupts for bootloader, 
       we are going to use poll mode here                               */
    REG( pdevice->p9730Regs, LAN_DMACTL ) = ( (1             << LAN_DMACTL_BLKINT_SHF) |
                                              (MAXXFER_ANY   << LAN_DMACTL_MAXXFER_SHF)|
                                              (ENDIAN_LITTLE << LAN_DMACTL_ENDIAN_SHF) ) ;

    
    /* Added RcvAFull & RcvBFull here
       initialize time out value                     */
    REG( pdevice->p9730Regs, LAN_TIMOUT ) = LAN_SAA9730_DEFAULT_TIME_OUT_CNT ;

    /* initial MAC control register */
//    REG( pdevice->p9730Regs, LAN_MACCTL ) = ( (CONMODE_FORCE_MII << LAN_MACCTL_CONMODE_SHF) |
//                                              (LAN_MACCTL_FULLDUP_SET) ) ;


    /* initialize CAM control register,  accept broadcast packets */
    REG( pdevice->p9730Regs, LAN_CAMCTL ) = ( (1 << LAN_CAMCTL_BROADCAST_SHF) |
                                              (1 << LAN_CAMCTL_COMPARE_SHF)   ) ;

    /* initialize Txm control register */
    REG( pdevice->p9730Regs, LAN_TXCTL  ) = 0 ;

    /* initialize Rcv control register */
    REG( pdevice->p9730Regs, LAN_RXCTL  ) = (1 << LAN_RXCTL_STRIPCRC_SHF) ;

    /* initialize CAM enable register, turn on all entries */
//    REG( pdevice->p9730Regs, LAN_CAMENA ) = 0x03ff ;
    REG( pdevice->p9730Regs, LAN_CAMENA ) = 0x1 ;

    return( OK );
}



/************************************************************************
 *
 *                          LAN_SAA9730_start
 *  Description :
 *  -------------
 *    Start the SAA9730 LAN controller operation
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_start( t_LAN_SAA9730_device *pdevice )          
{
    /* For Tx, turn on MAC then DMA */
    REG( pdevice->p9730Regs, LAN_TXCTL )  |= LAN_TXCTL_ENTX_SET ;
    REG( pdevice->p9730Regs, LAN_DMACTL ) |= LAN_DMACTL_ENTX_SET ;

    /* For Rx, turn on DMA then MAC */
    REG( pdevice->p9730Regs, LAN_DMACTL ) |= LAN_DMACTL_ENRX_SET ;
    REG( pdevice->p9730Regs, LAN_RXCTL )  |= LAN_RXCTL_RXEN_SET ;
    return( OK );
}




/************************************************************************
 *      Implementation : Device driver services
 ************************************************************************/



/************************************************************************
 *
 *                          LAN_SAA9730_init
 *  Description :
 *  -------------
 *  This service initializes the lan driver and configures
 *  the MAC-address for the 'EN0' LAN interface.
 *  The MAC-address is read during 'init' via the 'syscon' parameter:
 *    -'SYSCON_COM_EN0_MAC_ADDR_ID'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   INOUT, not used
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    INT32 rcode ;

    /* initialize shared services */
    LAN_error_lookup_init() ;

    /* get SAA9730 base address */
    IF_ERROR( (rcode), 
              (SYSCON_read(SYSCON_BOARD_SAA9730_BASE_ID,
              &(minor_device[minor].p9730Regs),
              sizeof(minor_device[minor].p9730Regs)) ) )

    minor_device[minor].p9730Regs = (void *)KSEG1(minor_device[minor].p9730Regs);
    
#ifdef  ETH_DEBUG
    sprintf( msg, "\n\r LAN_SAA9730_init: %08x\n\r", (UINT32) minor_device[minor].p9730Regs ) ;
    PUTS( DEFAULT_PORT, msg ) ;
#endif

	
    /* allocate LAN RX/TX frame buffer space */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_allocate_buffers(&minor_device[minor])) )

    /* stop SAA9730 LAN contoller */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_stop(&minor_device[minor])) )

    /* initialize SAA9730 CAM registers */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_CAM_init(&minor_device[minor])) )

    /* load SAA9730 CAM registers */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_CAM_load(&minor_device[minor])) )

    /* initialize SAA9730 DMA context registers */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_DMA_init(&minor_device[minor])) ) 

    /* Check SAA9730 MII op-mode and align with MAC-CTL */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_check_MII_opmode(&minor_device[minor])) )
	
    /* start SAA9730 LAN controller */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_start(&minor_device[minor])) )

#ifdef  ETH_DEBUG
    /* dump SAA9730 LAN controller registers */
    IF_ERROR( (rcode), 
              (LAN_SAA9730_dump_regs(&minor_device[minor])) )
#endif

    LAN_SAA9730_state = LAN_SAA9730_DRIVER_IS_STARTED ;

    return( OK ) ;
}


/************************************************************************
 *
 *                          LAN_SAA9730_open
 *  Description :
 *  -------------
 *  This service registers a mac-layer defined receive-handler in the
 *  LAN-drivers ISR-context to allow for interrupt controlled receive
 *  frame processing in the network stack. No external buffer
 *  administration is required as the protocol-layers are responsible for
 *  handling buffer-allocation and data copy-ing to the allocated buffer
 *  payload area. At return from 'receive' handler, the LAN-drivers
 *  local RX-buffer (packet) is released for re-use. After 'open'
 *  has been called, the LAN-driver's 'read' service will call the
 *  registered receive-handler by any frame reception with direct
 *  reference to the LAN-drivers packet space and no read data will be
 *  delivered in the IO-descriptor.
 *
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   IN,    LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_open(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_OPEN_desc *p_param )   /* IN: receive handler reference */
{
    /* register user defined receive handler */
    usr_receive = p_param->receive ;

    return( OK ) ;
}



/************************************************************************
 *
 *                          LAN_SAA9730_read
 *  Description :
 *  -------------
 *  This service polls the specified LAN interface for any received frame.
 *  If any frame has been received, it will be read into the user allocated
 *  variable, *p_param; if none present, completion = 'ERROR_LAN_NO_FRAME'
 *  will be returned.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_LAN_NO_FRAME':     no frame present on this LAN interface
 *  'ERROR_LAN_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param )   /* INOUT: LAN frame           */
{
    int	                 BufferIndex;
    int	                 PacketIndex;
    UINT32               status ;
    UINT32               *pPacket;
    UINT32               wBufLen ; 
    UINT8                *address;
    t_LAN_SAA9730_device *pdevice ;

    UINT32 rc       = ERROR_LAN_NO_FRAME ;
    UINT8  *pbData  = p_param->data ;
    UINT32 dwLength = p_param->length ;

    if ( LAN_SAA9730_state == LAN_SAA9730_DRIVER_IS_STOPPED )
    {
        return( ERROR_LAN_NO_FRAME ) ;
    }

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;
        
	
    while (1) 
    {
        /* address next packet */
        if (pdevice->NextRcvToUseIsA )
        {
            BufferIndex = 0;
        }
        else
        {
            BufferIndex = 1;
        }

        PacketIndex = pdevice->NextRcvPacketIndex;
        pPacket     = (UINT32* )pdevice->RcvBuffer[BufferIndex][PacketIndex];

        status = le32_to_cpu( REG(pPacket, RXPACKET_STATUS) ) ;
        if ( ( ((status) & RXPACKET_STATUS_FLAG_MSK) >> RXPACKET_STATUS_FLAG_SHF ) == RX_READY )
        {
            /* No, more packets */
            break;
        }
        else
        {
            /* check for next buffer to be enabled */
            if (PacketIndex == 0)
            {
                if (BufferIndex == 0)
                {
                    LAN_SAA9730_enable_buffer( pdevice, 1 ) ;
                }
                else
                {
                    LAN_SAA9730_enable_buffer( pdevice, 0 ) ;
                }
            }
            
            /* check the rx status */
            if ( status & RXPACKET_STATUS_GOOD_MSK )
            {
               /* received packet is good */
               wBufLen = status & RXPACKET_STATUS_SIZE_MSK ;
               pdevice->status.rx_bytes += wBufLen;
               pdevice->status.rx_packets++;

               if (usr_receive != NULL)
               {
                   /* we have a registered receive handler */
                   if ( wBufLen == 0 )
                   {
                       /* drop the packet */
                       pdevice->status.rx_zero_length_errors++;
                   }
                   else
                   {
                       /* count any broadcast's */
                       if ( !memcmp( ((UINT8*)pPacket) + 4, mac_broadcast_adr, SYS_MAC_ADDR_SIZE ) )
                       {
                           pdevice->status.multicast++ ;
                       }

                       /* call the handler */
                       (*usr_receive)( wBufLen, ((UINT8*)pPacket) + 4 ) ;

                       /* packet processed */
                       rc = OK;
                   }
               }
               else
               {
                   /* we don't have a registered receive handler */

                   if ( wBufLen == 0 ) 
                   {
                       /* drop the packet */
                       pdevice->status.rx_zero_length_errors++;
                   }

                   if ( wBufLen > dwLength ) 
                   {
                       /* drop the packet */
                       pdevice->status.rx_buffer_length_errors++;
                   }
                   else
                   {
                       /* count any broadcast's */
                       if ( !memcmp( ((UINT8*)pPacket) + 4, mac_broadcast_adr, SYS_MAC_ADDR_SIZE ) )
                       {
                           pdevice->status.multicast++ ;
                       }
                       address = (UINT8* ) pPacket;
                       address += 4;
                       memcpy ( pbData, (UINT8* )address, wBufLen);

                       /* we got the packet, return */
                       rc = OK;
                   }
               }
            }
            else
            {
                /* received packet has  errors, drop this packet */
                pdevice->status.rx_errors++;
                if (status & RXPACKET_STATUS_CRCERR_MSK)
                     pdevice->status.rx_crc_errors++;

                if (status & RXPACKET_STATUS_ALIGNERR_MSK)
                     pdevice->status.rx_frame_errors++;

                if (status & RXPACKET_STATUS_OVERFLOW_MSK)
                     pdevice->status.rx_fifo_errors++;

                if (status & RXPACKET_STATUS_LONGERR_MSK)
                     pdevice->status.rx_length_errors++;
            }

            /* go to next packet in sequence */
            pdevice->NextRcvPacketIndex++;
            if ( pdevice->NextRcvPacketIndex >= LAN_SAA9730_RCV_Q_SIZE )
            {
                pdevice->NextRcvPacketIndex = 0;
                if ( BufferIndex )
                {
                    pdevice->NextRcvToUseIsA  = TRUE;
                }
                else
                {
                    pdevice->NextRcvToUseIsA  = FALSE;
                }
            }

            /* check for any received packet */
            if (rc == OK)
            {
                return(OK) ;
            }
        }
    }

    if ( ((poll_count++) % 100) == 0 )
    {
        LAN_SAA9730_check_RX_controller_state( pdevice ) ;
        LAN_SAA9730_check_MII_opmode( pdevice ) ;
    }
    return( ERROR_LAN_NO_FRAME ) ;
}

/************************************************************************
 *
 *                          LAN_SAA9730_write
 *  Description :
 *  -------------
 *  This service requests transmission of a frame on the LAN interface. It is the caller's
 *  responsibility to fill in all information including the destination and source addresses and
 *  the frame type.  The length parameter gives the number of bytes in the ethernet frame.
 *  The routine will not return until the frame has been transmitted or an error has occurred.  If
 *  the frame transmits successfully, OK is returned.  If an error occurred, a message is sent to
 *  the serial port and the routine returns non-zero.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_LAN_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param )   /* OUT: frame to transmit     */
{
    int i ;
    UINT32 status ;
    UINT8  *pbData  = p_param->data ;
    UINT32 dwLength = p_param->length ;

    UINT8                         *pbPacketData;
    UINT32                        *pPacket ;
    int	                          BufferIndex;
    int	                          PacketIndex;
    t_LAN_SAA9730_device          *pdevice ;

    if ( LAN_SAA9730_state == LAN_SAA9730_DRIVER_IS_STOPPED )
    {
        return( ERROR_LAN_TXM_ERROR ) ;
    }

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;
        
    BufferIndex = pdevice->NextTxmBufferIndex;
    PacketIndex = pdevice->NextTxmPacketIndex;

	
    pdevice->NextTxmPacketIndex++;
    if ( pdevice->NextTxmPacketIndex >= LAN_SAA9730_TXM_Q_SIZE )
    {
        pdevice->NextTxmPacketIndex = 0;
        pdevice->NextTxmBufferIndex ^= 1;
    }

    pPacket = (UINT32*)pdevice->TxmBuffer[BufferIndex][PacketIndex];
	
    if ( (BufferIndex == 0)  &&
         ( REG( pdevice->p9730Regs, LAN_OK2USE ) & LAN_OK2USE_TXA_SET ) )
    {
        return(ERROR_LAN_NO_TXM_RESOURCES);
    }

    if ( (BufferIndex == 1)  &&
         ( REG( pdevice->p9730Regs, LAN_OK2USE ) & LAN_OK2USE_TXB_SET ) )
    {
        return(ERROR_LAN_NO_TXM_RESOURCES);
    }

    pbPacketData = (UINT8* )pPacket ;
    pbPacketData += 4;

    /* copy the bits */
    memcpy( pbPacketData, pbData, dwLength);
    pdevice->status.tx_bytes += dwLength;
    pdevice->status.tx_packets++;

 
    /* check to PAD packet up to minimum size */
    if (dwLength < LAN_SAA9730_MIN_PACKET_SIZE)
    {
        memset( (pbPacketData + dwLength), 0, (LAN_SAA9730_MIN_PACKET_SIZE-dwLength) ) ;
        dwLength = LAN_SAA9730_MIN_PACKET_SIZE ;
    }

    /* Set transmit status WORD for hardware (LAN-DMA-ENGINE) */
    REG( pPacket, TXPACKET_CONTROL ) = cpu_to_le32( (TX_READY << TXPACKET_CONTROL_FLAG_SHF) |
                                                    (dwLength << TXPACKET_CONTROL_SIZE_SHF) ) ;

    /* Start set hardware */
    if (BufferIndex == 0)
    {
        /* Txm_Buf_A */

        /* Set Ok2Use_TX_A = 1 */
        REG( pdevice->p9730Regs, LAN_OK2USE ) |= LAN_OK2USE_TXA_SET ;
    }
    else 
    { 
        /* Txm_Buf_B */

        /* Set Ok2Use_TX_B = 1 */
        REG( pdevice->p9730Regs, LAN_OK2USE ) |= LAN_OK2USE_TXB_SET ;
    }

    /* Now wait until either an error occurs or transmission completes successfully */
    i = 0 ;
    while( 1 ) 
    {
          /* check status from DMA */
          status = le32_to_cpu( REG(pPacket, TXPACKET_STATUS) ) ;
          if ( ( ((status) & TXPACKET_STATUS_FLAG_MSK) >> TXPACKET_STATUS_FLAG_SHF ) == TX_HWDONE ) 
          {
                /* Check for error. */
                if (status & TXPACKET_STATUS_ERROR_MSK)
                {
                    pdevice->status.tx_errors++;
                    if (status & TXPACKET_STATUS_EXDEFER_MSK)
                        pdevice->status.tx_aborted_errors++;

                    if (status & TXPACKET_STATUS_LATECOLLERR_MSK)
                        pdevice->status.tx_window_errors++;

                    if (status & TXPACKET_STATUS_LOSTCARRIER_MSK)
                        pdevice->status.tx_carrier_errors++;

                    if (status & TXPACKET_STATUS_UNDERRUN_MSK)
                        pdevice->status.tx_fifo_errors++;

                    if (status & TXPACKET_STATUS_SQERR_MSK)
                        pdevice->status.tx_heartbeat_errors++;

                    pdevice->status.collisions += 
                      ((status & TXPACKET_STATUS_COLLISIONS_MSK) >> TXPACKET_STATUS_COLLISIONS_SHF) ;
                }

              break ;
          }

          /* wait 1 ms */
          sys_wait_ms( 1 ) ;

          /* wait at most 1 second */
          i++ ;
          if ( i > 1000 )
          {
              pdevice->status.tx_errors++;
              pdevice->status.tx_timeout_errors++;

              /* re-init driver and controller */
              LAN_SAA9730_init( 0, 0, NULL ) ;

              return( ERROR_LAN_TXM_ERROR ) ;
          }
    }

    /* free the space */
    REG( pPacket, TXPACKET_CONTROL ) = cpu_to_le32( (TX_EMPTY << TXPACKET_CONTROL_FLAG_SHF) ) ;
    return( OK ) ;
}

/************************************************************************
 *
 *                          LAN_SAA9730_ctrl
 *  Description :
 *  -------------
 *  This service requests special service via 'ctrl'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, LAN variable of type, t_LAN_IO_desc.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_CTRL_desc *p_param )   /* IN-OUT:                       */
{
    t_LAN_SAA9730_device          *pdevice ;

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;

    switch( p_param->command )
    {
      case LAN_CTRL_DISPLAY_STATISTICS:
#ifdef ETH_DEBUG
          LAN_SAA9730_dump_regs( pdevice ) ;
          LAN_SAA9730_dump_packetstatus( pdevice ) ;
#endif
          LAN_SAA9730_MII_status( pdevice ) ;
          LAN_SAA9730_dump_status( pdevice ) ;
      break ;

      case LAN_CTRL_STOP_CONTROLLER:
          LAN_SAA9730_stop( pdevice ) ;
      break ;

      case LAN_CTRL_START_CONTROLLER:
          /* re-init driver and controller */
          LAN_SAA9730_init( 0, 0, NULL ) ;
      break ;

      default:
      break ;
    }
    return( OK ) ;
}


/************************************************************************
 *
 *                          LAN_SAA9730_dump_regs
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller registers
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
#ifdef  ETH_DEBUG
static
INT32 LAN_SAA9730_dump_regs( t_LAN_SAA9730_device *pdevice )
{
    volatile void *plan ;

    plan = pdevice->p9730Regs ;

    sprintf( msg, "\n\r LAN_SAA9730_dump_regs:\n\r") ;
    PUTS( DEFAULT_PORT, msg ) ;

    sprintf( msg, "     LAN_TXBUFA:   0x%08X\n\r", REG(plan, LAN_TXBUFA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_TXBUFB:   0x%08X\n\r", REG(plan, LAN_TXBUFB) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_RXBUFA:   0x%08X\n\r", REG(plan, LAN_RXBUFA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_RXBUFB:   0x%08X\n\r", REG(plan, LAN_RXBUFB) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_PCKCNT:   0x%08X\n\r", REG(plan, LAN_PCKCNT) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_OK2USE:   0x%08X\n\r", REG(plan, LAN_OK2USE) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_DMACTL:   0x%08X\n\r", REG(plan, LAN_DMACTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_TIMOUT:   0x%08X\n\r", REG(plan, LAN_TIMOUT) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_DMASTA:   0x%08X\n\r", REG(plan, LAN_DMASTA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_DMATST:   0x%08X\n\r", REG(plan, LAN_DMATST) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_PAUSE:    0x%08X\n\r", REG(plan, LAN_PAUSE) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_REMPAUSE: 0x%08X\n\r", REG(plan, LAN_REMPAUSE) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_MACCTL:   0x%08X\n\r", REG(plan, LAN_MACCTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_CAMCTL:   0x%08X\n\r", REG(plan, LAN_CAMCTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_TXCTL:    0x%08X\n\r", REG(plan, LAN_TXCTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_TXSTA:    0x%08X\n\r", REG(plan, LAN_TXSTA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_RXCTL:    0x%08X\n\r", REG(plan, LAN_RXCTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_RXSTA:    0x%08X\n\r", REG(plan, LAN_RXSTA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_MDDATA:   0x%08X\n\r", REG(plan, LAN_MDDATA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_MDCTL:    0x%08X\n\r", REG(plan, LAN_MDCTL) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_CAMADR:   0x%08X\n\r", REG(plan, LAN_CAMADR) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_CAMDAT:   0x%08X\n\r", REG(plan, LAN_CAMDAT) ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     LAN_CAMENA:   0x%08X\n\r", REG(plan, LAN_CAMENA) ) ;
    PUTS( DEFAULT_PORT, msg ) ;

    return( OK ) ;
}
#endif

/************************************************************************
 *
 *                          LAN_SAA9730_dump_status
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller statistics
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
void LAN_SAA9730_dump_status( t_LAN_SAA9730_device *pdevice )
{
    sprintf( msg, " Packets received:               %ld\n\r", pdevice->status.rx_packets ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Packets transmitted:            %ld\n\r", pdevice->status.tx_packets ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Bytes received:                 %ld\n\r", pdevice->status.rx_bytes ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Bytes transmitted:              %ld\n\r", pdevice->status.tx_bytes ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive errors:                 %ld\n\r", pdevice->status.rx_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit errors:                %ld\n\r", pdevice->status.tx_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Multicasts received:            %ld\n\r", pdevice->status.multicast ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Collisions:                     %ld\n\r", pdevice->status.collisions) ;
    PUTS( DEFAULT_PORT, msg ) ;
#ifdef ETH_DEBUG
    sprintf( msg, "     rx_zero_length_errors:      %ld\n\r", pdevice->status.rx_zero_length_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     rx_buffer_length_errors:    %ld\n\r", pdevice->status.rx_buffer_length_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
#endif
    sprintf( msg, " Receive length errors:          %ld\n\r", pdevice->status.rx_length_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive CRC errors:             %ld\n\r", pdevice->status.rx_crc_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive frame error:            %ld\n\r", pdevice->status.rx_frame_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive FIFO overrun errors :   %ld\n\r", pdevice->status.rx_fifo_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit aborted errors:        %ld\n\r", pdevice->status.tx_aborted_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit lost carrier errors:   %ld\n\r", pdevice->status.tx_carrier_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit FIFO underrun errors:  %ld\n\r", pdevice->status.tx_fifo_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit signal quality errors: %ld\n\r", pdevice->status.tx_heartbeat_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit late collision errors: %ld\n\r", pdevice->status.tx_window_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit timeout errors:        %ld\n\r", pdevice->status.tx_timeout_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
}


/************************************************************************
 *
 *                          LAN_SAA9730_MII_status
 *  Description :
 *  -------------
 *    Display MII status 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_MII_status( t_LAN_SAA9730_device *pdevice )
{
    UINT32 opmode ;

    /* check link status, spin here till station is not busy */
    while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

    /* set PHY address = 'STATUS' */
     REG( pdevice->p9730Regs, LAN_MDCTL ) = ( (LAN_SAA9730_MD_CA_BUSY) |
                                              (PHY_ADDRESS)     |
                                              (PHY_STATUS) ) ;

    /* check link status, spin here till station is not busy */
    while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

    /* wait for 1 ms. */
    sys_wait_ms( 1 ) ;

    /* check the link status */
    if ( REG( pdevice->p9730Regs, LAN_MDDATA ) & PHY_STATUS_LINK_UP )
    {
        /* set PHY address = 'REG31' */
         REG( pdevice->p9730Regs, LAN_MDCTL ) = ( (LAN_SAA9730_MD_CA_BUSY) |
                                                  (PHY_ADDRESS)     |
                                                  (PHY_REG31) ) ;

        /* check link status, spin here till station is not busy */
        while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

        /* wait for 1 ms. */
        sys_wait_ms( 1 ) ;

        /* get operation mode */
        opmode = ( ( REG( pdevice->p9730Regs, LAN_MDDATA ) & PHY_REG31_OPMODE_MSK ) >> PHY_REG31_OPMODE_SHF ) ;
        sprintf( msg, " MII status: Link is up, mode: %s\n\n\r",opmode_info[opmode]) ;
        PUTS( DEFAULT_PORT, msg ) ;
    }
    else
    {
        /* link is down */
        sprintf( msg, " MII status: Link is down !\n\n\r" ) ;
        PUTS( DEFAULT_PORT, msg ) ;
    }
    return(OK) ;
}


/************************************************************************
 *
 *                          LAN_SAA9730_dump_packetstatus
 *  Description :
 *  -------------
 *    Dump all SAA9730 LAN controller packet status
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
#ifdef ETH_DEBUG
static
void LAN_SAA9730_dump_packetstatus( t_LAN_SAA9730_device *pdevice )
{
    UINT32               Pa;
    UINT32               i, j ;

    sprintf( msg, "\n\r LAN_SAA9730_dump_packetstatus:\n\r") ;
    PUTS( DEFAULT_PORT, msg ) ;

    sprintf( msg, " NextRcvToUseIsA:    %d\n\r", pdevice->NextRcvToUseIsA ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " NextRcvPacketIndex: %d\n\r", pdevice->NextRcvPacketIndex) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " NextTxmBufferIndex: %d\n\r", pdevice->NextTxmBufferIndex) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " NextTxmPacketIndex: %d\n\r", pdevice->NextTxmPacketIndex) ;
    PUTS( DEFAULT_PORT, msg ) ;

    /* Display TX packet status */
    for ( i=0; i < 2; i++ )
    {
        sprintf( msg, "\n\r TX-Buffer: %d\n\r",i) ;
        PUTS( DEFAULT_PORT, msg ) ;
        for ( j= 0; j < LAN_SAA9730_TXM_Q_SIZE; j++ )
        {
            Pa = pdevice->TxmBuffer[i][j] ;
            sprintf( msg, " 0x%08X", (UINT32)le32_to_cpu(REG32(Pa)) ) ;
            PUTS( DEFAULT_PORT, msg ) ;
        }
    }

    /* Display RX packet status */
    for ( i=0; i < 2; i++ )
    {
        sprintf( msg, "\n\r RX-Buffer: %d\n\r",i) ;
        PUTS( DEFAULT_PORT, msg ) ;
        for ( j= 0; j < LAN_SAA9730_RCV_Q_SIZE; j++ )
        {
            if ( (j !=0) && (j%8 == 0))
            {
                sprintf( msg, "\n\r") ;
                PUTS( DEFAULT_PORT, msg ) ;
            }
            Pa = pdevice->RcvBuffer[i][j] ;
            sprintf( msg, " 0x%08X", (UINT32)le32_to_cpu(REG32(Pa)) ) ;
            PUTS( DEFAULT_PORT, msg ) ;
        }
    }
    sprintf( msg, "\n\r") ;
    PUTS( DEFAULT_PORT, msg ) ;
}
#endif


/************************************************************************
 *
 *                          LAN_SAA9730_check_MII_opmode
 *  Description :
 *  -------------
 *    Check MII opmode and align with MAC 
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_check_MII_opmode( t_LAN_SAA9730_device *pdevice )
{
    UINT32 opmode ;

    /* check link status, spin here till station is not busy */
    while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

    /* set PHY address = 'STATUS' */
     REG( pdevice->p9730Regs, LAN_MDCTL ) = ( (LAN_SAA9730_MD_CA_BUSY) |
                                              (PHY_ADDRESS)     |
                                              (PHY_STATUS) ) ;

    /* check link status, spin here till station is not busy */
    while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

    /* wait for 1 ms. */
    sys_wait_ms( 1 ) ;

    /* check the link status */
    if ( REG( pdevice->p9730Regs, LAN_MDDATA ) & PHY_STATUS_LINK_UP )
    {
        /* set PHY address = 'REG31' */
         REG( pdevice->p9730Regs, LAN_MDCTL ) = ( (LAN_SAA9730_MD_CA_BUSY) |
                                                  (PHY_ADDRESS)     |
                                                  (PHY_REG31) ) ;

        /* check link status, spin here till station is not busy */
        while ( REG( pdevice->p9730Regs, LAN_MDCTL ) & LAN_SAA9730_MD_CA_BUSY ) ;

        /* wait for 1 ms. */
        sys_wait_ms( 1 ) ;

        /* get operation mode */
        opmode = ( ( REG( pdevice->p9730Regs, LAN_MDDATA ) & PHY_REG31_OPMODE_MSK ) >> PHY_REG31_OPMODE_SHF ) ;

        /* check fullduplex */
        if ( opmode_is_fullduplex[opmode] )
        {
            /* FULL-DUPLEX */
            REG( pdevice->p9730Regs, LAN_MACCTL ) = ( (CONMODE_FORCE_MII << LAN_MACCTL_CONMODE_SHF) |
                                                      (LAN_MACCTL_FULLDUP_SET) ) ;
        }
        else
        {
            /* HALF-DUPLEX */
            REG( pdevice->p9730Regs, LAN_MACCTL ) = (CONMODE_FORCE_MII << LAN_MACCTL_CONMODE_SHF) ;
        }
    }
    else
    {
        /* link is down: nothing to align */
        
    }
    return(OK) ;
}


/************************************************************************
 *
 *                          LAN_SAA9730_check_RX_controller_state
 *  Description :
 *  -------------
 *    Check LAN controllers RX state machine for any lock-up
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'pdevice',     IN,    reference for this device  context
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static
INT32 LAN_SAA9730_check_RX_controller_state( t_LAN_SAA9730_device *pdevice )
{
    UINT32               status ;
    UINT32               last_packet_word ;
    UINT32               flag ;
    UINT32               size ;
    UINT32               Pa;
    UINT32               i, j ;

    if ( (REG( pdevice->p9730Regs, LAN_DBGRXS ) & LAN_DBGRXS_RXDII_MSK ) == LAN_DBGRXS_RXDII_ERROR )
    {
        /* reset LAN controller */
        // printf("LAN_SAA9730_check_RX_controller_state: RXDII-ERROR, RX-state machine is locked up\n") ;

        /* re-init driver and controller */
        LAN_SAA9730_init( 0, 0, NULL ) ;
        return(OK) ;
    }

    /* Check RX packet status */
    for ( i=0; i < 2; i++ )
    {
        for ( j= 1; j < LAN_SAA9730_RCV_Q_SIZE; j++ )
        {
            Pa = pdevice->RcvBuffer[i][j] ;
            last_packet_word = (*(UINT32*)(Pa - 4)) ;
            status = le32_to_cpu( REG(Pa, RXPACKET_STATUS) ) ;
            size   = status & RXPACKET_STATUS_SIZE_MSK ; 
            flag   = (status & RXPACKET_STATUS_FLAG_MSK) >> RXPACKET_STATUS_FLAG_SHF ;
            if ( ( flag == RX_INVALID_STAT )      ||
                 ( size >  LAN_MAX_FRAME_LENGTH ) || 
                 ( last_packet_word != 0 )           )
            {
                /* re-init driver and controller */
                // printf("LAN_SAA9730_check_RX_controller_state: RXPCK-ERROR, RX-state machine is locked up\n") ;
                LAN_SAA9730_init( 0, 0, NULL ) ;
                return(OK) ;
            }
        }
    }
    return(OK) ;
}
