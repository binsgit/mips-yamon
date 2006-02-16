
/************************************************************************
 *
 *      iic_piix4.c
 *
 *      The 'IIC_PIIX4' module implements the IIC device driver
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

#include <sysdefs.h>
#include <sys_api.h>
#include <io_api.h>
#include <syserror.h>
#include <syscon_api.h>
#include <sysdev.h>
#include <iic_api.h>
#include <iic_piix4_api.h>
#include <piix4.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

#define SMB_WRITE( offset, data ) \
    REG8( sys_reg_addr( bigend, sizeof(UINT8), base, offset ) ) = data;

#define SMB_READ( offset, p_data ) \
    *p_data = REG8( sys_reg_addr( bigend, sizeof(UINT8), base, offset ) );

/************************************************************************
 *  Macro Definitions
*************************************************************************/

/************************************************************************
 *      Public variables
 ************************************************************************/

/************************************************************************
 *      Static variables
 ************************************************************************/

static void *base;
static bool bigend;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static INT32 
iic_piix4_init(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    void   *param );		/* INOUT: device parameter block       */

static INT32 
iic_piix4_read(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_read_descriptor *param );  /* INOUT: read buffer	       */

static INT32 
iic_piix4_write(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_write_descriptor *param );	     /* IN: write buffer       */

static INT32 
iic_piix4_ctrl(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_ctrl_descriptor *param );	     /* INOUT: IIC device data */

static void 
iic_piix4_poll( void );


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          iic_piix4_init
 *  Description :
 *  -------------
 *  This service initializes the IIC driver
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'param',     INOUT, not used
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0) or error code
 *
 ************************************************************************/
static INT32 
iic_piix4_init(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    void   *param )		/* INOUT: device parameter block       */
{
    SYSCON_read( SYSCON_BOARD_PIIX4_SMB_BASE_ID,
		 (void *)&base,
		 sizeof(UINT32) );

    base = (void *)KSEG1( (UINT32)base );

    SYSCON_read( SYSCON_COM_PIIX4_IIC_BIGEND_ID,
		 (void *)&bigend,
		 sizeof(bool) );

    return OK;
}


/************************************************************************
 *
 *                          iic_piix4_read
 *  Description :
 *  -------------
 *  This service reads data from the specified IIC device into the
 *  user allocated variable, *param.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'param',     INOUT, variable of type, t_IIC_read_descriptor.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC device data read into user variable
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 ************************************************************************/
static INT32 
iic_piix4_read(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_read_descriptor *param )   /* INOUT: read buffer	       */
{
    /* TBD : Not ready */
    return ERROR_IIC_COMM_ERROR;
}


/************************************************************************
 *
 *                          iic_piix4_write
 *  Description :
 *  -------------
 *  This service writes user data to the specified IIC device.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'param',     INOUT, variable of type, t_IIC_write_descriptor.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 ************************************************************************/
static INT32 
iic_piix4_write(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_write_descriptor *param )	     /* IN: write buffer       */
{
    UINT8			writecount;
    UINT8			prom_offset;
    UINT8			*bufptr;

    if (param->length <= 1)
    {
        return ERROR_IIC_DATA_ERROR;  
    }

    bufptr = param->buffer;
    prom_offset = *bufptr++;
    writecount = param->length - 1;

    /* Poll for ready and acknowledge SMBus interrupts */
    iic_piix4_poll();

    for (;writecount-- ; prom_offset++, bufptr++)
    {
        /* Setup slave address and write command */
        SMB_WRITE( PIIX4_SMBHSTADD_OFS,
                   (param->IICaddress << PIIX4_SMBHSTADD_ADDR_SHF) &
                   ~PIIX4_SMBHSTADD_READ_BIT );

        /* Setup command (= offset in slave) */
        SMB_WRITE( PIIX4_SMBHSTCMD_OFS, prom_offset );

        /* Setup data to write */
        SMB_WRITE( PIIX4_SMBHSTDAT0_OFS, *bufptr );

        /* Start Byte Data operation */
        SMB_WRITE( PIIX4_SMBHSTCNT_OFS, 
                  (PIIX4_SMBHSTCNT_CP_BDRW << PIIX4_SMBHSTCNT_CP_SHF) |
                   PIIX4_SMBHSTCNT_START_BIT );

        /* Wait for operation */
        sys_wait_ms(5);

        /* Poll for done and acknowledge SMBus interrupts */
        iic_piix4_poll();
    }

    return OK;
}


/************************************************************************
 *
 *                          iic_piix4_ctrl
 *  Description :
 *  -------------
 *  This service comprise following aggregated IIC services:
 *   1) 'write_read' IIC device.
 *   2) 'test'       IIC device.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'param',   INOUT, variable of type, t_IIC_ctrl_descriptor.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK' = 0x00:              IIC service completed successfully
 *  'ERROR_IIC_COMM_ERROR':   communication error detected
 *
 ************************************************************************/
static INT32 
iic_piix4_ctrl(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_ctrl_descriptor *param )	     /* INOUT: IIC device data */
{
    t_IIC_write_read_descriptor *descr;
    INT32			rcode;
    UINT8			readcount;
    UINT8			prom_offset;
    UINT8			*bufptr;

    rcode = OK;

    switch( param->command )
    {
      case IIC_CTRL_TEST :

        /* TEST command: (TBD not ready) */
	rcode = OK;
	break;

      case IIC_CTRL_WRITE_READ :

        /* WRITE-READ command: */

	descr = (t_IIC_write_read_descriptor *)param->data;

	if( (descr->write_length != 1) ||
	    (descr->read_length  == 0) )
	{
	    rcode = ERROR_IIC_DATA_ERROR;  
            break;
	}

	readcount = descr->read_length;
	prom_offset = descr->write_buffer[0];
	bufptr = descr->read_buffer;

        /* Poll for ready and acknowledge SMBus interrupts */
        iic_piix4_poll();

	for (;readcount-- ; prom_offset++, bufptr++)
	{
            /* Setup slave address and read command */
            SMB_WRITE( PIIX4_SMBHSTADD_OFS,
                       (param->IICaddress << PIIX4_SMBHSTADD_ADDR_SHF) |
                       PIIX4_SMBHSTADD_READ_BIT );

            /* Setup command (= offset in slave) */
            SMB_WRITE( PIIX4_SMBHSTCMD_OFS, prom_offset );

	    /* Start Byte Data operation */
	    SMB_WRITE( PIIX4_SMBHSTCNT_OFS, 
		       (PIIX4_SMBHSTCNT_CP_BDRW << PIIX4_SMBHSTCNT_CP_SHF) |
		       PIIX4_SMBHSTCNT_START_BIT );

            /* Poll for done and acknowledge SMBus interrupts */
            iic_piix4_poll();

            /* read data byte */
            SMB_READ( PIIX4_SMBHSTDAT0_OFS, bufptr );
        }

	break;
	
      default :

        rcode = ERROR_IIC_UNKNOWN_COMMAND;
	break;
    }

    return rcode;
}


/************************************************************************
 *
 *                          iic_piix4_poll
 *  Description :
 *  -------------
 *  This routine polls the SMBus to be not busy
 *    and acknowledges any SMBus interrupt.
 *
 ************************************************************************/
static void 
iic_piix4_poll( void )
{
    volatile UINT8 status;
    for (;;)
    {
        sys_wait_ms(1);
        SMB_READ( PIIX4_SMBHSTSTS_OFS, &status );
        if (status & PIIX4_SMBHSTSTS_BUSY_BIT) continue;
        status &= ( PIIX4_SMBHSTSTS_FAILED_BIT |
                    PIIX4_SMBHSTSTS_COL_BIT    |
                    PIIX4_SMBHSTSTS_DE_BIT     |
                    PIIX4_SMBHSTSTS_INT_BIT );
        if (status == 0) break;
        SMB_WRITE( PIIX4_SMBHSTSTS_OFS, status);
    }
}

/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          iic_piix4_install
 *  Description :
 *  -------------
 *
 *  Installs the IIC PIIX4 device driver services in 
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
iic_piix4_install(void)
{
	/* pre-initialize local variables and install device services */
	IO_install(         SYS_MAJOR_IIC,        /* major device number */
             (t_io_service) iic_piix4_init,  	  /* 'init'  service     */
                            NULL,                 /* 'open'  service  na */
                            NULL,                 /* 'close' service  na */
             (t_io_service) iic_piix4_read,	  /* 'read'  service     */
             (t_io_service) iic_piix4_write,	  /* 'write' service     */
             (t_io_service) iic_piix4_ctrl );	  /* 'ctrl'  service     */

	/* call our own 'init' service */
	return IO_init( SYS_MAJOR_IIC, 0, NULL);
}
