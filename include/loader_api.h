
/************************************************************************
 *
 *  loader_api.h
 *
 *  API file for load image module
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


#ifndef LOADER_API_H
#define LOADER_API_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>

/************************************************************************
 *  Definitions
*************************************************************************/

typedef enum load_formats 
{
    MOTOROLA_S3 = 0,
    MICROSOFT_BIN
} t_image_format;

#define ERROR_LOADER             0x0000d000 /* for compile time check       */
#define ERROR_LOAD_NONE              0xd000 /* No error                     */
#define ERROR_LOAD_UNSUPPORTED_PORT  0xd001 /* Port not supported           */
#define ERROR_LOAD_AMBIGUOUS_TYPE    0xd002 /* Load format is ambigous      */
#define ERROR_LOAD_UNKNOWN_TYPE      0xd003 /* Load format is unknown       */
#define ERROR_LOAD_ILLEGAL_FORMAT    0xd004 /* Load format not compliant    */
#define ERROR_LOAD_CHECKSUM          0xd005 /* Checksum error               */
#define ERROR_LOAD_LINE_TOO_LONG     0xd006 /* Line too long                */
#define ERROR_LOAD_NET               0xd007 /* Load error from net          */
#define ERROR_LOAD_STORE             0xd008 /* Load error by store of data  */
#define ERROR_LOAD_BREAK             0xd009 /* Load was interrupted by user */
#define ERROR_LOAD_NO_S7_RECORD      0xd00a /* S7 termination record is lacking */
#define ERROR_LOAD_ILLEGAL_CHARACTER 0xd00b /* Illegal character in line    */
#define ERROR_LOAD_UART_COMM_ERROR   0xd00c /* UART communication error     */


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          loader_init
 *  Description :
 *  -------------
 *
 *  Init load module
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
loader_init( void );


/************************************************************************
 *
 *                          loader_image
 *  Description :
 *  -------------
 *
 *  Load image from serial port or Ethernet
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
loader_image(
    UINT32	   port,		/* Port number			*/
    UINT32	   ip,			/* IP addr (valid for PORT_NET) */
    char           *filename,		/* valid for PORT_NET		*/
    void	   **addr,		/* Output : Start address	*/
    t_image_format *imageformat,	/* Output : Format		*/
    UINT32	   *error_pos,  	/* Output : Position of error   */
    UINT32	   *raw_error );	/* Output : Raw error from 
                                                    failing subsystem   */

#endif /* #ifndef LOADER_API_H */
