/************************************************************************
 *
 *  init_core.c
 *
 *  Core card specific startup code (c-code)
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
#include <excep_api.h>
#include <stdio.h>
#include <product.h>
#include <malta.h>
#include <gt64120.h>
#include <core_bonito64.h>
#include <bonito64.h>
#include <core_sys.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Bonito64 interrupt mask */
#define INT_MASK_BONITO (BONITO_ICU_SYSTEMERR |\
			 BONITO_ICU_DRAMPERR  |\
			 BONITO_ICU_RETRYERR )

/************************************************************************
 *  Public variables
 ************************************************************************/

UINT32 _bonito = KSEG1(BONITO_REG_BASE);
UINT32 malta_pci_io_base;

/************************************************************************
 *  Static variables
 ************************************************************************/

static t_EXCEP_ref isr_ref;
static bool        cpu_isr_parm;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
gt64120_isr( 
    void *data );

static void
bonito64_isr( 
    void *data );

static void
msc01_isr( 
    void *data );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_core_init
 *  Description :
 *  -------------
 *
 *  Core card specific initialisation code
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_core_init( 
    bool   early,	/* TRUE -> Before initmodules(), FALSE -> after */
    UINT32 intline,	/* Interrupt line for North Bridge		*/
    bool   cpu_isr )	/* TRUE  -> Interrupt line connected to CPU.
			 * FALSE -> Interrupt line connected to
			 *          interrupt controller.
			 */
{
    UINT32 clock_period_ns;

    if( early )
    {
        /* Setup North Bridge mapping global variable */
        /* Setup Base address for PCI IO access */

        switch( sys_corecard )
        {
          case MIPS_REVISION_CORID_QED_RM5261 :
          case MIPS_REVISION_CORID_CORE_LV :
          case MIPS_REVISION_CORID_CORE_FPGA :
          case MIPS_REVISION_CORID_CORE_FPGAr2 :
	    sys_nb_base = KSEG1(MALTA_CORECTRL_BASE);
	    malta_pci_io_base = MALTA_PCI_IO_BASE;
	    break;

          case MIPS_REVISION_CORID_BONITO64 :
	  case MIPS_REVISION_CORID_CORE_20K :
	  case MIPS_REVISION_CORID_CORE_EMUL_20K :
	    sys_nb_base = BONITO_REG_BASE;
	    malta_pci_io_base = BONITO_PCIIO_BASE;
	    break;

	  case MIPS_REVISION_CORID_CORE_SYS :
	  case MIPS_REVISION_CORID_CORE_FPGA2 :
	  case MIPS_REVISION_CORID_CORE_EMUL_SYS :
	  case MIPS_REVISION_CORID_CORE_FPGA3 :
	  case MIPS_REVISION_CORID_CORE_24K :
	    /* Setup North Bridge mapping global variable */
	    sys_nb_base = MSC01_REGADDR_BASE;
	    malta_pci_io_base = CORE_SYS_PCIIO_BASE;
	    break;

	  /* Add new core cards here */

          default : /* Should never happen */
	    break;
        }
    }
    else
    {
	/* Store cpu parameter */
	cpu_isr_parm = cpu_isr;

        switch( sys_corecard )
        {
          case MIPS_REVISION_CORID_QED_RM5261 :
          case MIPS_REVISION_CORID_CORE_LV :
          case MIPS_REVISION_CORID_CORE_FPGA :
          case MIPS_REVISION_CORID_CORE_FPGAr2 :

            /* Register ISR */
	    if( cpu_isr_parm )
	    {
	        EXCEP_register_cpu_isr( 
	            intline,
		    gt64120_isr,
		    NULL,
		    &isr_ref );
            }
	    else
	    {
	        EXCEP_register_ic_isr(
		    intline,
		    gt64120_isr,
		    NULL,
		    &isr_ref );
	    }

            /* Enable GT64120 interrupts */
            GT_W32( sys_nb_base, 
		    GT_CPUINT_MASK_OFS, 
		        GT_CPUINT_MASK_MEMOUT_BIT    |
			GT_CPUINT_MASK_DMAOUT_BIT    |
			GT_CPUINT_MASK_CPUOUT_BIT    |
			GT_CPUINT_MASK_MASRDERR0_BIT |
			GT_CPUINT_MASK_SLVWRERR0_BIT |
			GT_CPUINT_MASK_MASWRERR0_BIT |	
			GT_CPUINT_MASK_SLVRDERR0_BIT |
			GT_CPUINT_MASK_ADDRERR0_BIT );

	    break;

          case MIPS_REVISION_CORID_BONITO64 :
	  case MIPS_REVISION_CORID_CORE_20K :
	  case MIPS_REVISION_CORID_CORE_EMUL_20K :

	    /*  Inform Bonito64 on the CPU clock period (used for example 
	     *  for SDRAM refresh).
	     *
	     *  The result from clock period calculation is rounded 
	     *  up (i.e. frequency rounded down) in order to avoid 
	     *  too long a refresh interval.
	     */
	     clock_period_ns = ((UINT32)1e9 + sys_busfreq_hz - 1) / sys_busfreq_hz;

	     /* Bonito64 has a limited number of bits for the clock period */
	     clock_period_ns = 
	         MIN( BONITO_IODEVCFG_CPUCLOCKPERIOD >> 
		          BONITO_IODEVCFG_CPUCLOCKPERIOD_SHIFT, 
		      clock_period_ns);

	     sys_func_noram(
		 (t_sys_func_noram)bonito64_write_iodevcfg,
		   (BONITO_IODEVCFG & ~BONITO_IODEVCFG_CPUCLOCKPERIOD) |
		   (clock_period_ns << BONITO_IODEVCFG_CPUCLOCKPERIOD_SHIFT),
		 0, 0 );

	    /* Register ISR */
	    if( cpu_isr_parm )
	    {
	        EXCEP_register_cpu_isr( 
	            intline,
	            bonito64_isr,
		    NULL,
		    &isr_ref );
            }
	    else
	    {
	        EXCEP_register_ic_isr( 
	            intline,
	            bonito64_isr,
		    NULL,
		    &isr_ref );
	    }

	    /* Select Bonito64 Int0/Int1 */
	    BONITO_INTSTEER = CORE_BONITO64_INT_HI ? 0xFFFFFFFF : 0;

	    /* Set polarity of internal sources to active-high */
	    BONITO_INTPOL   = MSK(15);
	    
	    /* Set level/edge of internal sources (9:4 level, others edge) */
	    BONITO_INTEDGE  = (MSK(4) << 0)  |
			      (MSK(5) << 10);

	    /* Clear all interrupts to reset edge-triggered interrupts */
	    BONITO_INTENCLR = 0xFFFFFFFF;

	    /* Enable the Bonito64 interrupts we are interested in */
	    BONITO_INTENSET = INT_MASK_BONITO;

	    break;

	  case MIPS_REVISION_CORID_CORE_SYS :
	  case MIPS_REVISION_CORID_CORE_FPGA2 :
	  case MIPS_REVISION_CORID_CORE_EMUL_SYS :
	  case MIPS_REVISION_CORID_CORE_FPGA3 :
	  case MIPS_REVISION_CORID_CORE_24K :

	    /* Register ISR */
	    if( cpu_isr_parm )
	    {
	        EXCEP_register_cpu_isr( 
	            intline,
	            msc01_isr,
		    NULL,
		    &isr_ref );
            }
	    else
	    {
	        EXCEP_register_ic_isr( 
	            intline,
	            msc01_isr,
		    NULL,
		    &isr_ref );
	    }

	    /* Set up interrupt controller */
	    /* enable interrupt line 1 (pci p-err and pci s-err) */
	    {
		if (!sys_eicmode) {
#define MSC01_IC_PCIINTNO 1
#define IC_REG(ofs) (*(volatile UINT32*)&msc01_ic_regbase[ofs])
		    UINT8 *msc01_ic_regbase = (UINT8 *)MSC01_IC_REG_BASE;

		    IC_REG(MSC01_IC_SUP_OFS + (MSC01_IC_PCIINTNO*MSC01_IC_SUP_STEP)) =
			(0<<MSC01_IC_SUP_EDGE_SHF) | (0<<MSC01_IC_SUP_PRI_SHF);

		    IC_REG(MSC01_IC_DISL_OFS) = 0xffffffff;
		    IC_REG(MSC01_IC_DISH_OFS) = 0xffffffff;
		    IC_REG(MSC01_IC_ENAL_OFS) = 1 << MSC01_IC_PCIINTNO;

		    IC_REG(MSC01_IC_GENA_OFS) = MSC01_IC_GENA_GENA_BIT;
#undef IC_REG
		}

		/* Enable interrupt source */
		REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTCFG) =
		    MSC01_PCI_INTCFG_MWP_BIT |
		    MSC01_PCI_INTCFG_MRP_BIT |
		    MSC01_PCI_INTCFG_SWP_BIT |
		    MSC01_PCI_INTCFG_SRP_BIT |
		    MSC01_PCI_INTCFG_SE_BIT ;
	    }

	    break;

	  /* Add new core cards here */

          default : /* Should never happen */
	    break;
        }
    }
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

static void
gt64120_isr( 
    void *data )
{
    UINT32 cause, mask;

    /* Determine cause */
    GT_L32( sys_nb_base, GT_INTRCAUSE_OFS, cause );
    GT_L32( sys_nb_base, GT_CPUINT_MASK_OFS, mask );
    cause &= mask;

    if( cause )
    {
        /* clear interrupt */
        GT_W32( sys_nb_base, GT_INTRCAUSE_OFS, 0 );
	
        printf( "\nGalileo 64120 Interrupt. Cause = 0x%08x\n", cause );

	/* Common interrupt messages are decoded here. */
	if (cause & GT_INTRCAUSE_MEMOUT_BIT) 
	  printf("MemOut: CPU or PCI accessed an address out of range in the memory decoding or a burst access to 8-/16- bit devices.\n");
	if (cause & GT_INTRCAUSE_DMAOUT_BIT) 
	  printf("DMAOut: DMA accessed an address out of range.\n");
	if (cause & GT_INTRCAUSE_CPUOUT_BIT) 
	  printf("CPUOut: CPU accessed an address out of range.\n");

        EXCEP_run_default_esr_handler();
    }
    else
    {
        /*  On some old core cards, there were glitches on CoreHi.
	 *  If this happens, the system controller will indicate
	 *  that no interrupt has actually occurred. In this case,
	 *  we deregister gt64120_isr()
	 */
	if( cpu_isr_parm )
	    EXCEP_deregister_cpu_isr( isr_ref );
	else
	    EXCEP_deregister_ic_isr( isr_ref );	
    }
}


static void
bonito64_isr( 
    void *data )
{
    UINT32 cause, mask;

    cause =  BONITO_INTISR;
    mask  =  INT_MASK_BONITO;
    cause &= mask;

    /* clear interrupt */
    BONITO_INTENCLR = cause;
    BONITO_INTENSET = cause;

    printf( "\nBonito64 Interrupt. Cause = 0x%08x\n", cause );
    EXCEP_run_default_esr_handler();
}



static void
msc01_isr( 
    void *data )
{
    UINT32 cause, pcistat;

    cause =  REG(MSC01_IC_REG_BASE, MSC01_IC_ISAL);
    pcistat  =  REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTSTAT);

    /* clear interrupt */
    REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTSTAT) = pcistat;

    printf( "\nMSC01 Interrupt. Cause = 0x%02x, PCI status = 0x%02x\n", cause, pcistat );
    EXCEP_run_default_esr_handler();
}



