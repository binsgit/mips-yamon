
/************************************************************************
 *
 *      sead.h
 *
 *      The 'SEAD' module defines the SEAD board specific
 *      address-MAP, registers, etc.
 *
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

#ifndef SEAD_H
#define  SEAD_H

/************************************************************************
 *  Note: all addresses are physical !
 *  Must be mapped into kseg0 (system RAM) or kseg1 (registers, devices)
*************************************************************************/

/************************************************************************
 *  Include files
 ************************************************************************/

#include "sysdefs.h"
#include <msc01_memmap.h>
#include <msc01_biu.h>
#include <msc01_ic.h>
#include <msc01_pbc.h>
#include <msc01_mc.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* The following mapping matches the msc01_system1 testbench used in
 * the SOC-it deliverables.  In this testbench, the IP blocks are attached
 * to the kernel as follows:
 *
 *   IP1:  "Testbench Monitor", not used in a real system
 *   IP2:  AHB bridge, PCI bridge, or nothing, depending on testbench build
 *   IP3:  Peripheral bus bridge.
 *
 * This configuration is reflected by the defines below:
 */
#define MSC01_AHB_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP2)
#define MSC01_PCI_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP2)
#define MSC01_PBC_REG_BASE	MSC01_REGADDR(MSC01_REGSPACE_IP3)


/************************************************************************
 ************************************************************************
 *  SEAD physical address MAP (512 MByte)
 ************************************************************************
 ************************************************************************/

#define  SEAD_SYSTEMRAM_BASE         	0x00000000 /* System RAM:     */
#define  SEAD_SYSTEMRAM_SIZE         	0x10000000 /*   256 MByte     */

#define  SEAD_RESERVED1_BASE         	0x10000000 /* Reserved:       */
#define  SEAD_RESERVED1_SIZE         	0x0B000000 /*   176 MByte     */

#define  SEAD_SD_BASE       	  	0x1B000000 /* SDRAM control:  */
#define  SEAD_SD_SIZE       	  	0x00100000 /*     1 MByte     */

#define  SEAD_GP_BASE              	0x1B100000 /* GPIO module:    */
#define  SEAD_GP_SIZE              	0x00100000 /*     1 MByte     */

#define  SEAD_PF_BASE                	0x1B200000 /* PF module:      */
#define  SEAD_PF_SIZE                	0x00100000 /*     1 MByte     */

#define  SEAD_RESERVED2_BASE         	0x1B300000 /* Reserved:       */
#define  SEAD_RESERVED2_SIZE         	0x00D00000 /*    13 MByte     */

#define  SEAD_SYSTEMFLASH_BASE       	0x1C000000 /* System Flash:   */
#define  SEAD_SYSTEMFLASH_SIZE       	0x01FC0000 /*  31.75 MByte    */

#define  SEAD_SYSTEMFLASH_SECTORSIZE 	0x00020000 /* Sect. = 128 KB  */
#define  SEAD_SYSTEMFLASH_BANKCOUNT   	         2 /*                 */
#define  SEAD_SYSTEMFLASH_BLOCKCOUNT   	        64 /*                 */


#define  SEAD_FILEFLASH_BASE         	0x1DFC0000 /* File Flash:     */
#define  SEAD_FILEFLASH_SIZE         	0x00040000 /*   256 KByte     */

#define  SEAD_FILEFLASH_SECTORSIZE   	0x00020000 /* Sect. = 128 KB  */

#define  SEAD_SRAM_BASE              	0x1E000000 /* System RAM:     */
#define  SEAD_SRAM_SIZE              	0x00400000 /*     4 MByte     */

#define  SEAD_RESERVED3_BASE         	0x1E400000 /* Reserved:       */
#define  SEAD_RESERVED3_SIZE         	0x00400000 /*     4 MByte     */

#define  SEAD_PI_BASE           	0x1E800000 /* PI-bus ctrl:    */
#define  SEAD_PI_SIZE           	0x00400000 /*     4 MByte     */

#define  SEAD_RESERVED4_BASE         	0x1EC00000 /* Reserved:       */
#define  SEAD_RESERVED4_SIZE         	0x00400000 /*     4 MByte     */

#define  SEAD_DEVICES_BASE           	0x1F000000 /* Devices:        */
#define  SEAD_DEVICES_SIZE           	0x00C00000 /*    12 MByte     */

#define  SEAD_BOOTPROM_BASE          	0x1FC00000 /* Boot PROM:      */
#define  SEAD_BOOTPROM_SIZE          	0x00100000 /*     1 MByte     */


/************************************************************************
 *  SEAD, peripheral bus devices, base adresses:
 ************************************************************************/

#define  SEAD_HDSP2532_BASE          	0x1F000400 /* ASCII display	*/
#define  SEAD_TI16C550_DEV0_BASE     	0x1F000800 /* TI16C550 UART0	*/
#define  SEAD_TI16C550_DEV1_BASE     	0x1F000900 /* TI16C550 UART1	*/


/************************************************************************
 ************************************************************************
 *  SEAD FPGA, PERIPHERAL REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *  REVISION: MUST BE FIXED TO  0x1FC00010 on any baseboard
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_REVISION      	     	0x1FC00010 /* REVISION		*/


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: REVISION ********/

/* field: RTLID */
#define SEAD_REVISION_RTLID_SHF		26
#define SEAD_REVISION_RTLID_MSK		(MSK(6) << SEAD_REVISION_RTLID_SHF)
#define SEAD_REVISION_RTLID_BASIC	0x1
#define SEAD_REVISION_RTLID_SOCIT101	0x2

/* field: RTLMAJ */
#define SEAD_REVISION_RTLMAJ_SHF	19
#define SEAD_REVISION_RTLMAJ_MSK	(MSK(7) << SEAD_REVISION_RTLMAJ_SHF)

/* field: RTLMIN */
#define SEAD_REVISION_RTLMIN_SHF	12
#define SEAD_REVISION_RTLMIN_MSK	(MSK(7) << SEAD_REVISION_RTLMIN_SHF)

/* field: PROID */
#define SEAD_REVISION_PROID_SHF		4
#define SEAD_REVISION_PROID_MSK		(MSK(4) << SEAD_REVISION_PROID_SHF)

/* field: PRORV */
#define SEAD_REVISION_PRORV_SHF		0
#define SEAD_REVISION_PRORV_MSK		(MSK(4) << SEAD_REVISION_PRORV_SHF)


/************************************************************************
 *  PERIPHERAL BUS SWITCHES (P-SWITCH):
*************************************************************************/

/************************************************************************
 *      P-SW Register Addresses
*************************************************************************/

#define SEAD_PSWITCH		   	0x1F000200 /* P-SWITCH         	*/


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: PSWITCH ********/
/* bits 7:0: VAL */
#define SEAD_PSWITCH_VAL_SHF	    	0
#define SEAD_PSWITCH_VAL_MSK	    	(MSK(8) << SEAD_PSWITCH_VAL_SHF)


/* bit 0: S3-1 */
#define SEAD_PSWITCH_SW1_SHF	    	0
#define SEAD_PSWITCH_SW1_MSK	    	(MSK(1) << SEAD_PSWITCH_SW1_SHF)
#define SEAD_PSWITCH_SW1_ON           	SEAD_PSWITCH_SW1_MSK

/* bit 1: S3-2 */
#define SEAD_PSWITCH_SW2_SHF	    	1
#define SEAD_PSWITCH_SW2_MSK	    	(MSK(1) << SEAD_PSWITCH_SW2_SHF)
#define SEAD_PSWITCH_SW2_ON           	SEAD_PSWITCH_SW2_MSK

/* bit 2: S3-3 */
#define SEAD_PSWITCH_SW3_SHF	    	2
#define SEAD_PSWITCH_SW3_MSK	    	(MSK(1) << SEAD_PSWITCH_SW3_SHF)
#define SEAD_PSWITCH_SW3_ON           	SEAD_PSWITCH_SW3_MSK

/* bit 3: S3-4 */
#define SEAD_PSWITCH_SW4_SHF	    	3
#define SEAD_PSWITCH_SW4_MSK	    	(MSK(1) << SEAD_PSWITCH_SW4_SHF)
#define SEAD_PSWITCH_SW4_ON           	SEAD_PSWITCH_SW4_MSK

/* bit 4: S3-5 */
#define SEAD_PSWITCH_SW5_SHF	    	4
#define SEAD_PSWITCH_SW5_MSK	    	(MSK(1) << SEAD_PSWITCH_SW5_SHF)
#define SEAD_PSWITCH_SW5_ON           	SEAD_PSWITCH_SW5_MSK

/* bit 5: S3-6 */
#define SEAD_PSWITCH_SW6_SHF	    	5
#define SEAD_PSWITCH_SW6_MSK	    	(MSK(1) << SEAD_PSWITCH_SW6_SHF)
#define SEAD_PSWITCH_SW6_ON           	SEAD_PSWITCH_SW6_MSK

/* bit 6: S3-7 */
#define SEAD_PSWITCH_SW7_SHF	    	6
#define SEAD_PSWITCH_SW7_MSK	  	(MSK(1) << SEAD_PSWITCH_SW7_SHF)
#define SEAD_PSWITCH_SW7_ON           	SEAD_PSWITCH_SW7_MSK

/* bit 7: S3-8 */
#define SEAD_PSWITCH_SW8_SHF	    	7
#define SEAD_PSWITCH_SW8_MSK	  	(MSK(1) << SEAD_PSWITCH_SW8_SHF)
#define SEAD_PSWITCH_SW8_ON           	SEAD_PSWITCH_SW8_MSK


/************************************************************************
 *  PERIPHERAL BUS LEDs (P-LED):
*************************************************************************/

/************************************************************************
 *      P-LED Register Addresses
*************************************************************************/

#define SEAD_PLED		   	0x1F000210 /* P-LED         	*/


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: PLED ********/
/* bits 7:0: VAL */
#define SEAD_PLED_VAL_SHF	    	0
#define SEAD_PLED_VAL_MSK	    	(MSK(8) << SEAD_PLED_VAL_SHF)


/* bit 0:  */
#define SEAD_PLED_BIT0_SHF	    	0
#define SEAD_PLED_BIT0_MSK	    	(MSK(1) << SEAD_PLED_BIT0_SHF)
#define SEAD_PLED_BIT0_ON           	SEAD_PLED_BIT0_MSK

/* bit 1:  */
#define SEAD_PLED_BIT1_SHF	    	1
#define SEAD_PLED_BIT1_MSK	    	(MSK(1) << SEAD_PLED_BIT1_SHF)
#define SEAD_PLED_BIT1_ON           	SEAD_PLED_BIT1_MSK

/* bit 2:  */
#define SEAD_PLED_BIT2_SHF	    	2
#define SEAD_PLED_BIT2_MSK	    	(MSK(1) << SEAD_PLED_BIT2_SHF)
#define SEAD_PLED_BIT2_ON           	SEAD_PLED_BIT2_MSK

/* bit 3:  */
#define SEAD_PLED_BIT3_SHF	    	3
#define SEAD_PLED_BIT3_MSK	    	(MSK(1) << SEAD_PLED_BIT3_SHF)
#define SEAD_PLED_BIT3_ON           	SEAD_PLED_BIT3_MSK

/* bit 4:  */
#define SEAD_PLED_BIT4_SHF	    	4
#define SEAD_PLED_BIT4_MSK	    	(MSK(1) << SEAD_PLED_BIT4_SHF)
#define SEAD_PLED_BIT4_ON           	SEAD_PLED_BIT4_MSK

/* bit 5:  */
#define SEAD_PLED_BIT5_SHF	    	5
#define SEAD_PLED_BIT5_MSK	    	(MSK(1) << SEAD_PLED_BIT5_SHF)
#define SEAD_PLED_BIT5_ON           	SEAD_PLED_BIT5_MSK

/* bit 6:  */
#define SEAD_PLED_BIT6_SHF	    	6
#define SEAD_PLED_BIT6_MSK	    	(MSK(1) << SEAD_PLED_BIT6_SHF)
#define SEAD_PLED_BIT6_ON           	SEAD_PLED_BIT6_MSK

/* bit 7:  */
#define SEAD_PLED_BIT7_SHF	    	7
#define SEAD_PLED_BIT7_MSK	    	(MSK(1) << SEAD_PLED_BIT7_SHF)
#define SEAD_PLED_BIT7_ON           	SEAD_PLED_BIT7_MSK


/************************************************************************
 *  ASCII DISPLAY
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_ASCIIPOS0          	0x1F0005C0
#define SEAD_ASCIIPOS1          	0x1F0005C8
#define SEAD_ASCIIPOS2          	0x1F0005D0
#define SEAD_ASCIIPOS3          	0x1F0005D8
#define SEAD_ASCIIPOS4          	0x1F0005E0
#define SEAD_ASCIIPOS5          	0x1F0005E8
#define SEAD_ASCIIPOS6          	0x1F0005F0
#define SEAD_ASCIIPOS7          	0x1F0005F8


/************************************************************************
 ************************************************************************
 *  SEAD FPGA, PERIPHERAL BUS CONTROLLER REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_PI_TIMSRAM             	(SEAD_PI_BASE + 0x0010)
#define SEAD_PI_TIMOTHER            	(SEAD_PI_BASE + 0x0020)


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: PI_TIMSRAM ********/

/* bit 2:0 : */
#define SEAD_PI_TIMSRAM_CS1_SHF		0
#define SEAD_PI_TIMSRAM_CS1_MSK		(MSK(3) << SEAD_PI_TIMSRAM_CS1_SHF)

/* bit 7:3 : */
#define SEAD_PI_TIMSRAM_CS2_SHF		3
#define SEAD_PI_TIMSRAM_CS2_MSK		(MSK(5) << SEAD_PI_TIMSRAM_CS2_SHF)

/* bit 10:8 : */
#define SEAD_PI_TIMSRAM_RD1_SHF		8
#define SEAD_PI_TIMSRAM_RD1_MSK		(MSK(3) << SEAD_PI_TIMSRAM_RD1_SHF)

/* bit 16:11 : */
#define SEAD_PI_TIMSRAM_RD2_SHF		11
#define SEAD_PI_TIMSRAM_RD2_MSK		(MSK(6) << SEAD_PI_TIMSRAM_RD2_SHF)

/* bit 19:17 : */
#define SEAD_PI_TIMSRAM_WE1_SHF		17
#define SEAD_PI_TIMSRAM_WE1_MSK		(MSK(3) << SEAD_PI_TIMSRAM_WE1_SHF)

/* bit 25:20 : */
#define SEAD_PI_TIMSRAM_WE2_SHF		20
#define SEAD_PI_TIMSRAM_WE2_MSK		(MSK(6) << SEAD_PI_TIMSRAM_WE2_SHF)

/* bit 31:26 : */
#define SEAD_PI_TIMSRAM_ADH_SHF		26
#define SEAD_PI_TIMSRAM_ADH_MSK		(MSK(6) << SEAD_PI_TIMSRAM_ADH_SHF)


/******** reg: PI_TIMOTHER ********/

/* bit 2:0 : */
#define SEAD_PI_TIMOTHER_CS1_SHF	0
#define SEAD_PI_TIMOTHER_CS1_MSK	(MSK(3) << SEAD_PI_TIMOTHER_CS1_SHF)

/* bit 7:3 : */
#define SEAD_PI_TIMOTHER_CS2_SHF	3
#define SEAD_PI_TIMOTHER_CS2_MSK	(MSK(5) << SEAD_PI_TIMOTHER_CS2_SHF)

/* bit 10:8 : */
#define SEAD_PI_TIMOTHER_RD1_SHF	8
#define SEAD_PI_TIMOTHER_RD1_MSK	(MSK(3) << SEAD_PI_TIMOTHER_RD1_SHF)

/* bit 16:11 : */
#define SEAD_PI_TIMOTHER_RD2_SHF	11
#define SEAD_PI_TIMOTHER_RD2_MSK	(MSK(6) << SEAD_PI_TIMOTHER_RD2_SHF)

/* bit 19:17 : */
#define SEAD_PI_TIMOTHER_WE1_SHF	17
#define SEAD_PI_TIMOTHER_WE1_MSK	(MSK(3) << SEAD_PI_TIMOTHER_WE1_SHF)

/* bit 25:20 : */
#define SEAD_PI_TIMOTHER_WE2_SHF	20
#define SEAD_PI_TIMOTHER_WE2_MSK	(MSK(6) << SEAD_PI_TIMOTHER_WE2_SHF)

/* bit 31:26 : */
#define SEAD_PI_TIMOTHER_ADH_SHF	26
#define SEAD_PI_TIMOTHER_ADH_MSK	(MSK(6) << SEAD_PI_TIMOTHER_ADH_SHF)



/************************************************************************
 ************************************************************************
 *  SEAD FPGA, GENERAL CONTROL REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *  NMI STATUS, ACK AND PSU STANDBY CONTROL :
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_NMISTATUS              	(SEAD_PI_BASE + 0x0040)
#define SEAD_NMIACK                 	(SEAD_PI_BASE + 0x0048)

/*  Address SEAD_PI_BASE + 0x50 is used for different purposes on
 *  SEAD and SEAD-2 boards.
 *  On SEAD it is used for SW Power Off.
 *  On SEAD-2 it is used for SW reset.
 */
#define SEAD_PSUSTBY                	(SEAD_PI_BASE + 0x0050) 
#define SEAD2_SWRESET			(SEAD_PI_BASE + 0x0050) 

/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: NMISTATUS ********/

/* bit 0: */
#define SEAD_NMISTATUS_FLAG_SHF		0
#define SEAD_NMISTATUS_FLAG_MSK		(MSK(1) << SEAD_NMISTATUS_FLAG_SHF)
#define SEAD_NMISTATUS_FLAG_BIT  	SEAD_NMISTATUS_FLAG_MSK

/* bit 1: */
#define SEAD_NMISTATUS_WERR_SHF		1
#define SEAD_NMISTATUS_WERR_MSK		(MSK(1) << SEAD_NMISTATUS_WERR_SHF)
#define SEAD_NMISTATUS_WERR_BIT  	SEAD_NMISTATUS_WERR_MSK

/******** reg: NMIACK ********/

/* bit 0: */
#define SEAD_NMIACK_ACK_SHF		0
#define SEAD_NMIACK_ACK_MSK		(MSK(1) << SEAD_NMIACK_ACK_SHF)
#define SEAD_NMIACK_ACK_BIT  		SEAD_NMIACK_ACK_MSK


/******** reg: PSUSTBY (SEAD only)  ********/

/* bits 7:0: STBY */
#define SEAD_PSUSTBY_STBY_SHF	    	0
#define SEAD_PSUSTBY_STBY_MSK	    	(MSK(8) << SEAD_PSUSTBY_STBY_SHF)
#define SEAD_PSUSTBY_STBY_GOSTBY   	0x4D     /* magic value to go stby  */


/******** reg: SWRESET (SEAD-2 only) ********/

#define SEAD2_SWRESET_SWRST_SHF	        0
#define SEAD2_SWRESET_SWRST_MSK		(MSK(8) << SEAD2_SWRESET_SWRST_SHF)
#define SEAD2_SWRESET_SWRST_GORESET	0x4D


/************************************************************************
 ************************************************************************
 *  SEAD FPGA, SDRAM CONTROLLER REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_SD_REGSSET			(SEAD_SD_BASE + 0x0008)
#define SEAD_SD_MRSCODE			(SEAD_SD_BASE + 0x0010)
#define SEAD_SD_CONFIG			(SEAD_SD_BASE + 0x0018)
#define SEAD_SD_LATENCIES		(SEAD_SD_BASE + 0x0020)
#define SEAD_SD_TREFRESH		(SEAD_SD_BASE + 0x0028)
#define SEAD_SD_SPDCNF			(SEAD_SD_BASE + 0x0040)
#define SEAD_SD_SPDADR			(SEAD_SD_BASE + 0x0048)
#define SEAD_SD_SPDDAT			(SEAD_SD_BASE + 0x0050)

/************************************************************************
 *      Register field encodings
*************************************************************************/

/******** reg: SD_REGSSET ********/

#define SEAD_SD_REGSSET_SET_SHF		0
#define SEAD_SD_REGSSET_SET_MSK		(MSK(1) << SEAD_SD_REGSSET_SET_SHF)
#define SEAD_SD_REGSSET_SET_BIT	        SEAD_SD_REGSSET_SET_MSK


/******** reg: SD_MRSCODE ********/

#define SEAD_SD_MRSCODE_WBL_SHF		9
#define SEAD_SD_MRSCODE_WBL_MSK		(MSK(1) << SEAD_SD_MRSCODE_WBL_SHF)
#define SEAD_SD_MRSCODE_TM_SHF		7
#define SEAD_SD_MRSCODE_TM_MSK		(MSK(2) << SEAD_SD_MRSCODE_TM_SHF)
#define SEAD_SD_MRSCODE_CL_SHF		4
#define SEAD_SD_MRSCODE_CL_MSK		(MSK(3) << SEAD_SD_MRSCODE_CL_SHF)
#define SEAD_SD_MRSCODE_BT_SHF		3
#define SEAD_SD_MRSCODE_BT_MSK		(MSK(1) << SEAD_SD_MRSCODE_BT_SHF)
#define SEAD_SD_MRSCODE_BL_SHF		0
#define SEAD_SD_MRSCODE_BL_MSK		(MSK(3) << SEAD_SD_MRSCODE_BL_SHF)


/******** reg: SD_CONFIG ********/

#define SEAD_SD_CONFIG_CSN_SHF		12
#define SEAD_SD_CONFIG_CSN_MSK        	(MSK(4) << SEAD_SD_CONFIG_CSN_SHF)
#define SEAD_SD_CONFIG_BANKN_SHF	8
#define SEAD_SD_CONFIG_BANKN_MSK	(MSK(4) << SEAD_SD_CONFIG_BANKN_SHF)
#define SEAD_SD_CONFIG_ROWW_SHF		4
#define SEAD_SD_CONFIG_ROWW_MSK		(MSK(4) << SEAD_SD_CONFIG_ROWW_SHF)
#define SEAD_SD_CONFIG_COLW_SHF		0
#define SEAD_SD_CONFIG_COLW_MSK		(MSK(4) << SEAD_SD_CONFIG_COLW_SHF)


/******** reg: SD_LATENCIES ********/

#define SEAD_SD_LATENCIES_TRP_SHF	12
#define SEAD_SD_LATENCIES_TRP_MSK	(MSK(4) << SEAD_SD_LATENCIES_TRP_SHF)
#define SEAD_SD_LATENCIES_TRP_MIN	2
#define SEAD_SD_LATENCIES_TRP_MAX	4
#define SEAD_SD_LATENCIES_TRAS_SHF	8
#define SEAD_SD_LATENCIES_TRAS_MSK	(MSK(4) << SEAD_SD_LATENCIES_TRAS_SHF)
#define SEAD_SD_LATENCIES_TRCD_SHF	4
#define SEAD_SD_LATENCIES_TRCD_MSK	(MSK(4) << SEAD_SD_LATENCIES_TRCD_SHF)
#define SEAD_SD_LATENCIES_TRCD_MIN	2
#define SEAD_SD_LATENCIES_TRCD_MAX	3
#define SEAD_SD_LATENCIES_TDPL_SHF	0
#define SEAD_SD_LATENCIES_TDPL_MSK	(MSK(4) << SEAD_SD_LATENCIES_TDPL_SHF)


/******** reg: SD_TREFRESH ********/

#define SEAD_SD_TREFRESH_TREF_SHF	0
#define SEAD_SD_TREFRESH_TREF_MSK	(MSK(11) << SEAD_SD_TREFRESH_TREF_SHF)


/******** reg: SD_SPDCNF ********/

#define SEAD_SD_SPDCNF_CPDIV_SHF	0
#define SEAD_SD_SPDCNF_CPDIV_MSK	(MSK(10) << SEAD_SD_SPDCNF_CPDIV_SHF)


/******** reg: SD_SPDADR ********/

#define SEAD_SD_SPDADR_RDADR_SHF	0
#define SEAD_SD_SPDADR_RDADR_MSK	(MSK(8) << SEAD_SD_SPDADR_RDADR_SHF)


/******** reg: SD_SPDDAT ********/

#define SEAD_SD_SPDDAT_READ_ERR_SHF	9
#define SEAD_SD_SPDDAT_READ_ERR_MSK	(MSK(1) << SEAD_SD_SPDDAT_READ_ERR_SHF)
#define SEAD_SD_SPDDAT_READ_ERR_BIT	SEAD_SD_SPDDAT_READ_ERR_MSK

#define SEAD_SD_SPDDAT_BUSY_SHF		8
#define SEAD_SD_SPDDAT_BUSY_MSK		(MSK(1) << SEAD_SD_SPDDAT_BUSY_SHF)
#define SEAD_SD_SPDDAT_BUSY_BIT		SEAD_SD_SPDDAT_BUSY_MSK

#define SEAD_SD_SPDDAT_RDATA_SHF	0
#define SEAD_SD_SPDDAT_RDATA_MSK	(MSK(8) << SEAD_SD_SPDDAT_RDATA_SHF)





/************************************************************************
 ************************************************************************
 *  SEAD FPGA, GENERAL PURPOSE INPUT/OUTPUT REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_GP_DIRRAWB0		(SEAD_GP_BASE + 0x00C0)
#define SEAD_GP_DIRRAWB1		(SEAD_GP_BASE + 0x00C8)
#define SEAD_GP_DIRRAWA0		(SEAD_GP_BASE + 0x00D0)
#define SEAD_GP_DIRRAWA1		(SEAD_GP_BASE + 0x00D8)

#define SEAD_GP_INB0			(SEAD_GP_BASE + 0x0100)
#define SEAD_GP_INB1			(SEAD_GP_BASE + 0x0108)
#define SEAD_GP_INA0			(SEAD_GP_BASE + 0x0110)
#define SEAD_GP_INA1			(SEAD_GP_BASE + 0x0118)

#define SEAD_GP_OUTRAWB0		(SEAD_GP_BASE + 0x01C0)
#define SEAD_GP_OUTRAWB1		(SEAD_GP_BASE + 0x01C8)
#define SEAD_GP_OUTRAWA0		(SEAD_GP_BASE + 0x01D0)
#define SEAD_GP_OUTRAWA1		(SEAD_GP_BASE + 0x01D8)


#define SEAD_GP_DIRINB0			(SEAD_GP_BASE + 0x0040)
#define SEAD_GP_DIRINB1			(SEAD_GP_BASE + 0x0048)
#define SEAD_GP_DIRINA0			(SEAD_GP_BASE + 0x0050)
#define SEAD_GP_DIRINA1			(SEAD_GP_BASE + 0x0058)

#define SEAD_GP_DIROUTB0		(SEAD_GP_BASE + 0x0080)
#define SEAD_GP_DIROUTB1		(SEAD_GP_BASE + 0x0088)
#define SEAD_GP_DIROUTA0		(SEAD_GP_BASE + 0x0090)
#define SEAD_GP_DIROUTA1		(SEAD_GP_BASE + 0x0098)


#define SEAD_GP_OUTSETB0		(SEAD_GP_BASE + 0x0140)
#define SEAD_GP_OUTSETB1		(SEAD_GP_BASE + 0x0148)
#define SEAD_GP_OUTSETA0		(SEAD_GP_BASE + 0x0150)
#define SEAD_GP_OUTSETA1		(SEAD_GP_BASE + 0x0158)

#define SEAD_GP_OUTCLRB0		(SEAD_GP_BASE + 0x0180)
#define SEAD_GP_OUTCLRB1		(SEAD_GP_BASE + 0x0188)
#define SEAD_GP_OUTCLRA0		(SEAD_GP_BASE + 0x0190)
#define SEAD_GP_OUTCLRA1		(SEAD_GP_BASE + 0x0198)


/************************************************************************
 *      Register field encodings
*************************************************************************/

/* 1 bit per pin.
 * "B0" controls UC_B[31:0]
 * "B1" controls UC_B[63:32]
 * "A0" controls UC_A[31:0]
 * "A1" controls UC_A[39:32]
 */



/************************************************************************
 ************************************************************************
 *  SEAD FPGA, PERFORMANCE & F-SW/F-LED MODULE REGISTERS:
 ************************************************************************
 ************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define SEAD_PF_MODE			(SEAD_PF_BASE + 0x0000)
#define SEAD_PF_FSWITCH			(SEAD_PF_BASE + 0x0020)
#define SEAD_PF_FLED			(SEAD_PF_BASE + 0x0040)
#define SEAD_PF_SPEED			(SEAD_PF_BASE + 0x0060)

/************************************************************************
 *      Register field encodings
*************************************************************************/

/******** reg: PF_MODE ********/

#define SEAD_PF_MODE_FORCE_SHF		0
#define SEAD_PF_MODE_FORCE_MSK		(MSK(1) << SEAD_PF_MODE_FORCE_SHF)
#define SEAD_PF_MODE_FORCE_BIT	        SEAD_PF_MODE_FORCE_MSK

#define SEAD_PF_MODE_MVAL_SHF		1
#define SEAD_PF_MODE_MVAL_MSK		(MSK(3) << SEAD_PF_MODE_MVAL_SHF)
#define SEAD_PF_MODE_MVAL_SPEED		0	/* speed-o-meter mode	*/
#define SEAD_PF_MODE_MVAL_HTMSS		1	/* hit/miss mode	*/
#define SEAD_PF_MODE_MVAL_SIHT		2	/* sysint/hit mode	*/
#define SEAD_PF_MODE_MVAL_CCHIC		3	/* cache/instcompl mode	*/
#define SEAD_PF_MODE_MVAL_FLED		7	/* PF_FLED mode		*/


/******** reg: PF_FSWITCH ********/
/* bits 7:0: VAL */
#define SEAD_PF_FSWITCH_VAL_SHF	    	0
#define SEAD_PF_FSWITCH_VAL_MSK	    	(MSK(8) << SEAD_PF_FSWITCH_VAL_SHF)


/* bit 0: S5-1 */
#define SEAD_PF_FSWITCH_SW1_SHF	    	0
#define SEAD_PF_FSWITCH_SW1_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW1_SHF)
#define SEAD_PF_FSWITCH_SW1_ON          SEAD_PF_FSWITCH_SW1_MSK

/* bit 1: S5-2 */
#define SEAD_PF_FSWITCH_SW2_SHF	    	1
#define SEAD_PF_FSWITCH_SW2_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW2_SHF)
#define SEAD_PF_FSWITCH_SW2_ON          SEAD_PF_FSWITCH_SW2_MSK

/* bit 2: S5-3 */
#define SEAD_PF_FSWITCH_SW3_SHF	    	2
#define SEAD_PF_FSWITCH_SW3_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW3_SHF)
#define SEAD_PF_FSWITCH_SW3_ON          SEAD_PF_FSWITCH_SW3_MSK

/* bit 3: S5-4 */
#define SEAD_PF_FSWITCH_SW4_SHF	    	3
#define SEAD_PF_FSWITCH_SW4_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW4_SHF)
#define SEAD_PF_FSWITCH_SW4_ON          SEAD_PF_FSWITCH_SW4_MSK

/* bit 4: S5-5 */
#define SEAD_PF_FSWITCH_SW5_SHF	    	4
#define SEAD_PF_FSWITCH_SW5_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW5_SHF)
#define SEAD_PF_FSWITCH_SW5_ON          SEAD_PF_FSWITCH_SW5_MSK

/* bit 5: S5-6 */
#define SEAD_PF_FSWITCH_SW6_SHF	    	5
#define SEAD_PF_FSWITCH_SW6_MSK	    	(MSK(1) << SEAD_PF_FSWITCH_SW6_SHF)
#define SEAD_PF_FSWITCH_SW6_ON          SEAD_PF_FSWITCH_SW6_MSK

/* bit 6: S5-7 */
#define SEAD_PF_FSWITCH_SW7_SHF	    	6
#define SEAD_PF_FSWITCH_SW7_MSK	  	(MSK(1) << SEAD_PF_FSWITCH_SW7_SHF)
#define SEAD_PF_FSWITCH_SW7_ON          SEAD_PF_FSWITCH_SW7_MSK

/* bit 7: S5-8 */
#define SEAD_PF_FSWITCH_SW8_SHF	    	7
#define SEAD_PF_FSWITCH_SW8_MSK	  	(MSK(1) << SEAD_PF_FSWITCH_SW8_SHF)
#define SEAD_PF_FSWITCH_SW8_ON          SEAD_PF_FSWITCH_SW8_MSK


/******** reg: PF_FLED ********/
/* bits 7:0: VAL */
#define SEAD_PF_FLED_VAL_SHF	    	0
#define SEAD_PF_FLED_VAL_MSK	    	(MSK(8) << SEAD_PF_FLED_VAL_SHF)


/* bit 0:  */
#define SEAD_PF_FLED_BIT0_SHF	    	0
#define SEAD_PF_FLED_BIT0_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT0_SHF)
#define SEAD_PF_FLED_BIT0_ON           	SEAD_PF_FLED_BIT0_MSK

/* bit 1:  */
#define SEAD_PF_FLED_BIT1_SHF	    	1
#define SEAD_PF_FLED_BIT1_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT1_SHF)
#define SEAD_PF_FLED_BIT1_ON           	SEAD_PF_FLED_BIT1_MSK

/* bit 2:  */
#define SEAD_PF_FLED_BIT2_SHF	    	2
#define SEAD_PF_FLED_BIT2_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT2_SHF)
#define SEAD_PF_FLED_BIT2_ON           	SEAD_PF_FLED_BIT2_MSK

/* bit 3:  */
#define SEAD_PF_FLED_BIT3_SHF	    	3
#define SEAD_PF_FLED_BIT3_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT3_SHF)
#define SEAD_PF_FLED_BIT3_ON           	SEAD_PF_FLED_BIT3_MSK

/* bit 4:  */
#define SEAD_PF_FLED_BIT4_SHF	    	4
#define SEAD_PF_FLED_BIT4_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT4_SHF)
#define SEAD_PF_FLED_BIT4_ON           	SEAD_PF_FLED_BIT4_MSK

/* bit 5:  */
#define SEAD_PF_FLED_BIT5_SHF	    	5
#define SEAD_PF_FLED_BIT5_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT5_SHF)
#define SEAD_PF_FLED_BIT5_ON           	SEAD_PF_FLED_BIT5_MSK

/* bit 6:  */
#define SEAD_PF_FLED_BIT6_SHF	    	6
#define SEAD_PF_FLED_BIT6_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT6_SHF)
#define SEAD_PF_FLED_BIT6_ON           	SEAD_PF_FLED_BIT6_MSK

/* bit 7:  */
#define SEAD_PF_FLED_BIT7_SHF	    	7
#define SEAD_PF_FLED_BIT7_MSK	    	(MSK(1) << SEAD_PF_FLED_BIT7_SHF)
#define SEAD_PF_FLED_BIT7_ON           	SEAD_PF_FLED_BIT7_MSK


/******** reg: PF_SPEED ********/

#define SEAD_PF_SPEED_VAL_SHF		0
#define SEAD_PF_SPEED_VAL_MSK		(MSK(8) << SEAD_PF_SPEED_VAL_SHF)



/************************************************************************
 ************************************************************************
 * Physical address setup for sead2_msc01
 ************************************************************************
 ************************************************************************/

#define SEAD_MSC01_MEMORY_BASE	SEAD_SYSTEMRAM_BASE  /* MEM */
#define SEAD_MSC01_MEMORY_SIZE	SEAD_SYSTEMRAM_SIZE

#define SEAD_MSC01_IP2MEM_BASE	0x1b400000  /* IP2 (unused) must point somewhere */
#define SEAD_MSC01_IP2MEM_SIZE	0x00400000

#define SEAD_MSC01_IP1MEM_BASE	0x1b800000  /* IP1 (unused) must point somewhere */
#define SEAD_MSC01_IP1MEM_SIZE	0x00400000
 
#define SEAD_MSC01_REG_BASE	0x1bc00000
#define SEAD_MSC01_REG_SIZE	0x00400000

#define SEAD_MSC01_PBCMEM_BASE	0x1c000000  /* IP3 */
#define SEAD_MSC01_PBCMEM_SIZE	0x04000000
#define SEAD_MSC01_IP3MEM_SIZE	0x00400000  /* temporary size during start up */


/**** IRQ definitions for SEAD MSC01 devices ****/

#define SEAD_MSC01_INTLINE_COUNTER	0
#define SEAD_MSC01_INTLINE_TTY0		2
#define SEAD_MSC01_INTLINE_TTY1		3
#define SEAD_MSC01_IC_COUNT		4

#define SEAD_MSC01_CPUINT		(S_StatusIM2 - S_StatusIM)


/**** SW reset definitions for SEAD MSC01 devices ****/

#define SEAD2_MSC01_MC_RESET_SHF        15
#define SEAD2_MSC01_MC_RESET_MSK	(MSK(1) << SEAD2_MSC01_MC_RESET_SHF)
#define SEAD2_MSC01_MC_RESET_BIT	SEAD2_MSC01_MC_RESET_MSK


/************************************************************************
 ************************************************************************
 *      MISC definitions
 ************************************************************************
 ************************************************************************/

/*
 *  SEAD temporary hack to set the processor clock for further
 *  calculations
 */
#define SEAD_MIN_FREQ_MHZ		2
#define SEAD_MAX_FREQ_MHZ		25


#endif /* #ifndef SEAD_H */
