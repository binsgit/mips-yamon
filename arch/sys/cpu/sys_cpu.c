
/************************************************************************
 *
 *  sys_cpu.c
 *
 *  CPU specific configuration
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
#include <sys_api.h>
#include <mips.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char *name_mips      = MIPS_NAME;

static char *name_4Kc	    = "MIPS 4Kc";
static char *name_4Kmp	    = "MIPS 4Km/4Kp";
static char *name_4KEc	    = "MIPS 4KEc";
static char *name_4KEc_R2   = "MIPS 4KEc";      /* No new name for the R2 version */
static char *name_4KEmp	    = "MIPS 4KEm/4KEp";
static char *name_4KEmp_R2  = "MIPS 4KEm/4KEp"; /* No new name for the R2 version */
static char *name_4KSc	    = "MIPS 4KSc";
static char *name_4KSd	    = "MIPS 4KSd";
static char *name_5Kc	    = "MIPS 5Kc";
static char *name_5Kf	    = "MIPS 5Kf";
static char *name_5KEc	    = "MIPS 5KEc";
static char *name_5KEf	    = "MIPS 5KEf";
static char *name_20Kc      = "MIPS 20Kc";
static char *name_25Kf      = "MIPS 25Kf";
static char *name_24Kc      = "MIPS 24Kc";
static char *name_24Kf      = "MIPS 24Kf";
static char *name_24KEc     = "MIPS 24KEc";
static char *name_24KEf     = "MIPS 24KEf";
static char *name_34Kc      = "MIPS 34Kc";
static char *name_34Kf      = "MIPS 34Kf";
static char *name_74Kc      = "MIPS 74Kc";
static char *name_74Kf      = "MIPS 74Kf";
static char *name_m4k	    = "MIPS M4K";
static char *name_qed52xx   = "QED RM5261";  /* Assume 5261  */
static char *name_qed70xx   = "QED RM7061A"; /* Assume 7061A */

#define BPW_SETTINGS	8
static UINT32 ibpw[BPW_SETTINGS];
static UINT32 dbpw[BPW_SETTINGS];

#define ASSOC_SETTINGS	8
static UINT32 iassoc[ASSOC_SETTINGS];
static UINT32 dassoc[ASSOC_SETTINGS];


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          sys_decode_procid
 *  Description :
 *  -------------
 *
 *  Map processor ID field to string hodling the name of the CPU
 *  
 *  Return values :
 *  ---------------
 *
 *  String holding name of CPU
 *
 ************************************************************************/
char *
sys_decode_procid( void )
{
    UINT32 fir;

    switch( sys_processor )
    {
      case MIPS_4Kc :
        return name_4Kc;
      case MIPS_4Kmp :
        return name_4Kmp;
      case MIPS_4KEc :
        return name_4KEc;
      case MIPS_4KEc_R2 :
        return name_4KEc_R2;
      case MIPS_4KEmp :
        return name_4KEmp;
      case MIPS_4KEmp_R2 :
        return name_4KEmp_R2;
      case MIPS_4KSc :
        return name_4KSc;
      case MIPS_4KSd :
        return name_4KSd;
      case MIPS_5K :
	/*  A 5Kf may be distinguished from a 5Kc with an FPU
	 *  added by another party.
	 *  This is done using the FPU implementation register (FIR).
	 *  The FIR Processor ID field of a 5Kf will have the same
	 *  value as the Processor ID field of CP0 PRId register.
	 */
	if( ((sys_fpu & M_FIRImp) >> S_FIRImp) == K_PRIdImp_Opal )
	
	{
	    return name_5Kf;
	}
	return name_5Kc;
      case MIPS_5KE :
	/*  A 5KEf may be distinguished from a 5KEc with an FPU
	 *  added by another party.
	 *  This is done using the FPU implementation register (FIR).
	 *  The FIR Processor ID field of a 5KEf will have the same
	 *  value as the Processor ID field of CP0 PRId register.
	 */
	if( ((sys_fpu & M_FIRImp) >> S_FIRImp) == K_PRIdImp_5KE )
	{
	    return name_5KEf;
	}
	return name_5KEc;
      case MIPS_20Kc :
        return name_20Kc;
      case MIPS_25Kf :
        return name_25Kf;
      case MIPS_24K :
        return sys_fpu ? name_24Kf : name_24Kc;
      case MIPS_24KE :
        return sys_fpu ? name_24KEf : name_24KEc;
      case MIPS_34K :
        return sys_fpu ? name_34Kf : name_34Kc;
      case MIPS_74K :
        return sys_fpu ? name_74Kf : name_74Kc;
      case MIPS_M4K :
        return name_m4k;
      case QED_RM52XX :
        return name_qed52xx;
      case QED_RM70XX :
        return name_qed70xx;
      default :
        return NULL;
    }
}


/************************************************************************
 *
 *                          sys_cpu_config
 *  Description :
 *  -------------
 *
 *  Configure CPU cache/mmu settings
 *
 *  This feature is present specifically to support configuration
 *  testing of the core in a lead vehicle, and is not supported
 *  in any other environment.  Attempting to use this feature
 *  outside of the scope of a lead vehicle is a violation of the
 *  MIPS Architecture, and may cause unpredictable operation of
 *  the processor.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cpu_config( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting )
{
    UINT32 config1, config1_reset, l, s;
    bool   cache_configurable, mmu_configurable;
    
    SYSCON_read( SYSCON_CPU_CACHE_CONFIGURABLE_ID,
		 &cache_configurable,
		 sizeof(bool) );

    SYSCON_read( SYSCON_CPU_MMU_CONFIGURABLE_ID,
		 &mmu_configurable,
		 sizeof(bool) );

    if( mmu_configurable && mmu )
    {
        sys_cpu_mmu_config( setting->mmu_tlb ? 0 : 1 );

	SYSCON_write( SYSCON_CPU_TLB_AVAIL_ID,
		      (void *)&setting->mmu_tlb,
		      sizeof( bool ) );
    }

    if( cache_configurable )
    {
        SYSCON_read( SYSCON_CPU_CP0_CONFIG1_ID,
                     &config1,
	             sizeof(UINT32) );

        SYSCON_read( SYSCON_CPU_CP0_CONFIG1_RESET_ID,
                     &config1_reset,
	             sizeof(UINT32) );

        if( icache )
        {
	    if( setting->i_bpw == 0 )
	    {
	        l = 0;
	        s = (config1_reset & M_Config1IS) >> S_Config1IS;
	    }
	    else
	    {
	        l = (config1_reset & M_Config1IL) >> S_Config1IL;

	        for( s = 0;
	             CACHE_CALC_SPW(s) != setting->i_bpw / CACHE_CALC_LS(l);
	             s++ );
	    }

            sys_cpu_icache_config( s, l, setting->i_assoc - 1, config1_reset );
        }

        if( dcache )
        {
	    if( setting->d_bpw == 0 )
	    {
	        l = 0;
	        s = (config1_reset & M_Config1DS) >> S_Config1DS;
	    }
	    else
	    {
	        l = (config1_reset & M_Config1DL) >> S_Config1DL;

	        for( s = 0;
	             CACHE_CALC_SPW(s) != setting->d_bpw / CACHE_CALC_LS(l);
	             s++ );
	    }

            sys_cpu_dcache_config( s, l, setting->d_assoc - 1, config1_reset );
        }
    }
}


/************************************************************************
 *                          sys_cpu_cache_bpw
 ************************************************************************/
void
sys_cpu_cache_bpw(
    bool         icache,	/* TRUE -> icache, FALSE -> dcache	*/
    t_sys_array  *sys_array )
{
    UINT32 spw, spw_min, linesize;

    /* Calc number of sets (lines) per way and determine linesize */      
    if( icache )
    {
	sys_array->array = ibpw;
        spw      = sys_icache_lines / sys_icache_assoc;
        linesize = sys_icache_linesize;
    }
    else
    {
	sys_array->array = dbpw;
        spw      = sys_dcache_lines / sys_dcache_assoc;
        linesize = sys_dcache_linesize;
    }

    spw_min = 64;  /* Minimum 64 sets per way */

    switch( sys_processor )
    {
      case MIPS_M4K   : /* CPU has no cache */
         sys_array->count = 0;
	 break;
      case MIPS_5K       :
      case MIPS_5KE      :
         spw_min = 128;
	 /* Fall through !! */
      case MIPS_24K      :
      case MIPS_24KE     :
      case MIPS_34K      :
      case MIPS_74K      :
      case MIPS_4Kc      :
      case MIPS_4Kmp     :
      case MIPS_4KEc     :
      case MIPS_4KEc_R2  :
      case MIPS_4KEmp    :
      case MIPS_4KEmp_R2 :
      case MIPS_4KSc     :
      case MIPS_4KSd     :
      case MIPS_20Kc     :
      case MIPS_25Kf     :
	 /* Init count of valid settings */
         sys_array->count = 0;

	 /*  Sets per way may be (64),128,256,512,1024,2048,4096.
	  *  Available spw are calculated by starting with the
	  *  hardware default and stepping down.
	  *  The check on count should not be necessary.
	  *  64 sets per way is not allowed for 5Kc/5Kf.
	  */
	 while( (spw >= spw_min) && (sys_array->count < BPW_SETTINGS - 1) )
	 {
	     sys_array->array[sys_array->count] = spw * linesize;  /* Calc bytes per way */
	     sys_array->count++;
	     spw >>= 1;
         }

	 /* 0 bytes per way is also possible by setting line size to 0 */
	 sys_array->array[sys_array->count] = 0;
	 sys_array->count++;

	 break;

      default :
         /* Not possible to configure cache size */
	 sys_array->array[0]		  = spw * linesize;
         sys_array->count = 1;
	 break;
    }
}


/************************************************************************
 *                          sys_cpu_cache_assoc
 ************************************************************************/
void
sys_cpu_cache_assoc(
    bool         icache,	/* TRUE -> icache, FALSE -> dcache	*/
    t_sys_array  *sys_array )
{
    UINT32 max;
    UINT32 i;
    UINT32 index;
    
    if (icache) {
	sys_array->array = iassoc;
        max = sys_icache_assoc;
    }
    else {
	sys_array->array = dassoc;
        max = sys_dcache_assoc;
    }

    /* Determine max associativity */      

    switch( sys_processor )
    {
      case MIPS_M4K   : /* CPU has no cache */
         sys_array->count = 0;
	 break;
      case MIPS_4Kc      :
      case MIPS_4KEc     :
      case MIPS_4KEc_R2  :
      case MIPS_4KEmp    :
      case MIPS_4KEmp_R2 :
      case MIPS_4KSc     :
      case MIPS_4KSd     :
      case MIPS_4Kmp     :
      case MIPS_5K       :
      case MIPS_5KE      :
      case MIPS_20Kc     :
      case MIPS_25Kf     :
      case MIPS_24K      :
      case MIPS_24KE     :
      case MIPS_34K      :
      case MIPS_74K      :
	/*  Valid way counts are 1..max. 
	 *  The MIN macro is not really necessary.
	 */
	for( index = 0, i = MIN( ASSOC_SETTINGS, max );
	     index < MIN( ASSOC_SETTINGS, max );
	     index++, i-- )
        {
	    sys_array->array[index] = i;
	}
   
	sys_array->count = max;

	break;

      default :
        /* Not possible to configure cache size */
	sys_array->array[0] = max;
        sys_array->count = 1;
	break;
    }
}


/************************************************************************
 *
 *                          sys_decode_compid
 *  Description :
 *  -------------
 *
 *  Map Company ID field of CPO PrId register to string with company name.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
char *
sys_decode_compid( 
    UINT8 compid )	/* Company ID field of CP0 PRId register	*/
{
    switch( compid )
    {
      case K_PRIdCoID_MIPS :
        return name_mips;
      default :
        return NULL;
    }
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


