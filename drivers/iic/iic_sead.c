
/************************************************************************
 *
 *      iic_sead.c
 *
 *      The 'IIC_SEAD' module implements the IIC device driver
 *      for SEAD and SEAD-2 platforms.
 *
 *        1) init  serial device:  configure and initialize IIC driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  not used
 *        5) write serial device:  not used
 *        6) ctrl  serial device:  a) write+read IIC device
 *                                 b) test IIC device
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
#include <sys_api.h>
#include <io_api.h>
#include <syserror.h>
#include <syscon_api.h>
#include <sysdev.h>
#include <iic_api.h>
#include <product.h>
#include <sead.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

/************************************************************************
 *      Public variables
 ************************************************************************/

/************************************************************************
 *      Static variables
 ************************************************************************/

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static INT32 
iic_sead_init(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    void   *param );		/* INOUT: device parameter block       */

static INT32 
iic_sead_read(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_read_descriptor *param );  /* INOUT: read buffer	       */

static INT32 
iic_sead_write(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_write_descriptor *param );	     /* IN: write buffer       */

static INT32 
iic_sead_ctrl(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_ctrl_descriptor *param );	     /* INOUT: IIC device data */

/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          iic_sead_init
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
iic_sead_init(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    void   *param )		/* INOUT: device parameter block       */
{
    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
    {
	/* Speed up spd clock as SEAD is max 25 Mhz */
	REG(MSC01_MC_REG_BASE, MSC01_MC_SPD_CFG) =
	    (SEAD_MAX_FREQ_MHZ * 5 + 15) / 16;
    }
    return OK;
}


/************************************************************************
 *
 *                          iic_sead_read
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
iic_sead_read(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_read_descriptor *param )   /* INOUT: read buffer	       */
{
    /* TBD : Not ready */
    return ERROR_IIC_COMM_ERROR;
}


/************************************************************************
 *
 *                          iic_sead_write
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
iic_sead_write(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_write_descriptor *param )	     /* IN: write buffer       */
{
    /* TBD : Not ready */
    return ERROR_IIC_COMM_ERROR;
}


/************************************************************************
 *
 *                          iic_sead_ctrl
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
iic_sead_ctrl(
    UINT32 major,		/* IN: major device number             */
    UINT32 minor,		/* IN: minor device number             */
    t_IIC_ctrl_descriptor *param )	     /* INOUT: IIC device data */
{
    t_IIC_write_read_descriptor *descr;
    INT32			rcode;
    UINT8			readcount;
    UINT8			prom_offset;
    UINT8			*bufptr;
    volatile UINT32		data;

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

	readcount   = descr->read_length;
	prom_offset = descr->write_buffer[0];
	bufptr      = descr->read_buffer;

	for (;readcount-- ; prom_offset++, bufptr++)
	{   
	    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	    {
		/* Start read access */
		REG(MSC01_MC_REG_BASE, MSC01_MC_SPD_ADR) = prom_offset;
		do
		{
		    data = REG(MSC01_MC_REG_BASE, MSC01_MC_SPD_DAT);
		}
		while( data & MSC01_MC_SPD_DAT_BUSY_MSK );

		if( data & MSC01_MC_SPD_DAT_RDERR_MSK )
		{
		    rcode = ERROR_IIC_DATA_ERROR;  
		    break;
		}
		*bufptr = data & MSC01_MC_SPD_DAT_RDATA_MSK;
	    }
	    else
	    {
		/* Start read access */
		REG32(KSEG1(SEAD_SD_SPDADR)) = prom_offset;

		do
		{
		    data = REG32(KSEG1(SEAD_SD_SPDDAT));
		}
		while( data & SEAD_SD_SPDDAT_BUSY_BIT );

		if( data & SEAD_SD_SPDDAT_READ_ERR_BIT )
		{
		    rcode = ERROR_IIC_DATA_ERROR;  
		    break;
		}

		*bufptr = data & SEAD_SD_SPDDAT_RDATA_MSK;
	    }
        }

	break;
	
      default :

        rcode = ERROR_IIC_UNKNOWN_COMMAND;
	break;
    }

    return rcode;
}


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          iic_sead_install
 *  Description :
 *  -------------
 *
 *  Installs the IIC SEAD device driver services in 
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
iic_sead_install(void)
{
	/* pre-initialize local variables and install device services */
	IO_install(         SYS_MAJOR_IIC,        /* major device number */
             (t_io_service) iic_sead_init,  	  /* 'init'  service     */
                            NULL,                 /* 'open'  service  na */
                            NULL,                 /* 'close' service  na */
             (t_io_service) iic_sead_read,	  /* 'read'  service     */
             (t_io_service) iic_sead_write,	  /* 'write' service     */
             (t_io_service) iic_sead_ctrl );	  /* 'ctrl'  service     */

	/* call our own 'init' service */
	return IO_init( SYS_MAJOR_IIC, 0, NULL);
}
