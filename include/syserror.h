#ifndef SYSERROR_H
#define SYSERROR_H

/************************************************************************
 *
 *      SYSERROR.h
 *
 *      The 'SYSERROR' module defines system error definitions.
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

/************************************************************************
 *  Macro Definitions
*************************************************************************/

#define SYSERROR_DOMAIN_SHIFT     12
#define SYSERROR_DOMAIN_MASK      0xff   /* limit the number of domains 
                                            to 255 */

#define SYSERROR_ID_SHIFT         0
#define SYSERROR_ID_MASK          0xfff

#define SYSERROR_DOMAIN( value ) (UINT32)(((UINT32)value >> SYSERROR_DOMAIN_SHIFT) & \
                                            (         SYSERROR_DOMAIN_MASK ))
#define SYSERROR_ID( value )     (UINT32)(((UINT32)value >> SYSERROR_ID_SHIFT) & \
                                            (         SYSERROR_ID_MASK ))
#define SYSERROR_DOMAIN_COUNT     SYSERROR_DOMAIN_MASK

/************************************************************************
 *  ERROR CODE RANGE allocation
*************************************************************************/

/***********************************************************************
 *                                                                     *
 *      ERROR CODE RANGE           MODULE             DEFINED IN       *
 *    =====================   ==================     ==============    *
 *    0000'0001 - 0000'0FFF   (reserved)                               *
 *    0000'1000 - 0000'1FFF   (reserved)                               *
 *    0000'2000 - 0000'2FFF   IO                     io_api.h          *
 *    0000'3000 - 0000'3FFF   SERIAL                 serial_api.h      *
 *    0000'4000 - 0000'4FFF   LAN                    lan_api.h         *
 *    0000'5000 - 0000'5FFF   IIC                    iic_api.h         *
 *    0000'6000 - 0000'6FFF   EEPROM                 eeprom_api.h      *
 *    0000'7000 - 0000'7FFF   RTC                    rtc_api.h         *
 *    0000'8000 - 0000'8FFF   SYSCON                 syscon_api.h      *
 *    0000'9000 - 0000'9FFF   FLASH                  flash_api.h       *
 *    0000'A000 - 0000'AFFF   NET                    net_api.h         *
 *    0000'B000 - 0000'BFFF   EXCEP                  excep_api.h       *
 *    0000'C000 - 0000'CFFF   SYSENV                 sysenv_api.h      *
 *    0000'D000 - 0000'DFFF   LOADER                 loader_api.h      *
 *    0000'E000 - 0000'EFFF   ENV		     env_api.h	       *
 *    0000'F000 - 0000'FFFF   PCI		     pci_api.h	       *
 *    0001'0000 - 0001'0FFF   SHELL		     shell_api.h       *
 *    0001'1000 - 0001'1FFF   IDE		     ide_api.h	       *
 *								       *
 ***********************************************************************/


#define  ERROR_IO       0x00002000     /* IO driver subsystem          */
#define  ERROR_SERIAL   0x00003000     /* SERIAL driver subsystem      */
#define  ERROR_LAN      0x00004000     /* LAN driver subsystem         */
#define  ERROR_IIC      0x00005000     /* IIC driver subsystem         */
#define  ERROR_EEPROM   0x00006000     /* EEPROM driver subsystem      */
#define  ERROR_RTC      0x00007000     /* RTC driver subsystem         */
#define  ERROR_SYSCON   0x00008000     /* SYSCON subsystem             */
#define  ERROR_FLASH    0x00009000     /* FLASH subsystem              */
#define  ERROR_NET      0x0000a000     /* NET subsystem                */
#define  ERROR_EXCEP    0x0000b000     /* EXCEP subsystem              */
#define  ERROR_SYSENV   0x0000c000     /* SYSENV subsystem             */
#define  ERROR_LOADER   0x0000d000     /* LOADER subsystem             */
#define  ERROR_ENV      0x0000e000     /* ENV subsystem		       */
#define  ERROR_PCI	0x0000f000     /* PCI subsystem		       */
#define  ERROR_SHELL	0x00010000     /* SHELL subsystem	       */
#define  ERROR_IDE	0x00011000     /* IDE subsystem		       */

/************************************************************************
 *  Generic system completion codes
*************************************************************************/

#define  OK             0x00000000     /* operation completed 
                                                           successfully */
#define  NOT_OK         0xffffffff     /* operation completed 
                                                    not successfully    */

#endif /* #ifndef SYSERROR_H */

