
/************************************************************************
 *
 *  pci_api.h
 *
 *  PCI functions API
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


#ifndef PCI_API_H
#define PCI_API_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <syserror.h>

/************************************************************************
 *  Definitions
 ************************************************************************/


/* Known vendors and device IDs */
#define PCI_VENDID_GALILEO		0x11ab
#define PCI_DEVID_64120			0x4620

#define PCI_VENDID_PHILIPS		0x1131
#define PCI_DEVID_9730			0x9730

#define PCI_VENDID_INTEL_A		0x1011
#define PCI_DEVID_PPB			0x0022

#define PCI_VENDID_INTEL		0x8086
#define PCI_DEVID_PIIX4_BRIDGE		0x7110
#define PCI_DEVID_PIIX4_IDE		0x7111
#define PCI_DEVID_PIIX4_USB		0x7112
#define PCI_DEVID_PIIX4_POWER		0x7113

#define PCI_VENDID_SYMBIOS		0x1000
#define PCI_DEVID_SCSI			0x0001

#define PCI_VENDID_AMD                  0x1022
#define PCI_DEVID_79C973                0x2000

#define PCI_VENDID_CRYSTAL		0x1013
#define PCI_DEVID_4281			0x6005

#define PCI_VENDID_ALGORITHMICS		0xdf53	/* TBD : Likely to change */
#define PCI_DEVID_BONITO64		0x0001	/* TBD : May change	  */

#define PCI_VENDID_MIPS			0x153f
#define PCI_DEVID_MSC01			0x0001	/* TBD : May change	  */

/* Local bus number */
#define PCI_BUS_LOCAL			0


/*  Mapping between PCI device number and address bit 
 *  (ADP<n>) used for IDSEL is :
 *  
 *  Device number 1 maps to ADP11
 *  Device number 2 maps to ADP12
 *  ...
 *  Device number 21 maps to ADP31
 *
 *  In other words :
 *
 *  Address bit = Device number + 10 (Device number = 1..21)
 */

#define PCI_DEVNUM_MAX		  21
#define PCI_DEVNUM2IDSEL_OFFSET	  10
#define PCI_IDSEL2DEVNUM(idsel)	  ((idsel)  - PCI_DEVNUM2IDSEL_OFFSET)
#define PCI_DEVNUM2IDSEL(devnum)  ((devnum) + PCI_DEVNUM2IDSEL_OFFSET)

#ifndef _ASSEMBLER_


/* PCI BAR definition */

typedef struct pci_bar
{
    struct pci_bar  *next;	/* Reserved (linked list ptr)		*/

    bool	    fixed;	/* TRUE -> Fixed location BAR		*/
    UINT8           pos;	/* BAR position				*/
    UINT8           io;		/* 1 -> IO mapped, 0 -> Memory mapped   */
    UINT8	    prefetch;   /* Prefetch field obtained from device	*/
    UINT32	    start;	/* PCI side start address of range	*/
    UINT32	    size;	/* Size of range			*/
}
t_pci_bar;


/* PCI device definition */

typedef struct
{
    UINT8      bus;
    UINT8      dev;
    UINT8      function;
    UINT8      ht;
    UINT8      multi;
    UINT8      lat_tim;
    UINT16     devid;
    UINT16     vendorid;
    UINT16     status;
    UINT8      max_lat;
    UINT8      min_gnt;
    UINT8      intpin;
    UINT8      intline;

    bool       alloc_err;	/* TRUE -> No ressources allocated */
    UINT8      bar_count;	/* Number of active BARs	*/

/* Array of BARs */
#define PCI_BAR_MAXCOUNT	6
    t_pci_bar  bar[PCI_BAR_MAXCOUNT];
}
t_pci_cfg_dev;


/* Error codes */

#define ERROR_PCI            0x0000f000 /* for compile time check    */
#define ERROR_PCI_MALLOC         0xf000
#define ERROR_PCI_RANGE          0xf001
#define ERROR_PCI_UNKNOWN_DEVICE 0xf002
#define ERROR_PCI_ABORT          0xf003
#define ERROR_PCI_STRUCTURE      0xf004


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          pci_init
 *  Description :
 *  -------------
 *
 *  PCI module initialisation function
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
pci_init( void );


/************************************************************************
 *
 *                          pci_lookup_bar
 *  Description :
 *  -------------
 *
 *  Determine base address (physical) of specific BAR of specific device.
 *
 *  If more than one device fits the IDs and function number, the
 *  device is selected based on the following priority :
 *
 *  1) Known local (bus 0) device
 *  2) First unknown local (bus 0) device found (based on device number)
 *  3) First unknown remote (bus != 0) device found (based on device number)
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE  : BAR found
 *  FALSE : BAR not found
 *
 ************************************************************************/
bool
pci_lookup_bar(
    UINT8         busnum,		/* Bus number			*/
    UINT8         devnum,		/* Device number		*/
    UINT8	  function,		/* Function number		*/
    UINT8	  bar,			/* Address of BAR register	*/
    void	  **param );		/* OUT : Base address of BAR	*/


/************************************************************************
 *
 *                          pci_display
 *  Description :
 *  -------------
 *
 *  Display devices detected during autodetect and display the 
 *  autoconfiguration data (memory space and IO space allocations)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
pci_display( void );


/************************************************************************
 *
 *                          pci_busfreq_string
 *  Description :
 *  -------------
 *
 *  Format string containing PCI bus freqency in format xx.yy MHz
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
pci_busfreq_string(
    char   *msg,
    UINT32 freq_kHz );


/************************************************************************
 *
 *                          pci_config_read32/16/8
 *  Description :
 *  -------------
 *
 *  Low level pci configuration space read routines (32/16/8 bit)
 *
 *  Return values :
 *  ---------------
 *
 *  0 (No error) or ERROR_PCI_ABORT    
 *
 ************************************************************************/
UINT32
pci_config_read32(
    UINT32 busnum,	/* IN   bus number           */
    UINT32 devnum,	/* IN   device number        */
    UINT32 func,	/* IN   function number      */
    UINT32 reg,		/* IN   address		     */
    UINT32  *data );	/* OUT  pointer to data read */

UINT32
pci_config_read16(
    UINT32 busnum,	/* IN   bus number           */
    UINT32 devnum,	/* IN   device number        */
    UINT32 func,	/* IN   function number      */
    UINT32 reg,		/* IN   address		     */
    UINT16  *data );	/* OUT  pointer to data read */

UINT32
pci_config_read8(
    UINT32 busnum,	/* IN   bus number           */
    UINT32 devnum,	/* IN   device number        */
    UINT32 func,	/* IN   function number      */
    UINT32 reg,		/* IN   address		     */
    UINT8  *data );	/* OUT  pointer to data read */


/************************************************************************
 *
 *                          pci_config_write32/16/8
 *  Description :
 *  -------------
 *
 *  Low level pci configuration space write routines (32/16/8 bit)
 *
 *  Return values :
 *  ---------------
 *
 *  0 (No error) or ERROR_PCI_ABORT    
 *
 ************************************************************************/
UINT32
pci_config_write32(
    UINT32 busnum,	/* IN   bus number      */
    UINT32 devnum,	/* IN   device number   */
    UINT32 func,	/* IN   function number */
    UINT32 reg,		/* IN   address		*/
    UINT32 data );	/* IN   data to write   */

UINT32
pci_config_write16(
    UINT32 busnum,	/* IN   bus number      */
    UINT32 devnum,	/* IN   device number   */
    UINT32 func,	/* IN   function number */
    UINT32 reg,		/* IN   address		*/
    UINT16 data );	/* IN   data to write   */

UINT32
pci_config_write8(
    UINT32 busnum,	/* IN   bus number      */
    UINT32 devnum,	/* IN   device number   */
    UINT32 func,	/* IN   function number */
    UINT32 reg,		/* IN   address		*/
    UINT8  data );	/* IN   data to write   */



#endif /* #ifndef _ASSEMBLER_ */

#endif /* #ifndef PCI_API_H */



