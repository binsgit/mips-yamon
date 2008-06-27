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
#include <socitsc.h>
#include <gcmp.h>
#include <gic.h>

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
bool gic_present;
bool gcmp_present;
bool iocu_present;
bool io_coherent;

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

static void
socitsc_isr( 
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
    void *icu_regbase;
    void *pci_regbase;
    void (*isrfunc)(void *);

    if (early) {
        /* Setup North Bridge mapping global variable */
        /* Setup Base address for PCI IO access */

        switch (sys_sysconid) {
	case MIPS_REVISION_SCON_GT64120:
	    sys_nb_base = KSEG1(MALTA_GT64120_BASE);
	    malta_pci_io_base = GT64120_PCIIO_BASE;
	    break;

	case MIPS_REVISION_SCON_BONITO:
	    sys_nb_base = BONITO_REG_BASE;
	    malta_pci_io_base = BONITO_PCIIO_BASE;
	    break;

	case MIPS_REVISION_SCON_ROCIT2:
	    /*
	     * MIPSCMP
	     * Check for CMP
	     */
	    gcmp_present = (GCMPGCB(GCMPBASE,GCMPB) & GCMP_GCB_GCMPB_GCMPBASE_MSK) == GCMPBASE;
	    /* check for IOCU */
	    iocu_present = gcmp_present && (GCMPGCB(GCMPBASE,GC) & GCMP_GCB_GC_NUMIOCU_MSK);
	    io_coherent = iocu_present;
	    if (io_coherent) {
	      /* debug switch allows the IOCU to be effectively disabled */
	      if ((*(unsigned int *)0xbf403000 & 0x80) == 0)
		io_coherent = 0;
	    }

	case MIPS_REVISION_SCON_SOCIT:
	case MIPS_REVISION_SCON_ROCIT:
	    sys_nb_base = MSC01_REGADDR_BASE;
	    malta_pci_io_base = MSC01_PCIIO_BASE;
	    gic_present = gic_probe();
	    
	    break;

	case MIPS_REVISION_SCON_SOCITSC:
	    sys_nb_base = SOCITSC_SMX_REG_BASE;
	    malta_pci_io_base = SOCITSC_PCIIO_BASE;
	    break;

	  /* Add controller types here */

          default : /* Should never happen */
	    break;
        }
    }
    else {
	/* Store cpu parameter */
	cpu_isr_parm = cpu_isr;

        switch (sys_sysconid) {
	case MIPS_REVISION_SCON_GT64120:
            /* Register ISR */
	    if( cpu_isr_parm ) {
	        EXCEP_register_cpu_isr( 
	            intline,
		    gt64120_isr,
		    NULL,
		    &isr_ref);
            }
	    else {
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

	case MIPS_REVISION_SCON_BONITO:

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

	case MIPS_REVISION_SCON_SOCIT:
	case MIPS_REVISION_SCON_ROCIT:
	case MIPS_REVISION_SCON_ROCIT2:	/* GICFIXME: need GIC handler */
	    isrfunc = msc01_isr;
	    pci_regbase = (void *)MSC01_PCI_REG_BASE;
	    icu_regbase = (void *)MSC01_IC_REG_BASE;
	    goto socit_setup;
		
	case MIPS_REVISION_SCON_SOCITSC:
	    isrfunc = socitsc_isr;
	    pci_regbase = (void *)KSEG1(SOCITSC_MIPS_PCI_REG_BASE);
	    icu_regbase = (void *)KSEG1(SOCITSC_MIPS_ICU_REG_BASE);

	socit_setup:
	    /* Register ISR */
	    if( cpu_isr_parm )
	    {
	        EXCEP_register_cpu_isr( 
	            intline,
	            isrfunc,
		    NULL,
		    &isr_ref );
            }
	    else
	    {
	        EXCEP_register_ic_isr( 
	            intline,
	            isrfunc,
		    NULL,
		    &isr_ref );
	    }

	    /* Set up interrupt controller */
	    /* enable interrupt line 1 (pci p-err and pci s-err) */
	    {
		if (!sys_eicmode) {
	   		/* Only do this if we are Standard ROCIT2 ( and not ROCIT2 + GIC ) */
			if (!gic_present) {
#define MSC01_IC_PCIINTNO 1
#define IC_REG(ofs) (*(volatile UINT32*)&msc01_ic_regbase[ofs])
				REGP(icu_regbase, MSC01_IC_SUP_OFS + (MSC01_IC_PCIINTNO*MSC01_IC_SUP_STEP)) =
					(0<<MSC01_IC_SUP_EDGE_SHF) | (0<<MSC01_IC_SUP_PRI_SHF);
		    		REG(icu_regbase,MSC01_IC_DISL) = 0xffffffff;
		    		REG(icu_regbase,MSC01_IC_DISH) = 0xffffffff;
		    		REG(icu_regbase,MSC01_IC_ENAL) = 1 << MSC01_IC_PCIINTNO;
		    		REG(icu_regbase,MSC01_IC_GENA) = MSC01_IC_GENA_GENA_BIT;
			}
			else gic_init();
		}

		/* Enable interrupt source */
		REG(pci_regbase, MSC01_PCI_INTCFG) =
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

static void
socitsc_isr( 
    void *data )
{
    UINT32 cause, pcistat;

    cause =  REG(KSEG1(SOCITSC_MIPS_ICU_REG_BASE), MSC01_IC_ISAL);
    pcistat  =  REG(KSEG1(SOCITSC_MIPS_PCI_REG_BASE), MSC01_PCI_INTSTAT);

    /* clear interrupt */
    REG(KSEG1(SOCITSC_MIPS_PCI_REG_BASE), MSC01_PCI_INTSTAT) = pcistat;

    printf( "\nSOCitSC Interrupt. Cause = 0x%02x, PCI status = 0x%02x\n", cause, pcistat );
    EXCEP_run_default_esr_handler();
}


