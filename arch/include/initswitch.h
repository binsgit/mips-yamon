/************************************************************************
 *
 *      INITSWITCH.h
 *
 *      The 'INITSWITCH' defines switches to control which
 *      modules to include in YAMON.
 *
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


/*********************************************************************
*
*  Modules to include in YAMON.
*
*  If set to 1, module will be included in YAMON.
*  If set to 0, module will not be included in YAMON.
*
*  The sequence of module initialisation is important since some
*  modules depend on others.
*  The sequence is defined in the function initmodules().
*
*  The modules are :
*  -----------------
*
*  SYSCON :   	        SYSCON module used for accessing system objects
*  IO :		        IO module used for interfacing to drivers
*  EXCEP :		Exception handling (registration mechanism)
*  RTC :		Real Time Clock driver
*  FREQ :	        CPU and BUS frequency detection and optimisations
*  PCI :	        PCI autodetection/autodiscovery
*  IIC :                IIC controller driver
*  EEPROM_IIC :         EEPROM access functions
*  FLASH_STRATA :	Strataflash driver
*  SYSENV :		Raw environment records
*  ENV :		YAMON environment varibles
*  SERIAL_SAA9730 :	Philips SAA9730 UART driver (one device)
*  SERIAL_TI16550 :	TI16550 UART driver (two devices)
*  LAN_SAA9730 :	Philips SAA9730 Ethernet driver
*  LAN_AM79C973 :	AMD 79C973 Ethernet driver
*  NET :		Networking protocols
*  IDE :		IDE raw disk I/O
*
*********************************************************************/

#define  INCLUDE_SYSCON              1
#define  INCLUDE_IO                  1
#define  INCLUDE_EXCEP               1
#define  INCLUDE_RTC		     1
#define  INCLUDE_FREQ		     1
#define  INCLUDE_PCI		     1
#define  INCLUDE_IIC         	     1
#define  INCLUDE_EEPROM_IIC          1
#define  INCLUDE_FLASH_STRATA        1
#define  INCLUDE_SYSENV              1
#define  INCLUDE_ENV	     	     1
#define  INCLUDE_SERIAL     	     1
#define  INCLUDE_LAN_SAA9730         1
#define  INCLUDE_LAN_AM79C973        1
#define  INCLUDE_NET                 1
#define  INCLUDE_IDE                 1


