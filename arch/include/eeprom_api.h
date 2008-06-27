#ifndef EEPROM_API_H
#define EEPROM_API_H

/************************************************************************
 *
 *      EEPROM_api.h
 *
 *      The 'EEPROM_api' module defines the EEPROM device driver
 *      interface to be used via 'EEPROM' device driver services:
 *
 *        1) init  serial device:  configure and initialize EEPROM driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  read EEPROM device
 *        5) write serial device:  write EEPROM device
 *        6) ctrl  serial device:  not used
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
 *    Include files
 ************************************************************************/

#include "sysdefs.h"
#include "syserror.h"

/************************************************************************
 *   EEPROM device driver, ERROR completion codes
*************************************************************************/

#define ERROR_EEPROM                0x00006000 /* for compile time check */
#define ERROR_EEPROM_COMM_ERROR         0x6000 /* Communication error    */
#define ERROR_EEPROM_UNKNOWN_DEVICE     0x6001 /* Unknown minor device   */
#define ERROR_EEPROM_PARAM_OUT_OF_SPACE 0x6002 /* offset+length exceeds EEPROM space */
#define ERROR_EEPROM_READ_ONLY          0x6003 /* EEPROM is read-only    */


/************************************************************************
 *  Parameter definitions
*************************************************************************/


/* 
    EEPROM read descriptor, to be used with EEPROM-'read' service
                                                                        */
typedef struct EEPROM_read_descriptor
{
    UINT32 offset ;        /* EEPROM byte offset address */
    UINT32 length ;        /* number of bytes to read */
    UINT8  *buffer ;       /* pointer for buffer to store read data */
} t_EEPROM_read_descriptor ;


/* 
    EEPROM write descriptor, to be used with EEPROM-'write' service
                                                                        */
typedef struct EEPROM_write_descriptor
{
    UINT32 offset ;        /* EEPROM byte offset address */
    UINT32 length ;        /* number of bytes to write */
    UINT8  *buffer ;       /* pointer for buffer of data to be written */
} t_EEPROM_write_descriptor ;


/************************************************************************
 *  EEPROM device driver, minor device numbers
*************************************************************************/

typedef enum EEPROM_MINOR_devices
{
    EEPROM_MINOR_NM24C09 = 0,   /* System: Fairchild, 1024 byte, 
                                   upper 512 byte write-protected 	*/
    EEPROM_MINOR_SPD000,        /* PC-SDRAM		            	*/

    EEPROM_MINOR_DEVICE_COUNT   /* number of minor EEPROM devices 	*/
} t_EEPROM_MINOR_devices ;



/************************************************************************
 *  EEPROM device driver services, called by IO subsystem
*************************************************************************/



/* 
   NAME:  'init'

   DESCRIPTION:
   This service initializes the EEPROM driver to handle all 
   EEPROM devices on this board environment (set of minor devices)


   RETURN VALUES:
   'OK' = 0x00

                                                                        */
typedef INT32 (*t_EEPROM_init_service)( 
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* not used                            */
          void   *p_param ) ;    /* not used                            */



/* 
   NAME: 'open'    NOT USED

   DESCRIPTION:
     -

   RETURN VALUES:
     -
                                                                        */


/* 
   NAME: 'close'   NOT USED

   DESCRIPTION:
     -

   RETURN VALUES:
     -
                                                                        */

/* 
   NAME: 'read'

   DESCRIPTION:
   This service reads data from a specified location of a specified
   EEPROM device.

   RETURN VALUES:
   'OK' = 0x00:                 specified EEPROM value read
   'ERROR_EEPROM_COMM_ERROR':   communication error detected
                                                                        */
typedef INT32 (*t_EEPROM_read_service)( 
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_read_descriptor *p_param ) ; /* INOUT: read data     */


/* 
   NAME: 'write'

   DESCRIPTION:
   This service writes data into a specified location of a specified
   EEPROM device.

   RETURN VALUES:
   'OK' = 0x00:                 specified EEPROM value written 
   'ERROR_EEPROM_COMM_ERROR':   communication error detected

                                                                        */
typedef INT32 (*t_EEPROM_write_service)( 
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_EEPROM_write_descriptor *p_param ) ; /* OUT: write data     */


/* 
   NAME: 'ctrl'    NOT USED

   DESCRIPTION:
     -

   RETURN VALUES:
     -
                                                                        */


#endif /* #ifndef EEPROM_API_H */
