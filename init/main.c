
/************************************************************************
 *
 *  main.c
 *
 *  First C-function
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

#include <sysdefs.h>
#include <string.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <initmodules_api.h>
#include <env_api.h>
#include <shell_api.h>
#include <loader_api.h>
#include <init.h>
#include <product.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/*  Macro for conversion between time interval in ns and number of
 *  SysAD bus cycles.
 *  Round result up.
 */
#define NS2COUNT_ROUND_UP(ns,count)		   \
    count =  (sys_busfreq_hz + 999999) / 1000000;  \
    count *= ns;				   \
    count =  (count + 999) / 1000;

/************************************************************************
 *  Public variables (some not used here)
 ************************************************************************/

/* Platform attributes */
UINT32 sys_processor;
UINT32 sys_platform;
UINT32 sys_corecard;
UINT32 sys_manid;
UINT32 sys_manpd;
UINT32 sys_nb_base;
UINT32 sys_ramsize;
UINT32 sys_freemem;

/* CPU attributes */
bool   sys_64bit;
bool   sys_mips32_64;
bool   sys_mips16e;
bool   sys_smallpage_support;
bool   sys_ejtag;
bool   sys_mt;
bool   sys_dsp;
bool   sys_eicmode;
UINT8  sys_arch_rev;
UINT32 sys_fpu;

/* CPU support for hw cache coherency */
bool   sys_cpu_cache_coherency;

/* Level 2 cache configuration */
bool   sys_l2cache;
bool   sys_l2cache_enabled;
UINT32 sys_l2cache_lines;
UINT32 sys_l2cache_linesize;
UINT32 sys_l2cache_assoc;

/* Level 1 cache configuration */
UINT32 sys_icache_linesize;
UINT32 sys_icache_lines;
UINT32 sys_icache_assoc;
UINT32 sys_dcache_linesize;
UINT32 sys_dcache_lines;
UINT32 sys_dcache_assoc;

char   *sys_default_prompt = DEFAULT_PROMPT;
char   sys_default_display[] = DEFAULT_PROMPT;

/************************************************************************
 *  Static variables
 ************************************************************************/
static char sys_revision[] = _REVMAJ_ _REVMIN_;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
core_optimize( void );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          c_entry
 *  Description :
 *  -------------
 *
 *  First C-function
 *
 *  Return values :
 *  ---------------
 *
 *  Never returns
 *
 ************************************************************************/
int
c_entry(void)
{
    t_sys_cpu_decoded decoded;
    bool	      init_from_env;
    int               len;

    /* Determine CPU type */
    sys_cpu_type();

    /* If FPU is available, enable it */
    if( sys_fpu )
        sys_fpu_enable();

    /* If sys_default_prompt must be modified at run time, do it here */

    /* Set up default display
     *  example YAM203b2,  YAMO203b,  YAMON203,  YAMO203x
     *          beta       candidate  release    post release
     */
    len = sizeof(sys_revision) - 1;
    if (sys_revision[0] == '0')
    {
	/* remove leading '0' character */
	memcpy(&sys_revision[0], &sys_revision[1], len--);
    }
    if (len <= 5)
    {
	/* "203", "203d", "203dx" */
        memcpy(&sys_default_display[8-len], sys_revision, len);
    }
    else {
	/* "204beta02", "204beta02xx" */
	memcpy(&sys_default_display[3], sys_revision, 4);
        for (len = 4; sys_revision[len+1]; len++)
	{
	    if (sys_revision[len] >= '1' && sys_revision[len] <= '9')
		break;
	}
        sys_default_display[7] = sys_revision[len];
    }

    /*  Early (before module initialisation)
     *  platform specific initialisation 
     */
    arch_platform_init( TRUE );
      
    /* Initialise modules */
    initmodules();
  
    /* Optimise system controller settings */
    DISP_STR( "OPTIMIZE" );
    core_optimize();

    /*  Late (after module initialisation)
     *  platform specific initialisation 
     */
    arch_platform_init( FALSE );

    /* Configure cpu according to environment variable "cpuconfig" */

    /*  This feature is present specifically to support configuration
     *  testing of the core in a lead vehicle, and is not supported
     *  in any other environment.  Attempting to use this feature
     *  outside of the scope of a lead vehicle is a violation of the
     *  MIPS Architecture, and may cause unpredictable operation of
     *  the processor.
     */

    SYSCON_read( SYSCON_BOARD_INIT_BASED_ON_ENV_ID,
		 (void *)&init_from_env, sizeof(bool) );

    if( init_from_env &&
        env_get( "cpuconfig", NULL, &decoded, sizeof(t_sys_cpu_decoded) ) )
    {
        DISP_STR( "CPU_U" );
	sys_cpu_config( TRUE, TRUE, TRUE, &decoded );
    }

    DISP_STR("INITDONE");

    /* Initialise loader (TBD : load could be handled as any other module) */
    loader_init();

    /* Enable interrupts */
    sys_enable_int();

    /* Done with initialisation. Now jump to shell (never returns) */
    shell_setup();

    return ERROR_STRUCTURE;	/* Should never happen */
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          core_optimize
 *  Description :
 *  -------------
 *
 *  Re-configure system controller based on detected bus frequency.
 *
 *  Read various SDRAM requirements in ns, convert to number 
 *  of cycles at the SysAD bus frequency and configure using 
 *  this number.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
static void
core_optimize()
{
    UINT32 ns, count;

    /* count = bus frequency (in MHz) * rate (in us) */

    /**** Refresh rate ****/

    /* Read rate */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_SPD_ID,
		    &ns, sizeof(UINT32)) == OK)
    {
        /* Config rate */
        SYSCON_write( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_CFG_ID,
		      &ns, sizeof(UINT32) );
    }

    /**** SRAS precharge time ****/

    /* Read rate */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_NS_SPD_ID,
		    &ns, sizeof(UINT32)) == OK)
    {
        NS2COUNT_ROUND_UP(ns,count);

	/* Write cycles */
        SYSCON_write( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_CYCLES_CFG_ID,
		      &count, sizeof(UINT32) );
    }

    /**** SRAS to SCAS delay ****/

    /* Read rate */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_NS_SPD_ID,
		    &ns, sizeof(UINT32)) == OK)
    {
        NS2COUNT_ROUND_UP(ns,count);

	/* Write cycles */
        SYSCON_write( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_CYCLES_CFG_ID,
		      &count, sizeof(UINT32) );
    }
}

