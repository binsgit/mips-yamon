
/************************************************************************
 *
 *      SERIAL_TI16550.c
 *
 *      The 'SERIAL_TI16550' module implements the SERIAL-TI16550C
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
 *      Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <io_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <serial_api.h>
#include <serial_ti16550_api.h>
#include <excep_api.h>
#include <string.h>



/************************************************************************
 *  Definitions
*************************************************************************/

#define SET_ADDR( offset ) \
	sys_reg_addr( bigend, spacing, ti16550base, offset )


/* Register access */

/* #define PHY_RBR_W(minor, data)  SETREG(minor, data, rbr) */
#define PHY_THR_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_thr[minor]) = (data);	\
				else *(phy_thr[minor]) = (data);
#define PHY_IER_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_ier[minor]) = (data);	\
				else *(phy_ier[minor]) = (data);
#define PHY_FCR_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_fcr[minor]) = (data);	\
				else *(phy_fcr[minor]) = (data);
#define PHY_LCR_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_lcr[minor]) = (data);	\
				else *(phy_lcr[minor]) = (data);
#define PHY_MCR_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_mcr[minor]) = (data);	\
				else *(phy_mcr[minor]) = (data);
#define PHY_DLL_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_dll[minor]) = (data);	\
				else *(phy_dll[minor]) = (data);
#define PHY_DLM_W(minor, data)  if (size[minor] == sizeof(UINT8))	\
				  REG8(phy_dlm[minor]) = (data);	\
				else *(phy_dlm[minor]) = (data);


#define PHY_RBR_R(minor)	( size[minor] == sizeof(UINT8) ? \
	                        REG8(phy_rbr[minor]) :		 \
	                           *(phy_rbr[minor]) )
#define PHY_LSR_R(minor)	( size[minor] == sizeof(UINT8) ? \
	                        REG8(phy_lsr[minor]) :		 \
	                           *(phy_lsr[minor]) )
#define PHY_MSR_R(minor)	( size[minor] == sizeof(UINT8) ? \
	                        REG8(phy_msr[minor]) :		 \
	                           *(phy_msr[minor]) )

/************************************************************************
 *      Public variables
 ************************************************************************/


/************************************************************************
 *      Static variables
 ************************************************************************/

/* Is UART Big endian device or not */
static bool  bigend;

/* Base address of controller */
static void  *ti16550base;

/* Spacing of register */
static UINT8 spacing;

/* Size of registers (number of bytes) */
static UINT8 size[SERIAL_MAX_16550_DEVICES];

static volatile UINT32 *phy_rbr[SERIAL_MAX_16550_DEVICES];  /* '0', RO, receive buffer,       */
static volatile UINT32 *phy_thr[SERIAL_MAX_16550_DEVICES];  /* '0', WO, trans. holding,       */
static volatile UINT32 *phy_ier[SERIAL_MAX_16550_DEVICES];  /* '1', RW, int. enable,          */
static volatile UINT32 *phy_iir[SERIAL_MAX_16550_DEVICES];  /* '2', RO, int. ident,           */
static volatile UINT32 *phy_fcr[SERIAL_MAX_16550_DEVICES];  /* '2', WO, FIFO control,         */
static volatile UINT32 *phy_lcr[SERIAL_MAX_16550_DEVICES];  /* '3', RW, line control,         */
static volatile UINT32 *phy_mcr[SERIAL_MAX_16550_DEVICES];  /* '4', RW, modem control,        */
static volatile UINT32 *phy_lsr[SERIAL_MAX_16550_DEVICES];  /* '5', RW, line status,          */
static volatile UINT32 *phy_msr[SERIAL_MAX_16550_DEVICES];  /* '6', RW, modem status,         */
static volatile UINT32 *phy_scr[SERIAL_MAX_16550_DEVICES];  /* '7', RW, scratch               */
static volatile UINT32 *phy_dll[SERIAL_MAX_16550_DEVICES];  /* '0', RW, divisor latch LSB     */
static volatile UINT32 *phy_dlm[SERIAL_MAX_16550_DEVICES];  /* '1', RW, divisor latch MSB     */


/*  Databit conversion table.
 *  EEPROM-encoding to device control
 */
static const UINT8 serial_databits[SERIAL_DATABITS_MAX] = {

    SERIAL_ILLEGAL,   /* 0, undefined  */
    SERIAL_LCR_DATA7, /* 1, 7 databits */
    SERIAL_LCR_DATA8  /* 2, 8 databits */
};


/*  Parity conversion table.
 *  EEPROM-encoding to device control
 */
static const UINT8 serial_parity[SERIAL_PARITY_MAX] = {

    SERIAL_ILLEGAL,        /* 0, undefined  */
    SERIAL_LCR_PARITYNONE, /* 1, none       */
    SERIAL_LCR_PARITYODD,  /* 2, odd        */
    SERIAL_LCR_PARITYEVEN  /* 3, even       */
};


/*  Stopbits conversion table.
 *  EEPROM-encoding to device control
 */
static const UINT8 serial_stopbits[SERIAL_STOPBITS_MAX] = {

    SERIAL_ILLEGAL,     /* 0, undefined   */
    SERIAL_LCR_STOP1,   /* 1, 1 stopbit   */
    SERIAL_LCR_STOP15,  /* 2, 1.5 stopbit */
    SERIAL_LCR_STOP2    /* 3, 2 stopbit   */

};

static t_UART_statistics  uart_statistics[SERIAL_MAX_16550_DEVICES];
static UINT32		  poll_retcode[SERIAL_MAX_16550_DEVICES];
static UINT32		  shadow_ier[SERIAL_MAX_16550_DEVICES];
static UINT32		  shadow_mcr[SERIAL_MAX_16550_DEVICES];
static UINT32		  shadow_flow[SERIAL_MAX_16550_DEVICES];


/*  Receive buffer operation:
 *
 *  putptr points to the next free location
 *  When a byte is polled from the uart, it is stored by putptr,
 *  which is then cyclic incremented UNLESS it gets equal to getptr.
 *
 *  That way, getptr == putptr means buffer empty, and
 *            the buffer can hold POLLSIZE-1 bytes.
 */ 

#define POLLSIZE	0x800   /* Must be power of 2 and at least 128 */
#define HW_LIMIT_STOP	(POLLSIZE-64)  /* RTS OFF when 64 chars in buf */
#define HW_LIMIT_START	(POLLSIZE-32)  /* RTS  ON when 32 chars in buf */

static UINT16 *recv_putptr[SERIAL_MAX_16550_DEVICES];
static UINT16 *recv_getptr[SERIAL_MAX_16550_DEVICES];
static UINT16 *recv_flushptr[SERIAL_MAX_16550_DEVICES];
static UINT16  recv_buffer[SERIAL_MAX_16550_DEVICES][POLLSIZE];

static UINT32 minor_numbers[SERIAL_MAX_16550_DEVICES];

/* Available baudrates */
static t_SERIAL_baudrate_div *serial_baudrate;

/*  Boolean indicating whether interrupt handlers are registered or not.
 *  Initial value is 0 (FALSE) since data is in BSS.
 */
static bool  registered[SERIAL_MAX_16550_DEVICES];

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static INT32 
SERIAL_TI16550_init(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT32 *port );        /* IN: port mapping			  */

static INT32 
SERIAL_TI16550_read(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT8  *p_param );     /* OUT: character been read            */

static UINT32 
SERIAL_TI16550_irqpoll( 
    UINT32 minor );

static UINT32 
SERIAL_TI16550_irq( 
    UINT32 minor,
    UINT32 in_intrpt );

static INT32 
SERIAL_TI16550_write(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT8  *p_param );     /* IN: character to write              */

static INT32 
SERIAL_TI16550_ctrl(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    t_SERIAL_ctrl_descriptor *p_param );/* INOUT: IOCTL structure */

static void
serial_int_handler(
    void *data );	/* Holds the minor device number		*/


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          SERIAL_TI16550_install
 *  Description :
 *  -------------
 *
 *  Installs the serial TI16550 serial device drivers services in 
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
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 
SERIAL_TI16550_install( void )
{    
    UINT32 i;
    UINT32 tty;
    UINT32 major, minor;


    /* Initialise variables */
    for( i=0; i<SERIAL_MAX_16550_DEVICES; i++ )
    {
        minor_numbers[i] = i;
        shadow_mcr[i] = SERIAL_MCR_DTR | SERIAL_MCR_OUT2;
    }

    /* pre-initialize local variables and install device services */
    IO_install( SYS_MAJOR_SERIAL_TI16550,         /* major device number */
         (t_io_service) SERIAL_TI16550_init,      /* 'init'  service     */
                        NULL,                     /* 'open'  service  na */
                        NULL,                     /* 'close' service  na */
         (t_io_service) SERIAL_TI16550_read,      /* 'read'  service     */
         (t_io_service) SERIAL_TI16550_write,     /* 'write' service     */
         (t_io_service) SERIAL_TI16550_ctrl );    /* 'ctrl' service      */
    /* call our own 'init' service */

    /* TTY0 */
    tty = PORT_TTY0;
    SYSCON_read( SYSCON_COM_TTY0_MAJOR, (void *)(&major), sizeof(UINT32) );
    
    if (major == SYS_MAJOR_SERIAL_TI16550)
    {
        SYSCON_read(SYSCON_COM_TTY0_MINOR, (void *)(&minor), sizeof(UINT32));
	IO_init( SYS_MAJOR_SERIAL_TI16550, minor, &tty );
    }


    /* TTY1 */
    tty = PORT_TTY1;
    SYSCON_read( SYSCON_COM_TTY1_MAJOR, (void *)(&major), sizeof(UINT32) );
    if (major == SYS_MAJOR_SERIAL_TI16550)
    {
	SYSCON_read(SYSCON_COM_TTY1_MINOR, (void *)(&minor), sizeof(UINT32));
	IO_init( SYS_MAJOR_SERIAL_TI16550, minor, &tty );
    }
    return OK;
}



/************************************************************************
 *      Implementation : Device driver services
 ************************************************************************/


/************************************************************************
 *
 *                          SERIAL_TI16550_init
 *  Description :
 *  -------------
 *  This service initializes the serial driver and configures
 *  the applicable channels according to the configuration data, read
 *  from SYSCON
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'port',      IN,	port mapping (PORT_TTY0/PORT_TTY1)
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 ************************************************************************/
static
INT32 SERIAL_TI16550_init(
    UINT32 major,          /* IN: major device number			*/
    UINT32 minor,          /* IN: minor device number			*/
    UINT32 *port )         /* IN: port mapping				*/
{
    INT32		      rcode;
    UINT32		      brkresval;
    UINT8		      baudrate, databits, parity, stopbits, flowctrl;
    UINT8		      dll, dlm;
    t_SERIAL_ctrl_descriptor  ctrl;
    void		      *minor_data;
    bool		      ic_in_use;
    UINT32		      int_line;
    bool		      adjust_break;

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
		     SYSCON_COM_TTY0_REG_SIZE_ID :
		     SYSCON_COM_TTY1_REG_SIZE_ID,
		 (void *)&size[minor],
		 sizeof(UINT8) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_REG_SPACING_ID :
		     SYSCON_COM_TTY1_REG_SPACING_ID,
		 (void *)&spacing,
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

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_ADJUST_BREAK_ID :
		     SYSCON_COM_TTY1_ADJUST_BREAK_ID,
		 (void *)&adjust_break,
		 sizeof(bool) );

    SYSCON_read( (*port == PORT_TTY0) ? 
		     SYSCON_COM_TTY0_BIGEND_ID :
		     SYSCON_COM_TTY1_BIGEND_ID,
		 (void *)&bigend,
		 sizeof(bool) );

    SYSCON_read( (minor == SERIAL_MINOR_TI16550_UART0) ?
		     SYSCON_BOARD_TI16C550C_BASE0_ID :
		     SYSCON_BOARD_TI16C550C_BASE1_ID,
		 (void *)&ti16550base,
		 sizeof(void *) );

    SYSCON_read( (minor == SERIAL_MINOR_TI16550_UART0) ?
		     SYSCON_COM_TTY0_BAUDRATE_DIV_ID :
		     SYSCON_COM_TTY1_BAUDRATE_DIV_ID,
		 (void *)&serial_baudrate,
		 sizeof(t_SERIAL_baudrate_div *) );

    ti16550base  = (void *)KSEG1(ti16550base);


    /* Verify settings */
    if (baudrate >= SERIAL_BAUDRATE_MAX ||
        (*serial_baudrate)[baudrate] == SERIAL_ILLEGAL )
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
    dll      = (*serial_baudrate)[baudrate] & 0xff;
    dlm      = ((*serial_baudrate)[baudrate]>>8) & 0xff;
    databits = serial_databits[databits];
    parity   = serial_parity[parity];
    stopbits = serial_stopbits[stopbits];

    /* initialize physical register addresses based on size and spacing */
    phy_rbr[minor] = SET_ADDR( SERIAL_TI16550_RBR_OFS );
    phy_thr[minor] = SET_ADDR( SERIAL_TI16550_THR_OFS );
    phy_ier[minor] = SET_ADDR( SERIAL_TI16550_IER_OFS );
    phy_iir[minor] = SET_ADDR( SERIAL_TI16550_IIR_OFS );
    phy_fcr[minor] = SET_ADDR( SERIAL_TI16550_FCR_OFS );
    phy_lcr[minor] = SET_ADDR( SERIAL_TI16550_LCR_OFS );
    phy_mcr[minor] = SET_ADDR( SERIAL_TI16550_MCR_OFS );
    phy_lsr[minor] = SET_ADDR( SERIAL_TI16550_LSR_OFS );
    phy_msr[minor] = SET_ADDR( SERIAL_TI16550_MSR_OFS );
    phy_scr[minor] = SET_ADDR( SERIAL_TI16550_SCR_OFS );
    phy_dll[minor] = SET_ADDR( SERIAL_TI16550_DLL_OFS );
    phy_dlm[minor] = SET_ADDR( SERIAL_TI16550_DLM_OFS );

    /* init IER: clear all interrupts */
    PHY_IER_W(minor, 0);
    shadow_ier[minor] = 0;

    /* initalize poll buffer pointers */
    recv_getptr[minor] = recv_putptr[minor] = &recv_buffer[minor][0];

    /* clear statistical info */
    uart_statistics[minor].ua_tx_bytes   = 0;
    uart_statistics[minor].ua_rx_bytes   = 0;
    uart_statistics[minor].ua_rx_overrun = 0;
    uart_statistics[minor].ua_rx_parity  = 0;
    uart_statistics[minor].ua_rx_framing = 0;
    uart_statistics[minor].ua_rx_break   = 0;
    uart_statistics[minor].ua_rx_irqs    = 0;
    uart_statistics[minor].ua_no_of_init++;

    /*  Init baudrate : 
     *	    1) enable DL-registers
     *	    2) set DLL + DLM divisor registers
     *	    3) disable DL-registers                                
     */
    PHY_LCR_W(minor, SERIAL_LCR_DLAB);
    PHY_DLL_W(minor, dll);
    PHY_DLM_W(minor, dlm);
    PHY_LCR_W(minor, 0);

    /*  Init LCR: 
     *      1) 8-bit data
     *	    2) 1 stop bit
     *	    3) no parity                                           
     */
    PHY_LCR_W(minor, databits | stopbits | parity);

    /*  Init FCR: 
     *      1) enable FIFO
     *	    2) reset receiver FIFO
     *	    3) reset transmitter FIFO
     *	    4) set receive FIFO threshold to 4                     
     */
    PHY_FCR_W(minor, SERIAL_FCR_ENABLE     | 
		     SERIAL_FCR_RCVR_RESET | 
		     SERIAL_FCR_TXMT_RESET |
		     SERIAL_FCR_RCVFIFO_4);

    /*  Init MCR: 
     *      1) set DTR
     *	    2) set RTS
     *	    3) enable auto-flow control (both RX and TX)
     *           (Texas only;                SuperIO and Philips: No OPeration)
     *	    4) enable interrupt mask (OUT2)
     *           (SuperIO and Philips only;  Texas: NC on Atlas,SEAD,Malta)
     */
    shadow_flow[minor] = flowctrl == SERIAL_FLOWCTRL_HARDWARE ? SERIAL_MCR_AFE : 0;
    shadow_mcr[minor] = shadow_flow[minor] |
                        SERIAL_MCR_DTR |
                        SERIAL_MCR_RTS |
                        SERIAL_MCR_OUT2;
    PHY_MCR_W(minor, shadow_mcr[minor]);


    if( adjust_break )
    {
        /* limit BRKRES register to prevent low baudrates from forcing reset */
	rcode = SYSCON_read( SYSCON_BOARD_BRKRES_ID, &brkresval, sizeof(brkresval));

        if(rcode == OK && brkresval && brkresval < (*serial_baudrate)[baudrate]>>4)
        {
            brkresval = (*serial_baudrate)[baudrate] >> 4;
	  
	    rcode     = SYSCON_write( SYSCON_BOARD_BRKRES_ID,
				      &brkresval,
				      sizeof(brkresval));
        }
    }

    /* If this is the first call of init, we need to install interrupt handler */
    if( !registered[minor] )
    {    
        /* Determine parameter for interrupt service routine */
        minor_data = (void *)&minor_numbers[minor];

        /* Register interrupt handler */
        if( ic_in_use )
        {
            EXCEP_register_ic_isr(  int_line,
			            serial_int_handler,
			            minor_data,
				    NULL );
        }
        else
        {
	    EXCEP_register_cpu_isr( int_line,
			            serial_int_handler,
			            minor_data,
				    NULL );
        }

        registered[minor] = TRUE;
    }

    /* Configure driver to be interrupt driven */
    ctrl.sc_command = SERIAL_CTRL_RCV_IRQ_ON;
    SERIAL_TI16550_ctrl( major, minor, &ctrl );

    return( OK );
}


/************************************************************************
 *		serial_int_handler
 ************************************************************************/
static void
serial_int_handler(
    void *data )	/* Holds the minor device number		*/
{
    t_SERIAL_ctrl_descriptor  ctrl;

    ctrl.sc_command = SERIAL_CTRL_RCV_IRQ;

    SERIAL_TI16550_ctrl( 0, *(UINT32 *)data, &ctrl );
}


/************************************************************************
 *
 *                          SERIAL_TI16550_read
 *  Description :
 *  -------------
 *  This service polls the specified channel for any present character.
 *  If any character is present, it will be read into the user allocated
 *  variable; if none present, completion = 'ERROR_SERIAL_NO_CHARACTER'
 *  will be returned.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   OUT,   character read
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 character read into user variable
 *  'ERROR_SERIAL_NO_CHARACTER': no character present on channel
 *  'ERROR_SERIAL_COMM_ERROR':   communication error detected
 *  'ERROR_SERIAL_COMM_BREAK':   'BREAK' detected
 *
 ************************************************************************/
static INT32 
SERIAL_TI16550_read(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT8  *p_param )      /* OUT: character been read            */
{
    UINT32 rc;
    UINT32 lstat;
    UINT16 *getptr;

    /* check minor device number */
    if(minor >= SERIAL_MAX_16550_DEVICES)
    {
        return(ERROR_SERIAL_UNKNOWN_DEVICE);
    }

    /* check for device initialization */
    if( phy_lsr[minor] == NULL )
    {
        return( ERROR_SERIAL_UNKNOWN_DEVICE );
    }

    SERIAL_TI16550_irqpoll( minor ); /* service UART */

    getptr= recv_getptr[minor];
    if (getptr == recv_putptr[minor])
        return( ERROR_SERIAL_NO_CHARACTER );

    if (poll_retcode[minor] != OK)
    {
        poll_retcode[minor] = OK;
        /* flush receive buffer up to the last received ctrl/c or break */
        getptr = recv_flushptr[minor];

        /* always read CTRL/C in case of 'break' or ctrl/c */
        lstat = (*getptr & 0xff00) | CTRL_C;
        
        rc = ERROR_SERIAL_COMM_BREAK;
    }
    else
    {
        lstat = *getptr;
        rc = OK;
    }

    *p_param = lstat;
    uart_statistics[minor].ua_rx_bytes++;

    /* check for errors */
    if (lstat & ((SERIAL_LSR_OE<<8) |
                 (SERIAL_LSR_PE<<8) |
                 (SERIAL_LSR_FE<<8)))
    {
        if (rc == OK) rc = ERROR_SERIAL_COMM_ERROR ;
        if (lstat & (SERIAL_LSR_OE<<8)) uart_statistics[minor].ua_rx_overrun++;
        if (lstat & (SERIAL_LSR_PE<<8)) uart_statistics[minor].ua_rx_parity++;
        if (lstat & (SERIAL_LSR_FE<<8)) uart_statistics[minor].ua_rx_framing++;
    }

    if (++getptr >= &recv_buffer[minor][POLLSIZE])
        getptr= &recv_buffer[minor][0];
    recv_getptr[minor] = getptr;

    /* compute room in buffer */
    if ( (shadow_mcr[minor] & SERIAL_MCR_RTS) == 0 &&
        (((UINT32)getptr - (UINT32)recv_putptr[minor]) &
                ((POLLSIZE - 1) * sizeof(*getptr)))
                               >= HW_LIMIT_START * sizeof(*getptr) )
    {
        shadow_mcr[minor] |= SERIAL_MCR_RTS;
        PHY_MCR_W(minor, shadow_mcr[minor]);
    }

    return( rc );
}


/************************************************************************
 *
 *                          SERIAL_TI16550_irqpoll
 *  Description :
 *  -------------
 *  Disabling of interrupts around call of SERIAL_TI16550_irq()
 *
 ************************************************************************/
static UINT32 
SERIAL_TI16550_irqpoll( 
    UINT32 minor )
{
    UINT32 lstat;

    if(sys_disable_int())
    {
        lstat = SERIAL_TI16550_irq( minor, 0 );
        sys_enable_int();
    }
    else
        lstat = SERIAL_TI16550_irq( minor, 0 );

    return lstat;
}


/************************************************************************
 *
 *                          SERIAL_TI16550_irq
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
 *  recv_putptr2  pointer to next free position in cyclic queue.
 *
 *  static variables read:
 *  phy_lsr       pointer to UART line status register
 *  phy_rbr       pointer to UART receive data register
 *  recv_getptr   pointer to first unread position in cyclic queue.
 *  recv_flushptr pointer to position of last read break or ctrl/c
 *
 ************************************************************************/
static UINT32 
SERIAL_TI16550_irq( 
    UINT32 minor,    
    UINT32 in_intrpt )
{
    UINT32 lstat; /* LSR_OE, LSR_PE, LSR_FE and LSR_BI are clear on read */
    UINT8  rdata;
    UINT16 *putptr;
    UINT32 room;

    putptr = recv_putptr[minor];

    /* empty FIFO */
    for(lstat = PHY_LSR_R(minor); lstat & SERIAL_LSR_DR; lstat = PHY_LSR_R(minor))
    {
        rdata = PHY_RBR_R(minor) & 0xff;
        uart_statistics[minor].ua_rx_irqs += in_intrpt;

        if(lstat & SERIAL_LSR_BI)
        {
            rdata = CTRL_C;
            uart_statistics[minor].ua_rx_break++;
        }

        /* compute room left in buffer, AFTER this byte has been put */
        room = ((UINT32)recv_getptr[minor] - (UINT32)putptr - 1) &
                             ((POLLSIZE - 1) * sizeof(*putptr));

        if (room <= HW_LIMIT_STOP * sizeof(*putptr) && shadow_flow[minor] != 0)
        {
            shadow_mcr[minor] &= ~SERIAL_MCR_RTS;
            PHY_MCR_W(minor, shadow_mcr[minor]);
        }

        if(room == 0)
        {
            lstat |= SERIAL_LSR_OE;  /* overrun */
            /* overwrite previous char (overflow anyway) */
            if (--putptr < &recv_buffer[minor][0])
                putptr= &recv_buffer[minor][POLLSIZE-1];
        }

        *putptr = (lstat << 8) | rdata;

        if(rdata == CTRL_C)
        {
            poll_retcode[minor]  = ERROR_SERIAL_COMM_BREAK;
            recv_flushptr[minor] = putptr;
        }

        /* increase putptr to its future position */
        if( ++putptr >= &recv_buffer[minor][POLLSIZE] )
            putptr= &recv_buffer[minor][0];
    }

    recv_putptr[minor] = putptr;
    return lstat;
}


/************************************************************************
 *
 *                          SERIAL_TI16550_write
 *  Description :
 *  -------------
 *  This service writes a character on the specified channel
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    pointer to character to write
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 character has been written on channel
 *
 ************************************************************************/
static INT32 
SERIAL_TI16550_write(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    UINT8  *p_param )      /* IN: pointer to character to write   */
{

    /* check minor device number */
    if(minor >= SERIAL_MAX_16550_DEVICES)
    {
        return(ERROR_SERIAL_UNKNOWN_DEVICE);
    }

    /* check for device initialization */
    if( phy_lsr[minor] == NULL )
    {
        return( ERROR_SERIAL_UNKNOWN_DEVICE );
    }

    /* check line status and modem status */
    for (;;)
    {
        /* OBS: LSR_OE, LSR_PE, LSR_FE and LSR_BI are cleared on read */
        if ( (SERIAL_TI16550_irqpoll( minor ) & SERIAL_LSR_THRE) &&
             ((PHY_MSR_R(minor) & SERIAL_MSR_CTS) || shadow_flow[minor] == 0)) break;
    }

    /* write character */
    PHY_THR_W(minor, *p_param);
    uart_statistics[minor].ua_tx_bytes++;

    return( OK );
}


/************************************************************************
 *
 *                          SERIAL_TI16550_ctrl
 *  Description :
 *  -------------
 *  This service performs the actions defined by t_SERIAL_ctrl_command_ids
 *  on the specified channel.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, IOCTL structure
 *
 *  Return values :
 *  ---------------
 *  'OK' = 0x00:                 normal return
 *  ERROR_SERIAL_COMM_BREAK:     break or ctrl/c received 
 *  ERROR_SERIAL_COMM_ERROR:     receive error, characters lost
 *
 ************************************************************************/
static INT32 
SERIAL_TI16550_ctrl(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    t_SERIAL_ctrl_descriptor *p_param ) /* INOUT: IOCTL structure */
{
    UINT32 rcode;

    /* check minor device number */
    if(minor >= SERIAL_MAX_16550_DEVICES)
    {
        return(ERROR_SERIAL_UNKNOWN_DEVICE);
    }

    /* check for device initialization */
    if( phy_lsr[minor] == NULL )
    {
        return( ERROR_SERIAL_UNKNOWN_DEVICE );
    }

    if(p_param == NULL)
    {
        return( ERROR_SERIAL_INVALID_ARGUMENT );
    }

    rcode = OK;
    switch( p_param->sc_command )
    {
      case SERIAL_CTRL_POLL_BREAK:
        SERIAL_TI16550_irqpoll(minor); /* service UART */
        if( (rcode = poll_retcode[minor]) != OK )
        {
            /* flush read buffer */
            rcode = SERIAL_TI16550_read( major, minor, (UINT8 *)&rcode );
        }
        break;

      case SERIAL_CTRL_GET_STATISTICS:
        memcpy((UINT8*)&p_param->sc_arg.sc_statistics, 
               (UINT8 *)&uart_statistics[minor],
               sizeof(uart_statistics[minor]));
        break;

      case SERIAL_CTRL_GET_LSR:
        p_param->sc_arg.sc_linestat = SERIAL_TI16550_irqpoll( minor );
        break;

      case SERIAL_CTRL_RCV_IRQ:
        SERIAL_TI16550_irq( minor, 1 );
        break ;

      case SERIAL_CTRL_RCV_IRQ_ON:
        PHY_IER_W(minor,    SERIAL_IER_RCVEN);
	shadow_ier[minor] = SERIAL_IER_RCVEN;
        break ;

      case SERIAL_CTRL_RCV_IRQ_OFF:
        shadow_ier[minor] = 0;
	PHY_IER_W(minor, 0);
        break;

      case SERIAL_CTRL_FORCE_DTR:
        if (p_param->sc_arg.sc_dtr == 0)
            shadow_mcr[minor] &= ~SERIAL_MCR_DTR;
        else
            shadow_mcr[minor] |=  SERIAL_MCR_DTR;
        PHY_MCR_W(minor, shadow_mcr[minor]);
        break;

      case SERIAL_CTRL_GET_MSR:
        p_param->sc_arg.sc_msr = PHY_MSR_R(minor);
        break;

      case SERIAL_CTRL_SEND_BREAK:
        if (size[minor] == sizeof(UINT8))
	{
            *(volatile UINT8*)(phy_lcr[minor]) |= SERIAL_LCR_BREAK;
            sys_wait_ms(250);
            *(volatile UINT8*)(phy_lcr[minor]) &= ~SERIAL_LCR_BREAK;
	}
	else
	{
            *(phy_lcr[minor]) |= SERIAL_LCR_BREAK;
            sys_wait_ms(250);
            *(phy_lcr[minor]) &= ~SERIAL_LCR_BREAK;
	}
        break;

      default:
        rcode = ERROR_SERIAL_INVALID_ARGUMENT;
    }

    return( rcode );
}



