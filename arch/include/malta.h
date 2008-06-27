/************************************************************************
 *
 *      malta.h
 *
 *      The 'Malta' module defines the Malta board specific
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

#ifndef MALTA_H
#define MALTA_H

/************************************************************************
 *  Note: all addresses are physical ! 
 *  Must be mapped into kseg0 (system RAM) or kseg1 (registers, devices)
*************************************************************************/

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <piix4.h>
#include <pci_api.h>
#include <sys_api.h>
#include <mips.h>

/************************************************************************
 *  Malta, EEPROM devices,  IIC-slaveadresses
*************************************************************************/

#define MALTA_EEPROM_IICADR_SN      0x54 /* EEPROM with serial number   */
#define MALTA_EEPROM_IICADR_SPD000  0x50 /* PC-SDRAM, 256 bytes         */


/************************************************************************
 *  Malta physical address MAP (512 MByte)
*************************************************************************/

#define MALTA_SYSTEMRAM_BASE      0x00000000  /* System RAM:          */
#define MALTA_SYSTEMRAM_SIZE      0x10000000  /*   256 MByte          */
 
#define MALTA_GT64120_BASE        0x1BE00000  /* GT64120:             */
#define MALTA_GT64120_SIZE	  0x00200000  /*     2 MByte          */

#define MALTA_CBUS_BASE		  0x1C000000  /* CBUS:		      */
#define MALTA_CBUS_SIZE		  0x04000000  /*    64 MByte          */

#define MALTA_MONITORFLASH_BASE   0x1E000000  /* Monitor Flash:       */
#define MALTA_MONITORFLASH_SIZE   0x003E0000  /*     4 MByte          */
#define MALTA_MONITORFLASH_SECTORSIZE 0x00010000 /* Sect. = 64 KB     */

#define MALTA_RESERVED_BASE2      0x1E400000  /* Reserved:            */
#define MALTA_RESERVED_SIZE2      0x00C00000  /*    12 MByte          */

#define MALTA_FPGA_BASE           0x1F000000  /* FPGA:                */
#define MALTA_FPGA_SIZE           0x00C00000  /*    12 MByte          */

#define MALTA_BOOTROM_BASE        0x1FC00000  /* Boot ROM:            */
#define MALTA_BOOTROM_SIZE        0x00400000  /*     4 MByte          */

/************************************************************************
 *  Use of PCI ranges (Memory vs I/O)
*************************************************************************/

#define GT64120_PCIMEM_BASE		0x10000000  /* PCI memory:          */
#define GT64120_PCIMEM_SIZE		0x0be00000  /*  128 + 2 + 60 MByte  */

#define GT64120_PCIIO_BASE		0x18000000  /* PCI IO:              */
#define GT64120_PCIIO_SIZE		0x00200000  /*     2 MByte          */

#define BONITO_PCIMEM_BASE		0x10000000  /* PCI memory:          */
#define BONITO_PCIMEM_SIZE		0x0c000000  /*    96 Mbyte          */

#define BONITO_PCIIO_BASE		0x1fd00000  /* PCI IO:              */
#define BONITO_PCIIO_SIZE		0x00100000  /*     1 MByte          */

#define MSC01_PCIMEM_BASE		0x10000000  /* PCI memory:          */
#define MSC01_PCIMEM_SIZE		0x08000000  /*    64 Mbyte          */

#define MSC01_PCIIO_BASE		0x1b000000   /* PCI IO:              */
#define MSC01_PCIIO_SIZE		0x00800000  /*    16 Mbyte          */

#define SOCITSC_PCIMEM_BASE		0x10000000  /* PCI memory:          */
#define SOCITSC_PCIMEM_SIZE		0x08000000  /*   128 MByte          */

/* FIXME... could be smaller? */
#define SOCITSC_PCIIO_BASE		0x17000000  /* PCI IO:              */
#define SOCITSC_PCIIO_SIZE		0x01000000  /*    16 Mbyte          */

/************************************************************************
 * Malta FILEFLASH: upper 128 KByte (2x64 KByte sectors) of MONITORFLASH
*************************************************************************/

#define MALTA_FILEFLASH_BASE       0x1E3E0000 /* File Flash:           */
#define MALTA_FILEFLASH_SIZE       0x00020000 /*   128 KByte           */

#define MALTA_FILEFLASH_SECTORSIZE 0x00010000 /* Sect. = 64 KB	       */


/************************************************************************
 *  Malta FPGA, register address map:
*************************************************************************/

/************************************************************************
 *  REVISION: MUST BE FIXED TO  0x1FC00010 on any baseboard
*************************************************************************/

/************************************************************************
 *      Register Addresses
*************************************************************************/

#define MALTA_REVISION            0x1FC00010 /* REVISION              */
#define MALTA_SWITCH              0x1F000200 /* SWITCH                */
#define MALTA_STATUS              0x1F000208 /* STATUS                */
#define MALTA_JMPRS               0x1F000210 /* JMPRS                 */
#define MALTA_NMISTATUS           0x1F000024 /* NMISTATUS	      */
#define MALTA_NMIACK              0x1F000104 /* NMIACK                */
#define MALTA_LEDBAR              0x1F000408 /* LEDBAR       bit 7:0  */
#define MALTA_ASCIIWORD           0x1F000410 /* ASCIIWORD    bit 32:0 */
#define MALTA_ASCIIPOS0           0x1F000418 /* ASCIIPOS0    bit 7:0  */
#define MALTA_ASCIIPOS1           0x1F000420 /* ASCIIPOS1    bit 7:0  */
#define MALTA_ASCIIPOS2           0x1F000428 /* ASCIIPOS2    bit 7:0  */
#define MALTA_ASCIIPOS3           0x1F000430 /* ASCIIPOS3    bit 7:0  */
#define MALTA_ASCIIPOS4           0x1F000438 /* ASCIIPOS4    bit 7:0  */
#define MALTA_ASCIIPOS5           0x1F000440 /* ASCIIPOS5    bit 7:0  */
#define MALTA_ASCIIPOS6           0x1F000448 /* ASCIIPOS6    bit 7:0  */
#define MALTA_ASCIIPOS7           0x1F000450 /* ASCIIPOS7    bit 7:0  */
#define MALTA_SOFTRES             0x1F000500 /* SOFTRES               */
#define MALTA_BRKRES              0x1F000508 /* BRKRES                */
#define MALTA_TI16C550		  0x1F000900 /* TI16C550 UART (tty2)  */
#define MALTA_GPOUT               0x1F000A00 /* GPOUT                 */
#define MALTA_GPINP               0x1F000A08 /* GPINP                 */
#define MALTA_I2CINP              0x1F000b00 /* I2CINP                */
#define MALTA_I2COE               0x1F000b08 /* I2COE                 */
#define MALTA_I2COUT              0x1F000b10 /* I2COUT                */
#define MALTA_I2CSEL		  0x1F000b18 /* I2CSEL		      */
#define MALTA_SOFTEND		  0x1F000C00 /* Soft Endianness       */


/************************************************************************
 *      Register field encodings
*************************************************************************/


/******** reg: REVISION ********/

/* field: SCON */
#define MALTA_REVISION_SCON_SHF	  24
#define MALTA_REVISION_SCON_MSK	  (MSK(8) << MALTA_REVISION_SCON_SHF)

/* field: FPGRV */
#define MALTA_REVISION_FPGRV_SHF  16
#define MALTA_REVISION_FPGRV_MSK  (MSK(8) << MALTA_REVISION_FPGRV_SHF)

/* field: CORID */
#define MALTA_REVISION_CORID_SHF  10
#define MALTA_REVISION_CORID_MSK  (MSK(6) << MALTA_REVISION_CORID_SHF)

/* field: CORRV */
#define MALTA_REVISION_CORRV_SHF  8
#define MALTA_REVISION_CORRV_MSK  (MSK(2) << MALTA_REVISION_CORRV_SHF)

/* field: PROID */
#define MALTA_REVISION_PROID_SHF  4
#define MALTA_REVISION_PROID_MSK  (MSK(4) << MALTA_REVISION_PROID_SHF)

/* field: PRORV */
#define MALTA_REVISION_PRORV_SHF  0
#define MALTA_REVISION_PRORV_MSK  (MSK(4) << MALTA_REVISION_PRORV_SHF)


/******** reg: NMISTATUS ********/

/* bit 1: SOUTHBRIDGE */
#define MALTA_NMISTATUS_SB_SHF    1
#define MALTA_NMISTATUS_SB_MSK    (MSK(1) << MALTA_NMISTATUS_SB_SHF)
#define MALTA_NMISTATUS_SB_BIT    MALTA_NMISTATUS_SB_MSK

/* bit 0: PUSHBUTTON */
#define MALTA_NMISTATUS_ONNMI_SHF 0
#define MALTA_NMISTATUS_ONNMI_MSK (MSK(1) << MALTA_NMISTATUS_ONNMI_SHF)
#define MALTA_NMISTATUS_ONNMI_BIT MALTA_NMISTATUS_ONNMI_MSK


/******** reg: NMIACK ********/

/* bit 0: NMI */
#define MALTA_NMIACK_ONNMI_SHF    0
#define MALTA_NMIACK_ONNMI_MSK    (MSK(1) << MALTA_NMIACK_ONNMI_SHF)
#define MALTA_NMIACK_ONNMI_BIT    MALTA_NMIACK_ONNMI_MSK


/******** reg: SWITCH ********/

/* bits 7:0: S2 */
#define MALTA_SWITCH_S2_SHF      0
#define MALTA_SWITCH_S2_MSK      (MSK(8) << MALTA_SWITCH_S2_SHF)


/******** reg: STATUS ********/

/* bit 4 : MFWR */
#define MALTA_STATUS_MFWR_SHF     4
#define MALTA_STATUS_MFWR_MSK     (MSK(1) << MALTA_STATUS_MFWR_SHF)
#define MALTA_STATUS_MFWR_BIT     MALTA_STATUS_MFWR_MSK

/* bit 3 : S54 */
#define MALTA_STATUS_S54_SHF      3
#define MALTA_STATUS_S54_MSK      (MSK(1) << MALTA_STATUS_S54_SHF)
#define MALTA_STATUS_S54_SET      MALTA_STATUS_S54_MSK

/* bit 2 : S53 */
#define MALTA_STATUS_S53_SHF      2
#define MALTA_STATUS_S53_MSK      (MSK(1) << MALTA_STATUS_S53_SHF)
#define MALTA_STATUS_S53_SET      MALTA_STATUS_S53_MSK

/* bit 1: BIGEND */
#define MALTA_STATUS_BIGEND_SHF   1
#define MALTA_STATUS_BIGEND_MSK   (MSK(1) << MALTA_STATUS_BIGEND_SHF)
#define MALTA_STATUS_BIGEND_BIT   MALTA_STATUS_BIGEND_MSK


/******** reg: JMPRS ********/

/* bit 4:2: PCICLK */
#define MALTA_JMPRS_PCICLK_SHF      2
#define MALTA_JMPRS_PCICLK_MSK      (MSK(3) << MALTA_JMPRS_PCICLK_SHF)
#define MALTA_JMPRS_PCICLK_10MHZ    7
#define MALTA_JMPRS_PCICLK_12_5MHZ  4
#define MALTA_JMPRS_PCICLK_16_67MHZ 5
#define MALTA_JMPRS_PCICLK_20MHZ    1
#define MALTA_JMPRS_PCICLK_25MHZ    2
#define MALTA_JMPRS_PCICLK_30MHZ    3
#define MALTA_JMPRS_PCICLK_33_33MHZ 0
#define MALTA_JMPRS_PCICLK_37_5MHZ  6

/* bit 1: EELOCK */
#define MALTA_JMPRS_EELOCK_SHF      1
#define MALTA_JMPRS_EELOCK_MSK      (MSK(1) << MALTA_JMPRS_EELOCK_SHF)
#define MALTA_JMPRS_EELOCK_BIT      MALTA_JMPRS_EELOCK_MSK


/******** reg: SOFTRES ********/

/* bits 7:0: RESET */
#define MALTA_SOFTRES_RESET_SHF     0
#define MALTA_SOFTRES_RESET_MSK     (MSK(8) << MALTA_SOFTRES_RESET_SHF)
#define MALTA_SOFTRES_RESET_GORESET 0x42     /* magic value to reset    */    


/******** reg: BRKRES ********/

/* bits 7:0: WIDTH */
#define MALTA_BRKRES_WIDTH_SHF      0
#define MALTA_BRKRES_WIDTH_MSK      (MSK(8) << MALTA_BRKRES_WIDTH_SHF)


/******** reg: GPOUT ********/

/* bits 7:0: OUTVAL */
#define MALTA_GPOUT_OUTVAL_SHF      0
#define MALTA_GPOUT_OUTVAL_MSK      (MSK(8) << MALTA_GPOUT_OUTVAL_SHF)


/******** reg: GPINP ********/

/* bits 7:0: INPVAL */
#define MALTA_GPINP_INPVAL_SHF      0
#define MALTA_GPINP_INPVAL_MSK      (MSK(8) << MALTA_GPINP_INPVAL_SHF)


/******** reg: I2CINP ********/

#define MALTA_I2CINP_I2CSCL_SHF     1
#define MALTA_I2CINP_I2CSCL_MSK     (MSK(1) << MALTA_I2CINP_I2CSCL_SHF)
#define MALTA_I2CINP_I2CSCL_BIT     MALTA_I2CINP_I2CSCL_MSK

#define MALTA_I2CINP_I2CSDA_SHF     0
#define MALTA_I2CINP_I2CSDA_MSK     (MSK(1) << MALTA_I2CINP_I2CSDA_SHF)
#define MALTA_I2CINP_I2CSDA_BIT     MALTA_I2CINP_I2CSDA_MSK


/******** reg: I2COE ********/

#define MALTA_I2COE_I2CSCL_SHF      1
#define MALTA_I2COE_I2CSCL_MSK      (MSK(1) << MALTA_I2COE_I2CSCL_SHF)
#define MALTA_I2COE_I2CSCL_BIT      MALTA_I2COE_I2CSCL_MSK

#define MALTA_I2COE_I2CSDA_SHF      0
#define MALTA_I2COE_I2CSDA_MSK      (MSK(1) << MALTA_I2COE_I2CSDA_SHF)
#define MALTA_I2COE_I2CSDA_BIT      MALTA_I2COE_I2CSDA_MSK


/******** reg: I2COUT ********/

#define MALTA_I2COUT_I2CSCL_SHF     1
#define MALTA_I2COUT_I2CSCL_MSK     (MSK(1) << MALTA_I2COUT_I2CSCL_SHF)
#define MALTA_I2COUT_I2CSCL_BIT     MALTA_I2COUT_I2CSCL_MSK

#define MALTA_I2COUT_I2CSDA_SHF     0
#define MALTA_I2COUT_I2CSDA_MSK     (MSK(1) << MALTA_I2COUT_I2CSDA_SHF)
#define MALTA_I2COUT_I2CSDA_BIT     MALTA_I2COUT_I2CSDA_MSK


/******** reg: I2CSEL ********/

#define MALTA_I2CSEL_FPGA_SHF       0
#define MALTA_I2CSEL_FPGA_MSK       (MSK(1) << MALTA_I2CSEL_FPGA_SHF)
#define MALTA_I2CSEL_FPGA_BIT       MALTA_I2CSEL_FPGA_MSK


/******** reg: SOFTEND *******/

/* field: VALID */
#define MALTA_SOFTEND_VALID_SHF	    31
#define MALTA_SOFTEND_VALID_MSK     (MSK(1) << MALTA_SOFTEND_VALID_SHF)
#define MALTA_SOFTEND_VALID_BIT	    MALTA_SOFTEND_VALID_MSK

/* field: DONE */
#define MALTA_SOFTEND_DONE_SHF      1
#define MALTA_SOFTEND_DONE_MSK      (MSK(1) << MALTA_SOFTEND_DONE_SHF)
#define MALTA_SOFTEND_DONE_BIT      MALTA_SOFTEND_DONE_MSK

/* field: SYSRESET */
#define MALTA_SOFTEND_SYSRESET_SHF  0
#define MALTA_SOFTEND_SYSRESET_MSK  (MSK(1) << MALTA_SOFTEND_SYSRESET_SHF)
#define MALTA_SOFTEND_SYSRESET_BIT  MALTA_SOFTEND_SYSRESET_MSK


/************************************************************************
 *      PCI definitions
 ************************************************************************/

/*  IO addresses (Physical addresses) for UINT8 access.
 *  Macro MALTA_PCI_IO_ADDR8 swaps the 2 lsb bits of the addr in case 
 *  CPU is running Big Endian. 
 *  This is since PIIX4 is a PCI device, and PCI is always Little Endian.
 */

#ifndef _ASSEMBLER_
extern UINT32      malta_pci_io_base;
#endif

#define MALTA_PCI0_IO_BASE         malta_pci_io_base
#define MALTA_PCI0_ADDR( addr )    (MALTA_PCI0_IO_BASE + (addr))
#define MALTA_PCI_IO_ADDR8( addr ) SWAP_BYTEADDR_EL(MALTA_PCI0_ADDR(addr))

/* PIIX4 South Bridge IO access (KSEG1) */
/* piix4_pci_io_base is declared locally to get better code generation:     */
/* volatile UINT8 *piix4_pci_io_base =                                      */
/*      (volatile UINT8 *)KSEG1(MALTA_PCI0_IO_BASE + PIIX4_IO_BRIDGE_START) */
#define MALTA_PIIX4_IO8( addr )    piix4_pci_io_base[ SWAP_BYTEADDR_EL(addr) ]

/* SMSC SuperIO access (KSEG1). Same base as PIIX4 */
#define MALTA_SUPER_IO8( addr )    MALTA_PIIX4_IO8( addr )

/*  PIIX4 RTC (Physical addresses).
 *  Corrected for endianness !
 */
#define MALTA_RTCADR               MALTA_PCI_IO_ADDR8(0x70)
#define MALTA_RTCDAT               MALTA_PCI_IO_ADDR8(0x71)

/*  SuperIO UARTs (Physical addresses). 
 *  Not corrected for endianness (address swapping) since this is done by
 *  driver.
 */
#define MALTA_SMSC_COM1_ADR        0x3f8
#define MALTA_SMSC_COM2_ADR	   0x2f8
#define MALTA_UART0ADR		   MALTA_PCI0_ADDR(MALTA_SMSC_COM1_ADR)
#define MALTA_UART1ADR		   MALTA_PCI0_ADDR(MALTA_SMSC_COM2_ADR)

/*  SuperIO Parallel port (Physical addresses). 
 *  Not corrected for endianness (address swapping) since this is done by
 *  driver.
 */
#define MALTA_SMSC_1284_ADR	   0x378
#define MALTA_1284ADR		   MALTA_PCI0_ADDR(MALTA_SMSC_1284_ADR)

/*  SuperIO Floppy disk (Physical addresses). 
 *  Not corrected for endianness (address swapping) since this is done by
 *  driver.
 */
#define MALTA_SMSC_FDD_ADR	   0x3f0
#define MALTA_FDDADR		   MALTA_PCI0_ADDR(MALTA_SMSC_FDD_ADR)


/*  SuperIO Keyboard, Mouse (Physical addresses). 
 *  Not corrected for endianness (address swapping) since this is done by
 *  driver.
 */
#define MALTA_SMSC_KYBD_ADR	   0x60  /* Fixed 0x60, 0x64 */
#define MALTA_KYBDADR		   MALTA_PCI0_ADDR(MALTA_SMSC_KYBD_ADR)
#define MALTA_SMSC_MOUSE_ADR	   MALTA_SMSC_KYBD_ADR
#define MALTA_MOUSEADR		   MALTA_KYBDADR

/* ADP bit used as IDSEL during configuration cycles */

#define MALTA_IDSEL_SLOT1	   28
#define MALTA_IDSEL_SLOT2	   29
#define MALTA_IDSEL_SLOT3	   30
#define MALTA_IDSEL_SLOT4	   31
#define MALTA_IDSEL_PIIX4	   20
#define MALTA_IDSEL_AM79C973       21
#define MALTA_IDSEL_CRYSTAL	   22

/* PCI device numbers */
#define MALTA_DEVNUM_PIIX4        PCI_IDSEL2DEVNUM(MALTA_IDSEL_PIIX4)
#define MALTA_DEVNUM_AM79C973     PCI_IDSEL2DEVNUM(MALTA_IDSEL_AM79C973)
#define MALTA_DEVNUM_CRYSTAL      PCI_IDSEL2DEVNUM(MALTA_IDSEL_CRYSTAL)
#define MALTA_DEVNUM_PCI_SLOT1    PCI_IDSEL2DEVNUM(MALTA_IDSEL_SLOT1)
#define MALTA_DEVNUM_PCI_SLOT2    PCI_IDSEL2DEVNUM(MALTA_IDSEL_SLOT2)
#define MALTA_DEVNUM_PCI_SLOT3    PCI_IDSEL2DEVNUM(MALTA_IDSEL_SLOT3)
#define MALTA_DEVNUM_PCI_SLOT4    PCI_IDSEL2DEVNUM(MALTA_IDSEL_SLOT4)

/**** IRQ lines for Malta devices ****/

/* PCI INTA..D       */
#define MALTA_INTLINE_PCIA	  10
#define MALTA_INTLINE_PCIB	  10
#define MALTA_INTLINE_PCIC	  11
#define MALTA_INTLINE_PCID	  11
/* PCI slot 1        */
#define MALTA_INTLINE_SLOT1_A	  MALTA_INTLINE_PCIA
#define MALTA_INTLINE_SLOT1_B     MALTA_INTLINE_PCIB
#define MALTA_INTLINE_SLOT1_C     MALTA_INTLINE_PCIC
#define MALTA_INTLINE_SLOT1_D     MALTA_INTLINE_PCID
/* PCI slot 2        */
#define MALTA_INTLINE_SLOT2_A	  MALTA_INTLINE_PCIB
#define MALTA_INTLINE_SLOT2_B	  MALTA_INTLINE_PCIC
#define MALTA_INTLINE_SLOT2_C	  MALTA_INTLINE_PCID
#define MALTA_INTLINE_SLOT2_D	  MALTA_INTLINE_PCIA
/* PCI slot 3        */
#define MALTA_INTLINE_SLOT3_A	  MALTA_INTLINE_PCIC
#define MALTA_INTLINE_SLOT3_B	  MALTA_INTLINE_PCID
#define MALTA_INTLINE_SLOT3_C	  MALTA_INTLINE_PCIA
#define MALTA_INTLINE_SLOT3_D	  MALTA_INTLINE_PCIB
/* PCI slot 4        */
#define MALTA_INTLINE_SLOT4_A	  MALTA_INTLINE_PCID
#define MALTA_INTLINE_SLOT4_B	  MALTA_INTLINE_PCIA
#define MALTA_INTLINE_SLOT4_C	  MALTA_INTLINE_PCIB
#define MALTA_INTLINE_SLOT4_D	  MALTA_INTLINE_PCIC
/* PIIX4/SuperIO devices */
#define MALTA_INTLINE_KYBD	  1
#define MALTA_INTLINE_TTY1	  3
#define MALTA_INTLINE_TTY0	  4
#define MALTA_INTLINE_FDD	  6
#define MALTA_INTLINE_1284	  7
#define MALTA_INTLINE_MOUSE	  12

/* Local PCI devices */
#define MALTA_INTLINE_79C973	  MALTA_INTLINE_PCIB
#define MALTA_INTLINE_4281	  MALTA_INTLINE_PCIC
#define MALTA_INTLINE_PIIX4_USB   MALTA_INTLINE_PCID
#define MALTA_INTLINE_PIIX4_SMB   9

/**** CPU interrupt lines  ****/
#define MALTA_CPUINT_PIIX4	  (S_StatusIM2 - S_StatusIM)
#define MALTA_CPUINT_COREHI	  (S_StatusIM5 - S_StatusIM)
#define MALTA_CPUINT_SMI	  (S_StatusIM3 - S_StatusIM)

/**** CPU EIC interrupt levels  ****/
#define MALTA_EICINT_SW0	  1
#define MALTA_EICINT_SW1	  2
#define MALTA_EICINT_PIIX4	  3
#define MALTA_EICINT_SMI	  4
#define MALTA_EICINT_MB2	  5 /* Not used */
#define MALTA_EICINT_MB3	  6 /* Not used */
#define MALTA_EICINT_COREHI	  7
#define MALTA_EICINT_MSC01TMR	  8
#define MALTA_EICINT_MSC01PCI	  9
#define MALTA_EICINT_PERFCTR	  10
#define MALTA_EICINT_CPUCTR	  11


/************************************************************************
 *      MISC definitions
 ************************************************************************/

/* Lowest possible frequency for cpu (used during init for conservative
 * setup of timing (e.g. SDRAM refresh)
 */
#define MALTA_CPUFREQ_LOWEST_MHZ  2


#endif /* #ifndef MALTA_H */









