
/************************************************************************
 *
 *  pci_core.c
 *
 *  Core card specific PCI code
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
#include <syserror.h>

#include <pci.h>
#include <pci_api.h>

#include <product.h>
#include <atlas.h>
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

/* SDRAM ranges on Galileo */
static UINT32 scs10_start, scs10_size;
static UINT32 scs32_start, scs32_size;

/* Galileo names */
static char *name_galileo	= "Galileo";
static char *name_64120		= "64120";
static char *name_algorithmics  = "Algorithmics";
static char *name_bonito64	= "Bonito64";
static char *name_mips          = "MIPS";
static char *name_sysctrl	= "MSC01";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
config_gt64120( void );

static UINT32
config_bonito64( void );

static UINT32
config_sysctrl( void );

static void
gt_calc_range( 
    UINT32 ld,
    UINT32 hd,
    UINT32 *start,
    UINT32 *size );

static UINT32
sc_calc_pwr2( UINT32 arg );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_pci_config_controller
 *  Description :
 *  -------------
 *
 *  Configure system controller regarding PCI.
 *
 *  Return values :
 *  ---------------
 *
 *  OK or ERROR_PCI_STRUCTURE (should never happen)
 *
 ************************************************************************/
UINT32
arch_pci_config_controller(
    t_pci_bar_req *bar_req,	   /* Array for fixed bar requests	*/
    UINT32	  *bar_count,	   /* Current number of fixed bar req.	*/
    UINT32	  max_bar_count,   /* Max fixed bar requests		*/
    UINT8	  intline,	   /* Interrupt line for controller	*/
    t_known_dev   *controller )    /* Controller data to be filled out  */
{
#ifdef BONITO_DEVID_HACK
#undef     PCI_DEVID_BONITO64
    UINT16 PCI_DEVID_BONITO64;
#endif
    controller->intline = intline;
  
    switch( sys_corecard )
    {
      case MIPS_REVISION_CORID_CORE_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA2 :
      case MIPS_REVISION_CORID_CORE_EMUL_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA3 :
      case MIPS_REVISION_CORID_CORE_24K :

	/**** MIPS system controller: SOC-it or ROCit ****/
	DISP_STR( sys_sysconid == MSC01_ID_SC_ROCIT ? "ROCit" : "SOCit101" );

	/* Setup known device data */
	controller->vendorid = PCI_VENDID_MIPS;
	controller->devid    = PCI_DEVID_MSC01;
	controller->function = 0;
	controller->vendor   = name_mips;
	controller->device   = name_sysctrl;

	/* Configure System Controller */
	config_sysctrl();

	if( *bar_count + 1 > max_bar_count )
	    return ERROR_PCI_STRUCTURE;

        /*  BAR requirements due to system Ctrl */

	bar_req[*bar_count].vendorid         = PCI_VENDID_MIPS;
	bar_req[*bar_count].devid	     = PCI_DEVID_MSC01;
	bar_req[*bar_count].function	     = 0;
	bar_req[*bar_count].bar.pos	     = PCI_BAR(0);
	bar_req[*bar_count].bar.io	     = FALSE;
	bar_req[*bar_count].bar.prefetch     = FALSE;
	bar_req[*bar_count].bar.start        = 0;
	bar_req[*bar_count].bar.size         = sc_calc_pwr2( sys_ramsize );

	(*bar_count) += 1;

	return OK;

	
      case MIPS_REVISION_CORID_BONITO64 :
      case MIPS_REVISION_CORID_CORE_20K :
      case MIPS_REVISION_CORID_CORE_EMUL_20K :

        /**** Bonito64 system controller ****/

        DISP_STR( "BONITO64" );

	/* Setup known device data */
	controller->vendorid = PCI_VENDID_ALGORITHMICS;
#ifdef BONITO_DEVID_HACK
	/* Needed for CORE_EMUL_20K until Rick fixes Device ID */
	/* Read actual device id and use that as reuired device id */
	arch_pci_config_access( 0, 0x11, 0, 2, FALSE, 2, &PCI_DEVID_BONITO64);
#endif
	controller->devid    = PCI_DEVID_BONITO64;
	controller->function = 0;
	controller->vendor   = name_algorithmics;
	controller->device   = name_bonito64;

	/* Configure Bonito64 */
	config_bonito64();

	if( *bar_count + 2 > max_bar_count )
	    return ERROR_PCI_STRUCTURE;

        /*  BAR requirements due to Bonito */

	bar_req[*bar_count].vendorid         = PCI_VENDID_ALGORITHMICS;
	bar_req[*bar_count].devid	     = PCI_DEVID_BONITO64;
	bar_req[*bar_count].function	     = 0;
	bar_req[*bar_count].bar.pos	     = PCI_BAR(0);
	bar_req[*bar_count].bar.io	     = FALSE;
	bar_req[*bar_count].bar.prefetch     = FALSE;
	bar_req[*bar_count].bar.start        = 0;
	bar_req[*bar_count].bar.size         = sys_ramsize / 2;

	bar_req[*bar_count + 1].vendorid     = PCI_VENDID_ALGORITHMICS;
	bar_req[*bar_count + 1].devid	     = PCI_DEVID_BONITO64;
	bar_req[*bar_count + 1].function     = 0;
	bar_req[*bar_count + 1].bar.pos	     = PCI_BAR(1);
	bar_req[*bar_count + 1].bar.io	     = FALSE;
	bar_req[*bar_count + 1].bar.prefetch = FALSE;
	bar_req[*bar_count + 1].bar.start    = sys_ramsize / 2;
	bar_req[*bar_count + 1].bar.size     = sys_ramsize / 2;

	(*bar_count) += 2;

	return OK;


      case MIPS_REVISION_CORID_QED_RM5261 :
      case MIPS_REVISION_CORID_CORE_LV :
      case MIPS_REVISION_CORID_CORE_FPGA :
      case MIPS_REVISION_CORID_CORE_FPGAr2 :

        /**** Galileo GT64120 system controller ****/

        DISP_STR( "GT64120" );

	/* Setup known device data */
	controller->vendorid = PCI_VENDID_GALILEO;
	controller->devid    = PCI_DEVID_64120;
	controller->function = 0;
	controller->vendor   = name_galileo;
	controller->device   = name_64120;

        /* Determine SDRAM banks */
        gt_calc_range( GT_SCS10LD_OFS, GT_SCS10HD_OFS, 
		       &scs10_start, &scs10_size );

        gt_calc_range( GT_SCS32LD_OFS, GT_SCS32HD_OFS, 
		       &scs32_start, &scs32_size );

	/* Configure Galileo */
        config_gt64120();

	if( *bar_count + 3 > max_bar_count )
	    return ERROR_PCI_STRUCTURE;

        /*  BAR requirements due to Galileo system controller */

	bar_req[*bar_count].vendorid         = PCI_VENDID_GALILEO;
	bar_req[*bar_count].devid	     = PCI_DEVID_64120;
	bar_req[*bar_count].function	     = 0;
	bar_req[*bar_count].bar.pos	     = GT_BAR_SCS10_POS;
	bar_req[*bar_count].bar.io	     = FALSE;
	bar_req[*bar_count].bar.prefetch     = FALSE;
	bar_req[*bar_count].bar.start        = scs10_start;
	bar_req[*bar_count].bar.size         = scs10_size;

	bar_req[*bar_count + 1].vendorid     = PCI_VENDID_GALILEO;
	bar_req[*bar_count + 1].devid	     = PCI_DEVID_64120;
	bar_req[*bar_count + 1].function     = 0;
	bar_req[*bar_count + 1].bar.pos	     = GT_BAR_SCS32_POS;
	bar_req[*bar_count + 1].bar.io	     = FALSE;
	bar_req[*bar_count + 1].bar.prefetch = FALSE;
	bar_req[*bar_count + 1].bar.start    = scs32_start;
	bar_req[*bar_count + 1].bar.size     = scs32_size;

        /*  Special requirement due to the fact that the Galileo
	 *  will not let us disable the internal registers memory
	 *  mapped BAR.
	 *  We map it where is does no harm (no overlap).
	 */
	bar_req[*bar_count + 2].vendorid     = PCI_VENDID_GALILEO;
	bar_req[*bar_count + 2].devid	     = PCI_DEVID_64120;
	bar_req[*bar_count + 2].function     = 0;
	bar_req[*bar_count + 2].bar.pos	     = GT_BAR_INTERNAL_MEM_POS;
	bar_req[*bar_count + 2].bar.io	     = FALSE;
	bar_req[*bar_count + 2].bar.prefetch = FALSE;
	bar_req[*bar_count + 2].bar.start    = scs32_start + scs32_size;
	bar_req[*bar_count + 2].bar.size     = 16;

	(*bar_count) += 3;

	return OK;

      /* Add new core cards here */

      default : /* Should never happen */
	return ERROR_PCI_STRUCTURE;
    }
}


/************************************************************************
 *
 *                          arch_pci_config_access
 *  Description :
 *  -------------
 *
 *  PCI configuration cycle (read or write)
 *
 *  Return values :
 *  ---------------
 *
 *  OK		    : If no error.
 *  ERROR_PCI_ABORT : If master abort (no target) or target abort.
 *
 ************************************************************************/
UINT32
arch_pci_config_access(
    UINT32 busnum,		/* PCI bus number (0 = local bus)	*/
    UINT32 devnum,		/* PCI device number			*/
    UINT32 func,		/* Function number of device		*/
    UINT32 reg,			/* Device register			*/
    bool   write,		/* TRUE -> Config write, else read	*/
    UINT8  size,		/* Sizeof data (1/2/4 bytes)		*/
    void   *data )		/* write or read data			*/
{
    UINT32 intr;
    UINT32 data32, align, pos;
    UINT32 rc;

    /* Always perform 32 bit access */
    if( size != sizeof(UINT32) )
    {
        /* Calc 32 bit aligned word */
        align = reg & ~MSK(2);

        /* Read word */
        rc = arch_pci_config_access(
                 busnum, devnum, func, align,
	         FALSE, sizeof(UINT32), (void *)&data32 );
    
        if( rc != OK ) 
	    return rc;

        switch( size )
        {
          case sizeof(UINT8)  :

	    /*  Calc position of byte within word 
	     *  (PCI is always little endian).
	     */
	    pos = (reg & MSK(2)) * 8;

            if( write )
	    {
	        /* Modify word */
	        data32 &= ~(MSK(8) << pos);
	        data32 |= *(UINT8 *)data << pos;
	    }
	    else
	    {
	        /* Calc byte */
	        *(UINT8 *)data = (data32 >> pos) & 0xFF;
            }

            break;

          case sizeof(UINT16) :

	    /*  Calc position of halfword within word 
	     *  (PCI is always little endian).
	     */
	    pos = (reg & (MSK(1) << 1)) * 8;

            if( write )
	    {
	        /* Modify word */
	        data32 &= ~(MSK(16) << pos);
	        data32 |= *(UINT16 *)data << pos;
	    }
	    else
	    {
	        /* Calc halfword */
	        *(UINT16 *)data = (data32 >> pos) & 0xFFFF;
            }

            break;
    
          default : /* Should not happen */
            break;
        }

	if( write )
	{
	    /* Write the modified word */
            rc = arch_pci_config_access(
                     busnum, devnum, func, align,
	             TRUE, sizeof(UINT32), (void *)&data32 );
        }

	return rc;
    }

    switch( sys_corecard )
    {
      case MIPS_REVISION_CORID_BONITO64 :
      case MIPS_REVISION_CORID_CORE_20K :
      case MIPS_REVISION_CORID_CORE_EMUL_20K :

        /**** Bonito64 system controller ****/

	if( busnum == PCI_BUS_LOCAL )
	{
	    if ( (devnum == 0) || (devnum > PCI_DEVNUM_MAX) )
	    {
	        return ERROR_PCI_ABORT;
	    }
	    if ( devnum == PCI_IDSEL2DEVNUM(ATLAS_IDSEL_CORE) )
	    {
		/* Access is to BONITO64 itself
		 * According to Algorithmics: Don't loop back via PCI,
		 * rather access Bonito's cfg registers from CPU side.
		 */
		if( write )
		{
		    /* Avoid writes to DeviceID and ClassCode regs */
		    if (reg != PCI_ID && reg != PCI_CCREV)
		    {
		    	BONITO_PCI_REG(reg) = *(UINT32 *)data;
		    }
		}
		else
		{
	 	   *(UINT32 *)data = BONITO_PCI_REG(reg);
		}
        	return OK;
	    }
	}

	/* Clear cause register bits */
	BONITO_PCICMD |= (BONITO_PCICMD_MABORT_CLR | BONITO_PCICMD_MTABORT_CLR);

	/* Setup pattern to be used as PCI "address" for Type 0	cycle */

	if( busnum == PCI_BUS_LOCAL )
	{
	    /* IDSEL */
            data32 = 1 << PCI_DEVNUM2IDSEL(devnum);
	}
	else	
	{
	    /* Bus number */
	    data32 = busnum << PCI_CFG_TYPE1_BUS_SHF;

	    /* Device number */
	    data32 |= devnum << PCI_CFG_TYPE1_DEV_SHF;
	}

	/* Function (same for Type 0/1) */
	data32 |= func << PCI_CFG_TYPE0_FUNC_SHF;

	/* Register number (same for Type 0/1) */
	data32 |= reg << PCI_CFG_TYPE0_REG_SHF;

	if( busnum == PCI_BUS_LOCAL )
	{
	    /* Type 0 */
	    BONITO_PCIMAP_CFG = data32 >> 16;
	}
	else
	{
	    /* Type 1 */
	    BONITO_PCIMAP_CFG = (data32 >> 16) | 0x10000;
	}
	  
	/* Flush Bonito register block */
	BONITO_PCIMAP_CFG;
	sys_sync();
	
	/* Perform access */
	if( write )
	{
	    REG32( KSEG1(BONITO_PCICFG_BASE) + (data32 & 0xFFFF) ) = 
	        *(UINT32 *)data;

	    /* Wait till done */
	    while( BONITO_PCIMSTAT & 0xF );
	}
	else
	{
	    *(UINT32 *)data = 
	        REG32( KSEG1(BONITO_PCICFG_BASE) + (data32 & 0xFFFF) );
	}

	/* Detect Master/Target abort */
        if( BONITO_PCICMD & (BONITO_PCICMD_MABORT_CLR | BONITO_PCICMD_MTABORT_CLR) )
        {
            /* Error occurred */

            /* Clear bits */
	    BONITO_PCICMD |= (BONITO_PCICMD_MABORT_CLR | BONITO_PCICMD_MTABORT_CLR);
    
            return ERROR_PCI_ABORT;
        }
        return OK;

      case MIPS_REVISION_CORID_QED_RM5261 :
      case MIPS_REVISION_CORID_CORE_LV :
      case MIPS_REVISION_CORID_CORE_FPGA :
      case MIPS_REVISION_CORID_CORE_FPGAr2 :

        /**** Galileo system controller ****/

        /* Workaround : Galileo seems to stall if requested to access
         *              device 31
         */ 
        if( (busnum == PCI_BUS_LOCAL) && (devnum == 31) )
            return ERROR_PCI_ABORT;

        /* Clear cause register bits */
        GT_W32( sys_nb_base, GT_INTRCAUSE_OFS, 
	        ~(GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT) );

        /* Setup address */
        GT_W32( sys_nb_base, GT_PCI0_CFGADDR_OFS,
	            (busnum     << GT_PCI0_CFGADDR_BUSNUM_SHF)   |
	            (devnum     << GT_PCI0_CFGADDR_DEVNUM_SHF)   |
	            (func       << GT_PCI0_CFGADDR_FUNCTNUM_SHF) |
	            ((reg >> 2) << GT_PCI0_CFGADDR_REGNUM_SHF)   |
	            GT_PCI0_CFGADDR_CONFIGEN_BIT );

        /* Perform the access */
        if( write )
            GT_W32( sys_nb_base, GT_PCI0_CFGDATA_OFS, *(UINT32 *)data );
        else
            GT_L32( sys_nb_base, GT_PCI0_CFGDATA_OFS, *(UINT32 *)data );
	
        /* Check for master or target abort */
        GT_L32( sys_nb_base, GT_INTRCAUSE_OFS, intr );

        if( intr & (GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT) )
        {
            /* Error occurred */

            /* Clear bits */
            GT_W32( sys_nb_base, GT_INTRCAUSE_OFS, 
	            ~(GT_INTRCAUSE_MASABORT0_BIT | GT_INTRCAUSE_TARABORT0_BIT) );
    
            return ERROR_PCI_ABORT;
        }
        return OK;

      case MIPS_REVISION_CORID_CORE_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA2 :
      case MIPS_REVISION_CORID_CORE_EMUL_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA3 :
      case MIPS_REVISION_CORID_CORE_24K :

        /**** MIPS system controller ****/

        /* Clear cause register bits */
        REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTSTAT) =
	        MSC01_PCI_INTSTAT_MA_BIT | MSC01_PCI_INTSTAT_TA_BIT;

        /* Setup address */
        REG(MSC01_PCI_REG_BASE, MSC01_PCI_CFGADDR) =
	            (busnum     << MSC01_PCI_CFGADDR_BNUM_SHF) |
	            (devnum     << MSC01_PCI_CFGADDR_DNUM_SHF) |
	            (func       << MSC01_PCI_CFGADDR_FNUM_SHF) |
	            ((reg >> 2) << MSC01_PCI_CFGADDR_RNUM_SHF);
	sys_sync();
        /* Perform the access */
        if( write )
	{
            REG(MSC01_PCI_REG_BASE, MSC01_PCI_CFGDATA) = *(UINT32 *)data;
	    sys_sync();
	}
        else
            *(UINT32 *)data = REG(MSC01_PCI_REG_BASE, MSC01_PCI_CFGDATA);
	
        /* Check for master or target abort */
        intr = REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTSTAT);

        if( intr & (MSC01_PCI_INTSTAT_MA_BIT | MSC01_PCI_INTSTAT_TA_BIT) )
        {
            /* Error occurred */

            /* Clear bits */
            REG(MSC01_PCI_REG_BASE, MSC01_PCI_INTSTAT) =
	            MSC01_PCI_INTSTAT_MA_BIT | MSC01_PCI_INTSTAT_TA_BIT ;
    
            return ERROR_PCI_ABORT;
        }
        return OK;
	
      /* Add new core cards here */

      default : /* Should never happen */

	return ERROR_PCI_ABORT;
    }
}


/************************************************************************
 *
 *                          arch_pci_lattim
 *  Description :
 *  -------------
 *
 *  Latency timer value to be written to PCI device (Max_Lat)
 *
 *  Return values :
 *  ---------------
 *
 *  Value to be written to Max_Lat.
 *
 ************************************************************************/
UINT8
arch_pci_lattim( 
    t_pci_cfg_dev *dev )	/* PCI device structure			*/
{
    /*  Determine latency timer
     *  Special handling of Galileo System Controller.
     */

    return 
       ((dev->vendorid == PCI_VENDID_GALILEO ) && (dev->devid == PCI_DEVID_64120)) ?
            MAX( dev->min_gnt, MAX(GT_LATTIM_MIN, PCI_LATTIM_FIXED) ) :
            MAX( dev->min_gnt, PCI_LATTIM_FIXED );
}


/************************************************************************
 *
 *                          arch_pci_multi
 *  Description :
 *  -------------
 *
 *  Extract multi field from PCI configuration word 0xc
 *  (Word holding Bist/Header Type/Latency Timer/Cache Line Size)
 *
 *  Return values :
 *  ---------------
 *
 *  multi field
 *
 ************************************************************************/
UINT8
arch_pci_multi( 
    t_pci_cfg_dev *dev,
    UINT32	  bhlc )
{
    /*  Special handling of Galileo System Controller :
     *  It is a multi function device, but we only use function 0.
     */
    if( (dev->vendorid == PCI_VENDID_GALILEO ) &&
        (dev->devid    == PCI_DEVID_64120) )
    {
        return 0;
    }
    else
        return (bhlc & PCI_BHLC_MULTI_MSK) >> PCI_BHLC_MULTI_SHF;
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          config_bonito64
 ************************************************************************/
static UINT32
config_bonito64( void )
{
    UINT32 cmd;
    UINT32 line;
    UINT32 mask, trans;
  
    /* Map PCI addresses transparently */

    mask  =  ~((sys_ramsize >> (BONITO_PCIMEMBASECFG_ASHIFT+1)) - 1);
    trans =  ~mask + 1;

    mask  &= (BONITO_PCIMEMBASECFG_MEMBASE0_MASK >> 
	      BONITO_PCIMEMBASECFG_MEMBASE0_MASK_SHIFT);

    trans &= (BONITO_PCIMEMBASECFG_MEMBASE0_TRANS >> 
	      BONITO_PCIMEMBASECFG_MEMBASE0_TRANS_SHIFT);

    BONITO_PCIMEMBASECFG =
        (mask  << BONITO_PCIMEMBASECFG_MEMBASE0_MASK_SHIFT)  | 
	(0     << BONITO_PCIMEMBASECFG_MEMBASE0_TRANS_SHIFT) | 
        (mask  << BONITO_PCIMEMBASECFG_MEMBASE1_MASK_SHIFT)  |
	(trans << BONITO_PCIMEMBASECFG_MEMBASE1_TRANS_SHIFT) |
        (sys_cpu_cache_coherency ?
	    (BONITO_PCIMEMBASECFG_MEMBASE0_CACHED |
	     BONITO_PCIMEMBASECFG_MEMBASE1_CACHED) : 0);

    /*  If CPU is big endian, we need to handle endianness due to
     *  PCI being little endian.
     *
     *  We have chosen the following setup of Bonito64 
     *  (through BONGENCFG register) :
     *
     *  PCI device doing DMA :
     *
     *      Bonito64 swaps byte lanes.
     *
     *  CPU accesses PCI device :
     *
     *      Bonito64 does NOT swap byte lanes. 
     *
     *      This means that software must do the following :
     *
     *      Word access     : Do nothing.
     *      Halfword access : Software must invert bit 1 of address.
     *      Byte access     : Software must invert 2 msb of address.
     */

    /*  Enable IOBC and configure byteswapping */
    BONITO_BONGENCFG =
        BONITO_BONGENCFG	  |
#ifdef EB
        BONITO_BONGENCFG_BYTESWAP |
#endif
        BONITO_BONGENCFG_UNCACHED |
	BONITO_BONGENCFG_SNOOPEN  |
	(sys_cpu_cache_coherency ?
	    (BONITO_BONGENCFG_WBEHINDEN | BONITO_BONGENCFG_PREFETCHEN) :
	    0);

    /* Invalidate lines */
    for( line = 0; line < 4; line ++ )
    {
        cmd = (0    << BONITO_PCICACHECTRL_CACHECMD_SHIFT)     |
	      (line << BONITO_PCICACHECTRL_CACHECMDLINE_SHIFT);

        BONITO_PCICACHECTRL = cmd;
        BONITO_PCICACHECTRL = cmd | BONITO_PCICACHECTRL_CMDEXEC;

        while(BONITO_PCICACHECTRL & BONITO_PCICACHECTRL_CMDEXEC)
        {
            ;
        }

        BONITO_PCICACHECTRL = cmd;
    }

    return OK;
}


/************************************************************************
 *                          config_gt64120
 ************************************************************************/
static UINT32
config_gt64120( void )
{
    UINT32 rc;
    UINT32 data, bare;

    /**** Setup GT64120 BAR ****/

    /* Disable all BARs except SCS30 and SCS10 */
    bare = 
        GT_PCI0_BARE_SWSCS3BOOTDIS_BIT |
	GT_PCI0_BARE_SWSCS32DIS_BIT    |
	GT_PCI0_BARE_SWSCS10DIS_BIT    |
	GT_PCI0_BARE_INTIODIS_BIT      |
	GT_PCI0_BARE_INTMEMDIS_BIT     |
	GT_PCI0_BARE_CS3BOOTDIS_BIT    |
	GT_PCI0_BARE_CS20DIS_BIT;

    /* Check range and calc mask for GT bank size registers */

    if( scs10_size != 0 )
    {
        rc = pci_check_range( scs10_start, scs10_size, &data );
        if( rc != OK ) return rc;
        /* Write BAR size register */
        GT_W32( sys_nb_base, GT_PCI0_BS_SCS10_OFS, data );
    }
    else
        bare |= GT_PCI0_BARE_SCS10DIS_BIT;

    if( scs32_size != 0 )
    {
        rc = pci_check_range( scs32_start, scs32_size, &data );
        if( rc != OK ) return rc;
        /* Write BAR size register */
        GT_W32( sys_nb_base, GT_PCI0_BS_SCS32_OFS, data );
    }
    else
        bare |= GT_PCI0_BARE_SCS32DIS_BIT;

    GT_W32( sys_nb_base, GT_PCI0_BARE_OFS, bare );

    return OK;
}


/************************************************************************
 *                          gt_calc_range
 ************************************************************************/
static void
gt_calc_range( 
    UINT32 ld,
    UINT32 hd,
    UINT32 *start,
    UINT32 *size )
{
    UINT32 end;

    GT_L32( sys_nb_base, ld, *start );
    GT_L32( sys_nb_base, hd, end );

    /* TBD : Remove "magic" numbers */
    *start = (*start & 0x7FFF)  << 21;
    end    = ((end & 0x7F) + 1) << 21;

    *size  = (end > *start) ? end - *start : 0;
}


/************************************************************************
 *                          config_sysctrl
 ************************************************************************/
static UINT32
config_sysctrl( void )
{
    /*  If CPU is big endian, we need to handle endianness due to
     *  PCI being little endian.
     *
     *  We have chosen the following setup of MSC01
     *
     *  PCI device doing DMA :
     *      MSC01 swaps byte lanes.
     *
     *  CPU accesses PCI device :
     *      MSC01 does NOT swap byte lanes. 
     *
     *      This means that software must do the following :
     *
     *      Word access     : Do nothing.
     *      Halfword access : Software must invert bit 1 of address.
     *      Byte access     : Software must invert 2 msb of address.
     *
     *  Above setup has been completed in msc01_core.S
     */

    /* Set up changes since early setup in msc01_core.S */
    REG(MSC01_PCI_REG_BASE, MSC01_PCI_BAR0) = -sc_calc_pwr2(sys_ramsize);

    return OK;
}


/************************************************************************
 *                          sc_calc_pwr2ramsize
 *  Calculate least size in power of 2 that can hold arg
 ************************************************************************/
static UINT32
sc_calc_pwr2( UINT32 arg )
{
    UINT32 mysize;

    for ( mysize = 1; mysize < arg; mysize <<= 1) ;
    return mysize;
}
