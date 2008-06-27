
/************************************************************************
 *
 *      ATLAS.h
 *
 *      The 'ATLAS' module defines the ATLAS board specific
 *      address-MAP, registers, etc.
 *
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

#ifndef ATLAS_H
#define  ATLAS_H

/************************************************************************
 *  Note: all addresses are physical ! 
 *  Must be mapped into kseg0 (system RAM) or kseg1 (registers, devices)
*************************************************************************/

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <mips.h>
#include <pci_api.h>

/************************************************************************
 *  ATLAS, EEPROM devices,  IIC-slaveadresses
*************************************************************************/

#define  ATLAS_EEPROM_IICADR_SN       0x54  /* NM24C09, 1024 bytes      */
#define  ATLAS_EEPROM_IICADR_SPD000   0x50 /* PC-SDRAM, 256 bytes      */

/************************************************************************
 *  ATLAS, IIC-bus frequencies for the SAA9730 controller
*************************************************************************/
#define  ATLAS_IICFREQ_5156HZ         0x05
#define  ATLAS_IICFREQ_10312HZ        0x01
#define  ATLAS_IICFREQ_68750HZ        0x04
#define  ATLAS_IICFREQ_103125HZ       0x06
#define  ATLAS_IICFREQ_275000HZ       0x00
#define  ATLAS_IICFREQ_412500HZ       0x02


/************************************************************************
 *  ATLAS physical address MAP (512 MByte)
*************************************************************************/

#define  ATLAS_SYSTEMRAM_BASE         0x00000000 /* System RAM:     */
#define  ATLAS_SYSTEMRAM_SIZE         0x10000000 /*   256 MByte     */

#define  ATLAS_GT64120_BASE	      0x1BE00000 /* GT64120:        */
#define  ATLAS_GT64120_SIZE	      0x00200000 /*     2 MByte     */

#define  ATLAS_SYSTEMFLASH_BASE       0x1C000000 /* System Flash:   */
#define  ATLAS_SYSTEMFLASH_SIZE       0x02000000 /*    32 MByte     */

#define  ATLAS_SYSTEMFLASH_SECTORSIZE 0x00020000 /* Sect. = 128 KB  */
#define  ATLAS_SYSTEMFLASH_BANKCOUNT           2 /*                 */
#define  ATLAS_SYSTEMFLASH_BLOCKCOUNT         64 /*                 */

#define  ATLAS_MONITORFLASH_BASE      0x1E000000 /* Monitor Flash:  */
#define  ATLAS_MONITORFLASH_SIZE      0x003E0000 /*     4 MByte     */
#define  ATLAS_MONITORFLASH_SECTORSIZE 0x00010000 /* Sect. = 64 KB  */

#define  ATLAS_RESERVED_BASE          0x1E400000 /* Reserved:       */
#define  ATLAS_RESERVED_SIZE          0x00C00000 /*    12 MByte     */

#define  ATLAS_FPGA_BASE              0x1F000000 /* FPGA:           */
#define  ATLAS_FPGA_SIZE              0x00C00000 /*    12 MByte     */

#define  ATLAS_BOOTROM_BASE           0x1FC00000 /* Boot ROM:       */
#define  ATLAS_BOOTROM_SIZE           0x00400000 /*     4 MByte     */


/************************************************************************
 *  Use of PCI ranges (Memory vs I/O)
*************************************************************************/

#define  GT64120_PCIMEM_BASE          0x10000000 /* PCI 1 memory:   */
#define  GT64120_PCIMEM_SIZE          0x0be00000 /*  128+2+60 MByte */

#define  GT64120_PCIIO_BASE           0x18000000 /* PCI 2 memory    */
#define  GT64120_PCIIO_SIZE           0x00200000 /*     2 MByte     */

/************************************************************************
 *  ATLAS FILEFLASH: upper 128 KByte (2x64 KByte sectors) of MONITORFLASH
*************************************************************************/

#define  ATLAS_FILEFLASH_BASE         0x1E3E0000 /* File Flash:     */
#define  ATLAS_FILEFLASH_SIZE         0x00020000 /*   128 KByte     */

#define  ATLAS_FILEFLASH_SECTORSIZE   0x00010000 /* Sect. = 64 KB   */


/************************************************************************
 *  ATLAS, devices, base adresses:
*************************************************************************/

#define  ATLAS_ICTA_BASE              0x1F000000 /* FPGA, Int. ctrl,
                                                    ref. 'icta.h'       */
#define  ATLAS_TMRA_BASE              0x1F000300 /* FPGA, Timer device,
                                                    ref. 'tmra.h'       */
#define  ATLAS_TI16C550_BASE          0x1F000900 /* TI16C550 UART,
                                                    upper connector     */

/************************************************************************
 *  ATLAS FPGA, register address map:
*************************************************************************/

/************************************************************************
 *  REVISION: MUST BE FIXED TO  0x1FC00010 on any baseboard
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_REVISION              0x1FC00010 /* REVISION              */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: REVISION ********/


/* field: FPGRV */
#define ATLAS_REVISION_FPGRV_SHF	16
#define ATLAS_REVISION_FPGRV_MSK	(MSK(8) << ATLAS_REVISION_FPGRV_SHF)

/* field: CORID */
#define ATLAS_REVISION_CORID_SHF	10
#define ATLAS_REVISION_CORID_MSK	(MSK(6) << ATLAS_REVISION_CORID_SHF)

/* field: CORRV */
#define ATLAS_REVISION_CORRV_SHF	8
#define ATLAS_REVISION_CORRV_MSK	(MSK(2) << ATLAS_REVISION_CORRV_SHF)

/* field: PROID */
#define ATLAS_REVISION_PROID_SHF	4
#define ATLAS_REVISION_PROID_MSK	(MSK(4) << ATLAS_REVISION_PROID_SHF)

/* field: PRORV */
#define ATLAS_REVISION_PRORV_SHF	0
#define ATLAS_REVISION_PRORV_MSK	(MSK(4) << ATLAS_REVISION_PRORV_SHF)


/************************************************************************
 *  HWINTACK: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_HWINTACK              0x1F000100 /* HWINTACK              */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: HWINTACK ********/

/* bit 1: ACKDEG */
#define ATLAS_HWINTACK_ACKDEG_SHF	1
#define ATLAS_HWINTACK_ACKDEG_MSK	(MSK(1) << ATLAS_HWINTACK_ACKDEG_SHF)
#define ATLAS_HWINTACK_ACKDEG_SET	ATLAS_HWINTACK_ACKDEG_MSK


/************************************************************************
 *  NMISTATUS: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_NMISTATUS                  0x1F000024 /* NMISTATUS	*/


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: NMISTATUS ********/

/* bit 0: NMISTATUS */
#define ATLAS_NMISTATUS_ONNMI_SHF	0
#define ATLAS_NMISTATUS_ONNMI_MSK	(MSK(1) << ATLAS_NMISTATUS_ONNMI_SHF)
#define ATLAS_NMISTATUS_ONNMI_BIT	ATLAS_NMISTATUS_ONNMI_MSK	


/************************************************************************
 *  NMIACK:
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_NMIACK			0x1F000104 /* NMIACK		*/


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: NMIACK ********/

/* bit 0: NMI */
#define ATLAS_NMIACK_ONNMI_SHF		0
#define ATLAS_NMIACK_ONNMI_MSK		(MSK(1) << ATLAS_NMIACK_ONNMI_SHF)
#define ATLAS_NMIACK_ONNMI_BIT		ATLAS_NMIACK_ONNMI_MSK


/************************************************************************
 *  SWITCHES / STATUS: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_SWVALUE               0x1F000200 /* SWVALUE               */
#define ATLAS_STATUS                0x1F000208 /* STATUS                */
#define ATLAS_JMPRS                 0x1F000210 /* JMPRS                 */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: SWVALUE ********/

/* bit 11: S14 */
#define ATLAS_SWVALUE_S14_SHF	    11
#define ATLAS_SWVALUE_S14_MSK	    (MSK(1) << ATLAS_SWVALUE_S14_SHF)
#define ATLAS_SWVALUE_S14_SET       ATLAS_SWVALUE_S14_MSK

/* bit 10: S13 */
#define ATLAS_SWVALUE_S13_SHF	    10
#define ATLAS_SWVALUE_S13_MSK	    (MSK(1) << ATLAS_SWVALUE_S13_SHF)
#define ATLAS_SWVALUE_S13_SET       ATLAS_SWVALUE_S13_MSK

/* bit 9: S12 */
#define ATLAS_SWVALUE_S12_SHF	    9
#define ATLAS_SWVALUE_S12_MSK	    (MSK(1) << ATLAS_SWVALUE_S12_SHF)
#define ATLAS_SWVALUE_S12_SET       ATLAS_SWVALUE_S12_MSK

/* bit 8: S11 */
#define ATLAS_SWVALUE_S11_SHF	    8
#define ATLAS_SWVALUE_S11_MSK	    (MSK(1) << ATLAS_SWVALUE_S11_SHF)
#define ATLAS_SWVALUE_S11_SET       ATLAS_SWVALUE_S11_MSK

/* bits 7:0: S2 */
#define ATLAS_SWVALUE_S2_SHF	    0
#define ATLAS_SWVALUE_S2_MSK	    (MSK(8) << ATLAS_SWVALUE_S2_SHF)


/******** reg: STATUS ********/

/* bit 4 : MFWR */
#define ATLAS_STATUS_MFWR_SHF	    4
#define ATLAS_STATUS_MFWR_MSK	    (MSK(1) << ATLAS_STATUS_MFWR_SHF)
#define ATLAS_STATUS_MFWR_SET        ATLAS_STATUS_MFWR_MSK

/* bit 3 : S54 */
#define ATLAS_STATUS_S54_SHF	    3
#define ATLAS_STATUS_S54_MSK	    (MSK(1) << ATLAS_STATUS_S54_SHF)
#define ATLAS_STATUS_S54_SET        ATLAS_STATUS_S54_MSK

/* bit 2 : S53 */
#define ATLAS_STATUS_S53_SHF	    2
#define ATLAS_STATUS_S53_MSK	    (MSK(1) << ATLAS_STATUS_S53_SHF)
#define ATLAS_STATUS_S53_SET        ATLAS_STATUS_S53_MSK

/* bit 1: BIGEND */
#define ATLAS_STATUS_BIGEND_SHF	    1
#define ATLAS_STATUS_BIGEND_MSK	    (MSK(1) << ATLAS_STATUS_BIGEND_SHF)
#define ATLAS_STATUS_BIGEND_SET     ATLAS_STATUS_BIGEND_MSK

/* bit 0: PCISYS */
#define ATLAS_STATUS_PCISYS_SHF	    0
#define ATLAS_STATUS_PCISYS_MSK	    (MSK(1) << ATLAS_STATUS_PCISYS_SHF)
#define ATLAS_STATUS_PCISYS_SET     ATLAS_STATUS_PCISYS_MSK


/******** reg: JMPRS ********/

/* bit 1: EELOCK */
#define ATLAS_JMPRS_EELOCK_SHF	    1
#define ATLAS_JMPRS_EELOCK_MSK	    (MSK(1) << ATLAS_JMPRS_EELOCK_SHF)
#define ATLAS_JMPRS_EELOCK_SET      ATLAS_JMPRS_EELOCK_MSK

/* bit 0: PCI33M */
#define ATLAS_JMPRS_PCI33M_SHF	    0
#define ATLAS_JMPRS_PCI33M_MSK	    (MSK(1) << ATLAS_JMPRS_PCI33M_SHF)
#define ATLAS_JMPRS_PCI33M_SET      ATLAS_JMPRS_PCI33M_MSK


/************************************************************************
 *  DISPLAYS: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_LEDGREEN              0x1F000400 /* LEDGREEN              */
#define ATLAS_LEDBAR                0x1F000408 /* LEDBAR                */
#define ATLAS_ASCIIWORD             0x1F000410 /* ASCIIWORD             */
#define ATLAS_ASCIIPOS0             0x1F000418 /* ASCIIPOS0             */
#define ATLAS_ASCIIPOS1             0x1F000420 /* ASCIIPOS1             */
#define ATLAS_ASCIIPOS2             0x1F000428 /* ASCIIPOS2             */
#define ATLAS_ASCIIPOS3             0x1F000430 /* ASCIIPOS3             */
#define ATLAS_ASCIIPOS4             0x1F000438 /* ASCIIPOS4             */
#define ATLAS_ASCIIPOS5             0x1F000440 /* ASCIIPOS5             */
#define ATLAS_ASCIIPOS6             0x1F000448 /* ASCIIPOS6             */
#define ATLAS_ASCIIPOS7             0x1F000450 /* ASCIIPOS7             */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: LEDGREEN ********/

/* bits 0: ON */
#define ATLAS_LEDGREEN_ON_SHF	    0
#define ATLAS_LEDGREEN_ON_MSK	    (MSK(1) << ATLAS_LEDGREEN_ON_SHF)
#define ATLAS_LEDGREEN_ON_SET       ATLAS_LEDGREEN_ON_MSK



/************************************************************************
 *  RESET CONTROL: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_SOFTRES               0x1F000500 /* SOFTRES               */
#define ATLAS_BRKRES                0x1F000508 /* BRKRES                */

/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: SOFTRES ********/

/* bits 7:0: RESET */
#define ATLAS_SOFTRES_RESET_SHF	    0
#define ATLAS_SOFTRES_RESET_MSK	    (MSK(8) << ATLAS_SOFTRES_RESET_SHF)
#define ATLAS_SOFTRES_RESET_GORESET 0x42     /* magic value to reset    */    


/******** reg: BRKRES ********/

/* bits 7:0: WIDTH */
#define ATLAS_BRKRES_WIDTH_SHF	    0
#define ATLAS_BRKRES_WIDTH_MSK	    (MSK(8) << ATLAS_BRKRES_WIDTH_SHF)


/************************************************************************
 *  PSU STANDBY CONTROL: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_PSUSTBY               0x1F000600 /* PSUSTBY               */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: PSUSTBY ********/

/* bits 7:0: STBY */
#define ATLAS_PSUSTBY_STBY_SHF	    0
#define ATLAS_PSUSTBY_STBY_MSK	    (MSK(8) << ATLAS_PSUSTBY_STBY_SHF)
#define ATLAS_PSUSTBY_STBY_GOSTBY   0x4D     /* magic value to go stby  */    


/************************************************************************
 *  SYSTEM FLASH WRITE PROTECT CONTROL: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_SFWCTRL               0x1F000700 /* SFWCTRL               */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: SFWCTRL ********/

/* bits 7:0: WRENA */
#define ATLAS_SFWCTRL_WRENA_SHF	    0
#define ATLAS_SFWCTRL_WRENA_MSK	    (MSK(8) << ATLAS_SFWCTRL_WRENA_SHF)
#define ATLAS_SFWCTRL_WRENA_ENSFWRITE 0xC7   /* magic value to enable 
                                                write protect           */    


/************************************************************************
 *  RTC-device indirect register access: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_RTCADR                0x1F000800 /* RTCADR                */
#define ATLAS_RTCDAT                0x1F000808 /* RTCDAT                */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: RTCADR ********/

/* bits 7:0: ADR */
#define ATLAS_RTCADR_ADR_SHF	    0
#define ATLAS_RTCADR_ADR_MSK	    (MSK(8) << ATLAS_RTCADR_ADR_SHF)


/******** reg: RTCDAT ********/

/* bits 7:0: DATA */
#define ATLAS_RTCDAT_DATA_SHF	    0
#define ATLAS_RTCDAT_DATA_MSK	    (MSK(8) << ATLAS_RTCDAT_DATA_SHF)


/************************************************************************
 *  General Purpose Input Output Pin Control: 
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define ATLAS_GPOUT                 0x1F000A00 /* GPOUT                 */
#define ATLAS_GPINP                 0x1F000A08 /* GPINP                 */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: GPOUT ********/

/* bits 7:0: OUTVAL */
#define ATLAS_GPOUT_OUTVAL_SHF	    0
#define ATLAS_GPOUT_OUTVAL_MSK	    (MSK(8) << ATLAS_GPOUT_OUTVAL_SHF)


/******** reg: GPINP ********/

/* bits 7:0: INPVAL */
#define ATLAS_GPINP_INPVAL_SHF	    0
#define ATLAS_GPINP_INPVAL_MSK	    (MSK(8) << ATLAS_GPINP_INPVAL_SHF)



/************************************************************************
 *      PCI definitions
 ************************************************************************/

/* ADP bit used as IDSEL during configuration cycles */
#define ATLAS_IDSEL_21150	    25
#define ATLAS_IDSEL_SYM53C810A	    26
#define ATLAS_IDSEL_CORE	    27
#define ATLAS_IDSEL_CONNECTOR	    28
#define ATLAS_IDSEL_SAA9730	    29


/**** Interrupt lines for ATLAS devices (on interrupt controller) ****/

/* PCI INTA..D       */
#define ATLAS_INTLINE_PCIA	    15
#define ATLAS_INTLINE_PCIB	    16
#define ATLAS_INTLINE_PCIC	    17
#define ATLAS_INTLINE_PCID	    18
/* Compact PCI A..D  */
#define ATLAS_INTLINE_CPCIA	    8
#define ATLAS_INTLINE_CPCIB	    9
#define ATLAS_INTLINE_CPCIC	    10
#define ATLAS_INTLINE_CPCID	    11
/* Local PCI devices */
#define ATLAS_INTLINE_9730	    ATLAS_INTLINE_PCIB
#define ATLAS_INTLINE_SCSI	    ATLAS_INTLINE_PCIC
/* Other devices     */
#define ATLAS_INTLINE_COREHI        5

/**** CPU interrupt lines used by devices ****/
#define ATLAS_CPUINT_ICTA	    (S_StatusIM2 - S_StatusIM)


/************************************************************************
 *      MISC definitions
 ************************************************************************/

/* Lowest possible frequency for cpu (used during init for conservative
 * setup of timing (e.g. SDRAM refresh)
 */
#define ATLAS_CPUFREQ_LOWEST_MHZ     2

/* PCI device numbers */
#define ATLAS_DEVNUM_SAA9730       PCI_IDSEL2DEVNUM(ATLAS_IDSEL_SAA9730)
#define ATLAS_DEVNUM_PCI_SLOT	   PCI_IDSEL2DEVNUM(ATLAS_IDSEL_CONNECTOR)

#endif /* #ifndef ATLAS_H */
