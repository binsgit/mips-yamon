/************************************************************************
 *
 *      sysenv.c
 *
 *      The 'sysenv' module implements the system environment variable
 *      write and read functions, which operate on a RAM and FLASH
 *      storage media. The functions are normally called through the
 *      SYSCON api.
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

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <syscon_api.h>
#include <product.h>
#include <sysenv_api.h>

/* generic device driver interface */
#include <io_api.h>

/* drivers */
#include <flash_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/
/* File FLASH state definitions */
#define SYSENV_STATE_OK        0x42424242
#define SYSENV_STATE_ERROR     0x00000000

/* Header bytes in each record */
/* Register addresses per record */
#define SYSENV_HEADER          0

/* Header field encoding */
#define SYSENV_HEADER_SIZE     4

/* field: 'CONTROL' */
#define SYSENV_HEADER_CONTROL_SHF        0
#define SYSENV_HEADER_CONTROL_MSK        (MSK(8) << SYSENV_HEADER_CONTROL_SHF)

/* field: 'CHKSUM' */
#define SYSENV_HEADER_CHKSUM_SHF         8
#define SYSENV_HEADER_CHKSUM_MSK         (MSK(8) << SYSENV_HEADER_CHKSUM_SHF)

/* field: 'ID' */
#define SYSENV_HEADER_ID_SHF             16
#define SYSENV_HEADER_ID_MSK             (MSK(8) << SYSENV_HEADER_ID_SHF)

/* field: 'SIZE' */
#define SYSENV_HEADER_SIZE_SHF           24
#define SYSENV_HEADER_SIZE_MSK           (MSK(8) << SYSENV_HEADER_SIZE_SHF)


#define SYSENV_DATA                      0
/* field: '0' */
#define SYSENV_DATA_BYTE0_SHF            0
#define SYSENV_DATA_BYTE0_MSK            (MSK(8) << SYSENV_DATA_BYTE0_SHF)

/* field: '1' */
#define SYSENV_DATA_BYTE1_SHF            8
#define SYSENV_DATA_BYTE1_MSK            (MSK(8) << SYSENV_DATA_BYTE1_SHF)

/* field: '2' */
#define SYSENV_DATA_BYTE2_SHF            16
#define SYSENV_DATA_BYTE2_MSK            (MSK(8) << SYSENV_DATA_BYTE2_SHF)

/* field: '3' */
#define SYSENV_DATA_BYTE3_SHF            24
#define SYSENV_DATA_BYTE3_MSK            (MSK(8) << SYSENV_DATA_BYTE3_SHF)


/* control byte definitions: */
#define SYSENV_RECORD_IS_FREE  0xff
#define SYSENV_RECORD_IN_USE   0x42

/* Environment variable record */
typedef UINT8  t_SYSENV_elem[SYS_USER_ENVIRONMENT_DATA_SIZE+SYSENV_HEADER_SIZE] ;

/* lookup table entry element */
typedef struct sysenv_lookup_entry
{
    void  *ram_record ;    /* may keep a record reference to ram space,
                              during garbage collection. Normally
                              this variable is set to NULL */
    void  *flash_record ;  /* may keep a record reference to flash space
                              when a record is allocated. When no record
                              has been allocated for this entry, this
                              variable is set to NULL */
} t_SYSENV_lookup_entry ;

/************************************************************************
 *      Public variables
 ************************************************************************/

/************************************************************************
 *      Static variables
 ************************************************************************/

/* Global state of file flash */
static UINT32 SYSENV_state ;

/* lookup table */
static t_SYSENV_lookup_entry SYSENV_lookup[SYS_USER_ENVIRONMENT_MAX_INDEX+1] ;

/* maximum number of records in file flash */
static UINT32 SYSENV_max_records ;

/* index for next free record in file flash */
static UINT32  SYSENV_next_free ;

/* start and size of file flash */
static void  *SYSENV_file_flash_start ;
static UINT32 SYSENV_file_flash_size  ;

/* storage, used by write operations to format buffer for variable */
static t_SYSENV_elem SYSENV_write_buffer ;

/* this storage is used during garbage collection to store the variables
   temporary during the update, where variables are copied from flash
   to ram, flash is deleted and variables are copied back to flash. */
static t_SYSENV_elem SYSENV_ram_cache[SYS_USER_ENVIRONMENT_MAX_INDEX+1] ;

static char* sysenv_error_string[] =
{
    /* ERROR_SYSENV_OUT_OF_MEMORY    */  "Int. ERROR: No more dynamic memory",
    /* ERROR_SYSENV_ENV_VAR_TOO_BIG  */  "Int. ERROR: Environment var size too big",
    /* ERROR_SYSENV_NO_VARIABLE      */  "Int. ERROR: Environment var not created",
    /* ERROR_SYSENV_INVALID_INDEX    */  "Int. ERROR: Environment var index invalid",
    /* ERROR_SYSENV_UPDATE_READ_REFS */  "WARNING: All references for env variables must be re-read",
    /* ERROR_SYSENV_FLASH_INVALID    */  "Int. ERROR: FLASH is not usable"
} ;

static char* sysenv_error_hint_string[] =
{
    /* ERROR_SYSENV_OUT_OF_MEMORY    */  NULL,
    /* ERROR_SYSENV_ENV_VAR_TOO_BIG  */  NULL,
    /* ERROR_SYSENV_NO_VARIABLE      */  NULL,
    /* ERROR_SYSENV_INVALID_INDEX    */  NULL,
    /* ERROR_SYSENV_UPDATE_READ_REFS */  NULL,
    /* ERROR_SYSENV_FLASH_INVALID    */  "Re-init environment: erase -e",
} ;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/

/************************************************************************
 *
 *                          SYSENV_error_lookup
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
INT32 SYSENV_error_lookup( t_sys_error_string *p_param ) ;

/************************************************************************
 *
 *                          SYSENV_collect_garbage
 *  Description :
 *  -------------
 *
 *  Execute garbage collection.
 *
 *
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
 *  'ERROR_SYSENV_UPDATE_READ_REFS', all references for env variables 
                                     must be re-read.
 *
 ************************************************************************/
static INT32 SYSENV_collect_garbage( void ) ;


/************************************************************************
 *
 *                          SYSENV_check_inuse_record
 *  Description :
 *  -------------
 *  Check referenced record
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'pb',      INOUT,    environment flash record.
 *  'index',   INOUT,    environment flash record index.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'ERROR_SYSENV_FLASH_INVALID':  Environment is corrupted.
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
INT32 SYSENV_check_inuse_record( UINT8 *pb, UINT8 index ) ;


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          SYSENV_check_state
 *  Description :
 *  -------------
 *
 *  Check the 'sysenv' state.
 *
 *
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
 *  ERROR_SYSENV_FLASH_INVALID, sysenv is corrupted.
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
INT32 SYSENV_check_state( void )
{
    if (SYSENV_state != SYSENV_STATE_OK)
    {
        printf("\nWARNING: Environment variable flash area is invalid!"
	       "\nHINT   : Perform \"erase -e\"" );

        return( ERROR_SYSENV_FLASH_INVALID ) ;
    }
    return(OK) ;
}

/************************************************************************
 *
 *                          SYSENV_init
 *  Description :
 *  -------------
 *
 *  Initializes the 'sysenv' module.
 *
 *
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
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
INT32 SYSENV_init( void )
{
    t_sys_error_lookup_registration  registration;
    t_FLASH_ctrl_descriptor	     flash_ctrl;
    INT32			     rcode;
    UINT8			     *pf;
    int				     i, j ;
    UINT32			     default_switch;
    UINT32			     header, control, indx, size;
    
    SYSCON_read( SYSCON_BOARD_USE_DEFAULT_ID,
		 (void *)&default_switch,
		 sizeof(UINT32) );

    if( default_switch )
    {
        /* Erase fileflash */
        flash_ctrl.command = FLASH_CTRL_ERASE_FILEFLASH;
        IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, (UINT8 *)(&flash_ctrl) );
    }

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_SYSENV ) ;
    registration.lookup = SYSENV_error_lookup ;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    /* Set default SYSENV state = 'ERROR' */
    SYSENV_state = SYSENV_STATE_ERROR ;

    /* initialize lookup table */
    for ( i = 0; i <= SYS_USER_ENVIRONMENT_MAX_INDEX; i++)
    {
        SYSENV_lookup[i].ram_record   = &SYSENV_ram_cache[i] ;
        SYSENV_lookup[i].flash_record = NULL ;
    }

    /* get start of file flash */
    rcode = SYSCON_read( SYSCON_BOARD_FILEFLASH_BASE_ID,
                         &SYSENV_file_flash_start,
                         sizeof(SYSENV_file_flash_start) ) ;
    if (rcode != OK)
    {
        return(rcode) ;
    }
    else
        SYSENV_file_flash_start = (void *)KSEG1(SYSENV_file_flash_start);

    /* get size for file flash  */
    rcode = SYSCON_read( SYSCON_BOARD_FILEFLASH_SIZE_ID,
                         &SYSENV_file_flash_size,
                         sizeof(SYSENV_file_flash_size) ) ;
    if (rcode != OK)
    {
        return(rcode) ;
    }

    /* calculate maximum number of records in file flash */
    SYSENV_max_records = SYSENV_file_flash_size / sizeof(t_SYSENV_elem) ;

    /* initialize current context */
    pf = SYSENV_file_flash_start ;
    for ( i = 0; i < SYSENV_max_records; i++, pf += sizeof(t_SYSENV_elem) )
    {
	header = *(UINT32*)pf;
        control = REGFIELD(header,SYSENV_HEADER_CONTROL);
        indx = REGFIELD(header,SYSENV_HEADER_ID);
        size = REGFIELD(header,SYSENV_HEADER_SIZE);

        if ( control == SYSENV_RECORD_IS_FREE )
        { 
            /* This is the first free record in file flash */
            SYSENV_next_free = i ;
            break ;
        }

        if ( control == SYSENV_RECORD_IN_USE )
        { 
            /* This record is in use */
            /* check index: */
            if ( indx > SYS_USER_ENVIRONMENT_MAX_INDEX )
            {
                /* This is an error, file FLASH is corrupted ! */
                return( ERROR_SYSENV_FLASH_INVALID ) ;
            }

            /* check size: */
            if ( size  > SYS_USER_ENVIRONMENT_DATA_SIZE )
            {
               /* This is an error, file FLASH is corrupted ! */
               return( ERROR_SYSENV_FLASH_INVALID ) ;
            }

            /* check for record removal */
            if ( size  == 0 )
            {
                /* delete context */
                SYSENV_lookup[indx].flash_record = NULL ;
            }
            else
            {
                /* insert this context in lookup table */
                SYSENV_lookup[indx].flash_record = pf ;
                rcode = SYSENV_check_inuse_record( pf, indx ) ;
                if ( rcode != OK )
                {
                    return( rcode ) ;
                }
            }
        }
	else
        {
            /* Neither free nor in use:  file FLASH is corrupted ! */
            return( ERROR_SYSENV_FLASH_INVALID ) ;
        }
    }

    /* validate the free file FLASH area */
    pf = SYSENV_file_flash_start + SYSENV_next_free * sizeof(t_SYSENV_elem) ;
    for ( i = SYSENV_next_free; i < SYSENV_max_records; i++)
    {
        for ( j=0; j<(sizeof(t_SYSENV_elem)/sizeof(UINT32)); j++, pf+= sizeof(UINT32) ) 
        {
            if ( *(UINT32*)pf != 0xffffffff )    
            {
                /* This is an error, file FLASH is corrupted ! */
                return( ERROR_SYSENV_FLASH_INVALID ) ;
            }
        }
    }

    /* Set SYSENV state = 'OK' */
    SYSENV_state = SYSENV_STATE_OK ;
    return(OK) ;
}


/************************************************************************
 *
 *                          SYSENV_read
 *  Description :
 *  -------------
 *
 *  Read a system environment variable. Unformatted data are read from
 *  a data array, accessed via an 'index'. Data are not copied, just
 *  a pointer reference is being returned to the data array and the
 *  actual size of the data, which have been stored.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'var',          INOUT, data are not copied
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *
 *
 ************************************************************************/
INT32 SYSENV_read( t_user_environment_var *var )
{
    INT32 header;
    UINT8 *pf ;

    if (SYSENV_state != SYSENV_STATE_OK)
    {
        return( ERROR_SYSENV_FLASH_INVALID ) ;
    }

    if ( var->index > SYS_USER_ENVIRONMENT_MAX_INDEX )
    {
        return( ERROR_SYSENV_INVALID_INDEX ) ;
    }

    /* index OK, just try and get the context */
    pf = SYSENV_lookup[ var->index ].flash_record ;
    if ( pf == NULL )
    {
        return ( ERROR_SYSENV_NO_VARIABLE ) ; /* empty index */
    }

    /* OK, there is a environment variable */
    /* verify inuse record */
    if ( SYSENV_check_inuse_record( pf, var->index ) == OK )
    {
	header = *(UINT32*)pf;
        var->size       = REGFIELD(header,SYSENV_HEADER_SIZE);
        var->data_inout = (void*)(pf + SYSENV_HEADER_SIZE) ;
    }
    else
    {
        var->size       = 0 ;
        var->data_inout = NULL ;
    }

    if (var->size == 0)
    {
        /* Record has been deleted */
        return ( ERROR_SYSENV_NO_VARIABLE ) ;
    }
    return( OK ) ;
}


/************************************************************************
 *
 *                          SYSENV_write
 *  Description :
 *  -------------
 *
 *  Write data to a system environment variable. Unformatted data are
 *  written to a data array, accessed via an 'index'. Data are copied,
 *  into non-volatile memory (FLASH) and a pointer reference is
 *  returned to the data-array of this variable in FLASH. The actual
 *  size is stored too, to be returned in a 'read' for this variable.
 *  A system variable is being created with the first 'write' operation
 *  to this variable and deleted, if any succeeding 'write' to
 *  this 'index' contains a 'size' parameter of '0'.
 *
 *
 *  Parameters :
 *  ------------
 *
 *  'var',          INOUT, data are copied into non-volatile memory
 *                         and the storage-pointer for this memory
 *                         is being returned in the 'data_inout'
 *                         parameter of the parameter block.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *
 *
 ************************************************************************/
INT32 SYSENV_write( t_user_environment_var *var )
{
    int   i ;
    INT32 completion, rcode = OK ;
    UINT32 header;
    UINT8 *pf ;
    UINT8 *pdata ;
    UINT8 checksum ;
    t_FLASH_write_descriptor flash_write ;
    t_FLASH_ctrl_descriptor  flash_ctrl ;

    if (SYSENV_state != SYSENV_STATE_OK)
    {
        return( ERROR_SYSENV_FLASH_INVALID ) ;
    }

    /* validate user parameters */
    if ( var->index > SYS_USER_ENVIRONMENT_MAX_INDEX )
    {
        /* Index is out of valid range */
        return( ERROR_SYSENV_INVALID_INDEX ) ;
    }

    if ( var->size > SYS_USER_ENVIRONMENT_DATA_SIZE )
    {
        /* Size is too big */
        return( ERROR_SYSENV_ENV_VAR_TOO_BIG ) ;
    }

    /* Index and size is OK, just try and store the context */
    if (SYSENV_next_free < SYSENV_max_records)
    {
        /* normal case */
        completion = OK ;
    }
    else
    {
        /* This is the garbage collection case */
        completion = SYSENV_collect_garbage() ;
        if (completion != ERROR_SYSENV_UPDATE_READ_REFS)
        {
            return(completion) ;
        }
    }

    /* format buffer */
    pdata    = var->data_inout ;
    checksum = 0 ;
    for (i=0; i<var->size; i++)
    {
        checksum += pdata[i] ;
    }
    /* Header is independent of endianess */
    REGP(SYSENV_write_buffer,SYSENV_HEADER) = 
              (SYSENV_RECORD_IN_USE << SYSENV_HEADER_CONTROL_SHF) |
              (var->index           << SYSENV_HEADER_ID_SHF)      |
              (checksum             << SYSENV_HEADER_CHKSUM_SHF)  |
              (var->size            << SYSENV_HEADER_SIZE_SHF);
    memcpy( &SYSENV_write_buffer[ SYSENV_HEADER_SIZE ], var->data_inout , var->size ) ;

    /* get pointer to file flash record */
    pf = SYSENV_file_flash_start + (SYSENV_next_free * sizeof(t_SYSENV_elem)) ;
            
    /* store record in file flash */
    flash_write.adr     = PHYS(pf) ;
    flash_write.length  = var->size + SYSENV_HEADER_SIZE ;
    flash_write.buffer  = SYSENV_write_buffer ;
    flash_ctrl.command  = FLASH_CTRL_WRITE_FILEFLASH ;
    flash_ctrl.wr_param = &flash_write ;
    rcode = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, &flash_ctrl ) ;
    if (rcode != OK)
    {
        SYSENV_state = SYSENV_STATE_ERROR ;
        return(rcode) ;
    }

    /* update lookup table */
    SYSENV_lookup[ var->index ].flash_record = pf ; 

    /* update user reference */
    var->data_inout = (void*)(pf + SYSENV_HEADER_SIZE) ;

    /* Paranoid checking.. */
    header = *(UINT32*)pf;
    rcode = SYSENV_check_inuse_record( pf, REGFIELD(header,SYSENV_HEADER_ID) ) ;
    if (rcode != OK)
    {
        return(rcode) ;
    }

    /* advance to next free record in file flash */
    SYSENV_next_free++ ;

    return( completion ) ;
}


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          SYSENV_collect_garbage
 *  Description :
 *  -------------
 *
 *  Execute garbage collection.
 *
 *
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
 *  'ERROR_SYSENV_UPDATE_READ_REFS', all references for env variables 
                                     must be re-read.
 *
 ************************************************************************/
static INT32 SYSENV_collect_garbage( void )
{
    int            i ;
    INT32          rcode ;
    UINT32         header;
    t_SYSENV_elem *pf ;
    t_SYSENV_elem *pr ;
    t_FLASH_ctrl_descriptor  flash_ctrl ;
    t_FLASH_write_descriptor flash_write ;


    /* cache any variable in RAM */
    for ( i = 0; i <= SYS_USER_ENVIRONMENT_MAX_INDEX ;  i++ )
    {
        pf = SYSENV_lookup[ i ].flash_record ;
        pr = SYSENV_lookup[ i ].ram_record ;
        if ( pf != NULL )
        {
            /* copy flash data to RAM */
            header = *(UINT32*)pf;
            memcpy( pr, pf,
                    REGFIELD(header,SYSENV_HEADER_SIZE) + SYSENV_HEADER_SIZE ) ;

            /* delete reference */
            SYSENV_lookup[i].flash_record = NULL ;
        }
        else
        {
            /* set size to '0' to delete record */
            REGP(pr,SYSENV_HEADER) &= ~SYSENV_HEADER_SIZE_MSK;
        }
    }

    /* delete file flash */
    flash_ctrl.command = FLASH_CTRL_ERASE_FILEFLASH ;
    rcode = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, &flash_ctrl ) ;
    if (rcode != OK)
    {
        return(rcode) ;
    }

    /* reset context */
    SYSENV_next_free = 0 ;

    /* copy cached data back into flash */
    for ( i = 0; i <= SYS_USER_ENVIRONMENT_MAX_INDEX ;  i++ )
    {
        pr = SYSENV_lookup[ i ].ram_record ;
        header = *(UINT32*)pr;
        if (REGFIELD(header,SYSENV_HEADER_SIZE) != 0)
        {
            /* store cached data in flash */
            pf = SYSENV_file_flash_start + (SYSENV_next_free * sizeof(t_SYSENV_elem)) ;
            pr = SYSENV_lookup[ i ].ram_record ;
            flash_write.adr     = PHYS(pf) ;
            flash_write.length  = REGFIELD(header,SYSENV_HEADER_SIZE) + SYSENV_HEADER_SIZE ;
            flash_write.buffer  = (UINT8*)pr;
            flash_ctrl.command  = FLASH_CTRL_WRITE_FILEFLASH ;
            flash_ctrl.wr_param = &flash_write ;
            rcode = IO_ctrl( SYS_MAJOR_FLASH_STRATA, 0, &flash_ctrl ) ;
            if (rcode != OK)
            {
                return(rcode) ;
            }

            /* update reference */
            SYSENV_lookup[i].flash_record = pf ;

            /* advance to next free record in file flash */
            SYSENV_next_free++ ;
        }
        else
        {
            /* update reference */
            SYSENV_lookup[i].flash_record = NULL ;
        }
    }
    return(ERROR_SYSENV_UPDATE_READ_REFS) ;
}


/************************************************************************
 *
 *                          SYSENV_error_lookup
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
INT32 SYSENV_error_lookup( t_sys_error_string *p_param )
{
    UINT32 t, i ;

    i = 0 ;
    p_param->count = 0 ;
    t = SYSERROR_ID( p_param->syserror ) ;

    /* check for recognized error code */
    if (t < sizeof(sysenv_error_string)/sizeof(char*) )
    {
        /* fill in mandatory error message string */
        p_param->strings[SYSCON_ERRORMSG_IDX] = sysenv_error_string[t] ;
        i++ ;

        /* check for hint message */
        if ( sysenv_error_hint_string[t] != NULL)
        {
            /* fill in optional hint message string */
            p_param->strings[SYSCON_HINTMSG_IDX] = sysenv_error_hint_string[t] ;
            i++ ;
        }
    }
    p_param->count      = i ;
    return(OK) ;
}


/************************************************************************
 *
 *                          SYSENV_check_inuse_record
 *  Description :
 *  -------------
 *  Check referenced record
 * 
 *
 *  Parameters :
 *  ------------
 *
 *  'ph',      INOUT,    environment flash record.
 *  'index',   INOUT,    environment flash record index.
 *
 *
 *  Return values :
 *  ---------------
 *
 * 'ERROR_SYSENV_FLASH_INVALID':  Environment is corrupted.
 * 'OK' = 0x00: 
 *
 *
 ************************************************************************/
static
INT32 SYSENV_check_inuse_record( UINT8 *ph, UINT8 index )
{
    UINT32 i ;
    UINT32 header   ; 
    UINT32 size     ; 
    UINT8  checksum ;
    UINT8  *pf ;

    header = *(UINT32*)ph;

    /* check control */
    if ( REGFIELD(header,SYSENV_HEADER_CONTROL) == SYSENV_RECORD_IN_USE )
    { 
        /* check index */
        if ( REGFIELD(header,SYSENV_HEADER_ID) == index )
        {
            /* check size: */
            size = REGFIELD(header,SYSENV_HEADER_SIZE) ;
            pf   = ph + SYSENV_HEADER_SIZE  ;
            if ( size <= SYS_USER_ENVIRONMENT_DATA_SIZE )
            {
                /* calculate checksum */
		for (checksum = 0; size > 3; size -= 4, pf += 4)
		{
		    i = *(UINT32*)pf;
		    checksum += (i>>24) + (i>>16) + (i>>8) + i;
		}
		if (size > 0)
		{
		    i = *(UINT32*)pf;
		    switch (size)
		    {
			case 3: checksum += i >> 16; /* fall through */
			case 2: checksum += i >> 8;  /* fall through */
			case 1: checksum += i;
		    }
		}

                /* check checksum */
                if ( checksum == REGFIELD(header,SYSENV_HEADER_CHKSUM) )
                {
                    /* OK, header is valid */
                    return( OK ) ;
                }
            }
        }
    }

    /* This is an error, file FLASH is corrupted ! */

    /* Set SYSENV state = 'ERROR' */
    SYSENV_state = SYSENV_STATE_ERROR ;

    return( ERROR_SYSENV_FLASH_INVALID ) ;
}
