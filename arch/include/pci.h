
/************************************************************************
 *
 *  PCI definitions
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


#ifndef PCI_H
#define PCI_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>

#ifndef _ASSEMBLER_
#include <pci_api.h>
#endif /* #ifndef _ASSEMBLER_ */

/************************************************************************
 *  PCI definitions
 ************************************************************************/

#define PCI_MAX_FUNC			8	/* Max functions 	*/
#define PCI_MAX_BUS			256	/* Max busses		*/
#define PCI_MAX_DEV			64	/* Max devices    	*/

/* Alignment requirements of memory/IO ranges set by PCI-PCI bridges	*/
#define PCI_ALIGN_IO		       (1 << 12) /* IO range		*/
#define PCI_ALIGN_MEM		       (1 << 20) /* Memory range	*/

/* Default latency timer */
#define PCI_LATTIM_FIXED        0x20

/************************************************************************
 *  Register numbers (derived by offset addresses) and access types
 ************************************************************************/

#define PCI_HEADERTYPE0			0
#define PCI_HEADERTYPE1			1
#define PCI_HEADERTYPE_MAX		1  /* No support for cardbus */

/* Common header (32 bit registers) */
#define PCI_ID				0x00
#define PCI_SC				0x04
#define PCI_CCREV			0x08
#define PCI_BHLC			0x0c

/* Common header (8 bit registers) */
#define PCI_LATTIM			0x0d

/* Header 0 and 1 (32 bit registers) */
#define PCI_BAR_MIN			0x10
#define PCI_BAR(number)			(PCI_BAR_MIN + (number)*4)

/* Header 0 and 1 (8 bit registers) */
#define PCI_CAP_PTR			0x34
#define PCI_INTLINE			0x3c

/* Header 0 (32 bit registers) */
#define PCI_BAR_MAX			0x24
#define PCI_MMII			0x3c

/* Header 1 (32 bit registers) */
#define PCI_BAR_MAX_PPB			0x14
#define PCI_SSSP			0x18
#define PCI_IO				0x1c
#define PCI_MEM				0x20
#define PCI_PREFMEM			0x24
#define PCI_UPPERIO			0x30
#define PCI_BCII			0x3c

/* Header 1 (16 bit registers) */
#define PCI_BC				0x3e

/* First addr. not belonging to standard header */
#define PCI_FIRST_NON_STANDARD		0x40

/************************************************************************
 *  Register encodings
 ************************************************************************/

/* SC */
#define PCI_SC_CMD_IOS_SHF		0
#define PCI_SC_CMD_IOS_MSK		(MSK(1) << PCI_SC_CMD_IOS_SHF)
#define PCI_SC_CMD_IOS_BIT		PCI_SC_CMD_IOS_MSK

#define PCI_SC_CMD_MS_SHF		1
#define PCI_SC_CMD_MS_MSK		(MSK(1) << PCI_SC_CMD_MS_SHF)
#define PCI_SC_CMD_MS_BIT		PCI_SC_CMD_MS_MSK

#define PCI_SC_CMD_BM_SHF		2
#define PCI_SC_CMD_BM_MSK		(MSK(1) << PCI_SC_CMD_BM_SHF)
#define PCI_SC_CMD_BM_BIT		PCI_SC_CMD_BM_MSK

#define PCI_SC_CMD_PERR_SHF		6
#define PCI_SC_CMD_PERR_MSK		(MSK(1) << PCI_SC_CMD_PERR_SHF)
#define PCI_SC_CMD_PERR_BIT		PCI_SC_CMD_PERR_MSK

#define PCI_SC_CMD_SERR_SHF		8
#define PCI_SC_CMD_SERR_MSK		(MSK(1) << PCI_SC_CMD_SERR_SHF)
#define PCI_SC_CMD_SERR_BIT		PCI_SC_CMD_SERR_MSK

#define PCI_SC_CMD_FBB_SHF		9
#define PCI_SC_CMD_FBB_MSK		(MSK(1) << PCI_SC_CMD_FBB_SHF)
#define PCI_SC_CMD_FBB_BIT		PCI_SC_CMD_FBB_MSK

/* STATUS */
#define PCI_STATUS_FBB_SHF		7
#define PCI_STATUS_FBB_MSK		(MSK(1) << PCI_STATUS_FBB_SHF)
#define PCI_STATUS_FBB_BIT		PCI_STATUS_FBB_MSK

#define PCI_STATUS_CAP_SHF		4
#define PCI_STATUS_CAP_MSK		(MSK(1) << PCI_STATUS_CAP_SHF)
#define PCI_STATUS_CAP_BIT		PCI_STATUS_CAP_MSK

/* BAR */
#define PCI_BAR_IO_SHF			0
#define PCI_BAR_IO_MSK			(MSK(1) << PCI_BAR_IO_SHF)
#define PCI_BAR_IO_BIT			PCI_BAR_IO_MSK

#define PCI_BAR_TYPE_SHF		1
#define PCI_BAR_TYPE_MSK		(MSK(2) << PCI_BAR_TYPE_SHF)
#define PCI_BAR_TYPE_32			0
#define PCI_BAR_TYPE_32_1M		1
#define PCI_BAR_TYPE_64			2
#define PCI_BAR_TYPE_RSVD		3

#define PCI_BAR_PREFETCH_SHF		3
#define PCI_BAR_PREFETCH_MSK		(MSK(1) << PCI_BAR_PREFETCH_SHF)

#define PCI_BAR_IOSIZE_SHF		2
#define PCI_BAR_IOSIZE_MSK		(MSK(30) << PCI_BAR_IOSIZE_SHF)

#define PCI_BAR_MEMSIZE_SHF		4
#define PCI_BAR_MEMSIZE_MSK		(MSK(28) << PCI_BAR_MEMSIZE_SHF)

/* ID */
#define PCI_ID_DEVID_SHF		16
#define PCI_ID_DEVID_MSK		(MSK(16) << PCI_ID_DEVID_SHF)

#define PCI_ID_VENDORID_SHF		0
#define PCI_ID_VENDORID_MSK		(MSK(16) << PCI_ID_VENDORID_SHF)

/* SC */
#define PCI_SC_STATUS_SHF		16
#define PCI_SC_STATUS_MSK		(MSK(16) << PCI_SC_STATUS_SHF)

#define PCI_SC_COMMAND_SHF		0
#define PCI_SC_COMMAND_MSK		(MSK(16) << PCI_SC_COMMAND_SHF)

/* Class Code and Revision ID (CCREV) */
#define PCI_CCREV_CC_SHF	        8	
#define PCI_CCREV_CC_MSK		(MSK(24) << PCI_CCREV_CC_SHF)

#define PCI_CCREV_REVID_SHF		0
#define PCI_CCREV_REVID_MSK		(MSK(8) << PCI_CCREV_REVID_SHF)

/* BIST, Header Type, Lat timer, Cache line size (BHLC) */
#define PCI_BHLC_BIST_SHF	       24
#define PCI_BHLC_BIST_MSK	       (MSK(8) << PCI_BHLC_BIST_SHF)
#define PCI_BHLC_HT_SHF		       16
#define PCI_BHLC_HT_MSK		       (MSK(7) << PCI_BHLC_HT_SHF)
#define PCI_BHLC_MULTI_SHF	       23
#define PCI_BHLC_MULTI_MSK	       (MSK(1) << PCI_BHLC_MULTI_SHF)
#define PCI_BHLC_MULTI_BIT	       PCI_BHLC_MULTI_MSK
#define PCI_BHLC_LT_SHF		       8
#define PCI_BHLC_LT_MSK		       (MSK(8) << PCI_BHLC_LT_SHF)
#define PCI_BHLC_CLS_SHF	       0
#define PCI_BHLC_CLS_MSK	       (MSK(8) << PCI_BHLC_CLS_SHF)

/* Max Lat, Min Gnt, Int pin, Int line (MMII) */
#define PCI_MMII_MAXLAT_SHF	       24
#define PCI_MMII_MAXLAT_MSK	       (MSK(8) << PCI_MMII_MAXLAT_SHF)
#define PCI_MMII_MINGNT_SHF	       16
#define PCI_MMII_MINGNT_MSK	       (MSK(8) << PCI_MMII_MINGNT_SHF)
#define PCI_MMII_INTPIN_SHF	       8
#define PCI_MMII_INTPIN_MSK	       (MSK(8) << PCI_MMII_INTPIN_SHF)

#define PCI_MMII_INTPIN_NU	       0
#define PCI_MMII_INTPIN_A	       1
#define PCI_MMII_INTPIN_B	       2
#define PCI_MMII_INTPIN_C	       3
#define PCI_MMII_INTPIN_D	       4

#define PCI_MMII_INTLINE_SHF           0
#define PCI_MMII_INTLINE_MSK	       (MSK(8) << PCI_MMII_INTLINE_SHF)
#define PCI_MMII_INTLINE_NONE	       0xFF

/* Extended capabilities */
#define PCI_EXT_CAP_ID_OFS			0
#define PCI_EXT_CAP_PWR_MGMT_STATUS_OFS		4
#define PCI_EXT_CAP_NEXT_PTR_OFS		1

#define PCI_EXT_CAP_ID_PWR_MGMT			0x01
#define PCI_EXT_CAP_PWR_MGMT_STATUS_D0		0x8000

/* Sec lat, sub bus num, sec bus number, prim bus number (SSSP) */
#define PCI_SSSP_SLT_SHF	       24
#define PCI_SSSP_SLT_MSK	       (MSK(8) << PCI_SSSP_SLT_SHF)
#define PCI_SSSP_SUBBN_SHF	       16
#define PCI_SSSP_SUBBN_MSK	       (MSK(8) << PCI_SSSP_SUBBN_SHF)
#define PCI_SSSP_SECBN_SHF             8
#define PCI_SSSP_SECBN_MSK	       (MSK(8) << PCI_SSSP_SECBN_SHF)
#define PCI_SSSP_PBN_SHF	       0
#define PCI_SSSP_PBN_MSK	       (MSK(8) << PCI_SSSP_PBN_SHF)

/* Bridge control, int pin, int line (BCII) */
#define PCI_BCII_BC_SHF		       16
#define PCI_BCII_BC_MSK		       (MSK(16) << PCI_BCII_BC_SHF)

#define PCI_BCII_BC_PERR_SHF	       (16 + 0)
#define PCI_BCII_BC_PERR_MSK	       (MSK(1) << PCI_BCII_BC_PERR_SHF)
#define PCI_BCII_BC_PERR_BIT	       PCI_BCII_BC_PERR_MSK

#define PCI_BCII_BC_SERR_SHF	       (16 + 1)
#define PCI_BCII_BC_SERR_MSK	       (MSK(1) << PCI_BCII_BC_SERR_SHF)
#define PCI_BCII_BC_SERR_BIT	       PCI_BCII_BC_SERR_MSK

#define PCI_BCII_BC_MA_SHF	       (16 + 5)
#define PCI_BCII_BC_MA_MSK	       (MSK(1) << PCI_BCII_BC_MA_SHF)
#define PCI_BCII_BC_MA_BIT	       PCI_BCII_BC_MA_MSK

#define PCI_BCII_BC_FBB_SHF	       (16 + 7)
#define PCI_BCII_BC_FBB_MSK	       (MSK(1) << PCI_BCII_BC_FBB_SHF)
#define PCI_BCII_BC_FBB_BIT	       PCI_BCII_BC_FBB_MSK

/* IO */
#define PCI_IO_BASE_SHF			0
#define PCI_IO_BASE_MSK			(MSK(8) << PCI_IO_BASE_SHF)
#define PCI_IO_LIMIT_SHF		8
#define PCI_IO_LIMIT_MSK		(MSK(8) << PCI_IO_LIMIT_SHF)

/* UpperIO */
#define PCI_UPPERIO_BASE_SHF		0
#define PCI_UPPERIO_BASE_MSK		(MSK(16) << PCI_UPPERIO_BASE_SHF)
#define PCI_UPPERIO_LIMIT_SHF		16
#define PCI_UPPERIO_LIMIT_MSK		(MSK(16) << PCI_UPPERIO_LIMIT_SHF)

/* Mem */
#define PCI_MEM_BASE_SHF		0
#define PCI_MEM_BASE_MSK		(MSK(16) << PCI_MEM_BASE_SHF)
#define PCI_MEM_LIMIT_SHF		16
#define PCI_MEM_LIMIT_MSK		(MSK(16) << PCI_MEM_LIMIT_SHF)

/* PrefMem */
#define PCI_PREFMEM_BASE_SHF		0
#define PCI_PREFMEM_BASE_MSK		(MSK(16) << PCI_PREFMEM_BASE_SHF)
#define PCI_PREFMEM_LIMIT_SHF		16
#define PCI_PREFMEM_LIMIT_MSK		(MSK(16) << PCI_PREFMEM_LIMIT_SHF)



/************************************************************************
 *  PCI configuration cycle AD bus definition
 ************************************************************************/

/* Type 0 */
#define PCI_CFG_TYPE0_REG_SHF		0
#define PCI_CFG_TYPE0_REG_MSK		(MSK(6) << 2)

#define PCI_CFG_TYPE0_FUNC_SHF		8
#define PCI_CFG_TYPE0_FUNC_MSK		(MSK(3) << PCI_CFG_TYPE0_FUNC_SHF)

/* Type 1 */
#define PCI_CFG_TYPE1_REG_SHF		0
#define PCI_CFG_TYPE1_REG_MSK		(MSK(6) << 2)

#define PCI_CFG_TYPE1_FUNC_SHF		8
#define PCI_CFG_TYPE1_FUNC_MSK		(MSK(3) << PCI_CFG_TYPE0_FUNC_SHF)

#define PCI_CFG_TYPE1_DEV_SHF		11
#define PCI_CFG_TYPE1_DEV_MSK		(MSK(5) << PCI_CFG_TYPE1_DEV_SHF)

#define PCI_CFG_TYPE1_BUS_SHF		16
#define PCI_CFG_TYPE1_BUS_MSK		(MSK(8) << PCI_CFG_TYPE1_BUS_SHF)


#ifndef _ASSEMBLER_


/*  Structures and function prototypes used locally in implementation 
 *  of PCI configuration. 
 */


/* Known device */
typedef struct
{
    UINT16  vendorid;	/* Vendor ID			 */
    UINT16  devid;      /* Device ID			 */
    UINT8   function;   /* Function number		 */
    UINT8   intline;    /* Interrupt line used by device */
    char    *vendor;    /* String holding vendor name    */
    char    *device;    /* String holding device name    */
}
t_known_dev;

/* Fixed request for BAR setup */
typedef struct
{
    UINT16	vendorid;	/* Vendor ID		*/
    UINT16	devid;		/* Device ID		*/
    UINT8	function;	/* Function number	*/
    t_pci_bar	bar;		/* Requirements for BAR	*/
}
t_pci_bar_req;


/************************************************************************
 *  Public functions
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
pci_config( void );


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
    t_known_dev   *controller );   /* Controller data to be filled out  */


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
 *  OK              :  If no error.
 *  ERROR_PCI_ABORT : If master abort (no target) or target abort.
 *
 ************************************************************************/
UINT32
arch_pci_config_access(
    UINT32 busnum,		/* PCI bus number (0 = local bus)	*/
    UINT32 devnum,		/* PCI device number			*/
    UINT32 func,		/* Function number of device		*/
    UINT32 reg,			/* Device register 			*/
    bool   write,		/* TRUE -> Config write, else read	*/
    UINT8  size,		/* Sizeof data (1/2/4 bytes)		*/
    void   *data );		/* write or read data			*/


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
arch_pci_system_slot( void );


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
    UINT8 *number );		/* OUT : PCI slot number		*/


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
    UINT8 intpin );		/* Int. pin (A/B/C/D) used by device	*/



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
    UINT8 intpin );		/* Int. pin (A/B/C/D) used by device	*/


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
    t_pci_cfg_dev *dev );	/* PCI device structure			*/


/************************************************************************
 *
 *                          arch_pci_multi
 *  Description :
 *  -------------
 *
 *  Extrace multi field from PCI configuration word 0xc
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
    UINT32	  bhlc );


/************************************************************************
 *
 *                          pci_autoconfig
 *  Description :
 *  -------------
 *
 *  Configure PCI based on platform requirements.
 *
 *  Return values :
 *  ---------------
 *
 *     0: no pci error, else error
 *
 ************************************************************************/
UINT32
pci_autoconfig(
    t_pci_bar_req  *bar_req,		/* Special BAR requirements	*/
    UINT32	   bar_req_count,	/* Number of special BARs       */

    t_known_dev    *known_dev,		/* Array of known devices	*/
    UINT32	   known_dev_count );	/* Count of known devices	*/


/************************************************************************
 *
 *                          pci_check_range
 *  Description :
 *  -------------
 *
 *  Validate PCI memory range
 *
 *  Return values :
 *  ---------------
 *
 *  OK              :  Range OK
 *  ERROR_PCI_RANGE : Range not OK
 *
 ************************************************************************/
UINT32
pci_check_range( 
    UINT32 addr,			/* Start of range		*/
    UINT32 size,			/* Size of range		*/
    UINT32 *mask );			/* Mask corresponding to range	*/


#endif /* #ifndef _ASSEMBLER_ */


#endif /* #ifndef PCI_H */



