
/************************************************************************
 *
 *  disk.c
 *
 *  Monitor command for accessing IDE disks
 *
 *  disk [-f] (id [hda|hdb|hdc|hdd]                          ) |
 *            (read  hda|hdb|hdc|hdd <sector> <count> <addr> ) |
 *            (write hda|hdb|hdc|hdd <sector> <count> <addr> )
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
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <flash_api.h>
#include <io_api.h>
#include <ide_api.h>
#include <shell_api.h>
#include <shell.h>
#include <string.h>
#include <stdio.h>
#include <sysdev.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Number of sectors read/written for each '.' printed */
#define SECTORS_PER_DOT		100

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{
#define OPTION_FLUSH	0 
    { "f",  "Do not flush caches" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
get_options( 
    UINT32 argc,
    char   **argv,
    UINT32 *command,
    UINT32 *sector,
    UINT32 *count,
    UINT8  **addr,
    UINT32 *device,
    bool   *dev_all,
    bool   *flush );

static UINT32
do_access(
    t_ide_ctrl_descriptor *ide_ctrl,
    UINT8		  *addr,
    UINT32		  device,
    UINT32		  count );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/
 

/************************************************************************
 *                          disk
 ************************************************************************/
static MON_FUNC(disk)
{
    UINT32                 rc = OK;
    UINT32		   device, count;
    UINT8		   *addr;
    bool		   dev_all, flush;
    t_ide_ctrl_descriptor  ide_ctrl;
    
    rc = get_options( argc, 
		      argv, 
		      &ide_ctrl.command,
		      &ide_ctrl.u.sector.sector,
		      &count,
		      &addr,
      		      &device,
		      &dev_all,
		      &flush );

    if( rc == OK )
    {
        /* Possibly flush caches */
        if( flush )
            sys_flush_caches();

        if( dev_all )
	{
	    rc = shell_ide_display();
	}
	else
	{
            rc = do_access( &ide_ctrl, addr, device, count );

            /* Possibly flush caches */
            if( (rc == OK) && flush )
                sys_flush_caches();
        }
    }

    return rc;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options( 
    UINT32 argc,
    char   **argv,
    UINT32 *command,
    UINT32 *sector,
    UINT32 *count,
    UINT8  **addr,
    UINT32 *device,
    bool   *dev_all,
    bool   *flush )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    UINT32	   int_count = 0;
    bool	   dev_valid = FALSE;
    bool	   cmd_valid = FALSE;
    UINT32	   rc	     = OK;

    /* Defaults */
    *flush = TRUE;

    for( arg = 1; 
	          (rc == OK) && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    if( strcmp(decode.option, options[OPTION_FLUSH].option) == 0 )
	    {
	        *flush = FALSE;
	    }
	    else
	    {
	        rc 	         = SHELL_ERROR_OPTION;
	        shell_error_data = argv[arg];
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( int_count == 3 )
	        rc = SHELL_ERROR_SYNTAX;
	    else
	    {
	        switch( int_count )
	        {
	          case 0 : *sector = decode.number;	     break;
	          case 1 : *count  = decode.number;	     break;
	          case 2 : *addr   = (UINT8 *)decode.number; break;
	        }

	        int_count++;
	    }
	    break;
	  case SHELL_TOKEN_STRING :
	    if(      strcmp( decode.string, "hda" ) == 0 )
	    {
	        *device   = IDE_MINOR_PRIMARY_MASTER;
		dev_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "hdb" ) == 0 )
	    {
	        *device   = IDE_MINOR_PRIMARY_SLAVE;
		dev_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "hdc" ) == 0 )
	    {	    
	        *device   = IDE_MINOR_SECONDARY_MASTER;
		dev_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "hdd" ) == 0 )
	    {
	        *device   = IDE_MINOR_SECONDARY_SLAVE;
		dev_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "id" ) == 0 )
	    {
	        *command  = IDE_CMD_IDENTIFY;
		cmd_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "read" ) == 0 )
	    {
	        *command  = IDE_CMD_READ;
		cmd_valid = TRUE;
	    }
	    else if( strcmp( decode.string, "write" ) == 0 )
	    {
	        *command  = IDE_CMD_WRITE;
		cmd_valid = TRUE;
	    }
	    else 
	        rc = SHELL_ERROR_SYNTAX;
	    break;
	  default :
            rc = SHELL_ERROR_SYNTAX;
	    break;
        }
    }

    if( rc != OK )
        return rc;

    if( !cmd_valid )
        return SHELL_ERROR_SYNTAX;
   
    switch( *command )
    {
      case IDE_CMD_IDENTIFY :
	*flush   = FALSE;
	*dev_all = !dev_valid;

	if( int_count != 0 ) 
	    return SHELL_ERROR_SYNTAX;
	break;
      case IDE_CMD_WRITE : 
	*flush   = FALSE;
	/* Fallthrough !!! */
      case IDE_CMD_READ :
	*dev_all = FALSE;

	if( (int_count != 3) || !dev_valid ) 
	    return SHELL_ERROR_SYNTAX;
	else
            return sys_validate_range(
	                (UINT32)(*addr),
			(*count) * IDE_BYTES_PER_SECTOR,
			sizeof(UINT8),
			(*command == IDE_CMD_WRITE) );
    }

    return OK;
}


/************************************************************************
 *                          do_access
 ************************************************************************/
static UINT32
do_access(
    t_ide_ctrl_descriptor *ide_ctrl,
    UINT8		  *addr,
    UINT32		  device,
    UINT32		  count )
{
    UINT32		      rc;
    char		      *s;
    char		      msg[100];
    UINT32		      len;
    UINT32		      size_mb;
    t_FLASH_write_descriptor  flash_write;
    UINT8		      buffer_tmp[IDE_BYTES_PER_SECTOR];
    UINT32		      count_dot_freq = 0;
    UINT32		      dot_count = 0;
    bool		      print_dot;

    if( ide_ctrl->command == IDE_CMD_IDENTIFY )
    {
        rc = IO_ctrl( SYS_MAJOR_IDE, device, ide_ctrl );

        if( rc != OK )
	    return rc;

	/* Logical name */
        switch( device )
	{
          case IDE_MINOR_PRIMARY_MASTER   : s = "hda"; break;
          case IDE_MINOR_PRIMARY_SLAVE    : s = "hdb"; break;
          case IDE_MINOR_SECONDARY_MASTER : s = "hdc"; break;
	  case IDE_MINOR_SECONDARY_SLAVE  : s = "hdd"; break;
	}
	strcpy( msg, s ); 
	strcat( msg, ": " );

	/* Model name */
	len = strlen( msg );
	strncpy( &msg[len],
		 ide_ctrl->u.identify.model_name, 
		 IDE_MODEL_NAME_LEN );
	/* Remove spaces */
	len += IDE_MODEL_NAME_LEN - 1;
	while( msg[len] == ' ' )
	    len--;
	msg[len+1] = '\0';
	
	/* LBA sector count */
	len = strlen( msg );
        sprintf( &msg[len], 
		 ", LBA sectors = 0x%08x ", 
                 ide_ctrl->u.identify.lba_sector_count );

	/*  Size in MByte ("decimal" MB/GB)
	 *
	 *  Number of MByte = sector count / 10^6 * bytes per sector
	 *
	 *  We divide by 10^6 first in order to avoid overflow. This
	 *  however causes rounding down error, so we adjust for the
	 *  remainder from the division.
	 */
        size_mb =  ide_ctrl->u.identify.lba_sector_count / 1000000 *
		   IDE_BYTES_PER_SECTOR;
	
	size_mb += ide_ctrl->u.identify.lba_sector_count % 1000000 *
		   IDE_BYTES_PER_SECTOR /
		   1000000;

	len = strlen( msg );

        if( size_mb < 1000 )
            sprintf( &msg[len], "(%d MB)", (UINT32)size_mb );
        else
            sprintf( &msg[len], "(%d GB)", (UINT32)(size_mb / 1000) );

	/* Print string */
	strcat( msg, "\n" );
	if( SHELL_PUTS( msg ) ) return SHELL_ERROR_CONTROL_C_DETECTED;
    }
    else
    {
        /* Read/write */

	if( ide_ctrl->command == IDE_CMD_READ )
	{
	    /* Read (one sector at a time) */
            ide_ctrl->u.sector.buffer = buffer_tmp;
	    ide_ctrl->u.sector.count  = 1;

	    flash_write.adr	      = (UINT32)addr;
            flash_write.length	      = IDE_BYTES_PER_SECTOR;
            flash_write.buffer	      = buffer_tmp;
	}
	else
	{
	    /* Write (multiple sectors at a time) */
            ide_ctrl->u.sector.buffer = addr;
	    ide_ctrl->u.sector.count  = MIN(count, SECTORS_PER_DOT);
	    print_dot		      = (count > SECTORS_PER_DOT);
	}

	do
	{
            rc = IO_ctrl( SYS_MAJOR_IDE, device, ide_ctrl );

	    if( rc == OK )
	    {
	        count		          -= ide_ctrl->u.sector.count;
	        ide_ctrl->u.sector.sector += ide_ctrl->u.sector.count;

	        if( ide_ctrl->command == IDE_CMD_READ )
		{
		    /* This flash copy function can also handle RAM */
		    rc = IO_write( SYS_MAJOR_FLASH_STRATA, 0, &flash_write );

		    flash_write.adr += IDE_BYTES_PER_SECTOR;

		    print_dot = (++count_dot_freq % SECTORS_PER_DOT == 0);
		}
		else
		{
	            ide_ctrl->u.sector.buffer += ide_ctrl->u.sector.count *
					         IDE_BYTES_PER_SECTOR;
	            ide_ctrl->u.sector.count   = MIN(count, SECTORS_PER_DOT);

		    if( count == 0 ) 
		        print_dot = FALSE;
		}

                if( (rc == OK) && print_dot && shell_print_dot(&dot_count) )
		{
                    rc = SHELL_ERROR_CONTROL_C_DETECTED;
		}
	    }
        }
	while( count && (rc == OK) );
    }

    if( dot_count )
        printf( "\n" );

    return rc;
}


/* Command definition for disk */
static t_cmd cmd_def =
{
    "disk",
    disk,

    "disk [-f] (id [hda|hdb|hdc|hdd]                          ) |\n"
    "          (read  hda|hdb|hdc|hdd <sector> <count> <addr> ) |\n"
    "          (write hda|hdb|hdc|hdd <sector> <count> <addr> )",

    "Command for copying data to/from IDE harddisk or compact flash module.\n"
    "\n"
    "The disks are named the following way :\n"
    "\n"
    "Primary master   : hda\n"
    "Primary slave    : hdb\n"
    "Secondary master : hdc\n"
    "Secondary slave  : hdd\n"
    "\n"
    "When a single device is attached to an interface, it is recommended\n"
    "to set it as master. Otherwise, the device may not be detected\n"
    "immediately following a reset. For example, a 'disk id' command may\n"
    "not detect the device if executed directly following a reset, but\n"
    "will detect the device after a few seconds.\n"
    "\n"
    "Depending on the configuration, a command executed directly following\n"
    "a reset may take up to 30 seconds to complete (not including the time\n"
    "required for reading/writing data).\n"
    "\n"
    "Only LBA addressing is supported.\n"
    "\n"
    "Description :\n"
    "\n"
    "'disk id'    Lists disk parameters (ID, size) for all disks available\n"
    "             or the particular one (hda/hdb/hdc/hdd) requested.\n"
    "\n"
    "'disk read'  Reads <count> sectors starting at <sector>.\n"
    "             Data is written to <addr>.\n"
    "\n"
    "'disk write' Writes <count> sectors starting at <sector>.\n"
    "             Data is read from <addr>.\n"
    "\n"
    "If a read operation is performed, and <addr> is flash, the destination\n"
    "area must be cleared using the 'erase' command prior to the disk\n"
    "operation.\n"
    "\n"
    "Unless the -f option is applied, caches are flushed before and after\n"
    "a read operation (D-cache writeback and invalidate, I-cache invalidate).\n",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_disk_init
 *  Description :
 *  -------------
 *
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
t_cmd *
shell_disk_init( void )
{
    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_ide_display
 *  Description :
 *  -------------
 *
 *  Display IDE configuration
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
UINT32
shell_ide_display(void)
{
    t_ide_ctrl_descriptor ide_ctrl;
    UINT32		  rc;

    ide_ctrl.command = IDE_CMD_IDENTIFY;
  
    /* Ignore error codes (except ctrl-c) */

    rc = do_access( &ide_ctrl, NULL, IDE_MINOR_PRIMARY_MASTER,   0 );
    if( rc == SHELL_ERROR_CONTROL_C_DETECTED ) return rc;

    rc = do_access( &ide_ctrl, NULL, IDE_MINOR_PRIMARY_SLAVE,    0 );
    if( rc == SHELL_ERROR_CONTROL_C_DETECTED ) return rc;

    rc = do_access( &ide_ctrl, NULL, IDE_MINOR_SECONDARY_MASTER, 0 );
    if( rc == SHELL_ERROR_CONTROL_C_DETECTED ) return rc;

    rc = do_access( &ide_ctrl, NULL, IDE_MINOR_SECONDARY_SLAVE,  0 );
    if( rc == SHELL_ERROR_CONTROL_C_DETECTED ) return rc;

    return OK;
}
