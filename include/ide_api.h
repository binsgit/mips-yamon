
#ifndef IDE_API_H
#define IDE_API_H

/************************************************************************
 *
 *      ide_api.h
 *
 *      The 'ide_api' module defines the IDE device driver
 *      interface to be used via 'IDE' device driver services:
 *
 *      The 'IDE' module implements the IDE driver
 *      interface to be used via 'IO' device driver services:
 *
 *        1) init  device:  configure and initialize IDE driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  not used
 *        5) write device:  not used
 *        6) ctrl  device:  a) READ  sector
 *                          b) WRITE sector
 *			    c) READ info
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
 *   IDE device driver, ERROR completion codes
 ************************************************************************/

#define ERROR_IDE                     0x00011000 /* for compile time check   */
#define ERROR_IDE_INVALID_COMMAND        0x11000 /* Internal ERROR: Invalid control command */
#define ERROR_IDE_UNKNOWN_DEVICE         0x11001 /* Internal ERROR: Illegal minor number */
#define ERROR_IDE_NULL_BUFFER            0x11002 /* Internal ERROR: NULL buffer */
#define ERROR_IDE_UNAVAILABLE_DEVICE     0x11003 /* Device unavailable       */
#define ERROR_IDE_BAD_BLOCK              0x11004 /* Bad block detected       */
#define ERROR_IDE_UNCORRECTABLE_DATA     0x11005 /* Uncorrectable data error */
#define ERROR_IDE_MEDIA_CHANGE           0x11006 /* Media change             */
#define ERROR_IDE_ID_NOT_FOUND           0x11007 /* ID not found             */
#define ERROR_IDE_MEDIA_CHANGE_REQUESTED 0x11008 /* Media change requested   */
#define ERROR_IDE_ABORTED                0x11009 /* Aborted command          */
#define ERROR_IDE_TRACK_0_NOT_FOUND      0x1100a /* Track 0 not found        */
#define ERROR_IDE_ADDRESS_MARK_NOT_FOUND 0x1100b /* Address mark not found   */
#define ERROR_IDE_UNKNOWN                0x1100c /* Unknown IDE error        */



typedef struct
{
    UINT16    config;
    UINT16    cyl_count;
    UINT16    reserved0;
    UINT16    head_count;
    UINT16    obsolete0[2];
    UINT16    sectors_per_track;
    UINT16    vendor_specific0[3];
#define IDE_SERIAL_NUMBER_LEN	20
    char      serial_number[IDE_SERIAL_NUMBER_LEN];
    UINT16    obsolete1[2];
    UINT16    ecc_count;
#define IDE_FIRMWARE_REV_LEN	8
    char      firmware_rev[IDE_FIRMWARE_REV_LEN];
#define IDE_MODEL_NAME_LEN 	40
    char      model_name[IDE_MODEL_NAME_LEN];
    UINT16    sectors_per_interrupt;
    UINT16    reserved1;
    UINT16    capabilities;
    UINT16    reserved2;
    UINT16    timing_mode_pio;
    UINT16    timing_mode_dma;
    UINT16    application;
    UINT16    cyl_count_apparent;
    UINT16    head_count_apparent;
    UINT16    sectors_per_track_apparent;
    UINT32    capacity_apparent;
    UINT16    sectors_per_interrupt_apparent;
    UINT32    lba_sector_count;
    UINT16    modes_single_dma;
    UINT16    modes_multiple_dma;
    UINT16    modes_adv_pio;
    UINT16    min_cycle_time_dma;
    UINT16    recommended_cycle_time_dma;
    UINT16    minimum_cycle_time_pio;
    UINT16    minimom_cycle_time_pio_iordy;
    UINT16    reserved3[59];
    UINT16    vendor_specific1[32];
    UINT16    reserved4[96];
}
t_ide_identify;



/************************************************************************
 *   IDE 'ctrl' service
 ************************************************************************/

#define IDE_BYTES_PER_SECTOR	512  /* 512 bytes per sector		*/

typedef struct 
{
    UINT32 command;
#define IDE_CMD_READ		0x20
#define IDE_CMD_WRITE		0x30
#define IDE_CMD_IDENTIFY	0xEC

    union
    {
        struct
	{
	    UINT32 sector;  /* Sector number */
	    UINT32 count;   /* Sector count  */
            UINT8  *buffer; /* Data buffer   */
	} sector;
	
	t_ide_identify identify; /* Parameters read using IDE_IDENTIFY command */
    } u;
}
t_ide_ctrl_descriptor;


/************************************************************************
 *  IDE device driver, minor device numbers
*************************************************************************/

#define IDE_MINOR_PRIMARY_MASTER	0
#define IDE_MINOR_PRIMARY_SLAVE		1
#define IDE_MINOR_SECONDARY_MASTER	2
#define IDE_MINOR_SECONDARY_SLAVE	3

/************************************************************************
 *  IDE device driver services, called by IO subsystem
*************************************************************************/


/************************************************************************
 *
 *                          ide_install
 *  Description :
 *  -------------
 *
 *  Installs the IDE device driver services in the IO system at the 
 *  reserved device slot, found in the 'sysdev.h' file, which defines 
 *  all major device numbers.
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
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 
ide_install( void );

#endif /* #ifndef IDE_API_H */
