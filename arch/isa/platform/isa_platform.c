
/************************************************************************
 *
 *  isa_platform.c
 *
 *  Platform specific support of ISA bus
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
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <shell_api.h>
#include <sys_api.h>
#include <product.h>
#include <malta.h>
#include <stdio.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define PRT(msg)  if( SHELL_PUTS( msg ) ) return FALSE;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          isa_display
 *  Description :
 *  -------------
 *
 *  Display platform specific ISA bus configuration data.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE : No ctrl-c detected, FALSE : ctrl-c detected
 *
 ************************************************************************/
bool
isa_display( void )
{
    char msg[80];

    switch ( sys_platform )
    {
      case PRODUCT_MALTA_ID :
        PRT( "Device   Address(ISA)   Address(CPU)   Interrupt line\n" );
	PRT( "-----------------------------------------------------\n" );

	/* TTY0 */	
        sprintf( msg, "TTY0     0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_COM1_ADR,
			  MALTA_UART0ADR,
			  MALTA_INTLINE_TTY0 );
	PRT( msg );

	/* TTY1 */
        sprintf( msg, "TTY1     0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_COM2_ADR,
			  MALTA_UART1ADR,
			  MALTA_INTLINE_TTY1 );
	PRT( msg );

	/* LPT */
        sprintf( msg, "LPT      0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_1284_ADR,
			  MALTA_1284ADR,
			  MALTA_INTLINE_1284 );
	PRT( msg );

	/* FDD */
        sprintf( msg, "FDD      0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_FDD_ADR,
			  MALTA_FDDADR,
			  MALTA_INTLINE_FDD );
	PRT( msg );

	/* Keyboard */
        sprintf( msg, "KYBD     0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_KYBD_ADR,
			  MALTA_KYBDADR,
			  MALTA_INTLINE_KYBD );
	PRT( msg );

	/* Mouse */
        sprintf( msg, "MOUSE    0x%03x          0x%08x     IRQ%d\n",
		          MALTA_SMSC_MOUSE_ADR,
			  MALTA_MOUSEADR,
			  MALTA_INTLINE_MOUSE );
	PRT( msg );


        break;
      default : /* Should not happen */
        break;
    }

    return TRUE;
}

