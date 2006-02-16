#ifndef EEPROM_NM24C09_LAYOUT_H
#define EEPROM_NM24C09_LAYOUT_H

/************************************************************************
 *
 *      EEPROM_NM24C09_LAYOUT.h
 *
 *      The 'EEPROM_NM24C09_LAYOUT' module defines the variable layout
 *      of the NM24C09 eeprom.
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




/************************************************************************
 *    Include files
 ************************************************************************/

#include "sysdefs.h"

/************************************************************************
 *   Variable layout of the 1024 bytes NM24C09 EEPROM device
*************************************************************************/


/* The upper 512 bytes EEPROM memory is write protected */
#define EEPROM_COM_EN0_BASE	      512


/* Layout of EEPROM */
typedef struct
{
    /* Layout of eeprom depends on version field */
    UINT8 version;
#define EEPROM_VERSION_1	1

    /* Number of fields */
    UINT8 count;

    /*  Fields may be placed in any order.
     *  Each field consists of the following :
     * 
     *  1) A byte containing a unique ID.
     *  2) A byte containing the number of bytes of data to follow.
     *  3) An array of bytes (length given above) containing the actual
     *     data (MSB first).
     */

    /*  Mac address (Mandatory on platforms with Ethernet support) 
     *  Format : Binary (6 bytes) 
     */
    UINT8 id_mac;
#define EEPROM_ID_MAC		1
    UINT8 len_mac;
#define EEPROM_LEN_MAC		6
    UINT8 mac_field[EEPROM_LEN_MAC];

    /*  Serial number (Mandatory on all platforms) 
     *  Format : BCD (10 digits -> 5 bytes).
     */
    UINT8 id_sn;
#define EEPROM_ID_SN		2
    UINT8 len_sn;
#define EEPROM_LEN_SN		5
    UINT8 sn_field[EEPROM_LEN_SN];

    /* Checksum consists of a byte calculated as the modulo 256 sum of
     * all used bytes of the eeprom from the version field up to, but
     * not including, the csum field.
     */

    UINT8 csum;
}
t_eeprom_layout;

#endif /* #ifndef EEPROM_NM24C09_LAYOUT_H */
