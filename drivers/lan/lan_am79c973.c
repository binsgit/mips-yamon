/************************************************************************
 *
 *      LAN_AM79C973.c
 *
 *      The 'LAN_AM79C973' module implements the LAN_AM79C973 
 *      device driver as an IO device with following services:
 *
 *        1) init  serial device:  configure and initialize LAN 
 *                                 AM79C973 driver
 *        2) open  serial device:  register receive handler
 *        3) close serial device:  not used
 *        4) read  serial device:  poll for received frame
 *        5) write serial device:  request frame to be transmitted
 *        6) ctrl  serial device:  display diagnostics 
 *
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
#include <io_api.h>
#include <syscon_api.h>
#include <excep_api.h>
#include <sys_api.h>
#include <lan_api.h>
#include <lan_am79c973_api.h>


/************************************************************************
 *  Constant Definitions
*************************************************************************/
 
#define WORD0_OFS    0x00000       /* Word0:                       */
#define WORD1_OFS    0x00004       /* Word1:                       */


/* Maximum cache line size */
#define CACHE_LINE_SIZE 0x20

typedef enum
{
    LAN_MINOR_AM79C973_DEVICE_1 = 0,    /* The one and only AM79C973
		                                  LAN controller               */

	/******* ADD NEW MINOR DEVICES JUST BEFORE THIS LINE ONLY ********/

    LAN_MINOR_AM79C973_DEVICES
} t_LAN_MINOR_AM79C973_devices ;

/* PHY definitions */
#define PHY_CONTROL                     0
#define PHY_AUTOADV                     4

#define PHY_ADDRESS                     (30 << 5)

#define PHY_SUMMARY                     24

#define PHY_SUMMARY_LINK_UP             (1 << 3)
#define PHY_SUMMARY_FULL_DUPLEX         (1 << 2)
#define PHY_SUMMARY_AUTONEG_CHANGED     (1 << 1)
#define PHY_SUMMARY_SPEED_100MB         (1 << 0)


/* Init Block: 
   Size of Init Block                                                  */
#define LAN_AM79C973_INITBLOCK_SIZE    28

/* Init Block: 
   'TLEN' Number of Transmit Descriptor Ring Entries, (encoded)        */
#define LAN_AM79C973_INITBLOCK_TLEN    4

/* Init Block: 
   'RLEN' Number of Receive Descriptor Ring Entries, (encoded)         */
#define LAN_AM79C973_INITBLOCK_RLEN    4

/* 
   Number of descriptors in transmit ring, 
   derived from the TLEN field                                         */
#define LAN_AM79C973_TDRE_COUNT        (1 << LAN_AM79C973_INITBLOCK_TLEN)
#define LAN_AM79C973_TDRE_SIZE         16

/* 
   Number of descriptors in receive ring, 
   derived from the RLEN field                                         */
#define LAN_AM79C973_RDRE_COUNT        (1 << LAN_AM79C973_INITBLOCK_RLEN)
#define LAN_AM79C973_RDRE_SIZE         16

/* TX and RX data buffer size: fixed to 1536 bytes,                    */
#define LAN_AM79C973_BUFFER_SIZE       0x600
#define LAN_AM79C973_FCS_SIZE          0x4

/* 
   Number of TX data buffers,
   derived from the TLEN field                                         */
#define LAN_AM79C973_TX_BUFFERS        (1 << LAN_AM79C973_INITBLOCK_TLEN)

/* 
   Number of RX data buffers,
   derived from the RLEN field                                         */
#define LAN_AM79C973_RX_BUFFERS        (1 << LAN_AM79C973_INITBLOCK_RLEN)

/* Minimum data buffer size */
#define LAN_AM79C973_MIN_BUFFER_SIZE   60


/************************************************************************
 *  LAN AM79C973: Relative Register Addresses, for PCI-BAR1, 32-bit mode
*************************************************************************/

#define LAN_AM79C973_RDP32_OFS    0x10  /* RDP register, CSR reg. acc.  */
#define LAN_AM79C973_RAP32_OFS    0x14  /* RAP register                 */
#define LAN_AM79C973_RES32_OFS    0x18  /* RESET register               */
#define LAN_AM79C973_BDP32_OFS    0x1C  /* BDP register, BCR reg. acc.  */

/************************************************************************
 *  Macro Definitions
*************************************************************************/

/* The sequence is:
   a) Set 32-bit IO mode (writing '0' to offset 0x10)
   b) Read 'RESET' register
   c) Wait 10 ms.
   c) Set 32-bit IO mode
*/
#define LAN_AM79C973_RESET( membase, readvar ) \
{ \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RDP32_OFS)) = 0 ; \
  readvar = ((*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RES32_OFS)) & 0xffff) ; \
  sys_wait_ms( 10 ) ; \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RAP32_OFS)) = 0 ; \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RDP32_OFS)) = 0 ; \
}

#define CSR_READ( membase, csr, readvar ) \
{ \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RAP32_OFS)) = (csr) ; \
  readvar = ((*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RDP32_OFS)) & 0xffff) ; \
}

#define CSR_WRITE( membase, csr, writevar ) \
{ \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RAP32_OFS)) = (csr) ; \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RDP32_OFS)) = (writevar) & 0xffff ; \
}

#define BCR_READ( membase, bcr, readvar ) \
{ \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RAP32_OFS)) = (bcr) ; \
  readvar = ((*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_BDP32_OFS)) & 0xffff) ; \
}

#define BCR_WRITE( membase, bcr, writevar ) \
{ \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_RAP32_OFS)) = (bcr) ; \
  (*(volatile UINT32 *)((UINT32)(membase)+LAN_AM79C973_BDP32_OFS)) = (writevar) & 0xffff ; \
}

#define IF_ERROR( completion, function )  \
{ \
  completion = function ; \
  if ( completion != OK )  \
{ \
    return( completion ) ; \
} \
}

/************************************************************************
 *  Type Definitions
*************************************************************************/

/* protoype for int handler */
typedef void    (*t_inthandler)(void *data) ;


/*
 *      Network device statistics. 
 */
typedef struct net_device_stats
{
        UINT32   rx_packets;             /* total packets received       */
        UINT32   tx_packets;             /* total packets transmitted    */
        UINT32   rx_bytes;               /* total bytes received         */
        UINT32   tx_bytes;               /* total bytes transmitted      */
        UINT32   rx_errors;              /* bad packets received         */
        UINT32   tx_errors;              /* packet transmit problems     */
        UINT32   multicast;              /* multicast packets received   */
        UINT32   collisions;
        UINT32   interrupts ;            /* total number of interrupts   */

        /* detailed rx_errors: */
        UINT32   rx_zero_length_errors;
        UINT32   rx_buffer_length_errors;
        UINT32   rx_over_errors;         /* recved pkt with overflow     */
        UINT32   rx_crc_errors;          /* recved pkt with crc error    */
        UINT32   rx_frame_errors;        /* recv'd frame alignment error */
        UINT32   rx_fifo_errors;         /* recv'r fifo overrun          */
        UINT32   rx_no_resource_errors;  /* recv'r no resource errors    */

        /* detailed tx_errors */
        UINT32   tx_aborted_errors;
        UINT32   tx_carrier_errors;
        UINT32   tx_fifo_errors;
        UINT32   tx_heartbeat_errors;
        UINT32   tx_window_errors;
        UINT32   tx_timeout_errors;
        UINT32   tx_bus_parity_errors;

} t_net_device_stats ;


/*
   Device context for a AM79C973 LAN controller 
                                                                        */
typedef struct LAN_AM79C973_device
{
	/* pointer for the AM79C973 LAN controller, reg. base address   */
	void                       *p79C973Regs;

        /* Init Block, (pointer is in CPU address space)  */
        void                       *pInitBlock ;

        /* TX Descriptor Ring, (pointers are in CPU address space) */
	UINT32                     TXDRE[LAN_AM79C973_TDRE_COUNT] ;

        /* RX Descriptor Ring, (pointers are in CPU address space) */
	UINT32                     RXDRE[LAN_AM79C973_TDRE_COUNT] ;

        /* TX Buffers, (pointers are in CPU address space) */
	UINT32                     TxBuffer[LAN_AM79C973_TX_BUFFERS];

        /* RX Buffers, (pointers are in CPU address space) */
	UINT32                     RxBuffer[LAN_AM79C973_RX_BUFFERS];

        /* TDRE index */
        UINT8                      NextTDREIndex ;

        /* RDRE index */
        UINT8                      NextRDREIndex ;

        /* network statistics */
        t_net_device_stats         status ;

} t_LAN_AM79C973_device ;



/************************************************************************
 *  LAN AM79C973: Shared memory register field encodings
*************************************************************************/

/************************************************************************
 *  LAN AM79C973: Initialization Block (SSIZE32=1)
*************************************************************************/

#define INIT_WORD0_OFS    0x00000       /* Word0:                       */
#define INIT_WORD1_OFS    0x00004       /* Word1:                       */
#define INIT_WORD2_OFS    0x00008       /* Word2:                       */
#define INIT_WORD3_OFS    0x0000C       /* Word3:                       */
#define INIT_WORD4_OFS    0x00010       /* Word4:                       */
#define INIT_WORD5_OFS    0x00014       /* Word5:                       */
#define INIT_WORD6_OFS    0x00018       /* Word6:                       */


/******** reg: Init Block, WORD0 ********/

/* field: TLEN */
#define INIT_WORD0_TLEN_SHF             28
#define INIT_WORD0_TLEN_MSK             (MSK(4) << INIT_WORD0_TLEN_SHF)

/* field: RLEN */
#define INIT_WORD0_RLEN_SHF             20
#define INIT_WORD0_RLEN_MSK             (MSK(4) << INIT_WORD0_RLEN_SHF)

/* field: MODE */
#define INIT_WORD0_MODE_SHF             0
#define INIT_WORD0_MODE_MSK             (MSK(16) << INIT_WORD0_MODE_SHF)

/* field: PROM */
#define INIT_WORD0_PROM_SHF             15
#define INIT_WORD0_PROM_MSK             (MSK(1) << INIT_WORD0_PROM_SHF)
#define INIT_WORD0_PROM_SET             INIT_WORD0_PROM_MSK

/* field: DRCVBC */
#define INIT_WORD0_DRCVBC_SHF           14
#define INIT_WORD0_DRCVBC_MSK           (MSK(1) << INIT_WORD0_DRCVBC_SHF)
#define INIT_WORD0_DRCVBC_SET           INIT_WORD0_DRCVBC_MSK

/* field: DRCVPA */
#define INIT_WORD0_DRCVPA_SHF           13
#define INIT_WORD0_DRCVPA_MSK           (MSK(1) << INIT_WORD0_DRCVPA_SHF)
#define INIT_WORD0_DRCVPA_SET           INIT_WORD0_DRCVPA_MSK

/* field: PORTSEL */
#define INIT_WORD0_PORTSEL_SHF          7
#define INIT_WORD0_PORTSEL_MSK          (MSK(2) << INIT_WORD0_PORTSEL_SHF)

/* field: INTL */
#define INIT_WORD0_INTL_SHF             6
#define INIT_WORD0_INTL_MSK             (MSK(1) << INIT_WORD0_INTL_SHF)
#define INIT_WORD0_INTL_SET             INIT_WORD0_INTL_MSK

/* field: DRTY */
#define INIT_WORD0_DRTY_SHF             5
#define INIT_WORD0_DRTY_MSK             (MSK(1) << INIT_WORD0_DRTY_SHF)
#define INIT_WORD0_DRTY_SET             INIT_WORD0_DRTY_MSK

/* field: FCOLL */
#define INIT_WORD0_FCOLL_SHF            4
#define INIT_WORD0_FCOLL_MSK            (MSK(1) << INIT_WORD0_FCOLL_SHF)
#define INIT_WORD0_FCOLL_SET            INIT_WORD0_FCOLL_MSK

/* field: DXMTFCS */
#define INIT_WORD0_DXMTFCS_SHF          3
#define INIT_WORD0_DXMTFCS_MSK          (MSK(1) << INIT_WORD0_DXMTFCS_SHF)
#define INIT_WORD0_DXMTFCS_SET          INIT_WORD0_DXMTFCS_MSK

/* field: LOOP */
#define INIT_WORD0_LOOP_SHF             2
#define INIT_WORD0_LOOP_MSK             (MSK(1) << INIT_WORD0_LOOP_SHF)
#define INIT_WORD0_LOOP_SET             INIT_WORD0_LOOP_MSK

/* field: DTX */
#define INIT_WORD0_DTX_SHF              1
#define INIT_WORD0_DTX_MSK              (MSK(1) << INIT_WORD0_DTX_SHF)
#define INIT_WORD0_DTX_SET              INIT_WORD0_DTX_MSK

/* field: DRX */
#define INIT_WORD0_DRX_SHF              0
#define INIT_WORD0_DRX_MSK              (MSK(1) << INIT_WORD0_DRX_SHF)
#define INIT_WORD0_DRX_SET              INIT_WORD0_DRX_MSK

/******** reg: Init Block, WORD1 ********/

/* field: MACADR3 */
#define INIT_WORD1_MACADR3_SHF          24
#define INIT_WORD1_MACADR3_MSK          (MSK(8) << INIT_WORD1_MACADR3_SHF)

/* field: MACADR2 */
#define INIT_WORD1_MACADR2_SHF          16
#define INIT_WORD1_MACADR2_MSK          (MSK(8) << INIT_WORD1_MACADR2_SHF)

/* field: MACADR1 */
#define INIT_WORD1_MACADR1_SHF          8
#define INIT_WORD1_MACADR1_MSK          (MSK(8) << INIT_WORD1_MACADR1_SHF)

/* field: MACADR0 */
#define INIT_WORD1_MACADR0_SHF          0
#define INIT_WORD1_MACADR0_MSK          (MSK(8) << INIT_WORD1_MACADR0_SHF)


/******** reg: Init Block, WORD2 ********/

/* field: MACADR5 */
#define INIT_WORD2_MACADR5_SHF          8
#define INIT_WORD2_MACADR5_MSK          (MSK(8) << INIT_WORD2_MACADR5_SHF)

/* field: MACADR4 */
#define INIT_WORD2_MACADR4_SHF          0
#define INIT_WORD2_MACADR4_MSK          (MSK(8) << INIT_WORD2_MACADR4_SHF)

/******** reg: Init Block, WORD3 ********/

/******** reg: Init Block, WORD4 ********/

/******** reg: Init Block, WORD5 ********/

/******** reg: Init Block, WORD6 ********/


/************************************************************************
 *  LAN AM79C973: Receive Descriptor Element (SSIZE32=1)
*************************************************************************/

#define RDE_WORD0_OFS    0x00000       /* Word0:                       */
#define RDE_WORD1_OFS    0x00004       /* Word1:                       */
#define RDE_WORD2_OFS    0x00008       /* Word2:                       */
#define RDE_WORD3_OFS    0x0000C       /* Word3:                       */


/******** reg: RDE, WORD0 ********/

/******** reg: RDE, WORD1 ********/

/* field: OWN */
#define RDE_WORD1_OWN_SHF              31
#define RDE_WORD1_OWN_MSK              (MSK(1) << RDE_WORD1_OWN_SHF)

/* field: ERR */
#define RDE_WORD1_ERR_SHF              30
#define RDE_WORD1_ERR_MSK              (MSK(1) << RDE_WORD1_ERR_SHF)

/* field: FRAM */
#define RDE_WORD1_FRAM_SHF             29
#define RDE_WORD1_FRAM_MSK             (MSK(1) << RDE_WORD1_FRAM_SHF)

/* field: OFLO */
#define RDE_WORD1_OFLO_SHF             28
#define RDE_WORD1_OFLO_MSK             (MSK(1) << RDE_WORD1_OFLO_SHF)

/* field: CRC */
#define RDE_WORD1_CRC_SHF              27
#define RDE_WORD1_CRC_MSK              (MSK(1) << RDE_WORD1_CRC_SHF)

/* field: BUFF */
#define RDE_WORD1_BUFF_SHF             26
#define RDE_WORD1_BUFF_MSK             (MSK(1) << RDE_WORD1_BUFF_SHF)

/* field: STP */
#define RDE_WORD1_STP_SHF              25
#define RDE_WORD1_STP_MSK              (MSK(1) << RDE_WORD1_STP_SHF)

/* field: ENP */
#define RDE_WORD1_ENP_SHF              24
#define RDE_WORD1_ENP_MSK              (MSK(1) << RDE_WORD1_ENP_SHF)

/* field: BPE */
#define RDE_WORD1_BPE_SHF              23
#define RDE_WORD1_BPE_MSK              (MSK(1) << RDE_WORD1_BPE_SHF)

/* field: PAM */
#define RDE_WORD1_PAM_SHF              22
#define RDE_WORD1_PAM_MSK              (MSK(1) << RDE_WORD1_PAM_SHF)

/* field: LAFM */
#define RDE_WORD1_LAFM_SHF             21
#define RDE_WORD1_LAFM_MSK             (MSK(1) << RDE_WORD1_LAFM_SHF)

/* field: BAM */
#define RDE_WORD1_BAM_SHF              20
#define RDE_WORD1_BAM_MSK              (MSK(1) << RDE_WORD1_BAM_SHF)

/* field: ONES */
#define RDE_WORD1_ONES_SHF             12
#define RDE_WORD1_ONES_MSK             (MSK(4) << RDE_WORD1_ONES_SHF)

/* field: BCNT */
#define RDE_WORD1_BCNT_SHF             0
#define RDE_WORD1_BCNT_MSK             (MSK(12) << RDE_WORD1_BCNT_SHF)

/******** reg: RDE, WORD2 ********/

/* field: MCNT */
#define RDE_WORD2_MCNT_SHF             0
#define RDE_WORD2_MCNT_MSK             (MSK(12) << RDE_WORD2_MCNT_SHF)

/******** reg: RDE, WORD3 ********/


/************************************************************************
 *  LAN AM79C973: Transmit Descriptor Element (SSIZE32=1)
*************************************************************************/

#define TDE_WORD0_OFS    0x00000       /* Word0:                       */
#define TDE_WORD1_OFS    0x00004       /* Word1:                       */
#define TDE_WORD2_OFS    0x00008       /* Word2:                       */
#define TDE_WORD3_OFS    0x0000C       /* Word3:                       */


/******** reg: TDE, WORD0 ********/

/******** reg: TDE, WORD1 ********/

/* field: OWN */
#define TDE_WORD1_OWN_SHF              31
#define TDE_WORD1_OWN_MSK              (MSK(1) << TDE_WORD1_OWN_SHF)

/* field: ERR */
#define TDE_WORD1_ERR_SHF              30
#define TDE_WORD1_ERR_MSK              (MSK(1) << TDE_WORD1_ERR_SHF)

/* field: AFCS */
#define TDE_WORD1_AFCS_SHF             29
#define TDE_WORD1_AFCS_MSK             (MSK(1) << TDE_WORD1_AFCS_SHF)

/* field: MORE */
#define TDE_WORD1_MORE_SHF             28
#define TDE_WORD1_MORE_MSK             (MSK(1) << TDE_WORD1_MORE_SHF)

/* field: ONE */
#define TDE_WORD1_ONE_SHF              27
#define TDE_WORD1_ONE_MSK              (MSK(1) << TDE_WORD1_ONE_SHF)

/* field: DEF */
#define TDE_WORD1_DEF_SHF              26
#define TDE_WORD1_DEF_MSK              (MSK(1) << TDE_WORD1_DEF_SHF)

/* field: STP */
#define TDE_WORD1_STP_SHF              25
#define TDE_WORD1_STP_MSK              (MSK(1) << TDE_WORD1_STP_SHF)

/* field: ENP */
#define TDE_WORD1_ENP_SHF              24
#define TDE_WORD1_ENP_MSK              (MSK(1) << TDE_WORD1_ENP_SHF)

/* field: BPE */
#define TDE_WORD1_BPE_SHF              23
#define TDE_WORD1_BPE_MSK              (MSK(1) << TDE_WORD1_BPE_SHF)

/* field: ONES */
#define TDE_WORD1_ONES_SHF             12
#define TDE_WORD1_ONES_MSK             (MSK(4) << TDE_WORD1_ONES_SHF)

/* field: BCNT */
#define TDE_WORD1_BCNT_SHF             0
#define TDE_WORD1_BCNT_MSK             (MSK(12) << TDE_WORD1_BCNT_SHF)


/******** reg: TDE, WORD2 ********/

/* field: BUFF */
#define TDE_WORD2_BUFF_SHF             31
#define TDE_WORD2_BUFF_MSK             (MSK(1) << TDE_WORD2_BUFF_SHF)

/* field: UFLO */
#define TDE_WORD2_UFLO_SHF             30
#define TDE_WORD2_UFLO_MSK             (MSK(1) << TDE_WORD2_UFLO_SHF)

/* field: EXDEF */
#define TDE_WORD2_EXDEF_SHF            29
#define TDE_WORD2_EXDEF_MSK            (MSK(1) << TDE_WORD2_EXDEF_SHF)

/* field: LCOL */
#define TDE_WORD2_LCOL_SHF             28
#define TDE_WORD2_LCOL_MSK             (MSK(1) << TDE_WORD2_LCOL_SHF)

/* field: LCAR */
#define TDE_WORD2_LCAR_SHF             27
#define TDE_WORD2_LCAR_MSK             (MSK(1) << TDE_WORD2_LCAR_SHF)

/* field: RTRY */
#define TDE_WORD2_RTRY_SHF             26
#define TDE_WORD2_RTRY_MSK             (MSK(1) << TDE_WORD2_RTRY_SHF)

/* field: TRC */
#define TDE_WORD2_TRC_SHF              0
#define TDE_WORD2_TRC_MSK              (MSK(4) << TDE_WORD2_TRC_SHF)

/* Value definition for TX and RX descriptor entry owner semaphore     */
#define HOST_IS_OWNER                  0
#define CTRL_IS_OWNER                  1

/* Global driver states */
#define LAN_AM79C973_DRIVER_IS_STOPPED   0x42
#define LAN_AM79C973_DRIVER_IS_STARTED   0x44

/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/
static char msg[160] ;

/* Global driver state */
static UINT32 LAN_AM79C973_state = LAN_AM79C973_DRIVER_IS_STOPPED ;

/* MAC broadcast address */
static t_mac_addr mac_broadcast_adr = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } ;

/* User registered receive handler */
static UINT32 (*usr_receive)( UINT32 length, UINT8 *data ) = NULL ;

/* Device context per minor device for this driver */
static t_LAN_AM79C973_device   minor_device[LAN_MINOR_AM79C973_DEVICES] ;

/* Dynamic allocated memory state */
static UINT32 first_time_init  = 0 ;

/************************************************************************
 *      Static function prototypes, local helper functions
 ************************************************************************/

/************************************************************************
 *
 *                          LAN_AM79C973_allocate_memory
 *  Description :
 *  -------------
 *     This routine allocates memory for:
 *
 *     - Init Block,
 *     - TX and RX descriptor ring elements,
 *     - Tx data buffers,
 *     - Rx data buffers and
 *
 *    initializes the allocated structures.
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
INT32 LAN_AM79C973_allocate_memory( t_LAN_AM79C973_device *pdevice )  ;


/************************************************************************
 *
 *                          LAN_AM79C973_init_initblock
 *  Description :
 *  -------------
 *   This routine initializes the AM79C973 Init Block data structure.
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
INT32 LAN_AM79C973_init_initblock( t_LAN_AM79C973_device *pdevice ) ;


/************************************************************************
 *
 *                          LAN_AM79C973_init_ring_descriptors
 *  Description :
 *  -------------
 *   This routine initializes the AM79C973 ring descriptor
 *   data structures.
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
INT32 LAN_AM79C973_init_ring_descriptors( t_LAN_AM79C973_device *pdevice ) ;


/************************************************************************
 *
 *                          LAN_AM79C973_stop
 *  Description :
 *  -------------
 *   This routine stops the AM79C973 LAN controller
 *   by stopping  DMA transfer and resetting the chip.
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
INT32 LAN_AM79C973_stop( t_LAN_AM79C973_device *pdevice ) ;            


/************************************************************************
 *
 *                          LAN_AM79C973_MII_setup
 *  Description :
 *  -------------
 *    Setup MII: if PCI clock < 33 MHz, then force 10 MBit/sec and
 *    half duplex; else setup MII for autonegotiate.
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
INT32 LAN_AM79C973_MII_setup( t_LAN_AM79C973_device *pdevice ) ;


/************************************************************************
 *
 *                          LAN_AM79C973_MII_status
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
INT32 LAN_AM79C973_MII_status( t_LAN_AM79C973_device *pdevice ) ;        


/************************************************************************
 *
 *                          LAN_AM79C973_dump_regs
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller registers
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
INT32 LAN_AM79C973_dump_regs( t_LAN_AM79C973_device *pdevice ) ;
#endif


/************************************************************************
 *
 *                          LAN_AM79C973_dump_status
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller statistics
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
void LAN_AM79C973_dump_status( t_LAN_AM79C973_device *pdevice ) ;


/************************************************************************
 *
 *                         LAN_AM79C973_dump_descriptors 
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller TX and RX ring descriptors
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
INT32 LAN_AM79C973_dump_descriptors( t_LAN_AM79C973_device *pdevice ) ;
#endif


/************************************************************************
 *
 *                         LAN_AM79C973_receive
 *  Description :
 *  -------------
 *    Receive a packet 
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
INT32 LAN_AM79C973_receive( t_LAN_AM79C973_device *pdevice ) ;


/************************************************************************
 *
 *                          LAN_AM79C973_dump_device
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller TX and RX ring descriptors
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
INT32 LAN_AM79C973_dump_device( t_LAN_AM79C973_device *pdevice ) ;
#endif


/************************************************************************
 *      Static function prototypes, device driver IO functions
 ************************************************************************/


/************************************************************************
 *
 *                          LAN_AM79C973_init
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
INT32 LAN_AM79C973_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *
 *                          LAN_AM79C973_open
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
INT32 LAN_AM79C973_open(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_OPEN_desc *p_param ) ; /* IN: receive handler reference */


/************************************************************************
 *
 *                          LAN_AM79C973_read
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
INT32 LAN_AM79C973_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param ) ; /* INOUT: LAN frame           */


/************************************************************************
 *
 *                          LAN_AM79C973_write
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
INT32 LAN_AM79C973_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param ) ; /* OUT: frame to transmit     */

/************************************************************************
 *
 *                          LAN_AM79C973_ctrl
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
INT32 LAN_AM79C973_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_CTRL_desc *p_param ) ; /* IN-OUT:                       */




/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          LAN_AM79C973_install
 *  Description :
 *  -------------
 *
 *  Installs the AM79C973 LAN device drivers services in 
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
INT32 LAN_AM79C973_install( void )
{
    /* pre-initialize local variables and install device services */
    memset( minor_device, sizeof(minor_device), 0) ;
    IO_install(   SYS_MAJOR_LAN_AM79C973,         /* major device number */
           (t_io_service) LAN_AM79C973_init,      /* 'init'  service     */
           (t_io_service) LAN_AM79C973_open,      /* 'open'  service     */
                          NULL,                   /* 'close' service  na */
           (t_io_service) LAN_AM79C973_read,      /* 'read'  service     */
           (t_io_service) LAN_AM79C973_write,     /* 'write' service     */
           (t_io_service) LAN_AM79C973_ctrl ) ;   /* 'ctrl'  service     */

    /* initialize AM79C973 LAN device driver */
    if( IO_init( SYS_MAJOR_LAN_AM79C973, 0, NULL ) != OK )
    {
	/* Should not happen unless board is defect */
	IO_deinstall( SYS_MAJOR_LAN_AM79C973 );
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
 *                          LAN_AM79C973_allocate_memory
 *  Description :
 *  -------------
 *     This routine allocates memory for:
 *
 *     - Init Block
 *     - TX and RX descriptor ring elements
 *     - Tx data buffers
 *     - Rx data buffers 
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
INT32 LAN_AM79C973_allocate_memory( t_LAN_AM79C973_device *pdevice )  
{
    INT32         rcode ;
    t_sys_malloc  mem ;
    UINT32        ptmp ;
    int           i ;

    if (first_time_init == 0)
    {
        /* first time initialization */

        /* 1: allocate initialization block */
        mem.size     = LAN_AM79C973_INITBLOCK_SIZE ;
        mem.boundary = sizeof(UINT32) ;
        mem.memory   = (void*)&ptmp ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
        pdevice->pInitBlock = (void*) KSEG1( ptmp ) ;

        /* 2: allocate TX Descriptor Ring */
        mem.size     = LAN_AM79C973_TDRE_COUNT * LAN_AM79C973_TDRE_SIZE ;
        mem.boundary = LAN_AM79C973_TDRE_SIZE ;
        mem.memory   = (void*)&ptmp ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
        ptmp = KSEG1( ptmp ) ;
        for (i=0; i<LAN_AM79C973_TDRE_COUNT; i++)
        {
            pdevice->TXDRE[i] = ptmp ;
            ptmp             += LAN_AM79C973_TDRE_SIZE ;
        }

        /* 3: allocate RX Descriptor Ring */
        mem.size     = LAN_AM79C973_RDRE_COUNT * LAN_AM79C973_RDRE_SIZE ;
        mem.boundary = LAN_AM79C973_RDRE_SIZE ;
        mem.memory   = (void*)&ptmp ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
        ptmp = KSEG1( ptmp ) ;
        for (i=0; i<LAN_AM79C973_RDRE_COUNT; i++)
        {
            pdevice->RXDRE[i] = ptmp ;
            ptmp             += LAN_AM79C973_RDRE_SIZE ;
        }
    
        /* 4: allocate TX Data Buffers */
        mem.size     = LAN_AM79C973_TX_BUFFERS * LAN_AM79C973_BUFFER_SIZE ;
        mem.boundary = CACHE_LINE_SIZE ;
        mem.memory   = (void*)&ptmp ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
        ptmp = KSEG1( ptmp ) ;
        for (i=0; i<LAN_AM79C973_TX_BUFFERS; i++)
        {
            pdevice->TxBuffer[i] = ptmp ;
            ptmp             += LAN_AM79C973_BUFFER_SIZE ;
        }

        /* 5: allocate RX Data Buffers */
        mem.size     = LAN_AM79C973_RX_BUFFERS * LAN_AM79C973_BUFFER_SIZE ;
        mem.boundary = CACHE_LINE_SIZE ;
        mem.memory   = (void*)&ptmp ;
        rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                             &mem,
                             sizeof(t_sys_malloc) ) ;
        if (rcode != OK)
        {
            return( rcode ) ;
        }
        ptmp = KSEG1( ptmp ) ;
        for (i=0; i<LAN_AM79C973_RX_BUFFERS; i++)
        {
            pdevice->RxBuffer[i] = ptmp ;
            ptmp             += LAN_AM79C973_BUFFER_SIZE ;
        }

        /* register this event */
        first_time_init = 1 ;
    }

    /* Clear init block */
    memset( pdevice->pInitBlock, 0, LAN_AM79C973_INITBLOCK_SIZE ) ;

    /* Clear TX Descriptor Ring */
    for (i=0; i<LAN_AM79C973_TDRE_COUNT; i++)
    {
        memset( (void*)pdevice->TXDRE[i], 0, LAN_AM79C973_TDRE_SIZE ) ;
    }

    /* Clear RX Descriptor Ring */
    for (i=0; i<LAN_AM79C973_RDRE_COUNT; i++)
    {
        memset( (void*)pdevice->RXDRE[i], 0, LAN_AM79C973_RDRE_SIZE ) ;
    }

    /* Clear TX Data Buffers */
    for (i=0; i<LAN_AM79C973_TX_BUFFERS; i++)
    {
        memset( (void*)pdevice->TxBuffer[i], 0, LAN_AM79C973_BUFFER_SIZE ) ;
    }

    /* Clear RX Data Buffers */
    for (i=0; i<LAN_AM79C973_RX_BUFFERS; i++)
    {
        memset( (void*)pdevice->RxBuffer[i], 0, LAN_AM79C973_BUFFER_SIZE ) ;
    }

    return( OK );
}


/************************************************************************
 *
 *                          LAN_AM79C973_init_initblock
 *  Description :
 *  -------------
 *   This routine initializes the AM79C973 Init Block data structure.
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
INT32 LAN_AM79C973_init_initblock( t_LAN_AM79C973_device *pdevice )              
{
    volatile UINT32  *pib, *amd_base ;
    UINT32 tmp;

    pib = pdevice->pInitBlock ;

    /* Initialize Init Block word 0: TLEN=RLEN=4 (16), disable ring access */
    pib[0] = CPU_TO_LE32((LAN_AM79C973_INITBLOCK_TLEN<<INIT_WORD0_TLEN_SHF) |
                         (LAN_AM79C973_INITBLOCK_RLEN<<INIT_WORD0_RLEN_SHF) |
                         (                          0<<INIT_WORD0_DTX_SHF)  |
                         (                          0<<INIT_WORD0_DRX_SHF)  |
                         (                          3<<INIT_WORD0_PORTSEL_SHF));

    /* Initialize Init Block word 1&2: MAC-addres */
    amd_base = (UINT32 *) pdevice->p79C973Regs ;
    tmp    =  REG( amd_base, WORD0 ) ;
    pib[1] =  LE32_TO_CPU( tmp );
    tmp    =  REG( amd_base, WORD1 ) ;
    pib[2] =  LE32_TO_CPU( tmp );

    /* Initialize Init Block word 3&4: Logical filter-addres */
    pib[3] =  0;
    pib[4] =  0;

    /* Initialize Init Block word 5: Start of RX Descriptor Ring */
    tmp    =  PHYS( pdevice->RXDRE[0] );
    pib[5] =  LE32_TO_CPU( tmp );

    /* Initialize Init Block word 6: Start of TX Descriptor Ring */
    tmp    =  PHYS( pdevice->TXDRE[0] );
    pib[6] =  LE32_TO_CPU( tmp );

    return( OK );
}


/************************************************************************
 *
 *                          LAN_AM79C973_init_ring_descriptors
 *  Description :
 *  -------------
 *   This routine initializes the AM79C973 ring descriptor 
 *   data structures.
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
INT32 LAN_AM79C973_init_ring_descriptors( t_LAN_AM79C973_device *pdevice )              
{
    int i ;
    UINT32 tmp;

    /* Init TX Descriptor Ring */
    for (i=0; i<LAN_AM79C973_TDRE_COUNT; i++)
    {
        /* Link data buffer to descriptor */
        tmp = PHYS( pdevice->TxBuffer[i] ) ;
        REG( pdevice->TXDRE[i], TDE_WORD0 ) = CPU_TO_LE32( tmp ) ;

        /* Clear any status and control bits */
        REG( pdevice->TXDRE[i], TDE_WORD1 ) = 0 ;
        REG( pdevice->TXDRE[i], TDE_WORD2 ) = 0 ;
    }
    pdevice->NextTDREIndex = 0 ;

    /* Init RX Descriptor Ring */
    for (i=0; i<LAN_AM79C973_RDRE_COUNT; i++)
    {
        /* Link data buffer to descriptor */
        tmp = PHYS( pdevice->RxBuffer[i] ) ;
        REG( pdevice->RXDRE[i], RDE_WORD0 ) = CPU_TO_LE32( tmp ) ;

        /* Set buffer size, owner=controller */
        REG( pdevice->RXDRE[i], RDE_WORD1 ) =
	    CPU_TO_LE32( ((-LAN_AM79C973_BUFFER_SIZE) << RDE_WORD1_BCNT_SHF) |
	                 (                        0xf << RDE_WORD1_ONES_SHF) |
	                 (                          1 << RDE_WORD1_OWN_SHF) );
    }
    pdevice->NextRDREIndex = 0 ;

    return( OK );
}


/************************************************************************
 *
 *                          LAN_AM79C973_stop
 *  Description :
 *  -------------
 *   This routine stops the AM79C973 LAN controller
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
INT32 LAN_AM79C973_stop( t_LAN_AM79C973_device *pdevice )              
{
    UINT32 rvar ;

    /* CSR0: STOP=1 deassert any DMA activity, and disable interrupt */
    CSR_WRITE( pdevice->p79C973Regs, 0, 0x0004 )

    /* make a soft RESET of AM79C973 */
    LAN_AM79C973_RESET( pdevice->p79C973Regs, rvar )

    LAN_AM79C973_state = LAN_AM79C973_DRIVER_IS_STOPPED ;
    return( rvar ); /* keep compiler happy - return value is not used */
}


/************************************************************************
 *      Implementation : Device driver services
 ************************************************************************/



/************************************************************************
 *
 *                          LAN_AM79C973_init
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
INT32 LAN_AM79C973_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    bool   ic_in_use ;
    UINT32 int_line ;
    INT32  rcode ;
    UINT32 rvar ;
    t_LAN_AM79C973_device *pdevice ;

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;

    /* initialize shared services */
    LAN_error_lookup_init() ;

    /* get AM79C973 memory mapped base address */
    IF_ERROR( (rcode), 
              (SYSCON_read(SYSCON_BOARD_AM79C973_BASE_ID,
              &(pdevice->p79C973Regs),
              sizeof(pdevice->p79C973Regs)) ) )
    pdevice->p79C973Regs = (void *)KSEG1(pdevice->p79C973Regs) ;
    
    /* make a soft RESET of AM79C973 */
    LAN_AM79C973_RESET( pdevice->p79C973Regs, rvar )

    /* BCR20: Set softare style to 32 bit mode; 
              i.e. Init Block and DRE's are 32 bit structures */
    BCR_WRITE( pdevice->p79C973Regs, 20, 2)

    /* Setup PHY */
    IF_ERROR( (rcode), 
              (LAN_AM79C973_MII_setup(pdevice)) )

    /* allocate memory for LAN Init Block, ring descriptors, data buffers */
    IF_ERROR( (rcode), 
              (LAN_AM79C973_allocate_memory(pdevice)) )

    /* Init Init Block */
    IF_ERROR( (rcode), 
              (LAN_AM79C973_init_initblock(pdevice)) )

    /* Init ring descriptors */
    IF_ERROR( (rcode), 
              (LAN_AM79C973_init_ring_descriptors(pdevice)) )

    /* CSR1(LSA) & CSR2(MSA): Register Init Block */
    CSR_WRITE( pdevice->p79C973Regs, 1,  PHYS( pdevice->pInitBlock ) )
    CSR_WRITE( pdevice->p79C973Regs, 2, (PHYS( pdevice->pInitBlock ) >> 16) )

    /* Setup BCR registers; values expected to be present in EEPROM */
    rvar = 0 ;
    BCR_WRITE( pdevice->p79C973Regs, 9, rvar)
    rvar = 0x9ae1 ;
    BCR_WRITE( pdevice->p79C973Regs, 18, rvar)
    rvar = 0x1818 ;
    BCR_WRITE( pdevice->p79C973Regs, 22, rvar)
    rvar = 0x17 ;
    BCR_WRITE( pdevice->p79C973Regs, 25, rvar)
    rvar = 0x8 ;
    BCR_WRITE( pdevice->p79C973Regs, 26, rvar)
    rvar = 0 ;
    BCR_WRITE( pdevice->p79C973Regs, 27, rvar)

    /*
     * Set the No Underflow on Transmit (NOUFLO) bit, to avoid tx fifo errors
     * on 100Mbit, full duplex ethernets. This way the whole packet is
     * buffered in the fifo/sram before it is send.
     * Note the NOUFLO isn't implemented on all the supported controllers,
     * but as far as I can tell it is implemented on the 79C973 and 79C975
     * chips, if SRAM_SIZE > 0. For all others the transmit fifo threshold
     * value is set to max.  carstenl@mips.com
     */
    BCR_READ( pdevice->p79C973Regs,  18, rvar)
    rvar = rvar | (1<<11) ;
    BCR_WRITE( pdevice->p79C973Regs, 18, rvar)
    CSR_READ( pdevice->p79C973Regs,  80, rvar)
    rvar = rvar | (3<<10) ;
    CSR_WRITE( pdevice->p79C973Regs, 80, rvar)

    /* CSR0: INIT=1 assert initialization procedure                              */
    CSR_WRITE( pdevice->p79C973Regs, 0, 0x0001 )

    /* Await Initialization done */
    sys_wait_ms( 10 ) ; /* wait 10 ms */
    CSR_READ( pdevice->p79C973Regs,  0, rvar)
    if ( !(rvar & 0x0100) )
    {
        printf(" LAN_AM79C973_init: initialization failed\n") ;
        return( ERROR_LAN_INIT_FAILED ) ;
    }

    /* CSR0: STRT=1 start transmit and receive frame procedures */
    CSR_WRITE( pdevice->p79C973Regs, 0, 0x0002 )

    if (usr_receive == NULL)
    {
        /* get interrupt profile */
        IF_ERROR( (rcode),
                  (SYSCON_read( SYSCON_COM_EN0_IC_ID,
                                &(ic_in_use),
                                sizeof(ic_in_use)) ) )
        IF_ERROR( (rcode),
                  (SYSCON_read( SYSCON_COM_EN0_INTLINE_ID,
                                &(int_line),
                                sizeof(int_line)) ) )

        /* Register interrupt handler */
        if( ic_in_use )
        {
            EXCEP_register_ic_isr( int_line,
                                   (t_inthandler)LAN_AM79C973_receive,
                                   pdevice,
				   NULL );
        }
        else
        {
            EXCEP_register_cpu_isr( int_line,
                                    (t_inthandler)LAN_AM79C973_receive,
                                    pdevice,
				    NULL );
        }
    }

    /* CSR4: APAD_XMT=1 i.e. controller pads to minimum frame length (60 bytes)  */
    CSR_WRITE( pdevice->p79C973Regs, 4, 0x0914 )

    /* mask out TX interrupt: CSR3, bit 9 */
    CSR_WRITE( pdevice->p79C973Regs, 3, (1 << 9) )
    CSR_WRITE( pdevice->p79C973Regs, 5, (1 << 15) )

    /* enable interrupt: CSR0, bit 6 */
    CSR_WRITE( pdevice->p79C973Regs, 0, (1 << 6) )


    LAN_AM79C973_state = LAN_AM79C973_DRIVER_IS_STARTED ;

    return( OK ) ;
}


/************************************************************************
 *
 *                          LAN_AM79C973_open
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
INT32 LAN_AM79C973_open(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_OPEN_desc *p_param )   /* IN: receive handler reference */
{
    UINT32  old_ie ;

    /* register user defined receive handler */
    old_ie = sys_disable_int() ;
    usr_receive = p_param->receive ;
    if(old_ie) sys_enable_int();


    return( OK ) ;
}



/************************************************************************
 *
 *                          LAN_AM79C973_read
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
INT32 LAN_AM79C973_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param )   /* INOUT: LAN frame           */
{
    /* dummy function, as NET_poll, still calls this function */
    return( OK ) ;

}

/************************************************************************
 *
 *                          LAN_AM79C973_write
 *  Description :
 *  -------------
 *  This service requests transmission of a frame on the LAN interface. It is the caller's
 *  responsibility to fill in all information including the destination and source addresses and
 *  the frame type.  The length parameter gives the number of bytes in the ethernet frame.
 *  The routine will not return until the frame has been transmitted or an error has occured.  If
 *  the frame transmits successfully, OK is returned.  If an error occured, a message is sent to
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
INT32 LAN_AM79C973_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_IO_desc *p_param )   /* OUT: frame to transmit     */
{
    int    i ;
    int	   tdreidx ;
    UINT32 status, status2, control ;
    UINT8  *pb   = p_param->data ;
    UINT32 blen  = p_param->length ;
    t_LAN_AM79C973_device *pdevice ;
    UINT32 rcode, old_ie;


    if ( LAN_AM79C973_state == LAN_AM79C973_DRIVER_IS_STOPPED )
    {
        return( ERROR_LAN_TXM_ERROR ) ;
    }

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;
    tdreidx = pdevice->NextTDREIndex;

    i = 0 ;
    old_ie = sys_disable_int() ;
    while (1)
    {
        /* get status */
        status  = REG( pdevice->TXDRE[tdreidx], TDE_WORD1 ) ;
        status  = LE32_TO_CPU( status ) ;
        if ( ( ((status) & TDE_WORD1_OWN_MSK) >> TDE_WORD1_OWN_SHF) == HOST_IS_OWNER )
        {
            /* Check for error. */
            if (status & TDE_WORD1_ERR_MSK)
            {
                /* Some transmit error has been detected */
                pdevice->status.tx_errors++;
                if (status & TDE_WORD1_BPE_MSK )
                    pdevice->status.tx_bus_parity_errors++;
    
                status2 = REG( pdevice->TXDRE[tdreidx], TDE_WORD2 ) ;
                status2 = LE32_TO_CPU( status2 ) ;

                if (status2 & TDE_WORD2_LCOL_MSK )
                    pdevice->status.tx_window_errors++;
    
                if (status2 & TDE_WORD2_LCAR_MSK )
                    pdevice->status.tx_carrier_errors++;
    
                if (status2 & TDE_WORD2_UFLO_MSK )
                    pdevice->status.tx_fifo_errors++;
    
                pdevice->status.collisions +=
                          ((status2 & TDE_WORD2_TRC_MSK) >> TDE_WORD2_TRC_SHF ) ;
            }
            /* Clear error word for next transmit */
            REG( pdevice->TXDRE[tdreidx], TDE_WORD2 ) = 0 ;
    
            /* Copy data and request transmission of new buffer */
            memcpy( (void*)pdevice->TxBuffer[tdreidx], pb, blen ) ;
            control = ( (TDE_WORD1_OWN_MSK)  | 
                        (TDE_WORD1_STP_MSK)  |
                        (TDE_WORD1_ENP_MSK)  |
                        (TDE_WORD1_ONES_MSK) |
                        (((-blen) << TDE_WORD1_BCNT_SHF) & TDE_WORD1_BCNT_MSK) ) ;
            control = CPU_TO_LE32( control ) ;
            REG( pdevice->TXDRE[tdreidx], TDE_WORD1 ) = control ;
            pdevice->status.tx_bytes += blen ;
            pdevice->status.tx_packets++;
    
            /* Request immediate transmission */
            CSR_WRITE( pdevice->p79C973Regs, 0, 0x0048 )
    
            tdreidx++ ;
            if (tdreidx == LAN_AM79C973_TDRE_COUNT)
                tdreidx = 0 ;
            pdevice->NextTDREIndex = tdreidx ;
            rcode = OK ;
            break ;
        }
        else
        {
            /* wait 1 ms */
            sys_wait_ms( 1 ) ;
    
            /* wait at most 1 second */
            i++ ;
            if ( i > 1000 )
            {
                pdevice->status.tx_errors++;
                pdevice->status.tx_timeout_errors++;
    
                /* re-init driver and controller */
                LAN_AM79C973_init( 0, 0, NULL ) ;
                rcode = ERROR_LAN_TXM_ERROR ;
                break ;
            }
    
        }
    }
    if(old_ie) sys_enable_int();
    return( rcode ) ;
}

/************************************************************************
 *
 *                          LAN_AM79C973_ctrl
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
INT32 LAN_AM79C973_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_LAN_CTRL_desc *p_param )   /* IN-OUT:                       */
{
    t_LAN_AM79C973_device  *pdevice ;
    UINT32                 old_ie ;

    /* get device context for this minor device */
    pdevice = &minor_device[minor] ;

    switch( p_param->command )
    {
      case LAN_CTRL_DISPLAY_STATISTICS:
#ifdef ETH_DEBUG
          //LAN_AM79C973_dump_regs( pdevice ) ;
          LAN_AM79C973_dump_descriptors( pdevice ) ;
          LAN_AM79C973_dump_device( pdevice ) ;
#endif
          LAN_AM79C973_MII_status( pdevice ) ;
          LAN_AM79C973_dump_status( pdevice ) ;
      break ;

      case LAN_CTRL_STOP_CONTROLLER:
          LAN_AM79C973_stop( pdevice ) ;
      break ;

      case LAN_CTRL_START_CONTROLLER:
          /* re-init driver and controller */
          old_ie = sys_disable_int() ;
          LAN_AM79C973_init( 0, 0, NULL ) ;
          if(old_ie) sys_enable_int();
      break ;

      default:
      break ;
    }
    return( OK ) ;
}


/************************************************************************
 *
 *                          LAN_AM79C973_dump_regs
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller registers
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
INT32 LAN_AM79C973_dump_regs( t_LAN_AM79C973_device *pdevice )
{
    UINT32 rvar ;
    volatile void *plan ;

    plan = pdevice->p79C973Regs ;

    BCR_READ( plan, 2, rvar )
    printf(" BCR2  = %08x\n", rvar ) ;
    BCR_READ( plan, 4, rvar )
    printf(" BCR4  = %08x\n", rvar ) ;
    BCR_READ( plan, 5, rvar )
    printf(" BCR5  = %08x\n", rvar ) ;
    BCR_READ( plan, 6, rvar )
    printf(" BCR6  = %08x\n", rvar ) ;
    BCR_READ( plan, 7, rvar )
    printf(" BCR7  = %08x\n", rvar ) ;
    BCR_READ( plan, 9, rvar )
    printf(" BCR9  = %08x\n", rvar ) ;
    BCR_READ( plan, 18, rvar )
    printf(" BCR18  = %08x\n", rvar ) ;
    BCR_READ( plan, 22, rvar )
    printf(" BCR22  = %08x\n", rvar ) ;
    BCR_READ( plan, 23, rvar )
    printf(" BCR23  = %08x\n", rvar ) ;
    BCR_READ( plan, 24, rvar )
    printf(" BCR24  = %08x\n", rvar ) ;
    BCR_READ( plan, 25, rvar )
    printf(" BCR25  = %08x\n", rvar ) ;
    BCR_READ( plan, 26, rvar )
    printf(" BCR26  = %08x\n", rvar ) ;
    BCR_READ( plan, 27, rvar )
    printf(" BCR27  = %08x\n", rvar ) ;
    BCR_READ( plan, 32, rvar )
    printf(" BCR32  = %08x\n", rvar ) ;
    BCR_READ( plan, 33, rvar )
    printf(" BCR33  = %08x\n", rvar ) ;
    BCR_READ( plan, 35, rvar )
    printf(" BCR35  = %08x\n", rvar ) ;
    BCR_READ( plan, 36, rvar )
    printf(" BCR36  = %08x\n", rvar ) ;
    BCR_READ( plan, 37, rvar )
    printf(" BCR37  = %08x\n", rvar ) ;
    BCR_READ( plan, 38, rvar )
    printf(" BCR38  = %08x\n", rvar ) ;
    BCR_READ( plan, 39, rvar )
    printf(" BCR39  = %08x\n", rvar ) ;
    BCR_READ( plan, 40, rvar )
    printf(" BCR40  = %08x\n", rvar ) ;
    BCR_READ( plan, 41, rvar )
    printf(" BCR41  = %08x\n", rvar ) ;
    BCR_READ( plan, 42, rvar )
    printf(" BCR42  = %08x\n", rvar ) ;
    BCR_READ( plan, 43, rvar )
    printf(" BCR43  = %08x\n", rvar ) ;
    BCR_READ( plan, 44, rvar )
    printf(" BCR44  = %08x\n", rvar ) ;
    BCR_READ( plan, 45, rvar )
    printf(" BCR45  = %08x\n", rvar ) ;
    BCR_READ( plan, 46, rvar )
    printf(" BCR46  = %08x\n", rvar ) ;
    BCR_READ( plan, 47, rvar )
    printf(" BCR47  = %08x\n", rvar ) ;

    CSR_READ( plan, 12, rvar )
    printf(" CSR12  = %08x\n", rvar ) ;
    CSR_READ( plan, 13, rvar )
    printf(" CSR13  = %08x\n", rvar ) ;
    CSR_READ( plan, 14, rvar )
    printf(" CSR14  = %08x\n", rvar ) ;
    CSR_READ( plan, 116, rvar )
    printf(" CSR116  = %08x\n", rvar ) ;

    return( OK ) ;
}
#endif

/************************************************************************
 *
 *                          LAN_AM79C973_dump_status
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller statistics
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
void LAN_AM79C973_dump_status( t_LAN_AM79C973_device *pdevice )
{
    sprintf( msg, " Packets received:               %u\n\r", pdevice->status.rx_packets ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Packets transmitted:            %u\n\r", pdevice->status.tx_packets ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Bytes received:                 %u\n\r", pdevice->status.rx_bytes ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Bytes transmitted:              %u\n\r", pdevice->status.tx_bytes ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive errors:                 %u\n\r", pdevice->status.rx_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit errors:                %u\n\r", pdevice->status.tx_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Multicasts received:            %u\n\r", pdevice->status.multicast ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Collisions:                     %u\n\r", pdevice->status.collisions) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Interrupts:                     %u\n\r", pdevice->status.interrupts) ;
    PUTS( DEFAULT_PORT, msg ) ;
#ifdef ETH_DEBUG
    sprintf( msg, "     rx_zero_length_errors:      %u\n\r", pdevice->status.rx_zero_length_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, "     rx_buffer_length_errors:    %u\n\r", pdevice->status.rx_buffer_length_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
#endif
    sprintf( msg, " Receive CRC errors:             %u\n\r", pdevice->status.rx_crc_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive frame error:            %u\n\r", pdevice->status.rx_frame_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Receive FIFO overrun errors :   %u\n\r", pdevice->status.rx_fifo_errors ) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit aborted errors:        %u\n\r", pdevice->status.tx_aborted_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit lost carrier errors:   %u\n\r", pdevice->status.tx_carrier_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit FIFO underrun errors:  %u\n\r", pdevice->status.tx_fifo_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit signal quality errors: %u\n\r", pdevice->status.tx_heartbeat_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit late collision errors: %u\n\r", pdevice->status.tx_window_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
    sprintf( msg, " Transmit timeout errors:        %u\n\r", pdevice->status.tx_timeout_errors) ;
    PUTS( DEFAULT_PORT, msg ) ;
}

/************************************************************************
 *
 *                          LAN_AM79C973_MII_setup
 *  Description :
 *  -------------
 *    Setup MII: if PCI clock < 33 MHz, then force 10 MBit/sec and 
 *    half duplex; else setup MII for autonegotiate.  
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
INT32 LAN_AM79C973_MII_setup( t_LAN_AM79C973_device *pdevice )
{
    UINT32 pci_clk_freq ;
    UINT32 rcode ;

    /* Get PCI bus frequency */
    rcode = SYSCON_read( SYSCON_BOARD_PCI_FREQ_KHZ_ID,
                         (void *)&pci_clk_freq,
                         sizeof(UINT32) ) ;
    if ( rcode ) return( rcode ) ;
    
    if ( pci_clk_freq < 33000 )
    {
        /* PCI clock is below 33 MHz: 
           force PHY into speed='10 MBit/sec' */
        BCR_WRITE( pdevice->p79C973Regs, 32, 0x4080 )
        sys_wait_ms( 10 ) ;
        BCR_WRITE( pdevice->p79C973Regs, 32, 0x4000 )
    }
    else
    {
        /* PCI clock is above 33 MHz: 
           let PHY enter autonegotiate mode, as configured in the EEPROM */
    }

    return(OK) ;
}


/************************************************************************
 *
 *                          LAN_AM79C973_MII_status
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
INT32 LAN_AM79C973_MII_status( t_LAN_AM79C973_device *pdevice )
{
    char   msg[120] ;
    UINT32 rvar ;

    BCR_WRITE( pdevice->p79C973Regs, 33, (PHY_ADDRESS | PHY_SUMMARY) )
    BCR_READ(  pdevice->p79C973Regs, 34, rvar )

    if ( rvar & PHY_SUMMARY_LINK_UP )
    {
        sprintf( msg, " MII status: Link is up, mode: " ) ;
        if ( rvar & PHY_SUMMARY_SPEED_100MB )
        {
            strcat( msg, "100 Mbit/s, " ) ;
        }
        else
        {
            strcat( msg, "10 Mbit/s, " ) ;
        }
        if ( rvar & PHY_SUMMARY_FULL_DUPLEX)
        {
            strcat( msg, "full-duplex\n\n" ) ;
        }
        else
        {
            strcat( msg, "half-duplex\n\n" ) ;
        }
        PUTS( DEFAULT_PORT, msg ) ;
    }
    else
    {
        printf(" MII status: Link is down !\n\n" ) ;
    }

    return(OK) ;
}


/************************************************************************
 *
 *                          LAN_AM79C973_dump_descriptors
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller TX and RX ring descriptors
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
INT32 LAN_AM79C973_dump_descriptors( t_LAN_AM79C973_device *pdevice )
{
    UINT32 word0, word1, word2 ;
    int    i ;

    printf( "\n\n LAN_AM79C973_dump_descriptors:\n\n") ;

    printf(" p79C973Regs:    0x%08X\n", (UINT32) pdevice->p79C973Regs   ) ;
    printf(" pInitBlock:     0x%08X\n", (UINT32) pdevice->pInitBlock    ) ;
    printf(" NextTDREIndex:  %d\n",     pdevice->NextTDREIndex ) ;
    printf(" NextRDREIndex:  %d\n",     pdevice->NextRDREIndex ) ;

    /* Display TX Descriptor Ring status */
    for ( i=0; i < LAN_AM79C973_TDRE_COUNT; i++ )
    {
        word0 = REG( pdevice->TXDRE[i], TDE_WORD0 ) ;
        word0 = LE32_TO_CPU( word0 ) ;
        word1 = REG( pdevice->TXDRE[i], TDE_WORD1 ) ;
        word1 = LE32_TO_CPU( word1 ) ;
        word2 = REG( pdevice->TXDRE[i], TDE_WORD2 ) ;
        word2 = LE32_TO_CPU( word2 ) ;
        printf("   TXDRE %d, 0x%08X 0x%08X 0x%08X\n", i, word0, word1, word2 ) ;
    }

    printf("\n\n") ;

    /* Display RX Descriptor Ring status */
    for ( i=0; i < LAN_AM79C973_RDRE_COUNT; i++ )
    {
        word0 = REG( pdevice->RXDRE[i], RDE_WORD0 ) ;
        word0 = LE32_TO_CPU( word0 ) ;
        word1 = REG( pdevice->RXDRE[i], RDE_WORD1 ) ;
        word1 = LE32_TO_CPU( word1 ) ;
        word2 = REG( pdevice->RXDRE[i], RDE_WORD2 ) ;
        word2 = LE32_TO_CPU( word2 ) ;
        printf("   RXDRE %d, 0x%08X 0x%08X 0x%08X\n", i, word0, word1, word2 ) ;
    }
    return( OK ) ;
}
#endif

/************************************************************************
 *
 *                         LAN_AM79C973_receive
 *  Description :
 *  -------------
 *    Receive a packet 
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
INT32 LAN_AM79C973_receive( t_LAN_AM79C973_device *pdevice )
{
    int    rdreidx ;
    UINT32 status, control ;
    UINT8  *pb  ;
    UINT32 blen, intstat ;
#ifdef ETH_DEBUG
    UINT32 intstat4, intstat5, intstat7  ;
#endif

    /* increment number of interrupts */
    pdevice->status.interrupts++;

    rdreidx = pdevice->NextRDREIndex;

    /* Get interrupt status */
    CSR_READ( pdevice->p79C973Regs, 0, intstat )
#ifdef ETH_DEBUG
    CSR_READ( pdevice->p79C973Regs, 4, intstat4 )
    CSR_READ( pdevice->p79C973Regs, 5, intstat5 )
    CSR_READ( pdevice->p79C973Regs, 7, intstat7 )
    printf("1 %x %x %x %x\n", intstat, intstat4, intstat5, intstat7 ) ;
#endif

    /* Acknowledge interrupts */
    CSR_WRITE( pdevice->p79C973Regs, 0, ((intstat & ~0x004f) & 0xffff) )


    /* Check for RX interrupt */
    if (intstat & (1 << 10))
    {
#ifdef ETH_DEBUG
        /* Get interrupt status */
        CSR_READ( pdevice->p79C973Regs, 0, intstat )
        CSR_READ( pdevice->p79C973Regs, 4, intstat4 )
        CSR_READ( pdevice->p79C973Regs, 5, intstat5 )
        CSR_READ( pdevice->p79C973Regs, 7, intstat7 )
        printf("2 %x %x %x %x\n", intstat, intstat4, intstat5, intstat7 ) ;
#endif

        while (1) 
        {
            /* get status */
            status  = REG( pdevice->RXDRE[rdreidx], RDE_WORD1 ) ;
            status  = LE32_TO_CPU( status );

            if ( ( ((status) & RDE_WORD1_OWN_MSK) >> RDE_WORD1_OWN_SHF) == HOST_IS_OWNER )
            {
                /* Check for error. */
                if (status & RDE_WORD1_ERR_MSK)
                {
                    /* Some receive error has been detected */
    
                    /* received packet has  errors, drop this packet */
                    pdevice->status.rx_errors++;
                    if (status & RDE_WORD1_CRC_MSK)
                         pdevice->status.rx_crc_errors++;
    
                    if (status & RDE_WORD1_FRAM_MSK)
                         pdevice->status.rx_frame_errors++;
    
                    if (status & RDE_WORD1_OFLO_MSK)
                         pdevice->status.rx_over_errors++;
    
                    if (status & RDE_WORD1_BUFF_MSK)
                         pdevice->status.rx_fifo_errors++;
                }
                else
                {
                    /* We may have a good packet */
                    if ( (status & RDE_WORD1_STP_MSK) &&
                         (status & RDE_WORD1_ENP_MSK) )
                    {
                        /* Good packet: Start and End of packet received */
                        blen = REG( pdevice->RXDRE[rdreidx], RDE_WORD2 ) ;
                        blen = LE32_TO_CPU( blen ) ;
                        blen = ((blen & RDE_WORD2_MCNT_MSK) >> RDE_WORD2_MCNT_SHF) -
                               LAN_AM79C973_FCS_SIZE  ;
                        pb   = (void*)pdevice->RxBuffer[rdreidx] ;
    
                        pdevice->status.rx_bytes += blen ;
                        pdevice->status.rx_packets++;
    
                        if (usr_receive != NULL)
                        {
                            /* we have a registered receive handler */
                            if ( blen == 0 )
                            {
                                /* drop the packet */
                                pdevice->status.rx_errors++;
                                pdevice->status.rx_zero_length_errors++;
                            }
                            else
                            {
                                /* count any broadcast's */
                                if ( !memcmp( pb, mac_broadcast_adr, SYS_MAC_ADDR_SIZE ) )
                                {
                                    pdevice->status.multicast++ ;
                                }
    
                                /* call the handler */
                                (*usr_receive)( blen, pb ) ;
    
                            }
                        }
                        else
                        {
                            /* we don't have a registered receive handler */
         
                            /* drop the packet */
                            pdevice->status.rx_errors++;
                            pdevice->status.rx_no_resource_errors++;
    
                            if ( blen == 0 )
                            {
                                /* drop the packet */
                                pdevice->status.rx_zero_length_errors++;
                            }
         
                            /* count any broadcast's */
                            if ( !memcmp( pb, mac_broadcast_adr, SYS_MAC_ADDR_SIZE ) )
                            {
                                pdevice->status.multicast++ ;
                            }
                        }
                    }
                    else
                    {
                        /* Some weird case */
                        pdevice->status.rx_errors++;
                        pdevice->status.rx_buffer_length_errors++;
                    }
                }

                /* Acknowledge this packet */
                REG( pdevice->RXDRE[rdreidx], RDE_WORD1 ) = 0 ;
                control = ( (RDE_WORD1_OWN_MSK)  | 
                            (RDE_WORD1_ONES_MSK) |
                            (((-LAN_AM79C973_BUFFER_SIZE) << RDE_WORD1_BCNT_SHF) & RDE_WORD1_BCNT_MSK) ) ;
                REG( pdevice->RXDRE[rdreidx], RDE_WORD1 ) = CPU_TO_LE32( control ) ;
    
                /* advance to next receive descriptor */
                rdreidx++ ;
                if (rdreidx == LAN_AM79C973_RDRE_COUNT)
                    rdreidx = 0 ;
                pdevice->NextRDREIndex = rdreidx ;
    
            }
            else
            {
                /* No, more packets */
                break;
            }
        }
    }

    /* Acknowledge rest of pending interrupts, and re-enable INT */
    CSR_WRITE( pdevice->p79C973Regs, 0, 0x7940 )

#ifdef ETH_DEBUG
    /* Get interrupt status */
    CSR_READ( pdevice->p79C973Regs, 0, intstat )
    CSR_READ( pdevice->p79C973Regs, 4, intstat4 )
    CSR_READ( pdevice->p79C973Regs, 5, intstat5 )
    CSR_READ( pdevice->p79C973Regs, 7, intstat7 )
    printf("3 %x %x %x %x\n", intstat, intstat4, intstat5, intstat7 ) ;
    //CSR_WRITE( pdevice->p79C973Regs, 5, intstat1 )
    //CSR_WRITE( pdevice->p79C973Regs, 7, intstat2 )
#endif

    return OK ;

}


/************************************************************************
 *
 *                          LAN_AM79C973_dump_device
 *  Description :
 *  -------------
 *    Dump all AM79C973 LAN controller TX and RX ring descriptors
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
INT32 LAN_AM79C973_dump_device( t_LAN_AM79C973_device *pdevice )
{
    int    i ;

    printf( "\n\n LAN_AM79C973_dump_device:\n\n") ;

    printf(" p79C973Regs:    0x%08X\n", (UINT32) pdevice->p79C973Regs   ) ;
    printf(" pInitBlock:     0x%08X\n", (UINT32) pdevice->pInitBlock    ) ;
    printf(" NextTDREIndex:  %d\n",     pdevice->NextTDREIndex ) ;
    printf(" NextRDREIndex:  %d\n",     pdevice->NextRDREIndex ) ;

    /* Display TX Descriptor Ring */
    for ( i=0; i < LAN_AM79C973_TDRE_COUNT; i++ )
    {
        printf("   TXDRE %d, 0x%08X\n", i, pdevice->TXDRE[i] ) ;
    }

    printf("\n\n") ;

    /* Display RX Descriptor Ring */
    for ( i=0; i < LAN_AM79C973_RDRE_COUNT; i++ )
    {
        printf("   RXDRE %d, 0x%08X\n", i, pdevice->RXDRE[i] ) ;
    }

    /* Display TX Buffers */
    for ( i=0; i < LAN_AM79C973_TX_BUFFERS; i++ )
    {
        printf("   TxBuffer %d, 0x%08X\n", i, pdevice->TxBuffer[i] ) ;
    }

    /* Display RX Buffers */
    for ( i=0; i < LAN_AM79C973_RX_BUFFERS; i++ )
    {
        printf("   RxBuffer %d, 0x%08X\n", i, pdevice->RxBuffer[i] ) ;
    }

    return( OK ) ;
}
#endif

