
/************************************************************************
 *
 *  freq_core.c
 *
 *  Core card specifics of FREQ module
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
#include <product.h>
#include <gt64120.h>
#include <bonito64.h>
#include <core_bonito64.h>
#include <core_sys.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_core_estimate_busfreq
 *  Description :
 *  -------------
 *
 *  Estimate external bus (SysAD) clock frequency.
 *
 *  Return values :
 *  ---------------
 *
 *  Estimated frequency in Hz.
 *
 ************************************************************************/
UINT32
arch_core_estimate_busfreq( void )
{
    UINT32 cp0_countperiod;
    UINT32 endvalue;
    UINT32 cycle_per_count;
    UINT32 data = 0;

    /* Determine number of CPU cycles per CPU counter tick */
    SYSCON_read(
        SYSCON_CPU_CYCLE_PER_COUNT_ID,
        &cycle_per_count,
	sizeof(UINT32) );

    switch( sys_corecard )
    {
      case MIPS_REVISION_CORID_CORE_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA2 :
      case MIPS_REVISION_CORID_CORE_EMUL_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA3 :
      case MIPS_REVISION_CORID_CORE_24K :
	#define STARTVALUE 0xfffff

        /*  Use timer in System Controller in conjunction with cp0
	 *  timer to estimate the external bus frequency.
	 */

	/*  Timer will generate interrupt when it reaches it's compare
	 *  value. Setup timer in the System Controller.
	 */	
	REG(MSC01_IC_REG_BASE, MSC01_IC_TRLD) =  STARTVALUE-1;

        /* Configure System Controller timer to level */
	REG(MSC01_IC_REG_BASE, MSC01_IC_TCFG) =  0;
	/* Start timer and clear INT */
	REG(MSC01_IC_REG_BASE, MSC01_IC_TCFG) =  MSC01_IC_TCFG_ENA_MSK;

        /* Start CPU counter */
	SYSCON_write( SYSCON_CPU_CP0_COUNT_ID,
		      (void *)(&data),
		      sizeof(UINT32) );
	
	/* Wait for timer to expire */
	while( !(REG(MSC01_IC_REG_BASE, MSC01_IC_TCFG) & MSC01_IC_TCFG_INT_MSK) )
	{
	    ;
	}

	/* Read CPU counter */
        SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
                     (void *)(&data),
		     sizeof(UINT32) );

	REG(MSC01_IC_REG_BASE, MSC01_IC_TCFG) =  0;

	return (STARTVALUE * 1000)/(data * cycle_per_count) *
               (sys_cpufreq_hz/1000);
	#undef STARTVALUE

      case MIPS_REVISION_CORID_BONITO64 :
      case MIPS_REVISION_CORID_CORE_20K :
      case MIPS_REVISION_CORID_CORE_EMUL_20K :

        /*  Use timer in Bonito64 in conjunction with cp0 timer to
         *  estimate the external bus frequency.
	 */

	/*  Timer will generate interrupt when it reaches it's compare
	 *  value. Setup interrupt controller in Bonito64.
	 */	
	BONITO_INTPOL   |= (1 << 14);
	BONITO_INTEDGE  |= (1 << 14);
	BONITO_INTENCLR  = (1 << 14);

        /* Start Bonito64 timer */
	BONITO_TIMERCFG = 0x01fffff;
	BONITO_TIMERCFG = 0x09fffff;

        /* Start CPU counter */
	SYSCON_write( SYSCON_CPU_CP0_COUNT_ID,
		      (void *)(&data),
		      sizeof(UINT32) );
	
	/* Wait on Bonito64 interrupt */
	while( !(BONITO_INTISR & 0x4000) )
	{
	    ;
	}

	/* Read CPU counter */
        SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
                     (void *)(&data),
		     sizeof(UINT32) );

	return (0xfffff * 2 * 1000)/(data * cycle_per_count) *
               (sys_cpufreq_hz/1000);

      case MIPS_REVISION_CORID_QED_RM5261 :
      case MIPS_REVISION_CORID_CORE_LV :
      case MIPS_REVISION_CORID_CORE_FPGA :
      case MIPS_REVISION_CORID_CORE_FPGAr2 :

        /*  Use timer in GT64120 in conjunction with cp0 timer to
         *  estimate the external bus frequency.
         */

        #define STARTVALUE	((UINT32) 0x800000)
        #define COUNTPERIOD	((UINT32) 20)		/* ms */

        cp0_countperiod = 
            (sys_cpufreq_hz / 1000) * COUNTPERIOD / cycle_per_count;

        /* Start GT64120 counter */
        GT_W32( sys_nb_base, GT_TCCTRL_OFS,   0 );
	GT_W32( sys_nb_base, GT_TC0VALUE_OFS, STARTVALUE );
	GT_W32( sys_nb_base, GT_TCCTRL_OFS,   GT_TCCTRL_ENTC0_BIT);

        /* Wait on cp0 counter */
	SYSCON_write( SYSCON_CPU_CP0_COUNT_ID,
		      (void *)(&data),
		      sizeof(UINT32) );
        do
        {
            SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
	                 (void *)(&data),
			 sizeof(UINT32) );
        }
        while (data < cp0_countperiod);

        /* Stop GT64120 counter and read value */
	GT_W32( sys_nb_base, GT_TCCTRL_OFS,   0 );
	GT_L32( sys_nb_base, GT_TC0VALUE_OFS, endvalue );

        return ((STARTVALUE - endvalue) / COUNTPERIOD) * ((UINT32)1000);

      /* Add new core cards here ! */

      default : /* Should never happen */

	return sys_cpufreq_hz;
   }
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

