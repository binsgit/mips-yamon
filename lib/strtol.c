/************************************************************************
 *
 *  strtol.c
 *
 *  Note: signed long is assumed to equal INT32.
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

#include <ctype.h>
#include <sysdefs.h>
#include <errno.h>

#define	 max_allowable(A) 	 (MAXINT(sizeof(INT32))/A - 1)


long strtol(
	const 	char 	*str, 
		char 	**endptr, 
		int 	base)
{
	long  	i = 0;
	int   	s = 1;
	int 	c;
	
	/* skip white space */
	while(isspace(*str)) {
		str++;
	}
	
	/* sign flag check */
	if (*str == '+') str++;
	else if (*str == '-') {
		s = -1;
		str++;
	}

	if (*str == '0') {
		if (toupper(*++str) == 'X')	base = 16,str++;
		else if (base == 0)		base = 8;
		}
	if (base == 0) base = 10;
	
	
	if (base <= 10)
	/* digit str to number */
		for (; isdigit(*str); str++) {
			if (i < max_allowable(base))
				i = i * base + (*str - '0');
			else {
				i = MAXINT(sizeof(INT32));
				errno = ERANGE;
			}
		}
	else if (base > 10) {
		for (; (c = *str); str++) {
			if (isdigit(c))
			 	c = c - '0';
			else {
				c = toupper(c);

				if (c >= 'A' && c < ('A' - 10 + base))
				 	c = c - 'A' + 10;
				else
				 	break;
			}
			if (i < max_allowable(base))
			 	i = i * base + c;
			else {
			 	i = MAXINT(sizeof(INT32));
				errno = ERANGE;
			}
		}
	}	
 	else
	 	return 0;		/* negative base is not allowed */
	
	if (endptr) *endptr = (char *) str;

	if (s == -1)
		i = -i;

	return i;
}

