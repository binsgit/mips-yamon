
/************************************************************************
 *
 *      SERIAL_api.h
 *
 *      The 'serial_api' module defines the serial device driver
 *      interface to be used via 'IO' device driver services:
 *
 *        1) init  serial device:  configure and initialize serial driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  read character
 *        5) write serial device:  write character
 *        6) ctrl  serial device:  write character string
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

#ifndef SERIAL_API_H
#define SERIAL_API_H

/************************************************************************
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>

/************************************************************************
 *   Serial device driver, ERROR completion codes
*************************************************************************/

#define ERROR_SERIAL              0x00003000 /* for compile time check    */
#define ERROR_SERIAL_NO_CHARACTER     0x3000 /* No character present      */
#define ERROR_SERIAL_COMM_ERROR       0x3001 /* Communication error       */
#define ERROR_SERIAL_COMM_BREAK       0x3002 /* 'BREAK' detected          */
#define ERROR_SERIAL_UNKNOWN_DEVICE   0x3003 /* Unknown device            */
#define ERROR_SERIAL_INVALID_BAUDRATE 0x3004 /* Invalid baudrate selected */
#define ERROR_SERIAL_INVALID_DATABITS 0x3005 /* Invalid databits selected */
#define ERROR_SERIAL_INVALID_PARITY   0x3006 /* Invalid parity selected   */
#define ERROR_SERIAL_INVALID_STOPBITS 0x3007 /* Invalid stopbits selected */
#define ERROR_SERIAL_INVALID_ARGUMENT 0x3008 /* Invalid _ctrl argument    */
#define ERROR_SERIAL_INVALID_FLOWCTRL 0x3009 /* Invalid flowctrl selected */


/************************************************************************
 *  Serial device driver, configuration enumerations
*************************************************************************/

/* Serial device driver, baudrate configuration definitions */
typedef enum SERIAL_baudrate_ids
{
    SERIAL_BAUDRATE_NOT_DEFINED =  0,
    SERIAL_BAUDRATE_075_BPS     =  1,
    SERIAL_BAUDRATE_110_BPS     =  2,
    SERIAL_BAUDRATE_150_BPS     =  3,
    SERIAL_BAUDRATE_300_BPS     =  4,
    SERIAL_BAUDRATE_600_BPS     =  5,
    SERIAL_BAUDRATE_1200_BPS    =  6,
    SERIAL_BAUDRATE_1800_BPS    =  7,
    SERIAL_BAUDRATE_2400_BPS    =  8,
    SERIAL_BAUDRATE_4800_BPS    =  9,
    SERIAL_BAUDRATE_7200_BPS    = 10,
    SERIAL_BAUDRATE_9600_BPS    = 11,
    SERIAL_BAUDRATE_14400_BPS   = 12,
    SERIAL_BAUDRATE_19200_BPS   = 13,
    SERIAL_BAUDRATE_38400_BPS   = 14,
    SERIAL_BAUDRATE_57600_BPS   = 15,
    SERIAL_BAUDRATE_115200_BPS  = 16,
    SERIAL_BAUDRATE_230400_BPS  = 17,
    SERIAL_BAUDRATE_460800_BPS  = 18,
    SERIAL_BAUDRATE_921600_BPS  = 19,
    SERIAL_BAUDRATE_MAX         = 20
} 
t_SERIAL_baudrate_ids;

#define SERIAL_ILLEGAL           0xff    /* ILLEGAL CONTROL VALUE	*/

typedef UINT16 t_SERIAL_baudrate_div[SERIAL_BAUDRATE_MAX];


/* Serial device driver, data bits configuration definitions */
typedef enum SERIAL_databits_ids
{
    SERIAL_DATABITS_NOT_DEFINED =  0,
    SERIAL_DATABITS_7           =  1,
    SERIAL_DATABITS_8           =  2,
    SERIAL_DATABITS_MAX         =  3
} 
t_SERIAL_databits_ids;

/* Serial device driver, parity configuration definitions */
typedef enum SERIAL_parity_ids
{
    SERIAL_PARITY_NOT_DEFINED   =  0,
    SERIAL_PARITY_NONE          =  1,
    SERIAL_PARITY_ODD           =  2,
    SERIAL_PARITY_EVEN          =  3,
    SERIAL_PARITY_MAX           =  4
} 
t_SERIAL_parity_ids;

/* Serial device driver, stop bits configuration definitions */
typedef enum SERIAL_stopbits_ids
{
    SERIAL_STOPBITS_NOT_DEFINED =  0,
    SERIAL_STOPBITS_10          =  1,
    SERIAL_STOPBITS_15          =  2,
    SERIAL_STOPBITS_20          =  3,
    SERIAL_STOPBITS_MAX         =  4
} 
t_SERIAL_stopbits_ids;

/* Serial device driver, flow control configuration definitions */
typedef enum SERIAL_flowctrl_ids
{
    SERIAL_FLOWCTRL_NOT_DEFINED =  0,
    SERIAL_FLOWCTRL_NONE        =  1,
    SERIAL_FLOWCTRL_HARDWARE    =  2,
#if 0
    /* Not supported yet */
    SERIAL_FLOWCTRL_XON_XOFF    =  3,
#endif
    SERIAL_FLOWCTRL_MAX         =  3
} 
t_SERIAL_flowctrl_ids;


/************************************************************************
 *  Serial device driver, minor device numbers
*************************************************************************/

/* Maximum number of TI16550 devices supported:                         */
#define SERIAL_MAX_16550_DEVICES        2
#define SERIAL_MINOR_TI16550_UART0	0    /* TI16550 UART subdev 0  */
#define SERIAL_MINOR_TI16550_UART1	1    /* TI16550 UART subdev 1  */

/* Only one SAA9730 device */
#define SERIAL_MINOR_SAA9730_UART	0    /* SAA9730 UART            */


/************************************************************************
 *  SERIAL 'ctrl' service, command codes
*************************************************************************/

typedef enum SERIAL_ctrl_command_ids
{
    SERIAL_CTRL_POLL_BREAK = 0,      /* SERIAL poll for break or ctrl/c */
    SERIAL_CTRL_GET_STATISTICS,           /* SERIAL get uart statistics */
    SERIAL_CTRL_GET_LSR,        /* SERIAL get uart line status register */
    SERIAL_CTRL_RCV_IRQ,            /* SERIAL interrupt service routine */
    SERIAL_CTRL_RCV_IRQ_ON,          /* SERIAL set uart receive irq bit */
    SERIAL_CTRL_RCV_IRQ_OFF,       /* SERIAL clear uart receive irq bit */
    SERIAL_CTRL_FORCE_DTR,        /* SERIAL set uart dtr line on or off */
    SERIAL_CTRL_GET_MSR,       /* SERIAL get uart modem status register */
    SERIAL_CTRL_SEND_BREAK         /* SERIAL send BREAK on tx data line */
} 
t_SERIAL_ctrl_command_ids;


/************************************************************************
 *  Parameter definitions
*************************************************************************/


typedef struct UART_statistics {
  UINT32 ua_tx_bytes ;
  UINT32 ua_rx_bytes ;
  UINT32 ua_rx_overrun ;
  UINT32 ua_rx_parity ;
  UINT32 ua_rx_framing ;
  UINT32 ua_rx_break ;
  UINT32 ua_rx_irqs ;
  UINT32 ua_no_of_init ;
} 
t_UART_statistics ;


typedef struct SERIAL_ctrl_descriptor
{
    UINT32 sc_command ;    /* IN: 'ctrl' command (SERIAL_POLL_INTR,..)      */
    union {
       t_UART_statistics  sc_statistics;  /* OUT: statistics since last     */
                                          /*        call of SERIAL_init()   */
       UINT32             sc_linestat;    /* OUT: uart line status register */
       UINT32             sc_dtr;         /* IN:  uart dtr control          */
       UINT32             sc_msr;         /* OUT: uart modem status registr */
       UINT32             sc_fill[8];     /* IN/OUT for new ioctl commands  */
    } sc_arg;

} 
t_SERIAL_ctrl_descriptor ;


#endif /* #ifndef SERIAL_API_H */
