
/************************************************************************
 *
 *      SERIAL_SAA9730.c
 *
 *      The 'SERIAL_SAA9730' module implements the SERIAL-SAA9730
 *      device driver as an IO device with following services:
 *
 *        1) init  device:  configure and initialize serial driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  read character from serial device
 *        5) write device:  write character to serial device
 *        6) ctrl  device:  statistics and poll for ctrl/c
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

#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <io_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <serial_api.h>
#include <serial_saa9730_api.h>
#include <excep_api.h>
#include <string.h>

/************************************************************************
 *  Constants
*************************************************************************/

/* Offset addresses for the SAA9730 EVM (Event monitor) */
#define EVM_SAA9730_BASE_OFS     0x2000
#define EVM_SAA9730_ISR_OFS      (0x00>>2)
#define EVM_SAA9730_IER_OFS      (0x04>>2)
#define EVM_SAA9730_IMR_OFS      (0x08>>2)
#define EVM_SAA9730_IER_MASTER   0x00000001
#define EVM_SAA9730_IER_ALL      0xF7FFFFFF
#define EVM_SAA9730_GPIO0        0x00000010  /* ~RI */
#define EVM_SAA9730_GPIO1        0x00000020  /* ~DSR */

#define EVM_SAA9730_SECISR_OFS   (0x1C>>2)
#define EVM_SAA9730_SECIER_OFS   (0x20>>2)
#define EVM_SAA9730_SECIMR_OFS   (0x24>>2)
#define EVM_SAA9730_SECIER_UART1 0x00000008
#define EVM_SAA9730_SECIER_ALL   0xFFFFFFFF

#define EVM_SAA9730_IERSW_OFS    (0x2C>>2)
#define EVM_SAA9730_SECIERSW_OFS (0x30>>2)


/* Offset addresses for the SAA9730 UART mapped as 8-bit device on PCI  */
#define SERIAL_SAA9730_UART_OFS  0x21800 /* UART-device offset          */
#define SERIAL_SAA9730_RBR_OFS   0x00    /* Receiver Buffer             */
#define SERIAL_SAA9730_THR_OFS   0x00    /* Transmitter Holding         */
#define SERIAL_SAA9730_IER_OFS   0x01    /* Interrupt enable            */
#define SERIAL_SAA9730_IIR_OFS   0x02    /* Interrupt identification    */
#define SERIAL_SAA9730_FCR_OFS   0x02    /* FIFO control                */
#define SERIAL_SAA9730_LCR_OFS   0x03    /* Line control                */
#define SERIAL_SAA9730_MCR_OFS   0x04    /* Modem control               */
#define SERIAL_SAA9730_LSR_OFS   0x05    /* Line status                 */
#define SERIAL_SAA9730_MSR_OFS   0x06    /* Modem status                */
#define SERIAL_SAA9730_SCR_OFS   0x07    /* Scratch                     */
#define SERIAL_SAA9730_DLL_OFS   0x00    /* Divisor latch LSB           */
#define SERIAL_SAA9730_DLM_OFS   0x01    /* Divisor latch MSB           */
#define SERIAL_SAA9730_CFG_OFS   0x0c    /* Configuration register      */

/* FCR control */
#define SERIAL_FCR_ENABLE        0x01    /* enable FIFO's               */
#define SERIAL_FCR_RCVR_RESET    0x02    /* reset receiver FIFO         */
#define SERIAL_FCR_TXMT_RESET    0x04    /* reset transmit FIFO         */
#define SERIAL_FCR_RCVFIFO_1     0x00    /* receive FIFO threshold 1    */
#define SERIAL_FCR_RCVFIFO_4     0x40    /* receive FIFO threshold 4    */
#define SERIAL_FCR_RCVFIFO_8     0x80    /* receive FIFO threshold 8    */
#define SERIAL_FCR_RCVFIFO_14    0xc0    /* receive FIFO threshold 14   */

/* IER mask */
#define SERIAL_IER_RCVEN         0x01    /* receive interrupt enable    */

/* LCR control */
#define SERIAL_LCR_DATA5         0x00    /* 5-bit character             */
#define SERIAL_LCR_DATA6         0x01    /* 6-bit character             */
#define SERIAL_LCR_DATA7         0x02    /* 7-bit character             */
#define SERIAL_LCR_DATA8         0x03    /* 8-bit character             */
#define SERIAL_LCR_DATA_MSK      0x03    /* MASK for data field         */

#define SERIAL_LCR_STOP1         0x00    /* 1 stop bit                  */
#define SERIAL_LCR_STOP15        0x04    /* 1.5 stop bit, if data is 5-bit */
#define SERIAL_LCR_STOP2         0x04    /* 2 stop bit                  */
#define SERIAL_LCR_STOP_MSK      0x04    /* MASK for stop bit field     */

#define SERIAL_LCR_PARITYNONE    0x00    /* No parity                   */
#define SERIAL_LCR_PARITYODD     0x08    /* Odd parity                  */
#define SERIAL_LCR_PARITYEVEN    0x18    /* Even parity                 */
#define SERIAL_LCR_PARITYMARK    0x28    /* Mark parity                 */
#define SERIAL_LCR_PARITYSPACE   0x38    /* Space parity                */
#define SERIAL_LCR_PARITY_MASK   0x38    /* MASK for parity field       */

#define SERIAL_LCR_BREAK         0x40    /* request to send a 'break'   */
#define SERIAL_LCR_DLAB          0x80    /* enable divisor latch registers */

/* MCR control */
#define SERIAL_MCR_DTR           0x01    /* Data Terminal Ready         */
#define SERIAL_MCR_RTS           0x02    /* Request To Send             */
#define SERIAL_MCR_OUT1          0x04    /* General purpose output      */
#define SERIAL_MCR_OUT2          0x08    /* Interrupt enable            */

#define SERIAL_MCR_LOOP          0x10    /* Local loop back             */
#define SERIAL_MCR_AFE           0x20    /* Auto-flow control enable    */

/* LSR status */
#define SERIAL_LSR_DR            0x01    /* Character ready             */
#define SERIAL_LSR_OE            0x02    /* RX-ERROR: Overrun           */
#define SERIAL_LSR_PE            0x04    /* RX-ERROR: Parity            */
#define SERIAL_LSR_FE            0x08    /* RX-ERROR: Framing (stop bit)*/
#define SERIAL_LSR_BI            0x10    /* 'BREAK' detected            */
#define SERIAL_LSR_THRE          0x20    /* Transmit Holding empty      */
#define SERIAL_LSR_TEMT          0x40    /* Transmitter empty (IDLE)    */
#define SERIAL_LSR_FIFOERR       0x80    /* RX-ERROR: FIFO              */


/* MSR status */
#define SERIAL_MSR_DCTS          0x01    /* Delta clear to send         */
#define SERIAL_MSR_DDSR          0x02    /* Delta data set ready        */
#define SERIAL_MSR_TERI          0x04    /* Trailing edge ring ind.     */
#define SERIAL_MSR_DDCD          0x08    /* Delta data carrier detect   */
#define SERIAL_MSR_CTS           0x10    /* Clear to send               */
#define SERIAL_MSR_DSR           0x20    /* Data Set Ready              */
#define SERIAL_MSR_RI            0x40    /* Ring Indicator              */
#define SERIAL_MSR_DCD           0x80    /* Data carrier detect         */

/* CFR control */
#define SERIAL_CFG_BREAK         0x02    /* Sends a break               */
#define SERIAL_CFG_RTSEN         0x10    /* Enable RTS auto flow control*/
#define SERIAL_CFG_CTSEN         0x20    /* Enable CTS auto tx inhibit  */



/************************************************************************
 *  Macro Definitions
*************************************************************************/


/************************************************************************
 *  Type Definitions
*************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/


/* All physical register addresses are initialized here to
   increase speed. 

   The SAA9730 is sitting on the PCI bus with 8-bit device access 
                                                                     */
static volatile UINT8 *phy_rbr ;  /* '0', RO, receive buffer,       */
static volatile UINT8 *phy_thr ;  /* '0', WO, trans. holding,       */
static volatile UINT8 *phy_ier ;  /* '1', RW, int. enable,          */
static volatile UINT8 *phy_iir ;  /* '2', RO, int. ident,           */
static volatile UINT8 *phy_fcr ;  /* '2', WO, FIFO control,         */
static volatile UINT8 *phy_lcr ;  /* '3', RW, line control,         */
static volatile UINT8 *phy_mcr ;  /* '4', RW, modem control,        */
static volatile UINT8 *phy_lsr ;  /* '5', RW, line status,          */
static volatile UINT8 *phy_msr ;  /* '6', RW, modem status,         */
static volatile UINT8 *phy_scr ;  /* '7', RW, scratch               */
static volatile UINT8 *phy_dll ;  /* '0', RW, divisor latch LSB     */
static volatile UINT8 *phy_dlm ;  /* '1', RW, divisor latch MSB     */
static volatile UINT8 *phy_cfg ;  /* 'c', RW, configuration coltrol */

static volatile UINT32 *evm_base ; /* Event monitor base address */
/*
   Baudrate conversion table for UART divisor latch.
   EEPROM-encoding to divisor values at 3.6923 MHz crystal
 */
static const UINT16 serial_baudrate[SERIAL_BAUDRATE_MAX] = {
        SERIAL_ILLEGAL,   /*  0:  not defined   */
                0x0c04,   /*  1:      75 baud   */
                0x0831,   /*  2:     110 baud   */
                0x0601,   /*  3:     150 baud   */
                0x0300,   /*  4:     300 baud   */
                0x017f,   /*  5:     600 baud   */
                   191,   /*  6:   1.200 baud   */
                   127,   /*  7:   1.800 baud   */
                    95,   /*  8:   2.400 baud   */
                    47,   /*  9:   4.800 baud   */
                    31,   /* 10:   7.200 baud   */
                    23,   /* 11:   9.600 baud   */
                    15,   /* 12:  14.400 baud   */
                    11,   /* 13:  19.200 baud   */
                     5,   /* 14:  38.400 baud   */
                     3,   /* 15:  57.600 baud   */
                     1,   /* 16: 115.200 baud   */
                     0,   /* 17: 230.400 baud   */
        SERIAL_ILLEGAL,   /* 18: 460.800 baud   */
        SERIAL_ILLEGAL    /* 19: 921.600 baud   */
} ;


/*
   Databit conversion table.
   EEPROM-encoding to device control
 */
static const UINT8 serial_databits[SERIAL_DATABITS_MAX] = {

    SERIAL_ILLEGAL,   /* 0, undefined  */
    SERIAL_LCR_DATA7, /* 1, 7 databits */
    SERIAL_LCR_DATA8  /* 2, 8 databits */

} ;


/*
   Parity conversion table.
   EEPROM-encoding to device control
 */
static const UINT8 serial_parity[SERIAL_PARITY_MAX] = {

    SERIAL_ILLEGAL,        /* 0, undefined  */
    SERIAL_LCR_PARITYNONE, /* 1, none       */
    SERIAL_LCR_PARITYODD,  /* 2, odd        */
    SERIAL_LCR_PARITYEVEN  /* 3, even       */

} ;


/*
   Stopbits conversion table.
   EEPROM-encoding to device control
 */
static const UINT8 serial_stopbits[SERIAL_STOPBITS_MAX] = {

    SERIAL_ILLEGAL,     /* 0, undefined   */
    SERIAL_LCR_STOP1,   /* 1, 1 stopbit   */
    SERIAL_LCR_STOP15,  /* 2, 1.5 stopbit */
    SERIAL_LCR_STOP2    /* 3, 2 stopbit   */

} ;


static t_UART_statistics uart_statistics ;
static UINT32 poll_retcode ;
static UINT32 shadow_ier ;
static UINT32 shadow_mcr = (SERIAL_MCR_DTR | SERIAL_MCR_OUT2) ;
static UINT32 shadow_flow ;

/************************************************************************
** receive buffer operation:
**
** putptr points to the next free location
** When a byte is polled from the uart, it is stored by putptr,
** which is then cyclic incremented UNLESS it gets equal to getptr.
**
** That way, getptr == putptr means buffer empty, and
**           the buffer can hold POLLSIZE-1 bytes.
*/ 

#define POLLSIZE	0x800   /* Must be power of 2 and at least 128 */
#define HW_LIMIT_STOP	(POLLSIZE-64)  /* RTS OFF when 64 chars in buf */
#define HW_LIMIT_START	(POLLSIZE-32)  /* RTS  ON when 32 chars in buf */

static UINT16 *recv_putptr ;
static UINT16 *recv_getptr ;
static UINT16 *recv_flushptr ;
static UINT16  recv_buffer[POLLSIZE] ;

/*  Boolean indicating whether interrupt handler is registered or not */
static bool registered = FALSE;


/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static 
INT32 SERIAL_SAA9730_init(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT32 *port );        /* IN: port mapping			  */

static
INT32 SERIAL_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          UINT8  *p_param ) ;    /* OUT: character been read            */

static
UINT32 SERIAL_SAA9730_irqpoll( void ) ;

static
UINT32 SERIAL_SAA9730_irq( UINT32 in_intrpt ) ;

static
INT32 SERIAL_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          UINT8  *p_param ) ;    /* IN: character to write              */

static
INT32 SERIAL_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_SERIAL_ctrl_descriptor  *p_param ) ; /* IN: IOCTL structure */

static void
serial_int_handler(
    void *dummy );

/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          SERIAL_SAA9730_install
 *  Description :
 *  -------------
 *
 *  Installs the serial SAA9730 serial device drivers services in 
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
INT32 SERIAL_SAA9730_install( void )
{
    UINT32  tty;
    UINT32  major, minor;

    /* pre-initialize local variables and install device services */
    IO_install( SYS_MAJOR_SERIAL_SAA9730,         /* major device number */
         (t_io_service) SERIAL_SAA9730_init,      /* 'init'  service     */
                        NULL,                     /* 'open'  service  na */
                        NULL,                     /* 'close' service  na */
         (t_io_service) SERIAL_SAA9730_read,      /* 'read'  service     */
         (t_io_service) SERIAL_SAA9730_write,     /* 'write' service     */
         (t_io_service) SERIAL_SAA9730_ctrl ) ;   /* 'ctrl' service      */

    /* call our own 'init' service */

    /* TTY0 */
    tty = PORT_TTY0;
    SYSCON_read( SYSCON_COM_TTY0_MAJOR, (void *)(&major), sizeof(UINT32) );
    
    if (major == SYS_MAJOR_SERIAL_SAA9730)
    {
        SYSCON_read(SYSCON_COM_TTY0_MINOR, (void *)(&minor), sizeof(UINT32));
	IO_init( SYS_MAJOR_SERIAL_SAA9730, minor, &tty );
    }


    /* TTY1 */
    tty = PORT_TTY1;
    SYSCON_read( SYSCON_COM_TTY1_MAJOR, (void *)(&major), sizeof(UINT32) );
    if (major == SYS_MAJOR_SERIAL_SAA9730)
    {
	SYSCON_read(SYSCON_COM_TTY1_MINOR, (void *)(&minor), sizeof(UINT32));
	IO_init( SYS_MAJOR_SERIAL_SAA9730, minor, &tty );
    }
    return OK;
}


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *      Implementation : Device driver services
 ************************************************************************/

/************************************************************************
 *
 *                          SERIAL_SAA9730_init
 *  Description :
 *  -------------
 *  This service initializes the serial driver and configures
 *  the applicable channels according to the configuration data, read
 *  from the 'syscon' parameters:
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'port',      IN,    port mapping (PORT_TTY0/PORT_TTY1)
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 ************************************************************************/
static 
INT32 SERIAL_SAA9730_init(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT32 *port )         /* IN: port mapping			  */
{
    UINT8		      baudrate, databits, parity, stopbits, flowctrl ;
    UINT8		      dll, dlm;
    void		      *saa9730base ;
    t_SERIAL_ctrl_descriptor  ctrl;
    bool		      ic_in_use;
    UINT32		      int_line;

    /* Get settings */
    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_BAUDRATE_ID :
		     SYSCON_COM_TTY1_BAUDRATE_ID,
		 (void *)&baudrate,
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_DATABITS_ID :
		     SYSCON_COM_TTY1_DATABITS_ID,
		 (void *)&databits,
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_PARITY_ID :
		     SYSCON_COM_TTY1_PARITY_ID,
		 (void *)&parity,
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_STOPBITS_ID :
		     SYSCON_COM_TTY1_STOPBITS_ID,
		 (void *)&stopbits,
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_FLOWCTRL_ID :
		     SYSCON_COM_TTY1_FLOWCTRL_ID,
		 (void *)&flowctrl,
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_IC_IN_USE_ID :
		     SYSCON_COM_TTY1_IC_IN_USE_ID,
		 (void *)&ic_in_use,
		 sizeof(bool) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_INTLINE_ID :
		     SYSCON_COM_TTY1_INTLINE_ID,
		 (void *)&int_line,
		 sizeof(UINT32) );

    SYSCON_read( SYSCON_BOARD_SAA9730_BASE_ID,
		 (void *)&saa9730base,
		 sizeof(void *) );

    saa9730base  = (void *)KSEG1(saa9730base);


    /* Verify settings */
    if (baudrate >= SERIAL_BAUDRATE_MAX ||
	serial_baudrate[baudrate] == SERIAL_ILLEGAL )
    {
	return( ERROR_SERIAL_INVALID_BAUDRATE ) ;
    }

    if (databits >= SERIAL_DATABITS_MAX ||
	serial_databits[databits] == SERIAL_ILLEGAL )
    {
	return( ERROR_SERIAL_INVALID_DATABITS ) ;
    }

    if (parity >= SERIAL_PARITY_MAX ||
	serial_parity[parity] == SERIAL_ILLEGAL )
    {
	return( ERROR_SERIAL_INVALID_PARITY ) ;
    }

    if (stopbits >= SERIAL_STOPBITS_MAX ||
	serial_stopbits[stopbits] == SERIAL_ILLEGAL )
    {
	return( ERROR_SERIAL_INVALID_STOPBITS ) ;
    }

    if (flowctrl >= SERIAL_FLOWCTRL_MAX ||
        flowctrl == SERIAL_FLOWCTRL_NOT_DEFINED )
    {
	return( ERROR_SERIAL_INVALID_FLOWCTRL ) ;
    }


    /* convert */
    dll      = serial_baudrate[baudrate] & 0xff ;
    dlm      = (serial_baudrate[baudrate] >> 8) & 0xff ;
    databits = serial_databits[databits] ;
    parity   = serial_parity[parity] ;
    stopbits = serial_stopbits[stopbits] ;
    
    evm_base = (UINT32*)(saa9730base + EVM_SAA9730_BASE_OFS);

    /* initialize physical register addresses */
    saa9730base = saa9730base + SERIAL_SAA9730_UART_OFS ;
    phy_rbr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_RBR_OFS );
    phy_thr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_THR_OFS );
    phy_ier = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_IER_OFS );
    phy_iir = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_IIR_OFS );
    phy_fcr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_FCR_OFS );
    phy_lcr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_LCR_OFS );
    phy_mcr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_MCR_OFS );
    phy_lsr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_LSR_OFS );
    phy_msr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_MSR_OFS );
    phy_scr = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_SCR_OFS );
    phy_dll = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_DLL_OFS );
    phy_dlm = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_DLM_OFS );
    phy_cfg = saa9730base + SWAP_BYTEADDR_EL(SERIAL_SAA9730_CFG_OFS );
    

    /* init IER: clear all interrupts */
    *phy_ier   = 0;
    shadow_ier = 0;
    *(evm_base + EVM_SAA9730_SECIERSW_OFS) &= ~EVM_SAA9730_SECIER_UART1;

    /* initalize poll buffer pointers */
    recv_getptr =
    recv_putptr = &recv_buffer[0] ;

    /* clear statistical info */
    uart_statistics.ua_tx_bytes   = 0 ;
    uart_statistics.ua_rx_bytes   = 0 ;
    uart_statistics.ua_rx_overrun = 0 ;
    uart_statistics.ua_rx_parity  = 0 ;
    uart_statistics.ua_rx_framing = 0 ;
    uart_statistics.ua_rx_break   = 0 ;
    uart_statistics.ua_rx_irqs    = 0 ;
    uart_statistics.ua_no_of_init++ ;


    /* init baudrate to 38.400 buad: 
	1) enable DL-registers
	2) set DLL + DLM divisor registers
	3) disable DL-registers                                */
    *phy_lcr =  SERIAL_LCR_DLAB ;
    *phy_dll =  dll ;
    *phy_dlm =  dlm ;
    *phy_lcr =  0 ;


    /* init LCR: 
        1) 8-bit data
        2) 1 stop bit
	3) no parity                                           */
    *phy_lcr = ( databits | 
                 stopbits | 
                 parity     ) ;

    /* init FCR: 
        1) enable FIFO
        2) reset receiver FIFO
        3) reset transmitter FIFO                              */
    *phy_fcr = (SERIAL_FCR_ENABLE     | 
                SERIAL_FCR_RCVR_RESET | 
                SERIAL_FCR_TXMT_RESET) ;

    /* init CFG: 
        1) set CTSEN (not RTSEN, as RTS is controlled in sw)   */ 
    *phy_cfg =
    shadow_flow = flowctrl == SERIAL_FLOWCTRL_HARDWARE ? SERIAL_CFG_CTSEN : 0;

    /* init MCR: 
        1) set DTR
        2) set RTS
        3) set OUT2  (PC style interrupt mask)                 */ 
    shadow_mcr |= SERIAL_MCR_RTS;
    *phy_mcr = shadow_mcr;

    /* reenable interrupt mask */
    *(evm_base + EVM_SAA9730_SECIERSW_OFS) = EVM_SAA9730_SECIER_UART1;
    *phy_ier = shadow_ier ;

    /* If this is the first call of init, we need to install interrupt handler */
    if( !registered )
    {
        if( ic_in_use )
        {
            EXCEP_register_ic_isr( int_line,
			           serial_int_handler,
			           NULL,
			           NULL );
        }
        else
        {
	    EXCEP_register_cpu_isr( int_line,
			            serial_int_handler,
			            NULL,
				    NULL );
        }

	registered = TRUE;
    }

    /* Configure driver to be interrupt driven */
    ctrl.sc_command = SERIAL_CTRL_RCV_IRQ_ON;
    SERIAL_SAA9730_ctrl( major, minor, &ctrl );

    return( OK ) ;
}


/************************************************************************
 *		serial_int_handler
 ************************************************************************/
static void
serial_int_handler(
    void *dummy )
{
    t_SERIAL_ctrl_descriptor  ctrl;

    ctrl.sc_command = SERIAL_CTRL_RCV_IRQ;

    SERIAL_SAA9730_ctrl( 0, 0, &ctrl );
}


/************************************************************************
 *
 *                          SERIAL_SAA9730_read
 *  Description :
 *  -------------
 *  This service polls the specified channel for any present character.
 *  If any character is present, it will be read into the user allocated
 *  variable; if none present, completion = 'ERROR_SERIAL_NO_CHARACTER'
 *  will be returned.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   OUT,   character read
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 character read into user variable
 *  'ERROR_SERIAL_NO_CHARACTER': no character present on channel
 *  'ERROR_SERIAL_COMM_ERROR':   communication error detected
 *  'ERROR_SERIAL_COMM_BREAK':   'BREAK' detected
 *
 *
 ************************************************************************/
static
INT32 SERIAL_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          UINT8  *p_param )      /* OUT: character been read            */
{
    UINT32 rc ;
    UINT32 lstat ;
    UINT16 *getptr ;

    /* check for device initialization */
    if (phy_lsr == NULL)
    {
        return( ERROR_SERIAL_UNKNOWN_DEVICE )  ;
    }

    SERIAL_SAA9730_irqpoll(); /* service UART */

    getptr= recv_getptr ;
    if (getptr == recv_putptr)
        return( ERROR_SERIAL_NO_CHARACTER ) ;

    if (poll_retcode != OK)
    {
        poll_retcode = OK ;
        /* flush receive buffer up to the last received ctrl/c or break */
        getptr = recv_flushptr;

        /* always read CTRL/C in case of 'break' or ctrl/c */
        lstat = (*getptr & 0xff00) | CTRL_C;
        
        rc = ERROR_SERIAL_COMM_BREAK;
    }
    else
    {
        lstat = *getptr;
        rc = OK;
    }
    *p_param = lstat ;
    uart_statistics.ua_rx_bytes++ ;

    /* check for errors */
    if (lstat & ((SERIAL_LSR_OE<<8) |
                 (SERIAL_LSR_PE<<8) |
                 (SERIAL_LSR_FE<<8)))
    {
        if (rc == OK) rc = ERROR_SERIAL_COMM_ERROR ;
        if (lstat & (SERIAL_LSR_OE<<8)) uart_statistics.ua_rx_overrun++ ;
        if (lstat & (SERIAL_LSR_PE<<8)) uart_statistics.ua_rx_parity++ ;
        if (lstat & (SERIAL_LSR_FE<<8)) uart_statistics.ua_rx_framing++ ;
    }

    if (++getptr >= &recv_buffer[POLLSIZE])
        getptr= &recv_buffer[0] ;
    recv_getptr = getptr ;

    /* compute room in buffer */
    if ( (shadow_mcr & SERIAL_MCR_RTS) == 0 &&
         (((UINT32)getptr - (UINT32)recv_putptr) &
                ((POLLSIZE - 1) * sizeof(*getptr)))
                               >= HW_LIMIT_START * sizeof(*getptr) )
    {
        shadow_mcr |= SERIAL_MCR_RTS;
        *phy_mcr = shadow_mcr;
    }

    return( rc ) ;
}


/************************************************************************
 *
 *                          SERIAL_SAA9730_irqpoll
 *  Description :
 *  -------------
 *  Disabling of interrupts around call of SERIAL_SAA9730_irq()
 *
 ************************************************************************/

static
UINT32 SERIAL_SAA9730_irqpoll( void )
{
    UINT32 lstat;

    if (sys_disable_int())
    {
        lstat = SERIAL_SAA9730_irq( 0 );
        sys_enable_int();
    }
    else
        lstat = SERIAL_SAA9730_irq( 0 );

    return lstat;
}

/************************************************************************
 *
 *                          SERIAL_SAA9730_irq
 *  Description :
 *  -------------
 *  This service empties the UART's FIFO buffer and puts the chars into
 *  the cyclic receive buffer.
 *
 *  This routine is called in both polled and interrupt mode.
 *    and requires interrupts to be disabled.
 *
 *  static variables written:
 *  poll_retcode  BREAK or CTRL/C has been read
 *  recv_putptr   pointer to next free position in cyclic queue.
 *
 *
 *  static variables read:
 *  phy_lsr       pointer to UART line status register
 *  phy_rbr       pointer to UART receive data register
 *  recv_getptr   pointer to first unread position in cyclic queue.
 *  recv_flushptr pointer to position of last read break or ctrl/c
 *
 ************************************************************************/
static
UINT32 SERIAL_SAA9730_irq( UINT32 in_intrpt )
{
    UINT32 lstat ; /* LSR_OE, LSR_PE, LSR_FE and LSR_BI are clear on read */
    UINT8  rdata ;
    UINT16 *putptr ;
    UINT32 room ;

    putptr= recv_putptr ;

    /* empty FIFO*/
    for (lstat = *phy_lsr; lstat & SERIAL_LSR_DR; lstat = *phy_lsr)
    {
        rdata = *phy_rbr & 0xff;
        uart_statistics.ua_rx_irqs += in_intrpt;

        if (lstat & SERIAL_LSR_BI)
        {
            rdata = CTRL_C;
            uart_statistics.ua_rx_break++;
        }

        /* compute room left in buffer, AFTER this byte has been put */
        room = ((UINT32)recv_getptr - (UINT32)putptr - 1) &
                             ((POLLSIZE - 1) * sizeof(*putptr));

        if (room <= HW_LIMIT_STOP * sizeof(*putptr) && shadow_flow != 0)
        {
            shadow_mcr &= ~SERIAL_MCR_RTS;
            *phy_mcr = shadow_mcr;
        }

        if (room == 0)
        {
            lstat |= SERIAL_LSR_OE;  /* overrun */
            /* overwrite previous char (overflow anyway) */
            if (--putptr < &recv_buffer[0])
                putptr= &recv_buffer[POLLSIZE-1] ;
        }

        *putptr = (lstat << 8) | rdata;

        if (rdata == CTRL_C)
        {
            poll_retcode = ERROR_SERIAL_COMM_BREAK ;
            recv_flushptr = putptr;
        }

        /* increase putptr to its future position */
        if (++putptr >= &recv_buffer[POLLSIZE])
            putptr= &recv_buffer[0] ;
    }

    recv_putptr = putptr ;
    return lstat;
}


/************************************************************************
 *
 *                          SERIAL_SAA9730_write
 *  Description :
 *  -------------
 *  This service writes a character on the specified channel
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    pointer to character to write
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 character has been written on channel
 *
 *
 ************************************************************************/
static
INT32 SERIAL_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          UINT8  *p_param )      /* IN: pointer to character to write   */
{
    UINT32 lstat ;

    /* check for device initialization */
    if (phy_lsr == NULL)
    {
      return( ERROR_SERIAL_UNKNOWN_DEVICE )  ;
    }


    /* check modem status: not necessary at auto-flow */
/*
    {
        UINT8 mstat ;
        do 
        {
            mstat = *phy_msr ;
        }
        while (!(mstat & SERIAL_MSR_CTS));
    }
*/

    /* check line status */
    do 
    {
        /* OBS: LSR_OE, LSR_PE, LSR_FE and LSR_BI are cleared on read */
        lstat = SERIAL_SAA9730_irqpoll() ;
    }
    while (!(lstat & SERIAL_LSR_THRE)) ;

    /* write character */
    *phy_thr = *p_param ;
    uart_statistics.ua_tx_bytes++ ;

    return( OK ) ;
}


/************************************************************************
 *
 *                          SERIAL_SAA9730_ctrl
 *  Description :
 *  -------------
 *  This service performs the actions defined by t_SERIAL_ctrl_command_ids
 *  on the specified channel.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, IOCTL structure
 *
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 normal return
 *  ERROR_SERIAL_COMM_BREAK:     break or ctrl/c received 
 *  ERROR_SERIAL_COMM_ERROR:     receive error, characters lost
 *
 *
 ************************************************************************/
static
INT32 SERIAL_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_SERIAL_ctrl_descriptor *p_param ) /* INOUT: IOCTL structure */
{
    UINT32 rcode ;

    /* check for device initialization */
    if (phy_lsr == NULL)
    {
      return( ERROR_SERIAL_UNKNOWN_DEVICE )  ;
    }

    if (p_param == NULL)
    {
      return( ERROR_SERIAL_INVALID_ARGUMENT ) ;
    }

    rcode = OK ;
    switch( p_param->sc_command )
    {
      case SERIAL_CTRL_POLL_BREAK:
        SERIAL_SAA9730_irqpoll(); /* service UART */
        if( (rcode = poll_retcode) != OK )
        {
            /* flush read buffer */
            rcode = SERIAL_SAA9730_read( major, minor, (UINT8 *)&rcode );
        }
      break ;

      case SERIAL_CTRL_GET_STATISTICS:
        memcpy((UINT8*)&p_param->sc_arg.sc_statistics, 
               (UINT8 *)&uart_statistics,
               sizeof(uart_statistics)) ;
      break ;

      case SERIAL_CTRL_GET_LSR:
        p_param->sc_arg.sc_linestat = SERIAL_SAA9730_irqpoll();
      break ;

      case SERIAL_CTRL_RCV_IRQ:
        /* acknowledge to Philips Event Monitor */
        *(evm_base + EVM_SAA9730_SECISR_OFS) = EVM_SAA9730_SECIER_UART1;
        SERIAL_SAA9730_irq( 1 );
      break ;

      case SERIAL_CTRL_RCV_IRQ_ON:
        *(evm_base + EVM_SAA9730_IER_OFS)    |= EVM_SAA9730_IER_MASTER;
        *(evm_base + EVM_SAA9730_IERSW_OFS)  |= EVM_SAA9730_IER_MASTER;
        *(evm_base + EVM_SAA9730_SECIER_OFS) |= EVM_SAA9730_SECIER_UART1;
        *phy_ier   = 
        shadow_ier = SERIAL_IER_RCVEN;
      break ;

      case SERIAL_CTRL_RCV_IRQ_OFF:
        *(evm_base + EVM_SAA9730_SECIER_OFS) &= ~EVM_SAA9730_SECIER_UART1;
        shadow_ier =
        *phy_ier   = 0;
      break ;

      case SERIAL_CTRL_FORCE_DTR:
         if (p_param->sc_arg.sc_dtr == 0)
             shadow_mcr &= ~SERIAL_MCR_DTR;
         else
             shadow_mcr |=  SERIAL_MCR_DTR;
         *phy_mcr = shadow_mcr;
      break ;

      case SERIAL_CTRL_GET_MSR:
         p_param->sc_arg.sc_msr = *phy_msr & ~(SERIAL_MSR_DSR | SERIAL_MSR_RI);
         if ((*(evm_base + EVM_SAA9730_IMR_OFS) & EVM_SAA9730_GPIO0) == 0)
             p_param->sc_arg.sc_msr |= SERIAL_MSR_RI;
         if ((*(evm_base + EVM_SAA9730_IMR_OFS) & EVM_SAA9730_GPIO1) == 0)
             p_param->sc_arg.sc_msr |= SERIAL_MSR_DSR;
      break ;

      case SERIAL_CTRL_SEND_BREAK:
         /* the normal LCR_BREAK bit does not function porperly */
         *phy_cfg |= SERIAL_CFG_BREAK;
         sys_wait_ms(250);
         *phy_cfg &= ~SERIAL_CFG_BREAK;
      break ;

      default:
        rcode = ERROR_SERIAL_INVALID_ARGUMENT ;
    }

    return( rcode ) ;
}
