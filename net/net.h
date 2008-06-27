#ifndef NET_H
#define NET_H

/************************************************************************
 *
 *      NET.h
 *
 *      The 'NET' module defines common constants, types, macros atc.
 *      to be used in the net-subsystem.
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
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <syscon_api.h>

/************************************************************************
 *  Macro definitions
*************************************************************************/

//#define NET_DEBUG            1

#define _incr(src, type)	(src) += sizeof(type) / sizeof(*(src))
#define _get(src, type)		*((type*)(src))
#define _getx(src, type)	_get((src), type); _incr((src), type)
#define _put(src, dest, type)	*((type*)(dest)) = (type)(src)
#define _putx(src, dest, type)	_put((src), (dest), type); _incr((dest), type)

#define get1(src, dest)		{ dest = _getx(src, UINT8); }

#define get2(src, dest)		{ dest = _getx(src, UINT16); }

#if EB
#define get4(src, dest)		{ dest = _getx(src, UINT16); \
                                  dest = (dest << 16) | _getx(src, UINT16); }
#endif

#if EL
#define get4(src, dest)		{ dest = _getx(src, UINT16); \
                                  dest = dest | (_get(src, UINT16) << 16); _incr(src, UINT16); }
#endif

#define get6(src, dest)		{ *((UINT8*)&dest+0) = _getx(src, UINT8); \
                                  *((UINT8*)&dest+1) = _getx(src, UINT8); \
                                  *((UINT8*)&dest+2) = _getx(src, UINT8); \
                                  *((UINT8*)&dest+3) = _getx(src, UINT8); \
                                  *((UINT8*)&dest+4) = _getx(src, UINT8); \
                                  *((UINT8*)&dest+5) = _getx(src, UINT8); }

#define put1(src, dest)		{ _putx(src, dest, UINT8); }

#define put2(src, dest)		{ _putx(src, dest, UINT16); }

#if 1

#define putbe2(src, dest)	{ _putx(src >> 8, dest, UINT8); \
                                  _putx(src, dest, UINT8); }

#define put4(src, dest)		{ _putx(((UINT16*)&src)[0], dest, UINT16); \
                                  _putx(((UINT16*)&src)[1], dest, UINT16); }

#define put6(src, dest)		{ _putx(((UINT16*)&src)[0], dest, UINT16); \
                                  _putx(((UINT16*)&src)[1], dest, UINT16); \
                                  _putx(((UINT16*)&src)[2], dest, UINT16); }
#endif

#define IF_ERROR( completion, function )  \
{ \
  completion = function ; \
  if ( completion != OK )  \
{ \
    return( completion ) ; \
} \
}

#define IF_UPPER( completion, value, upper )  \
{ \
  if ( value  >= upper )  \
{ \
    return( completion ) ; \
} \
}

/************************************************************************
 *  Parameter definitions
*************************************************************************/


/************************************************************************
 *  NET public variables:
*************************************************************************/

extern UINT32 net_last_error ;
extern char   net_diag_msg[160] ;

extern UINT32 env_ipaddr;
extern UINT32 env_gateway;
extern UINT32 env_subnetmask;

/************************************************************************
 *  NET general services:
*************************************************************************/


/************************************************************************
 *
 *                          NET_gettime
 *  Description :
 *  -------------
 *  Get number of seconds since 1.1.1970.
 *
 *
 *  Parameters :
 *  ------------
 *  *time, OUT, user defined variable to return the time
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'
 *
 ************************************************************************/
UINT32 NET_gettime( UINT32 *time ) ;


/************************************************************************
 *
 *                          NET_checksum
 *  Description :
 *  -------------
 *  Return 16-bit one's complement of 16-bit one's complement sum.
 *
 *
 *  Parameters :
 *  ------------
 *  'pw',          IN,    Pointer for start of segment
 *  'words',       IN,    Number of 16-bit words in segment
 *
 *
 *  Return values :
 *  ---------------
 *  'IP-header checksum'
 *
 ************************************************************************/
UINT16 NET_checksum( UINT16 *pw, int words ) ;


/************************************************************************
 *
 *                          NET_APPL_init
 *  Description :
 *  -------------
 *  Initialize the net applications
 *
 *
 *  Parameters :
 *  ------------
 *  -
 *
 *
 *  Return values :
 *  ---------------
 *  'OK'(=0) 
 *
 ************************************************************************/
UINT32 NET_APPL_init( void );


#endif /* #ifndef NET_H */
