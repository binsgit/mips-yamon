#ifndef IO_API_H
#define IO_API_H

/************************************************************************
 *
 *      io_api.h
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
 *    Include files
 ************************************************************************/

#include "sysdefs.h"
#include "syserror.h"


/************************************************************************
 *  System Definitions
*************************************************************************/


/* 
   Device driver service function prototype
                                                                        */
typedef INT32 (*t_io_service)( 
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */


/************************************************************************
 *  ERROR completion codes for the IO system
*************************************************************************/

#define ERROR_IO             0x00002000 /* for compile time check       */
#define ERROR_IO_ILLEGAL_MAJOR   0x2000 /* Illegal major device number  */
#define ERROR_IO_NO_DRIVER       0x2001 /* Driver not installed         */
#define ERROR_IO_NO_SERVICE      0x2002 /* Driver service not implem.   */
#define ERROR_IO_NO_SPACE        0x2003 /* Major device slot not free   */
#define ERROR_IO_ILLEGAL_DEVICE  0x2004 /* No services implemented      */
#define ERROR_IO_TAG_SIZE        0x2005


/************************************************************************
 *    Public variables
 ************************************************************************/

/************************************************************************
 *   Public functions
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
          UINT32 devices  ) ; /* IN: max. 'devices' to be supported     */


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
          t_io_service ctrl ) ;  /* 'ctrl'  service function pointer    */


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
          UINT32       major );  /* IN: major device number             */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   *p_param ) ;    /* INOUT: device parameter block       */


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
          void   **p_data ) ;    /* OUT: device context data            */


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
          void   *p_data ) ;     /* IN: device context data             */


#endif /* #ifndef IO_API_H */
