
/************************************************************************
 *
 *  syscon.c
 *
 *  SYSCON module, generic parts (except TTY related parts)
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
 *  Include files
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sysdefs.h>
#include <sysdev.h>
#include <syscon_api.h>
#include <syscon.h>
#include <sys_api.h>
#include <syserror.h>
#include <string.h>
#include <product.h>
#include <sysenv_api.h>
#include <env_api.h>
#include <eeprom_api.h>
#include <io_api.h>
#include <spd.h>
#include <excep_api.h>
#include <net_api.h>


/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* stepping stones to reach routines linked in flash address area */
UINT32 (*flash_determine_icache_linesize)(UINT32) = sys_determine_icache_linesize_flash;
UINT32 (*flash_determine_icache_lines)(UINT32) = sys_determine_icache_lines_flash;
UINT32 (*flash_determine_icache_assoc)(UINT32) = sys_determine_icache_assoc_flash;
UINT32 (*flash_determine_dcache_linesize)(UINT32) = sys_determine_dcache_linesize_flash;
UINT32 (*flash_determine_dcache_lines)(UINT32) = sys_determine_dcache_lines_flash;
UINT32 (*flash_determine_dcache_assoc)(UINT32) = sys_determine_dcache_assoc_flash;
UINT32 (*flash_determine_scache_linesize)(UINT32) = sys_determine_l2cache_linesize_flash;
UINT32 (*flash_determine_scache_lines)(UINT32) = sys_determine_l2cache_lines_flash;
UINT32 (*flash_determine_scache_assoc)(UINT32) = sys_determine_l2cache_assoc_flash;


/* Constant used for determining endianness */
static const UINT32 endian_test = 0x12345678;

/* Reset value of CP0 config register */
static UINT32 config_init;

/* Memory allocation pointers */
static void *SYSCON_free;
static void *SYSCON_last;

/*  Keeps the memory chunk, to be allocated dynamically during 
 *  system initialization.
 */
static UINT8  SYSCON_dynamic_memory[SYS_MALLOC_DYNAMIC_MEMORY_SIZE];

/* Used for generic error string formatting */
static UINT8 syscon_generic_error_string[200] ;

/* Keeps the registered sys_error lookup function per subsystem */
static t_sys_error_lookup registered_lookup[SYSERROR_DOMAIN_COUNT] ;

/* Syscon error strings */
static char* syscon_error_string[] =
{
    /* ERROR_SYSCON_OUT_OF_MEMORY    */ "Internal: Out of memory (SYSCON)",
    /* ERROR_SYSCON_UNKNOWN_PARAM    */ "Internal: Invalid ID (SYSCON)",
    /* ERROR_SYSCON_IP_UNKNOWN	     */	"IP address unknown",
    /* ERROR_SYSCON_SUBNET_UNKNOWN   */	"Subnetmask unknown",
    /* ERROR_SYSCON_GATEWAY_UNKNOWN  */	"Default gateway unknown",
    /* ERROR_SYSCON_READ_ONLY_ID     */ "Internal: Read-only ID (SYSCON)",
    /* ERROR_SYSCON_WRITE_ONLY_ID    */ "Internal: Write-only ID (SYSCON)",
    /* ERROR_SYSCON_ENV_VAR_TOO_BIG  */ "Internal: Size of env. var too big (SYSCON)",
    /* ERROR_SYSCON_INVALID_CHAR_POS */ "Internal: Invalid character position (SYSCON)"
};

/* Array for the SYSCON objects */
static t_syscon_obj  syscon_objects[SYSCON_OBJ_COUNT];

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static INT32 
spd_read( 
    UINT32  minor,		/* Minor device number			*/
    UINT32  offset,		/* Byte position			*/
    UINT32  length,		/* Byte count				*/
    void    *user_variable );   /* Buffer for data			*/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          spd_read
 *  Description :
 *  -------------
 *
 *  Read SPD eeprom data by calling EEPROM driver
 *
 *  Return values :
 *  ---------------
 *
 *  Driver return value (OK in case of no error)
 *
 ************************************************************************/
static INT32 
spd_read( 
    UINT32  minor,		/* Minor device number			*/
    UINT32  offset,		/* Byte position			*/
    UINT32  length,		/* Byte count				*/
    void    *user_variable )    /* Buffer for data			*/
{
    t_EEPROM_read_descriptor readbuf;

    readbuf.offset = offset;
    readbuf.length = length;
    readbuf.buffer = user_variable;

    return IO_read( SYS_MAJOR_EEPROM_IIC, minor, &readbuf );
}


/************************************************************************
 *  Implementation : Static functions registered for handling particular 
 *  SYSCON objects for particular platform(s)
 ************************************************************************/


/************************************************************************
 *  board_sn_generic_read
 ************************************************************************/
static UINT32
board_sn_generic_read( 
    void   *param,
    void   *data,
    UINT32 size )
{
    t_sn_bcd  *sn;
    char      *msg = (char *)param;
    UINT8     number[3];
    UINT32    i;

    syscon_get_eeprom_data( NULL, &sn );
  
    if( sn != NULL )
    {
        msg[0] = '\0';
	for( i=0; i < SYS_SN_BCD_SIZE; i++ )
        {
            sprintf( number, "%d%d", (*sn)[i] >> 4, (*sn)[i] & 0xF );
	    strcat( msg, number );
        }

	return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *  com_en0_mac_addr_generic_read
 ************************************************************************/
static UINT32
com_en0_mac_addr_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    t_mac_addr *mac_addr;

    syscon_get_eeprom_data( &mac_addr, NULL );
  
    if( mac_addr != NULL )
    {
        memcpy( param, mac_addr, sizeof(t_mac_addr) );
	return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *  com_en0_ip_addr_generic_read
 ************************************************************************/
static UINT32
com_en0_ip_addr_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return
        env_get( "ipaddr", NULL, param, sizeof(UINT32) ) ?
	    OK : ERROR_SYSCON_IP_UNKNOWN;
}


/************************************************************************
 *  com_en0_ip_subnetmask_generic_read
 ************************************************************************/
static UINT32
com_en0_ip_subnetmask_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return 
        env_get( "subnetmask", NULL, param, sizeof(UINT32) ) ?
	    OK : ERROR_SYSCON_SUBNET_UNKNOWN;
}


/************************************************************************
 *  com_en0_ip_gatewayaddr_generic_read
 ************************************************************************/
static UINT32
com_en0_ip_gatewayaddr_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return
        env_get( "gateway", NULL, param, sizeof(UINT32) ) ?
	    OK : ERROR_SYSCON_GATEWAY_UNKNOWN;
}


/************************************************************************
 *  com_en0_enable
 ************************************************************************/
static UINT32
com_en0_enable(
    void   *param,
    void   *data,
    UINT32 size )
{
    sys_dma_enable( TRUE ) ;
    return( OK ) ;
}


/************************************************************************
 *  com_en0_disable
 ************************************************************************/
static UINT32
com_en0_disable(
    void   *param,
    void   *data,
    UINT32 size )
{
    sys_dma_enable( FALSE ) ;
    return( OK ) ;
}


/************************************************************************
 *  board_malloc_generic_read
 ************************************************************************/
static UINT32
board_malloc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    t_sys_malloc *p_mem = param;
    unsigned     fraction, number;

    *(p_mem->memory) = SYSCON_free;
    fraction	     =  (unsigned) SYSCON_free % (unsigned) p_mem->boundary;
    number           =  (unsigned) SYSCON_free / (unsigned) p_mem->boundary;

    if(fraction != 0)
    {
        /* align pointer */
        *(p_mem->memory) = (void*)((number + 1) * p_mem->boundary);
    }

    /* Check fullfillment of memory request */
    if((unsigned)((*(p_mem->memory)) + p_mem->size) > (unsigned) SYSCON_last)
    {
        p_mem->memory = NULL;
        return ERROR_SYSCON_OUT_OF_MEMORY;
    }
    else
    {
        /* allocate memory */
        SYSCON_free = (*(p_mem->memory)) + p_mem->size;
	return OK;
    }
}


/************************************************************************
 *  board_free_mem_generic_read
 ************************************************************************/
static UINT32
board_free_mem_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_freemem;
    return OK;
}


/************************************************************************
 *  board_appl_stack_size_generic_read
 ************************************************************************/
static UINT32
board_appl_stack_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SYS_APPL_STACK_SIZE;
    return OK;
}


/************************************************************************
 *  board_stack_size_generic_read
 ************************************************************************/
static UINT32
board_stack_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SYS_STACK_SIZE;
    return OK;
}


/************************************************************************
 *  board_cpu_clock_freq_generic_read
 ************************************************************************/
static UINT32
board_cpu_clock_freq_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_cpufreq_hz;
    return OK;
}


/************************************************************************
 *  board_bus_clock_freq_generic_read
 ************************************************************************/
static UINT32
board_bus_clock_freq_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_busfreq_hz;
    return OK;
}


/************************************************************************
 *  cpu_icache_size_generic_read
 ************************************************************************/
static UINT32
cpu_icache_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_icache_linesize * sys_icache_lines;
    return OK;
}


/************************************************************************
 *  cpu_icache_linesize_generic_read
 ************************************************************************/
static UINT32
cpu_icache_linesize_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_icache_linesize;
    return OK;
}


/************************************************************************
 *  cpu_icache_assoc_generic_read
 ************************************************************************/
static UINT32
cpu_icache_assoc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_icache_assoc;
    return OK;
}


/************************************************************************
 *  cpu_icache_avail_bpw_generic_read
 ************************************************************************/
static UINT32
cpu_icache_avail_bpw_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{ 
    sys_cpu_cache_bpw( TRUE, (t_sys_array *)param );
    return OK;
}


/************************************************************************
 *  cpu_icache_avail_assoc_generic_read
 ************************************************************************/
static UINT32
cpu_icache_avail_assoc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    sys_cpu_cache_assoc( TRUE, (t_sys_array *)param );
    return OK;
}


/************************************************************************
 *  cpu_icache_size_current_generic_read
 ************************************************************************/
static UINT32
cpu_icache_size_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_icache_linesize)(sys_processor) *
		       (*flash_determine_icache_lines)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_icache_linesize_current_generic_read
 ************************************************************************/
static UINT32
cpu_icache_linesize_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_icache_linesize)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_icache_assoc_current_generic_read
 ************************************************************************/
static UINT32
cpu_icache_assoc_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_icache_assoc)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_dcache_size_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_dcache_linesize * sys_dcache_lines;
    return OK;
}


/************************************************************************
 *  cpu_dcache_linesize_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_linesize_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_dcache_linesize;
    return OK;
}


/************************************************************************
 *  cpu_dcache_assoc_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_assoc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_dcache_assoc;
    return OK;
}


/************************************************************************
 *  cpu_dcache_avail_bpw_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_avail_bpw_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    sys_cpu_cache_bpw( FALSE, (t_sys_array *)param );
    return OK;
}


/************************************************************************
 *  cpu_dcache_avail_assoc_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_avail_assoc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    sys_cpu_cache_assoc( FALSE, (t_sys_array *)param );
    return OK;
}


/************************************************************************
 *  cpu_dcache_size_current_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_size_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_dcache_linesize)(sys_processor) *
		       (*flash_determine_dcache_lines)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_dcache_linesize_current_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_linesize_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_dcache_linesize)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_dcache_assoc_current_generic_read
 ************************************************************************/
static UINT32
cpu_dcache_assoc_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (*flash_determine_dcache_assoc)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_scache_size_generic_read
 ************************************************************************/
static UINT32
cpu_scache_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = sys_l2cache_linesize * sys_l2cache_lines;
    return OK;
}


/************************************************************************
 *  cpu_scache_linesize_generic_read
 ************************************************************************/
static UINT32
cpu_scache_linesize_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = sys_l2cache_linesize;
    return OK;
}


/************************************************************************
 *  cpu_scache_assoc_generic_read
 ************************************************************************/
static UINT32
cpu_scache_assoc_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = sys_l2cache_assoc;
    return OK;
}


/************************************************************************
 *  cpu_scache_size_current_generic_read
 ************************************************************************/
static UINT32
cpu_scache_size_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = (*flash_determine_scache_linesize)(sys_processor) *
		       (*flash_determine_scache_lines)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_scache_linesize_current_generic_read
 ************************************************************************/
static UINT32
cpu_scache_linesize_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = (*flash_determine_scache_linesize)(sys_processor);
    return OK;
}


/************************************************************************
 *  cpu_scache_assoc_current_generic_read
 ************************************************************************/
static UINT32
cpu_scache_assoc_current_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( !sys_l2cache )
        return ERROR_SYSCON_UNKNOWN_PARAM;

    *(UINT32 *)param = (*flash_determine_scache_assoc)(sys_processor);
    return OK;
}


/************************************************************************
 *  board_systemram_actual_size_generic_read
 ************************************************************************/
static UINT32
board_systemram_actual_size_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = sys_ramsize;
    return OK;
}


/************************************************************************
 *  cpu_cp0_config_reset_generic_read
 ************************************************************************/
static UINT32
cpu_cp0_config_reset_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = config_init;
    return OK;
}


/************************************************************************
 *  disk_environment_generic_read
 ************************************************************************/
static UINT32
disk_environment_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return SYSENV_read( param );
}

/************************************************************************
 *  file_batch_access_generic_read
 ************************************************************************/
static UINT32
file_batch_access_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    t_sys_batch_file_descriptor *dp ;
    UINT32 rc;
    UINT32 old_ie;

    /* Allow interrupts in application interface routine */
    old_ie = sys_disable_int();
    sys_enable_int();

    dp = param ;
    if (dp->mode == SYSCON_FILE_READ_MODE)
    {
        rc = NET_file_read( dp->host_ip,
                            dp->file_name,
                            dp->buffer,
                            &(dp->buffer_size) ) ;
    }
    else
    {
        rc = NET_file_write( dp->host_ip,
                             dp->file_name,
                             dp->buffer,
                             &(dp->buffer_size) ) ;
    }

    /* Restore interrupt condition */
    if (!old_ie) sys_disable_int();

    return rc;
}


/************************************************************************
 *  file_bootserver_generic_read
 ************************************************************************/
static UINT32
file_bootserver_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return
        env_get( "bootserver", NULL, param, sizeof(UINT32) ) ?
            OK : ERROR_SYSCON_IP_UNKNOWN;
}


/************************************************************************
 *  disk_environment_generic_write
 ************************************************************************/
static UINT32
disk_environment_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    return SYSENV_write( param );
}


/************************************************************************
 *  error_register_lookup_generic_write
 ************************************************************************/
static UINT32
error_register_lookup_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 tl;

    /* write user environment data */
    tl = ((t_sys_error_lookup_registration*)param)->prefix;

    if( tl < SYSERROR_DOMAIN_COUNT )
    {
        registered_lookup[tl] = 
	    ((t_sys_error_lookup_registration*)param)->lookup;

        return OK;
    }
    else
    {
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }
}


/************************************************************************
 *  error_lookup_generic_read
 ************************************************************************/
static UINT32
error_lookup_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 tl;
    UINT32 rcode;

    /* lookup syserror by registered domain service */ 
    tl = SYSERROR_DOMAIN( ((t_sys_error_string*)param)->syserror );
 
    if( ( tl < SYSERROR_DOMAIN_COUNT ) &&
        ( registered_lookup[tl] != NULL ) )
    {
        rcode = (*registered_lookup[tl])( (t_sys_error_string*)param );

        if ( (   rcode != OK) ||
             ( ((t_sys_error_string*)param)->count == 0 ) )
        {
            /* Force internal error message */
            tl = ((t_sys_error_string*)param)->syserror ;
            sprintf( syscon_generic_error_string, "Internal, code = %x", tl ) ;
            ((t_sys_error_string*)param)->strings[SYSCON_ERRORMSG_IDX] = 
                        syscon_generic_error_string ;
            ((t_sys_error_string*)param)->count = 1 ;
        }
	return OK;
    }
    else
    {
        /* Force internal error message */
        tl = ((t_sys_error_string*)param)->syserror ;
        sprintf( syscon_generic_error_string, "Internal, code = %x", tl ) ;
        ((t_sys_error_string*)param)->strings[SYSCON_ERRORMSG_IDX] =
                    syscon_generic_error_string ;
        ((t_sys_error_string*)param)->count = 1 ;
        return OK ;
    }
}


/************************************************************************
 *
 *                          SYSCON_error_lookup
 *  Description :
 *  -------------
 *  Lookup error code to error string(s)
 *
 *  Parameters :
 *  ------------
 * 
 *  'param',   INOUT,    variable of type, t_sys_error_string.
 *
 *  Return values :
 *  ---------------
 *
 * 'OK' = 0x00:  
 *
 ************************************************************************/
static INT32 
SYSCON_error_lookup( 
    t_sys_error_string *param )
{
    UINT32 t;

    param->count = 0;

    t = SYSERROR_ID( param->syserror );

    if( t < sizeof(syscon_error_string)/sizeof(char*) )
    {
        param->strings[SYSCON_ERRORMSG_IDX] = syscon_error_string[t];
        param->count = 1;
    }

    return(OK);
}


/************************************************************************
 *  board_systemram_sras2scas_ns_spd_read, 
 ************************************************************************/
static UINT32
board_systemram_sras2scas_ns_spd_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT8 data8;
    INT32 rc;
    
    rc = spd_read( EEPROM_MINOR_SPD000, SPD_RCDM, 1, (void *)&data8 );

    *(UINT32 *)param = data8;
     
    return rc;
}


/************************************************************************
 *  board_systemram_srasprchg_ns_spd_read, 
 ************************************************************************/
static UINT32
board_systemram_srasprchg_ns_spd_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT8 data8;
    INT32 rc;
    
    rc = spd_read( EEPROM_MINOR_SPD000, SPD_MRPT, 1, (void *)&data8 );

    *(UINT32 *)param = data8;
     
    return rc;
}


/************************************************************************
 *  board_systemram_refresh_ns_spd_read, 
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_spd_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT8 data8;
    INT32 rc;
    
    rc = spd_read( EEPROM_MINOR_SPD000, SPD_RFSH_RT, 1, (void *)&data8 );

    if( rc != OK )
        return rc;

    switch( REGFIELD( data8, SPD_RFSH_RT_RATE ) )
    {
      case SPD_RFSH_RT_RATE_125 :
      	/* 125 us */
	*(UINT32 *)param = 125000;
	break;

      case SPD_RFSH_RT_RATE_62_5 :
      	/* 62.5 us */
	*(UINT32 *)param = 62500;
	break;

      case SPD_RFSH_RT_RATE_31_3 :
	/* 31.3 us */
	*(UINT32 *)param = 31300;
	break;

      case SPD_RFSH_RT_RATE_15_625 :
	/* 15.625 us */
	*(UINT32 *)param = 15625;
	break;

      case SPD_RFSH_RT_RATE_7_8 :
	/* 7.8 us */
	*(UINT32 *)param = 7800;
	break;

      default : 
        /* Assume 3.9us */
	*(UINT32 *)param = 3900;
	break;
    }

    return OK;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/*  Functions of the format required for registered SYSCON functions.
 *  Local to SYSCON module.
 */

UINT32
syscon_true_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = TRUE;
    return OK;
}

UINT32
syscon_false_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = FALSE;
    return OK;
}

UINT32
syscon_endian_big_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = ((*(UINT8 *)(&endian_test) == 0x12) ? TRUE : FALSE );
    return OK;
}

UINT32
syscon_string_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(char **)param = (char *)data;
    return OK;
}

UINT32
syscon_uint8_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = *(UINT8 *)data;
    return OK;
}

UINT32
syscon_uint32_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = *(UINT32 *)data;
    return OK;
}

UINT32
syscon_bool_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = *(bool *)data;
    return OK;
}

UINT32
syscon_bool_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)data = *(bool *)param;
    return OK;
}


/************************************************************************
 *
 *
 *                          SYSCON_init
 *  Description :
 *  -------------
 *
 *  Initializes the 'syscon' module.
 *  
 *  Parameters :
 *  ------------
 *
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), successfull initialization
 *
 ************************************************************************/
INT32 
SYSCON_init( void )
{
    t_sys_error_lookup_registration registration;
    UINT32 i;

    /* initialize free and last pointer for dynamic memory allocation */
    SYSCON_free = &SYSCON_dynamic_memory[0] ;
    SYSCON_last = &SYSCON_dynamic_memory[SYS_MALLOC_DYNAMIC_MEMORY_SIZE-1];

    /* Register objects */

    syscon_register_generic( SYSCON_BOARD_STACK_SIZE_ID, 
			     board_stack_size_generic_read, NULL,
			     NULL,			    NULL );

    syscon_register_generic( SYSCON_BOARD_APPL_STACK_SIZE_ID,
			     board_appl_stack_size_generic_read, NULL,
			     NULL,				 NULL );

    syscon_register_generic( SYSCON_BOARD_FREE_MEM_ID,
			     board_free_mem_generic_read, NULL,
			     NULL,			  NULL );

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID,
			     board_systemram_actual_size_generic_read, NULL,
			     NULL,				       NULL );

    syscon_register_generic( SYSCON_CPU_ENDIAN_BIG_ID,
			     syscon_endian_big_read, NULL,
			     NULL,		     NULL );

    syscon_register_generic( SYSCON_DISK_ENVIRONMENT_ID,
			     disk_environment_generic_read,  NULL,
			     disk_environment_generic_write, NULL );

    syscon_register_generic( SYSCON_ERROR_REGISTER_LOOKUP_ID,
			     NULL,				  NULL,
			     error_register_lookup_generic_write, NULL );

    syscon_register_generic( SYSCON_ERROR_LOOKUP_ID,
			     error_lookup_generic_read, NULL,
			     NULL,			NULL );

    syscon_register_generic( SYSCON_BOARD_SN_ID,
			     board_sn_generic_read, NULL,
			     NULL,		    NULL );

    syscon_register_generic( SYSCON_COM_EN0_MAC_ADDR_ID,
			     com_en0_mac_addr_generic_read, NULL,
			     NULL,			    NULL );

    syscon_register_generic( SYSCON_BOARD_MALLOC_ID,
			     board_malloc_generic_read, NULL,
			     NULL,			NULL );

    syscon_register_generic( SYSCON_BOARD_CPU_CLOCK_FREQ_ID,
			     board_cpu_clock_freq_generic_read, NULL,
			     NULL,				NULL );

    syscon_register_generic( SYSCON_BOARD_BUS_CLOCK_FREQ_ID,
			     board_bus_clock_freq_generic_read, NULL,
			     NULL,				NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_SIZE_ID,
			     cpu_icache_size_generic_read, NULL,
			     NULL,			   NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_LINESIZE_ID,
			     cpu_icache_linesize_generic_read, NULL,
			     NULL,			       NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_ASSOC_ID,
			     cpu_icache_assoc_generic_read, NULL,
			     NULL,			    NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_AVAIL_BPW_ID,
			     cpu_icache_avail_bpw_generic_read, NULL,
			     NULL,				NULL );
    
    syscon_register_generic( SYSCON_CPU_ICACHE_AVAIL_ASSOC_ID,
			     cpu_icache_avail_assoc_generic_read, NULL,
			     NULL,				  NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_SIZE_CURRENT_ID,
			     cpu_icache_size_current_generic_read, NULL,
			     NULL,				   NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_LINESIZE_CURRENT_ID,
			     cpu_icache_linesize_current_generic_read, NULL,
			     NULL,				       NULL );

    syscon_register_generic( SYSCON_CPU_ICACHE_ASSOC_CURRENT_ID,
			     cpu_icache_assoc_current_generic_read, NULL,
			     NULL,				    NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_SIZE_ID,
			     cpu_dcache_size_generic_read, NULL,
			     NULL,			   NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_LINESIZE_ID,
			     cpu_dcache_linesize_generic_read, NULL,
			     NULL,			       NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_ASSOC_ID,
			     cpu_dcache_assoc_generic_read, NULL,
			     NULL,			    NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_AVAIL_BPW_ID,
			     cpu_dcache_avail_bpw_generic_read, NULL,
			     NULL,				NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_AVAIL_ASSOC_ID,
			     cpu_dcache_avail_assoc_generic_read, NULL,
			     NULL,				  NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_SIZE_CURRENT_ID,
			     cpu_dcache_size_current_generic_read, NULL,
			     NULL,				   NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_LINESIZE_CURRENT_ID,
			     cpu_dcache_linesize_current_generic_read, NULL,
			     NULL,				       NULL );

    syscon_register_generic( SYSCON_CPU_DCACHE_ASSOC_CURRENT_ID,
			     cpu_dcache_assoc_current_generic_read, NULL,
			     NULL,				    NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_SIZE_ID,
			     cpu_scache_size_generic_read, NULL,
			     NULL,				   NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_LINESIZE_ID,
			     cpu_scache_linesize_generic_read, NULL,
			     NULL,				       NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_ASSOC_ID,
			     cpu_scache_assoc_generic_read, NULL,
			     NULL,				    NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_SIZE_CURRENT_ID,
			     cpu_scache_size_current_generic_read, NULL,
			     NULL,				   NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_LINESIZE_CURRENT_ID,
			     cpu_scache_linesize_current_generic_read, NULL,
			     NULL,				       NULL );

    syscon_register_generic( SYSCON_CPU_SCACHE_ASSOC_CURRENT_ID,
			     cpu_scache_assoc_current_generic_read, NULL,
			     NULL,				    NULL );

    syscon_register_generic( SYSCON_COM_EN0_IP_ADDR_ID,
			     com_en0_ip_addr_generic_read, NULL,
			     NULL,			   NULL );

    syscon_register_generic( SYSCON_COM_EN0_IP_SUBNETMASK_ID,
			     com_en0_ip_subnetmask_generic_read, NULL,
			     NULL,				 NULL );

    syscon_register_generic( SYSCON_COM_EN0_IP_GATEWAYADDR_ID,
			     com_en0_ip_gatewayaddr_generic_read, NULL,
			     NULL,				  NULL );

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_SPD_ID,
			     board_systemram_refresh_ns_spd_read, NULL,
			     NULL,			          NULL );

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_NS_SPD_ID,
			     board_systemram_srasprchg_ns_spd_read, NULL,
			     NULL,			            NULL );

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_NS_SPD_ID,
			     board_systemram_sras2scas_ns_spd_read, NULL,
			     NULL,			            NULL );

    syscon_register_generic( SYSCON_COM_EN0_ENABLE_ID,
                             com_en0_enable, NULL,
                             NULL,                                NULL );

    syscon_register_generic( SYSCON_COM_EN0_DISABLE_ID,
                             com_en0_disable, NULL,
                             NULL,                                NULL );

    syscon_register_generic( SYSCON_FILE_BATCH_ACCESS_ID,
                             file_batch_access_generic_read, NULL,
                             NULL,                                NULL );

    syscon_register_generic( SYSCON_FILE_BOOTSERVER_ID,
                             file_bootserver_generic_read, NULL,
                             NULL,                                NULL );

    /* Reset value of Config */
    syscon_register_generic( SYSCON_CPU_CP0_CONFIG_RESET_ID,
			     cpu_cp0_config_reset_generic_read, NULL,
			     NULL,				NULL );

    /* Generic TTY */
    syscon_generic_tty_init();

    /* Platform specific, except TTY */
    syscon_arch_board_init(syscon_objects);

    /* Platform specific TTY */
    syscon_arch_tty_init();

    /* CPU specifics */
    syscon_arch_cpu_init(syscon_objects);

    /* reset lookup array */
    memset( (void*)&registered_lookup[0], 0, sizeof(registered_lookup) );

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_SYSCON );
    registration.lookup = SYSCON_error_lookup;

    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    /* Store initial setting of CP0 CONFIG register */
    SYSCON_read(
        SYSCON_CPU_CP0_CONFIG_ID,
	(void *)&config_init,
	sizeof(UINT32) );	

    return OK;
}


/************************************************************************
 *
 *                          SYSCON_read
 *  Description :
 *  -------------
 *
 *  Read the value of system configuration object given by 'id'.
 *
 *  Parameters :
 *  ------------
 *
 *  'id',           IN,    Object id.
 *  'param',        INOUT, Buffer for object value.
 *  'param_size',   IN,    Size of buffer (must match size of object).
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *  'ERROR_SYSCON_UNKNOWN_PARAM': parameter not available on this board
 *  'ERROR_SYSCON_SIZE_MISMATCH': parameter size check failed
 *
 ************************************************************************/
INT32 
SYSCON_read(
    t_syscon_ids  id,		/* IN: object ID			*/
    void          *param,       /* IN: Buffer for object value		*/
    UINT32	  param_size)   /* IN: Buffer size (bytes)		*/
{
    t_syscon_obj *obj;

    if( id >= SYSCON_OBJ_COUNT )
        return ERROR_SYSCON_UNKNOWN_PARAM;
    else
    {
        obj = &syscon_objects[id];

        if( obj->read )
	    return obj->read( param, obj->read_data, param_size );
	else
	    return ERROR_SYSCON_WRITE_ONLY_ID;
    }
}


/************************************************************************
 *
 *                          SYSCON_write
 *  Description :
 *  -------------
 *
 *  Write the system configuration object given by 'id'.
 *
 *  Parameters :
 *  ------------
 *
 *  'id',         IN,    Object id.
 *  'param',      IN,    Buffer containing value to write.
 *  'param_size', IN,    Size of buffer.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), parameter value has been set
 *  'ERROR_SYSCON_UNKNOWN_PARAM': parameter not available on this board
 *  'ERROR_SYSCON_SIZE_MISMATCH': parameter size check failed
 *
 ************************************************************************/
INT32 
SYSCON_write(
    t_syscon_ids  id,		/* IN: object ID			*/
    void          *param,       /* IN: Buffer with value		*/
    UINT32	  param_size)   /* IN: Size of buffer  (bytes)		*/
{
    t_syscon_obj *obj;

    if( id >= SYSCON_OBJ_COUNT )
        return ERROR_SYSCON_UNKNOWN_PARAM;
    else
    {
        obj = &syscon_objects[id];

        if( obj->write )
	    return obj->write( param, obj->write_data, param_size );
	else
	    return ERROR_SYSCON_READ_ONLY_ID;
    }
}


/************************************************************************
 *
 *                          syscon_register_generic
 *  Description :
 *  -------------
 *
 *  Function used to register generic SYSCON object functions.
 *
 *  A read and/or write function may be registered.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_generic(
    t_syscon_ids   id,			/* OBJECT ID from syscon_api.h	*/
    t_syscon_func  read_generic,	/* Generic read function	*/
    void	   *read_data,		/* Registered data		*/
    t_syscon_func  write_generic,	/* Generic write function	*/
    void	   *write_data )	/* Registered data		*/
{
    syscon_objects[id].read       = read_generic;
    syscon_objects[id].read_data  = read_data;
    syscon_objects[id].write      = write_generic;
    syscon_objects[id].write_data = write_data;
}


