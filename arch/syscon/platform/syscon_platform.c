
/************************************************************************
 *
 *  syscon_platform.c
 *
 *  Platform specific parts of SYSCON module (except TTY related parts)
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
#include <sysdev.h>
#include <syscon_api.h>
#include <syscon.h>
#include <sys_api.h>
#include <string.h>
#include <excep_api.h>
#include <pci_api.h>
#include <io_api.h>
#include <eeprom_nm24c09_layout.h>
#include <eeprom_api.h>

#include <spd.h>

#include <product.h>
#include <atlas.h>
#include <sead.h>
#include <malta.h>

#include <tmra.h>   /* Atlas specific   */
#include <icta.h>   /* Atlas specific   */
#include <piix4.h>  /* Malta specific */

/************************************************************************
 *  Definitions
 ************************************************************************/

#define WORD0_OFS    0x00000       /* Word0:                       */
#define WORD1_OFS    0x00004       /* Word1:                       */

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Names of boards/RTL */
static char	          *name_atlas      = "Atlas";
static char		  *name_sead       = "SEAD";
static char		  *name_sead2      = "SEAD-2";
static char		  *name_malta      = "Malta";
static char		  *name_basic_rtl  = "Basic RTL";

/* Pointer to array of objects */
static t_syscon_obj       *syscon_objects;

/* Size of ASCII display (if any) on platform */
static UINT32		  alpha_display_size;
/* Addresses of individual ASCII characters on ASCII display */
static volatile UINT32    *alpha_io[8];

/* Mapping between hex digit and ASCII character (e.g 0-> ASCII(0x30)) */
static const UINT8        bin2char[16] = { 0x30, 0x31, 0x32, 0x33, 
					   0x34, 0x35, 0x36, 0x37,
					   0x38, 0x39, 0x41, 0x42,
					   0x43, 0x44, 0x45, 0x46  };

/* Register size */
static UINT8  size8  = sizeof(UINT8);
static UINT8  size32 = sizeof(UINT32);

/* MAC address and Serial Number */
static UINT8       amd_mac_addr[8];
static t_sn_bcd    sn_buffer;
static t_mac_addr  mac_addr_buffer;
static t_mac_addr  *mac_addr_read = NULL;
static t_sn_bcd	   *sn_read       = NULL;

/* EEPROM definitions */
static UINT8	           eeprom_count;
static t_syscon_eeprom_def eeprom_def[2];

static char *eeprom_system_name  = "sys";
static char *eeprom_system_descr = "Device for storing system data";

static char *eeprom_spd0_name    = "spd0";
static char *eeprom_spd0_descr   = "Device used for SDRAM parameters (SPD)";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static INT32 
eeprom_read( 
    UINT32  minor,		/* Minor device number			*/
    UINT32  offset,		/* Byte position			*/
    UINT32  length,		/* Byte count				*/
    void    *user_variable );   /* Buffer for data			*/


/************************************************************************
 *  Implementation : Static functions registered for handling particular 
 *  SYSCON objects for particular platform(s)
 ************************************************************************/



/************************************************************************
 *  board_productid_generic_read
 ************************************************************************/
static UINT32
board_productid_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  Determine PRODUCT ID using MIPS_REVISION register
     *  (special hw address reserved on Atlas, Malta, SEAD, SEAD-2 
     *   boards).
     */

    *(UINT32 *)param = (REGP(KSEG1BASE, MIPS_REVISION)
                                      & MIPS_REVISION_PROID_MSK)
                                     >> MIPS_REVISION_PROID_SHF;
    return OK;
}


/************************************************************************
 *  board_productrev_generic_read
 ************************************************************************/
static UINT32
board_productrev_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  Determine PRODUCT ID using MIPS_REVISION register
     *  (special hw address reserved on Atlas, Malta, SEAD, SEAD-2 
     *   boards).
     */

    *(UINT32 *)param = (REGP(KSEG1BASE, MIPS_REVISION)
                                      & MIPS_REVISION_PRORV_MSK)
                                     >> MIPS_REVISION_PRORV_SHF;
    return OK;
}


/************************************************************************
 *  board_revision_generic_read
 ************************************************************************/
static UINT32
board_revision_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  Determine PRODUCT ID using MIPS_REVISION register
     *  (special hw address reserved on Atlas, Malta, SEAD, SEAD-2 
     *   boards).
     */

    *(UINT32 *)param = REGP(KSEG1BASE, MIPS_REVISION);
    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_sead_read
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_TREFRESH));

    *(UINT32 *)param = CYCLES2NS( REGFIELD(regval, SEAD_SD_TREFRESH_TREF) );

    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_sead_write
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 refcount;
  
    NS2COUNT_ROUND_DOWN( *(UINT32 *)param, refcount );

    REG32(KSEG1(SEAD_SD_TREFRESH)) = refcount;

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_sead_read
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_LATENCIES));

    *(UINT32 *)param = REGFIELD(regval, SEAD_SD_LATENCIES_TRP);

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_sead_write
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval, trp;

    regval = REG32(KSEG1(SEAD_SD_LATENCIES));

    if( *(UINT32 *)param < SEAD_SD_LATENCIES_TRP_MIN )
        trp = SEAD_SD_LATENCIES_TRP_MIN;
    else if( *(UINT32 *)param > SEAD_SD_LATENCIES_TRP_MAX )
        trp = SEAD_SD_LATENCIES_TRP_MAX;
    else
        trp = *(UINT32 *)param;
      
    regval &= ~SEAD_SD_LATENCIES_TRP_MSK;
    regval |= (trp << SEAD_SD_LATENCIES_TRP_SHF);

    REG32(KSEG1(SEAD_SD_LATENCIES)) = regval;
    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_sead_read  
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_LATENCIES));

    *(UINT32 *)param = REGFIELD(regval, SEAD_SD_LATENCIES_TRCD);

    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_sead_write  
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval, trcd;

    regval = REG32(KSEG1(SEAD_SD_LATENCIES));

    if( *(UINT32 *)param < SEAD_SD_LATENCIES_TRCD_MIN )
        trcd = SEAD_SD_LATENCIES_TRCD_MIN;
    else if( *(UINT32 *)param > SEAD_SD_LATENCIES_TRCD_MAX )
        trcd = SEAD_SD_LATENCIES_TRCD_MAX;
    else
        trcd = *(UINT32 *)param;
      
    regval &= ~SEAD_SD_LATENCIES_TRCD_MSK;
    regval |= (trcd << SEAD_SD_LATENCIES_TRCD_SHF);

    REG32(KSEG1(SEAD_SD_LATENCIES)) = regval;
    return OK;
}


/************************************************************************
 *    board_systemram_caslat_cycles_sead_read
 ************************************************************************/
static UINT32
board_systemram_caslat_cycles_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_MRSCODE));

    *(UINT32 *)param = REGFIELD(regval, SEAD_SD_MRSCODE_CL);

    return OK;
}


/************************************************************************
 *    board_systemram_read_burstlen_sead_read
 ************************************************************************/
static UINT32
board_systemram_read_burstlen_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_MRSCODE));

    switch( REGFIELD(regval, SEAD_SD_MRSCODE_BL ) )
    {
      case 0 :
        *(UINT32 *)param = 1;
	break;
      case 1 :
        *(UINT32 *)param = 2;
	break;
      case 2 :
        *(UINT32 *)param = 4;
	break;
      case 3 :
        *(UINT32 *)param = 8;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_write_burstlen_sead_read
 ************************************************************************/
static UINT32
board_systemram_write_burstlen_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    regval = REG32(KSEG1(SEAD_SD_MRSCODE));

    switch( REGFIELD(regval, SEAD_SD_MRSCODE_WBL) )
    {
      case 0 :
        return board_systemram_read_burstlen_sead_read(
	           param, data, size );
      case 1 :
        *(UINT32 *)param = 1;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *  com_en0_major_device_atlas_read
 ************************************************************************/
static UINT32
com_en0_major_device_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
   /*  On Atlas, the Philips SAA9730 IO controller, implements
    *  the LAN device; however 10 MBit/s only          
    */
    *(UINT32 *)param = SYS_MAJOR_LAN_SAA9730;
    return OK;
}


/************************************************************************
 *  com_en0_major_device_malta_read
 ************************************************************************/
static UINT32
com_en0_major_device_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
   /*  On Malta, the AMD 79C973 controller, implements
    *  the LAN device; speed and duplex mode are autonegotiated 
    */
    *(UINT32 *)param = SYS_MAJOR_LAN_AM79C973;
    return OK;
}

 
/************************************************************************
 *  com_en0_intline_atlas_read
 ************************************************************************/
static UINT32
com_en0_intline_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Return interrupt line used for EN0 on Atlas interrupt controller */
     *(UINT32 *)param = ICTA_INTSTATUS_CONINTBN_SHF;
     return OK;
}

 
/************************************************************************
 *  com_en0_intline_malta_read
 ************************************************************************/
static UINT32
com_en0_intline_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Return interrupt line used for EN0 on Malta interrupt controller */
    *(UINT32 *)param = MALTA_INTLINE_79C973;
    return OK;
}


/************************************************************************
 *  board_productname_read
 ************************************************************************/
static UINT32
board_productname_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(char **)param = (char *)data;
    return OK;
}

/************************************************************************
 *  board_nmi_werr_atlas_malta_read
 ************************************************************************/
static UINT32
board_nmi_werr_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 0;  /* no WERR bit on Atlas */
    return OK;
}


/************************************************************************
 *  board_nmi_werr_sead_read
 ************************************************************************/
static UINT32
board_nmi_werr_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REGP(KSEG1BASE, SEAD_NMISTATUS)
                                      & SEAD_NMISTATUS_WERR_MSK)
                                     >> SEAD_NMISTATUS_WERR_SHF;
    return OK;
}


/************************************************************************
 *  board_nmi_ack_atlas_malta_write
 ************************************************************************/
static UINT32
board_nmi_ack_atlas_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, MALTA_NMIACK) = MALTA_NMIACK_ONNMI_BIT;
    EXCEP_nmi = FALSE;  /* clear local copy of nmi status */
    return OK;
}


/************************************************************************
 *  board_nmi_ack_sead_write
 ************************************************************************/
static UINT32
board_nmi_ack_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, SEAD_NMIACK) = SEAD_NMIACK_ACK_BIT;
    EXCEP_nmi = FALSE;  /* clear local copy of nmi status */
    return OK;
}


/************************************************************************
 *  board_nmi_status_generic_read
 ************************************************************************/
static UINT32
board_nmi_status_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = EXCEP_nmi;
    return OK;
}


/************************************************************************
 *  board_corecardid_atlas_malta_read
 ************************************************************************/
static UINT32
board_corecardid_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Shared between Atlas and Malta */
    *(UINT32 *)param = (REGP(KSEG1BASE, ATLAS_REVISION)
                                      & ATLAS_REVISION_CORID_MSK)
                                     >> ATLAS_REVISION_CORID_SHF;
    return OK;
}


/************************************************************************
 *  board_corecardrev_atlas_malta_read
 ************************************************************************/
static UINT32
board_corecardrev_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Shared between Atlas and Malta */
    *(UINT32 *)param = (REGP(KSEG1BASE, ATLAS_REVISION)
                                      & ATLAS_REVISION_CORRV_MSK)
                                     >> ATLAS_REVISION_CORRV_SHF;
    return OK;
}


/************************************************************************
 *  board_fpgarev_atlas_malta_read
 ************************************************************************/
static UINT32
board_fpgarev_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Shared between Atlas and Malta */
    *(UINT32 *)param = (REGP(KSEG1BASE, ATLAS_REVISION)
                                      & ATLAS_REVISION_FPGRV_MSK)
                                     >> ATLAS_REVISION_FPGRV_SHF;
    return OK;
}


/************************************************************************
 *  board_rtl_sead_read
 ************************************************************************/
static UINT32
board_rtl_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REGP(KSEG1BASE, SEAD_REVISION)
                                      & SEAD_REVISION_RTLID_MSK)
                                     >> SEAD_REVISION_RTLID_SHF;
    return OK;
}


/************************************************************************
 *  board_rtl_name_sead_read
 ************************************************************************/
extern char *name_msc01;
static UINT32
board_rtl_name_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    switch( (REGP(KSEG1BASE,SEAD_REVISION)
                          & SEAD_REVISION_RTLID_MSK)
                         >> SEAD_REVISION_RTLID_SHF )
    {
      case SEAD_REVISION_RTLID_BASIC :
	*(char **)param = name_basic_rtl;
        break;
      case SEAD_REVISION_RTLID_SOCIT101 :
	*(char **)param = name_msc01;
        break;
      default :
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *  board_rtlrev_major_sead_read
 ************************************************************************/
static UINT32
board_rtlrev_major_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
    {
        *(UINT32 *)param = (REG(MSC01_BIU_REG_BASE, MSC01_SC_ID)
	                                          & MSC01_SC_ID_MAR_MSK)
	                                         >> MSC01_SC_ID_MAR_SHF;
        return OK;
    }
    *(UINT32 *)param = (REGP(KSEG1BASE, SEAD_REVISION)
                                      & SEAD_REVISION_RTLMAJ_MSK)
                                     >> SEAD_REVISION_RTLMAJ_SHF;
    return OK;
}


/************************************************************************
 *  board_rtlrev_minor_sead_read
 ************************************************************************/
static UINT32
board_rtlrev_minor_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
    {
        *(UINT32 *)param = (REG(MSC01_BIU_REG_BASE, MSC01_SC_ID)
	                                          & MSC01_SC_ID_MIR_MSK)
	                                         >> MSC01_SC_ID_MIR_SHF;
        return OK;
    }
    *(UINT32 *)param = (REGP(KSEG1BASE, SEAD_REVISION)
                                      & SEAD_REVISION_RTLMIN_MSK)
                                     >> SEAD_REVISION_RTLMIN_SHF;
    return OK;
}


/************************************************************************
 *  board_use_default_atlas_malta_read
 ************************************************************************/
static UINT32
board_use_default_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Read S5-4 switch */
    *(UINT32 *)param = (REGP( KSEG1BASE, ATLAS_STATUS)
                                       & ATLAS_STATUS_S54_MSK)
                                      >> ATLAS_STATUS_S54_SHF;
    return OK;
}


/************************************************************************
 *  board_use_default_sead_read
 ************************************************************************/
static UINT32
board_use_default_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* No default switch */
    *(UINT32 *)param = 0;
    return OK;
}


/************************************************************************
 *  board_softres_atlas_malta_write
 ************************************************************************/
static UINT32
board_softres_atlas_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_SOFTRES) = ATLAS_SOFTRES_RESET_GORESET;
    return OK;
}


/************************************************************************
 *  board_softres_sead2_write
 ************************************************************************/
static UINT32
board_softres_sead2_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* SEAD-2 only */
    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	REG(MSC01_MC_REG_BASE, MSC01_MC_ARB_CFG) = SEAD2_MSC01_MC_RESET_BIT;
    else
        REGP(KSEG1BASE, SEAD2_SWRESET) = SEAD2_SWRESET_SWRST_GORESET;
    return OK;
}


/************************************************************************
 *  board_systemflash_base_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_base_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_SYSTEMFLASH_BASE;
    return OK;
}


/************************************************************************
 *  board_systemflash_base_sead_read
 ************************************************************************/
static UINT32
board_systemflash_base_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_SYSTEMFLASH_BASE;
    return OK;
}


/************************************************************************
 *  board_systemflash_size_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_size_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_SYSTEMFLASH_SIZE;
    return OK;
}


/************************************************************************
 *  board_systemflash_size_sead_read
 ************************************************************************/
static UINT32
board_systemflash_size_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SYSTEMFLASH_SIZE;
    return OK;
}


/************************************************************************
 *  board_monitorflash_base_atlas_malta_read
 ************************************************************************/
static UINT32
board_monitorflash_base_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_MONITORFLASH_BASE;
    return OK;
}


/************************************************************************
 *  board_monitorflash_size_atlas_malta_read
 ************************************************************************/
static UINT32
board_monitorflash_size_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_MONITORFLASH_SIZE;
    return OK;
}


/************************************************************************
 *  board_fileflash_base_atlas_malta_read
 ************************************************************************/
static UINT32
board_fileflash_base_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_FILEFLASH_BASE;
    return OK;
}


/************************************************************************
 *  board_fileflash_base_sead_read
 ************************************************************************/
static UINT32
board_fileflash_base_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_FILEFLASH_BASE;
    return OK;
}


/************************************************************************
 *  board_fileflash_size_atlas_malta_read
 ************************************************************************/
static UINT32
board_fileflash_size_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_FILEFLASH_SIZE;
    return OK;
}


/************************************************************************
 *  board_fileflash_size_sead_read
 ************************************************************************/
static UINT32
board_fileflash_size_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_FILEFLASH_SIZE;
    return OK;
}


/************************************************************************
 *  board_systemram_base_sead_read
 ************************************************************************/
static UINT32
board_systemram_base_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_SYSTEMRAM_BASE;
    return OK;
}


/************************************************************************
 *  board_systemram_size_sead_read
 ************************************************************************/
static UINT32
board_systemram_size_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SYSTEMRAM_SIZE;
    return OK;
}


/************************************************************************
 *  board_sram_base_sead_read
 ************************************************************************/
static UINT32
board_sram_base_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_SRAM_BASE;
    return OK;
}


/************************************************************************
 *  board_sram_size_sead_read
 ************************************************************************/
static UINT32
board_sram_size_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SRAM_SIZE;
    return OK;
}


/************************************************************************
 *  board_gt64120_base_atlas_malta_read
 ************************************************************************/
static UINT32
board_gt64120_base_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_GT64120_BASE;
    return OK;
}


/************************************************************************
 *  board_saa9730_base_atlas_read
 ************************************************************************/
static UINT32
board_saa9730_base_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  Philips SAA9730 implements one function only with all IO-devices
     *   memory mapped into one contigous memory.
     *   According to spec. the BAR#1 is to be used !
     */

    if( pci_lookup_bar( PCI_BUS_LOCAL,
			   ATLAS_DEVNUM_SAA9730,
			   0,
			   PCI_BAR(1),
			   (void **)param ) )
    {
        return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *  board_icta_base_atlas_read
 ************************************************************************/
static UINT32
board_icta_base_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_ICTA_BASE;
    return OK;
}


/************************************************************************
 *  board_tmra_base_atlas_read
 ************************************************************************/
static UINT32
board_tmra_base_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_TMRA_BASE;
    return OK;
}


/************************************************************************
 *  board_ledgreen_atlas_read
 ************************************************************************/
static UINT32
board_ledgreen_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = REGP(KSEG1BASE, ATLAS_LEDGREEN);
    return OK;
}


/************************************************************************
 *  board_ledgreen_atlas_write
 ************************************************************************/
static UINT32
board_ledgreen_atlas_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_LEDGREEN) = *(UINT32 *)param;
    return OK;
}


/************************************************************************
 *  board_ledbar_atlas_malta_read
 ************************************************************************/
static UINT32
board_ledbar_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = REGP(KSEG1BASE, ATLAS_LEDBAR);
    return OK;
}


/************************************************************************
 *  board_ledbar_atlas_malta_write
 ************************************************************************/
static UINT32
board_ledbar_atlas_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_LEDBAR) = *(UINT32 *)param;
    return OK;
}


/************************************************************************
 *  board_ledbar_sead_read
 ************************************************************************/
static UINT32
board_ledbar_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = REGP(KSEG1BASE, SEAD_PLED);
    return OK;
}


/************************************************************************
 *  board_ledbar_sead_write
 ************************************************************************/
static UINT32
board_ledbar_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, SEAD_PLED) = *(UINT32 *)param;
    return OK;
}


/************************************************************************
 *  board_asciiword_atlas_malta_write
 ************************************************************************/
static UINT32
board_asciiword_atlas_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_ASCIIWORD) = *(UINT32 *)param;
    return OK;
}


/************************************************************************
 *  board_asciiword_sead_write
 ************************************************************************/
static UINT32
board_asciiword_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 tl, sl, i;

    sl = 8 ;

    if( alpha_display_size < 8 )
    {
        sl = alpha_display_size;
    }

    for(i=0; i<sl; i++)
    {
        tl = ( (*(UINT32 *)param) >> (28-(i*4)) ) & 0xf;
        tl = bin2char[ tl ];
        *alpha_io[ i ] = tl;
    }
 
    return OK;
}


/************************************************************************
 *  board_asciichar_write
 ************************************************************************/
static UINT32
board_asciichar_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 tl;
    UINT8  *ptb;

    tl  = ((t_sys_alphanumeric *)param)->posid;
    ptb = ((t_sys_alphanumeric *)param)->string;

    if ( tl < alpha_display_size )
    {
        *(alpha_io[tl]) = *ptb;
	return OK;
    }
    else
    {
        return ERROR_SYSCON_INVALID_CHAR_POS;
    }
}


/************************************************************************
 *  board_asciistring_write
 ************************************************************************/
static UINT32
board_asciistring_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 tl;
    UINT8  *ptb;

    tl  = ((t_sys_alphanumeric *)param)->posid;
    ptb = ((t_sys_alphanumeric *)param)->string;
    
    /* display string */
    while( tl < alpha_display_size )
    {
        if ( *ptb == 0 )
        {
            break;
        }
 
       *(alpha_io[tl]) = (*ptb) & 0x7f;
       tl++;
       ptb++;
    }
    
    /* check for string termination at max. char. pos. */
    if ( *ptb == 0 )
    {
        /* blank the rest */
        while ( tl < alpha_display_size )
        {
            *(alpha_io[ tl ]) = SP;
            tl++;
        }

        return OK;
    }
    else
    {
        return ERROR_SYSCON_INVALID_CHAR_POS;
    }
}


/************************************************************************
 *  millisec
 ************************************************************************/
static UINT32
millisec(
    UINT32 cpufreq_hz )
{
    UINT32 calc, cycle_per_count, count;

    if( cpufreq_hz == 0 )
        return 0;

    SYSCON_read( SYSCON_CPU_CYCLE_PER_COUNT_ID,
                 (void *)&cycle_per_count,
                 sizeof(UINT32) );

    SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		 (void *)&count,
		 sizeof(UINT32) );	

    calc = cpufreq_hz / ( 1000 * cycle_per_count );

    if( calc != 0 )
    {
	calc = count / calc;
    }
    else
    {
        calc = count * 1000 * cycle_per_count / cpufreq_hz;
    }

    return calc;
}


/************************************************************************
 *  board_get_millisec_atlas_malta_read
 ************************************************************************/
static UINT32
board_get_millisec_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = millisec( sys_cpufreq_hz );
    return OK;
}


/************************************************************************
 *  board_get_millisec_sead_read
 ************************************************************************/
static UINT32
board_get_millisec_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = millisec( sys_cpufreq_hz ?
				     sys_cpufreq_hz :
				     SEAD_MAX_FREQ_MHZ * 1000000 );
    return OK;
}


/************************************************************************
 *    board_piix4_smb_base_malta_read  
 ************************************************************************/
static UINT32
board_piix4_smb_base_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  PIIX4 SMB registers
     *  are memory mapped into SMB Bar
     */

    if( pci_lookup_bar( PCI_BUS_LOCAL,
			   MALTA_DEVNUM_PIIX4,
			   PIIX4_PCI_FUNCTION_POWER,
			   PIIX4_PCI_BAR_SMB_POS,
			   (void **)param ) )
    {
        return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *    board_piix4_power_base_malta_read  
 ************************************************************************/
static UINT32
board_piix4_power_base_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  PIIX4 Power Management registers
     *  are memory mapped into POWER Bar
     */

    if( pci_lookup_bar( PCI_BUS_LOCAL,
			   MALTA_DEVNUM_PIIX4,
			   PIIX4_PCI_FUNCTION_POWER,
			   PIIX4_PCI_BAR_POWER_POS,
			   (void **)param ) )
    {
        return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *  board_systemflash_sectorsize_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_sectorsize_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_SYSTEMFLASH_SECTORSIZE;
    return OK;
}

/************************************************************************
 * board_monitorflash_sectorsize_atlas_read 
 ************************************************************************/
static UINT32
board_monitorflash_sectorsize_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_MONITORFLASH_SECTORSIZE;
    return OK;
}



/************************************************************************
 * board_monitorflash_sectorsize_malta_read 
 ************************************************************************/
static UINT32
board_monitorflash_sectorsize_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = MALTA_MONITORFLASH_SECTORSIZE;
    return OK;
}



/************************************************************************
 *  board_systemflash_sectorsize_sead_read
 ************************************************************************/
static UINT32
board_systemflash_sectorsize_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SYSTEMFLASH_SECTORSIZE;
    return OK;
}


/************************************************************************
 *  board_fileflash_sectorsize_atlas_malta_read
 ************************************************************************/
static UINT32
board_fileflash_sectorsize_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_FILEFLASH_SECTORSIZE;
    return OK;
}


/************************************************************************
 *  board_fileflash_sectorsize_sead_read
 ************************************************************************/
static UINT32
board_fileflash_sectorsize_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_FILEFLASH_SECTORSIZE;
    return OK;
}


/************************************************************************
 *  board_systemflash_bankcount_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_bankcount_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_SYSTEMFLASH_BANKCOUNT;
    return OK;
}


/************************************************************************
 *  board_systemflash_bankcount_sead_read
 ************************************************************************/
static UINT32
board_systemflash_bankcount_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SYSTEMFLASH_BANKCOUNT;
    return OK;
}


/************************************************************************
 *  board_systemflash_blockcount_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_blockcount_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_SYSTEMFLASH_BLOCKCOUNT;
    return OK;
}


/************************************************************************
 *  board_systemflash_blockcount_sead_read
 ************************************************************************/
static UINT32
board_systemflash_blockcount_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = SEAD_SYSTEMFLASH_BLOCKCOUNT;
    return OK;
}


/************************************************************************
 *  board_systemflash_write_enable_atlas_write
 ************************************************************************/
static UINT32
board_systemflash_write_enable_atlas_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_SFWCTRL) = ATLAS_SFWCTRL_WRENA_ENSFWRITE;
    return OK;
}


/************************************************************************
 *  board_systemflash_write_enable_sead_write
 ************************************************************************/
static UINT32
board_systemflash_write_enable_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Nothing to do */
    return OK;
}


/************************************************************************
 *  board_systemflash_write_disable_atlas_write
 ************************************************************************/
static UINT32
board_systemflash_write_disable_atlas_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_SFWCTRL) = 0;
    return OK;
}


/************************************************************************
 *  board_systemflash_write_disable_sead_write
 ************************************************************************/
static UINT32
board_systemflash_write_disable_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Nothing to do */
    return OK;
}


/************************************************************************
 *  board_brkres_atlas_malta_read
 ************************************************************************/
static UINT32
board_brkres_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = REGP(KSEG1BASE, ATLAS_BRKRES) & 0xff;
    return OK;
}


/************************************************************************
 *  board_brkres_atlas_malta_write
 ************************************************************************/
static UINT32
board_brkres_atlas_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_BRKRES) = *(UINT32*)param;
    return OK;
}


/************************************************************************
 *  board_systemflash_write_protected_atlas_read
 ************************************************************************/
static UINT32
board_systemflash_write_protected_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32*)param = 
        (REGP(KSEG1BASE, ATLAS_SWVALUE) &  ATLAS_SWVALUE_S13_MSK) ?
	    1 : 0;

    return OK;
}


/************************************************************************
 *  board_systemflash_write_protected_sead_read
 ************************************************************************/
static UINT32
board_systemflash_write_protected_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 0;
    return OK;
}


/************************************************************************
 *  board_pci_freq_atlas_read
 ************************************************************************/
static UINT32
board_pci_freq_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 
        (REGP(KSEG1BASE, ATLAS_JMPRS) & ATLAS_JMPRS_PCI33M_MSK) ?
	    33330 : 16670;
    return OK;
}		  


/************************************************************************
 *  board_pci_freq_malta_read
 ************************************************************************/
static UINT32
board_pci_freq_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    switch( (REGP(KSEG1BASE, MALTA_JMPRS) & MALTA_JMPRS_PCICLK_MSK)
                                                >> MALTA_JMPRS_PCICLK_SHF )
    {
      case MALTA_JMPRS_PCICLK_10MHZ :
        *(UINT32 *)param = 10000;
	break;
      case MALTA_JMPRS_PCICLK_12_5MHZ :
        *(UINT32 *)param = 12500;
	break;
      case MALTA_JMPRS_PCICLK_16_67MHZ :
        *(UINT32 *)param = 16670;
	break;
      case MALTA_JMPRS_PCICLK_20MHZ :
        *(UINT32 *)param = 20000;
	break;
      case MALTA_JMPRS_PCICLK_25MHZ :
        *(UINT32 *)param = 25000;
	break;
      case MALTA_JMPRS_PCICLK_30MHZ :
        *(UINT32 *)param = 30000;
	break;
      case MALTA_JMPRS_PCICLK_33_33MHZ :
        *(UINT32 *)param = 33330;
	break;
      case MALTA_JMPRS_PCICLK_37_5MHZ :
        *(UINT32 *)param = 37500;
	break;
      default :   /* Should not happen */
        break;
    }

    return OK;
}		  


/************************************************************************
 *  board_pci_alloc_err_read
 ************************************************************************/
extern bool pci_alloc_err;
static UINT32 board_pci_alloc_err_read
(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(bool *)param = pci_alloc_err;
    return OK;
}		  


/************************************************************************
 *  board_psustby_atlas_write
 ************************************************************************/
static UINT32
board_psustby_atlas_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    REGP(KSEG1BASE, ATLAS_PSUSTBY) = ATLAS_PSUSTBY_STBY_GOSTBY;
    return OK;
}


/************************************************************************
 *  board_psustby_sead_write
 ************************************************************************/
static UINT32
board_psustby_sead_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Not valid on SEAD-2 */
    REGP(KSEG1BASE, SEAD_PSUSTBY) = SEAD_PSUSTBY_STBY_GOSTBY;
    return OK;
}


#if 0
/*  I don't know why this function does not work, so for now we'll have
 *  to skip the off command for the Malta board.
 */

/************************************************************************
 *  board_psustby_malta_write
 ************************************************************************/
static UINT32
board_psustby_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32  base;

    SYSCON_read( SYSCON_BOARD_PIIX4_POWER_BASE_ID,
		 (void *)&base,
		 sizeof(UINT32) );

    base = KSEG1( base );
    
    /* Set suspend type (Suspend to Disk) */
    REG16( SWAP_UINT16ADDR_EL( base + PIIX4_PMCNTRL_OFS ) ) = 
        (PIIX4_PMCNTRL_TYPE_STD << PIIX4_PMCNTRL_TYPE_SHF );

    /* Enable suspend */
    REG16( SWAP_UINT16ADDR_EL( base + PIIX4_PMCNTRL_OFS ) ) = 
        ( (PIIX4_PMCNTRL_TYPE_STD << PIIX4_PMCNTRL_TYPE_SHF ) |
	  PIIX4_PMCNTRL_SE_BIT );

    return OK;
}
#endif


/************************************************************************
 *  board_fileflash_write_protected_atlas_malta_read
 ************************************************************************/
static UINT32
board_fileflash_write_protected_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param =
        (REGP(KSEG1BASE, ATLAS_STATUS) & ATLAS_STATUS_MFWR_MSK) ?
	    0 : 1;

    return OK;
}

/************************************************************************
 *  board_monitorflash_write_protected_atlas_malta_read
 ************************************************************************/
static UINT32
board_monitorflash_write_protected_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param =
        (REGP(KSEG1BASE, ATLAS_STATUS) & ATLAS_STATUS_MFWR_MSK) ?
	    1 : 0;

    return OK;
}


/************************************************************************
 *  board_fileflash_write_protected_sead_read
 ************************************************************************/
static UINT32
board_fileflash_write_protected_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 0;
    return OK;
}


/************************************************************************
 *  board_rtc_addr_atlas_read
 ************************************************************************/
static UINT32
board_rtc_addr_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_RTCADR;
    return OK;
}


/************************************************************************
 *  board_rtc_addr_malta_read
 ************************************************************************/
static UINT32
board_rtc_addr_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = MALTA_RTCADR;
    return OK;
}


/************************************************************************
 *  board_rtc_data_atlas_read
 ************************************************************************/
static UINT32
board_rtc_data_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = ATLAS_RTCDAT;
    return OK;
}


/************************************************************************
 *  board_rtc_data_malta_read
 ************************************************************************/
static UINT32
board_rtc_data_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = MALTA_RTCDAT;
    return OK;
}


/************************************************************************
 *  board_eeprom_addr_atlas_read
 ************************************************************************/
static UINT32
board_eeprom_addr_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = ATLAS_EEPROM_IICADR_SN;
    return OK;
}


/************************************************************************
 *  board_eeprom_spd_addr_atlas_malta_read
 ************************************************************************/
static UINT32
board_eeprom_spd_addr_atlas_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = ATLAS_EEPROM_IICADR_SPD000;
    return OK;
}


/************************************************************************
 *  board_eeprom_spd_addr_sead_read
 ************************************************************************/
static UINT32
board_eeprom_spd_addr_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = 0;
    return OK;
}


/************************************************************************
 *  board_am79c973_base_malta_read
 ************************************************************************/
static UINT32
board_am79c973_base_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  AMD 79C973 CS and BC registers
     *  are io mapped into BAR#0.
     */

    if( pci_lookup_bar( PCI_BUS_LOCAL,
			   MALTA_DEVNUM_AM79C973,
			   0,
			   PCI_BAR(0),
			   (void **)param ) )
    {
        return OK;
    }
    else
        return ERROR_SYSCON_UNKNOWN_PARAM;
}


/************************************************************************
 *  board_softend_valid_malta_read
 *************************************************************************/
static UINT32
board_softend_valid_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
#ifdef SOFT_ENDIAN_DEBUG
    *(UINT32 *)param = (( REGP(KSEG1BASE, MALTA_SWITCH) & 128) >> 7);
#else
    *(UINT32 *)param = ( REGP(KSEG1BASE, MALTA_SOFTEND) &
    			 MALTA_SOFTEND_VALID_MSK ) >>
			 MALTA_SOFTEND_VALID_SHF;
#endif
    return OK;
}


/************************************************************************
 *  board_softend_done_malta_read
 *************************************************************************/
static UINT32
board_softend_done_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
#ifdef SOFT_ENDIAN_DEBUG
    *(UINT32 *)param = (( REGP(KSEG1BASE, MALTA_SWITCH) & 2) >> 1);
#else
    *(UINT32 *)param = ( REGP(KSEG1BASE, MALTA_SOFTEND) &
    			 MALTA_SOFTEND_DONE_MSK ) >>
			 MALTA_SOFTEND_DONE_SHF;
#endif
    return OK;
}


/************************************************************************
 *  board_softend_resetsys_malta_write
 *************************************************************************/
static UINT32
board_softend_resetsys_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
#ifdef SOFT_ENDIAN_DEBUG
    REGP(KSEG1BASE, MALTA_LEDBAR) = *(UINT32 *)param;
#else
    REGP(KSEG1BASE, MALTA_SOFTEND) = *(UINT32 *)param;
#endif
    return OK;
}


/************************************************************************
 *  com_iic_baudrate_atlas_read
 ************************************************************************/
static UINT32
com_iic_baudrate_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8*)param = ATLAS_IICFREQ_103125HZ;
    return OK;
}


/************************************************************************
 *  board_eeprom_def_generic_read
 ************************************************************************/
static UINT32
board_eeprom_def_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(t_syscon_eeprom_def **)param = eeprom_def;
    return OK;
}


/************************************************************************
 *
 *                          eeprom_read
 *  Description :
 *  -------------
 *
 *  Read eeprom data by calling EEPROM driver
 *
 *  Return values :
 *  ---------------
 *
 *  Driver return value (OK in case of no error)
 *
 ************************************************************************/
static INT32 
eeprom_read( 
    UINT32  minor,		/* Minor device number			*/
    UINT32  offset,		/* Byte position			*/
    UINT32  length,		/* Byte count				*/
    void    *user_variable )    /* Buffer for data			*/
{
    t_EEPROM_read_descriptor readbuf;

    readbuf.offset = offset + EEPROM_COM_EN0_BASE;
    readbuf.length = length;
    readbuf.buffer = user_variable;

    return IO_read( SYS_MAJOR_EEPROM_IIC, minor, &readbuf );
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_get_eeprom_data
 *  Description :
 *  -------------
 *
 *  Read data store in EEPROM.
 *  Data is stored in the format defined in file eeprom_nm24c09_layout.h 
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_get_eeprom_data( 
    t_mac_addr **mac_addr,  /* Set to mac addr (NULL if invalid)	*/
    t_sn_bcd   **sn )	    /* Set to S/N (NULL if invalid)		*/
{
    t_eeprom_layout eeprom;
    UINT32	    minor = EEPROM_MINOR_NM24C09;
    UINT8	    csum, csum_read;
    UINT32	    pos;
    UINT8	    id, len;
    UINT8	    value[255];
    UINT32          *amd_base;
    static bool     first = TRUE;

    if( (sys_platform != PRODUCT_MALTA_ID) &&
        (sys_platform != PRODUCT_ATLASA_ID) )
    {
        goto eeprom_error;
    }

    /* Self-initialisation */
    if( first )
        first = FALSE;
    else
    {
        if( mac_addr )
	    *mac_addr = mac_addr_read;
	
	if( sn )
	    *sn = sn_read;

        return;
    }

    /* Initial values */
    pos           = 0;
    mac_addr_read = NULL;
    sn_read       = NULL;
 
#ifdef WORKAROUND_CORE_24K
    if (sys_corecard == MIPS_REVISION_CORID_CORE_24K) {
	/* Fake up a board serial number and read MAC address from AMD chip */
	static t_sn_bcd fake_serialnumber = {0x99, 0x99, 0x99, 0x99, 0x99};
	sn_read = &fake_serialnumber;
	goto getamdmac;
    }
#endif

    /* Version */
    if( eeprom_read( minor, pos++, 1, &eeprom.version ) != OK )
        goto eeprom_error;

    csum = eeprom.version;
			
    switch( eeprom.version )
    {
      case EEPROM_VERSION_1 :

	/* Count */
        if( eeprom_read( minor, pos++, 1, &eeprom.count ) != OK )
            goto eeprom_error;

	csum += eeprom.count;

	while( eeprom.count-- )
	{
	    /**** Read field ****/

	    /* ID */
            if( eeprom_read( minor, pos++, 1, &id ) != OK )
                goto eeprom_error;

	    csum += id;

	    /* len */
            if( eeprom_read( minor, pos++, 1, &len ) != OK )
	        goto eeprom_error;

	    csum += len;

	    /* Read field */
            if( eeprom_read( minor, pos, len, value ) != OK )
	        goto eeprom_error;

	    switch( id )
	    {
	      case EEPROM_ID_MAC :
	        /* MAC address */
		if( len == SYS_MAC_ADDR_SIZE )
		{
		    mac_addr_read = &mac_addr_buffer;
	            memcpy( mac_addr_buffer, value, len );
		}
		else
		    goto eeprom_error;
		break;
	      case EEPROM_ID_SN :
	        /* Serial number */
		if( len == SYS_SN_BCD_SIZE )
		{
		    sn_read = &sn_buffer;
	            memcpy( sn_buffer, value, len );
		}
		else
		    goto eeprom_error;
		break;
	      default :
	        /* Ignore */
		break;
	    }

	    pos += len;

	    while( len-- )
	        csum += value[len];
        }

	/* Read checksum */
        if( eeprom_read( minor, pos, 1, &csum_read ) != OK )
            goto eeprom_error;

	if( csum_read != csum )
	    goto eeprom_error;

	break;

      default :
        /* Unknown eeprom format */
        goto eeprom_error;
    }	

#ifdef WORKAROUND_CORE_24K
getamdmac:
#endif
    if( sys_platform == PRODUCT_MALTA_ID )
    {
        /* Get MAC-address from AMD-EEPROM */
        if( SYSCON_read( SYSCON_BOARD_AM79C973_BASE_ID,
                         &(amd_base),
                         sizeof(amd_base) ) == OK )
        {
            amd_base = (void *)KSEG1( amd_base ) ;
            REG( &amd_mac_addr, WORD0 ) = LE32_TO_CPU( REG( amd_base, WORD0 ) ) ;
            REG( &amd_mac_addr, WORD1 ) = LE32_TO_CPU( REG( amd_base, WORD1 ) ) ;
            mac_addr_read = (void*)&amd_mac_addr ;
        }
        else
        {
            mac_addr_read = NULL ;
        }
    }

    if( mac_addr )
        *mac_addr = mac_addr_read;
 
    if( sn )
        *sn = sn_read;

    return;

eeprom_error :
	  
    mac_addr_read = NULL;
    sn_read       = NULL;

    if( mac_addr )
        *mac_addr = NULL;
 
    if( sn )
        *sn = NULL;
}


/************************************************************************
 *
 *                          syscon_arch_board_init
 *  Description :
 *  -------------
 *
 *  Initialize platform specific part of SYSCON (except TTY related parts)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_board_init( 
    t_syscon_obj *objects )		/* Array of SYSCON objects	*/
{
    syscon_objects = objects;

    switch( sys_platform )
    {
      case PRODUCT_MALTA_ID :
      case PRODUCT_ATLASA_ID :

	/*  Core card specifics */

	if( sys_platform == PRODUCT_MALTA_ID )
	{
	    syscon_arch_core_init( objects,
			           /* RAM size */
				   MALTA_SYSTEMRAM_SIZE,
				   /* PCI memory space (transparent) */
				   GT64120_PCIMEM_BASE,
				   GT64120_PCIMEM_SIZE,
				   0,
				   /* PCI IO space (non-transparent) */
				   0,
				   GT64120_PCIIO_SIZE,
				   GT64120_PCIIO_BASE );
        }
	else
	{
	    syscon_arch_core_init( objects,
			           /* RAM size */
			           ATLAS_SYSTEMRAM_SIZE,
			           /* PCI Memory space (transparent) */
			           GT64120_PCIMEM_BASE,
			           GT64120_PCIMEM_SIZE,
			           0,
			           /* PCI IO space (transparent) */
			           GT64120_PCIIO_BASE,
			           GT64120_PCIIO_SIZE,
			           0 );
        }
	
        /*  Setup LED display addresses.
         *  Atlas and Malta have same mapping.
	 */
        alpha_display_size = 8;
        alpha_io[0]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS0);
        alpha_io[1]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS1);
        alpha_io[2]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS2);
        alpha_io[3]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS3);
        alpha_io[4]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS4);
        alpha_io[5]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS5);
        alpha_io[6]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS6);
        alpha_io[7]	   = (UINT32 *)(KSEG1BASE + ATLAS_ASCIIPOS7);

	/* Setup EEPROM data */
	eeprom_count = 2;

	eeprom_def[0].minor = EEPROM_MINOR_NM24C09;
	eeprom_def[0].name  = eeprom_system_name;
	eeprom_def[0].descr = eeprom_system_descr;

	eeprom_def[1].minor = EEPROM_MINOR_SPD000;
	eeprom_def[1].name  = eeprom_spd0_name;
	eeprom_def[1].descr = eeprom_spd0_descr;

        break;

      case PRODUCT_SEAD_ID  :
      case PRODUCT_SEAD2_ID :
	if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	{
	    sys_nb_base = KSEG1(SEAD_MSC01_REG_BASE);
	    syscon_arch_core_init( objects, 0, 0, 0, 0, 0, 0, 0 );
        }

        /*  Setup LED display addresses */
        alpha_display_size = 8;
        alpha_io[0]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS0);
        alpha_io[1]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS1);
        alpha_io[2]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS2);
        alpha_io[3]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS3);
        alpha_io[4]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS4);
        alpha_io[5]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS5);
        alpha_io[6]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS6);
        alpha_io[7]	   = (UINT32 *)(KSEG1BASE + SEAD_ASCIIPOS7);

	/* Setup EEPROM data */
	eeprom_count = 1;

	eeprom_def[0].minor = EEPROM_MINOR_SPD000;
	eeprom_def[0].name  = eeprom_spd0_name;
	eeprom_def[0].descr = eeprom_spd0_descr;

	/*  The objects below are usually registered by
	 *  syscon_arch_core_init(), but this function is not called
	 *  for SEAD and SEAD-2 cards, since they don't have a core
	 *  card.
	 */

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_BASE_ID,
            board_systemram_base_sead_read, NULL,
	    NULL,			    NULL );

	syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SIZE_ID,
            board_systemram_size_sead_read, NULL,
	    NULL,			    NULL );

	if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	    break;

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_CFG_ID,
	     board_systemram_refresh_ns_sead_read,  NULL,
	     board_systemram_refresh_ns_sead_write, NULL );

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_CYCLES_CFG_ID,
	     board_systemram_srasprchg_cycles_sead_read,  NULL,
	     board_systemram_srasprchg_cycles_sead_write, NULL );

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_CYCLES_CFG_ID,
	     board_systemram_sras2scas_cycles_sead_read,  NULL,
	     board_systemram_sras2scas_cycles_sead_write, NULL );

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_CASLAT_CYCLES_CFG_ID,
	     board_systemram_caslat_cycles_sead_read, NULL,
	     NULL,				      NULL );

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_READ_BURSTLEN_CFG_ID,
	     board_systemram_read_burstlen_sead_read, NULL,
	     NULL,				      NULL );

        syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_WRITE_BURSTLEN_CFG_ID,
	     board_systemram_write_burstlen_sead_read, NULL,
	     NULL,				       NULL );

        break;

      default :
        break;
    }

    /* Register objects */

    syscon_register_generic( SYSCON_BOARD_PRODUCTID_ID,
			     board_productid_generic_read, NULL,
			     NULL,			   NULL );

    syscon_register_generic( SYSCON_BOARD_PRODUCTREV_ID,
			     board_productrev_generic_read, NULL,
			     NULL,			    NULL );

    syscon_register_generic( SYSCON_BOARD_REVISION_ID,
			     board_revision_generic_read, NULL,
			     NULL,			  NULL );

    syscon_register_generic( SYSCON_BOARD_NMI_STATUS_ID,
			      board_nmi_status_generic_read, NULL,
			      NULL,			     NULL );

    syscon_register_id_board( SYSCON_BOARD_PRODUCTNAME_ID,
			      /* Atlas */
			      board_productname_read, 
			      (void *)name_atlas,
			      NULL,		      
			      NULL,
			      /* SEAD */
			      board_productname_read, 
			      (sys_platform == PRODUCT_SEAD_ID) ?
			          (void *)name_sead :
			          (void *)name_sead2,
			      NULL,		      
			      NULL,
			      /* Malta */
			      board_productname_read, 
			      (void *)name_malta,
			      NULL,		      
			      NULL );

    syscon_register_id_board( SYSCON_BOARD_NMI_WERR_ID,
			      /* Atlas */
			      board_nmi_werr_atlas_malta_read, NULL,
			      NULL,			       NULL,
			      /* SEAD */
			      board_nmi_werr_sead_read,	       NULL,
			      NULL,			       NULL,
			      /* Malta */
			      board_nmi_werr_atlas_malta_read, NULL,
			      NULL,			       NULL );

    syscon_register_id_board( SYSCON_BOARD_NMI_ACK_ID,
			      /* Atlas */
			      NULL,			       NULL,
			      board_nmi_ack_atlas_malta_write, NULL,
			      /* SEAD */
			      NULL,			       NULL,
			      board_nmi_ack_sead_write,	       NULL,
			      /* Malta */
			      NULL,			       NULL,
			      board_nmi_ack_atlas_malta_write, NULL );

    syscon_register_id_board( SYSCON_BOARD_CORECARDID_ID,
			      /* Atlas */
			      board_corecardid_atlas_malta_read, NULL,
			      NULL,				 NULL,
			      /* SEAD */
			      NULL,				 NULL,
			      NULL,				 NULL,
			      /* Malta */
			      board_corecardid_atlas_malta_read, NULL,
			      NULL,				 NULL );

    syscon_register_id_board( SYSCON_BOARD_CORECARDREV_ID,
			      /* Atlas */
			      board_corecardrev_atlas_malta_read, NULL,
			      NULL,				  NULL,
			      /* SEAD */
			      NULL,				  NULL,
			      NULL,				  NULL,
			      /* Malta */
			      board_corecardrev_atlas_malta_read, NULL,
			      NULL,				  NULL );

    syscon_register_id_board( SYSCON_BOARD_FPGAREV_ID,
			      /* Atlas */
			      board_fpgarev_atlas_malta_read, NULL,
			      NULL,			      NULL,
			      /* SEAD */
			      NULL,			      NULL,
			      NULL,			      NULL,
      			      board_fpgarev_atlas_malta_read, NULL,
			      NULL,			      NULL );

    syscon_register_id_board( SYSCON_BOARD_RTL_ID,
			      /* Atlas */
			      NULL,		   NULL,
			      NULL,		   NULL,
			      /* SEAD */
			      board_rtl_sead_read, NULL,
			      NULL,		   NULL,
			      /* Malta */
			      NULL,		   NULL,
			      NULL,		   NULL );

    syscon_register_id_board( SYSCON_BOARD_RTL_NAME_ID,
			      /* Atlas */
			      NULL,		        NULL,
			      NULL,			NULL,
			      /* SEAD */
			      board_rtl_name_sead_read, NULL,
			      NULL,			NULL,
			      /* Malta */
			      NULL,		        NULL,
			      NULL,			NULL );

    syscon_register_id_board( SYSCON_BOARD_RTLREV_MAJOR_ID,
			      /* Atlas */
			      NULL,		            NULL,
			      NULL,			    NULL,
			      /* SEAD */
			      board_rtlrev_major_sead_read, NULL,
			      NULL,			    NULL,
			      /* Malta */
			      NULL,			    NULL,
			      NULL,			    NULL );

    syscon_register_id_board( SYSCON_BOARD_RTLREV_MINOR_ID,
			      /* Atlas */
			      NULL,		            NULL,
			      NULL,			    NULL,
			      /* SEAD */
			      board_rtlrev_minor_sead_read, NULL,
			      NULL,			    NULL,
			      /* Malta */
			      NULL,		            NULL,
			      NULL,			    NULL );

    syscon_register_id_board( SYSCON_BOARD_USE_DEFAULT_ID,
			      /* Atlas */
			      board_use_default_atlas_malta_read, NULL,
			      NULL,				  NULL,
			      /* SEAD */
			      board_use_default_sead_read,	  NULL,
			      NULL,				  NULL,
			      /* Malta */
			      board_use_default_atlas_malta_read, NULL,
			      NULL,				  NULL );

    syscon_register_id_board( SYSCON_BOARD_INIT_BASED_ON_ENV_ID,
			      /* Atlas */
			      syscon_true_read,  NULL,
			      NULL,		 NULL,
			      /* SEAD (no default switch) */
			      syscon_false_read, NULL,
			      NULL,		 NULL,
			      /* Malta */
			      syscon_true_read,  NULL,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_BOARD_SOFTRES_ID,
			      /* Atlas */
			      NULL,
			      NULL,
			      board_softres_atlas_malta_write, 
			      NULL,
			      /* SEAD */
			      NULL,
			      NULL,
			      (sys_platform == PRODUCT_SEAD2_ID) ?
			          board_softres_sead2_write :
				  NULL,
			      NULL,
			      /* Malta */
			      NULL,
			      NULL,
			      board_softres_atlas_malta_write, 
			      NULL );

    syscon_register_id_board( SYSCON_BOARD_BRKRES_ID,
			      /* Atlas */
			      board_brkres_atlas_malta_read,  NULL,
			      board_brkres_atlas_malta_write, NULL,
			      /* SEAD */
			      NULL,			      NULL,
			      NULL,			      NULL,
			      board_brkres_atlas_malta_read,  NULL,
			      board_brkres_atlas_malta_write, NULL );

    syscon_register_id_board( SYSCON_BOARD_PSUSTBY_ID,
			      /* Atlas */
			      NULL,
			      NULL,
			      board_psustby_atlas_write,  
			      NULL,
			      /* SEAD */
			      NULL,
			      NULL,
			      (sys_platform == PRODUCT_SEAD_ID) ?
			          board_psustby_sead_write : 
				  NULL,
			      NULL,
			      /* Malta */
			      NULL,
			      NULL,
#if 0
/*  I don't know why this function does not work, so for now we'll have
 *  to skip the off command for the Malta board.
 */
			      board_psustby_malta_write, 
			      NULL );
#else
			      NULL,
			      NULL );
#endif

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_BASE_ID,
			      /* Atlas */
			      board_systemflash_base_atlas_read, NULL,
			      NULL,				 NULL,
			      /* SEAD */
			      board_systemflash_base_sead_read,  NULL,
			      NULL,				 NULL,
			      /* Malta */
			      NULL,			         NULL,
			      NULL,				 NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,
			      /* Atlas */
			      board_systemflash_size_atlas_read, NULL,
			      NULL,				 NULL,
			      /* SEAD */
			      board_systemflash_size_sead_read,  NULL,
			      NULL,				 NULL,
			      /* Malta */
			      NULL,				 NULL,
			      NULL,				 NULL );

    syscon_register_id_board( SYSCON_BOARD_MONITORFLASH_BASE_ID,
			      /* Atlas */
			      board_monitorflash_base_atlas_malta_read, NULL,
			      NULL,					NULL,
			      /* SEAD */
			      NULL,					NULL,
			      NULL,					NULL,
			      /* Malta */
			      board_monitorflash_base_atlas_malta_read, NULL,
			      NULL,					NULL );

    syscon_register_id_board( SYSCON_BOARD_MONITORFLASH_SIZE_ID,
			      /* Atlas */
			      board_monitorflash_size_atlas_malta_read, NULL,
			      NULL,					NULL,
			      /* SEAD */
			      NULL,					NULL,
			      NULL,					NULL,
			      /* Malta */
			      board_monitorflash_size_atlas_malta_read, NULL,
			      NULL,					NULL );

    syscon_register_id_board( SYSCON_BOARD_FILEFLASH_BASE_ID,
			      /* Atlas */
			      board_fileflash_base_atlas_malta_read, NULL,
			      NULL,				     NULL,
			      /* SEAD */
			      board_fileflash_base_sead_read,	     NULL,
			      NULL,				     NULL,
			      /* Malta */
			      board_fileflash_base_atlas_malta_read, NULL,
			      NULL,				     NULL );

    syscon_register_id_board( SYSCON_BOARD_FILEFLASH_SIZE_ID,
			      /* Atlas */
			      board_fileflash_size_atlas_malta_read, NULL,
			      NULL,				     NULL,
			      /* SEAD */
			      board_fileflash_size_sead_read,	     NULL,
			      NULL,				     NULL,
			      /* Malta */
			      board_fileflash_size_atlas_malta_read, NULL,
			      NULL,				     NULL );

    syscon_register_id_board( SYSCON_BOARD_SRAM_BASE_ID,
			      /* Atlas */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      board_sram_base_sead_read, NULL,
			      NULL,			 NULL,
			      /* Malta */
			      NULL,			 NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_SRAM_SIZE_ID,
			      /* Atlas */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      board_sram_size_sead_read, NULL,
			      NULL,			 NULL,
			      /* Malta */
			      NULL,			 NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_GT64120_BASE_ID,
			      /* Atlas */
			      board_gt64120_base_atlas_malta_read, NULL,
			      NULL,				   NULL,
			      /* SEAD */
			      NULL,				   NULL,
			      NULL,				   NULL,
			      /* Malta */
			      board_gt64120_base_atlas_malta_read, NULL,
			      NULL,				   NULL );

    syscon_register_id_board( SYSCON_BOARD_SAA9730_BASE_ID,
			      /* Atlas */
			      board_saa9730_base_atlas_read, NULL,
			      NULL,			     NULL,
			      /* SEAD */
			      NULL,			     NULL,
			      NULL,			     NULL,
			      /* Malta */
			      NULL,			     NULL,
			      NULL,			     NULL );

    /* TBD */
    syscon_register_id_board( SYSCON_BOARD_SYM53C810A_BASE_ID,
			      /* Atlas */
			      NULL,  NULL,
			      NULL,  NULL,
			      /* SEAD */
			      NULL,  NULL,
			      NULL,  NULL,
			      /* Malta */
			      NULL,  NULL,
			      NULL,  NULL );

    /* TBD */		    
    syscon_register_id_board( SYSCON_BOARD_INTEL21150_BASE_ID,
			      /* Atlas */
			      NULL,  NULL,
			      NULL,  NULL,
			      /* SEAD */
			      NULL,  NULL,
			      NULL,  NULL,
			      /* Malta */
			      NULL,  NULL,
			      NULL,  NULL );

    syscon_register_id_board( SYSCON_BOARD_ICTA_BASE_ID,
			      /* Atlas */
			      board_icta_base_atlas_read, NULL,
			      NULL,			  NULL,
			      /* SEAD */
			      NULL,			  NULL,
			      NULL,			  NULL,
			      /* Malta */
			      NULL,			  NULL,
			      NULL,			  NULL );

    syscon_register_id_board( SYSCON_BOARD_TMRA_BASE_ID,
			      /* Atlas */
			      board_tmra_base_atlas_read, NULL,
			      NULL,			  NULL,
			      /* SEAD */
			      NULL,			  NULL,
			      NULL,			  NULL,
			      /* Malta */
			      NULL,			  NULL,
			      NULL,			  NULL );

    syscon_register_id_board( SYSCON_BOARD_AM79C973_BASE_ID,
			      /* Atlas */
			      NULL,			      NULL,
			      NULL,			      NULL,
			      /* SEAD */
			      NULL,			      NULL,
			      NULL,			      NULL,
			      /* Malta */
			      board_am79c973_base_malta_read, NULL,
			      NULL,			      NULL );

    syscon_register_id_board( SYSCON_BOARD_LEDGREEN_ID,
			      /* Atlas */
			      board_ledgreen_atlas_read,  NULL,
			      board_ledgreen_atlas_write, NULL,
			      /* SEAD */
			      NULL,			  NULL,
			      NULL,			  NULL,
			      /* Malta */
			      NULL,			  NULL,
			      NULL,			  NULL );

    syscon_register_id_board( SYSCON_BOARD_LEDBAR_ID,
			      /* Atlas */
			      board_ledbar_atlas_malta_read,  NULL,
			      board_ledbar_atlas_malta_write, NULL,
			      /* SEAD */
			      board_ledbar_sead_read,	      NULL,
			      board_ledbar_sead_write,	      NULL,
			      /* Malta */
			      board_ledbar_atlas_malta_read,  NULL,
			      board_ledbar_atlas_malta_write, NULL );

    syscon_register_id_board( SYSCON_BOARD_ASCIIWORD_ID,
			      /* Atlas */
			      NULL,				 NULL,
			      board_asciiword_atlas_malta_write, NULL,
			      /* SEAD */
			      NULL,				 NULL,
			      board_asciiword_sead_write,	 NULL,
			      /* Malta */
			      NULL,				 NULL,
			      board_asciiword_atlas_malta_write, NULL );

    syscon_register_id_board( SYSCON_BOARD_ASCIICHAR_ID,
			      /* Atlas */
			      NULL,			   NULL,
			      board_asciichar_write,	   NULL,
			      /* SEAD */
			      NULL,			   NULL,
			      board_asciichar_write,	   NULL,
			      /* Malta */
			      NULL,			   NULL,
			      board_asciichar_write,	   NULL );

    syscon_register_id_board( SYSCON_BOARD_ASCIISTRING_ID,
			      /* Atlas */
			      NULL,		       NULL,
			      board_asciistring_write, NULL,
			      /* SEAD */
			      NULL,		       NULL,
			      board_asciistring_write, NULL,
			      /* Malta */
			      NULL,		       NULL,
			      board_asciistring_write, NULL );

    syscon_register_id_board( SYSCON_BOARD_GET_MILLISEC_ID,
			      /* Atlas */
			      board_get_millisec_atlas_malta_read, NULL,
			      NULL,			     NULL,
			      /* SEAD */
			      board_get_millisec_sead_read,  NULL,
			      NULL,			     NULL,
			      /* Malta */
			      board_get_millisec_atlas_malta_read, NULL,
			      NULL,			     NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_SECTORSIZE_ID,
			      /* Atlas */
			      board_systemflash_sectorsize_atlas_read, NULL,
			      NULL,				       NULL,
			      /* SEAD */
			      board_systemflash_sectorsize_sead_read,  NULL,
			      NULL,				       NULL,
			      /* Malta */
			      NULL,				       NULL,
			      NULL,				       NULL );

    syscon_register_id_board( SYSCON_BOARD_MONITORFLASH_SECTORSIZE_ID,
			      /* Atlas */
			      board_monitorflash_sectorsize_atlas_read, NULL,
			      NULL,				        NULL,
			      /* SEAD */
			      NULL,  NULL,
			      NULL,				        NULL,
			      /* Malta */
			      board_monitorflash_sectorsize_malta_read, NULL,
			      NULL,				        NULL );

    syscon_register_id_board( SYSCON_BOARD_FILEFLASH_SECTORSIZE_ID,
			      /* Atlas */
			      board_fileflash_sectorsize_atlas_malta_read, NULL,
			      NULL,					   NULL,
			      /* SEAD */
			      board_fileflash_sectorsize_sead_read,	   NULL,
			      NULL,					   NULL,
			      /* Malta */
			      board_fileflash_sectorsize_atlas_malta_read, NULL,
			      NULL,					   NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_BANKCOUNT_ID,
			      /* Atlas */
			      board_systemflash_bankcount_atlas_read, NULL,
			      NULL,				      NULL,
			      /* SEAD */
			      board_systemflash_bankcount_sead_read,  NULL,
			      NULL,				      NULL,
			      /* Malta */
			      NULL,				      NULL,
			      NULL,				      NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_BLOCKCOUNT_ID,
			      /* Atlas */
			      board_systemflash_blockcount_atlas_read, NULL,
			      NULL,				       NULL,
			      /* SEAD */
			      board_systemflash_blockcount_sead_read,  NULL,
			      NULL,				       NULL,
			      /* Malta */
			      NULL,				       NULL,
			      NULL,				       NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_WRITE_ENABLE_ID,
			      /* Atlas */
			      NULL,					  NULL,
			      board_systemflash_write_enable_atlas_write, NULL,
			      /* SEAD */
			      NULL,                                       NULL,
			      board_systemflash_write_enable_sead_write,  NULL,
			      /* Malta */
			      NULL,                                       NULL,
			      NULL,					  NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_WRITE_DISABLE_ID,
			      /* Atlas */
			      NULL,					   NULL,
			      board_systemflash_write_disable_atlas_write, NULL,
			      /* SEAD */
			      NULL,					  NULL,
			      board_systemflash_write_disable_sead_write, NULL,
			      /* Malta */
			      NULL,					  NULL,
			      NULL,					  NULL );

    syscon_register_id_board( SYSCON_BOARD_SYSTEMFLASH_WRITE_PROTECTED_ID,
			      /* Atlas */
			      board_systemflash_write_protected_atlas_read, NULL,
			      NULL,					    NULL,
			      /* SEAD */
			      board_systemflash_write_protected_sead_read,  NULL,
			      NULL,					    NULL,
			      /* Malta */
			      NULL,					    NULL,
			      NULL,					    NULL );

    syscon_register_id_board( SYSCON_BOARD_MONITORFLASH_WRITE_PROTECTED_ID,
			      /* Atlas */
			      board_monitorflash_write_protected_atlas_malta_read, NULL,
			      NULL,						NULL,
			      /* SEAD */
			      NULL,	NULL,
			      NULL,	NULL,
			      /* Malta */
			      board_monitorflash_write_protected_atlas_malta_read, NULL,
			      NULL,						NULL );

    syscon_register_id_board( SYSCON_BOARD_FILEFLASH_WRITE_PROTECTED_ID,
			      /* Atlas */
			      board_fileflash_write_protected_atlas_malta_read, NULL,
			      NULL,						NULL,
			      /* SEAD */
			      board_fileflash_write_protected_sead_read,	NULL,
			      NULL,						NULL,
			      /* Malta */
			      board_fileflash_write_protected_atlas_malta_read, NULL,
			      NULL,						NULL );

    syscon_register_id_board( SYSCON_BOARD_PCI_FREQ_KHZ_ID,
			      /* Atlas */
			      board_pci_freq_atlas_read, NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* Malta */
			      board_pci_freq_malta_read, NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_PCI_ALLOC_ERR_ID,
			      /* Atlas */
			      board_pci_alloc_err_read,  NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* Malta */
			      board_pci_alloc_err_read,  NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_RTC_ADDR_ID,
			      /* Atlas */
			      board_rtc_addr_atlas_read, NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* Malta */
			      board_rtc_addr_malta_read, NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_RTC_DATA_ID,
			      /* Atlas */
			      board_rtc_data_atlas_read, NULL,
			      NULL,			 NULL,
			      /* SEAD */
			      NULL,			 NULL,
			      NULL,			 NULL,
			      /* Malta */
			      board_rtc_data_malta_read, NULL,
			      NULL,			 NULL );

    syscon_register_id_board( SYSCON_BOARD_RTC_REG_SIZE_ID,
			      /* Atlas */
			      syscon_uint8_read, &size32,
			      NULL,		 NULL,
			      /* SEAD */
			      NULL,		 NULL,
			      NULL,		 NULL,
			      /* Malta */
			      syscon_uint8_read, &size8,
			      NULL,		 NULL );

    syscon_register_id_board( SYSCON_BOARD_EEPROM_ADDR,
			      /* Atlas */
			      board_eeprom_addr_atlas_malta_read, NULL,
			      NULL,				  NULL,
			      /* SEAD */
			      NULL,				  NULL,
			      NULL,				  NULL,
			      /* Malta */
			      board_eeprom_addr_atlas_malta_read, NULL,
			      NULL,				  NULL );


    syscon_register_id_board( SYSCON_BOARD_EEPROM_SPD_ADDR,
			      /* Atlas */
			      board_eeprom_spd_addr_atlas_malta_read, NULL,
			      NULL,				      NULL,
			      /* SEAD */
			      board_eeprom_spd_addr_sead_read,        NULL,
			      NULL,				      NULL,
			      /* Malta */
			      board_eeprom_spd_addr_atlas_malta_read, NULL,
			      NULL,				      NULL );

    syscon_register_id_board( SYSCON_COM_IIC_BAUDRATE_ID,
			      /* Atlas */
			      com_iic_baudrate_atlas_read, NULL,
			      NULL,			   NULL,
			      /* SEAD */
			      NULL,			   NULL,
			      NULL,			   NULL,
			      /* Malta */
			      NULL,			   NULL,
			      NULL,			   NULL );

    syscon_register_id_board( SYSCON_BOARD_PIIX4_SMB_BASE_ID,
			      /* Atlas   */
			      NULL,			       NULL,
			      NULL,			       NULL,
			      /* SEAD    */
			      NULL,			       NULL,
			      NULL,			       NULL,
			      /* Malta */
			      board_piix4_smb_base_malta_read, NULL,
			      NULL,			       NULL );

    syscon_register_id_board( SYSCON_BOARD_PIIX4_POWER_BASE_ID,
			      /* Atlas   */
			      NULL,			         NULL,
			      NULL,			         NULL,
			      /* SEAD    */
			      NULL,			         NULL,
			      NULL,			         NULL,
			      /* Malta */
			      board_piix4_power_base_malta_read, NULL,
			      NULL,			         NULL );

    syscon_register_id_board( SYSCON_COM_PIIX4_IIC_BIGEND_ID,
			      /* Atlas   */
			      NULL,	         NULL,
			      NULL,		 NULL,
			      /* SEAD    */
			      NULL,		 NULL,
			      NULL,		 NULL,
			      /* Malta (PCI is always little endian) */
			      syscon_false_read, NULL,
			      NULL,		 NULL );

    syscon_register_id_board( SYSCON_COM_EN0_MAJOR_DEVICE_ID,
                              /* Atlas   */
                              com_en0_major_device_atlas_read, NULL,
                              NULL,			       NULL,
                              /* SEAD    */
                              NULL,			       NULL,
                              NULL,			       NULL,
                              /* Malta */
                              com_en0_major_device_malta_read, NULL,
                              NULL,			       NULL );
 
    syscon_register_id_board( SYSCON_COM_EN0_IC_ID,
                              /* Atlas   */
                              syscon_true_read, NULL,
                              NULL,		NULL,
                              /* SEAD    */
                              NULL,		NULL,
                              NULL,		NULL,
                              /* Malta */
                              syscon_true_read, NULL,
                              NULL,		NULL );
 
    syscon_register_id_board( SYSCON_COM_EN0_INTLINE_ID,
                              /* Atlas   */
                              com_en0_intline_atlas_read, NULL,
                              NULL,			  NULL,
                               /* SEAD    */
                              NULL,			  NULL,
                              NULL,			  NULL,
                              /* Malta */
                              com_en0_intline_malta_read, NULL,
                              NULL,			  NULL );

    syscon_register_id_board( SYSCON_BOARD_SOFTEND_VALID_ID,
    			      /* Atlas */
			      NULL,			      NULL,
			      NULL,			      NULL,
			      /* SEAD */
			      NULL,			      NULL,
			      NULL,			      NULL,
			      /* Malta */
			      board_softend_valid_malta_read, NULL,
			      NULL,			      NULL );

    syscon_register_id_board( SYSCON_BOARD_SOFTEND_DONE_ID,
    			      /* Atlas */
			      NULL,			     NULL,
			      NULL,			     NULL,
			      /* SEAD */
			      NULL,			     NULL,
			      NULL,			     NULL,
			      /* Malta */
			      board_softend_done_malta_read, NULL,
			      NULL,			     NULL );

    syscon_register_id_board( SYSCON_BOARD_SOFTEND_RESETSYS_ID,
    			      /* Atlas */
			      NULL,				  NULL,
			      NULL,				  NULL,
			      /* SEAD */
			      NULL,				  NULL,
			      NULL,				  NULL,
			      /* Malta */
			      NULL,				  NULL,
			      board_softend_resetsys_malta_write, NULL );

    syscon_register_generic( SYSCON_BOARD_EEPROM_COUNT_ID,
			     syscon_uint8_read, (void *)&eeprom_count,
			     NULL,	        NULL );

    syscon_register_generic( SYSCON_BOARD_EEPROM_DEF_ID,
			     board_eeprom_def_generic_read, NULL,
			     NULL,			    NULL );
}


/************************************************************************
 *
 *                          syscon_register_id_board
 *  Description :
 *  -------------
 *
 *  Function used to register platform specific SYSCON object functions.
 *
 *  A read and/or write function may be registered for each platform.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed for the particular platform.
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
syscon_register_id_board(
    t_syscon_ids   id,			/* OBJECT ID from syscon_api.h	*/
    /* Atlas */
    t_syscon_func  read_atlas,		/* Atlas read function		*/
    void	   *read_data_atlas,	/* Registered data		*/
    t_syscon_func  write_atlas,		/* Atlas write function		*/
    void	   *write_data_atlas,	/* Registered data		*/
    /* SEAD */
    t_syscon_func  read_sead,		/* SEAD read function		*/
    void	   *read_data_sead,	/* Registered data		*/
    t_syscon_func  write_sead,		/* SEAD write function		*/
    void	   *write_data_sead,	/* Registered data		*/
    /* Malta */
    t_syscon_func  read_malta,		/* Malta read function		*/
    void	   *read_data_malta,	/* Registered data		*/
    t_syscon_func  write_malta,		/* Malta write function		*/
    void	   *write_data_malta )	/* Registered data		*/
{
    t_syscon_obj *obj;

    obj = &syscon_objects[id];

    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :
        obj->read       = read_atlas;
	obj->read_data  = read_data_atlas;
	obj->write      = write_atlas;
	obj->write_data = write_data_atlas;
	break;
      case PRODUCT_SEAD_ID  :
      case PRODUCT_SEAD2_ID :
        obj->read       = read_sead;
	obj->read_data  = read_data_sead;
	obj->write      = write_sead;
	obj->write_data = write_data_sead;
	break;
      case PRODUCT_MALTA_ID :
        obj->read       = read_malta;
	obj->read_data  = read_data_malta;
	obj->write      = write_malta;
	obj->write_data = write_data_malta;
	break;
      default : /* Should not happen */
        obj->read  = NULL;
	obj->write = NULL;
	break;
    }    
}
