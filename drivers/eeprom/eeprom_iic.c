/************************************************************************
 *
 *      EEPROM_IIC.c
 *
 *      The 'EEPROM_IIC' module implements the EEPROM with IIC-bus access 
 *      device driver as an IO device with following services:
 *
 *        1) init  device:  configure and initialize EEPROM driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  read EEPROM device
 *        5) write device:  write EEPROM device
 *        6) ctrl  device:  not used
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
#include <string.h>
#include <io_api.h>
#include <syscon_api.h>
#include <iic_api.h>
#include <eeprom_api.h>
#include <eeprom_iic_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

/*
    Device context per EEPROM
                                                                        */
typedef struct EEPROM_IIC_device
{
    UINT32  size ;           /* size of EEPROM device                   */
    UINT32  pagesize ;       /* pagesize of EEPROM device               */
    UINT8   iicadr ;         /* IIC-slave base address                  */
    UINT8   mode ;           /* read-only or read-write                 */
} t_EEPROM_IIC_device ;


#define EEPROM_READ_ONLY_MODE    0x42
#define EEPROM_READ_WRITE_MODE   0x43

/************************************************************************
 *      NM24C09 EEPROM profile 
 ************************************************************************/

#define EEPROM_NM24C09_SIZE          1024
#define EEPROM_NM24C09_PAGESIZE      256


/************************************************************************
 *      SPD EEPROM profile of PC-SDRAM
 ************************************************************************/

#define EEPROM_SPD_SIZE              256
#define EEPROM_SPD_PAGESIZE          256


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
  EEPROM device context
*/
static
t_EEPROM_IIC_device device[EEPROM_MINOR_DEVICE_COUNT] ;

/*
  Temporary buffer for read and write operations 
*/
static UINT8  iic_buffer[EEPROM_NM24C09_PAGESIZE+4] ;


/* Error strings */
static char* error_strings[] =
{
    /* ERROR_EEPROM_COMM_ERROR */         "I2C communication error detected",
    /* ERROR_EEPROM_UNKNOWN_DEVICE */     "Unknown device",
    /* ERROR_EEPROM_PARAM_OUT_OF_SPACE */ "Address space out of range",
    /* ERROR_EEPROM_READ_ONLY */          "EEPROM is read-only"
};


/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          EEPROM_IIC_init
 *  Description :
 *  -------------
 *  This service initializes the EEPROM IIC driver.
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
INT32 EEPROM_IIC_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *
 *                          EEPROM_IIC_read
 *  Description :
 *  -------------
 *  This service reads data from a specified location of a specified
 *  EEPROM device.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_EEPROM_read_descriptor.
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
INT32 EEPROM_IIC_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_read_descriptor *p_param ) ; /* INOUT: read buffer   */


/************************************************************************
 *
 *                          EEPROM_IIC_write
 *  Description :
 *  -------------
 *  This service writes data into a specified location of a specified
 *  EEPROM device.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_EEPROM_write_descriptor.
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
INT32 EEPROM_IIC_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_write_descriptor *p_param ) ; /* IN: write buffer    */


static INT32 
error_lookup( 
    t_sys_error_string *param );


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          EEPROM_IIC_install
 *  Description :
 *  -------------
 *
 *  Installs the EEPROM device driver services in 
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
INT32 EEPROM_IIC_install( void )
{
	/* pre-initialize local variables and install device services */
	IO_install( SYS_MAJOR_EEPROM_IIC,      /* major device number */
         (t_io_service) EEPROM_IIC_init,       /* 'init'  service     */
                        NULL,                  /* 'open'  service  na */
                        NULL,                  /* 'close' service  na */
         (t_io_service) EEPROM_IIC_read,       /* 'read'  service     */
         (t_io_service) EEPROM_IIC_write,      /* 'write' service     */
                        NULL  ) ;              /* 'ctrl'  service  na */

	/* call our own 'init' service */
	return IO_init( SYS_MAJOR_EEPROM_IIC, 0, NULL);
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          EEPROM_IIC_init
 *  Description :
 *  -------------
 *  This service initializes the EEPROM driver.
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
INT32 EEPROM_IIC_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    t_sys_error_lookup_registration  registration;

    /* register error lookup function */
    registration.prefix = SYSERROR_DOMAIN( ERROR_EEPROM );
    registration.lookup = error_lookup;

    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    /* initialize EEPROM profile for NM24C09 */
    device[EEPROM_MINOR_NM24C09].size     = EEPROM_NM24C09_SIZE ;
    device[EEPROM_MINOR_NM24C09].pagesize = EEPROM_NM24C09_PAGESIZE ;
    device[EEPROM_MINOR_NM24C09].mode     = EEPROM_READ_WRITE_MODE ;

    SYSCON_read( SYSCON_BOARD_EEPROM_ADDR,
		 &device[EEPROM_MINOR_NM24C09].iicadr,
		 sizeof(UINT8) );

    /* initialize EEPROM profile for SPD PC-SDRAM */
    device[EEPROM_MINOR_SPD000].size      = EEPROM_SPD_SIZE ;
    device[EEPROM_MINOR_SPD000].pagesize  = EEPROM_SPD_PAGESIZE ;
    device[EEPROM_MINOR_SPD000].mode      = EEPROM_READ_ONLY_MODE ;

    SYSCON_read( SYSCON_BOARD_EEPROM_SPD_ADDR,
		 &device[EEPROM_MINOR_SPD000].iicadr,
		 sizeof(UINT8) );

    return( OK ) ;
}

/************************************************************************
 *
 *                          EEPROM_IIC_read
 *  Description :
 *  -------------
 *  This service reads data from a specified location of a specified
 *  EEPROM device.
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_EEPROM_read_descriptor.
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
INT32 EEPROM_IIC_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_read_descriptor *p_param )   /* INOUT: read buffer   */
{
    int    rcode ;
    UINT32 offset, left ;
    UINT8  startpage, endpage, iicadr, readcount ;
    UINT8  *user_buffer ;
    UINT8  roffset ;
    t_IIC_write_read_descriptor iic_read ;
    t_IIC_ctrl_descriptor       iic_ctrl ;

    /* init ctrl descriptors */
    iic_ctrl.command = IIC_CTRL_WRITE_READ ;
    iic_ctrl.data    = &iic_read ;
    iic_read.write_length = 1 ;
    iic_read.write_buffer = &roffset  ;

    /* check minor device number */
    if (minor >= EEPROM_MINOR_DEVICE_COUNT)
    {
        return(ERROR_EEPROM_UNKNOWN_DEVICE) ;
    }

    /* check read request */
    if ( (p_param->offset + p_param->length) > device[minor].size )
    {
        return(ERROR_EEPROM_PARAM_OUT_OF_SPACE) ;
    }

    offset      = p_param->offset ;
    left        = p_param->length ;
    user_buffer = p_param->buffer ;
    while (left > 0)
    {
      startpage = offset / device[minor].pagesize ;
      endpage   = (offset + left) / device[minor].pagesize ;

      /* 1: calculate IIC slave address */
      iicadr = device[minor].iicadr + startpage ;

      /* 2: calculate bytes to read */
      if (startpage != endpage)
      {
          readcount = device[minor].pagesize - 
                      offset % device[minor].pagesize ;
      }
      else
      {
          readcount = left ;
      }

      /* 3: read 'readcount' */
      iic_ctrl.IICaddress  = iicadr ;
      roffset              = offset % device[minor].pagesize ;
      iic_read.read_length = readcount ;
      iic_read.read_buffer = user_buffer ;
      rcode = IO_ctrl( SYS_MAJOR_IIC, 0, &iic_ctrl ) ;
      if ( rcode != OK)
      {
          return(rcode) ;
      }

      /* 4: update context */
      left        = left - readcount ;
      offset      = offset + readcount ;
      user_buffer = user_buffer + readcount ;
    }

    return( OK ) ;
}

/************************************************************************
 *
 *                          EEPROM_IIC_write
 *  Description :
 *  -------------
 *  This service writes data into a specified location of a specified
 *  EEPROM device.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, variable of type, t_EEPROM_write_descriptor.
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
INT32 EEPROM_IIC_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_write_descriptor *p_param )   /* IN: write buffer    */
{
    int    rcode ;
    UINT32 offset, left ;
    UINT8  startpage, endpage, iicadr, writecount ;
    UINT8  *user_buffer ;
    t_IIC_write_descriptor iic_write ;

    /* check minor device number */
    if (minor >= EEPROM_MINOR_DEVICE_COUNT)
    {
        return(ERROR_EEPROM_UNKNOWN_DEVICE) ;
    }

    /* check write request */
    if ( (p_param->offset + p_param->length) > device[minor].size )
    {
        return(ERROR_EEPROM_PARAM_OUT_OF_SPACE) ;
    }

    /* check mode */
    if ( device[minor].mode == EEPROM_READ_ONLY_MODE)
    {
        return(ERROR_EEPROM_READ_ONLY) ;
    }

    offset      = p_param->offset ;
    left        = p_param->length ;
    user_buffer = p_param->buffer ;
    while (left > 0)
    {
      startpage = offset / device[minor].pagesize ;
      endpage   = (offset + left) / device[minor].pagesize ;

      /* 1: calculate IIC slave address */
      iicadr = device[minor].iicadr + startpage ;

      /* 2: calculate bytes to write */
      if (startpage != endpage)
      {
          writecount = device[minor].pagesize - 
                       offset % device[minor].pagesize ;
      }
      else
      {
          writecount = left ;
      }

      /* 3: prepend offset to user data in temp buffer */
      iic_buffer[0] = offset % device[minor].pagesize ;
      memcpy( &iic_buffer[1], user_buffer, writecount ) ;

      /* 4: write 'writecount' */
      iic_write.IICaddress = iicadr ;
      iic_write.length     = writecount + 1 ;
      iic_write.buffer     = &iic_buffer[0] ;
      rcode = IO_write(SYS_MAJOR_IIC,0,&iic_write) ;
      if ( rcode != OK)
      {
          return(rcode) ;
      }

      /* 5: update context */
      left        = left - writecount ;
      offset      = offset + writecount ;
      user_buffer = user_buffer + writecount ;
    }
    return( OK ) ;
}


/************************************************************************
 *                          error_lookup
 ************************************************************************/
static INT32 
error_lookup( 
    t_sys_error_string *param )
{
    UINT32 index = SYSERROR_ID( param->syserror );

    if( index < sizeof(error_strings)/sizeof(char*) )
    {
        param->strings[SYSCON_ERRORMSG_IDX] = error_strings[index];
        param->count = 1;
    }
    else
        param->count = 0;

    return OK;
}
