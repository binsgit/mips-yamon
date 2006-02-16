
/************************************************************************
 *
 *      SERIAL_TI16550_api.h
 *
 *      The 'SERIAL_TI16550_api' module defines the specific public
 *      interface for the TI16550 serial device driver. The
 *      generic serial device driver interface is defined in the
 *      'serial_api' module, which is supported by this driver.
 *
 *      The specific public interface comprise:
 *
 *        1) SERIAL_TI16550_install: installs the 'serial' device driver
 *           services into the 'io' system.
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

#ifndef SERIAL_TI16550_API_H
#define SERIAL_TI16550_API_H

/************************************************************************
 *    Include files
 ************************************************************************/

#include <sysdefs.h>


/************************************************************************
 *  Definitions
*************************************************************************/

/* Offset addresses for the TI16550 UART mapped into 8-bit access    */
#define SERIAL_TI16550_RBR_OFS   0    /* Receiver Buffer             */
#define SERIAL_TI16550_THR_OFS   0    /* Transmitter Holding         */
#define SERIAL_TI16550_IER_OFS   1    /* Interrupt enable            */
#define SERIAL_TI16550_IIR_OFS   2    /* Interrupt identification    */
#define SERIAL_TI16550_FCR_OFS   2    /* FIFO control                */
#define SERIAL_TI16550_LCR_OFS   3    /* Line control                */
#define SERIAL_TI16550_MCR_OFS   4    /* Modem control               */
#define SERIAL_TI16550_LSR_OFS   5    /* Line status                 */
#define SERIAL_TI16550_MSR_OFS   6    /* Modem status                */
#define SERIAL_TI16550_SCR_OFS   7    /* Scratch                     */
#define SERIAL_TI16550_DLL_OFS   0    /* Divisor latch LSB           */
#define SERIAL_TI16550_DLM_OFS   1    /* Divisor latch MSB           */

/* IER mask */
#define SERIAL_IER_RCVEN         0x01    /* receive interrupt enable    */

/* FCR control */
#define SERIAL_FCR_ENABLE        0x01    /* enable FIFO's               */
#define SERIAL_FCR_RCVR_RESET    0x02    /* reset receiver FIFO         */
#define SERIAL_FCR_TXMT_RESET    0x04    /* reset transmit FIFO         */
#define SERIAL_FCR_RCVFIFO_1     0x00    /* receive FIFO threshold 1    */
#define SERIAL_FCR_RCVFIFO_4     0x40    /* receive FIFO threshold 4    */
#define SERIAL_FCR_RCVFIFO_8     0x80    /* receive FIFO threshold 8    */
#define SERIAL_FCR_RCVFIFO_14    0xc0    /* receive FIFO threshold 14   */

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
#define SERIAL_MCR_OUT2          0x08    /* General purpose output      */

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


/************************************************************************
 *   Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          SERIAL_TI16550_install
 *  Description :
 *  -------------
 *
 *  Installs the serial TI16550 device drivers services in 
 *  the IO system at the reserved device slot, found in the
 *  'sys_dev.h' file, which defines all major device numbers.
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
INT32 SERIAL_TI16550_install( void );



#endif /* #ifndef SERIAL_TI16550_API_H */
