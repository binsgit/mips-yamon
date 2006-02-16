
/************************************************************************
 *
 *      INITMODULES.C
 *
 *      This module is a generic template for installation and
 *      initialization of modules to be included as blackboxes.
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


/*****************************************************************************
 *
 * Defines, needed in INITMODULES.C for the modules to be incl. in the system
 *
 ****************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <sys_api.h>
#include <product.h>
#include <syscon_api.h>
#include <io_api.h>
#include <pci.h>

/*********************************************************************
*
*  Complete list of modules to be included in YAMON.
*
*  The file initswitch.h defines whether a specific module is included
*  or not (defines INCLUDE_SYSCON, INCLUDE_IO, INCLUDE_RTC ...).
*
*  The bootstrap sequence of module initialisation is important since 
*  some modules depend on others.
*
*********************************************************************/

#include <initswitch.h>

INT32 EXCEP_init(void);
INT32 RTC_install(void);
INT32 freq_init(void);
INT32 IIC_SAA9730_install(void);
INT32 iic_piix4_install(void);
INT32 iic_sead_install(void);
INT32 EEPROM_IIC_install(void);
INT32 FLASH_STRATA_install(void);
INT32 SYSENV_init(void);
INT32 env_init(void);
INT32 SERIAL_SAA9730_install(void);
INT32 SERIAL_TI16550_install(void);
INT32 LAN_SAA9730_install(void);
INT32 LAN_AM79C973_install(void);
INT32 NET_init(void);
INT32 ide_install(void);


/*****************************************************************************
 *
 * Get modules installed and initialized.
 *
 ****************************************************************************/

UINT32 
initmodules( void )
{
    UINT32 product = 0xffffffff;

#if INCLUDE_SYSCON	   /* Must be 1st module		    */
    /* Initialize SYSCON */	 
    SYSCON_init() ;

    SYSCON_read( SYSCON_BOARD_PRODUCTID_ID, &product, sizeof(product) );
    /* Now we can use DISP_STR (DISP_STR depends on SYSCON)	    */
#endif


#if INCLUDE_IO		  /* Depends on SYSCON			    */
    DISP_STR( "IO" );
    IO_setup( 30 ) ; /* make room for 30 device drivers */	 
#endif


#if INCLUDE_EXCEP	  /* Depends on IO			    */
    DISP_STR( "EXCEP" );
    EXCEP_init() ;
#endif


#if INCLUDE_RTC		  /* Depends on IO			    */
    if( (product == PRODUCT_ATLASA_ID) ||
        (product == PRODUCT_MALTA_ID) )
    {
	DISP_STR( "RTC" );
	RTC_install();
    }
#endif


#if INCLUDE_FREQ	  /* Depends on RTC			    */
    DISP_STR( "FREQ" );
    freq_init();
#endif


#if INCLUDE_PCI		  /* Depends on FREQ due to sys_wait_ms() */
    if( (product == PRODUCT_ATLASA_ID) ||
        (product == PRODUCT_MALTA_ID) )
    {
	DISP_STR( "PCI" );
        switch( pci_init() )
        {
	  case OK: break;
          case ERROR_PCI_MALLOC :
	    DISP_STR("E:P_ALLO");
	    goto loop;
          case ERROR_PCI_RANGE :
	    DISP_STR("E:P_RANG");
	    goto loop;
          case ERROR_PCI_UNKNOWN_DEVICE :
	    DISP_STR("E:P_UNKN");
	    goto loop;
          default : 
	    DISP_STR("E:P_CFG");
loop:
	    while(1);
        }
    }
#endif


#if INCLUDE_IIC		  /* Depends on PCI		            */
    DISP_STR( "IIC" );
    if(product == PRODUCT_ATLASA_ID)
    {
	IIC_SAA9730_install();
    }
    else if(product == PRODUCT_MALTA_ID)
    {
	iic_piix4_install();
    }
    else if( (product == PRODUCT_SEAD_ID) ||
	     (product == PRODUCT_SEAD2_ID) )
    {
	iic_sead_install();
    }
#endif


#if INCLUDE_EEPROM_IIC	  /* Depends on IIC_SAA9730		    */
    DISP_STR( "EEPROM" );
    EEPROM_IIC_install();
#endif


#if INCLUDE_FLASH_STRATA  /* Depends on IO			    */
    DISP_STR( "FLASH" );
    FLASH_STRATA_install();
#endif


#if INCLUDE_SYSENV	  /* Depends on FLASH_STRATA		    */
    DISP_STR( "SYSENV" );
    SYSENV_init();
#endif


#if INCLUDE_ENV		  /* Depends on SYSENV			    */
    DISP_STR( "ENV" );
    env_init();
#endif


#if INCLUDE_SERIAL	  /* Depends on ENV			    */
    DISP_STR( "SERIAL" );
    if( product == PRODUCT_ATLASA_ID )
    {
	SERIAL_SAA9730_install();
    }
    SERIAL_TI16550_install();
#endif


#if INCLUDE_LAN_SAA9730	  /* Depends on ENV			    */
    if( product == PRODUCT_ATLASA_ID )
    {
	DISP_STR( "SAA_LAN" );
	LAN_SAA9730_install();
    }
#endif


#if INCLUDE_LAN_AM79C973  /* Depends on ENV			    */
    if (product == PRODUCT_MALTA_ID)
    {
	DISP_STR( "AMD_LAN" );
	LAN_AM79C973_install();
    }
#endif


#if INCLUDE_NET		  /* Depends on LAN driver		    */
    DISP_STR( "NET" );
    NET_init() ;
#endif


#if INCLUDE_IDE
    if( product == PRODUCT_MALTA_ID )
    {
	DISP_STR( "IDE" );
	ide_install();
    }
#endif

    return(OK) ;
}



