
/************************************************************************
 *
 *  init_platform.c
 *
 *  Platform specific startup code (c-code)
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2004 MIPS Technologies, Inc. All rights reserved.
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
#include <init.h>
#include <sys_api.h>
#include <syscon_api.h>
#include <pci_api.h>
#include <excep_api.h>
#include <stdio.h>
#include <product.h>
#include <malta.h>
#include <atlas.h>
#include <sead.h>
#include <core_sys.h>
#include <piix4.h>	/* Malta specific		*/
#include <superio.h>	/* Malta specific		*/

/************************************************************************
 *  Definitions
 ************************************************************************/

/* PCI access of 16 bit registers (PCI is always little endian) */
#define R16( addr )     REG16( SWAP_UINT16ADDR_EL( addr ) )

/************************************************************************
 *  Public variables (some not used here)
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Base of PIIX4 Power Management Module (Malta only) */
static UINT32 base;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
smi_isr( 
    void *data );


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_platform_init
 *  Description :
 *  -------------
 *
 *  Platform specific initialisation code
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_platform_init(
    bool early )	/* TRUE -> Before initmodules(), FALSE -> after */
{
    UINT32 data;
    UINT8  data8;
    volatile UINT8  *piix4_pci_io_base;

    /* TBD : Some of this may be moved to drivers (or a new "SuperIO" module) */

    if( early )
    {	
        /*  Setup variables defining the platform.
	 *  Assume that the IDs are detected using MIPS_REVISION register
         *  (special hw address reserved on Atlas, Malta, SEAD, SEAD-2 
         *  boards). If this is not the case, please modify the code.
	 */

	/*  Platform ID.
	 *  Value PRODUCT_THIRD_PARTY_ID is reserved for boards not
	 *  from MIPS Technologies.
	 */
        sys_platform = REGFIELD(REG32(KSEG1(MIPS_REVISION)), MIPS_REVISION_PROID);

	if( sys_platform == PRODUCT_THIRD_PARTY_ID )
	{
            /*  Manufacturer ID and Manufacturer's product ID */
            sys_manid = REGFIELD(REG32(KSEG1(MIPS_REVISION)), MIPS_REVISION_MANID);
            sys_manpd = REGFIELD(REG32(KSEG1(MIPS_REVISION)), MIPS_REVISION_MANPD);
	}

        /*  Core card ID.
	 *  Core cards are used on Atlas and Malta boards.
	 */
	if( (sys_platform == PRODUCT_MALTA_ID) || (sys_platform == PRODUCT_ATLASA_ID) )
	{
            sys_corecard = REGFIELD(REG32(KSEG1(MIPS_REVISION)), MIPS_REVISION_CORID);

	    if (sys_corecard == MIPS_REVISION_CORID_CORE_EMUL)
	    {
	        if (REG(MSC01_BIU_REG_BASE, MSC01_BIU_RBBASL) == CORE_SYS_REG_BASE)
		    sys_corecard = MIPS_REVISION_CORID_CORE_EMUL_SYS;
	        else
		    sys_corecard = MIPS_REVISION_CORID_CORE_EMUL_20K;
	    }
        }
	else
	    sys_corecard = MIPS_REVISION_CORID_NA;

	/* Do early platform specific initialisation
	 * (Initialisation required before YAMON modules are initialised).
	 */
        switch( sys_platform )
	{
	  case PRODUCT_SEAD2_ID :
	    /* SEAD with SOC-it 101 shares a lot of code with coreSYS */
	    /* That is achieved by setting up an artificial core card */
	    sys_corecard = 
	       (REGFIELD(REG32(KSEG1(SEAD_REVISION)), SEAD_REVISION_RTLID)
						   == SEAD_REVISION_RTLID_SOCIT101)
		? MIPS_REVISION_CORID_SEAD_MSC01
		: MIPS_REVISION_CORID_NA;
	    break;

          case PRODUCT_MALTA_ID :

            /* Perform early core specific initialisation */
	    /*  Setup Base address for PCI IO access. */
            arch_core_init( TRUE, MALTA_CPUINT_COREHI, TRUE );

            /* Enable I/O access */

            pci_config_read32( PCI_BUS_LOCAL, 
			       MALTA_DEVNUM_PIIX4,
			       PIIX4_PCI_FUNCTION_BRIDGE, 
			       PCI_SC, 
			       &data );

            pci_config_write32( PCI_BUS_LOCAL, 
			        MALTA_DEVNUM_PIIX4,
			        PIIX4_PCI_FUNCTION_BRIDGE,
			        PCI_SC, 
			        data | PCI_SC_CMD_IOS_BIT );

	    /* Select ISA mode (default is EIO) */

	    pci_config_read32( PCI_BUS_LOCAL,
			       MALTA_DEVNUM_PIIX4,
			       PIIX4_PCI_FUNCTION_BRIDGE,
			       PIIX4_PCI_GENCFG,
			       &data );

	    pci_config_write32( PCI_BUS_LOCAL,
				MALTA_DEVNUM_PIIX4,
				PIIX4_PCI_FUNCTION_BRIDGE,
				PIIX4_PCI_GENCFG,
				data | PIIX4_GENCFG_ISA_BIT );

	    /* Set top of memory accessible by ISA or DMA devices */

	    pci_config_read8( PCI_BUS_LOCAL,
			      MALTA_DEVNUM_PIIX4,
			      PIIX4_PCI_FUNCTION_BRIDGE,
			      PIIX4_PCI_TOM,
			      &data8 );

	    data8 &= ~PIIX4_TOM_TOM_MSK;
	    data8 |= PIIX4_TOM_TOM_16MB << PIIX4_TOM_TOM_SHF;

	    pci_config_write8( PCI_BUS_LOCAL,
			       MALTA_DEVNUM_PIIX4,
			       PIIX4_PCI_FUNCTION_BRIDGE,
			       PIIX4_PCI_TOM,
			       data8 );

            /**** Config mode ****/

	    piix4_pci_io_base = (volatile UINT8*)KSEG1(MALTA_PCI0_IO_BASE + PIIX4_IO_BRIDGE_START);

            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_KEY_START;


            /**** Serial port com 2 (TTY1) ****/

            /* Select device */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_DEVNUM;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_DEVNUM_COM2;

            /* Set base address */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASEHI;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_COM2_ADR >> 8;
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASELO;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_COM2_ADR & 0xFF;

            /* Select IRQ */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_IRQ;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_INTLINE_TTY1;

	    /* Mode register (Enable High Speed) */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_MODE;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_MODE_HIGHSPEED_BIT;

            /* Activate device */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_ACTIVATE;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_ACTIVATE_ENABLE_BIT;


            /**** Serial port com 1 (TTY0) ****/

            /* Select device */	   
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_DEVNUM;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_DEVNUM_COM1;

            /* Set base address */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASEHI;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_COM1_ADR >> 8;
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASELO;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_COM1_ADR & 0xFF;

            /* Select IRQ */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_IRQ;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_INTLINE_TTY0;

	    /* Mode register (Enable High Speed) */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_MODE;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_MODE_HIGHSPEED_BIT;

            /* Activate device */
	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_ACTIVATE;
	    MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_ACTIVATE_ENABLE_BIT;


            /**** Parallel port (1284) ****/

            /* Select device */   
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_DEVNUM;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_DEVNUM_PARALLEL;

            /* Set base address */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASEHI;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_1284_ADR >> 8;
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_BASELO;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_SMSC_1284_ADR & 0xFF;

            /* Select IRQ */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_IRQ;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_INTLINE_1284;

            /* Activate device */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_ACTIVATE;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_ACTIVATE_ENABLE_BIT;


            /**** Floppy Disk ****/

            /* Select device */   
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_DEVNUM;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_DEVNUM_FDD;

            /* Activate device */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_ACTIVATE;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_ACTIVATE_ENABLE_BIT;

	    /**** Keyboard, Mouse ****/

            /* Select device */   
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_DEVNUM;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_DEVNUM_KYBD;

            /* Select IRQ for keyboard */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_IRQ;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_INTLINE_KYBD;

            /* Select IRQ for mouse */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_IRQ2;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = MALTA_INTLINE_MOUSE;

            /* Activate device */
            MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_ACTIVATE;
            MALTA_SUPER_IO8( SMSC_DATA_OFS )   = SMSC_CONFIG_ACTIVATE_ENABLE_BIT;


            /**** Run mode ****/

	    MALTA_SUPER_IO8( SMSC_CONFIG_OFS ) = SMSC_CONFIG_KEY_EXIT;


            /* Initialise PIIX4 counter 0 (binary, 16 bit, mode 0) */
	    MALTA_PIIX4_IO8( PIIX4_TCW_OFS ) = 
	        (PIIX4_TCW_CS_0   << PIIX4_TCW_CS_SHF)    |
	        (PIIX4_TCW_RW_16  << PIIX4_TCW_RW_SHF)    |
	        (PIIX4_TCW_MODE_0 << PIIX4_TCW_MODE_SHF);

            /* Initial count (0) */
	    MALTA_PIIX4_IO8( PIIX4_TMRCNT0_OFS ) = 0;
	    MALTA_PIIX4_IO8( PIIX4_TMRCNT0_OFS ) = 0;

            /*  Enable IO access to Power Management device 
             *  8 bit register (PCI always little endian) 
	     */
            pci_config_write8( PCI_BUS_LOCAL, 
			       MALTA_DEVNUM_PIIX4,
			       PIIX4_PCI_FUNCTION_POWER,
			       PIIX4_PCI_PMREGMISC,
			       PIIX4_PMREGMISC_ENABLE_BIT );

            /* Disable I2C controller in FPGA */
            REG32( KSEG1( MALTA_I2CSEL ) ) &= ~MALTA_I2CSEL_FPGA_BIT;

            /* Enable SMBus Host Interface and configure it for IRQ9 */
            pci_config_write8( PCI_BUS_LOCAL,
			       MALTA_DEVNUM_PIIX4,
			       PIIX4_PCI_FUNCTION_POWER,
			       PIIX4_PCI_SMBHCFG,
			         (PIIX4_SMBHCFG_IS_IRQ9 << PIIX4_SMBHCFG_IS_SHF) |
			         PIIX4_SMBHCFG_EN_BIT );

	    /* IDE configuration, IDE Decode Enable (Primary IDE) */
	    pci_config_write16( PCI_BUS_LOCAL,
				MALTA_DEVNUM_PIIX4,
				PIIX4_PCI_FUNCTION_IDE,
				PIIX4_PCI_IDETIM_PRIM,
				PIIX4_IDETIM_IDE_BIT );

	    /* IDE configuration, IDE Decode Enable (Secondary IDE) */
	    pci_config_write16( PCI_BUS_LOCAL,
				MALTA_DEVNUM_PIIX4,
				PIIX4_PCI_FUNCTION_IDE,
				PIIX4_PCI_IDETIM_SEC,
				PIIX4_IDETIM_IDE_BIT );

				
            break;

          case PRODUCT_ATLASA_ID :

	    /* Setup North Bridge mapping global variable */
	    sys_nb_base = KSEG1(ATLAS_CORECTRL_BASE);

            /* Perform early core specific initialisation */
            arch_core_init( TRUE, ATLAS_INTLINE_COREHI, FALSE );

	    break;

	  default :
	   
	    break;
        }
    }
    else
    {
        /* !early */

        switch( sys_platform )
	{
          case PRODUCT_ATLASA_ID :

	    /* Perform core specific initialisation */
            arch_core_init( FALSE, ATLAS_INTLINE_COREHI, FALSE );
	    break;

          case PRODUCT_MALTA_ID :

	    /* Perform core specific initialisation */
            arch_core_init( FALSE, MALTA_CPUINT_COREHI, TRUE );

	    /**** Setup SMI (System Management Interrupt) ****/

            SYSCON_read( SYSCON_BOARD_PIIX4_POWER_BASE_ID,
		         (void *)&base,
			 sizeof(UINT32) );

	    base = KSEG1( (UINT32)base );

	    /*  Set LID, Thermal polarity to active low.
	     *  Set Global Standby Timer Initial Count to 0x7F.
	     */
	    REG32( base + PIIX4_GLBCTL_OFS ) |= 
	        ( (0x7F << PIIX4_GLBCTL_GSTBY_CNT_SHF) |
		  PIIX4_GLBCTL_LID_POL_BIT	       |
		  PIIX4_GLBCTL_THRM_POL_BIT );

	    /* Disable all SMI sources in Global Enable Register */
	    R16( base + PIIX4_GLBEN_OFS ) = 0;

	    /*  Disable all SMI sources in General Purpose Enable
	     *  Register, except GPI (PCI Power Management Event).
	     *  This means that the board way be awakened by
	     *  e.g. Ethernet Magic(TM) Packets.
	     */
	    R16( base + PIIX4_GPEN_OFS ) = PIIX4_GPEN_GPIEN_BIT;
	    
	    /* Clear General Purpose Status Register */
	    R16( base + PIIX4_GPSTS_OFS ) = 0xFFFF;
	    
	    /* Clear Power Management Status Register */
	    R16( base + PIIX4_PMSTS_OFS ) = 0xFFFF;

	    /* Clear Global Status Register */
	    R16( base + PIIX4_GLBSTS_OFS ) = 0xFFFF;

            /* Register ISR for SMI interrupts */
	    EXCEP_register_cpu_isr( MALTA_CPUINT_SMI,
				    smi_isr,
				    NULL,
				    NULL );

	    /* Enable SMI (should never be generated) */
	    REG32( base + PIIX4_GLBCTL_OFS ) |= 
	        ( PIIX4_GLBCTL_SMI_EN_BIT | 
		  PIIX4_GLBCTL_EOS_BIT );

	    break;

	  default :
	    break;
        }
    }      
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

static void
smi_isr( 
    void *data )
{
    UINT16 status, gpsts, pmsts, glben, gpen;

    status = R16( base + PIIX4_GLBSTS_OFS );
    gpsts  = R16( base + PIIX4_GPSTS_OFS );
    pmsts  = R16( base + PIIX4_PMSTS_OFS );
    glben  = R16( base + PIIX4_GLBEN_OFS );
    gpen   = R16( base + PIIX4_GPEN_OFS );

    printf( "\nSMI interrupt."
	    "\nStatus = 0x%04x"
	    "\nGPSTS  = 0x%04x"
	    "\nPMSTS  = 0x%04x" 
	    "\nGLBEN  = 0x%04x"
	    "\nGPEN   = 0x%04x"
	    "\n",
	      status, gpsts, pmsts, glben, gpen );

    while(1);
}
