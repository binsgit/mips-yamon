
/************************************************************************
 *
 *      ide.c
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
 *      Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <io_api.h>
#include <ide_api.h>
#include <stdio.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

#define IDE_DATA_OFS		0
#define IDE_ERROR_OFS		1
#define IDE_SECTOR_COUNT_OFS	2
#define IDE_SECTOR_NUMBER_OFS	3
#define IDE_CYL_LOW_OFS		4
#define IDE_CYL_HI_OFS		5
#define IDE_DRIVE_HEAD_OFS	6
#define IDE_COMMAND_OFS		7
#define IDE_STATUS_OFS		7


#define IDE_ERROR_BBK_SHF	7
#define IDE_ERROR_BBK_MSK	(MSK(1) << IDE_ERROR_BBK_SHF)
#define IDE_ERROR_BBK_BIT	IDE_ERROR_BBK_MSK

#define IDE_ERROR_UNC_SHF	6
#define IDE_ERROR_UNC_MSK	(MSK(1) << IDE_ERROR_UNC_SHF)
#define IDE_ERROR_UNC_BIT	IDE_ERROR_UNC_MSK

#define IDE_ERROR_MC_SHF	5
#define IDE_ERROR_MC_MSK	(MSK(1) << IDE_ERROR_MC_SHF)
#define IDE_ERROR_MC_BIT	IDE_ERROR_MC_MSK

#define IDE_ERROR_IDNF_SHF	4
#define IDE_ERROR_IDNF_MSK	(MSK(1) << IDE_ERROR_IDNF_SHF)
#define IDE_ERROR_IDNF_BIT	IDE_ERROR_IDNF_MSK

#define IDE_ERROR_MCR_SHF	3
#define IDE_ERROR_MCR_MSK	(MSK(1) << IDE_ERROR_MCR_SHF)
#define IDE_ERROR_MCR_BIT	IDE_ERROR_MCR_MSK

#define IDE_ERROR_ABRT_SHF	2
#define IDE_ERROR_ABRT_MSK	(MSK(1) << IDE_ERROR_ABRT_SHF)
#define IDE_ERROR_ABRT_BIT	IDE_ERROR_ABRT_MSK

#define IDE_ERROR_TK0NF_SHF	1
#define IDE_ERROR_TK0NF_MSK	(MSK(1) << IDE_ERROR_TK0NF_SHF)
#define IDE_ERROR_TK0NF_BIT	IDE_ERROR_TK0NF_MSK

#define IDE_ERROR_AMNF_SHF	0
#define IDE_ERROR_AMNF_MSK	(MSK(1) << IDE_ERROR_AMNF_SHF)
#define IDE_ERROR_AMNF_BIT	IDE_ERROR_AMNF_MSK


#define IDE_DRIVE_HEAD_DEV_SHF	4
#define IDE_DRIVE_HEAD_DEV_MSK  (MSK(1) << IDE_DRIVE_HEAD_DEV_SHF)
#define IDE_DRIVE_HEAD_DEV_BIT  IDE_DRIVE_HEAD_DEV_MSK


#define IDE_STATUS_ERR_SHF	0
#define IDE_STATUS_ERR_MSK	(MSK(1) << IDE_STATUS_ERR_SHF)
#define IDE_STATUS_ERR_BIT	IDE_STATUS_ERR_MSK

#define IDE_STATUS_DRQ_SHF	3
#define IDE_STATUS_DRQ_MSK	(MSK(1) << IDE_STATUS_DRQ_SHF)
#define IDE_STATUS_DRQ_BIT	IDE_STATUS_DRQ_MSK

#define IDE_STATUS_DSC_SHF	4
#define IDE_STATUS_DSC_MSK	(MSK(1) << IDE_STATUS_DSC_SHF)
#define IDE_STATUS_DSC_BIT	IDE_STATUS_DSC_MSK

#define IDE_STATUS_DF_SHF	5
#define IDE_STATUS_DF_MSK	(MSK(1) << IDE_STATUS_DF_SHF)
#define IDE_STATUS_DF_BIT	IDE_STATUS_DF_MSK

#define IDE_STATUS_DRDY_SHF	6
#define IDE_STATUS_DRDY_MSK	(MSK(1) << IDE_STATUS_DRDY_SHF)
#define IDE_STATUS_DRDY_BIT	IDE_STATUS_DRDY_MSK

#define IDE_STATUS_BSY_SHF	7
#define IDE_STATUS_BSY_MSK	(MSK(1) << IDE_STATUS_BSY_SHF)
#define IDE_STATUS_BSY_BIT	IDE_STATUS_BSY_MSK

/*  Calc addresses to use for 8 and 32 bit accesses to IDE controller.
 *  Macro SWAP_BYTEADDR_EL swaps the 2 lsb bits of the addr in case 
 *  CPU is running Big Endian. 
 *  This is since we assume the IDE controller is a PCI device, 
 *  and PCI is always Little Endian.
 */
#define IO_ADDR(ofs)		(pci_io_base + ide_io_base + (ofs))
#define IO32(ofs)		REG32(KSEG1(IO_ADDR(ofs)))
#define IO8(ofs)		REG8(KSEG1(SWAP_BYTEADDR_EL(IO_ADDR(ofs))))

#define IDE_PRIMARY_IO_CMD_BASE		0x1f0
#define IDE_SECONDARY_IO_CMD_BASE	0x170
#define IDE_DEV_MASTER			0
#define IDE_DEV_SLAVE			1

/************************************************************************
 *  Macro Definitions
*************************************************************************/

/************************************************************************
 *      Public variables
 ************************************************************************/

/************************************************************************
 *      Static variables
 ************************************************************************/

static char* ide_error_string[] = 
{
    /* ERROR_IDE_INVALID_COMMAND */  
    "Internal ERROR: Invalid control command",

    /* ERROR_IDE_UNKNOWN_DEVICE */  
    "Internal ERROR: Illegal minor number",

    /* ERROR_IDE_NULL_BUFFER */	
    "Internal ERROR: NULL buffer",

    /* ERROR_IDE_UNAVAILABLE_DEVICE */
    "Device unavailable",

    /* ERROR_IDE_BAD_BLOCK */
    "Bad block detected",

    /* ERROR_IDE_UNCORRECTABLE_DATA */
    "Uncorrectable data error",

    /* ERROR_IDE_MEDIA_CHANGE */
    "Media change",

    /* ERROR_IDE_ID_NOT_FOUND */
    "ID not found (damaged or non-existent sector)",

    /* ERROR_IDE_MEDIA_CHANGE_REQUESTED */
    "Media change requested",

    /* ERROR_IDE_ABORTED */
    "Aborted command (illegal command or disk drive error)",

    /* ERROR_IDE_TRACK_0_NOT_FOUND */
    "Track 0 not found",

    /* ERROR_IDE_ADDRESS_MARK_NOT_FOUND	*/
    "Address mark not found",

    /* ERROR_IDE_UNKNOWN */
    "Unknown IDE error"
};

static UINT32 pci_io_base;    /* Base access of PCI I/O range		*/
static UINT32 ide_io_base;    /* Offset of IDE device within I/O range	*/
static UINT32 sector;	      /* Last sector read (or failed)		*/
static bool   sector_valid;   /* 'sector' valid for error handling	*/
static char   diag_msg[40];   /* Diagnostics error message		*/

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

static UINT32
get_error( void );

/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          ide_init
 *  Description :
 *  -------------
 *
 *  This service initializes the IDE driver.
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
 ************************************************************************/
static INT32 
ide_init(
    UINT32 major,          /* IN: major device number             */
    UINT32 minor,          /* IN: minor device number             */
    void   *p_param )      /* INOUT: device parameter block       */
{
    /* Read base of PCI I/O range */
    return SYSCON_read(
        SYSCON_CORE_PCI_IO_OFFSET,
	(void *)&pci_io_base,
	sizeof(UINT32) );
}


/************************************************************************
 *
 *                          ide_ctrl
 *  Description :
 *  -------------
 *  This service comprises following specific IDE services:
 *
 *    1) IDE_CMD_READ
 *    2) IDE_CMD_WRITE
 *    3) IDE_CMD_IDENTIFY
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    variable of type, t_ide_ctrl_descriptor.
 *
 *  Return values :
 *  ---------------
 *
 *  OK (0x00):                   IDE service completed successfully
 *  Other :			 See error codes in ide_api.h
 *
 ************************************************************************/
static INT32 
ide_ctrl(
    UINT32                major,     /* IN: major device number		*/
    UINT32		  minor,     /* IN: minor device number		*/
    t_ide_ctrl_descriptor *p_param ) /* IN: write data			*/
{
    UINT32          i,t;
    UINT32	    sector_count;
    UINT8	    dev;
    UINT16	    buf_id[IDE_BYTES_PER_SECTOR/2]; /* 16 bit aligned  */
    t_ide_identify  *identify_struct;
    UINT8	    *buffer;
    UINT8	    status;
    UINT32	    data32;

    /* Sector number is not valid for error handling any more */
    sector_valid = FALSE;

    switch( minor )
    {
      case IDE_MINOR_PRIMARY_MASTER :
        ide_io_base = IDE_PRIMARY_IO_CMD_BASE;
        dev	    = IDE_DEV_MASTER;
	break;
      case IDE_MINOR_PRIMARY_SLAVE :
        ide_io_base = IDE_PRIMARY_IO_CMD_BASE;
        dev	    = IDE_DEV_SLAVE;
	break;
      case IDE_MINOR_SECONDARY_MASTER :
        ide_io_base = IDE_SECONDARY_IO_CMD_BASE;
        dev	    = IDE_DEV_MASTER;
	break;
      case IDE_MINOR_SECONDARY_SLAVE :
        ide_io_base = IDE_SECONDARY_IO_CMD_BASE;
        dev	    = IDE_DEV_SLAVE;
	break;
      default :
        return ERROR_IDE_UNKNOWN_DEVICE;
    }

    switch( p_param->command )
    {
      case IDE_CMD_READ  :
      case IDE_CMD_WRITE :
        sector          = p_param->u.sector.sector;
	sector_count    = p_param->u.sector.count;
	buffer          = p_param->u.sector.buffer;
	if( buffer == NULL ) 
	    return ERROR_IDE_NULL_BUFFER;
	break;
      case IDE_CMD_IDENTIFY : 
	sector_count    = 1;
        buffer          = (UINT8 *)buf_id;
	break;	
      default : 
	return ERROR_IDE_INVALID_COMMAND;
    }

    /**** Determine if device is available ****/

    /* Select device */
    IO8(IDE_DRIVE_HEAD_OFS) = (dev << IDE_DRIVE_HEAD_DEV_SHF) | 0xE0;

    /* Wait until BSY is cleared (may take up to 30 seconds, depending
     * on configuration).
     */
    while( IO8(IDE_STATUS_OFS) & IDE_STATUS_BSY_MSK )
        ;

    /* Devices present and functional should now have the DRDY bit set */
    if( ( IO8(IDE_STATUS_OFS) & (IDE_STATUS_DRDY_MSK | 
				 IDE_STATUS_DF_MSK) ) !=
	      IDE_STATUS_DRDY_BIT )
    {
        return ERROR_IDE_UNAVAILABLE_DEVICE;
    }

    /* Now perform the requested operation */
    while( sector_count-- )
    {
        if( (p_param->command != IDE_CMD_IDENTIFY) && 
	    (sector & 0xF0000000) )
	{
	    /* Bits 31:28 are not used for IDE */

            /* Sector number is now valid for error handling */
	    sector_valid = TRUE;
	    return ERROR_IDE_ID_NOT_FOUND;
	}

        /* Wait until device is ready */
        while( !(IO8(IDE_STATUS_OFS) & IDE_STATUS_DRDY_BIT) )
            ;

        if( p_param->command != IDE_CMD_IDENTIFY )
        {
	    /* Setup sector (LBA mode) */
	    IO8(IDE_SECTOR_COUNT_OFS)  = 1;
	    IO8(IDE_SECTOR_NUMBER_OFS) = (sector >> 0)  & 0xFF;
	    IO8(IDE_CYL_LOW_OFS)       = (sector >> 8)  & 0xFF;
	    IO8(IDE_CYL_HI_OFS)        = (sector >> 16) & 0xFF;
	    IO8(IDE_DRIVE_HEAD_OFS)    = ((sector >> 24) & 0x0F) | 
					 (dev << IDE_DRIVE_HEAD_DEV_SHF) | 
					 0xE0;
        }

        /* Issue command */
        IO8(IDE_COMMAND_OFS) = p_param->command;

        /*  Wait until device is ready to exchange data or
	 *  an error is detected 
	 */
	do
	{
	    status = IO8(IDE_STATUS_OFS);
	}
        while( ((status & IDE_STATUS_DRQ_BIT) == 0) &&
	       ((status & IDE_STATUS_ERR_BIT) == 0) )
	    ;

        /*  Note : The PIIX4 Southbridge will convert 32 bit accesses
         *  to the IDE_DATA to two 16 bit accesses to the device.
         */

        if( p_param->command != IDE_CMD_WRITE )
        {
            /* Check if read caused error */
            if( status & IDE_STATUS_ERR_BIT )
	        return get_error();

	    /* Read data from device sector buffer */
            for( i=0; i < IDE_BYTES_PER_SECTOR / 4; i++ )
	    {
	        data32 = IO32(IDE_DATA_OFS);

	        /* Byte oriented data */
	        buffer[0] = (data32 >> 0)  & 0xff;
	        buffer[1] = (data32 >> 8)  & 0xff;
	        buffer[2] = (data32 >> 16) & 0xff;
	        buffer[3] = (data32 >> 24) & 0xff;

#ifdef EB
		if( p_param->command == IDE_CMD_IDENTIFY )
		{
		    UINT8 tmp;

		    /* Identity data is 16 bit oriented */
		    tmp       = buffer[0];
		    buffer[0] = buffer[1];
	            buffer[1] = tmp;

		    tmp       = buffer[2];
		    buffer[2] = buffer[3];
	            buffer[3] = tmp;
		}
#endif

		buffer+= 4;
	    }
        }
        else
        {
            /* Write data to device sector buffer */
            for( i=0; i < IDE_BYTES_PER_SECTOR / 4; i++ )
	    {
	        data32 = (buffer[0] << 0)  |
		         (buffer[1] << 8)  |
			 (buffer[2] << 16) |
			 (buffer[3] << 24);

	        IO32(IDE_DATA_OFS) = data32;

		buffer+= 4;
	    }

	    /*  Wait until device is done.
	     *  This will be the case when DRQ and BSY are both cleared.
	     */
	    while( IO8(IDE_STATUS_OFS) & IDE_STATUS_DRQ_BIT )
	        ;
            while( IO8(IDE_STATUS_OFS) & IDE_STATUS_BSY_BIT )
	        ;

	    /* Check if write caused error */
            if( IO8(IDE_STATUS_OFS) & IDE_STATUS_ERR_BIT )
                return get_error();
        }

        sector++;
    }

    if( p_param->command == IDE_CMD_IDENTIFY )
    {
	identify_struct = &p_param->u.identify;

        /* Decode buffer */
        i = 0;

        identify_struct->config    =		          buf_id[i++];
	identify_struct->cyl_count =			  buf_id[i++];
        identify_struct->reserved0 =			  buf_id[i++];
        identify_struct->head_count=			  buf_id[i++];

	for( t=0; t<2; t++ )
	{
            identify_struct->obsolete0[t] =		  buf_id[i++];
	}        

	identify_struct->sectors_per_track =		  buf_id[i++];

	for( t=0; t<3; t++ )    
	{
	    identify_struct->vendor_specific0[t] =	  buf_id[i++];
	}

	for( t=0; t<10; t++ )
	{
	  identify_struct->serial_number[t*2]   =	  (UINT8)(buf_id[i] >> 8);
	  identify_struct->serial_number[t*2+1] =	  (UINT8)(buf_id[i++] & 0xFF);
	}

	for( t=0; t<2; t++ )
	{
	    identify_struct->obsolete1[t] =		  buf_id[i++];
	}

        identify_struct->ecc_count =			  buf_id[i++];

	for( t=0; t<4; t++ )
	{
	    identify_struct->firmware_rev[t*2] =	  (UINT8)(buf_id[i] >> 8);
	    identify_struct->firmware_rev[t*2+1] =	  (UINT8)(buf_id[i++] & 0xFF);
	}

	for( t=0; t<20; t++ )
	{
	    identify_struct->model_name[t*2] =		  (UINT8)(buf_id[i] >> 8);
	    identify_struct->model_name[t*2+1] =	  (UINT8)(buf_id[i++] & 0xFF);
	}

        identify_struct->sectors_per_interrupt =          buf_id[i++];
        identify_struct->reserved1 =			  buf_id[i++];
        identify_struct->capabilities =			  buf_id[i++];
        identify_struct->reserved2 =			  buf_id[i++];
        identify_struct->timing_mode_pio =		  buf_id[i++];
        identify_struct->timing_mode_dma =		  buf_id[i++];
        identify_struct->application =			  buf_id[i++];
        identify_struct->cyl_count_apparent =		  buf_id[i++];
        identify_struct->head_count_apparent =		  buf_id[i++];
        identify_struct->sectors_per_track_apparent =	  buf_id[i++];

        identify_struct->capacity_apparent =              (UINT32)buf_id[i] |
						          ((UINT32)buf_id[i+1] << 16);
        i+=2;
							  
        identify_struct->sectors_per_interrupt_apparent = buf_id[i++];

        identify_struct->lba_sector_count =		  (UINT32)buf_id[i] |
							  ((UINT32)buf_id[i+1] << 16);
        i+=2;
							  
        identify_struct->modes_single_dma =		  buf_id[i++];
        identify_struct->modes_multiple_dma =		  buf_id[i++];
        identify_struct->modes_adv_pio =		  buf_id[i++];
        identify_struct->min_cycle_time_dma =		  buf_id[i++];
        identify_struct->recommended_cycle_time_dma =	  buf_id[i++];
        identify_struct->minimum_cycle_time_pio =	  buf_id[i++];
        identify_struct->minimom_cycle_time_pio_iordy =	  buf_id[i++];

	for( t=0; t<59; t++ )
	{
            identify_struct->reserved3[59] =		  buf_id[i++];
	}

	for( t=0; t<32; t++ )
	{
            identify_struct->vendor_specific1[32] =	  buf_id[i++];
	}

	for( t=0; t<96; t++ )
	{
	    identify_struct->reserved4[96] =		  buf_id[i++];
	}
    }

    return OK;
}


/************************************************************************
 *                          get_error
 ************************************************************************/
static UINT32
get_error( void )
{
    UINT8 error = IO8(IDE_ERROR_OFS);

    /* Sector number is now valid for error handling */
    sector_valid = TRUE;

    if(      error & IDE_ERROR_BBK_BIT )
        return ERROR_IDE_BAD_BLOCK;
    else if( error & IDE_ERROR_UNC_BIT )
        return ERROR_IDE_UNCORRECTABLE_DATA;
    else if( error & IDE_ERROR_MC_BIT )
        return ERROR_IDE_MEDIA_CHANGE;
    else if( error & IDE_ERROR_IDNF_BIT )
        return ERROR_IDE_ID_NOT_FOUND;
    else if( error & IDE_ERROR_MCR_BIT )
        return ERROR_IDE_MEDIA_CHANGE_REQUESTED;
    else if( error & IDE_ERROR_ABRT_BIT )
        return ERROR_IDE_ABORTED;
    else if( error & IDE_ERROR_TK0NF_BIT )
        return ERROR_IDE_TRACK_0_NOT_FOUND;
    else if( error & IDE_ERROR_AMNF_BIT )
        return ERROR_IDE_ADDRESS_MARK_NOT_FOUND;
    else
        return ERROR_IDE_UNKNOWN;
}


/************************************************************************
 *                          ide_error_lookup
 ************************************************************************/
static INT32 
ide_error_lookup( 
    t_sys_error_string *p_param )
{
    UINT32 t = SYSERROR_ID( p_param->syserror );

    /* check for recognized error code */
    if( t < sizeof(ide_error_string)/sizeof(char*) )
    {
        /* fill in mandatory error message string */
        p_param->strings[SYSCON_ERRORMSG_IDX] = ide_error_string[t];

	if( sector_valid )
	{
	    sprintf( diag_msg, "Failing sector = 0x%08x", sector );
	    p_param->strings[SYSCON_DIAGMSG_IDX] = diag_msg;
	    p_param->count = 2;	
	}
	else
            p_param->count = 1;
    }
    else
        p_param->count = 0;
 
    return OK;
}


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


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
ide_install( void )
{
    t_sys_error_lookup_registration registration;

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_IDE );
    registration.lookup = ide_error_lookup;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration, 
                  sizeof( registration ) );

    /* Install device services */
    IO_install( SYS_MAJOR_IDE,                  /* major device number */
                    (t_io_service)ide_init,	/* 'init'  service     */
                    NULL,			/* 'open'  service  na */
                    NULL,			/* 'close' service  na */
                    NULL,			/* 'read'  service  na */
		    NULL,			/* 'write' service  na */
                    (t_io_service)ide_ctrl ) ;	/* 'ctrl'  service     */

    /* call our own 'init' service */
    return IO_init( SYS_MAJOR_IDE, 0, NULL);
}
