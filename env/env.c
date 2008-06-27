
/************************************************************************
 *
 *  env.c
 *
 *  ENV module
 *
 *  This module implements YAMON environment variables. It will create
 *  default variables as well.
 *
 *  This file holds the generic part of the ENV module (except for TTY
 *  related parts, which are found in ./env_tty.c).
 *
 *  Platform specific parts are found in yamon/arch/env directory. 
 * 
 *  ENV uses the SYSENV module, which manages "raw" records.
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
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <syscon_api.h>
#include <sysenv_api.h>
#include <shell_api.h>  /* Due to env_print_all */
#include <env_api.h>
#include <env.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syserror.h>
#include <errno.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    UINT32        index;
    UINT8         attr;
    char          *default_value;
    t_env_decode  decode;
    char	  data[SYS_USER_ENVIRONMENT_DATA_SIZE];
}
t_env_var_info;

#define INDEX_NONE	      (SYS_USER_ENVIRONMENT_MAX_INDEX + 1)
#define INDEX_REARRANGED      (SYS_USER_ENVIRONMENT_MAX_INDEX + 2)

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char* error_string[] =
{
    /* ERROR_ENV_VAR_NAME_LEN */    "Too long variable name",
    /* ERROR_ENV_VAR_RO */	    "Read only variable",
    /* ERROR_ENV_VAR_VALUE */	    "Illegal value",
    /* ERROR_ENV_VAR_OVERFLOW */    "Too many environment variables",
    /* ERROR_ENV_VAR_VAL_LEN */     "Too long variable string"
};

static char	       *default_cpuconfig = "";

static bool            cache_configurable, mmu_configurable;
static UINT32	       config_init;

static t_env_var       env_vars[SYS_USER_ENVIRONMENT_MAX_INDEX + 2];
static t_env_var_info  env_var_info[SYS_USER_ENVIRONMENT_MAX_INDEX + 1];
static UINT32	       env_var_count;
static bool            env_corrupted;

/*  If TRUE, variables that are otherwise RO, may be written
 *  (used during initialisation of system variables).
 */
static bool	       ext_priviliges = FALSE;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static bool 
decode_number( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size,		/* Size of decoded data			*/
    UINT32 base );		/* Base (8/10/16)			*/

static bool 
hex_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/

static bool
validate_cache(
    bool   icache,		/* TRUE -> ICACHE, FALSE -> DCACHE      */
    UINT32 bpw,
    UINT32 assoc );


static bool
validate_cpuconfig(
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/

static INT32 
error_lookup( 
    t_sys_error_string *param );

static UINT32
access_var(
    char         *name,
    char	 **value,
    UINT8	 attr,
    char	 *default_value,
    t_env_decode decode );

static void
set_env_var(
    t_env_var      *env_var,
    t_env_var_info *info );

static bool
lookup_env(
    char   *name,
    UINT32 *index );

static int
compare(
    UINT32 *x,
    UINT32 *y );

static UINT32
remove_var(
    UINT32 index,
    bool   user,
    bool   system,
    bool   error_on_ro );

static UINT32 
env_to_flash(
    UINT32    index_local,
    UINT32    *index_flash );

static void 
env_remove_from_flash( 
    UINT32 index );

#ifdef EB
static void
convert_endianness( 
    char *s );
#endif

static void 
re_init( void );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          decode_number
 *  Description :
 *  -------------
 *
 *  Decode number
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool 
decode_number( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size,		/* Size of decoded data			*/
    UINT32 base )		/* Base (8/10/16)			*/
{
    char   *endp;
    UINT32 number;

    if( !raw )
        return FALSE;

    /* Do the decoding */
    errno = 0;

    number = strtoul( raw, &endp, base );

    if( decoded )
    {
        switch( size )
	{
	  case sizeof(UINT8) :
	    *(UINT8 *)decoded  = (UINT8)number;
	    break;
	  case sizeof(UINT16) :
	    *(UINT16 *)decoded = (UINT16)number;
	    break;
	  case sizeof(UINT32) :
	    *(UINT32 *)decoded = (UINT32)number;
	    break;
	  default :
	    return FALSE;
        }
    }  

    return
        ( errno || (*endp != '\0') ) ?
	    FALSE : TRUE;
}


/************************************************************************
 *
 *                          hex_s2num
 *
 *  Description :
 *  -------------
 *
 *  Decode decimal number
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool 
hex_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return decode_number( raw, decoded, size, 16 );
}


/************************************************************************
 *
 *                          validate_cache
 *  Description :
 *  -------------
 *
 *  Validate cache setting
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool
validate_cache(
    bool   icache,		/* TRUE -> ICACHE, FALSE -> DCACHE      */
    UINT32 bpw,
    UINT32 assoc )
{
    t_sys_array sys_array;
    UINT32	i;

    sys_cpu_cache_bpw( icache, &sys_array );

    for( i=0; 
	     ( i < sys_array.count ) &&
	     ( bpw != sys_array.array[i] );
	 i++ );
    
    if( i == sys_array.count ) return FALSE;

    sys_cpu_cache_assoc( icache, &sys_array );

    for( i=0; 
	     ( i < sys_array.count ) &&
	     ( assoc != sys_array.array[i] );
	 i++ );
    
    if( i == sys_array.count ) return FALSE;

    return TRUE;
}


/************************************************************************
 *
 *                          validate_cpuconfig
 *  Description :
 *  -------------
 *
 *  Validate and decode cpuconfig environment variable of format :
 *
 *  [<i_bpw>,<i_assoc>,<d_bpw>,<d_assoc>[,(tlb|fixed)]] | (tlb|fixed) 
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool
validate_cpuconfig(
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    char   *endp;
    UINT32 i_bpw, i_assoc, d_bpw, d_assoc;
    bool   mmu_tlb;
    
    if( !raw ) return FALSE;

    errno = 0;

    if( *raw == '\0' ) 
    {
        i_bpw   = sys_icache_lines/sys_icache_assoc*sys_icache_linesize; 
        i_assoc = sys_icache_assoc;
        d_bpw   = sys_dcache_lines/sys_dcache_assoc*sys_dcache_linesize; 
        d_assoc = sys_dcache_assoc;
        mmu_tlb = ((config_init >> 8) & 1) ? FALSE : TRUE;
    }
    else
    {
        if( cache_configurable )
        {
            /* Get i_bpw */
            i_bpw = (UINT32)strtoul( raw, &endp, 10 );

            if( (*endp != ',') || errno )
                return FALSE;
            endp++; /* Skip ',' */
            raw = endp;
            if( *endp == ' ' )
                return FALSE;

            /* Get i_assoc */    
            i_assoc = (UINT32)strtoul( raw, &endp, 10 );

            if( (*endp != ',') || errno )
                return FALSE;
            endp++; /* Skip ',' */
            raw = endp;
            if( *endp == ' ' )
                return FALSE;

            /* Get d_bpw */
            d_bpw = (UINT32)strtoul( raw, &endp, 10 );

            if( (*endp != ',') || errno )
                return FALSE;
            endp++; /* Skip ',' */
            raw = endp;
            if( *endp == ' ' )
                return FALSE;

            /* Get d_assoc */    
            d_assoc = (UINT32)strtoul( raw, &endp, 10 );

	    if( errno )
	        return FALSE;

	    if( mmu_configurable )
	    {
                if( *endp != ',' )
                    return FALSE;
                endp++; /* Skip ',' */
                raw = endp;
                if( *endp == ' ' )
                    return FALSE;
            }
   	    else
	    {
	        if( *endp != '\0' )
	            return FALSE;
            }

            /* Validate I-cache */
            if( !validate_cache( TRUE, i_bpw, i_assoc ) )
                return FALSE;

            /* Validate D-cache */
            if( !validate_cache( FALSE, d_bpw, d_assoc ) )
                return FALSE;
         }

         if( mmu_configurable )
         {
            /* Get mmu */
            if( strcmp( raw, "tlb" ) == 0 )
	        mmu_tlb = TRUE;
            else if( strcmp( raw, "fixed" ) == 0 )
	        mmu_tlb = FALSE;
	    else
	        return FALSE;
         }
    }

    /* Store decoded data */
    if( decoded )
    {
        if( size != sizeof( t_sys_cpu_decoded ) )
	    return FALSE;
	   
	((t_sys_cpu_decoded *)decoded)->i_bpw   = i_bpw;
	((t_sys_cpu_decoded *)decoded)->i_assoc = i_assoc;
	((t_sys_cpu_decoded *)decoded)->d_bpw   = d_bpw;
	((t_sys_cpu_decoded *)decoded)->d_assoc = d_assoc;
	((t_sys_cpu_decoded *)decoded)->mmu_tlb = mmu_tlb;
    }
        
    return TRUE;
}


/************************************************************************
 *
 *                          error_lookup
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
error_lookup( 
    t_sys_error_string *param )
{
    UINT32 t = SYSERROR_ID( param->syserror );

    /* check for recognized error code */
    if( t < sizeof(error_string)/sizeof(char *) )
    {
        param->count = 1;

        /* fill in mandatory error message string */
        param->strings[SYSCON_ERRORMSG_IDX] = error_string[t];
    }
    else
        param->count = 0;

    return OK;
}


/************************************************************************
 *                          access_var
 ************************************************************************/
static UINT32
access_var(
    char         *name,
    char	 **value,
    UINT8	 attr,
    char	 *default_value,
    t_env_decode decode )
{	 
    t_env_var      *env_var;
    t_env_var_info *info;
    UINT32	   index;
    UINT32	   index_flash;
    UINT32	   rc;
    char	   old_data[SYS_USER_ENVIRONMENT_DATA_SIZE];

    if( strlen(name) > ENV_VAR_MAXLEN_NAME )
    {
        return ERROR_ENV_VAR_NAME_LEN;   
    }

    /* Check length of data for buffer (name + value + 2 * '\0') */
    if( strlen(name) + strlen(*value) + 2 > SYS_USER_ENVIRONMENT_DATA_SIZE )
    {
        return ERROR_ENV_VAR_VAL_LEN;
    }
   
    if( lookup_env( name, &index ) )
    {
        /* Found */
	env_var = &env_vars[index];
	info    = &env_var_info[index];

	switch( info->attr )
	{
	  case ENV_ATTR_RO :
	    if( !ext_priviliges ) 
	        return ERROR_ENV_VAR_RO;

	  /**** Fall through ****/

	  case ENV_ATTR_RW :
	  case ENV_ATTR_USER :

	    if( strcmp( env_var->val, *value ) != 0 )
	    {
	        /* new value, check validity of data */
	        if( info->decode &&
		    !info->decode( *value, NULL, 0 ) )
	        {
	            return ERROR_ENV_VAR_VALUE;
	        }

	        /* Store old value */
	        strcpy( old_data, env_var->val );

	        /* Generate new data */
		strcpy( env_var->val, *value );

		/* Write to flash */
		rc = env_to_flash( index, &index_flash );

		if( rc != OK )
		{
	            /* Could not write, restore old data */
		    strcpy( env_var->val, old_data );
		    return rc;
		}
	    }
		
	    if( info->attr == ENV_ATTR_USER )
	    {
		info->default_value = default_value;
		info->decode        = decode;
		info->attr          = attr;
	    }

	    break;

	  default :
	    /* Ignore */
	    break;
        }
    }
    else
    {
        /* Not found, so create it */
	if( env_var_count == SYS_USER_ENVIRONMENT_MAX_INDEX + 1 )
	{
	    /* Too many */
	    return ERROR_ENV_VAR_OVERFLOW;
        }
	else
	{
	    /* Allocate variable */
	    index   = env_var_count;
	    env_var = &env_vars[index];
	    info    = &env_var_info[index];

	    strcpy( info->data, name );

	    set_env_var( env_var, info );

	    if( *value )
	        strcpy( env_var->val, *value );
	    else
	        *env_var->val = '\0';
	    
	    info->index	        = INDEX_NONE;
	    info->default_value	= default_value;
	    info->decode	= decode;
	    info->attr		= attr;

	    /* Write to flash */
	    rc = env_to_flash( index, &index_flash );

	    if( rc != OK )
	    {
	        /* Could not write */
  	        return rc;
	    }
	    else if( index_flash != INDEX_REARRANGED )
	    {
	        /* Write OK, store index */
		env_var_info[index].index = index_flash;

		env_var_count++;	
                
		/* New NULL var */
		env_vars[env_var_count].name = NULL;
    		env_vars[env_var_count].val  = NULL;
	    }
	}
    }

    return OK;
}


/************************************************************************
 *                          set_env_var
 ************************************************************************/
static void
set_env_var(
    t_env_var      *env_var,
    t_env_var_info *info )
{
    env_var->name = info->data;
    env_var->val  = &info->data[strlen(env_var->name) + 1];
}


/************************************************************************
 *                          lookup_env
 ************************************************************************/
static bool
lookup_env(
    char   *name,
    UINT32 *index )
{
    /* Find variable */
    for( *index = 0; 
           (*index < env_var_count) &&
	   (strcmp( env_vars[*index].name, name ) != 0); 
	 (*index)++ );
  
    return ( *index == env_var_count ) ? FALSE : TRUE;
}


/************************************************************************
 *                          compare
 ************************************************************************/
static int
compare(
    UINT32 *x,
    UINT32 *y )
{
    return strcmp( env_vars[*x].name, env_vars[*y].name );
}


/************************************************************************
 *                          remove_var
 ************************************************************************/
static UINT32
remove_var(
    UINT32 index,
    bool   user,
    bool   system,
    bool   error_on_ro )
{
    t_env_var_info *info;
    UINT32	   rc;

    info = &env_var_info[index];

    switch( env_var_info[index].attr )
    {
      case ENV_ATTR_RW :
        if( system )
	{
	    /* Set to default value    */
	    rc = access_var( env_vars[index].name, 
			     &info->default_value, ENV_ATTR_RW,
			     info->default_value, NULL );

            if( rc != OK )\
	        return rc;
        }
        break;
      case ENV_ATTR_USER :
        if( user )
	{
	    /* Remove from flash */
	    env_remove_from_flash( env_var_info[index].index );

	    /* Move last record to the empty position */
	    env_var_info[index]  = env_var_info[env_var_count - 1];
	    set_env_var( &env_vars[index], &env_var_info[index] );

	    /* New NULL */
	    env_var_count--;
	    env_vars[env_var_count].name = NULL;
    	    env_vars[env_var_count].val  = NULL;
        }
	break;
      case ENV_ATTR_RO :
      default :
	/* Ignore request */
	if( error_on_ro )
	{
	    return ERROR_ENV_VAR_RO;
	}
	break;
    }

    return OK;
}


/************************************************************************
 *                          env_to_flash
 ************************************************************************/
static UINT32 
env_to_flash(
    UINT32    index_local,
    UINT32    *index_flash )
{
    t_user_environment_var disk_env_var;
    char		   s[SYS_USER_ENVIRONMENT_DATA_SIZE];
    UINT32		   len_name;
    UINT32		   i;
    INT32		   rc;
    char		   *val;

    t_env_var      *env_var = &env_vars[index_local]; 
    t_env_var_info *info    = &env_var_info[index_local];

    *index_flash = info->index;
    len_name     = strlen(env_var->name);
    val          = env_var->val ? env_var->val : "\0";

    disk_env_var.size = len_name + strlen(val) + 2;

    if( disk_env_var.size > SYS_USER_ENVIRONMENT_DATA_SIZE )
    {
        *index_flash = INDEX_NONE;
        return ERROR_ENV_VAR_VAL_LEN;
    }

    strcpy( s, env_var->name );
    strcpy( &s[len_name + 1], val );

#ifdef EB
    convert_endianness( s );
    
    /* Round size up to a whole multiplum of words */
    i = (sizeof(UINT32) - (disk_env_var.size % sizeof(UINT32))) %
            sizeof(UINT32);
    
    disk_env_var.size += i;
#endif

    disk_env_var.data_inout = (void *)s;

    if( *index_flash == INDEX_NONE )
    {
        /* Find free index */
	*index_flash = 0;

	do
	{
	    for( i=0; 
	         (i<env_var_count) && (env_var_info[i].index != *index_flash);
	         i++ );
		 
            if( i == env_var_count )	/* Found free index */
	        break;
	    else			/* Not free	    */
	        (*index_flash)++;
	
        }
	while( *index_flash <= SYS_USER_ENVIRONMENT_MAX_INDEX );

	if( *index_flash > SYS_USER_ENVIRONMENT_MAX_INDEX )
	{
	    /* Should not happen */
	    *index_flash = INDEX_NONE;
            return ERROR_ENV_VAR_OVERFLOW;
        }
    }

    disk_env_var.index = *index_flash;
    
    rc = SYSCON_write(
	         SYSCON_DISK_ENVIRONMENT_ID,
		 (void *)&disk_env_var,
		 sizeof(t_user_environment_var) );

    switch( rc )
    {
      case 0 :
	return OK;
      case ERROR_SYSENV_UPDATE_READ_REFS :
        /* Flash was rearranged, so re-read */
	re_init();
	*index_flash = INDEX_REARRANGED;
	return OK;
      default :
        *index_flash = INDEX_NONE;
	return rc;
    }
}


/************************************************************************
 *                          env_remove_from_flash
 ************************************************************************/
static void 
env_remove_from_flash( 
    UINT32 index )
{
    t_user_environment_var env_var;

    env_var.index      = index;
    env_var.size       = 0;
    env_var.data_inout = NULL;
    
    SYSCON_write(
	   SYSCON_DISK_ENVIRONMENT_ID,
	   (void *)&env_var,
	   sizeof(t_user_environment_var) );
} 


#ifdef EB
/************************************************************************
 *                          convert_endianness
 ************************************************************************/
static void
convert_endianness( 
    char *s )
{ 
    UINT32 i;
    UINT32 *data = (UINT32 *)s;

    for(i=0; i<SYS_USER_ENVIRONMENT_DATA_SIZE / 4; i++ )
    {
        data[i] = SWAPEND32( data[i] );
    }
}
#endif


/************************************************************************
 *                          re_init
 ************************************************************************/
static void 
re_init( void )
{
    /* Flash was rearranged, so indices must be updated */

    t_user_environment_var env_var;
    UINT32		   index;
    UINT32		   i;
 
    for( i=0; i<= SYS_USER_ENVIRONMENT_MAX_INDEX; i++ )
    {
        env_var.index = i;

        if( SYSCON_read(
	         SYSCON_DISK_ENVIRONMENT_ID,
		 (void *)&env_var,
		 sizeof(t_user_environment_var) ) == OK )
	{
	    /* Found variable */
	    if( lookup_env(
	           (char *)env_var.data_inout,
		   &index) )
	    {
	        /* lookup_env should never fail in this case */
	        env_var_info[index].index = env_var.index;
	    }
        }	 
    }
}



/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          env_init
 *  Description :
 *  -------------
 *
 *  Init ENV module (create System environment variables)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
env_init(void)
{
    t_user_environment_var	     env_var;
    UINT32			     i, t, count;
    char			     *s;
    bool			     default_switch;
    char			     *raw;
    char			     msg[40];
    t_sys_error_lookup_registration  registration;

    /* Register error handling function */
    registration.prefix = SYSERROR_DOMAIN( ERROR_ENV );
    registration.lookup = error_lookup;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    env_var_count = 0;
    env_corrupted = FALSE;

    /* Read environment variables */
    for( i=0; i<= SYS_USER_ENVIRONMENT_MAX_INDEX; i++ )
    {
        env_var.index = i;

        if( SYSCON_read(
	         SYSCON_DISK_ENVIRONMENT_ID,
		 (void *)&env_var,
		 sizeof(t_user_environment_var) ) == OK )
	{
	    env_var_info[env_var_count].index  = env_var.index;
	    env_var_info[env_var_count].attr   = ENV_ATTR_USER;
	    env_var_info[env_var_count].decode = NULL;

	    /* Make local copy of data */
	    s = env_var_info[env_var_count].data;

	    memcpy( s,
		    (char *)env_var.data_inout,
		    SYS_USER_ENVIRONMENT_DATA_SIZE );
#ifdef EB
            convert_endianness( s );
#endif

	    /* Check that data contains two \0 terminations */
	    for( t=0, count = 0; t< SYS_USER_ENVIRONMENT_DATA_SIZE; t++ )
	    {
	        if( s[t] == '\0' )
		    count++;
	    }

	    if( count >= 2 )
	    {
	        /* Setup name and value */		
	        env_vars[env_var_count].name = s;
	        env_vars[env_var_count].val  = &s[strlen(s) + 1];
	        env_var_count++;
            }
	    else
	        env_corrupted = TRUE;
	}
    }
    
    env_vars[env_var_count].name = NULL;
    env_vars[env_var_count].val  = NULL;

    /* Setup system environment variables if this was not done
     * at a previous run.
     */

    SYSCON_read( SYSCON_BOARD_USE_DEFAULT_ID,
		 (void *)&default_switch,
		 sizeof(UINT32) );

    SYSCON_read( SYSCON_CPU_CACHE_CONFIGURABLE_ID,
		 (void *)&cache_configurable,
		 sizeof(UINT32) );

    SYSCON_read( SYSCON_CPU_MMU_CONFIGURABLE_ID,
		 (void *)&mmu_configurable,
		 sizeof(UINT32) );

    SYSCON_read( SYSCON_CPU_CP0_CONFIG_RESET_ID,
		 (void *)&config_init,
		 sizeof(UINT32) );
    
    ext_priviliges = TRUE;

    /* YAMON revision */
    if( env_set( "yamonrev", YAMON_REV_STRING, ENV_ATTR_RO, NULL, NULL ) != OK )
        env_corrupted = TRUE;

    /* Size of RAM */
    sprintf( msg, "0x%08x", sys_ramsize );
    if( env_set( "memsize", msg, ENV_ATTR_RO, NULL, hex_s2num ) != OK )
        env_corrupted = TRUE;

    /* Prompt */
    if( default_switch || !env_get( "prompt", &raw, NULL, 0 ) )
        raw = sys_default_prompt;	  /* Create new */
    if( env_set( "prompt", raw, ENV_ATTR_RW, sys_default_prompt, NULL ) != OK )
        env_corrupted = TRUE;

    /* Start command */
    if( default_switch || !env_get( "start", &raw, NULL, 0 ) )
        raw = "";		  /* Create new */
    if( env_set( "start", raw, ENV_ATTR_RW, "", NULL ) != OK )
        env_corrupted = TRUE;

    /* Startdelay */
    if( default_switch || !env_get( "startdelay", &raw, NULL, 0 ) )
        raw = "";		  /* Create new */
    if( env_set( "startdelay", raw, ENV_ATTR_RW, "", NULL ) != OK )
        env_corrupted = TRUE;

    /**** Cache configuration ****/

    if( cache_configurable || mmu_configurable )
    {
        if( default_switch || !env_get( "cpuconfig", &raw, NULL, 0 ) || 
	    !validate_cpuconfig( raw, NULL, 0) )
        {
            raw = default_cpuconfig;	/* Create new */
        }

        if( env_set( "cpuconfig", raw, ENV_ATTR_RW, 
		     default_cpuconfig, validate_cpuconfig ) != OK )
	{
	    env_corrupted = TRUE;
	}
    }
    else
    {
        if( env_unset( "cpuconfig" ) != OK )
	    env_corrupted = TRUE;
    }

    /* TTY variables */
    if( !env_setup_env_tty( default_switch ) )
        env_corrupted = TRUE;

    /* Board settings */
    if( !env_setup_env_board( default_switch ) )
        env_corrupted = TRUE;

    ext_priviliges = FALSE;

    /* FPU settings */
    shell_fpuemul_init();
}


/************************************************************************
 *
 *                          env_get
 *  Description :
 *  -------------
 *
 *  Lookup environment variable based on name.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> found, FALSE -> not found
 *
 ************************************************************************/
bool
env_get(
    char   *name,		/* Name of environment variable		*/
    char   **raw,		/* Raw text string			*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    UINT32 index;
    bool   rc = TRUE;

    if( !lookup_env( name, &index ) )
    {
        rc = FALSE;
    }
    else
    {
        /* Found */
        if( raw )
	    *raw = env_vars[index].val;

	if( env_var_info[index].decode )
	{
	    /*  decoded parameter may be NULL, in which case this is
	     *  simply a validation of the data.
	     */
            rc = env_var_info[index].decode( env_vars[index].val, decoded, size );
        }
	else
        {
	    /*  No decode function, so reply TRUE unless a decoded
	     *  parameter is given, which we can't fill out.
	     */
	    rc = decoded ? FALSE : TRUE;
        } 
    }

    return rc;
}


/************************************************************************
 *
 *                          env_set
 *  Description :
 *  -------------
 *
 *  Set (and possibly create) environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
env_set(
    char         *name,
    char	 *value,
    UINT8	 attr,
    char         *default_value,
    t_env_decode decode )
{
    /* check validity of data if applicable */
    if (decode && !decode(value, NULL, 0 ) )
    {
	return ERROR_ENV_VAR_VALUE;
    }
    return access_var( name, &value, attr, default_value, decode );
}


/************************************************************************
 *
 *                          env_unset
 *  Description :
 *  -------------
 *
 *  Delete environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
env_unset(
    char   *name )	/* Name of variable */
{
    UINT32 index;

    return 
        lookup_env( name, &index ) ?
            /* Found     */
	    remove_var( index, TRUE, TRUE, TRUE ) :
	    /* Not found */
	    OK;
}


/************************************************************************
 *
 *                          env_print_all
 *  Description :
 *  -------------
 *
 *  Display all env. variables
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
env_print_all( void )
{
    t_env_var	   *env_var;
    t_env_var_info *info;
    UINT32	   index;
    char	   *s;
    UINT32	   sort[SYS_USER_ENVIRONMENT_MAX_INDEX + 1];
    UINT32	   i;
    UINT32	   maxlen = 0;

    /* Sort and print variables */

    for( i = 0; i < env_var_count; i++ )
        sort[i] = i;	    

    qsort(
        (char *)sort,
	(int)env_var_count,
	(int)sizeof(UINT32),
	compare );

    /* Print variables */

    if( env_var_count != 0 )
    {
        /* Find longest name */
        for( i=0; i < env_var_count; i++ )
        {
            env_var = &env_vars[sort[i]];
	    maxlen  = MAX( maxlen, strlen( env_var->name ) );
        }

	/* Print variables */
        for( i=0; i < env_var_count; i++ )
        {
            index = sort[i];

            env_var = &env_vars[index];
	    info    = &env_var_info[index];
		
            if(SHELL_PUTC( '\n' )) return;
            if(SHELL_PUTS( env_var->name )) return;
		
            switch( info->attr )
	    {
	      case ENV_ATTR_RW :
	        s = "(R/W)"; break;
	      case ENV_ATTR_RO :
	        s = "(RO)"; break;
	      case ENV_ATTR_USER :
	        s = "(USER)"; break;
	      default :
	        s = NULL; break;
	    }		      				    

            if( s )
	    {
	        if(SHELL_PUTS_INDENT( s, maxlen + 1 )) return;
	    }
	        
	    if(SHELL_PUTS_INDENT( env_var->val, maxlen + 8 )) 
	        return;
        }

        if(SHELL_PUTS( "\n\n" )) return;
    }
}


/************************************************************************
 *
 *                          env_remove
 *  Description :
 *  -------------
 *
 *  Remove all user and or system variables
 *
 *  Return values :
 *  ---------------
 *
 *  Error code (OK = No error)
 *
 ************************************************************************/
UINT32
env_remove(
    bool user,		/* TRUE -> remove user variables		*/
    bool system )	/* TRUE -> remove system variables		*/
{
    UINT32   index;
    UINT32   old_count;
    UINT32   rc = OK;
    
    index = 0;

    while( (index < env_var_count) && (rc == OK) )
    {
        old_count = env_var_count;

        rc = remove_var( index, user, system, FALSE );

        /*  If env_var_count was decreased, the
         *  variable was deleted and the last variable
	 *  was moved to it's place in the array
	 */
        if( old_count == env_var_count )
        {
            /* Variable was not deleted */
            index++;
        }
    }

    return rc;
}


/************************************************************************
 *
 *                          env_ip_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format x.y.z.w to 4 bytes
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_ip_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    char   *endp;
    UINT32 number;
    UINT32 i;
    UINT32 value = 0;

    /* Check that we have the string */
    if( !raw )
        return FALSE;

    /* Check size of variable for decoded data */
    if( decoded && (size != sizeof(UINT32)) )
        return FALSE;
    
    for(i=0; i<sizeof(UINT32); i++)
    {
        value <<= 8;

        errno = 0;

        number = strtoul( raw, &endp, 10 );

	if( errno || (number > 255) || (endp == raw) )
	    return FALSE;
	   
	value |= number;   

	if( i != (sizeof(UINT32) - 1) )
	{
	    /* Not last number */
	    if( *endp != '.' )
	        return FALSE;
	    else
	        raw = endp + 1;	/* Skip dot */
	}
    }	

    if( decoded )
    {
        /* Always deliver data in network byte order (big endian) */
#ifdef EL
        *(UINT32 *)decoded = SWAPEND32(value);
#else
        *(UINT32 *)decoded = value;
#endif
    }

    return ( *endp == '\0' ) ? TRUE : FALSE;    
}


/************************************************************************
 *
 *                          env_setup_cpuconfig
 *  Description :
 *  -------------
 *
 *  Setup string for "cpuconfig" environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
env_setup_cpuconfig(
    char		*s,		
    t_sys_cpu_decoded   *decoded )
{
    *s = '\0';

    if( cache_configurable )
    {    
        sprintf( s, "%d,%d,%d,%d",    
		        decoded->i_bpw, decoded->i_assoc,
			decoded->d_bpw, decoded->d_assoc );  
	
	if( mmu_configurable )
	    strcat( s, "," );
    }

    if( mmu_configurable )
        strcat( s, decoded->mmu_tlb ? "tlb" : "fixed" );
}


/************************************************************************
 *
 *                          env_get_all
 *  Description :
 *  -------------
 *
 *  Get a pointer to the array of environment variables
 *
 *  Return values :
 *  ---------------
 *
 *  Pointer to array of env. variables
 *
 ************************************************************************/
t_env_var
*env_get_all( void )
{
    return env_vars;
}
/************************************************************************
 *
 *                          env_mac_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format xx.xx.xx.xx.xx.xx to 6 bytes
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_mac_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    t_mac_addr *mac_addr;
    char       *endp;
    UINT8      number;
    UINT32     i;
  
    if( !raw || (strlen(raw) != 5+6*2) )
        return FALSE;

    if( decoded && (size != sizeof(t_mac_addr)) )
        return FALSE;

    mac_addr = (t_mac_addr *)decoded;
    errno    = 0;
    
    for( i=0; i<sizeof(t_mac_addr); i++)
    {
        if( *raw == '\0' )
	    return FALSE;

	number = strtoul( raw, &endp, 16 );

        if( errno )
	    return FALSE;
	if( endp == raw )
	    return FALSE;
	
        raw = &endp[1];

	if( mac_addr )
	    (*mac_addr)[i] = number;
    }    

    return (*endp == '\0') ? TRUE : FALSE;
}


/************************************************************************
 *
 *                          env_check
 *  Description :
 *  -------------
 *
 *  Determine whether env. variables have been corrupted (due to flash
 *  corruption). If this is the case, print warning.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> corrupted, FALSE -> not corrupted (normal state)
 *
 ************************************************************************/
bool
env_check(void)
{
    if( env_corrupted )
    {
        printf("\nWARNING: Environment variables not valid!" 
	       "\nHINT   : Perform \"erase -e\"" );
	return TRUE;
    }
    else
        return FALSE;
}

