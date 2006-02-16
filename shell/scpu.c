
/************************************************************************
 *
 *  scpu.c
 *
 *  Monitor command for configuring cpu (cache, MMU etc.)
 *
 *  This feature is present specifically to support configuration
 *  testing of the core in a lead vehicle, and is not supported
 *  in any other environment.  Attempting to use this feature
 *  outside of the scope of a lead vehicle is a violation of the
 *  MIPS Architecture, and may cause unpredictable operation of
 *  the processor.
 *
 *  Configurable cache only :
 *
 *      scpu ( [-i|-d]+ [-a|-u|(-r|-p)+]    ) |
 *           ( (-i|-d) <bpw> [<assoc>] [-p] ) |
 *           ( (-i|-d) <assoc> [<bpw>] [-p] )
 *
 *  Configurable MMU only :
 *
 *      scpu [-a|-u|(-r|-p)+] | ( (tlb|fixed) [-p] )
 *
 *  Configurable cache and MMU
 *
 *      scpu ( [-i|-d|-m]+ [-a|-u|(-r|-p)+] ) |
 *           ( (-i|-d) <bpw> [<assoc>] [-p] ) |
 *           ( (-i|-d) <assoc> [<bpw>] [-p] ) |
 *           ( (tlb|fixed) [-p]		    )
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
#include <syscon_api.h>
#include <env_api.h>
#include <sys_api.h>
#include <shell_api.h>
#include <mips.h>
#include <shell.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    bool   icache;	/* Select I-cache		     */
    bool   dcache;	/* Select D-cache		     */
    bool   mmu;		/* Select MMU			     */
    bool   available;   /* List available settings	     */
    bool   update;	/* Update from environment variables */
    bool   reset;	/* Set to hardware reset values      */
    bool   permanent;	/* Commit to flash		     */
    UINT32 bpw;		/* Bytes per way		     */
    UINT32 assoc;	/* Associativity (number of ways)    */
    UINT32 mmu_type;	/* MMU type			     */
#define MMU_TLB	   0
#define MMU_FIXED  1
}
t_parms;


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/


static t_cmd_option options[] =
{
  { "a", "Display available configurations." },
  { "u", "Configure based on environment variable." },
  { "r", "Reset configuration to hardware reset value." },
  { "p", "Commit configuration to environment variable." },

  /* Space for cache/mmu options */
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL }
};
#define OPTION_COUNT_BASIC	(sizeof(options)/sizeof(t_cmd_option) - 3)

static t_cmd_option options_enhanced[] =
{
#define OPTION_ICACHE   0
  { "i", "Display/configure instruction-cache settings." },
#define OPTION_DCACHE   1
  { "d", "Display/configure data-cache settings." },
#define OPTION_MMU	2
  { "m", "Display/configure MMU type." }
};

static bool cache_configurable, mmu_configurable;

static UINT32 *i_bpw_array,   i_bpw_count;
static UINT32 *d_bpw_array,   d_bpw_count;
static UINT32 *i_assoc_array, i_assoc_count;
static UINT32 *d_assoc_array, d_assoc_count;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

static UINT32
get_options(
    UINT32    argc,
    char      **argv,
    t_parms   *parms );

static void
disp_available( 
    bool icache,
    bool dcache,
    bool mmu
);

static void
disp_current( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting );

static UINT32
store_current( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting );

static bool
legal_bpw(
    bool   icache, 
    UINT32 bpw );

static bool
legal_assoc( 
    bool   icache,
    UINT32 bpw );


/************************************************************************
 *                          scpu
 ************************************************************************/
static MON_FUNC(scpu)
{
    t_parms            parms;
    UINT32	       rc;
    t_sys_array	       sys_array;
    UINT32	       config, config1;
    t_sys_cpu_decoded  decoded;
    t_sys_cpu_decoded  setting;

    /* Determine available settings */

    SYSCON_read( SYSCON_CPU_ICACHE_AVAIL_BPW_ID,
		 &sys_array,
		 sizeof(t_sys_array) );
    i_bpw_count = sys_array.count;
    i_bpw_array = sys_array.array;

    SYSCON_read( SYSCON_CPU_DCACHE_AVAIL_BPW_ID,
		 &sys_array,
		 sizeof(t_sys_array) );
    d_bpw_count = sys_array.count;
    d_bpw_array = sys_array.array;

    SYSCON_read( SYSCON_CPU_ICACHE_AVAIL_ASSOC_ID,
		 &sys_array,
		 sizeof(t_sys_array) );
    i_assoc_count = sys_array.count;
    i_assoc_array = sys_array.array;

    SYSCON_read( SYSCON_CPU_DCACHE_AVAIL_ASSOC_ID,
		 &sys_array,
		 sizeof(t_sys_array) );
    d_assoc_count = sys_array.count;
    d_assoc_array = sys_array.array;

    /* Get options */

    rc = get_options( argc, argv, &parms );

    if( rc != OK )
        return rc;
    else
    {
	SYSCON_read( parms.reset ?
		         SYSCON_CPU_CP0_CONFIG1_RESET_ID :
			 SYSCON_CPU_CP0_CONFIG1_ID,
		     &config1,
		     sizeof(UINT32) );

	SYSCON_read( parms.reset ?
		         SYSCON_CPU_CP0_CONFIG_RESET_ID :
		         SYSCON_CPU_CP0_CONFIG_ID,
		     &config,
		     sizeof(UINT32) );

	setting.i_bpw   = CACHE_CALC_BPW(((config1 & M_Config1IL) >> S_Config1IL),
					 ((config1 & M_Config1IS) >> S_Config1IS));

	setting.i_assoc = CACHE_CALC_ASSOC((config1 & M_Config1IA) >> S_Config1IA);

	setting.d_bpw   = CACHE_CALC_BPW(((config1 & M_Config1DL) >> S_Config1DL),
					 ((config1 & M_Config1DS) >> S_Config1DS));

	setting.d_assoc = CACHE_CALC_ASSOC((config1 & M_Config1DA) >> S_Config1DA);

        setting.mmu_tlb = ((config >> 8) & 1) ? FALSE : TRUE;


	/**** Actions ****/

	if( parms.available )
	{
	    disp_available( parms.icache, parms.dcache, parms.mmu );
	    return OK;
        }

	if( !parms.update && !parms.reset && !parms.permanent &&
	    (parms.bpw == -1) && (parms.assoc == -1)          &&
	    (parms.mmu_type == -1) )
        {
	    /* Request for display of current settings */
	    disp_current( parms.icache,
			  parms.dcache,
			  parms.mmu,
			  &setting );

	    return OK;
	}      

	if( parms.permanent &&
	    !parms.reset &&
	    (parms.bpw == -1) && (parms.assoc == -1) &&
	    (parms.mmu_type == -1) )
	{
	    /*  "scpu [-i|-d|-m] -p" command.
	     *  Store current configuration (i, d or both) in
	     *  environment variable(s).
	     */
	    return store_current( parms.icache,
				  parms.dcache,
				  parms.mmu,
				  &setting );
	}

	if( !parms.reset )
	{
	    /**** Update or Set request ****/

	    if( parms.update )
	    {
	        if( !env_get( "cpuconfig", NULL, (void *)&decoded,
			      sizeof(t_sys_cpu_decoded) ) )
		{
		    return SHELL_ERROR_VAR_FLASH;
		}
	    }

	    /* Setup I-Cache */

	    if( parms.icache )
	    {
	        if( parms.update )
	        {
		    setting.i_bpw   = decoded.i_bpw;
		    setting.i_assoc = decoded.i_assoc;
	        }
		else
		{
	            if(parms.bpw != -1)   setting.i_bpw   = parms.bpw;
	            if(parms.assoc != -1) setting.i_assoc = parms.assoc;
		}
            }

	    /* Setup D-Cache */

	    if( parms.dcache )
	    {
	        if( parms.update )
	        {
		    setting.d_bpw   = decoded.d_bpw;
		    setting.d_assoc = decoded.d_assoc;
	        }
		else
		{
	            if(parms.bpw != -1)   setting.d_bpw   = parms.bpw;
	            if(parms.assoc != -1) setting.d_assoc = parms.assoc;
		}
            }

	    /* Setup TLB */

	    if( parms.mmu )
	    {
	        if( parms.update )
	        {
		    setting.mmu_tlb = decoded.mmu_tlb;
	        }
		else
		{
	            setting.mmu_tlb = (parms.mmu_type == MMU_TLB) ?
				          TRUE : FALSE;
		}
	    }
        }

	/* Configure */
	sys_cpu_config( parms.icache,
			parms.dcache,
			parms.mmu,
			&setting );

	if( parms.permanent )
	{
	    return store_current( parms.icache,
			          parms.dcache,
				  parms.mmu,
				  &setting );
	}

	return OK;
    }
}


static UINT32
get_options(
    UINT32    argc,
    char      **argv,
    t_parms   *parms )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    bool	   ok    = TRUE;
    UINT32	   error = SHELL_ERROR_SYNTAX;
    UINT32	   count = 0;
    UINT32	   integer1, integer2;

    /* Setup default */
    parms->available  = FALSE;
    parms->update     = FALSE;
    parms->reset      = FALSE;
    parms->permanent  = FALSE;
    parms->icache     = FALSE;
    parms->dcache     = FALSE;
    parms->mmu	      = FALSE;
    parms->bpw	      = -1;
    parms->assoc      = -1;
    parms->mmu_type   = -1;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    
	    if(      strcmp( decode.option, "a" ) == 0 )
	    {
		parms->available = TRUE;
	    }
	    else if( strcmp( decode.option, "u" ) == 0 )
	    {
		parms->update    = TRUE;
	    }
	    else if( strcmp( decode.option, "r" ) == 0 )
	    {
		parms->reset     = TRUE;
	    }
	    else if( strcmp( decode.option, "p" ) == 0 )
	    {
		parms->permanent = TRUE;
	    }
	    else if( cache_configurable && 
		     (strcmp( decode.option, "i" ) == 0) )
	    {
		parms->icache    = TRUE;
	    }
	    else if( cache_configurable && 
		     (strcmp( decode.option, "d" ) == 0) )
	    {
		parms->dcache    = TRUE;
	    }
	    else if( mmu_configurable && cache_configurable &&
		     (strcmp( decode.option, "m" ) == 0) )
	    {
	        parms->mmu       = TRUE;
	    }
	    else
	    {
		error	         = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }

	    break;

	  case SHELL_TOKEN_NUMBER :

	    if( !cache_configurable )
	        ok = FALSE;
	    else if( count == 0 )
	        integer1 = decode.number;
	    else if( count == 1 )
	        integer2 = decode.number;
	    else
	        ok = FALSE;

	    count++;
	    
	    break;

	  case SHELL_TOKEN_STRING :

	    if( !mmu_configurable || (parms->mmu_type != -1) )
	        ok = FALSE;
	    else
	    {
	        if( strcmp( decode.string, "tlb" ) == 0 )
		    parms->mmu_type = MMU_TLB;
	        else if( strcmp( decode.string, "fixed" ) == 0 )
		    parms->mmu_type = MMU_FIXED;
		 else
		    ok = FALSE;
	    }

	    break;

	  default : /* Should not happen */

	    ok = FALSE;
	    break;
        }
    }

    if( !ok )
        return error;

    if( !parms->icache && !parms->dcache && !parms->mmu && 
        ( parms->mmu_type == -1 ) )
    {
        parms->icache = cache_configurable;
        parms->dcache = cache_configurable;
        parms->mmu    = mmu_configurable;
    }

    /* Verify */
    if( parms->available )
    {
        if( parms->update		||
	    parms->reset		||
	    parms->permanent		||
	    (count != 0)		||
	    (parms->mmu_type != -1) )
	{
	    return SHELL_ERROR_SYNTAX;
        }
	else
	    return OK;
    }

    if( parms->update )
    {
        if( parms->reset		||
	    parms->permanent		||
	    (count != 0)		||
	    (parms->mmu_type != -1) )
	{
	    return SHELL_ERROR_SYNTAX;
        }
	else
	    return OK;
    }

    if( parms->reset )
    {
        if( (count != 0)		||
	    (parms->mmu_type != -1) )
	{
	    return SHELL_ERROR_SYNTAX;
	}
	else
	    return OK;
    }

    if( count != 0 )
    {
        if( parms->mmu || (parms->mmu_type != -1) )
	    return SHELL_ERROR_SYNTAX;
	
	/* Must select one and only one cache (i/d) */    
        if( parms->icache && parms->dcache )
	    return SHELL_ERROR_SYNTAX;
        if( !parms->icache && !parms->dcache )
	    return SHELL_ERROR_SYNTAX;

	if( legal_bpw( parms->icache, integer1 ) )
	    parms->bpw   = integer1;
	else if( legal_assoc( parms->icache, integer1 ) )
	    parms->assoc = integer1;
	else
	    return SHELL_ERROR_ILLEGAL_CACHE_CFG;

	if( count == 2 )
	{
	    if( parms->bpw == -1 )
	    {
	        if( legal_bpw( parms->icache, integer2 ) )
	            parms->bpw = integer2;
		else
		    return SHELL_ERROR_ILLEGAL_CACHE_CFG;
	    }
	    else
	    {
	        if( legal_assoc( parms->icache, integer2 ) )
	            parms->assoc = integer2;
		else
		    return SHELL_ERROR_ILLEGAL_CACHE_CFG;
	    }
	}
    }

    if( parms->mmu_type != -1 )
    {
        if( parms->mmu )
	    return SHELL_ERROR_SYNTAX;
	else
	{
	    parms->mmu = TRUE;
	    return OK;
        }
    }

    return OK;
}


static bool
legal_bpw(
    bool   icache, 
    UINT32 bpw )
{
    UINT32 count;
    UINT32 *array;
    UINT32 i;
    
    count = icache ? i_bpw_count : d_bpw_count;
    array = icache ? i_bpw_array : d_bpw_array;

    for( i=0; 
	     (i<count) &&
	     (bpw != array[i]);
	 i++ );

    return (i == count) ? FALSE : TRUE;
}


static bool
legal_assoc( 
    bool   icache,
    UINT32 assoc )
{
    UINT32 count;
    UINT32 *array;
    UINT32 i;
    
    count = icache ? i_assoc_count : d_assoc_count;
    array = icache ? i_assoc_array : d_assoc_array;

    for( i=0; 
	     (i<count) &&
	     (assoc != array[i]);
	 i++ );

    return (i == count) ? FALSE : TRUE;

}


static void
disp_available( 
    bool icache,
    bool dcache,
    bool mmu )
{
    UINT32	 i;
    char	 msg[200];

    SHELL_PUTS( "Available settings :\n" );

    /* I-cache */	   
    if( icache )
    {
        /* BPW */
        sprintf( msg, "  I-Cache bytes per way :" );

	for( i = 0; i<i_bpw_count; i++ )
	{
	    sprintf( &msg[strlen(msg)], 
		     (i == 0) ? 
		         " 0x%x"  : 
			 ", 0x%x" , 
		     i_bpw_array[i] );
	}
        sprintf( &msg[strlen(msg)], "\n" );

        SHELL_PUTS( msg );

	/* ASSOC */
        sprintf( msg, "  I-Cache associativity :" );

	for( i = 0; i<i_assoc_count; i++ )
	{
	    sprintf( &msg[strlen(msg)], 
		     (i == 0) ? 
		         " %d"  : 
			 ", %d" , 
		     i_assoc_array[i] );
	}
        sprintf( &msg[strlen(msg)], "\n" );

        SHELL_PUTS( msg );
    }

    /* D-cache */	   
    if( dcache )
    {
        /* BPW */
        sprintf( msg, "  D-Cache bytes per way :" );

	for( i = 0; i<d_bpw_count; i++ )
	{
	    sprintf( &msg[strlen(msg)],
		     (i == 0) ? 
		         " 0x%x"  : 
			 ", 0x%x" , 
		     d_bpw_array[i] );
	}
        sprintf( &msg[strlen(msg)], "\n" );

        SHELL_PUTS( msg );

	/* ASSOC */
        sprintf( msg, "  D-Cache associativity :" );

	for( i = 0; i<d_assoc_count; i++ )
	{
	    sprintf( &msg[strlen(msg)],
		     (i == 0) ? 
		         " %d"  : 
			 ", %d" , 
		     d_assoc_array[i] );
	}
        sprintf( &msg[strlen(msg)], "\n" );

        SHELL_PUTS( msg );
    }

    /* MMU */
    if( mmu )
    {
        SHELL_PUTS( "  MMU types             : tlb, fixed\n" );
    }
}


static UINT32
store_current( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting )
{
    char		s[80];
    t_sys_cpu_decoded   decoded;    

    if( !env_get( "cpuconfig", NULL, 
		  (void *)&decoded, sizeof(t_sys_cpu_decoded) ) )
    {
        return SHELL_ERROR_VAR_FLASH;
    }

    if( icache )
    {
        decoded.i_bpw	= setting->i_bpw;
	decoded.i_assoc = setting->i_assoc;
    }

    if( dcache )
    {
        decoded.d_bpw	= setting->d_bpw;
	decoded.d_assoc = setting->d_assoc;
    }

    if( mmu )
    {
        decoded.mmu_tlb = setting->mmu_tlb;
    }

    env_setup_cpuconfig( s, &decoded );

    return env_set( "cpuconfig", s, ENV_ATTR_RW, NULL, NULL );
}


static void
disp_current( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting )
{
    char msg[80];

    SHELL_PUTS( "Current settings :\n" );
	   
    if( icache )
    {
        sprintf( msg, "  I-Cache bytes per way = 0x%x\n", setting->i_bpw );
	SHELL_PUTS( msg );
        sprintf( msg, "  I-Cache associativity = %d\n", setting->i_assoc );
	SHELL_PUTS( msg );
    }

    if( dcache )
    {
        sprintf( msg, "  D-Cache bytes per way = 0x%x\n", setting->d_bpw );
	SHELL_PUTS( msg );
        sprintf( msg, "  D-Cache associativity = %d\n", setting->d_assoc );
	SHELL_PUTS( msg );
    }

    if( mmu )
    {
        SHELL_PUTS( "  MMU                   = " );
	SHELL_PUTS( setting->mmu_tlb ? "tlb\n" : "fixed\n" );
    }
}

static char synopsis_cache[] =
    "scpu ( [-i|-d]+ [-a|-u|(-r|-p)+]    ) |\n"
    "     ( (-i|-d) <bpw> [<assoc>] [-p] ) |\n"
    "     ( (-i|-d) <assoc> [<bpw>] [-p] )";

static char synopsis_mmu[] =
    "scpu [-a|-u|(-r|-p)+] | ( (tlb|fixed) [-p] )";

static char synopsis_cache_mmu[] =
    "scpu ( [-i|-d|-m]+ [-a|-u|(-r|-p)+] ) |\n"
    "     ( (-i|-d) <bpw> [<assoc>] [-p] ) |\n"
    "     ( (-i|-d) <assoc> [<bpw>] [-p] ) |\n"
    "     ( (tlb|fixed) [-p]             )";

static t_cmd cmd_def =
{
    "scpu",
    scpu,

    NULL,

    "Configure or view current cpu configuration.\n\n"

    "scpu does not by default modify the semi-permanent scpu\n"
    "setting recorded in the environment variable 'cpuconfig'.\n"
    "By default, cpuconfig is an empty string, implying processor\n"
    "specific hardware reset configuration.\n"
    "Use the '-p' option if you want to set the environment variable.\n"
    "Use 'unsetenv cpuconfig' if you wish to reset cpuconfig to an\n"
    "empty string.\n\n"

    "The following operations are available :\n\n"

    "Display available settings.\n"
    "Display current configuration.\n"
    "Edit configuration.\n"
    "Setup configuration based on environment variable.\n"
    "Reset configuration to hardware default.\n"
    "Store current configuration in environment variable.\n\n"

    "'scpu' without options or parameters displays the current\n"
    "configuration.",

    options,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_scpu_init
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
shell_scpu_init( 
    bool cache,
    bool mmu )
{
    cache_configurable = cache;
    mmu_configurable   = mmu;

    cmd_def.option_count = OPTION_COUNT_BASIC;

    if( cache )
    {
	memcpy( &options[ cmd_def.option_count++ ],
		&options_enhanced[ OPTION_ICACHE ],
		sizeof( t_cmd_option ) );

	memcpy( &options[ cmd_def.option_count++ ],
		&options_enhanced[ OPTION_DCACHE ],
		sizeof( t_cmd_option ) );

        cmd_def.syntax = synopsis_cache;
    }
    
    if( mmu )
    {
        if( cache )
	{
            memcpy( &options[ cmd_def.option_count++ ],
	            &options_enhanced[ OPTION_MMU ],
		    sizeof( t_cmd_option ) );
        }

        cmd_def.syntax = cache ?
			     synopsis_cache_mmu :
			     synopsis_mmu;
    }

    return &cmd_def;
}




