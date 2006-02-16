
/************************************************************************
 *
 *  cpu_c.c
 *
 *  Cache C-functions
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
#include <sys_api.h>
#include <syscon_api.h>
#include <mips.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <shell_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    char   *name;
    UINT8  width;
    UINT32 syscon_id;
    UINT32 gdb_offset;
    UINT32 rc;
    union
    {
        UINT32 value32;
	UINT64 value64;
    }     
           val;
}
t_cp0_reg;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static UINT32 maxlen_name_left, maxlen_name_right;
static bool   val64_left;

/* CP0 registers */
static t_cp0_reg regs[] =
{
    { "Index",         32, SYSCON_CPU_CP0_INDEX_ID,	       GDB_FR_CP0_INDEX    ,0,0},
    { "Random",	       32, SYSCON_CPU_CP0_RANDOM_ID,	       GDB_FR_CP0_RANDOM   ,0,0},
    { "EntryLo0",      64, SYSCON_CPU_CP0_ENTRYLO0_ID,	       GDB_FR_CP0_ENTRYLO0 ,0,0},
    { "EntryLo1",      64, SYSCON_CPU_CP0_ENTRYLO1_ID,	       GDB_FR_CP0_ENTRYLO1 ,0,0},
    { "Context",       64, SYSCON_CPU_CP0_CONTEXT_ID,	       GDB_FR_CP0_CONTEXT  ,0,0},
    { "PageMask",      32, SYSCON_CPU_CP0_PAGEMASK_ID,	       GDB_FR_CP0_PAGEMASK ,0,0},
    { "PageGrain",     32, SYSCON_CPU_CP0_PAGEGRAIN_ID,	       GDB_FR_CP0_PAGEGRAIN,0,0},
    { "Wired",	       32, SYSCON_CPU_CP0_WIRED_ID,	       GDB_FR_CP0_WIRED	   ,0,0},
    { "HWREna",	       32, SYSCON_CPU_CP0_HWRENA_ID,	       GDB_FR_CP0_HWRENA   ,0,0},
    { "BadVAddr",      64, SYSCON_CPU_CP0_BADVADDR_ID,	       GDB_FR_BADVADDR	   ,0,0},
    { "Count",	       32, SYSCON_CPU_CP0_COUNT_ID,	       GDB_FR_CP0_REG9	   ,0,0},
    { "EntryHi",       64, SYSCON_CPU_CP0_ENTRYHI_ID,	       GDB_FR_CP0_ENTRYHI  ,0,0},
    { "Compare",       32, SYSCON_CPU_CP0_COMPARE_ID,	       GDB_FR_CP0_REG11	   ,0,0},
    { "Status",	       32, SYSCON_CPU_CP0_STATUS_ID,	       GDB_FR_STATUS	   ,0,0},
    { "IntCtl",        32, SYSCON_CPU_CP0_INTCTL_ID,	       GDB_FR_CP0_INTCTL   ,0,0},
    { "SRSCtl",        32, SYSCON_CPU_CP0_SRSCTL_ID,	       GDB_FR_CP0_SRSCTL   ,0,0},
    { "SRSMap",        32, SYSCON_CPU_CP0_SRSMAP_ID,	       GDB_FR_CP0_SRSMAP   ,0,0},
    { "Cause",	       32, SYSCON_CPU_CP0_CAUSE_ID,	       GDB_FR_CAUSE	   ,0,0},
    { "EPC",	       64, SYSCON_CPU_CP0_EPC_ID,	       GDB_FR_EPC	   ,0,0},
    { "PRId",	       32, SYSCON_CPU_CP0_PRID_ID,	       GDB_FR_CP0_PRID	   ,0,0},
    { "EBase",	       32, SYSCON_CPU_CP0_EBASE_ID,	       GDB_FR_CP0_EBASE	   ,0,0},
    { "Config",	       32, SYSCON_CPU_CP0_CONFIG_ID,	       GDB_FR_CP0_CONFIG   ,0,0},
    { "Config1",       32, SYSCON_CPU_CP0_CONFIG1_ID,	       GDB_FR_CP0_CONFIG1  ,0,0},
    { "Config2",       32, SYSCON_CPU_CP0_CONFIG2_ID,	       GDB_FR_CP0_CONFIG2  ,0,0},
    { "Config3",       32, SYSCON_CPU_CP0_CONFIG3_ID,	       GDB_FR_CP0_CONFIG3  ,0,0},
    { "LLAddr",	       64, SYSCON_CPU_CP0_LLADDR_ID,	       -1		   ,0,0},
    { "WatchLo",       64, SYSCON_CPU_CP0_WATCHLO_ID,	       GDB_FR_CP0_WATCHLO  ,0,0},
    { "WatchHi",       64, SYSCON_CPU_CP0_WATCHHI_ID,	       GDB_FR_CP0_WATCHHI  ,0,0},
    { "XContext",      64, SYSCON_CPU_CP0_XCONTEXT_ID,	       -1		   ,0,0},
    { "Debug",	       32, SYSCON_CPU_CP0_DEBUG_ID,	       GDB_FR_CP0_DEBUG	   ,0,0},
    { "TraceControl",  32, SYSCON_CPU_CP0_TRACECONTROL_ID,     -1		   ,0,0},
    { "TraceControl2", 32, SYSCON_CPU_CP0_TRACECONTROL2_ID,    -1		   ,0,0},
    { "UserTraceData", 32, SYSCON_CPU_CP0_USERTRACEDATA_ID,    -1		   ,0,0},
    { "TraceBPC",      32, SYSCON_CPU_CP0_TRACEBPC_ID,	       -1		   ,0,0},
    { "DEPC",	       64, SYSCON_CPU_CP0_DEPC_ID,	       GDB_FR_CP0_DEPC	   ,0,0},
    { "PerfCntCtrl0",  32, SYSCON_CPU_CP0_PERFCOUNT_ID,	       -1		   ,0,0},
    { "PerfCntCount0", 32, SYSCON_CPU_CP0_PERFCOUNT_COUNT0_ID, -1		   ,0,0},
    { "PerfCntCtrl1",  32, SYSCON_CPU_CP0_PERFCOUNT_CTRL1_ID,  -1		   ,0,0},
    { "PerfCntCount1", 32, SYSCON_CPU_CP0_PERFCOUNT_COUNT1_ID, -1		   ,0,0},
    { "PerfCntCtrl2",  32, SYSCON_CPU_CP0_PERFCOUNT_CTRL2_ID,  -1		   ,0,0},
    { "PerfCntCount2", 32, SYSCON_CPU_CP0_PERFCOUNT_COUNT2_ID, -1		   ,0,0},
    { "ErrCtl",	       32, SYSCON_CPU_CP0_ERRCTL_ID,	       -1		   ,0,0},
    { "DErrCtl",       32, SYSCON_CPU_CP0_DERRCTL_ID,	       -1		   ,0,0},
    { "IErrCtl",       32, SYSCON_CPU_CP0_IERRCTL_ID,	       -1		   ,0,0},
    { "CacheErr",      32, SYSCON_CPU_CP0_CACHEERR_ID,	       -1		   ,0,0},

    { "TagLo",	       32, SYSCON_CPU_CP0_TAGLO_ID,	       -1		   ,0,0},
    { "ITagLo",	       32, SYSCON_CPU_CP0_ITAGLO_ID,	       -1		   ,0,0},
    { "DTagLo",	       32, SYSCON_CPU_CP0_DTAGLO_ID,	       -1		   ,0,0},
    { "L23TagLo",      32, SYSCON_CPU_CP0_L23TAGLO_ID,	       -1		   ,0,0},

    { "DataLo",	       32, SYSCON_CPU_CP0_DATALO_ID,	       -1		   ,0,0},
    { "IDataLo",       32, SYSCON_CPU_CP0_IDATALO_ID,	       -1		   ,0,0},
    { "DDataLo",       32, SYSCON_CPU_CP0_DDATALO_ID,	       -1		   ,0,0},
    { "L23DataLo",     32, SYSCON_CPU_CP0_L23DATALO_ID,	       -1		   ,0,0},

    { "TagHi",	       32, SYSCON_CPU_CP0_TAGHI_ID,	       -1		   ,0,0},
    { "ITagHi",	       32, SYSCON_CPU_CP0_ITAGHI_ID,	       -1		   ,0,0},
    { "DTagHi",	       32, SYSCON_CPU_CP0_DTAGHI_ID,	       -1		   ,0,0},
    { "L23TagHi",      32, SYSCON_CPU_CP0_L23TAGHI_ID,	       -1		   ,0,0},

    { "DataHi",	       32, SYSCON_CPU_CP0_DATAHI_ID,	       -1		   ,0,0},
    { "IDataHi",       32, SYSCON_CPU_CP0_IDATAHI_ID,	       -1		   ,0,0},
    { "DDataHi",       32, SYSCON_CPU_CP0_DDATAHI_ID,	       -1		   ,0,0},
    { "L23DataHi",     32, SYSCON_CPU_CP0_L23DATAHI_ID,	       -1		   ,0,0},

    { "ErrorEPC",      64, SYSCON_CPU_CP0_ERROREPC_ID,	       GDB_FR_CP0_ERROREPC ,0,0},
    { "DESAVE",	       64, SYSCON_CPU_CP0_DESAVE_ID,	       -1		   ,0,0},

    { "IWatchLo0",     64, SYSCON_CPU_CP0_IWATCHLO0_ID,        -1                  ,0,0},
    { "IWatchHi0",     64, SYSCON_CPU_CP0_IWATCHHI0_ID,        -1                  ,0,0},
    { "IWatchLo1",     64, SYSCON_CPU_CP0_IWATCHLO1_ID,        -1                  ,0,0},
    { "IWatchHi1",     64, SYSCON_CPU_CP0_IWATCHHI1_ID,        -1                  ,0,0},
    { "DWatchLo0",     64, SYSCON_CPU_CP0_DWATCHLO0_ID,        -1                  ,0,0},
    { "DWatchHi0",     64, SYSCON_CPU_CP0_DWATCHHI0_ID,        -1                  ,0,0},
    { "DWatchLo1",     64, SYSCON_CPU_CP0_DWATCHLO1_ID,        -1                  ,0,0},
    { "DWatchHi1",     64, SYSCON_CPU_CP0_DWATCHHI1_ID,        -1                  ,0,0},

    { "MVPControl",    32, SYSCON_CPU_CP0_MVPCONTROL_ID,       -1		   ,0,0},
    { "MVPConf0",      32, SYSCON_CPU_CP0_MVPCONF0_ID,         -1		   ,0,0},
    { "MVPConf1",      32, SYSCON_CPU_CP0_MVPCONF1_ID,         -1		   ,0,0},
    { "VPEControl",    32, SYSCON_CPU_CP0_VPECONTROL_ID,       -1		   ,0,0},
    { "VPEConf0",      32, SYSCON_CPU_CP0_VPECONF0_ID,         -1		   ,0,0},
    { "VPEConf1",      32, SYSCON_CPU_CP0_VPECONF1_ID,         -1		   ,0,0},
    { "YQMask",        32, SYSCON_CPU_CP0_YQMASK_ID,           -1		   ,0,0},
    { "VPESchedule",   32, SYSCON_CPU_CP0_VPESCHEDULE_ID,      -1		   ,0,0},
    { "VPEScheFBack",  32, SYSCON_CPU_CP0_VPESCHEFBACK_ID,     -1		   ,0,0},
    { "TCStatus",      32, SYSCON_CPU_CP0_TCSTATUS_ID,         -1		   ,0,0},
    { "TCBind",        32, SYSCON_CPU_CP0_TCBIND_ID,           -1		   ,0,0},
    { "TCRestart",     64, SYSCON_CPU_CP0_TCRESTART_ID,        -1		   ,0,0},
    { "TCHalt",        32, SYSCON_CPU_CP0_TCHALT_ID,           -1		   ,0,0},
    { "TCContext",     64, SYSCON_CPU_CP0_TCCONTEXT_ID,        -1		   ,0,0},
    { "TCSchedule",    32, SYSCON_CPU_CP0_TCSCHEDULE_ID,       -1		   ,0,0},
    { "TCScheFBack",   32, SYSCON_CPU_CP0_TCSCHEFBACK_ID,      -1		   ,0,0},
    { "SRSConf0",      32, SYSCON_CPU_CP0_SRSCONF0_ID,         -1		   ,0,0},
    { "SRSConf1",      32, SYSCON_CPU_CP0_SRSCONF1_ID,         -1		   ,0,0},
    { "SRSConf2",      32, SYSCON_CPU_CP0_SRSCONF2_ID,         -1		   ,0,0},
    { "SRSConf3",      32, SYSCON_CPU_CP0_SRSCONF3_ID,         -1		   ,0,0},
    { "SRSConf4",      32, SYSCON_CPU_CP0_SRSCONF4_ID,         -1		   ,0,0}
};
#define REG_COUNT (sizeof(regs)/sizeof(t_cp0_reg))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static bool
sys_uncached( void );

static void
sys_l2_flush_all( void );

static int
compare(
   t_cp0_reg *x,
   t_cp0_reg *y );

static UINT32
find_reg(
   char *reg_name );

static bool
print_reg( 
    bool   multi,
    bool   silent,
    bool   width32,
#define W32 TRUE
#define W64 FALSE
    bool   even,
    char   *s,
    UINT64 value,
    bool   shell );

static UINT32
print_regs(
    UINT32     tc,
    bool       all,
    bool       silent,
    UINT32     index_single,
    t_gdb_regs *context );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          sys_dcache_flush_all
 *  Description :
 *  -------------
 *
 *  Flush entire DCACHE.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_dcache_flush_all( void )
{
    UINT32 line;
    UINT32 addr;

    if( sys_uncached() )
        return;

    /* Flush DCACHE */
    for( line = 0, addr = KSEG0(0);
         line < sys_dcache_lines; 
	 line++, addr += sys_dcache_linesize )
    {
        sys_dcache_flush_index( addr );
    }

    /* Flush L2 */
    if( sys_l2cache_enabled )
	sys_l2_flush_all();
}  


/************************************************************************
 *
 *                          sys_icache_invalidate_all
 *  Description :
 *  -------------
 *
 *  Invalidate entire ICACHE.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_icache_invalidate_all( void )
{
    UINT32 line;
    UINT32 addr;

    if( sys_uncached() )
        return;

    /* Flush L2 */
    if( sys_l2cache_enabled )
        sys_l2_flush_all();

    /* Invalidate ICACHE */
    for( line = 0, addr = KSEG0(0); 
         line < sys_icache_lines; 
         line++, addr += sys_icache_linesize )
    {
        sys_icache_invalidate_index( addr );
    }

    sys_flush_pipeline();
}


/************************************************************************
 *
 *                          sys_flush_caches
 *  Description :
 *  -------------
 *
 *  First flush entire DCACHE, then invalidate entire ICACHE
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_flush_caches( void )
{
    sys_dcache_flush_all();
    sys_icache_invalidate_all();
}


/************************************************************************
 *
 *                          sys_flush_cache_line
 *  Description :
 *  -------------
 *
 *  First flush DCACHE line, then invalidate ICACHE line
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_flush_cache_line( 
    void *addr )
{
    /* word align address */
    addr = (void *)((UINT32)addr & ~0x3);

    sys_dcache_flush_addr( (UINT32)addr );
    sys_icache_invalidate_addr( (UINT32)addr, TRUE );
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *  			compare
 ************************************************************************/
static int
compare(
   t_cp0_reg *x,
   t_cp0_reg *y )
{
   if( !x->name )
       return 1;
   if( !y->name )
       return -1;

   return strcmp( x->name, y->name );
}


/************************************************************************
 *  find_reg
 ************************************************************************/
static UINT32
find_reg(
    char *reg_name )
{
    UINT32 i;

    for (i = 0; i < REG_COUNT; i++)
	if (strcasecmp(reg_name, regs[i].name) == 0)
	    break;
 
    return i;
}


/************************************************************************
 *
 *                          print_reg
 *  Description :
 *  -------------
 *
 *  Print CP0 register while handling indentation so that print of 
 *  additional registers is lined up nicely.
 *
 ************************************************************************/
static bool
print_reg( 
    bool   multi,
    bool   silent,
    bool   width32,
    bool   even,
    char   *s,
    UINT64 value,
    bool   shell )
{
    char   msg[80];
    UINT32 len;
    UINT32 i;
    UINT32 maxlen_left;
    
    len = strlen( s );

    if( silent )
    {
        if( even )
	    maxlen_name_left  = MAX(maxlen_name_left, len);
	else
	    maxlen_name_right = MAX(maxlen_name_right, len);

	if( !width32 )
	   val64_left = TRUE;
	
	return OK;
    }
    else
        maxlen_left = maxlen_name_left + (val64_left ? 23 : 15);

    strcpy( msg, s );

    i = len;

    if( multi )
    {
        for( ; i<(even ? maxlen_name_left : maxlen_name_right); i++ )
            msg[i] = ' ';
    }

    if( width32 )
        sprintf( &msg[i], " = 0x%08x", (UINT32)value );
    else
        sprintf( &msg[i], " = 0x%08x%08x", HI32(value), LO32(value) );

    if( even && multi )
    {
        len = strlen( msg );
    
        for( i=len; i<maxlen_left; i++ )
	    msg[i] = ' ';

	msg[i] = '\0';
    }
    else
        strcat( msg, "\n" );

    if( shell )
        return SHELL_PUTS( msg );
    else
    {
        printf( "%s", msg );
	return FALSE;
    }
}


/************************************************************************
 *
 *                          print_regs
 *  Description :
 *  -------------
 *
 *  Print one or all CP0 registers that are available for CPU.
 *
 ************************************************************************/
static UINT32
print_regs(
    UINT32     tc,	   
    bool       all,
    bool       silent,
    UINT32     index_single,
    t_gdb_regs *context )
{
    UINT32 i;
    UINT32 rc;
    char   *name;
    UINT8  width;
    UINT32 syscon_id;
    UINT32 gdb_offset;
    bool   even = TRUE;
    UINT32 value32;
    UINT64 value64;

    if( silent )
    {
        maxlen_name_left  = 0;
	maxlen_name_right = 0;
	val64_left        = FALSE;
    }

    for(i=(all ? 0 : index_single); i<=(all ? REG_COUNT-1 : index_single); i++ )
    {
        name  = regs[i].name;
	width = sys_64bit ? regs[i].width : 32;

	if( silent )
	{
	    syscon_id  = regs[i].syscon_id;
	    gdb_offset = regs[i].gdb_offset;

	    if (sys_mt && tc != 0) {
		t_syscon_reginfo_def reginfo;
		reginfo.id = syscon_id;
		rc = SYSCON_read (SYSCON_CPU_REGINFO_ID, &reginfo, sizeof(reginfo));
		
		if (rc == OK) {
		    if (width == 64)
			value64 = (UINT64) sys_cp0_mtread32(reginfo.reg, reginfo.sel, tc);
		    else
			value32 = sys_cp0_mtread32(reginfo.reg, reginfo.sel, tc);
		}
	    }
	    else {
		rc = 
		    (width == 64) ?
                    SYSCON_read( syscon_id, &value64, sizeof(UINT64) ) :
		    SYSCON_read( syscon_id, &value32, sizeof(UINT32) );
	    }

	    if( (rc == OK) && context )
	    {
                if( EXCEP_nmi && 
	            ((gdb_offset == GDB_FR_CAUSE)       ||
		     (gdb_offset == GDB_FR_CP0_WATCHLO) ||
		     (gdb_offset == GDB_FR_CP0_WATCHHI)) )
	        {
                    /*  Cause, WatchLo, WatchHi don't make sense for NMI, 
		     *  since they are modified by the reset code, 
		     *  which is excecuted upon an NMI.
		     *  Status is also modified somewhat (IE,KSU fields
		     *  are cleared), but we display it anyway.
		     */
	            rc = NOT_OK;
	        }
	        else if( gdb_offset != -1 )
	        {
	            if(width == 64)
		        value64 = *GDB_REG64(context, gdb_offset);
		    else
		        value32 = *GDB_REG32(context, gdb_offset);
	        }
	        else
	        {
		    rc = NOT_OK;
	        }
	    }

	    regs[i].rc = rc;

	    if(width == 32)
	        regs[i].val.value32 = value32;
	    else
	        regs[i].val.value64 = value64;
        }
	else
	{
	    rc = regs[i].rc;

	    if(width == 32)
	        value32 = regs[i].val.value32;
	    else
	        value64 = regs[i].val.value64;
        }

	if( (rc != OK) && !all )
	    return SHELL_ERROR_UNKNOWN_CP0_REG;

	if( rc == OK )
	{
	    if( print_reg(
		     all,
		     silent,
	             ((width == 64) ? W64 : W32),
		     even,
		     name,
		     ((width == 64) ? value64 : (UINT64)value32),
		     context ? FALSE : TRUE ) )
	    {
	        return SHELL_ERROR_CONTROL_C_DETECTED;
	    }
	    else
	        even = !even;
	}
    }

    if( !context )
    {
        if( all && !even && !silent )
            return SHELL_PUTS( "\n" ) ? SHELL_ERROR_CONTROL_C_DETECTED : OK;
    }
    else
    {
        if( sys_fpu && (UINT32)context->cp0_status & M_StatusCU1 )
        {
	    print_reg( all, silent, W32, even, "FCSR", (UINT32)context->cp1_fsr, FALSE );
	    even = !even;
        }

        print_reg(
	    all,
	    silent,
	    (sys_64bit ? W64 : W32),
	    even,  
	    "Hi", 
	    context->hi,
	    FALSE );

        print_reg(
	    all,
	    silent,
	    (sys_64bit ? W64 : W32),
	    !even,  
	    "Lo", 
	    context->lo,
	    FALSE );

        if( !even && !silent ) 
	    printf( "\n" );
    }

    return OK;
}


/************************************************************************
 *  sys_uncached
 ************************************************************************/
static bool
sys_uncached( void )
{
    UINT32 config;
  
    SYSCON_read( SYSCON_CPU_CP0_CONFIG_ID, 
		 &config,
		 sizeof(UINT32) );

    return
        ((config & M_ConfigK0) >> S_ConfigK0) == K_CacheAttrU ?
	    TRUE : FALSE;
}


/************************************************************************
 *  sys_l2_flush_all
 ************************************************************************/
static void
sys_l2_flush_all( void )
{	     
    UINT32 line;
    UINT32 addr;

    for( line = 0, addr = KSEG0(0);
         line < sys_l2cache_lines; 
	 line++, addr += sys_l2cache_linesize )
    {
        sys_scache_flush_index( addr );
    }
}


/************************************************************************
 *
 *                          sys_cp0_printreg_all
 *  Description :
 *  -------------
 *
 *  Print CP0 registers.
 *
 ************************************************************************/
UINT32
sys_cp0_printreg_all( 
    UINT32 tc,
    t_gdb_regs  *context ) // If not NULL, this holds the context to be dumped.
			   // If NULL, print current values of all CP0 regs.
{
    static bool init = TRUE;

    /* Note : This function is self-initialising. The first
     * call causes the cp0 table to be sorted.
     */
    if( init )
    {
	init = FALSE;	

        /* Sort the table */
        qsort( (char *)regs,
               (int)REG_COUNT,
	       (int)sizeof(t_cp0_reg),
	       compare );
    }

    /* Pass 1 : Get availability and values of registers and determine indentations */
    print_regs( tc, TRUE, TRUE,  0, context );

    /* Pass 2 : Print values */
    return print_regs( tc, TRUE, FALSE, 0, context );
}


/************************************************************************
 *
 *                          sys_cp0_printreg
 *  Description :
 *  -------------
 *
 *  Print value of CP0 register specified by name.
 *
 ************************************************************************/
UINT32
sys_cp0_printreg(
    UINT32 tc,	     // TC to use
    char *reg_name ) // Name of register.
{
    /* Register selected by name */
    UINT32 i = find_reg( reg_name );

    /* Pass 1 (get availability and value of register */
    print_regs(tc, FALSE, TRUE, i, NULL );

    return ( i == REG_COUNT ) ?
               SHELL_ERROR_UNKNOWN_CP0_REG :
	print_regs(tc, FALSE, FALSE, i, NULL ); /* Pass 2 : Print value */
}


/************************************************************************
 *
 *                          sys_cp0_writereg
 *  Description :
 *  -------------
 *
 *  Write to CP0 register specified by name.
 *
 ************************************************************************/
UINT32
sys_cp0_writereg(
    UINT32  tc,	      // TC to use
    char   *reg_name, // Name of register.
    UINT64 value )    // Value to be written.
{
    /* Register selected by name */
    UINT32 width;
    UINT32 rc;
    UINT32 i;
    UINT32 value32 = (UINT32)value;
    
    i = find_reg( reg_name );

    if( i == REG_COUNT )
        return SHELL_ERROR_UNKNOWN_CP0_REG;

    width = sys_64bit ? regs[i].width : 32;

    rc = 
        (width == 64) ?
            SYSCON_write( regs[i].syscon_id, &value,   sizeof(UINT64) ) :
            SYSCON_write( regs[i].syscon_id, &value32, sizeof(UINT32) );
 
    if( rc != OK )
        return SHELL_ERROR_RO_CP0_REG;
    else
        return OK;
}



