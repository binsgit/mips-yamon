
/************************************************************************
 *
 *  excep_platform.c
 *
 *  Platform specific exception functions
 *
 *  TBD : This file calls pci functions. Since the EXCEP module is 
 *  installed before the PCI module, this is not very nice (but it
 *  works with the present implementation of the PCI module).
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

#include <excep.h>
#include <excep_api.h>

#include <sys_api.h>
#include <pci_api.h>

#include <product.h>

/* Atlas */
#include <atlas.h>
#include <icta.h>

/* Malta */
#include <malta.h>
#include <piix4.h>

/* Sead-2 */
#include <sead.h>
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
static volatile UINT8  *piix4_pci_io_base;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_excep_init_intctrl
 *  Description :
 *  -------------
 *
 *  Initialise interrupt controller.
 *
 *  Parameters :
 *  ------------
 *
 *  Function will fill out the ic_count and ic_int parameters.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_init_intctrl(
    UINT32 *ic_count,   /* Number of interrupts handled by interrupt 
			   controller (0 if no controller is available) */
    UINT32 *ic_int )    /* HW interrupt used by interrupt controller	*/
{
    UINT32 data;
    volatile UINT32 *ic_reg;
    
    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

        /* Atlas interrupt controller (icta) needs no initialisation */

        *ic_count = ICTA_IC_COUNT;
	*ic_int   = ATLAS_CPUINT_ICTA;

        break;

      case PRODUCT_MALTA_ID :

	/**** Setup PCI A..D sterring ****/

        /*  PCI A..B are routed to IRQ10.
	 *  PCI C..D are routed to IRQ11.
	 */

	    piix4_pci_io_base = (volatile UINT8*)KSEG1(MALTA_PCI0_IO_BASE + PIIX4_IO_BRIDGE_START);

	pci_config_write32( 
	    PCI_BUS_LOCAL,
	    MALTA_DEVNUM_PIIX4,
	    PIIX4_PCI_FUNCTION_BRIDGE,
	    PIIX4_PCI_PIRQRC,
	    ( PIIX4_PIRQRC_PCIA_IR_IRQ10 << PIIX4_PIRQRC_PCIA_IR_SHF ) |
	    ( PIIX4_PIRQRC_PCIB_IR_IRQ10 << PIIX4_PIRQRC_PCIB_IR_SHF ) |
	    ( PIIX4_PIRQRC_PCIC_IR_IRQ11 << PIIX4_PIRQRC_PCIC_IR_SHF ) |
	    ( PIIX4_PIRQRC_PCID_IR_IRQ11 << PIIX4_PIRQRC_PCID_IR_SHF )
	);

	/**** Setup for level trigger on IRQ10 and IRQ11 ****/

	MALTA_PIIX4_IO8( PIIX4_ELCR2_OFS ) |= (PIIX4_ELCR2_IRQ10LEVEL_BIT |
					       PIIX4_ELCR2_IRQ11LEVEL_BIT );

	/**** Enable Serial IRQ ****/

	/* GENCFG */

	pci_config_read32(
	    PCI_BUS_LOCAL,
	    MALTA_DEVNUM_PIIX4,
	    PIIX4_PCI_FUNCTION_BRIDGE,
	    PIIX4_PCI_GENCFG,
	    &data );

	pci_config_write32(
	    PCI_BUS_LOCAL,
	    MALTA_DEVNUM_PIIX4,
	    PIIX4_PCI_FUNCTION_BRIDGE,
	    PIIX4_PCI_GENCFG,
	    data | PIIX4_GENCFG_SERIRQ_BIT );

	/* SERIRQ */

	pci_config_read8(
	    PCI_BUS_LOCAL,
	    MALTA_DEVNUM_PIIX4,
	    PIIX4_PCI_FUNCTION_BRIDGE,
	    PIIX4_PCI_SERIRQC,
	    (UINT8 *)&data );

	pci_config_write8(
	    PCI_BUS_LOCAL,
	    MALTA_DEVNUM_PIIX4,
	    PIIX4_PCI_FUNCTION_BRIDGE,
	    PIIX4_PCI_SERIRQC,
	    *(UINT8 *)(&data) | PIIX4_SERIRQC_ENABLE_BIT | PIIX4_SERIRQC_CONT_BIT );

        /**** Initialise PIIX4 master interrupt controller ****/

	/* ICW1 */
	MALTA_PIIX4_IO8( PIIX4_ICW1M_OFS ) = PIIX4_ICW1_ICWSEL_BIT |
					     PIIX4_ICW1_ICW4WR_BIT;

        /* ICW2 */
	MALTA_PIIX4_IO8( PIIX4_ICW2M_OFS ) = 0 << PIIX4_ICW2_BASE_SHF;

	/* ICW3 */
	MALTA_PIIX4_IO8( PIIX4_ICW3M_OFS ) = PIIX4_ICW3M_CAS_BIT;

	/* ICW4 */
	MALTA_PIIX4_IO8( PIIX4_ICW4M_OFS ) = PIIX4_ICW4_UPMODE_BIT;

	/* OCW3 (select ISR when later reading OCW3) */
	MALTA_PIIX4_IO8( PIIX4_OCW3M_OFS ) = 
			       (PIIX4_OCW3_OCWSEL_3 << PIIX4_OCW3_OCWSEL_SHF) |
			       (PIIX4_OCW3_RRC_ISR  << PIIX4_OCW3_RRC_SHF)    |
			        PIIX4_OCW3_ESMM_BIT;


        /**** Initialise PIIX4 slave interrupt controller ****/

	/* ICW1 */
	MALTA_PIIX4_IO8( PIIX4_ICW1S_OFS ) = PIIX4_ICW1_ICWSEL_BIT |
					     PIIX4_ICW1_ICW4WR_BIT;

        /* ICW2 */
	MALTA_PIIX4_IO8( PIIX4_ICW2S_OFS ) = 1 << PIIX4_ICW2_BASE_SHF;

	/* ICW3 */
	MALTA_PIIX4_IO8( PIIX4_ICW3S_OFS ) = PIIX4_ICW3S_SID_DEF << 
					     PIIX4_ICW3S_SID_SHF;

	/* ICW4 */
	MALTA_PIIX4_IO8( PIIX4_ICW4S_OFS ) = PIIX4_ICW4_UPMODE_BIT;


	/**** Mask all interrupts ****/

	MALTA_PIIX4_IO8( PIIX4_OCW1M_OFS ) = 0xFF;
	MALTA_PIIX4_IO8( PIIX4_OCW1S_OFS ) = 0xFF;

        /* The slave controller is attached to master IRQ2 */

        MALTA_PIIX4_IO8( PIIX4_OCW1M_OFS ) &= ~(1 << 2 );


	/**** Fill out parameters ****/
		
	*ic_count = PIIX4_IRQ_COUNT;
	*ic_int   = sys_eicmode ? MALTA_EICINT_PIIX4 : MALTA_CPUINT_PIIX4;

	break;

      case PRODUCT_SEAD2_ID :

	/* Sead-2 msc01 system interrupt controller */
	if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	{
#define STEP MSC01_IC_SUP_STEP
	    /* Set up interrupt controller */
	    /* enable interrupt lines for tty0 and tty1 */

	    ic_reg = (volatile UINT32 *)MSC01_IC_REG_BASE;

	    ic_reg[(MSC01_IC_SUP_OFS + (SEAD_MSC01_INTLINE_TTY0 * STEP))/4] =
		    (0<<MSC01_IC_SUP_EDGE_SHF) | (0<<MSC01_IC_SUP_PRI_SHF);
	    ic_reg[(MSC01_IC_SUP_OFS + (SEAD_MSC01_INTLINE_TTY1 * STEP))/4] =
		    (0<<MSC01_IC_SUP_EDGE_SHF) | (0<<MSC01_IC_SUP_PRI_SHF);

	    ic_reg[MSC01_IC_DISL_OFS/4] = 0xffffffff;
	    ic_reg[MSC01_IC_DISH_OFS/4] = 0xffffffff;
	    ic_reg[MSC01_IC_ENAL_OFS/4] = (1 << SEAD_MSC01_INTLINE_TTY0) |
		                          (1 << SEAD_MSC01_INTLINE_TTY1);

	    ic_reg[MSC01_IC_GENA_OFS/4] = MSC01_IC_GENA_GENA_BIT;
#undef STEP

	    /**** Fill out parameters ****/
	    *ic_count = SEAD_MSC01_IC_COUNT;
	    *ic_int   = SEAD_MSC01_CPUINT;
	    break;
	}
	/* else fall through */

      default :

        /* No interrupt controller */
	*ic_count = 0;

        break;
    }
}


/************************************************************************
 *
 *                          arch_excep_enable_int
 *  Description :
 *  -------------
 *
 *  Enable specific source in interrupt controller
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_enable_int(
    UINT32 ic_line )    /* Interrupt source                             */
{
    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

        REG32(KSEG1( ATLAS_ICTA_BASE + ICTA_INTSETEN_OFS )) =
            1 << ic_line;
        break;

      case PRODUCT_MALTA_ID :

        if( ic_line < 8 )
        {
            MALTA_PIIX4_IO8( PIIX4_OCW1M_OFS ) &= ~(1 << ic_line);
        }
        else
        {
            MALTA_PIIX4_IO8( PIIX4_OCW1S_OFS ) &= ~(1 << (ic_line - 8));

            /* remember the slave is attached to master IRQ2 */
            MALTA_PIIX4_IO8( PIIX4_OCW1M_OFS ) &= ~(1 << 2 );
        }

        break;

      default :

        /* No interrupt controller */
        break;
    }
}


/************************************************************************
 *
 *                          arch_excep_disable_int
 *  Description :
 *  -------------
 *
 *  Disable specific source in interrupt controller
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_disable_int(
    UINT32 ic_line )    /* Interrupt source                             */
{
    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

        REG32(KSEG1( ATLAS_ICTA_BASE + ICTA_INTRSTEN_OFS )) =
            1 << ic_line;
        break;

      case PRODUCT_MALTA_ID :

        if( ic_line < 8 )
        {
            MALTA_PIIX4_IO8( PIIX4_OCW1M_OFS ) |= (1 << ic_line);
        }
        else
        {
            MALTA_PIIX4_IO8( PIIX4_OCW1S_OFS ) |= (1 << (ic_line - 8));
        }

        break;

      default :

        /* No interrupt controller */
        break;
    }
}


/************************************************************************
 *
 *                          arch_excep_pending
 *  Description :
 *  -------------
 *
 *  Return pending interrupt(s) in interrupt controller.
 *  On some platforms, all pending interrupts are returned, on other
 *  platforms only the hightest priority interrupt is returned.
 *
 *  Return values :
 *  ---------------
 *
 *  Pending interrupt(s)
 *
 ************************************************************************/
UINT32
arch_excep_pending( void )
{
    UINT32 index;

    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

	/* Return all pending interrupts */
        return REG32(KSEG1( ATLAS_ICTA_BASE + ICTA_INTSTATUS_OFS ));

      case PRODUCT_MALTA_ID :

        /* Poll master */
	MALTA_PIIX4_IO8( PIIX4_OCW3M_OFS ) = 
           (PIIX4_OCW3_OCWSEL_3 << PIIX4_OCW3_OCWSEL_SHF) |
	   PIIX4_OCW3_PMC_BIT;

	index = MALTA_PIIX4_IO8( PIIX4_OCW3M_OFS );

	if( !(index & 0x80) )  /* MSb indicates pending interrupt */
	{
	    /* No interrupt */
	    return 0;
        }

	index &= 0x7;

	if( index == 2 )  /* Slave controller cascaded using IRQ2 */
	{
	    /* Poll slave */
	    MALTA_PIIX4_IO8( PIIX4_OCW3S_OFS ) = 
		(PIIX4_OCW3_OCWSEL_3 << PIIX4_OCW3_OCWSEL_SHF) |
	        PIIX4_OCW3_PMC_BIT;

	    index = MALTA_PIIX4_IO8( PIIX4_OCW3S_OFS );

	    if( !(index & 0x80) )  /* MSb indicates pending interrupt */
	    {
	        /* No interrupt */
	        return 0;
            }

	    index = (index & 0x7) + 8;
        }

	return 1 << index;

      case PRODUCT_SEAD2_ID :

	/* Return all pending interrupts */
	return REG(MSC01_IC_REG_BASE, MSC01_IC_ISAL);

      default :

        /* No interrupt controller */
	return 0;
    }
}


/************************************************************************
 *
 *                          arch_excep_eoi
 *  Description :
 *  -------------
 *
 *  Perform EOI cycle for indicated interrupt
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_eoi(
    UINT32 index )
{
    switch( sys_platform )
    {
      case PRODUCT_MALTA_ID :

#define NSEOI    ( (PIIX4_OCW2_CODE_NSEOI << PIIX4_OCW2_CODE_SHF) | \
		   (PIIX4_OCW2_OCWSEL_2   << PIIX4_OCW2_OCWSEL_SHF) )

	/* Perform Non-specific EOI for master (ignore index) */
	MALTA_PIIX4_IO8( PIIX4_OCW2M_OFS ) = NSEOI;

	/* Perform Non-specific EOI for slave (ignore index) */
	MALTA_PIIX4_IO8( PIIX4_OCW2S_OFS ) = NSEOI;

	break;

      default :

        /* No EOI needed */
	break;
    }
}

/************************************************************************
 *
 *                          arch_eic_init
 *  Description :
 *  -------------
 *
 *  Initialise EIC (Extended Interrupt Controller)
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_eic_init(void)
{
    volatile UINT32 *ic_reg;
    UINT32 i;

    ic_reg = (volatile UINT32 *)MSC01_IC_REG_BASE;

    /* Reset controller */
    ic_reg[MSC01_IC_RST_OFS/4] = MSC01_IC_RST_RST_BIT;
    for (i = 0; i < 64; i++) {
	/* Program RAM to use default register set */
	ic_reg[MSC01_IC_RAMW_OFS/4] = (i << MSC01_IC_RAMW_ADDR_SHF)
	    | (0 << MSC01_IC_RAMW_DATA_SHF);
	/* Set level mode */
	ic_reg[MSC01_IC_SUP_OFS/4 + i*2] = 0;
    }
    ic_reg[MSC01_IC_GENA_OFS/4] = MSC01_IC_GENA_GENA_BIT;
}

/************************************************************************
 *
 *                          arch_eic_enable_int
 *  Description :
 *  -------------
 *
 *  Enable EIC interrupt
 *
 *  Parameters :
 *  ------------
 *
 *  cpu_int: EIC interrupt to enable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 *  The only EIC currently supported is in SOCit
 *
 ************************************************************************/
void
arch_eic_enable_int(
    UINT32 cpu_int
    )
{
    volatile UINT32 *ic_reg;
    ic_reg = (volatile UINT32 *)MSC01_IC_REG_BASE;

    if (cpu_int < 32)
	ic_reg[MSC01_IC_ENAL_OFS/4] = 1 << cpu_int;
    else
	ic_reg[MSC01_IC_ENAH_OFS/4] = 1 << (cpu_int - 32);
}


/************************************************************************
 *
 *                          arch_eic_disable_int
 *  Description :
 *  -------------
 *
 *  Disable EIC interrupt
 *
 *  Parameters :
 *  ------------
 *
 *  cpu_int: EIC interrupt to disable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 *  The only EIC currently supported is in SOCit
 *
 ************************************************************************/
void
arch_eic_disable_int(
    UINT32 cpu_int
    )
{
    volatile UINT32 *ic_reg;
    ic_reg = (volatile UINT32 *)MSC01_IC_REG_BASE;
    if (cpu_int < 32)
	ic_reg[MSC01_IC_DISL_OFS/4] = 1 << cpu_int;
    else
	ic_reg[MSC01_IC_DISH_OFS/4] = 1 << (cpu_int - 32);
}

/************************************************************************
 *
 *                          arch_eic_eoi
 *  Description :
 *  -------------
 *
 *  Perform EOI cycle for indicated interrupt on EIC
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_eic_eoi(
    UINT32 index )
{
    /* Level triggered interrupts ? */
}

