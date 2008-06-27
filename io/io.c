/************************************************************************
 *
 *      io.c
 *
 *      The 'io' module implements a generic api for all device drivers.
 *      Through this interface all interaction between the user and the
 *      device driver is executed by use of a standard set of io-functions
 *      to:
 *
 *        1) init  device,
 *        2) open  device,
 *        3) close device,
 *        4) read  device,
 *        5) write device,
 *        6) ctrl  device.
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

#include "sysdefs.h"
#include "syserror.h"
#include "io_api.h"
#include "syscon_api.h"

/************************************************************************
 *      Definitions
 ************************************************************************/

/*
   IO device entry definition
                                                                        */
typedef struct IO_device
{
	t_io_service  init  ;
	t_io_service  open  ;
	t_io_service  close ;
	t_io_service  read  ;
	t_io_service  write ;
	t_io_service  ctrl  ;
	void          *data ;
	unsigned int  state ;
} t_IO_device ;



/************************************************************************
 *   IO enumeration definitions
*************************************************************************/

/*
   IO device entry states
                                                                        */
typedef enum IO_device_entry_state
{
    IO_DEVICE_NOT_PRESENT = 0x42,       /* IO device is not present     */
    IO_DEVICE_PRESENT     = 0x4242,     /* IO device has been installed */

} t_IO_device_entry_state ;

/************************************************************************
 *  Macro Definitions
*************************************************************************/


#define IO_CHECK_MAJOR( major, last )  \
if ( major > last )  \
{ \
    return( ERROR_IO_ILLEGAL_MAJOR ) ; \
} 

/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/

static t_IO_device  *IO ;    /* keeps the ref. for the IO device table  */
static UINT32       last_device ;    /* keeps the configured last device
                                        number of the IO table.
					(The size of the IO table is 
					'last_device'+1)                */


/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          IO_dummy_no_driver
 *  Description :
 *  -------------
 *
 *  This is a dummy device driver service, which is inserted in each
 *  service entry of each allocated device in the IO device table.
 *  This will ensure a device service to be handled with a correct
 *  service completion ('ERROR_IO_NO_DRIVER') as default service.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *
 ************************************************************************/
static
INT32 IO_dummy_no_driver(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *
 *                          IO_dummy_no_service
 *  Description :
 *  -------------
 *
 *  This is a dummy device driver service, which is the default service,
 *  for unsupported services of installed device drivers.
 *  This will ensure a device service to be handled with a correct
 *  service completion ('ERROR_IO_NO_SERVICE') as default service.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'ERROR_IO_NO_SERVICE':     Unsupported service
 *
 ************************************************************************/
static
INT32 IO_dummy_no_service(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          IO_setup
 *  Description :
 *  -------------
 *
 *  Initializes the 'IO' module.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'devices',   IN,    allocate space for max. 'devices' to be
 *                      handled by the IO system
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_NO_SPACE':       Not enough memory
 *
 ************************************************************************/
INT32 IO_setup(  
          UINT32 devices  )   /* IN: max. 'devices' to be supported     */
{
    t_sys_malloc       mem ;
    int i, rcode ;

    /* calculate IO table size and allocate table */
    mem.size     = devices * sizeof(t_IO_device) ;
    mem.boundary = 8 ;        /* double long word */
    mem.memory   = (void**) &IO ; 
    rcode = SYSCON_read( SYSCON_BOARD_MALLOC_ID,
                         &mem,
                         sizeof(mem) ) ;
    if (rcode != OK)
    {
        return( rcode ) ;
    }

    /* initialize io device table */
    for (i=0; i<devices; i++)
    {
        IO[i].init  = IO_dummy_no_driver ;
        IO[i].open  = IO_dummy_no_driver ;
        IO[i].close = IO_dummy_no_driver ;
        IO[i].read  = IO_dummy_no_driver ;
        IO[i].write = IO_dummy_no_driver ;
        IO[i].ctrl  = IO_dummy_no_driver ;
        IO[i].data  = NULL ;
        IO[i].state = IO_DEVICE_NOT_PRESENT ;
    }

    /* save the configured number of devices */
    last_device = devices - 1 ;
    return( OK ) ;
}


/************************************************************************
 *
 *                          IO_install
 *  Description :
 *  -------------
 *
 *  Installs the device drivers services in the IO system at the
 *  specified device slot, which is adressed with the 'major'
 *  device number. The service function pointers may be 'NULL',
 *  but at least one service must be supplied.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'init',      IN,    device drivers 'init'  service function pointer
 *  'open',      IN,    device drivers 'open'  service function pointer
 *  'close',     IN,    device drivers 'close' service function pointer
 *  'read',      IN,    device drivers 'read'  service function pointer
 *  'write',     IN,    device drivers 'write' service function pointer
 *  'ctrl',      IN,    device drivers 'ctrl'  service function pointer
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_DEVICE': No device services are provided
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 IO_install(
          UINT32       major,    /* IN: major device number             */
          t_io_service init,     /* 'init'  service function pointer    */
          t_io_service open,     /* 'open'  service function pointer    */
          t_io_service close,    /* 'close' service function pointer    */
          t_io_service read,     /* 'read'  service function pointer    */
          t_io_service write,    /* 'write' service function pointer    */
          t_io_service ctrl )    /* 'ctrl'  service function pointer    */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* check, if device slot has been allocated by another driver */
    if ( IO[major].state == IO_DEVICE_PRESENT )
    {
        return( ERROR_IO_NO_SPACE ) ;
    }

    /* check if any service present */
    if ( (init == NULL) && (open  == NULL) && (close == NULL) && 
         (read == NULL) && (write == NULL) && (ctrl  == NULL) )
    {
        return( ERROR_IO_ILLEGAL_DEVICE ) ;
    }

    /* if present, install 'init' service */
    if ( init != NULL )
    {
        IO[major].init  = init ;
    }
    else
    {
         IO[major].init  = IO_dummy_no_service ;
    }

    /* if present, install 'open' service */
    if ( open != NULL )
    {
         IO[major].open  = open ;
    }
    else
    {
        IO[major].open  = IO_dummy_no_service ;
    }

    /* if present, install 'close' service */
    if ( close != NULL )
    {
        IO[major].close  = close ;
    }
    else
    {
         IO[major].close  = IO_dummy_no_service ;
    }

    /* if present, install 'read' service */
    if ( read != NULL )
    {
        IO[major].read  = read ;
    }
    else
    {
        IO[major].read  = IO_dummy_no_service ;
    }

    /* if present, install 'write' service */
    if ( write != NULL )
    {
        IO[major].write  = write ;
    }
    else
    {
        IO[major].write  = IO_dummy_no_service ;
    }

    /* if present, install 'ctrl' service */
    if ( ctrl != NULL )
    {
        IO[major].ctrl  = ctrl ;
    }
    else
    {
        IO[major].ctrl  = IO_dummy_no_service ;
    }

    /* complete device installation */
    IO[major].data  = NULL ;
    IO[major].state = IO_DEVICE_PRESENT ;
    return( OK ) ;
}


/************************************************************************
 *
 *                          IO_deinstall
 *  Description :
 *  -------------
 *
 *  Deinstall device driver at the  specified device slot, which is 
 *  adressed with the 'major' device number.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *
 ************************************************************************/
INT32 IO_deinstall(
          UINT32       major )   /* IN: major device number             */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    IO[major].init  = IO_dummy_no_driver ;
    IO[major].open  = IO_dummy_no_driver ;
    IO[major].close = IO_dummy_no_driver ;
    IO[major].read  = IO_dummy_no_driver ;
    IO[major].write = IO_dummy_no_driver ;
    IO[major].ctrl  = IO_dummy_no_driver ;
    IO[major].data  = NULL ;
    IO[major].state = IO_DEVICE_NOT_PRESENT ;
    return( OK ) ;
}


/************************************************************************
 *
 *                          IO_init
 *  Description :
 *  -------------
 *
 *  This function looks up the 'init' function of the driver by use of
 *  the 'major' device number and calls the 'init' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'init'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].init( major, minor, p_param ) ) ;
}


/************************************************************************
 *
 *                          IO_open
 *  Description :
 *  -------------
 *
 *  This function looks up the 'open' function of the driver by use of
 *  the 'major' device number and calls the 'open' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'open'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_open(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].open( major, minor, p_param ) ) ;
}



/************************************************************************
 *
 *                          IO_close
 *  Description :
 *  -------------
 *
 *  This function looks up the 'close' function of the driver by use of
 *  the 'major' device number and calls the 'close' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'close'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_close(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].close( major, minor, p_param ) ) ;
}



/************************************************************************
 *
 *                          IO_read
 *  Description :
 *  -------------
 *
 *  This function looks up the 'read' function of the driver by use of
 *  the 'major' device number and calls the 'read' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'read'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_read(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].read( major, minor, p_param ) ) ;
}



/************************************************************************
 *
 *                          IO_write
 *  Description :
 *  -------------
 *
 *  This function looks up the 'write' function of the driver by use of
 *  the 'major' device number and calls the 'write' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'write'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].write( major, minor, p_param ) ) ;
}


/************************************************************************
 *
 *                          IO_ctrl
 *  Description :
 *  -------------
 *
 *  This function looks up the 'ctrl' function of the driver by use of
 *  the 'major' device number and calls the 'ctrl' function with the
 *  supplied parameters, 'minor' and 'p_param'.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *  'ERROR_IO_NO_FUNCTION':   Driver don't implement 'ctrl'
 *
 *
 *  Note :
 *  ------
 *  The return values above are the IO-system specific values only.
 *  Errors, which are specific for the device driver, may be returned too.
 *
 ************************************************************************/
INT32 IO_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* call service */
    return( IO[major].ctrl( major, minor, p_param ) ) ;
}


/************************************************************************
 *
 *                          IO_lookup
 *  Description :
 *  -------------
 *
 *  Lookup device driver data context pointer
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'p_data',    OUT,   data context pointer
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *
 ************************************************************************/
INT32 IO_lookup(  
          UINT32 major,          /* IN: major device number             */
          void   **p_data )      /* OUT: device context data            */
{
    /* in case any error detected, return 'NULL' */
    *p_data = NULL ;

    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* check, if driver has been installed */
    if ( IO[major].state == IO_DEVICE_PRESENT )
    {
        *p_data = IO[major].data ;
        return( OK ) ;
    }
    else
    {
        return( ERROR_IO_NO_DRIVER ) ;
    }
}


/************************************************************************
 *
 *                          IO_save
 *  Description :
 *  -------------
 *
 *  Save device driver data context pointer
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'p_data',    IN,    data context pointer
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR': Illegal major device number
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *
 ************************************************************************/
INT32 IO_save(  
          UINT32 major,          /* IN: major device number             */
          void   *p_data )       /* IN: device context data             */
{
    /* check for invalid major device number */
    IO_CHECK_MAJOR( major, last_device ) ;

    /* check, if driver has been installed */
    if ( IO[major].state == IO_DEVICE_PRESENT )
    {
        IO[major].data = p_data ;
        return( OK ) ;
    }
    else
    {
        return( ERROR_IO_NO_DRIVER ) ;
    }
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          IO_dummy_no_driver
 *  Description :
 *  -------------
 *
 *  This is a dummy device driver service, which is inserted in each
 *  service entry of each allocated device in the IO device table.
 *  This will ensure a device service to be handled with a correct
 *  service completion ('ERROR_IO_NO_DRIVER') as default service.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'ERROR_IO_NO_DRIVER':     Driver not installed
 *
 ************************************************************************/
static
INT32 IO_dummy_no_driver(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    return( ERROR_IO_NO_DRIVER ) ;
}



/************************************************************************
 *
 *                          IO_dummy_no_service
 *  Description :
 *  -------------
 *
 *  This is a dummy device driver service, which is the default service,
 *  for unsupported services of installed device drivers.
 *  This will ensure a device service to be handled with a correct
 *  service completion ('ERROR_IO_NO_SERVICE') as default service.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, device specific parameter block
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'ERROR_IO_NO_SERVICE':     Unsupported service
 *
 ************************************************************************/
static
INT32 IO_dummy_no_service(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    return( ERROR_IO_NO_SERVICE ) ;
}
