
/************************************************************************
 *
 *  pci_platform.c
 *
 *  Platform but not core card specific PCI code
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
#include <syserror.h>
#include <sys_api.h>
#include <syscon_api.h>

#include <pci.h>
#include <pci_api.h>

#include <product.h>
#include <atlas.h>
#include <malta.h>

#include <piix4.h>	/* Malta specific */

/************************************************************************
 *  Definitions
 ************************************************************************/

/* System slot in Compact PCI Rack (Atlas specific) */
#define SYSTEM_SLOT         ((REG32(KSEG1(ATLAS_STATUS))) & ATLAS_STATUS_PCISYS_SET)

/* Max number of special BARs (i.e. BARs with fixed mapping) */
#define PCI_MAX_FIXED_BARS  6

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/


/**** Atlas specific ****/


/* Known Atlas devices */

static t_known_dev known_dev_atlas[] =
{
    { PCI_VENDID_PHILIPS, 
      PCI_DEVID_9730,
      0,
      ATLAS_INTLINE_9730,
      "Philips", 
      "SAA9730" },

    { PCI_VENDID_SYMBIOS,      
      PCI_DEVID_SCSI,
      0,
      ATLAS_INTLINE_SCSI,
      "Symbios",  
      "53C810A"  },

    { PCI_VENDID_INTEL_A,    
      PCI_DEVID_PPB,
      0,
      PCI_MMII_INTLINE_NONE,
      "Intel",    
      "21150" },

    { 0 }      /* Reserved for system controller (core card specific) */
};
#define PCI_KNOWN_DEV_ATLAS	(sizeof(known_dev_atlas)/sizeof(t_known_dev))



/**** Malta specific ****/


/* Known Malta devices */

static t_known_dev known_dev_malta[] =
{
    { PCI_VENDID_AMD,
      PCI_DEVID_79C973,
      0,
      MALTA_INTLINE_79C973,
      "AMD",
      "79C973" },

    { PCI_VENDID_CRYSTAL,
      PCI_DEVID_4281,
      0,
      MALTA_INTLINE_4281,
      "Crystal",
      "4281" },

    { PCI_VENDID_INTEL,
      PCI_DEVID_PIIX4_BRIDGE,
      PIIX4_PCI_FUNCTION_BRIDGE,
      PCI_MMII_INTLINE_NONE,
      "Intel",
      "PIIX4 Bridge" },

    { PCI_VENDID_INTEL,
      PCI_DEVID_PIIX4_IDE,
      PIIX4_PCI_FUNCTION_IDE,
      PCI_MMII_INTLINE_NONE,
      "Intel",
      "PIIX4 IDE" },

    { PCI_VENDID_INTEL,
      PCI_DEVID_PIIX4_USB,
      PIIX4_PCI_FUNCTION_USB,
      MALTA_INTLINE_PIIX4_USB,
      "Intel",
      "PIIX4 USB" },

    { PCI_VENDID_INTEL,
      PCI_DEVID_PIIX4_POWER,
      PIIX4_PCI_FUNCTION_POWER,
      MALTA_INTLINE_PIIX4_SMB,
      "Intel",
      "PIIX4 Power" },

    { 0 }      /* Reserved for system controller (core card specific) */
};
#define PCI_KNOWN_DEV_MALTA	(sizeof(known_dev_malta)/sizeof(t_known_dev))


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          pci_config
 *  Description :
 *  -------------
 *
 *  Autodetect and autoconfigure PCI
 *
 *  Return values :
 *  ---------------
 *
 *  0 = OK, otherwise error
 *
 ************************************************************************/
UINT32 
pci_config( void )
{
    t_pci_bar_req bar_req[PCI_MAX_FIXED_BARS];
    UINT32	  bar_count = 0;
    t_known_dev   *known_devs;
    UINT32	  known_devs_count;
    UINT32	  rc;
    UINT8	  intline;

    DISP_STR("PCI");

    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

	/* Known Atlas devices */
	known_devs	 = known_dev_atlas;
	known_devs_count = PCI_KNOWN_DEV_ATLAS;
	intline		 = ATLAS_INTLINE_COREHI;
	break;
	
      case PRODUCT_MALTA_ID :

	/* Known Malta devices */
	known_devs	 = known_dev_malta;
	known_devs_count = PCI_KNOWN_DEV_MALTA;
	/* System controller not connected to interrupt controller */  
	intline		 = PCI_MMII_INTLINE_NONE;  

	/**** Special BAR requirement due to PIIX4 device ****/

	/* Bridge */
	bar_req[0].vendorid     = PCI_VENDID_INTEL;
        bar_req[0].devid	= PCI_DEVID_PIIX4_BRIDGE;
	bar_req[0].function	= 0;
	bar_req[0].bar.pos	= PIIX4_PCI_BAR_BRIDGE_POS;
	bar_req[0].bar.io	= TRUE;
	bar_req[0].bar.prefetch = FALSE;
	bar_req[0].bar.start    = PIIX4_IO_BRIDGE_START;
	bar_req[0].bar.size     = PIIX4_IO_BRIDGE_SIZE;

	/* Power management */
	bar_req[1].vendorid     = PCI_VENDID_INTEL;
        bar_req[1].devid	= PCI_DEVID_PIIX4_POWER;
	bar_req[1].function	= PIIX4_PCI_FUNCTION_POWER;
	bar_req[1].bar.pos	= PIIX4_PCI_BAR_POWER_POS;
	bar_req[1].bar.io	= TRUE;
	bar_req[1].bar.prefetch = FALSE;
	bar_req[1].bar.start    = PIIX4_IO_POWER_START;
	bar_req[1].bar.size     = PIIX4_IO_POWER_SIZE;

	/* SMBus */
	bar_req[2].vendorid     = PCI_VENDID_INTEL;
        bar_req[2].devid	= PCI_DEVID_PIIX4_POWER;
	bar_req[2].function	= PIIX4_PCI_FUNCTION_POWER;
	bar_req[2].bar.pos	= PIIX4_PCI_BAR_SMB_POS;
	bar_req[2].bar.io	= TRUE;
	bar_req[2].bar.prefetch = FALSE;
	bar_req[2].bar.start    = PIIX4_IO_SMB_START;
	bar_req[2].bar.size     = PIIX4_IO_SMB_SIZE;

	bar_count = 3;

        break;

      default :	/* Should not happen */

        return ERROR_PCI_STRUCTURE;
    }

    /* Configure system controller regarding PCI */
    rc = arch_pci_config_controller( bar_req, &bar_count, PCI_MAX_FIXED_BARS,
				     intline,
			             &known_devs[known_devs_count - 1] );
    if( rc != OK )
        return rc;

    DISP_STR("PCI_AUTO");

    /* Perform generic auto-detect and auto-configuration */
    return pci_autoconfig( bar_req, bar_count,
			   known_devs, known_devs_count );
}


/************************************************************************
 *
 *                          arch_pci_system_slot
 *  Description :
 *  -------------
 *
 *  Determine whether board is located in Compact PCI system slot.
 *  Platforms that don't have a compact PCI connector will always
 *  return FALSE.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE ->  In Compact PCI system slot
 *  FALSE -> Not in system slot.
 *
 ************************************************************************/
bool
arch_pci_system_slot( void )
{
    if( (sys_platform == PRODUCT_ATLASA_ID) )
        return SYSTEM_SLOT;
    else
        return FALSE;
}


/************************************************************************
 *
 *                          arch_pci_slot
 *  Description :
 *  -------------
 *
 *  Determine whether a device number corresponds to a PCI slot
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE ->  PCI slot
 *  FALSE -> Not PCI slot
 *
 ************************************************************************/
bool
arch_pci_slot( 
    UINT8 dev,			/* PCI device number			*/
    UINT8 *number )		/* OUT : PCI slot number		*/
{
    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

        *number = 1;  /* Only 1 slot */
        return (dev == ATLAS_DEVNUM_PCI_SLOT) ? TRUE : FALSE;

      case PRODUCT_MALTA_ID :

        switch( dev )
	{
	  case MALTA_DEVNUM_PCI_SLOT4 :
	    *number = 4;
	    return TRUE;
	  case MALTA_DEVNUM_PCI_SLOT3 :
	    *number = 3;
	    return TRUE;
	  case MALTA_DEVNUM_PCI_SLOT2 :
	    *number = 2;
	    return TRUE;
	  case MALTA_DEVNUM_PCI_SLOT1 :
	    *number = 1;
	    return TRUE;
        }
	return FALSE;

      default :
        return FALSE;
    }
}


/************************************************************************
 *
 *                          arch_pci_slot_intline
 *  Description :
 *  -------------
 *
 *  Determine the interrupt line (for interrupt controller) used for
 *  PCI slot identified by PCI device number.
 *
 *  Return values :
 *  ---------------
 *
 *  Interrupt line
 *
 ************************************************************************/
UINT8
arch_pci_slot_intline( 
    UINT8 dev,			/* PCI device number of slot		*/
    UINT8 intpin )		/* Int. pin (A/B/C/D) used by device	*/
{
    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :

        switch( intpin )
        {
          case PCI_MMII_INTPIN_A :
            return ATLAS_INTLINE_PCIA;
          case PCI_MMII_INTPIN_B :
            return ATLAS_INTLINE_PCIB;
          case PCI_MMII_INTPIN_C :
            return ATLAS_INTLINE_PCIC;
          case PCI_MMII_INTPIN_D :
            return ATLAS_INTLINE_PCID;
	}
        return PCI_MMII_INTLINE_NONE;

      case PRODUCT_MALTA_ID :
        
	switch( dev )
	{
	  case MALTA_DEVNUM_PCI_SLOT1 :
	  
	    switch( intpin )
	    {
	      case PCI_MMII_INTPIN_A :	
	        return MALTA_INTLINE_SLOT1_A;
	      case PCI_MMII_INTPIN_B :
	        return MALTA_INTLINE_SLOT1_B;
	      case PCI_MMII_INTPIN_C :
	        return MALTA_INTLINE_SLOT1_C;
	      case PCI_MMII_INTPIN_D :
	        return MALTA_INTLINE_SLOT1_D;
	    }
	    return PCI_MMII_INTLINE_NONE;

	  case MALTA_DEVNUM_PCI_SLOT2 :
	  
	    switch( intpin )
	    {
	      case PCI_MMII_INTPIN_A :	
	        return MALTA_INTLINE_SLOT2_A;
	      case PCI_MMII_INTPIN_B :
	        return MALTA_INTLINE_SLOT2_B;
	      case PCI_MMII_INTPIN_C :
	        return MALTA_INTLINE_SLOT2_C;
	      case PCI_MMII_INTPIN_D :
	        return MALTA_INTLINE_SLOT2_D;
	    }
	    return PCI_MMII_INTLINE_NONE;

	  case MALTA_DEVNUM_PCI_SLOT3 :
	  
	    switch( intpin )
	    {
	      case PCI_MMII_INTPIN_A :	
	        return MALTA_INTLINE_SLOT3_A;
	      case PCI_MMII_INTPIN_B :
	        return MALTA_INTLINE_SLOT3_B;
	      case PCI_MMII_INTPIN_C :
	        return MALTA_INTLINE_SLOT3_C;
	      case PCI_MMII_INTPIN_D :
	        return MALTA_INTLINE_SLOT3_D;
	    }
	    return PCI_MMII_INTLINE_NONE;


	  case MALTA_DEVNUM_PCI_SLOT4 :
	  
	    switch( intpin )
	    {
	      case PCI_MMII_INTPIN_A :	
	        return MALTA_INTLINE_SLOT4_A;
	      case PCI_MMII_INTPIN_B :
	        return MALTA_INTLINE_SLOT4_B;
	      case PCI_MMII_INTPIN_C :
	        return MALTA_INTLINE_SLOT4_C;
	      case PCI_MMII_INTPIN_D :
	        return MALTA_INTLINE_SLOT4_D;
	    }
	    return PCI_MMII_INTLINE_NONE;

        }
	/* Should not happen */
	return PCI_MMII_INTLINE_NONE;


      default :

        /* Should not happen */
	return PCI_MMII_INTLINE_NONE;
    }
}


/************************************************************************
 *
 *                          arch_pci_remote_intline
 *  Description :
 *  -------------
 *
 *  Determine the interrupt line (for interrupt controller) used for
 *  PCI interrupt pin on Compact PCI connector
 *
 *  Return values :
 *  ---------------
 *
 *  Interrupt line
 *
 ************************************************************************/
UINT8
arch_pci_remote_intline( 
    UINT8 intpin )		/* Int. pin (A/B/C/D) used by device	*/
{
    if( (sys_platform == PRODUCT_ATLASA_ID) )
    {
        switch( intpin )
        {
          case PCI_MMII_INTPIN_A :
            return ATLAS_INTLINE_CPCIA;
          case PCI_MMII_INTPIN_B :
            return ATLAS_INTLINE_CPCIB;
          case PCI_MMII_INTPIN_C :
            return ATLAS_INTLINE_CPCIC;
          case PCI_MMII_INTPIN_D :
            return ATLAS_INTLINE_CPCID;
          default :
            return PCI_MMII_INTLINE_NONE;
        }
    }
    else /* Should not happen */
        return PCI_MMII_INTLINE_NONE;
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

