/************************************************************************
 *
 *      IIC_SAA9730.c
 *
 *      The 'IIC_SAA9730' module implements the IIC device driver
 *      interface to be used via 'IIC' device driver services:
 *
 *        1) init  serial device:  configure and initialize IIC driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  read IIC device
 *        5) write serial device:  write IIC device
 *        6) ctrl  serial device:  a) write+read IIC device
 *                                 b) test IIC device
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
#include <iic_api.h>
#include <iic_saa9730_api.h>

#include <string.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

#define IIC_TIMEOUT                 1000 /* IIC-bus command timeout ms. */

#define IIC_READCMD                 TRUE
#define IIC_WRITECMD                FALSE

/************************************************************************
 *  IIC port: Relative Register Addresses
*************************************************************************/

#define IIC_TFR_OFS       0x2400 /* IIC port: Transfer register         */
#define IIC_STC_OFS       0x2404 /* IIC port: Status and control reg.   */

/************************************************************************
 *  IIC port: Register field encodings
*************************************************************************/

/******** reg: TFR ********/

/* field: BYTE2 */
#define IIC_TFR_BYTE2_SHF        24
#define IIC_TFR_BYTE2_MSK        (MSK(8) << IIC_TFR_BYTE2_SHF)
#define IIC_TFR_BYTE2_SET        IIC_TFR_BYTE2_MSK

/* field: BYTE1 */
#define IIC_TFR_BYTE1_SHF        16
#define IIC_TFR_BYTE1_MSK        (MSK(8) << IIC_TFR_BYTE1_SHF)
#define IIC_TFR_BYTE1_SET        IIC_TFR_BYTE1_MSK

/* field: BYTE0 */
#define IIC_TFR_BYTE0_SHF        8
#define IIC_TFR_BYTE0_MSK        (MSK(8) << IIC_TFR_BYTE0_SHF)
#define IIC_TFR_BYTE0_SET        IIC_TFR_BYTE0_MSK

/* field: BYTE2ATTR */
#define IIC_TFR_BYTE2ATTR_SHF    6
#define IIC_TFR_BYTE2ATTR_MSK    (MSK(2) << IIC_TFR_BYTE2ATTR_SHF)
#define IIC_TFR_BYTE2ATTR_SET    IIC_TFR_BYTE2ATTR_MSK

/* field: BYTE1ATTR */
#define IIC_TFR_BYTE1ATTR_SHF    4
#define IIC_TFR_BYTE1ATTR_MSK    (MSK(2) << IIC_TFR_BYTE1ATTR_SHF)
#define IIC_TFR_BYTE1ATTR_SET    IIC_TFR_BYTE1ATTR_MSK

/* field: BYTE0ATTR */
#define IIC_TFR_BYTE0ATTR_SHF    2
#define IIC_TFR_BYTE0ATTR_MSK    (MSK(2) << IIC_TFR_BYTE0ATTR_SHF)
#define IIC_TFR_BYTE0ATTR_SET    IIC_TFR_BYTE0ATTR_MSK

/* field: ERR */
#define IIC_TFR_ERR_SHF          1
#define IIC_TFR_ERR_MSK          (MSK(1) << IIC_TFR_ERR_SHF)
#define IIC_TFR_ERR_SET          IIC_TFR_ERR_MSK

/* field: BUSY */
#define IIC_TFR_BUSY_SHF         0
#define IIC_TFR_BUSY_MSK         (MSK(1) << IIC_TFR_BUSY_SHF)
#define IIC_TFR_BUSY_SET         IIC_TFR_BUSY_MSK

/* Values for BYTExATTR fields: */
#define IIC_TFR_BYTEATTR_NOP     0x0
#define IIC_TFR_BYTEATTR_STOP    0x1
#define IIC_TFR_BYTEATTR_CONT    0x2
#define IIC_TFR_BYTEATTR_START   0x3

/******** reg: STC ********/

/* field: CLK */
#define IIC_STC_CLK_SHF          8
#define IIC_STC_CLK_MSK          (MSK(3) << IIC_STC_CLK_SHF)
#define IIC_STC_CLK_SET          IIC_STC_CLK_MSK

/* field: IRQ */
#define IIC_STC_IRQ_SHF          7
#define IIC_STC_IRQ_MSK          (MSK(1) << IIC_STC_IRQ_SHF)
#define IIC_STC_IRQ_SET          IIC_STC_IRQ_MSK

/* field: APERR */
#define IIC_STC_APERR_SHF        5
#define IIC_STC_APERR_MSK        (MSK(1) << IIC_STC_APERR_SHF)
#define IIC_STC_APERR_SET        IIC_STC_APERR_MSK

/* field: DTERR */
#define IIC_STC_DTERR_SHF        4
#define IIC_STC_DTERR_MSK        (MSK(1) << IIC_STC_DTERR_SHF)
#define IIC_STC_DTERR_SET        IIC_STC_DTERR_MSK

/* field: ERR */
#define IIC_STC_ERR_SHF          1
#define IIC_STC_ERR_MSK          (MSK(1) << IIC_STC_ERR_SHF)
#define IIC_STC_ERR_SET          IIC_STC_ERR_MSK

/* field: BUSY */
#define IIC_STC_BUSY_SHF         0
#define IIC_STC_BUSY_MSK         (MSK(1) << IIC_STC_BUSY_SHF)
#define IIC_STC_BUSY_SET         IIC_STC_BUSY_MSK

/* Values for CLK field: */
#define IIC_STC_CLK_5156HZ       0x05
#define IIC_STC_CLK_10312HZ      0x01
#define IIC_STC_CLK_68750HZ      0x04
#define IIC_STC_CLK_103125HZ     0x06
#define IIC_STC_CLK_275000HZ     0x00
#define IIC_STC_CLK_412500HZ     0x02



/*
    IIC Command Buffer 
                                                                        */
typedef struct IIC_SAA9730_cmd_buffer
{
    UINT8 data[3];              /* Byte2, Byte1, Byte0 */
    UINT8 attrib;               /* Attributes for Byte2, Byte1, Byte0  */
    UINT8 index;                /* Current index, 0 = Byte2,..         */
    UINT8 *pread[3];            /* Pointers for storing read data      */
} t_IIC_SAA9730_cmd_buffer ;


/************************************************************************
 *  Macro Definitions
*************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/

/*
    IIC Command Buffer 
*/
static t_IIC_SAA9730_cmd_buffer buf ;

/*
    9730 base register address 
*/
static void *saa9730base ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

/************************************************************************
 *      Driver services:
 ************************************************************************/


/************************************************************************
 *
 *                          IIC_SAA9730_init
 *  Description :
 *  -------------
 *  This service initializes the IIC driver and configures
 *  the IIC-bus speed via the 'syscon' parameter:
 *
 *  'SYSCON_COM_IIC_BAUDRATE_ID'
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
INT32 IIC_SAA9730_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *
 *                          IIC_SAA9730_read
 *  Description :
 *  -------------
 *  This service reads data from the specified IIC device into the
 *  user allocated variable, *p_param.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_read_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC device data read into user variable
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_read_descriptor *p_param ) ; /* INOUT: read buffer      */


/************************************************************************
 *
 *                          IIC_SAA9730_write
 *  Description :
 *  -------------
 *  This service writes user data to the specified IIC device.
 *
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_write_descriptor *p_param ) ; /* IN: write buffer       */


/************************************************************************
 *
 *                          IIC_SAA9730_ctrl
 *  Description :
 *  -------------
 *  This service comprise following aggregated IIC services:
 *   1) 'write_read' IIC device.
 *   2) 'test'       IIC device.
 *
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_ctrl_descriptor *p_param ) ; /* INOUT: IIC device data  */

/************************************************************************
 *      Local helper functions:
 ************************************************************************/

/************************************************************************
 *
 *                          IIC_SAA9730_reset
 *  Description :
 *  -------------
 *
 *  Reset command buffer
 *
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  -
 *
 *
 ************************************************************************/
static 
void IIC_SAA9730_reset( void ) ;


/************************************************************************
 *
 *                          IIC_SAA9730_check
 *  Description :
 *  -------------
 *
 *  Check status and control register for any detected errors.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_check( void ) ;


/************************************************************************
 *
 *                          IIC_SAA9730_get_data
 *  Description :
 *  -------------
 *
 *  Get data from BYTE2, BYTE1 and BYTE0 of Transfer Register and
 *  save it in user allocated buffer space.
 *  
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
 *  -
 *
 ************************************************************************/
static 
void IIC_SAA9730_get_data( void ) ;


/************************************************************************
 *
 *                          IIC_SAA9730_flush
 *  Description :
 *  -------------
 *
 *  Flush out any command or data of command buffer into Transfer
 *  Register.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *  ERROR_IIC_TIMEOUT:        IIC service timed-out
 * 
 ************************************************************************/
static 
INT32 IIC_SAA9730_flush( void ) ;


/************************************************************************
 *
 *                          IIC_SAA9730_add
 *  Description :
 *  -------------
 *
 *  Add another data byte to command buffer and flush in case
 *  command buffer gets full.
 *  
 *
 *  Parameters :
 *  ------------
 *  'data':                   pointer for next data byte to add
 *  'attrib':                 either 'nop', 'start', 'cont', 'stop'
 *  'read':                   'true' by read, 'false' by 'write'.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *  ERROR_IIC_TIMEOUT:        IIC service timed-out
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_add( UINT8 *data,
                       UINT8 attrib,
                       UINT8 read ) ;


/************************************************************************
 *
 *                          IIC_SAA9730_wait
 *  Description :
 *  -------------
 *  
 *  Await IIC-bus operation being completed
 *  
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
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_TIMEOUT'       IIC service timed-out
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_wait( void ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          IIC_SAA9730_install
 *  Description :
 *  -------------
 *
 *  Installs the IIC SAA9730 device drivers services in 
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
INT32 IIC_SAA9730_install( void )
{
	/* pre-initialize local variables and install device services */
	IO_install( SYS_MAJOR_IIC,                /* major device number */
         (t_io_service) IIC_SAA9730_init,         /* 'init'  service     */
                        NULL,                     /* 'open'  service  na */
                        NULL,                     /* 'close' service  na */
         (t_io_service) IIC_SAA9730_read,         /* 'read'  service     */
         (t_io_service) IIC_SAA9730_write,        /* 'write' service     */
         (t_io_service) IIC_SAA9730_ctrl ) ;      /* 'ctrl'  service     */

	/* call our own 'init' service */
	return IO_init( SYS_MAJOR_IIC, 0, NULL);
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          IIC_SAA9730_init
 *  Description :
 *  -------------
 *  This service initializes the IIC driver and configures
 *  the IIC-bus speed via the 'syscon' parameter:
 *
 *  'SYSCON_COM_IIC_BAUDRATE_ID'
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
INT32 IIC_SAA9730_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    int rcode ;

    /* get base for SAA9730 */
    rcode = SYSCON_read( SYSCON_BOARD_SAA9730_BASE_ID,
                         &saa9730base,
                         sizeof(saa9730base) ) ;

    saa9730base = (void *)KSEG1( saa9730base );

    /* reset command buffer */
    IIC_SAA9730_reset() ;

    /* STC-register: set speed = 103.125 KHz */
    REG(saa9730base, IIC_STC) = IIC_STC_CLK_103125HZ << IIC_STC_CLK_SHF;

    return( rcode ) ;
}

/************************************************************************
 *
 *                          IIC_SAA9730_read
 *  Description :
 *  -------------
 *  This service reads data from the specified IIC device into the
 *  user allocated variable, *p_param.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_read_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC device data read into user variable
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_read_descriptor *p_param )   /* INOUT: read buffer      */
{
    int i, rcode ;
    UINT8 addr;
    UINT8 *rdbuf ;

    /* Prepare IIC-read: set slave address and 'read' flag */
    addr = ((p_param->IICaddress) << 1) | 0x01 ; 

    /* Start IIC-read operation */
    rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
    if (rcode != OK) return( rcode ) ;

    /* Continue IIC-read operation */
    rdbuf = p_param->buffer ; 
    for (i = 1; i < p_param->length ; i++)
    {
        rcode = IIC_SAA9730_add(rdbuf++, IIC_TFR_BYTEATTR_CONT, IIC_READCMD);
        if (rcode != OK) return( rcode ) ;
    }

    /* Complete IIC-read operation */
    rcode = IIC_SAA9730_add(rdbuf, IIC_TFR_BYTEATTR_STOP, IIC_READCMD);
    if (rcode != OK) return( rcode ) ;

    /* Get any remaining received data in command buffer */
    rcode = IIC_SAA9730_flush() ;
    return( rcode ) ;
}

/************************************************************************
 *
 *                          IIC_SAA9730_write
 *  Description :
 *  -------------
 *  This service writes user data to the specified IIC device.
 *
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_write_descriptor *p_param )   /* IN: write buffer       */
{
    int i, rcode ;
    UINT8 addr;
    UINT8 *wrbuf ;

    /* Prepare IIC-write: set slave address and 'write' condition */
    addr = ((p_param->IICaddress) << 1) & 0xFE ; 

    /* Start IIC-write operation */
    rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
    if (rcode != OK) return( rcode ) ;

    /* Continue IIC-write operation */
    wrbuf = p_param->buffer ; 
    for (i = 1; i < p_param->length ; i++)
    {
        rcode = IIC_SAA9730_add(wrbuf++, IIC_TFR_BYTEATTR_CONT, IIC_WRITECMD);
        if (rcode != OK) return( rcode ) ;
    }

    /* Complete IIC-write operation */
    rcode = IIC_SAA9730_add(wrbuf, IIC_TFR_BYTEATTR_STOP, IIC_WRITECMD);
    if (rcode != OK) return( rcode ) ;

    /* Flush any remaining data in command buffer */
    rcode = IIC_SAA9730_flush() ;

    /*  Wait until write has finished */
    if (rcode == OK)
    {
        rcode = -1 ;
        while (rcode != OK)
        {
            IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
            IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_STOP, IIC_WRITECMD);
            rcode = IIC_SAA9730_flush() ;
        }
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          IIC_SAA9730_ctrl
 *  Description :
 *  -------------
 *  This service comprise following aggregated IIC services:
 *   1) 'write_read' IIC device.
 *   2) 'test'       IIC device.
 *
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_IIC_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 *
 ************************************************************************/
static
INT32 IIC_SAA9730_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_IIC_ctrl_descriptor *p_param )   /* INOUT: IIC device data  */
{
    int i, rcode ;
    t_IIC_write_read_descriptor *wrd ;
    UINT8 addr;
    UINT8 *buffer ;

    /* set default error, if command not recognized */
    rcode = ERROR_IIC_UNKNOWN_COMMAND ;

    if (p_param->command == IIC_CTRL_TEST)
    {
        /* TEST command: */

        /* Prepare IIC-write: set slave address and 'write' condition */
        addr = ((p_param->IICaddress) << 1) & 0xFE ; 

        /* Start IIC-write operation */
        rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
        if (rcode != OK) return( rcode ) ;


        /* Stop IIC-write operation */
        rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_STOP, IIC_WRITECMD) ;
        if (rcode != OK) return( rcode ) ;

        /* Flush any remaining data in command buffer */
        rcode = IIC_SAA9730_flush() ;
        return( rcode ) ;
    }

    if (p_param->command == IIC_CTRL_WRITE_READ)
    {
        /* WRITE-READ command: */

        /* Prepare IIC-write: set slave address and 'write' condition */
        addr = ((p_param->IICaddress) << 1) & 0xFE ; 

        /* Start IIC-write operation */
        rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
        if (rcode != OK) return( rcode ) ;


        /* Continue IIC-write operation */
        wrd = p_param->data ;
        buffer = wrd->write_buffer ; 
        for (i = 0; i < wrd->write_length; i++)
        {
            rcode = IIC_SAA9730_add( buffer++, 
                                     IIC_TFR_BYTEATTR_CONT, 
                                     IIC_WRITECMD );
            if (rcode != OK) return( rcode ) ;
        }

        /* Prepare IIC-read: set slave address and 'read' condition */
        addr = ((p_param->IICaddress) << 1) | 0x01 ; 

        /* Start IIC-read operation */
        rcode = IIC_SAA9730_add( &addr, IIC_TFR_BYTEATTR_START, IIC_WRITECMD) ;
        if (rcode != OK) return( rcode ) ;

        /* Continue IIC-read operation */
        buffer = wrd->read_buffer ; 
        for (i = 1; i < wrd->read_length; i++)
        {
            rcode = IIC_SAA9730_add( buffer++, 
                                     IIC_TFR_BYTEATTR_CONT, 
                                     IIC_READCMD);
            if (rcode != OK) return( rcode ) ;
        }

        /* Complete IIC-read operation */
        rcode = IIC_SAA9730_add(buffer, IIC_TFR_BYTEATTR_STOP, IIC_READCMD);
        if (rcode != OK) return( rcode ) ;

        /* Flush any remaining data in command buffer */
        rcode = IIC_SAA9730_flush() ;
        return( rcode ) ;
    }

    return( rcode ) ;
}

/************************************************************************
 *      Local helper functions:
 ************************************************************************/


/************************************************************************
 *
 *                          IIC_SAA9730_reset
 *  Description :
 *  -------------
 *
 *  Reset command buffer
 *
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  -
 *
 *
 ************************************************************************/
static 
void IIC_SAA9730_reset( void )
{
    /* reset command buffer */
    buf.index    = 0;
    buf.attrib   = 0;
    buf.data[0]  = 0;
    buf.data[1]  = 0;
    buf.data[2]  = 0;
    buf.pread[0] = 0 ;
    buf.pread[1] = 0 ;
    buf.pread[2] = 0 ;
}


/************************************************************************
 *
 *                          IIC_SAA9730_check
 *  Description :
 *  -------------
 *
 *  Check status and control register for any detected errors.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_check( void )
{
    UINT32 rcode = OK ;

    /* check for any error at all */
    if ( REG(saa9730base, IIC_STC) & IIC_STC_ERR_SET )
    {
        /* check for address phase error */
        if ( REG(saa9730base, IIC_STC) & IIC_STC_APERR_SET )
        {
           /* yes, it was a address phase error */
           rcode = ERROR_IIC_ADDRESS_ERROR ;
        }
        else
        {
           /* no, it was a data error */
           rcode = ERROR_IIC_DATA_ERROR ;
        }

        /* reset any error flag */
        REG(saa9730base, IIC_STC) &= ~IIC_STC_ERR_SET;
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          IIC_SAA9730_get_data
 *  Description :
 *  -------------
 *
 *  Get data from BYTE2, BYTE1 and BYTE0 of Transfer Register and
 *  save it in user allocated buffer space.
 *  
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
 *  -
 *
 ************************************************************************/
static 
void IIC_SAA9730_get_data( void )
{
    int i;
    UINT32 iicdata ;

    /* get data from controller */
    iicdata = REG(saa9730base, IIC_TFR) ;

    /* save it in user allocated buffer space */
    for (i = 0; i < buf.index; i++)
    {
        if (buf.pread[i] != NULL)
        {
            *(buf.pread[i]) = (UINT8)(iicdata >> ((3-i)*8));
        }
    }
}


/************************************************************************
 *
 *                          IIC_SAA9730_flush
 *  Description :
 *  -------------
 *
 *  Flush out any command or data of command buffer into Transfer
 *  Register.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  -
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *  ERROR_IIC_TIMEOUT:        IIC service timed-out
 * 
 ************************************************************************/
static 
INT32 IIC_SAA9730_flush( void )
{
    int rcode = OK ;
    UINT32 iicdata ;

    /* check for any pending commands */
    if ( buf.index > 0 )
    {
        /* TFR-register: request 9730 to transmit max. 3 bytes */
        iicdata = ( (buf.data[0] << 24) | 
                    (buf.data[1] << 16) | 
                    (buf.data[2] <<  8) | 
                    (buf.attrib) ) ;

        REG(saa9730base, IIC_TFR) = iicdata ;

        rcode = IIC_SAA9730_wait() ;
        if (rcode == OK)
        {
            rcode = IIC_SAA9730_check() ;
            if (rcode == OK)
            {
               IIC_SAA9730_get_data() ;
            }
        }

        /* reset command buffer */
        IIC_SAA9730_reset() ;
    }
    return( rcode ) ;
}

/************************************************************************
 *
 *                          IIC_SAA9730_add
 *  Description :
 *  -------------
 *
 *  Add another data byte to command buffer and flush in case
 *  command buffer gets full.
 *  
 *
 *  Parameters :
 *  ------------
 *  'data':                   pointer for next data byte to add
 *  'attrib':                 either 'nop', 'start', 'cont', 'stop'
 *  'read':                   'true' by read, 'false' by 'write'.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  ERROR_IIC_ADDRESS_ERROR:  IIC address error
 *  ERROR_IIC_DATA_ERROR:     IIC data error
 *  ERROR_IIC_TIMEOUT:        IIC service timed-out
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_add( UINT8 *data,
                       UINT8 attrib,
                       UINT8 read )
{
    /* check for read or write */
    if ( read == IIC_READCMD )
    {
        /* read command: save pointer for storing data */
        buf.data[buf.index]    = 0 ;
        buf.pread[buf.index++] = data ;
    }
    else
    {
        /* write command: store data */
        buf.data[buf.index]    = *data ;
        buf.pread[buf.index++] = 0 ;
    }

    /* set attribute for stored byte */
    buf.attrib |= ((attrib)&0x03) << (2*(4 - buf.index));


    /* check for buffer full, and flush */
    if (buf.index == 3)
    {
        return( IIC_SAA9730_flush() ) ;
    }
    return OK ;
}


/************************************************************************
 *
 *                          IIC_SAA9730_wait
 *  Description :
 *  -------------
 *  
 *  Await IIC-bus operation being completed
 *  
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
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_TIMEOUT'       IIC service timed-out
 *
 *
 ************************************************************************/
static 
INT32 IIC_SAA9730_wait( void )
{
    UINT32 rc, first, latest, previous, accumulate, millisec ;

    accumulate = 0 ;

    /* get millisecond count */
    rc = SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                      &first,
                      sizeof(first) ) ;
    previous = first ;
    millisec = IIC_TIMEOUT ;


    while( 1 )
    {
        if ( !(REG(saa9730base, IIC_STC) & IIC_STC_BUSY_SET) )
        {
            return OK ;
        }

        /* get millisecond count */
        rc = SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                          &latest,
                          sizeof(latest) ) ;
        if ( latest >= previous )
        {
            /* counter still not wrapped */
            if ( (accumulate + (latest - first)) > millisec )
            {
                /* time-out */
                break ;
            }
        }
        else
        {
            /* counter did wrap */
            accumulate = accumulate + (previous - first) ;
            if ( (accumulate + latest) > millisec )
            {
                /* time-out */
                break ;
            }

            /* reset first */
            first = 0 ;
        }

        /* prepare next delta time */
        previous = latest ;
    }

    /* time-out */

    /* reset busy flag and any error flag */
    REG(saa9730base, IIC_STC) &= ~(IIC_STC_BUSY_SET | IIC_STC_ERR_SET);
         
    return( ERROR_IIC_TIMEOUT ) ;
}
