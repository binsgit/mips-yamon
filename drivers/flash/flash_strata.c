/************************************************************************
 *
 *      FLASH_STRATA.C
 *
 *      The 'FLASH_STRATA' module implements the FLASH device driver
 *      interface to be used via 'IO' device driver services:
 *
 *        1) init  device:  configure and initialize FLASH driver
 *        2) open  device:  not used
 *        3) close device:  not used
 *        4) read  device:  not used
 *        5) write device:  write FLASH device
 *        6) ctrl  device:  a) ERASE_SYSTEMFLASH
 *                          b) ERASE_FILEFLASH
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
#include <syserror.h>
#include <sysdev.h>
#include <mips.h>
#include <io_api.h>
#include <syscon_api.h>
#include <flash_api.h>
#include <flash_strata_api.h>
#include <sys_api.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

/* FLASH-device, relative register address inside a block */
#define FLASH_BLOCKSTATUS_OFS                   0x0008

/* field: LOCK */
#define FLASH_BLOCKSTATUS_LOCK_MSK          0x00010001

/* flash memory ID's */
#define INTEL_MNFCR_CODE			0x0089
#define STRATA_DEV1_CODE			0x0014
#define STRATA_DEV2_CODE			0x0015

#define FLASH_QUERYQCHAR_OFS                    0x0040
#define FLASH_QUERYRCHAR_OFS                    0x0044
#define FLASH_QUERYYCHAR_OFS                    0x0048

#define FLASH_QUERYQCHAR                    0x00510051
#define FLASH_QUERYRCHAR                    0x00520052
#define FLASH_QUERYYCHAR                    0x00590059

/* status */
#define FLASH_READY				0x0000
#define FLASH_NOT_DETECTED			0x0001
#define FLASH_BUSY				0x0002
#define FLASH_ERROR				0x0003
#define FLASH_TIMEOUT                           0x0004
#define FLASH_INVALID_SECTOR                    0x0005
#define FLASH_SECTOR_LOCKED                     0x0006

/* Commands and masks, etc... */
#define FLASH_STATUS_READY			0x00800080
#define FLASH_STATUS_MASK                       0x00FF00FF
#define FLASH_STATUS_OK                         0x00800080
#define FLASH_STATUS_ERASE                      0x00600060
#define FLASH_STATUS_LOCK                       0x00020002
#define FLASH_STATUS_LOW_VOLTAGE                0x00080008

#define FLASH_READ_COMMAND			0x00FF00FF
#define FLASH_ERASE_COMMAND			0x00200020
#define FLASH_CONFIRM_COMMAND		        0x00D000D0
#define FLASH_CLEAR_STATUS_COMMAND	        0x00500050
#define FLASH_WRITE_WORD_COMMAND	        0x00100010
#define FLASH_WRITE_BUFFER_COMMAND	        0x00E800E8
#define FLASH_STATUS_COMMAND		        0x00700070
#define FLASH_QUERY_COMMAND			0x00980098
#define FLASH_READ_ID_CODES_COMMAND		0x00900090
#define FLASH_CLEAR_LOCK_COMMAND                0x00600060
#define FLASH_LOCK_SECTOR                       0x00010001

/* Timeout values */
#define FLASH_TMOUT_100MS                       100
#define FLASH_TMOUT_250MS                       250
#define FLASH_TMOUT_500MS                       500
#define FLASH_TMOUT_1SEC                        1000
#define FLASH_TMOUT_2SEC                        2000
#define FLASH_TMOUT_2_5SEC                      2500

/* Retry counts */
#define FLASH_MAX_LOOPS				0xFFFFFFFF
#define FLASH_RETRY_5                           5
#define FLASH_RETRY_10                          10
#define FLASH_RETRY_20                          20
#define FLASH_RETRY_50                          50

/* Identification tags for memory devices */
typedef enum flash_device_id
{
	FLASH_SYSTEMFLASH_DEVICE = 0,
	FLASH_MONITORFLASH_DEVICE,
	FLASH_FILEFLASH_DEVICE,
	FLASH_BOOT_DEVICE,
	FLASH_UNKNOWN_DEVICE
} t_flash_device_id ;

/* union, to access bytes inside a word, independant of endianness */
typedef union flash_access
{
	volatile UINT32 d32 ;
	volatile UINT8  d8[4] ;
} t_flash_access ;

/************************************************************************
 *  Macro Definitions
*************************************************************************/

#define WRITE_ENABLE   SYSCON_write( SYSCON_BOARD_SYSTEMFLASH_WRITE_ENABLE_ID, \
                                    NULL, sizeof( UINT32 ) );

#define WRITE_DISABLE  SYSCON_write( SYSCON_BOARD_SYSTEMFLASH_WRITE_DISABLE_ID, \
                                    NULL, sizeof( UINT32 ) );

/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/

static char* flash_error_string[] = 
{
    /* ERROR_FLASH_PROGRAM_ERROR      */ "Flash device failure",
    /* ERROR_FLASH_INVALID_ADDRESS    */ "Invalid address",
    /* ERROR_FLASH_INVALID_COMMAND    */ "Internal ERROR: Invalid control command",
    /* ERROR_FLASH_TIME_OUT           */ "Internal ERROR: Flash device timed out during operation",
    /* ERROR_FLASH_VERIFY_ERROR       */ "Data verify error",
    /* ERROR_FLASH_LOCKED             */ "Some sectors are locked",
    /* ERROR_FLASH_ERASE_ERROR        */ "Sector has erase error",
    /* ERROR_FLASH_LOW_VOLTAGE        */ "Low programming voltage detected",
    /* ERROR_FLASH_WRITE_PROTECTED    */ "Flash is write protected",
    /* ERROR_FLASH_FILE_FLASH_PROT    */ "Environment FLASH is write protected",
    /* ERROR_FLASH_FILE_FLASH_LOCK    */ "Environment FLASH is lock-bit protected",
    /* ERROR_FLASH_MONITOR_FLASH_LOCK */ "Some MONITOR FLASH sector(s) locked",
    /* ERROR_FLASH_QRY_NOT_FOUND      */ "CFI Query-ID string of FLASH not found",
    /* ERROR_FLASH_BOOT_WRITE_PROTECTED */ "Write access to this area not allowed"
} ;

static char* flash_error_hint_string[] = 
{
    /* ERROR_FLASH_PROGRAM_ERROR      */ NULL,
    /* ERROR_FLASH_INVALID_ADDRESS    */ NULL,
    /* ERROR_FLASH_INVALID_COMMAND    */ NULL,
    /* ERROR_FLASH_TIME_OUT           */ NULL,
    /* ERROR_FLASH_VERIFY_ERROR       */ "Check flash has been erased before programming",
    /* ERROR_FLASH_LOCKED             */ "Unlock sector(s) before programming",
    /* ERROR_FLASH_ERASE_ERROR        */ NULL,
    /* ERROR_FLASH_LOW_VOLTAGE        */ NULL,
    /* ERROR_FLASH_WRITE_PROTECTED    */ "Disable write protection: Switch S1-3",
    /* ERROR_FLASH_FILE_FLASH_PROT    */ "Check programming addresses",
    /* ERROR_FLASH_FILE_FLASH_LOCK    */ "Disable 'clear lock-bit' protection: (MFWR-jumper) must be fitted",
    /* ERROR_FLASH_MONITOR_FLASH_LOCK */ NULL,
    /* ERROR_FLASH_QRY_NOT_FOUND      */ NULL,
    /* ERROR_FLASH_BOOT_WRITE_PROTECTED */ NULL
} ;

static UINT32 flash_last_error ;
static char   flash_diag_msg[160] ;


/* these variables are initialized at 'init' with the physical
   address boundaries of:

     a) system FLASH
     b) monitor FLASH
     c) file FLASH.

   Following rule is to apply:
     'start' <= physical-device-address-space < 'end'
*/

static UINT32  systemflash_phys_start ;
static UINT32  systemflash_phys_end ;
static UINT32  systemflash_block_size ;
static UINT32  systemflash_bank_count ;
static UINT32  systemflash_block_count ;

static UINT32  monitorflash_phys_start ;
static UINT32  monitorflash_phys_end ;
static UINT32  monitorflash_block_size ;

static UINT32  fileflash_phys_start ;
static UINT32  fileflash_phys_end ;
static UINT32  fileflash_block_size ;

static UINT32  boot_phys_start ;
static UINT32  boot_phys_end ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          FLASH_STRATA_init
 *  Description :
 *  -------------
 * This service initializes the FLASH driver to handle all
 * FLASH devices on this board environment.
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
INT32 FLASH_STRATA_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param ) ;    /* INOUT: device parameter block       */



/************************************************************************
 *
 *                          FLASH_STRATA_write
 *  Description :
 *  -------------
 *  This service writes data into a specified address location, which
 *  can be in either SYSTEM-FLASH or FILE-FLASH space. Default is RAM.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,        Physical address not impl.    
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_FLASH_write_descriptor *p_param ) ; /* IN: write data       */


/************************************************************************
 *
 *                          FLASH_STRATA_ctrl
 *  Description :
 *  -------------
 *  This service comprise following specific FLASH services:
 *    1) 'ERASE_SYSTEMFLASH'
 *    2) 'ERASE_FILEFLASH'
 *    3) 'FLASH_CTRL_ERASE_FLASH_AREA'
 *    4) 'FLASH_CTRL_INQUIRE_FLASH_AREA'
 *    5) 'FLASH_CTRL_TEST_SYSTEMFLASH'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH service completed successfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_FLASH_ctrl_descriptor *p_param ) ;  /* IN: write data       */


/************************************************************************
 *
 *                          FLASH_STRATA_is_system_flash_write_protected
 *  Description :
 *  -------------
 *  Check if system flash id write protected
 *  
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_system_flash_write_protected( void ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_is_file_flash_write_protected
 *  Description :
 *  -------------
 *  Check if file flash id write protected
 *  
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_file_flash_write_protected( void ) ;

/************************************************************************
 *
 *                          FLASH_STRATA_is_monitor_flash_write_protected
 *  Description :
 *  -------------
 *  Check if file flash id write protected
 *
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_monitor_flash_write_protected( void ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_devicetype
 *  Description :
 *  -------------
 *  Derive the memory device type from 'physical address'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'physadr': physical address inside a 512 MByte space
 *
 *
 *  Return values :
 *  ---------------
 *
 *  t_flash_device_id:    memory device type detected.
 *
 *
 ************************************************************************/
static
t_flash_device_id FLASH_STRATA_devicetype( UINT32 physadr ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_program_systemram
 *  Description :
 *  -------------
 *  Programs (store data) in the system RAM device.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:             Data stored in system RAM
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_program_systemram( t_FLASH_write_descriptor *p_param ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_program_flash
 *  Description :
 *  -------------
 *  Programs a Intel 28Fxxx-compliant flash device.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH programmed succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_program_flash( t_FLASH_write_descriptor *p_param ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_set_systemflash_read
 *  Description :
 *  -------------
 *  Set system flash device in read mode.
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
 * 'OK' = 0x00:                         System FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_systemflash_read( void ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_set_fileflash_read
 *  Description :
 *  -------------
 *  Set file flash device in read mode.
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
 * 'OK' = 0x00:                         File FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_fileflash_read( void ) ;

/************************************************************************
 *
 *                          FLASH_STRATA_set_monitorflash_read
 *  Description :
 *  -------------
 *  Set monitor flash device in read mode.
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
 * 'OK' = 0x00:                         File FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_monitorflash_read( void ) ;

/************************************************************************
 *
 *                          FLASH_STRATA_erase_fileflash
 *  Description :
 *  -------------
 *  Erase file flash, which is the last block (in each FLASH device)
 *  of the monitor flash.
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
 * 'OK' = 0x00:                         File FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_erase_fileflash( void ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_erase_flasharea
 *  Description :
 *  -------------
 *  Erase flash area; i.e. the driver erases the flash blocks inside 
 *  the specified memory area.
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS         Address area not inside FLASH
 *                                      devices
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_erase_flasharea( t_FLASH_ctrl_descriptor *p_param ) ;

 
/************************************************************************
 *
 *                          FLASH_STRATA_inquire_flasharea
 *  Description :
 *  -------------
 *  Inquire flash area; i.e. the driver calculates the effective flash
 *  block area covering the specified memory area.
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         Area specified inside valid
 *                                      FLASH device area
 *  ERROR_FLASH_INVALID_ADDRESS         Address area not inside FLASH
 *                                      devices
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_inquire_flasharea( t_FLASH_ctrl_descriptor *p_param ) ;

 
/************************************************************************
 *
 *                          FLASH_STRATA_test_systemflash
 *  Description :
 *  -------------
 *  Test complete system flash.
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
 * 'OK' = 0x00:                         System FLASH OK
 *  ERROR_FLASH_LOCKED                  Some sector(s) locked
 *  ERROR_FLASH_ERASE_ERROR             Some sector(s) have erase error
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_test_systemflash( void ) ;


/************************************************************************
 *
 *                          FLASH_STRATA_test_fileflash
 *  Description :
 *  -------------
 *  Test complete file flash (just one block)
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
 * 'OK' = 0x00:                         System FLASH OK
 *  ERROR_FLASH_LOCKED                  Some sector(s) locked
 *  ERROR_FLASH_ERASE_ERROR             Some sector(s) have erase error
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_test_fileflash( void ) ;


static
INT32 FLASH_STRATA_test_monitorflash( void ) ;

/************************************************************************
 *
 *                          FLASH_STRATA_wait_ready
 *  Description :
 *  -------------
 *  
 *  Await FLASH operation completes.
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
 * 'OK' = 0x00:                         File FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_wait_ready(UINT32 pw, UINT32 retry ) ;



/************************************************************************
 *
 *                          FLASH_STRATA_error_lookup
 *  Description :
 *  -------------
 *  Lookup error code to error string(s)
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   INOUT,    variable of type, t_sys_error_string.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:  
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_error_lookup( t_sys_error_string *p_param ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          FLASH_STRATA_install
 *  Description :
 *  -------------
 *
 *  Installs the STRATA FLASH device drivers services in 
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
INT32 FLASH_STRATA_install( void )
{
    t_sys_error_lookup_registration registration ;

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_FLASH ) ;
    registration.lookup = FLASH_STRATA_error_lookup ;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration, 
                  sizeof( registration ) );

    /* pre-initialize local variables and install device services */
    IO_install( SYS_MAJOR_FLASH_STRATA,         /* major device number */
           (t_io_service) FLASH_STRATA_init,    /* 'init'  service     */
                          NULL,                 /* 'open'  service  na */
                          NULL,                 /* 'close' service  na */
                          NULL,                 /* 'read'  service     */
           (t_io_service) FLASH_STRATA_write,   /* 'write' service     */
           (t_io_service) FLASH_STRATA_ctrl ) ; /* 'ctrl'  service     */

    /* call our own 'init' service */
    return IO_init( SYS_MAJOR_FLASH_STRATA, 0, NULL);
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          FLASH_STRATA_init
 *  Description :
 *  -------------
 *  This service initializes the FLASH_STRATA driver.
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
INT32 FLASH_STRATA_init(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          void   *p_param )      /* INOUT: device parameter block       */
{
    int rcode ;


    /* initialize system profile: FLASH, address boundaries */

    /* MIPS BOOT: */
    boot_phys_start = MIPS_RESET_VECTOR ;
    boot_phys_end   = 0x20000000 ; /* TBD: This size may change for future
                                           board support packages       */

    /* SYSTEM FLASH: */
    /* get base for system flash  */
    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BASE_ID,
                         &systemflash_phys_start,
                         sizeof(systemflash_phys_start) ) ;
    if (rcode != OK)
    {
        systemflash_phys_start = 0 ;
    }
    systemflash_phys_start = PHYS(systemflash_phys_start) ;

    /* get size for system flash  */
    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,
                         &systemflash_phys_end,
                         sizeof(systemflash_phys_end) ) ;
    if (rcode != OK)
    {
        systemflash_phys_end = 0 ;
    }
    systemflash_phys_end = systemflash_phys_start + systemflash_phys_end ;


    /* MONITOR FLASH: */
    /* get base for monitor flash  */
    rcode = SYSCON_read( SYSCON_BOARD_MONITORFLASH_BASE_ID,
                         &monitorflash_phys_start,
                         sizeof(monitorflash_phys_start) ) ;
    if (rcode != OK)
    {
        monitorflash_phys_start = 0 ;
    }
    monitorflash_phys_start = PHYS(monitorflash_phys_start) ;

    /* get size for monitor flash  */
    rcode = SYSCON_read( SYSCON_BOARD_MONITORFLASH_SIZE_ID,
                         &monitorflash_phys_end,
                         sizeof(monitorflash_phys_end) ) ;
    if (rcode != OK)
    {
        monitorflash_phys_end = 0 ;
    }
    monitorflash_phys_end = monitorflash_phys_start + monitorflash_phys_end ;

    /* FILE FLASH: */
    /* get base for file flash  */
    rcode = SYSCON_read( SYSCON_BOARD_FILEFLASH_BASE_ID,
                         &fileflash_phys_start,
                         sizeof(fileflash_phys_start) ) ;
    if (rcode != OK)
    {
        fileflash_phys_start = 0 ;
    }
    fileflash_phys_start = PHYS(fileflash_phys_start) ;

    /* get size for file flash  */
    rcode = SYSCON_read( SYSCON_BOARD_FILEFLASH_SIZE_ID,
                         &fileflash_phys_end,
                         sizeof(fileflash_phys_end) ) ;
    if (rcode != OK)
    {
        fileflash_phys_end = 0 ;
    }
    fileflash_phys_end = fileflash_phys_start + fileflash_phys_end ;

    if (p_param != NULL) 
    {
        /*  Hack that may be used e.g. for test of monitor flash.
	 *  Not used by YAMON.
	 */
        fileflash_phys_start = monitorflash_phys_start;
        monitorflash_phys_start = 0xffffffff;
    }

    /* get system flash sector size */
    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SECTORSIZE_ID,
                         &systemflash_block_size,
                         sizeof(systemflash_block_size) ) ;
    systemflash_block_size = 2 * systemflash_block_size ;
    if (rcode != OK)
    {
        systemflash_block_size = 0 ;
    }

    /* get monitor flash sector size */
    rcode = SYSCON_read( SYSCON_BOARD_MONITORFLASH_SECTORSIZE_ID,
                         &monitorflash_block_size,
                         sizeof(monitorflash_block_size) ) ;
    monitorflash_block_size = 2 * monitorflash_block_size ;
    if (rcode != OK)
    {
        monitorflash_block_size = 0 ;
    }

    /* get file flash sector size */
    rcode = SYSCON_read( SYSCON_BOARD_FILEFLASH_SECTORSIZE_ID,
                         &fileflash_block_size,
                         sizeof(fileflash_block_size) ) ;
    fileflash_block_size = 2 * fileflash_block_size ;
    if (rcode != OK)
    {
        fileflash_block_size = 0 ;
    }

    /* get system flash bank count */
    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BANKCOUNT_ID,
                         &systemflash_bank_count,
                         sizeof(systemflash_bank_count) ) ;
    if (rcode != OK)
    {
        systemflash_bank_count = 0 ;
    }

    /* get system flash block count */
    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BLOCKCOUNT_ID,
                         &systemflash_block_count,
                         sizeof(systemflash_block_count) ) ;
    if (rcode != OK)
    {
        systemflash_block_count = 0 ;
    }
    return( OK ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_write
 *  Description :
 *  -------------
 *  This service writes data into a specified address location, which
 *  can be in either SYSTEM-FLASH or FILE-FLASH space. Default is RAM.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         data has been stored
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS,        Physical address not impl.    
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_write(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_FLASH_write_descriptor *p_param )   /* IN: write data       */
{
    int rcode = OK ;

    /* set to default */
    flash_diag_msg[0] = 0 ;

    switch (FLASH_STRATA_devicetype(PHYS(p_param->adr)))
    {
        case FLASH_SYSTEMFLASH_DEVICE:
            /* program system FLASH */
            if ( FLASH_STRATA_is_system_flash_write_protected() )
            {
                rcode = ERROR_FLASH_WRITE_PROTECTED ;
            }
            else
            {
                WRITE_ENABLE ;
                rcode = FLASH_STRATA_program_flash( p_param ) ;

                /* set system flash banks in read array mode */
                FLASH_STRATA_set_systemflash_read();   
                WRITE_DISABLE ;
            }
          break ;

        case FLASH_FILEFLASH_DEVICE:
            /* Reject user to program FILE FLASH */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "Environment FLASH: map = (0x%08x,0x%08x), user = 0x%08x", 
                     (UINT32) fileflash_phys_start, 
                     (UINT32) fileflash_phys_end, 
                     (UINT32) PHYS(p_param->adr) ) ; 
            rcode = ERROR_FLASH_FILE_FLASH_PROT ;
          break ;

        case FLASH_MONITORFLASH_DEVICE:
            /* program monitor FLASH */
            rcode = FLASH_STRATA_program_flash( p_param ) ;

            /* set monitor flash in read array mode */
            FLASH_STRATA_set_monitorflash_read();   
          break ;

        case FLASH_BOOT_DEVICE:
            /* Boot device */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "MIPS boot media: map = (0x%08x,0x%08x), user = 0x%08x",
                     (UINT32) boot_phys_start,
                     (UINT32) boot_phys_end,
                     (UINT32) PHYS(p_param->adr) ) ;
            rcode = ERROR_FLASH_BOOT_WRITE_PROTECTED ;
          break ;

        default:
            /* program any RAM */
            rcode = FLASH_STRATA_program_systemram( p_param ) ;
          break ;
    }
    flash_last_error = rcode ;
    return( rcode ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_ctrl
 *  Description :
 *  -------------
 *  This service comprise following specific FLASH services:
 *    1) 'ERASE_SYSTEMFLASH'
 *    2) 'ERASE_FILEFLASH'
 *    3) 'FLASH_CTRL_ERASE_FLASH_AREA'
 *    4) 'FLASH_CTRL_INQUIRE_FLASH_AREA'
 *    5) 'FLASH_CTRL_TEST_SYSTEMFLASH'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH service completed successfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_ctrl(
          UINT32 major,          /* IN: major device number             */
          UINT32 minor,          /* IN: minor device number             */
          t_FLASH_ctrl_descriptor *p_param )    /* IN: write data       */
{
    int    rcode = OK ;

    /* set to default */
    flash_diag_msg[0] = 0 ;

    switch(p_param->command)
    {
        case FLASH_CTRL_ERASE_SYSTEMFLASH: 
            {
                rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BASE_ID,
                                     (void *)&(p_param->user_physadr),
                                     sizeof(UINT32) ) ;
                if (rcode == OK)
                {
                    rcode = SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,
                                         (void *)&(p_param->user_length),
                                         sizeof(UINT32) ) ;
                }
                if (rcode == OK)
                {
                    rcode = FLASH_STRATA_erase_flasharea(p_param) ;
                }
            }
          break ;
 
        case FLASH_CTRL_ERASE_FILEFLASH:
            rcode = FLASH_STRATA_erase_fileflash() ;
          break ;

        case FLASH_CTRL_ERASE_FLASH_AREA:
            rcode = FLASH_STRATA_erase_flasharea(p_param) ;
          break ;

        case FLASH_CTRL_INQUIRE_FLASH_AREA:
            rcode = FLASH_STRATA_inquire_flasharea(p_param) ;
          break ;

        case FLASH_CTRL_TEST_SYSTEMFLASH:
            rcode = FLASH_STRATA_test_systemflash() ;
            if (rcode == OK)
            {
                rcode = FLASH_STRATA_test_fileflash() ;
            }
          break ;

        case FLASH_CTRL_WRITE_FILEFLASH:
            /* program file FLASH */
            rcode = FLASH_STRATA_program_flash( p_param->wr_param ) ;
            if (rcode ==  ERROR_FLASH_LOCKED)
            {
                if ( FLASH_STRATA_is_file_flash_write_protected() )
                {
                    rcode = ERROR_FLASH_FILE_FLASH_LOCK ;
                }
            }


            /* set file flash in read array mode */
            FLASH_STRATA_set_fileflash_read();
          break ;

        case FLASH_CTRL_TEST_MONITORFLASH:
            rcode = FLASH_STRATA_test_monitorflash() ;
          break ;

        default:
            rcode = ERROR_FLASH_INVALID_COMMAND ;
          break ;
    }
    flash_last_error = rcode ;
    return( rcode ) ;
}


/************************************************************************
 *      Local helper functions
 ************************************************************************/

/************************************************************************
 *
 *                          FLASH_STRATA_devicetype
 *  Description :
 *  -------------
 *  Derive the memory device type from 'physical address'
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'physadr': physical address inside a 512 MByte space
 *
 *
 *  Return values :
 *  ---------------
 *
 *  t_flash_device_id:    memory device type detected.
 *
 *
 ************************************************************************/
static
t_flash_device_id FLASH_STRATA_devicetype( UINT32 physadr )
{
    /* check for system FLASH */
    if ((systemflash_phys_start <= physadr) && (physadr < systemflash_phys_end))
    {
        return(FLASH_SYSTEMFLASH_DEVICE) ;
    }

    /* check for monitor FLASH */
    if ((monitorflash_phys_start <= physadr) && (physadr < monitorflash_phys_end))
    {
        return(FLASH_MONITORFLASH_DEVICE) ;
    }

    /* check for file FLASH */
    if ((fileflash_phys_start <= physadr) && (physadr < fileflash_phys_end))
    {
        return(FLASH_FILEFLASH_DEVICE) ;
    }

    /* check for BOOT */
    if (( boot_phys_start <= physadr) && (physadr < boot_phys_end ))
    {
        return(FLASH_BOOT_DEVICE) ;
    }

    /* device not known */
    return(FLASH_UNKNOWN_DEVICE) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_program_systemram
 *  Description :
 *  -------------
 *  Programs (store data) in the system RAM device.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:             Data stored in system RAM
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_program_systemram( t_FLASH_write_descriptor *p_param )
{
    int len ;
    UINT8 *psrc ;
    UINT8 *pdst ;

    /* set addresses */
    psrc = (UINT8*) p_param->buffer  ;
    pdst = (UINT8*) p_param->adr ;
    len  = p_param->length  ;

    /* call our speedy memcpy */
    memcpy( pdst, psrc, len ) ;
    return(OK) ;
}

/************************************************************************
 *
 *                          FLASH_STRATA_program_flash
 *  Description :
 *  -------------
 *  Programs a Intel 28Fxxx-compliant flash device.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'p_param',   IN,    variable of type, t_FLASH_write_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH programmed succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_program_flash( t_FLASH_write_descriptor *p_param )
{
    int             rcode = OK ;
    t_flash_access  dw ;    /* destination words */
    int             i, j, wc, len, rest ;
    UINT8          *psrc ;
    UINT32          pdst ;

    /* convert addresses to kseg1 */
    psrc = p_param->buffer ;
    pdst = KSEG1(p_param->adr) ;
    len  = p_param->length ;

    /* check any special case */
    if (len <= 0)
    {
        return(OK) ;
    }

    /* test for initial alignment */
    rest = pdst & 0x3 ;
    if (rest)
    {
        /* Need for initial word alignment */

        /* read destination word */
        pdst &= ~0x3;
        dw.d32 = REG32(pdst) ;

        /* calculate upper byte boundary inside word */
        j = 4 ;
        if (len < (4-rest))
        {
            j = rest + len ;
        }

        /* merge rest into last dest. word */
        for (i=rest; i<j; i++)
        {
            /* copy source bytes into last word */
            dw.d8[i] = REG8(psrc) ;
            psrc++ ;
            len-- ;
        }

        /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
        REG32(pdst)  = FLASH_CLEAR_STATUS_COMMAND ;
    
        /* program this word */
        REG32(pdst) = FLASH_WRITE_WORD_COMMAND ;
        REG32(pdst) = dw.d32 ;

        /* await completion */
        rcode = FLASH_STRATA_wait_ready( pdst, FLASH_RETRY_10);   
        if (rcode)
        {
            return(rcode) ;
        }

        /* Verify programmed word */
        REG32(pdst)  = FLASH_READ_COMMAND ;
        if ( REG32(pdst) != dw.d32 )
        {
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "Data check read: (0x%08x)=0x%08x, Data written: 0x%08x", (UINT32)pdst, REG32(pdst), dw.d32) ; 
            return(ERROR_FLASH_VERIFY_ERROR) ;
        }

        /* adjust destination pointer */
        pdst += 4 ;

    }

    /* calculate words to program and some rest of bytes */
    wc   = len / 4 ;
    rest = len % 4 ;

    /* now, destination is word aligned, check source */

    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pdst)  = FLASH_CLEAR_STATUS_COMMAND ;

    if ((UINT32)psrc & 0x3)
    {
        /* destination only is word aligned */

        /* program word by word */
        for (i=0; i<wc; i++)
        {
            /* copy source bytes into dest. word */
            for (j=0; j<4; j++)
            {
                dw.d8[j] = REG8(psrc) ;
                psrc++ ;
            }

            /* program this word */
            REG32(pdst) = FLASH_WRITE_WORD_COMMAND ;
            REG32(pdst) = dw.d32 ;

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pdst, FLASH_RETRY_10);   
            if (rcode != OK)
            {
                return(rcode) ;
            }

            /* Verify programmed word */
            REG32(pdst)  = FLASH_READ_COMMAND ;
            if ( REG32(pdst) != dw.d32 )
            {
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Data check read: (0x%08x)=0x%08x, Data written: 0x%08x", (UINT32)pdst, REG32(pdst), dw.d32) ; 
                return(ERROR_FLASH_VERIFY_ERROR) ;
            }

            /* next destination word */
            pdst += 4 ;

        }
    }
    else
    {
        /* both source and destination is word aligned */
    
        /* program word by word */
        for (i=0; i<wc; i++)
        {
            /* program this word */
            dw.d32      = REG32(psrc) ; /* Source may be a FLASH device */
            REG32(pdst) = FLASH_WRITE_WORD_COMMAND ;
            REG32(pdst) = dw.d32 ;

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pdst, FLASH_RETRY_10);   
            if (rcode != OK)
            {
                return(rcode) ;
            }

            /* Verify programmed word */
            REG32(pdst)  = FLASH_READ_COMMAND ;
            if ( REG32(pdst) != dw.d32 )
            {
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Data check read: (0x%08x)=0x%08x, Data written: 0x%08x", (UINT32)pdst, REG32(pdst), dw.d32) ; 
                return(ERROR_FLASH_VERIFY_ERROR) ;
            }

            /* next destination and source word */
            pdst += 4 ;
            psrc += 4 ;
        }
    }

    if (rest)
    {
        /* we still have some bytes left */

        /* clear status and set flash to read mode */
        /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
        REG32(pdst)  = FLASH_CLEAR_STATUS_COMMAND ;
    
        /* issue 'READ ARRAY' command */
        REG32(pdst)  = FLASH_READ_COMMAND ;

        /* read destination word */
        dw.d32 = REG32(pdst) ;
        /* merge rest into last dest. word */
        for (i=0; i<rest; i++)
        {
            /* copy source bytes into last word */
            dw.d8[i] = REG8(psrc) ;
            psrc++ ;
        }

        /* program this word */
        REG32(pdst) = FLASH_WRITE_WORD_COMMAND ;
        REG32(pdst) = dw.d32 ;

        /* await completion */
        rcode = FLASH_STRATA_wait_ready( pdst, FLASH_RETRY_10);   
        if (rcode != OK)
        {
            return(rcode) ;
        }

        /* Verify programmed word */
        REG32(pdst)  = FLASH_READ_COMMAND ;
        if ( REG32(pdst) != dw.d32 )
        {
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "Data check read: (0x%08x)=0x%08x, Data written: 0x%08x", (UINT32)pdst, REG32(pdst), dw.d32 ) ; 
            return(ERROR_FLASH_VERIFY_ERROR) ;
        }

    }
    return( rcode ) ;
}



/************************************************************************
 *
 *                          FLASH_STRATA_set_systemflash_read
 *  Description :
 *  -------------
 *  Set system flash device in read mode.
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
 * 'OK' = 0x00:                         System FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_systemflash_read( void )
{
    UINT32          bank ;
    volatile UINT32 *pw ;
	 
    pw = (UINT32*) KSEG1(systemflash_phys_start) ;
    for (bank = 0; bank < systemflash_bank_count; bank++)
    {
        /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
        REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

        /* issue 'READ ARRAY' command */
        REG32(pw)  = FLASH_READ_COMMAND;

        /* next bank: */
        pw = (UINT32*)((UINT32)pw + (systemflash_block_count * systemflash_block_size)) ;
    }
    return( OK ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_set_fileflash_read
 *  Description :
 *  -------------
 *  Set file flash device in read mode.
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
 * 'OK' = 0x00:                         File FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_fileflash_read( void )
{
    volatile UINT32 *pw ;
	 
    pw = (UINT32*) KSEG1(fileflash_phys_start) ;
    
    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

    /* issue 'READ ARRAY' command */
    REG32(pw)  = FLASH_READ_COMMAND;

    return( OK ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_set_monitorflash_read
 *  Description :
 *  -------------
 *  Set monitor flash device in read mode.
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
 * 'OK' = 0x00:                         File FLASH set to read mode
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_set_monitorflash_read( void )
{
    volatile UINT32 *pw ;
	 
    pw = (UINT32*) KSEG1(monitorflash_phys_start) ;
    
    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

    /* issue 'READ ARRAY' command */
    REG32(pw)  = FLASH_READ_COMMAND;

    return( OK ) ;
}

/************************************************************************
 *
 *                          FLASH_STRATA_erase_fileflash
 *  Description :
 *  -------------
 *  Erase file flash, which is the last block (in each FLASH device)
 *  of the monitor flash.
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
 * 'OK' = 0x00:                         File FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_erase_fileflash( void )
{
    int             rcode = OK ;
    UINT32          pw;
	 
    /* We know, this is just one block to erase */
    pw = KSEG1(fileflash_phys_start) ;

    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND ;

    /* issue 'READ_ID_CODES' command */
    REG32(pw)  = FLASH_READ_ID_CODES_COMMAND ;

    /* read status and check for LOCKED block */
    if ( REG32(pw+FLASH_BLOCKSTATUS_OFS) & FLASH_BLOCKSTATUS_LOCK_MSK )     
    {
        /* issue 'clear lock-bit' command */
        REG32(pw)  = FLASH_CLEAR_LOCK_COMMAND ;

        /* issue 'confirm' command */
        REG32(pw)  = FLASH_CONFIRM_COMMAND;    

        /* await completion */
        rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
    }

    if (rcode == OK)
    {
        /* issue 'erase' command */
        REG32(pw)  = FLASH_ERASE_COMMAND;
    
        /* issue 'confirm' command */
        REG32(pw)  = FLASH_CONFIRM_COMMAND;    

        /* await completion */
        rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
    }

    /* check for hw write protect */
    if (rcode ==  ERROR_FLASH_LOCKED)
    {
        if ( FLASH_STRATA_is_file_flash_write_protected() )
        {
            rcode = ERROR_FLASH_FILE_FLASH_LOCK ;
        }
    }

    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

    /* issue 'READ ARRAY' command */
    REG32(pw)  = FLASH_READ_COMMAND;

    return( rcode ) ;
}

/************************************************************************
 *
 *                          FLASH_STRATA_erase_flasharea
 *  Description :
 *  -------------
 *  Erase flash area; i.e. the driver erases the flash blocks inside 
 *  the specified memory area.
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *  ERROR_FLASH_INVALID_ADDRESS         Address area not inside FLASH
 *                                      devices
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_erase_flasharea( t_FLASH_ctrl_descriptor *p_param )
{
    int rcode = OK ;
    UINT32 blocksize, start, end, blockcount ;
    UINT32 block ;
    bool   monitor_flash = FALSE ;
    UINT32 pw ;

    switch (FLASH_STRATA_devicetype(p_param->user_physadr))
    {
        case FLASH_SYSTEMFLASH_DEVICE:
            /* system FLASH */
            if ( FLASH_STRATA_is_system_flash_write_protected() )
            {
                return( ERROR_FLASH_WRITE_PROTECTED ) ;
            }
            else
            {
                /* validate upper boundary */
                if ( FLASH_STRATA_devicetype( p_param->user_physadr +
                                              p_param->user_length - 1 ) 
                     != FLASH_SYSTEMFLASH_DEVICE )
                {
                    return( ERROR_FLASH_INVALID_ADDRESS ) ;
                }
                WRITE_ENABLE ;
                blocksize = systemflash_block_size ;
            }
          break ;

        case FLASH_FILEFLASH_DEVICE:
            /* Reject erase of FILE FLASH */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "Environment FLASH: map = (0x%08x,0x%08x), user = 0x%08x", 
                     (UINT32) fileflash_phys_start, 
                     (UINT32) fileflash_phys_end, 
                     (UINT32) PHYS(p_param->user_physadr) ) ; 
            return( ERROR_FLASH_FILE_FLASH_PROT ) ;

        case FLASH_MONITORFLASH_DEVICE:
            /* Monitor FLASH */
            monitor_flash = TRUE ;

            /* validate upper boundary */
            if ( FLASH_STRATA_devicetype( p_param->user_physadr +
                                          p_param->user_length - 1 ) 
                 != FLASH_MONITORFLASH_DEVICE )
            {
                return( ERROR_FLASH_INVALID_ADDRESS ) ;
            }

            blocksize = monitorflash_block_size ;
          break ;

        case FLASH_BOOT_DEVICE:
            /* Boot device */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "MIPS boot media: map = (0x%08x,0x%08x), user = 0x%08x", 
                     (UINT32) boot_phys_start, 
                     (UINT32) boot_phys_end, 
                     (UINT32) PHYS(p_param->user_physadr) ) ; 
            return( ERROR_FLASH_BOOT_WRITE_PROTECTED ) ;

        default:
            return( ERROR_FLASH_INVALID_ADDRESS ) ;
    }

    /* calculate committed FLASH area */
    start      = (KSEG1(p_param->user_physadr)/blocksize) * blocksize ;
    end        = ((((KSEG1(p_param->user_physadr) + 
                          p_param->user_length)/blocksize)) * blocksize) - 1  ;
    if ( ((KSEG1(p_param->user_physadr)+p_param->user_length) % blocksize) != 0)
    {
        end = end + blocksize ;
    }
    blockcount = (end + 1 - start)/ blocksize ;

    /* return committed area to user */
    p_param->driver_physadr = PHYS(start) ;
    p_param->driver_length  = (end - start) + 1 ;

    /* erase FLASH area */
    pw = start ;

    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND ;

    for (block = 0; block < blockcount; block++)
    {
        /* issue 'READ_ID_CODES' command */
        REG32(pw)  = FLASH_READ_ID_CODES_COMMAND ;

        /* read status and check for LOCKED block */
        if ( REG32(pw+FLASH_BLOCKSTATUS_OFS) & FLASH_BLOCKSTATUS_LOCK_MSK )     
        {
            /* issue 'clear lock-bit' command */
            REG32(pw)  = FLASH_CLEAR_LOCK_COMMAND ;

            /* issue 'confirm' command */
            REG32(pw)  = FLASH_CONFIRM_COMMAND;    

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
        }
        if (rcode == OK)
        {
            /* issue 'erase' command */
            REG32(pw)  = FLASH_ERASE_COMMAND;
    
            /* issue 'confirm' command */
            REG32(pw)  = FLASH_CONFIRM_COMMAND;    

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
        }

        /* check for hw write protect */
        if ( (rcode == ERROR_FLASH_LOCKED) && (monitor_flash) )
        {
            if ( FLASH_STRATA_is_monitor_flash_write_protected() )
            {
                rcode = ERROR_FLASH_MONITOR_FLASH_LOCK ;
            }
        }


        /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
        REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

        /* issue 'READ ARRAY' command */
        REG32(pw)  = FLASH_READ_COMMAND;

        /* check status now, after having set back the flash in read array mode */
        if (rcode != OK)
        {
            break ;
        }

        /* next block */
        pw = pw + blocksize ;
    }
    /* enable write protection again in system flash */
    WRITE_DISABLE ;
    return( rcode ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_inquire_flasharea
 *  Description :
 *  -------------
 *  Inquire flash area; i.e. the driver calculates the effective flash
 *  block area covering the specified memory area.
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   IN,    variable of type, t_FLASH_ctrl_descriptor.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:                         Area specified inside valid
 *                                      FLASH device area
 *  ERROR_FLASH_INVALID_ADDRESS         Address area not inside FLASH
 *                                      devices
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_inquire_flasharea( t_FLASH_ctrl_descriptor *p_param )
{
    UINT32 blocksize, start, end ;

    //printf(" FLASH_STRATA_inquire_flasharea: %x, %x\n", p_param->user_physadr, p_param->user_length ) ;

    switch (FLASH_STRATA_devicetype(p_param->user_physadr))
    {
        case FLASH_SYSTEMFLASH_DEVICE:
            /* system FLASH */

            /* validate upper boundary */
            if ( FLASH_STRATA_devicetype( p_param->user_physadr +
                                          p_param->user_length - 1 ) 
                 != FLASH_SYSTEMFLASH_DEVICE )
            {
                return( ERROR_FLASH_INVALID_ADDRESS ) ;
            }

            blocksize = systemflash_block_size ;
          break ;

        case FLASH_FILEFLASH_DEVICE:
            /* Reject programming of FILE FLASH */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "Environment FLASH: map = (0x%08x,0x%08x), user = 0x%08x", 
                     (UINT32) fileflash_phys_start, 
                     (UINT32) fileflash_phys_end, 
                     (UINT32) PHYS(p_param->user_physadr) ) ; 
            return( ERROR_FLASH_FILE_FLASH_PROT ) ;

        case FLASH_MONITORFLASH_DEVICE:
            /* Monitor FLASH */

            /* validate upper boundary */
            if ( FLASH_STRATA_devicetype( p_param->user_physadr +
                                          p_param->user_length - 1 ) 
                 != FLASH_MONITORFLASH_DEVICE )
            {
                return( ERROR_FLASH_INVALID_ADDRESS ) ;
            }

            blocksize = monitorflash_block_size ;
          break ;

        case FLASH_BOOT_DEVICE:
            /* Boot device */
            memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
            sprintf( flash_diag_msg, "MIPS boot media: map = (0x%08x,0x%08x), user = 0x%08x",
                     (UINT32) boot_phys_start,
                     (UINT32) boot_phys_end,
                     (UINT32) PHYS(p_param->user_physadr) ) ;
            return( ERROR_FLASH_BOOT_WRITE_PROTECTED ) ;

        default:
            return( ERROR_FLASH_INVALID_ADDRESS ) ;
    }

    /* calculate committed FLASH area */
    start      = (KSEG1(p_param->user_physadr)/blocksize) * blocksize ;
    end        = ((((KSEG1(p_param->user_physadr) + 
                          p_param->user_length)/blocksize)) * blocksize) - 1  ;
    if ( ((KSEG1(p_param->user_physadr)+p_param->user_length) % blocksize) != 0)
    {
        end = end + blocksize ;
    }

    /* return committed area to user */
    p_param->driver_physadr = PHYS(start) ;
    p_param->driver_length  = (end - start) + 1 ;

    return( OK ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_test_systemflash
 *  Description :
 *  -------------
 *  Test complete file flash (just one block)
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
 * 'OK' = 0x00:                         System FLASH OK
 *  ERROR_FLASH_LOCKED                  Some sector(s) locked
 *  ERROR_FLASH_ERASE_ERROR             Some sector(s) have erase error
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_test_systemflash( void )
{
    int             rcode = OK ;
    UINT32          block, bank ;
    UINT32          pw ;
	 
    pw = KSEG1(systemflash_phys_start) ;
    for (bank = 0; bank < systemflash_bank_count; bank++)
    {
        for (block = 0; block < systemflash_block_count; block++)
        {
            /* issue 'QUERY' command */
            REG32(pw)  = FLASH_QUERY_COMMAND ;

#ifdef  FLASH_DEBUG
            sprintf( msg, "\n\r FLASH_STRATA_test_systemflash: Adr= 0x%08x, Sta= 0x%08x\n\r", 
                   pw, REG32(pw+FLASH_BLOCKSTATUS_OFS) ) ;
            PUTS( DEFAULT_PORT, msg ) ;
#endif

            /* Verify 'Q' char of the Query-unique ASCII string, part of CFI spec. */
            if ( REG32(pw+FLASH_QUERYQCHAR_OFS) != FLASH_QUERYQCHAR )     
            {
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYQCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYQCHAR_OFS) ) ;
                rcode = ERROR_FLASH_QRY_NOT_FOUND ;
                break ;
            }

            /* Verify 'R' char of the Query-unique ASCII string, part of CFI spec. */
            if ( REG32(pw+FLASH_QUERYRCHAR_OFS) != FLASH_QUERYRCHAR )     
            {
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYRCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYRCHAR_OFS) ) ;
                rcode = ERROR_FLASH_QRY_NOT_FOUND ;
                break ;
            }

            /* Verify 'Y' char of the Query-unique ASCII string, part of CFI spec. */
            if ( REG32(pw+FLASH_QUERYYCHAR_OFS) != FLASH_QUERYYCHAR )     
            {
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYYCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYYCHAR_OFS) ) ;
                rcode = ERROR_FLASH_QRY_NOT_FOUND ;
                break ;
            }


            /* next block */
            pw = pw + systemflash_block_size ;
        }

        if (rcode != OK)
        {
            break ;
        }
    }


    /* reset working pointer to start of system flash */
    pw = KSEG1(systemflash_phys_start) ;
    for (bank = 0; bank < systemflash_bank_count; bank++)
    {
        /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
        REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

        /* issue 'READ ARRAY' command */
        REG32(pw)  = FLASH_READ_COMMAND;

        /* next bank: */
        pw = pw + (systemflash_block_count * systemflash_block_size) ;
    }
    return( rcode ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_test_fileflash
 *  Description :
 *  -------------
 *  Test complete file flash.
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
 * 'OK' = 0x00:                         System FLASH OK
 *  ERROR_FLASH_LOCKED                  Some sector(s) locked
 *  ERROR_FLASH_ERASE_ERROR             Some sector(s) have erase error
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_test_fileflash( void )
{
    int             rcode = OK ;
    UINT32          pw ;
	 
    pw = KSEG1(fileflash_phys_start) ;

    /* issue 'QUERY' command */
    REG32(pw)  = FLASH_QUERY_COMMAND ;

    /* Verify 'Q' char of the Query-unique ASCII string, part of CFI spec. */
    if ( REG32(pw+FLASH_QUERYQCHAR_OFS) != FLASH_QUERYQCHAR )
    {
        memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
        sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYQCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYQCHAR_OFS) ) ;
        rcode = ERROR_FLASH_QRY_NOT_FOUND ;
    }

    /* Verify 'R' char of the Query-unique ASCII string, part of CFI spec. */
    if ( REG32(pw+FLASH_QUERYRCHAR_OFS) != FLASH_QUERYRCHAR )
    {
        memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
        sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYRCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYRCHAR_OFS) ) ;
        rcode = ERROR_FLASH_QRY_NOT_FOUND ;
    }

    /* Verify 'Y' char of the Query-unique ASCII string, part of CFI spec. */
    if ( REG32(pw+FLASH_QUERYYCHAR_OFS) != FLASH_QUERYYCHAR )
    {
        memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
        sprintf( flash_diag_msg, "Adr= %08x, (Adr)= %08x",pw+FLASH_QUERYYCHAR_OFS,
                                                                  REG32(pw+FLASH_QUERYYCHAR_OFS) ) ;
        rcode = ERROR_FLASH_QRY_NOT_FOUND ;
    }

    /* reset working pointer to start of system flash */
    pw = KSEG1(fileflash_phys_start) ;

    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

    /* issue 'READ ARRAY' command */
    REG32(pw)  = FLASH_READ_COMMAND;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_test_monitorflash
 *  Description :
 *  -------------
 *  Test and set lock bits of complete monitor flash (including file flash)
 *   The lower 1 Mbytes hold YAMON and are locked (write protected) 
 *   The upper 3 Mbytes are unlocked.
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
 * 'OK' = 0x00:                         System FLASH OK
 *  ERROR_FLASH_LOCKED                  Some sector(s) locked
 *  ERROR_FLASH_ERASE_ERROR             Some sector(s) have erase error
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_test_monitorflash( void )
{
    int             rcode = OK ;
    UINT32          pw ;
    UINT32          pw_lock_end;
    UINT32          pw_unlock_end;
	 
    pw = KSEG1(monitorflash_phys_start) ;
    pw_lock_end = pw + 0x00100000;
    pw_unlock_end = KSEG1(fileflash_phys_end) ;
    if (monitorflash_phys_start == 0 || pw_lock_end >= pw_unlock_end)
    {
        return ERROR_FLASH_INVALID_COMMAND ;
    }

    for (; pw < pw_lock_end; pw += monitorflash_block_size)
    {
        /* issue 'QUERY' command */
        REG32(pw)  = FLASH_READ_ID_CODES_COMMAND ;

        /* read status and check for LOCKED block */
        if ( (REG32( (pw+FLASH_BLOCKSTATUS_OFS) ) & FLASH_BLOCKSTATUS_LOCK_MSK) !=  FLASH_BLOCKSTATUS_LOCK_MSK)     
        {
            /* Lock */
            /* issue 'clear lock-bit' command */
            REG32(pw)  = FLASH_CLEAR_LOCK_COMMAND ;

            /* issue 'set lock-bit' command */
            REG32(pw)  = FLASH_LOCK_SECTOR ;

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
            if (rcode != OK)
            {
                rcode |= pw;
                goto out ;
            }
        }

    }
    for (; pw < pw_unlock_end; pw += monitorflash_block_size)
    {
        /* issue 'QUERY' command */
        REG32(pw)  = FLASH_READ_ID_CODES_COMMAND ;

        /* read status and check for LOCKED block */
        if ( REG32( (pw+FLASH_BLOCKSTATUS_OFS) ) & FLASH_BLOCKSTATUS_LOCK_MSK )
        {
            /* Unlock */
            /* issue 'clear lock-bit' command */
            REG32(pw)  = FLASH_CLEAR_LOCK_COMMAND ;

            /* issue 'confirm' command */
            REG32(pw)  = FLASH_CONFIRM_COMMAND;    

            /* await completion */
            rcode = FLASH_STRATA_wait_ready( pw, FLASH_RETRY_10);   
            if (rcode != OK)
            {
                rcode |= pw;
                goto out ;
            }
        }

    }

out:
    /* reset working pointer to start of monitor flash */
    pw = KSEG1(monitorflash_phys_start) ;
    /* issue 'CLEAR STATUS: SR-5,4,3,1' command */
    REG32(pw)  = FLASH_CLEAR_STATUS_COMMAND;

    /* issue 'READ ARRAY' command */
    REG32(pw)  = FLASH_READ_COMMAND;

    return( rcode ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_wait_ready
 *  Description :
 *  -------------
 *  
 *  Await FLASH operation completes.
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
 * 'OK' = 0x00:                         File FLASH erased succesfully
 *  ERROR_FLASH_PROGRAM_ERROR           Flash device failure
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_wait_ready(UINT32 pw, UINT32 retry )
{
    UINT32 first, latest, previous, accumulate, millisec ;

    accumulate = 0 ;

    /* get millisecond count */
    SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                 &first,
                 sizeof(first) ) ;
    previous = first ;
    millisec = FLASH_TMOUT_2_5SEC ; 
    while ( 1 )
    {
        /* is flash ready ? */
        if ( (REG32(pw) & FLASH_STATUS_READY) == FLASH_STATUS_READY )
        {
            /* yes, it is ready, read another time, according to errata. */

            /* is status OK ? */
            if ( (REG32(pw) & FLASH_STATUS_MASK) == FLASH_STATUS_OK )
            {
                /* yes, it is ok ! */
                return( OK ) ;
            }
            else
            {
                /* keep diagnose message */
                memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
                sprintf( flash_diag_msg, "Flash status: (0x%08x)=0x%08x",pw,REG32(pw)) ; 

                /* check for lock bit */
                if ( (REG32(pw) & FLASH_STATUS_LOCK) )
                    return( ERROR_FLASH_LOCKED ) ;

                /* check for lack of programming voltage */
                if ( (REG32(pw) & FLASH_STATUS_LOW_VOLTAGE) )
                    return( ERROR_FLASH_LOW_VOLTAGE ) ;

                /* check for erase error */
                if ( (REG32(pw) & FLASH_STATUS_ERASE) )
                    return( ERROR_FLASH_ERASE_ERROR ) ;

                /* some error is indicated */
                return( ERROR_FLASH_PROGRAM_ERROR ) ;
            }
        }

        /* get millisecond count */
        SYSCON_read( SYSCON_BOARD_GET_MILLISEC_ID,
                     &latest,
                     sizeof(latest) ) ;
        if ( latest >= previous )
        {
            /* counter still not wrapped */
            if ( (accumulate + (latest - first)) > millisec )
            {
                break ;
            }
        }
        else
        {
            /* counter did wrap */
            accumulate = accumulate + (previous - first) ;
            if ( (accumulate + latest) > millisec )
            {
                break ;
            }

            /* reset first */
            first = 0 ;
        }

        /* prepare next delta time */
        previous = latest ;
    }

    /* keep diagnose message */
    memset( flash_diag_msg, 0, sizeof(flash_diag_msg) ) ;
    sprintf( flash_diag_msg, "Adr=0x%08x",pw) ; 

    /* we should never end up here time-out */
    return( ERROR_FLASH_TIME_OUT ) ;
}


/************************************************************************
 *
 *                          FLASH_STRATA_error_lookup
 *  Description :
 *  -------------
 *  Lookup error code to error string(s)
 *  
 *
 *  Parameters :
 *  ------------
 * 
 *  'p_param',   INOUT,    variable of type, t_sys_error_string.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:  
 *
 *
 ************************************************************************/
static
INT32 FLASH_STRATA_error_lookup( t_sys_error_string *p_param )
{
    UINT32 t, i ;

    i = 0 ;
    p_param->count = 0 ;
    t = SYSERROR_ID( p_param->syserror ) ;
    if ( flash_last_error == p_param->syserror )
    {
        /* check for recognized error code */
        if (t < sizeof(flash_error_string)/sizeof(char*) )
        {
            /* fill in mandatory error message string */
            p_param->strings[SYSCON_ERRORMSG_IDX] = flash_error_string[t] ;
            i++ ;

            /* check for diagnose message */
            if ( flash_diag_msg[0] != 0 )
            {
                /* fill in optional diagnose message string */
                p_param->strings[SYSCON_DIAGMSG_IDX] = flash_diag_msg ;
                i++ ;
            }

            /* check for hint message */
            if ( flash_error_hint_string[t] != NULL)
            {
                /* fill in optional hint message string */
                p_param->strings[SYSCON_HINTMSG_IDX] = flash_error_hint_string[t] ;
                i++ ;
            }
        }
        p_param->count      = i ;
    }
 
    /* delete context */
    flash_last_error  = OK ;
    return(OK) ;
}

/************************************************************************
 *
 *                          FLASH_STRATA_is_system_flash_write_protected
 *  Description :
 *  -------------
 *  Check if system flash id write protected
 *  
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_system_flash_write_protected( void )
{
    UINT32 wr_protect ;

    /* check in SYSCON */
    SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_WRITE_PROTECTED_ID,
                 &wr_protect,
                 sizeof(wr_protect) ) ;
    if (wr_protect)
    {
        return( TRUE ) ;
    }
    else
    {
        return( FALSE ) ;
    }
}


/************************************************************************
 *
 *                          FLASH_STRATA_is_file_flash_write_protected
 *  Description :
 *  -------------
 *  Check if file flash id write protected
 *  
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_file_flash_write_protected( void )
{
    UINT32 wr_protect ;

    /* check in SYSCON */
    SYSCON_read( SYSCON_BOARD_FILEFLASH_WRITE_PROTECTED_ID,
                 &wr_protect,
                 sizeof(wr_protect) ) ;
    if (wr_protect)
    {
        return( TRUE ) ;
    }
    else
    {
        return( FALSE ) ;
    }
}


/************************************************************************
 *
 *                          FLASH_STRATA_is_monitor_flash_write_protected
 *  Description :
 *  -------------
 *  Check if file flash id write protected
 *  
 *
 *  Parameters :
 *  ------------
 *
 *
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE: not write protected
 *  TRUE:  write protected
 *
 *
 ************************************************************************/
static
bool FLASH_STRATA_is_monitor_flash_write_protected( void )
{
    UINT32 wr_protect ;

    /* check in SYSCON */
    SYSCON_read( SYSCON_BOARD_MONITORFLASH_WRITE_PROTECTED_ID,
                 &wr_protect,
                 sizeof(wr_protect) ) ;
    if (wr_protect)
    {
        return( TRUE ) ;
    }
    else
    {
        return( FALSE ) ;
    }
}
