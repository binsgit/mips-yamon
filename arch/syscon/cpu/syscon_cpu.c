
/************************************************************************
 *
 *  syscon_cpu.c
 *
 *  CPU specific parts of SYSCON module
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
#include <syscon.h>
#include <mips.h>
#include <qed.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Structures for CP0 registers */

typedef struct 
{
    UINT32 id;
    UINT8  number;
    UINT8  sel;
    bool   valid;
    UINT8  regsize;
}
t_syscon_cp0_reg;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Reset value of CONFIG1 register */
static UINT32 config1_init;

/* Cache/MMU configurability, availability */
static bool   cache_configurable;
static bool   mmu_configurable;
static bool   tlb;
static UINT8  tlb_reset;

static UINT8  register_sets;

/* Number of clock cycles per COUNT register increment */
static UINT32 cycle_per_count;

/* Pointer to array of SYSCON objects */
static t_syscon_obj       *syscon_objects;

/* CP0 registers */
#define SYSCON_INDEX		    0
#define SYSCON_RANDOM		    1
#define SYSCON_ENTRYLO0		    2
#define SYSCON_ENTRYLO1		    3
#define SYSCON_CONTEXT		    4
#define SYSCON_PAGEMASK		    5
#define SYSCON_PAGEGRAIN	    6
#define SYSCON_WIRED		    7
#define SYSCON_HWRENA		    8
#define SYSCON_BADVADDR		    9
#define SYSCON_COUNT		    10
#define SYSCON_ENTRYHI		    11
#define SYSCON_COMPARE		    12
#define SYSCON_STATUS		    13
#define SYSCON_INTCTL		    14
#define SYSCON_SRSCTL		    15
#define SYSCON_SRSMAP		    16
#define SYSCON_CAUSE		    17
#define SYSCON_EPC		    18
#define SYSCON_PRID		    19
#define SYSCON_EBASE		    20
#define SYSCON_CONFIG		    21
#define SYSCON_CONFIG1		    22
#define SYSCON_CONFIG2		    23
#define SYSCON_CONFIG3		    24
#define SYSCON_LLADDR		    25
#define SYSCON_WATCHLO		    26
#define SYSCON_WATCHHI		    27
#define SYSCON_XCONTEXT		    28
#define SYSCON_DEBUG		    29
#define SYSCON_TRACECONTROL	    30
#define SYSCON_TRACECONTROL2	    31
#define SYSCON_USERTRACEDATA	    32
#define SYSCON_TRACEBPC		    33
#define SYSCON_DEPC		    34
#define SYSCON_PERFCOUNT	    35      /* CTRL0 */
#define SYSCON_PERFCOUNT_COUNT0	    36
#define SYSCON_PERFCOUNT_CTRL1	    37
#define SYSCON_PERFCOUNT_COUNT1	    38
#define SYSCON_PERFCOUNT_CTRL2	    39
#define SYSCON_PERFCOUNT_COUNT2	    40
#define SYSCON_ERRCTL		    41
#define SYSCON_DERRCTL		    42
#define SYSCON_IERRCTL		    43
#define SYSCON_CACHEERR		    44

#define SYSCON_TAGLO		    45
#define SYSCON_ITAGLO		    46
#define SYSCON_DTAGLO		    47
#define SYSCON_L23TAGLO		    48

#define SYSCON_TAGHI		    49
#define SYSCON_ITAGHI		    50
#define SYSCON_DTAGHI		    51
#define SYSCON_L23TAGHI		    52

#define SYSCON_DATALO		    53
#define SYSCON_IDATALO		    54
#define SYSCON_DDATALO		    55
#define SYSCON_L23DATALO	    56

#define SYSCON_DATAHI		    57
#define SYSCON_IDATAHI		    58
#define SYSCON_DDATAHI		    59
#define SYSCON_L23DATAHI	    60

#define SYSCON_ERROREPC		    61
#define SYSCON_DESAVE		    62

#define SYSCON_WATCHLO0		    63
#define SYSCON_WATCHHI0		    64
#define SYSCON_WATCHLO1		    65
#define SYSCON_WATCHHI1		    66
#define SYSCON_WATCHLO2		    67
#define SYSCON_WATCHHI2		    68
#define SYSCON_WATCHLO3		    69
#define SYSCON_WATCHHI3		    70

#define SYSCON_MVPCONTROL	    71
#define SYSCON_MVPCONF0		    72
#define SYSCON_MVPCONF1		    73
#define SYSCON_VPECONTROL	    74
#define SYSCON_VPECONF0		    75
#define SYSCON_VPECONF1		    76
#define SYSCON_YQMASK		    77
#define SYSCON_VPESCHEDULE	    78
#define SYSCON_VPESCHEFBACK	    79
#define SYSCON_TCSTATUS		    80
#define SYSCON_TCBIND		    81
#define SYSCON_TCHALT		    82
#define SYSCON_TCRESTART	    83
#define SYSCON_TCCONTEXT	    84
#define SYSCON_TCSCHEDULE	    85
#define SYSCON_TCSCHEFBACK	    86
#define SYSCON_SRSCONF0		    87
#define SYSCON_SRSCONF1		    88
#define SYSCON_SRSCONF2		    89
#define SYSCON_SRSCONF3		    90
#define SYSCON_SRSCONF4		    91

#define SYSCON_CONFIG4		    92
#define SYSCON_CONFIG5		    93
#define SYSCON_CONFIG6		    94
#define SYSCON_CONFIG7		    95

#define SYSCON_TRACEIBPC	    96
#define SYSCON_TRACEDBPC	    97
#define SYSCON_USERLOCAL	    98
#define SYSCON_VPEOPT		    99

#define SYSCON_CP0_REG_COUNT	    (SYSCON_VPEOPT+1)

static t_syscon_cp0_reg cp0_reg[SYSCON_CP0_REG_COUNT] =
{
    { SYSCON_CPU_CP0_INDEX_ID,	          R_C0_Index,	      R_C0_SelIndex,         FALSE, 4 },
    { SYSCON_CPU_CP0_RANDOM_ID,		  R_C0_Random,	      R_C0_SelRandom,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ENTRYLO0_ID,	  R_C0_EntryLo0,      R_C0_SelEntryLo0,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ENTRYLO1_ID,	  R_C0_EntryLo1,      R_C0_SelEntryLo1,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONTEXT_ID,          R_C0_Context,	      R_C0_SelContext,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PAGEMASK_ID,         R_C0_PageMask,      R_C0_SelPageMask,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PAGEGRAIN_ID,        R_C0_PageGrain,     R_C0_SelPageGrain,     FALSE, 4 },
    { SYSCON_CPU_CP0_WIRED_ID,	          R_C0_Wired,	      R_C0_SelWired,	     FALSE, 4 },
    { SYSCON_CPU_CP0_HWRENA_ID,           R_C0_HWREna,	      R_C0_SelHWREna,	     FALSE, 4 },
    { SYSCON_CPU_CP0_BADVADDR_ID,         R_C0_BadVAddr,      R_C0_SelBadVAddr,	     FALSE, 4 },
    { SYSCON_CPU_CP0_COUNT_ID,	          R_C0_Count,	      R_C0_SelCount,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ENTRYHI_ID,          R_C0_EntryHi,	      R_C0_SelEntryHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_COMPARE_ID,          R_C0_Compare,	      R_C0_SelCompare,	     FALSE, 4 },
    { SYSCON_CPU_CP0_STATUS_ID,	          R_C0_Status,	      R_C0_SelStatus,	     FALSE, 4 },
    { SYSCON_CPU_CP0_INTCTL_ID,		  R_C0_IntCtl,	      R_C0_SelIntCtl,	     FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCTL_ID,		  R_C0_SRSCtl,	      R_C0_SelSRSCtl,	     FALSE, 4 },
    { SYSCON_CPU_CP0_SRSMAP_ID,		  R_C0_SRSMap,	      R_C0_SelSRSMap,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CAUSE_ID,	          R_C0_Cause,	      R_C0_SelCause,	     FALSE, 4 },
    { SYSCON_CPU_CP0_EPC_ID,	          R_C0_EPC,	      R_C0_SelEPC,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PRID_ID,	          R_C0_PRId,	      R_C0_SelPRId,	     FALSE, 4 },
    { SYSCON_CPU_CP0_EBASE_ID,            R_C0_EBase,	      R_C0_SelEBase,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG_ID,	          R_C0_Config,	      R_C0_SelConfig,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG1_ID,	  R_C0_Config1,	      R_C0_SelConfig1,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG2_ID,	  R_C0_Config2,	      R_C0_SelConfig2,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG3_ID,	  R_C0_Config3,	      R_C0_SelConfig3,	     FALSE, 4 },
    { SYSCON_CPU_CP0_LLADDR_ID,	          R_C0_LLAddr,	      R_C0_SelLLAddr,	     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHLO_ID,          R_C0_WatchLo,	      R_C0_SelWatchLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHHI_ID,          R_C0_WatchHi,	      R_C0_SelWatchHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_XCONTEXT_ID,         R_C0_XContext,      R_C0_SelXContext,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DEBUG_ID,	          R_C0_Debug,	      R_C0_SelDebug,	     FALSE, 4 },
    { SYSCON_CPU_CP0_TRACECONTROL_ID,     R_C0_TraceControl,  R_C0_SelTraceControl,  FALSE, 4 },
    { SYSCON_CPU_CP0_TRACECONTROL2_ID,    R_C0_TraceControl2, R_C0_SelTraceControl2, FALSE, 4 },
    { SYSCON_CPU_CP0_USERTRACEDATA_ID,    R_C0_UserTraceData, R_C0_SelUserTraceData, FALSE, 4 },
    { SYSCON_CPU_CP0_TRACEBPC_ID,         R_C0_TraceBPC,      R_C0_SelTraceBPC,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DEPC_ID,	          R_C0_DEPC,	      R_C0_SelDEPC,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_ID,        R_C0_PerfCnt,	      0 /* CTRL0 */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_COUNT0_ID, R_C0_PerfCnt,	      1 /* CNT0	 */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_CTRL1_ID , R_C0_PerfCnt,	      2 /* CTRL1 */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_COUNT1_ID, R_C0_PerfCnt,	      3 /* CNT1	 */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_CTRL2_ID,  R_C0_PerfCnt,	      4 /* CTRL2 */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_PERFCOUNT_COUNT2_ID, R_C0_PerfCnt,	      5 /* CNT2  */,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ERRCTL_ID,		  R_C0_ErrCtl,	      R_C0_SelErrCtl,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DERRCTL_ID,	  R_C0_DErrCtl,	      R_C0_SelDErrCtl,	     FALSE, 4 },
    { SYSCON_CPU_CP0_IERRCTL_ID,	  R_C0_IErrCtl,	      R_C0_SelIErrCtl,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CACHEERR_ID,         R_C0_CacheErr,      R_C0_SelCacheErr,	     FALSE, 4 },

    { SYSCON_CPU_CP0_TAGLO_ID,	          R_C0_TagLo,	      R_C0_SelTagLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ITAGLO_ID,	          R_C0_ITagLo,	      R_C0_SelITagLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DTAGLO_ID,           R_C0_DTagLo,	      R_C0_SelDTagLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_L23TAGLO_ID,	  R_C0_L23TagLo,      R_C0_SelL23TagLo,	     FALSE, 4 },

    { SYSCON_CPU_CP0_TAGHI_ID,	          R_C0_TagHi,	      R_C0_SelTagHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_ITAGHI_ID,           R_C0_ITagHi,	      R_C0_SelITagHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DTAGHI_ID,           R_C0_DTagHi,	      R_C0_SelDTagHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_L23TAGHI_ID,         R_C0_L23TagHi,      R_C0_SelL23TagHi,	     FALSE, 4 },

    { SYSCON_CPU_CP0_DATALO_ID,	          R_C0_DataLo,	      R_C0_SelDataLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_IDATALO_ID,          R_C0_IDataLo,	      R_C0_SelIDataLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DDATALO_ID,          R_C0_DDataLo,	      R_C0_SelDDataLo,	     FALSE, 4 },
    { SYSCON_CPU_CP0_L23DATALO_ID,        R_C0_L23DataLo,     R_C0_SelL23DataLo,     FALSE, 4 },

    { SYSCON_CPU_CP0_DATAHI_ID,	          R_C0_DataHi,	      R_C0_SelDataHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_IDATAHI_ID,          R_C0_IDataHi,	      R_C0_SelIDataHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DDATAHI_ID,          R_C0_DDataHi,	      R_C0_SelDDataHi,	     FALSE, 4 },
    { SYSCON_CPU_CP0_L23DATAHI_ID,        R_C0_L23DataHi,     R_C0_SelL23DataHi,     FALSE, 4 },

    { SYSCON_CPU_CP0_ERROREPC_ID,         R_C0_ErrorEPC,      R_C0_SelErrorEPC,	     FALSE, 4 },
    { SYSCON_CPU_CP0_DESAVE_ID,	          R_C0_DESAVE,	      R_C0_SelDESAVE,	     FALSE, 4 },

    { SYSCON_CPU_CP0_WATCHLO0_ID,         R_C0_WatchLo,       0,                     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHHI0_ID,         R_C0_WatchHi,       0,		     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHLO1_ID,         R_C0_WatchLo,       1,		     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHHI1_ID,         R_C0_WatchHi,       1,		     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHLO2_ID,         R_C0_WatchLo,       2,	             FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHHI2_ID,         R_C0_WatchHi,       2,		     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHLO3_ID,         R_C0_WatchLo,       3,		     FALSE, 4 },
    { SYSCON_CPU_CP0_WATCHHI3_ID,         R_C0_WatchHi,       3,		     FALSE, 4 },

    { SYSCON_CPU_CP0_MVPCONTROL_ID,	  R_C0_MVPCtl,        R_C0_SelMVPCtl,        FALSE, 4 },
    { SYSCON_CPU_CP0_MVPCONF0_ID,	  R_C0_MVPConf0,      R_C0_SelMVPConf0,	     FALSE, 4 },
    { SYSCON_CPU_CP0_MVPCONF1_ID,	  R_C0_MVPConf1,      R_C0_SelMVPConf1,      FALSE, 4 },
    { SYSCON_CPU_CP0_VPECONTROL_ID,	  R_C0_VPECtl,        R_C0_SelVPECtl,        FALSE, 4 },
    { SYSCON_CPU_CP0_VPECONF0_ID,	  R_C0_VPEConf0,      R_C0_SelVPEConf0,	     FALSE, 4 },
    { SYSCON_CPU_CP0_VPECONF1_ID,	  R_C0_VPEConf1,      R_C0_SelVPEConf1,	     FALSE, 4 },
    { SYSCON_CPU_CP0_YQMASK_ID, 	  R_C0_YQMask,        R_C0_SelYQMask,	     FALSE, 4 },
    { SYSCON_CPU_CP0_VPESCHEDULE_ID,	  R_C0_VPESchedule,   R_C0_SelVPESchedule,   FALSE, 4 },
    { SYSCON_CPU_CP0_VPESCHEFBACK_ID,	  R_C0_VPEScheFBack,  R_C0_SelVPEScheFBack,  FALSE, 4 },
    { SYSCON_CPU_CP0_TCSTATUS_ID,	  R_C0_TCStatus,      R_C0_SelTCStatus,	     FALSE, 4 },
    { SYSCON_CPU_CP0_TCBIND_ID,		  R_C0_TCBind,        R_C0_SelTCBind,        FALSE, 4 },
    { SYSCON_CPU_CP0_TCRESTART_ID,	  R_C0_TCRestart,     R_C0_SelTCRestart,     FALSE, 4 },
    { SYSCON_CPU_CP0_TCHALT_ID,		  R_C0_TCHalt,	      R_C0_SelTCHalt,	     FALSE, 4 },
    { SYSCON_CPU_CP0_TCCONTEXT_ID,	  R_C0_TCContext,     R_C0_SelTCContext,     FALSE, 4 },
    { SYSCON_CPU_CP0_TCSCHEDULE_ID,	  R_C0_TCSchedule,    R_C0_SelTCSchedule,    FALSE, 4 },
    { SYSCON_CPU_CP0_TCSCHEFBACK_ID,	  R_C0_TCScheFBack,   R_C0_SelTCScheFBack,   FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCONF0_ID,	  R_C0_SRSConf0,      R_C0_SelSRSConf0,      FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCONF1_ID,	  R_C0_SRSConf1,      R_C0_SelSRSConf1,      FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCONF2_ID,	  R_C0_SRSConf2,      R_C0_SelSRSConf2,      FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCONF3_ID,	  R_C0_SRSConf3,      R_C0_SelSRSConf3,      FALSE, 4 },
    { SYSCON_CPU_CP0_SRSCONF4_ID,	  R_C0_SRSConf4,      R_C0_SelSRSConf4,      FALSE, 4 },

    { SYSCON_CPU_CP0_CONFIG4_ID,	  R_C0_Config4,	      R_C0_SelConfig4,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG5_ID,	  R_C0_Config5,	      R_C0_SelConfig5,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG6_ID,	  R_C0_Config6,	      R_C0_SelConfig6,	     FALSE, 4 },
    { SYSCON_CPU_CP0_CONFIG7_ID,	  R_C0_Config7,	      R_C0_SelConfig7,	     FALSE, 4 },

    { SYSCON_CPU_CP0_TRACEIBPC_ID,	  R_C0_TraceIBPC,     R_C0_SelTraceIBPC,     FALSE, 4 },
    { SYSCON_CPU_CP0_TRACEDBPC_ID,	  R_C0_TraceDBPC,     R_C0_SelTraceDBPC,     FALSE, 4 },
    { SYSCON_CPU_CP0_USERLOCAL_ID,	  R_C0_UserLocal,     R_C0_SelUserLocal,     FALSE, 4 },
    { SYSCON_CPU_CP0_VPEOPT_ID,		  R_C0_VPEOpt,        R_C0_SelVPEOpt,        FALSE, 4 },
};

/************************************************************************
 *  Static function prototypes
 ************************************************************************/


/************************************************************************
 *  Implementation : Static functions registered for handling particular 
 *  SYSCON objects for particular CPU(s)
 ************************************************************************/


/************************************************************************
 *  cpu_read_cp0
 *  OBS: The size argument is buffer size - NOT register width
 ************************************************************************/
static UINT32
cpu_read_cp0(
    void   *param,
    void   *data,
    UINT32 size )
{
    t_syscon_cp0_reg *def = (t_syscon_cp0_reg *)data;
  
    if(!def->valid)
        return ERROR_SYSCON_UNKNOWN_PARAM;

    if( size == sizeof(UINT32) )
    {
	*(UINT32 *)param = sys_cp0_read32( def->number, def->sel );
	return OK;
    }
    else if( size == sizeof(UINT64) )
    {
        if( def->regsize == sizeof(UINT32) )
	    *(INT64 *)param = (INT64)sys_cp0_read32( def->number, def->sel );
	else
	    *(INT64 *)param =        sys_cp0_read64( def->number, def->sel );

	return OK;
    }
    else
	return ERROR_SYSCON_SIZE_MISMATCH;
}


/************************************************************************
 *  cpu_write_cp0
 *  OBS: The size argument is buffer size - NOT register width
 ************************************************************************/
static UINT32
cpu_write_cp0(
    void   *param,
    void   *data,
    UINT32 size )
{
    t_syscon_cp0_reg *def = (t_syscon_cp0_reg *)data;
  
    if(!def->valid)
        return ERROR_SYSCON_UNKNOWN_PARAM;

    if( size == sizeof(UINT32) )
    {
        if( def->regsize == sizeof(UINT32) )
            sys_cp0_write32( def->number, def->sel,               *(UINT32 *)param );
	else
            sys_cp0_write64( def->number, def->sel, (UINT64)(INT64)*(INT32 *)param );

	return OK;
    }
    else if( size == sizeof(UINT64) )
    {
        if( def->regsize == sizeof(UINT32) )
            sys_cp0_write32( def->number, def->sel, (UINT32)*(UINT64 *)param );
	else
            sys_cp0_write64( def->number, def->sel,         *(UINT64 *)param );

	return OK;
    }
    else
	return ERROR_SYSCON_SIZE_MISMATCH;
}


/************************************************************************
 *  cpu_cp0_config1_reset_mips32_read
 ************************************************************************/
static UINT32
cpu_cp0_config1_reset_mips32_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = config1_init;
    return OK;
}


/************************************************************************
 *  cpu_tlb_count_mips32_read
 ************************************************************************/
static UINT32
cpu_tlb_count_mips32_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 config1;
  
    SYSCON_read(
        SYSCON_CPU_CP0_CONFIG1_ID,
        (void *)&config1,
	sizeof(UINT32) );	

    *(UINT8 *)param = 
        (( config1 & M_Config1MMUSize ) >> S_Config1MMUSize) + 1;
    return OK;
}


/************************************************************************
 *  cpu_tlb_count_rm5261_read
 ************************************************************************/
static UINT32
cpu_tlb_count_rm5261_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = 
        (sys_processor == QED_RM52XX) ?
	    QED_RM52XX_TLB_ENTRIES :
	    QED_RM70XX_TLB_ENTRIES;

    return OK;
}

/************************************************************************
 *  reginfo_read
 ************************************************************************/
static UINT32 
reginfo_read ( 
    void *param,
    void *data,
    UINT32 size)
{
    t_syscon_reginfo_def *var;
    UINT32 i;

    if (size != sizeof(t_syscon_reginfo_def))
	return ERROR_SYSCON_SIZE_MISMATCH;

    var = (t_syscon_reginfo_def *)param;

    for (i = 0; i < SYSCON_CP0_REG_COUNT; i++)
    {
	if (cp0_reg[i].id == var->id) {
	    if (cp0_reg[i].valid) {
		var->reg = cp0_reg[i].number;
		var->sel = cp0_reg[i].sel;
		return OK;
	    }
	    break;
	}
    }

    return NOT_OK;
}

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_arch_cpu_init
 *  Description :
 *  -------------
 *
 *  Initialize cpu specific part of SYSCON
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_cpu_init( 
    t_syscon_obj *objects )		/* Array of SYSCON objects	*/
{
    UINT32 mask;
    UINT32 config, config1, config2, config3, ctrl;
    UINT32 srsconf;
    bool   taghilo, datahilo, lladdr, watch, trace, userlocal, cacheerr, errctl;
    UINT32 i;
    
    syscon_objects = objects;

    /* CP0 registers */
    for(i=0;i<SYSCON_CP0_REG_COUNT;i++)
    {
        syscon_register_generic(
	    cp0_reg[i].id,
	    cpu_read_cp0,  (void *)&(cp0_reg[i]),
	    cpu_write_cp0, (void *)&(cp0_reg[i]) );
    }

    taghilo = datahilo = TRUE;

    switch( sys_processor )
    {
      case MIPS_25Kf  :

	cp0_reg[SYSCON_L23TAGLO ].valid = TRUE;
	cp0_reg[SYSCON_L23DATALO].valid = TRUE;
	cp0_reg[SYSCON_L23TAGHI ].valid = TRUE;
	cp0_reg[SYSCON_L23DATAHI].valid = TRUE;

        /* Fallthrough !! */

      case MIPS_20Kc  :

        cp0_reg[SYSCON_DERRCTL].valid = TRUE;
	cp0_reg[SYSCON_IERRCTL].valid = TRUE;
	cp0_reg[SYSCON_ITAGLO ].valid = TRUE;
	cp0_reg[SYSCON_IDATALO].valid = TRUE;
	cp0_reg[SYSCON_DTAGLO ].valid = TRUE;
	cp0_reg[SYSCON_DDATALO].valid = TRUE;
	cp0_reg[SYSCON_ITAGHI ].valid = TRUE;
	cp0_reg[SYSCON_IDATAHI].valid = TRUE;
	cp0_reg[SYSCON_DTAGHI ].valid = TRUE;
	cp0_reg[SYSCON_DDATAHI].valid = TRUE;

	taghilo = datahilo = FALSE;

	goto common;

      case MIPS_74K:
	cp0_reg[SYSCON_ITAGHI].valid   = TRUE;
	cp0_reg[SYSCON_DTAGHI].valid   = TRUE;
	/* cp0_reg[SYSCON_L23TAGHI].valid = TRUE; */

	cp0_reg[SYSCON_CONFIG6].valid = TRUE;
	/* Fallthrough !! */

      case MIPS_34K:
      case MIPS_1004K:
        /* Fallthrough !! */

      case MIPS_24K :
      case MIPS_24KE :
	cp0_reg[SYSCON_CONFIG7].valid = TRUE;

	cp0_reg[SYSCON_ITAGLO].valid   = TRUE;
	cp0_reg[SYSCON_DTAGLO].valid   = TRUE;
	cp0_reg[SYSCON_IDATALO].valid  = TRUE;
	cp0_reg[SYSCON_DDATALO].valid  = TRUE;
	cp0_reg[SYSCON_IDATAHI].valid  = TRUE;
	cp0_reg[SYSCON_L23TAGLO].valid = TRUE;
	cp0_reg[SYSCON_L23DATALO].valid = TRUE;
	cp0_reg[SYSCON_L23DATAHI].valid = TRUE;

	taghilo = FALSE;
	datahilo = FALSE;
        goto common;

common:

      case MIPS_4Kc      :
      case MIPS_4Kmp     :
      case MIPS_4KEc     :
      case MIPS_4KEc_R2  :
      case MIPS_4KEmp    :
      case MIPS_4KEmp_R2 :
      case MIPS_4KSc     :
      case MIPS_4KSd     :
      case MIPS_M4K      :
      case MIPS_5K       :
      case MIPS_5KE      :

	/* Config registers */
	config = sys_cp0_read32( R_C0_Config, R_C0_SelConfig );

        /* Config1 */
	cp0_reg[SYSCON_CONFIG1  ].valid = TRUE;
	config1      = sys_cp0_read32( R_C0_Config1, R_C0_SelConfig1 );
	config1_init = config1;        /* Store initial setting of CP0 CONFIG1 register */

	cp0_reg[SYSCON_CONFIG2  ].valid = (config1 & M_Config1M) ? TRUE : FALSE;
	if( cp0_reg[SYSCON_CONFIG2].valid )
	{
	    config2 = sys_cp0_read32( R_C0_Config2, R_C0_SelConfig2 );
	    cp0_reg[SYSCON_CONFIG3].valid = (config2 & M_Config2M) ? TRUE : FALSE;
        }

	/* Trace */
	if( cp0_reg[SYSCON_CONFIG3].valid )
	{
	    config3 = sys_cp0_read32( R_C0_Config, R_C0_SelConfig3 );

	    trace =
	        (config3 & M_Config3TL) ?
	            TRUE : FALSE;

	    cp0_reg[SYSCON_TRACECONTROL ].valid = trace;
	    cp0_reg[SYSCON_TRACECONTROL2].valid = trace;
	    cp0_reg[SYSCON_USERTRACEDATA].valid = trace;
	    cp0_reg[SYSCON_TRACEIBPC    ].valid	= trace;
	    cp0_reg[SYSCON_TRACEDBPC    ].valid	= trace;

	    userlocal = 
	        (config3 & M_Config3ULRI) ?
	            TRUE : FALSE;
	    cp0_reg[SYSCON_USERLOCAL    ].valid	= userlocal;
        }

        /* Detect whether cache/TLB is configurable */

        /*  This feature is present specifically to support configuration
         *  testing of the core in a lead vehicle, and is not supported
         *  in any other environment.  Attempting to use this feature
         *  outside of the scope of a lead vehicle is a violation of the
         *  MIPS Architecture, and may cause unpredictable operation of
         *  the processor.
         */

        mask = sys_cpu_configurability();

        cache_configurable = 
            (mask & SYS_CPU_CONFIGURABILITY_CACHE) ? 
                TRUE : FALSE;

        mmu_configurable = 
            (mask & SYS_CPU_CONFIGURABILITY_MMU) ? 
	        TRUE : FALSE;

	/* TLB availability */
	tlb = 
	    ( (config & M_ConfigMT) >> S_ConfigMT == K_ConfigMT_TLBMMU ) ?
	        TRUE : FALSE;

        /* Watch */
	watch =
	    (config1 & M_Config1WR) ?
	        TRUE : FALSE;

	/* TagHi, TagLo, DataHi, DataLo */
	if (sys_processor == MIPS_M4K)
	    taghilo = datahilo = FALSE;

	/* LLAddr */
	lladdr = 
	    sys_mt ||
		(( sys_processor != MIPS_M4K ) &&
		 ( sys_processor != MIPS_5K  ) &&
		 ( sys_processor != MIPS_5KE ) &&
		 ( sys_processor != MIPS_24K ) &&
		 ( sys_processor != MIPS_24KE) &&
		 ( sys_processor != MIPS_34K ) &&
		 ( sys_processor != MIPS_1004K) &&
		 ( sys_processor != MIPS_74K ));

	/* CacheErr */
	cacheerr =
	  ( sys_processor != MIPS_4Kc      ) &&
	  ( sys_processor != MIPS_4Kmp     ) &&
	  ( sys_processor != MIPS_4KEc     ) &&
	  ( sys_processor != MIPS_4KEc_R2  ) &&
	  ( sys_processor != MIPS_4KEmp    ) &&
	  ( sys_processor != MIPS_4KEmp_R2 ) &&
	  ( sys_processor != MIPS_4KSc     ) &&
	  ( sys_processor != MIPS_4KSd     ) &&
	  ( sys_processor != MIPS_M4K      );

	/* ErrCtl */
	errctl =
          ( sys_processor != MIPS_20Kc ) &&
          ( sys_processor != MIPS_25Kf ) &&
          ( sys_processor != MIPS_M4K  );

	/* Release 2 */
        if( sys_arch_rev >= K_ConfigAR_Rel2 )
	{
            cp0_reg[SYSCON_PAGEGRAIN].valid = ( sys_processor != MIPS_24K &&
						sys_processor != MIPS_24KE &&
						sys_processor != MIPS_34K &&
						sys_processor != MIPS_1004K &&
						sys_processor != MIPS_74K) ?
		TRUE : FALSE;
            cp0_reg[SYSCON_HWRENA   ].valid = TRUE;
            cp0_reg[SYSCON_EBASE    ].valid = TRUE;
            cp0_reg[SYSCON_INTCTL   ].valid = TRUE;
            cp0_reg[SYSCON_SRSCTL   ].valid = TRUE;
	    /* Is this also dependent on config3.vint? */
	    if (sys_cp0_read32(R_C0_SRSCtl, R_C0_SelSRSCtl) & M_SRSCtlHSS)
		cp0_reg[SYSCON_SRSMAP   ].valid = TRUE;
        }

	/* EJTAG */
	cp0_reg[SYSCON_DEBUG    ].valid = sys_ejtag;
	cp0_reg[SYSCON_DEPC     ].valid = sys_ejtag;
	cp0_reg[SYSCON_DESAVE   ].valid = sys_ejtag;

	/* Performance counters */
        if( config1 & M_Config1PC )
	{
	    cp0_reg[SYSCON_PERFCOUNT       ].valid = TRUE;
	    cp0_reg[SYSCON_PERFCOUNT_COUNT0].valid = TRUE;
	    
	    ctrl = sys_cp0_read32( R_C0_PerfCnt, R_C0_SelPerfCnt );
	    
	    if( ctrl & M_PerfCntM )
	    {
	        cp0_reg[SYSCON_PERFCOUNT_CTRL1 ].valid = TRUE;
		cp0_reg[SYSCON_PERFCOUNT_COUNT1].valid = TRUE;
		
	        ctrl = sys_cp0_read32( R_C0_PerfCnt, R_C0_SelPerfCnt+1 );

	        if( ctrl & M_PerfCntM )
		{
		    cp0_reg[SYSCON_PERFCOUNT_CTRL2 ].valid = TRUE;
		    cp0_reg[SYSCON_PERFCOUNT_COUNT2].valid = TRUE;
		}
	    }
        }

	break;
        
      default :

	cache_configurable = FALSE;
	mmu_configurable   = FALSE;
	tlb		   = TRUE;
	lladdr		   = TRUE;
	watch		   = TRUE;
	cacheerr	   = TRUE;
	errctl		   = TRUE;
	break;
    }

    /* Following registers are always required */
    cp0_reg[SYSCON_BADVADDR].valid = TRUE;
    cp0_reg[SYSCON_COUNT   ].valid = TRUE;
    cp0_reg[SYSCON_COMPARE ].valid = TRUE;
    cp0_reg[SYSCON_STATUS  ].valid = TRUE;
    cp0_reg[SYSCON_CAUSE   ].valid = TRUE;
    cp0_reg[SYSCON_EPC     ].valid = TRUE;
    cp0_reg[SYSCON_PRID    ].valid = TRUE;
    cp0_reg[SYSCON_CONFIG  ].valid = TRUE;
    cp0_reg[SYSCON_ERROREPC].valid = TRUE;

    /* Following registers are required if TLB is present */
    cp0_reg[SYSCON_INDEX   ].valid = tlb;
    cp0_reg[SYSCON_RANDOM  ].valid = tlb;
    cp0_reg[SYSCON_ENTRYLO0].valid = tlb;
    cp0_reg[SYSCON_ENTRYLO1].valid = tlb;
    cp0_reg[SYSCON_CONTEXT ].valid = tlb;
    cp0_reg[SYSCON_PAGEMASK].valid = tlb;
    cp0_reg[SYSCON_WIRED   ].valid = tlb;
    cp0_reg[SYSCON_ENTRYHI ].valid = tlb;
    cp0_reg[SYSCON_XCONTEXT].valid = tlb && sys_64bit;

    cp0_reg[SYSCON_TAGLO   ].valid = taghilo;
    cp0_reg[SYSCON_TAGHI   ].valid = taghilo;
    cp0_reg[SYSCON_DATALO  ].valid = datahilo;
    cp0_reg[SYSCON_DATAHI  ].valid = datahilo;

    cp0_reg[SYSCON_LLADDR  ].valid = lladdr;

    if( watch )
    {
        if ( sys_processor == MIPS_24K ||
             sys_processor == MIPS_24KE||
	     sys_processor == MIPS_34K ||
	     sys_processor == MIPS_1004K ||
	     sys_processor == MIPS_74K )
	{
	  cp0_reg[SYSCON_WATCHLO0].valid = TRUE;
	  cp0_reg[SYSCON_WATCHHI0].valid = TRUE;
	  if (sys_cp0_read32(R_C0_WatchHi, 0) & M_WatchHiM) {
	    cp0_reg[SYSCON_WATCHLO1].valid = TRUE;
            cp0_reg[SYSCON_WATCHHI1].valid = TRUE;
	    if (sys_cp0_read32(R_C0_WatchHi, 1) & M_WatchHiM) {
	      cp0_reg[SYSCON_WATCHLO2].valid = TRUE;
	      cp0_reg[SYSCON_WATCHHI2].valid = TRUE;
	      if (sys_cp0_read32(R_C0_WatchHi, 2) & M_WatchHiM) {
		cp0_reg[SYSCON_WATCHLO3].valid = TRUE;
		cp0_reg[SYSCON_WATCHHI3].valid = TRUE;
	      }
	    }
	  }
	}
	else
	{
            cp0_reg[SYSCON_WATCHLO].valid = TRUE;
            cp0_reg[SYSCON_WATCHHI].valid = TRUE;
	}
    }

    cp0_reg[SYSCON_CACHEERR].valid = cacheerr;

    cp0_reg[SYSCON_ERRCTL].valid   = errctl;

    /* MT registers */
    if (sys_mt) {
	cp0_reg[SYSCON_MVPCONTROL].valid = TRUE;
	cp0_reg[SYSCON_MVPCONF0].valid = TRUE;
	ctrl = sys_cp0_read32( R_C0_MVPConf0, R_C0_SelMVPConf0 );
	if (ctrl & M_MVPConf0M)
	    cp0_reg[SYSCON_MVPCONF1].valid = TRUE;
	cp0_reg[SYSCON_VPECONTROL].valid = TRUE;
	cp0_reg[SYSCON_VPECONF0].valid = TRUE;
	ctrl = sys_cp0_read32( R_C0_VPEConf0, R_C0_SelVPEConf0 );
	if (ctrl & M_VPEConf0M)
	    cp0_reg[SYSCON_VPECONF1].valid = TRUE;
	cp0_reg[SYSCON_YQMASK].valid = TRUE;
	cp0_reg[SYSCON_VPESCHEDULE].valid = TRUE;	/* This is really optional... */
	cp0_reg[SYSCON_VPESCHEFBACK].valid = TRUE;	/* This is really optional... */
	cp0_reg[SYSCON_VPEOPT].valid = TRUE;		/* This is really optional... */
	cp0_reg[SYSCON_TCSTATUS].valid = TRUE;
	cp0_reg[SYSCON_TCBIND].valid = TRUE;
	cp0_reg[SYSCON_TCRESTART].valid = TRUE;
	cp0_reg[SYSCON_TCHALT].valid = TRUE;
	cp0_reg[SYSCON_TCCONTEXT].valid = TRUE;
	cp0_reg[SYSCON_TCSCHEDULE].valid = TRUE;	/* This is really optional... */
	cp0_reg[SYSCON_TCSCHEFBACK].valid = TRUE;	/* This is really optional... */

	cp0_reg[SYSCON_SRSCONF0].valid = TRUE;
	srsconf = sys_cp0_read32( R_C0_SRSConf0, R_C0_SelSRSConf0 );
	cp0_reg[SYSCON_SRSCONF1].valid = (srsconf & M_SRSConf0M) ? TRUE : FALSE;
	if (cp0_reg[SYSCON_SRSCONF1].valid) {
	    srsconf = sys_cp0_read32( R_C0_SRSConf1, R_C0_SelSRSConf1 );
	    cp0_reg[SYSCON_SRSCONF2].valid = (srsconf & M_SRSConf1M) ? TRUE : FALSE;
	}
	if (cp0_reg[SYSCON_SRSCONF2].valid) {
	    srsconf = sys_cp0_read32( R_C0_SRSConf2, R_C0_SelSRSConf2 );
	    cp0_reg[SYSCON_SRSCONF3].valid = (srsconf & M_SRSConf2M) ? TRUE : FALSE;
	}
	if (cp0_reg[SYSCON_SRSCONF3].valid) {
	    srsconf = sys_cp0_read32( R_C0_SRSConf3, R_C0_SelSRSConf3 );
	    cp0_reg[SYSCON_SRSCONF4].valid = (srsconf & M_SRSConf3M) ? TRUE : FALSE;
	}
    }

    /* Following registers are 64 bit on a 64 bit CPU */
    if (sys_64bit)
    {
	cp0_reg[SYSCON_ENTRYLO0  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_ENTRYLO1  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_CONTEXT   ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_BADVADDR  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_ENTRYHI   ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_EPC       ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_LLADDR    ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHLO   ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHHI   ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_XCONTEXT  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_DEPC      ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_ERROREPC  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_DESAVE    ].regsize = sizeof(UINT64);
        cp0_reg[SYSCON_L23TAGLO  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_L23TAGHI  ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_L23DATALO ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_L23DATAHI ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHLO0 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHHI0 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHLO1 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHHI1 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHLO2 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHHI2 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHLO3 ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_WATCHHI3 ].regsize = sizeof(UINT64);

	cp0_reg[SYSCON_TCRESTART ].regsize = sizeof(UINT64);
	cp0_reg[SYSCON_TCCONTEXT ].regsize = sizeof(UINT64);
    }


    /**** Register objects ****/

    syscon_register_generic( SYSCON_CPU_CYCLE_PER_COUNT_ID,
			    syscon_uint32_read, (void *)&cycle_per_count,
			    NULL,		NULL );

    syscon_register_generic( SYSCON_CPU_TLB_COUNT_RESET_ID,
			     syscon_uint8_read, (void *)&tlb_reset,
			     NULL,		NULL );

    syscon_register_generic( SYSCON_CPU_TLB_AVAIL_ID,
			     syscon_bool_read,  (void *)&tlb,
			     syscon_bool_write, (void *)&tlb );

    syscon_register_generic( SYSCON_CPU_CACHE_CONFIGURABLE_ID,
			     syscon_bool_read,  (void *)&cache_configurable,
			     NULL,		NULL );

    syscon_register_generic( SYSCON_CPU_MMU_CONFIGURABLE_ID,
			     syscon_bool_read,  (void *)&mmu_configurable,
			     NULL,		NULL );

    syscon_register_generic( SYSCON_CPU_EICMODE_ID,
			     syscon_bool_read,	(void *)&sys_eicmode,
			     NULL,		NULL );
    
    /* MIPS32/64 specifics */
    syscon_register_id_mips32( SYSCON_CPU_CP0_CONFIG1_RESET_ID,
			       /* MIPS32/64 */
			       cpu_cp0_config1_reset_mips32_read, NULL,
			       NULL,			          NULL,
			       /* Other */
			       NULL,			          NULL,
			       NULL,				  NULL );

    syscon_register_id_mips32( SYSCON_CPU_TLB_COUNT_ID,
			       /* MIPS32/64 */
			       cpu_tlb_count_mips32_read, NULL,
			       NULL,		          NULL,
			       /* Other (assume QED RM5261) */
			       cpu_tlb_count_rm5261_read, NULL,
			       NULL,		          NULL );

    syscon_register_generic( SYSCON_CPU_REGISTER_SETS_ID,
			     syscon_uint8_read, (void *)&register_sets,
			     NULL,		NULL );

    syscon_register_generic( SYSCON_CPU_REGINFO_ID,
			     reginfo_read, NULL,
			     NULL,		NULL );

    /* Determine initial TLB entry count */
    if( tlb )
    {
        SYSCON_read( SYSCON_CPU_TLB_COUNT_ID,
		     (void *)&tlb_reset,
		     sizeof(UINT8) );
    }
    else
    {
        tlb_reset = 0;
    }

    /* And register sets */
    if( sys_arch_rev >= K_ConfigAR_Rel2 ) {
	UINT32 srsctl;
        SYSCON_read( SYSCON_CPU_CP0_SRSCTL_ID,
		     (void *)&srsctl,
		     sizeof(UINT32) );
	register_sets = ((srsctl & M_SRSCtlHSS) >> S_SRSCtlHSS) + 1;
    }
    else 
	register_sets = 1;

    /* Setup clock cycles per COUNT register increment */
    switch( sys_processor )
    {
      case MIPS_4Kc      :
      case MIPS_4Kmp     :
      case MIPS_4KEc     :
      case MIPS_4KEc_R2  :
      case MIPS_4KEmp    :
      case MIPS_4KEmp_R2 :
      case MIPS_4KSc     :
      case MIPS_4KSd     :
        cycle_per_count = MIPS4K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_5K    :
      case MIPS_5KE   :
        cycle_per_count = MIPS5K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_20Kc  :
      case MIPS_25Kf  :
        cycle_per_count = MIPS20Kc_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_24K   :
      case MIPS_24KE  :
        cycle_per_count = MIPS24K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_34K   :
        cycle_per_count = MIPS34K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_74K   :
        cycle_per_count = MIPS74K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_1004K :
        cycle_per_count = MIPS1004K_COUNT_CLK_PER_CYCLE;
	break;
      case MIPS_M4K :
        cycle_per_count = MIPSM4K_COUNT_CLK_PER_CYCLE;
	break;
      case QED_RM70XX : /* Assume QED RM7061A */
        cycle_per_count = QED_RM7061A_COUNT_CLK_PER_CYCLE;
	break;
      case QED_RM52XX : /* Assume QED RM5261 */
        cycle_per_count = QED_RM5261_COUNT_CLK_PER_CYCLE;
	break;
      default : 
	  cycle_per_count = 2;
    }
}


/************************************************************************
 *
 *                          syscon_register_id_mips32
 *  Description :
 *  -------------
 *
 *  Function used to register SYSCON object functions for objects 
 *  that depend on whether CPU is MIPS32/64 or not.
 *
 *  A read and/or write function may be registered.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.

 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_id_mips32(
    t_syscon_ids   id,			 /* OBJECT ID from syscon_api.h */
    /* MIPS32/MIPS64 processor */
    t_syscon_func  read_mips32,		 /* MIPS32/64 read function	*/
    void	   *read_data_mips32,	 /* Registered data		*/
    t_syscon_func  write_mips32,	 /* MIPS32/64 write function	*/
    void	   *write_data_mips32,   /* Registered data		*/
    /* Other processor */
    t_syscon_func  read_other,		 /* "Other CPU" read function	*/
    void	   *read_data_other,	 /* Registered data		*/
    t_syscon_func  write_other,	         /* "Other CPU" write function	*/
    void	   *write_data_other )   /* Registered data		*/
{
    t_syscon_obj *obj;

    obj = &syscon_objects[id];

    if( sys_mips32_64 )
    {
        /* MIPS32/64 CPU */
        obj->read       = read_mips32;
	obj->read_data  = read_data_mips32;
	obj->write      = write_mips32;
	obj->write_data = write_data_mips32;
    }
    else
    {
        /* Not MIPS32/64 CPU */
        obj->read       = read_other;
	obj->read_data  = read_data_other;
	obj->write      = write_other;
	obj->write_data = write_data_other;
    }
}
