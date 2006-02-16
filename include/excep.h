
/************************************************************************
 *
 *  excep.h
 *
 *  Local definitions for EXCEP module
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

#ifndef EXCEP_H
#define EXCEP_H

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <excep_api.h>

/************************************************************************
 *  Definitions
*************************************************************************/


/************************************************************************
 *  Public variables
 ************************************************************************/

bool EXCEP_return_flag;

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          arch_excep_init_intctrl
 *  Description :
 *  -------------
 *
 *  Initialise interrupt controller.
 *
 *  Parameters :
 *  ------------
 *
 *  Function will fill out the ic_count and ic_int parameters.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_init_intctrl(
    UINT32 *ic_count,   /* Number of interrupts handled by interrupt 
			   controller (0 if no controller is available) */
    UINT32 *ic_int );   /* HW interrupt used by interrupt controller	*/


/************************************************************************
 *
 *                          arch_excep_enable_int
 *  Description :
 *  -------------
 *
 *  Enable specific source in interrupt controller
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
arch_excep_enable_int(
    UINT32 ic_line );	/* Interrupt source				*/


/************************************************************************
 *
 *                          arch_excep_disable_int
 *  Description :
 *  -------------
 *
 *  Disable specific source in interrupt controller
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
arch_excep_disable_int(
    UINT32 ic_line );	/* Interrupt source				*/


/************************************************************************
 *
 *                          arch_excep_pending
 *  Description :
 *  -------------
 *
 *  Return pending interrupt(s) in interrupt controller.
 *  On some platforms, all pending interrupts are returned, on other
 *  platforms only the hightest priority interrupt is returned.
 *
 *  Return values :
 *  ---------------
 *
 *  Pending interrupt(s)
 *
 ************************************************************************/
UINT32
arch_excep_pending( void );


/************************************************************************
 *
 *                          arch_excep_eoi
 *  Description :
 *  -------------
 *
 *  Perform EOI cycle for indicated interrupt
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_excep_eoi(
    UINT32 index );


/************************************************************************
 *
 *                          arch_eic_init
 *  Description :
 *  -------------
 *
 *  Initialise EIC (Extended Interrupt Controller)
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_eic_init(void);


/************************************************************************
 *
 *                          arch_eic_enable_int
 *  Description :
 *  -------------
 *
 *  Enable EIC interrupt
 *
 *  Parameters :
 *  ------------
 *
 *  cpu_int: EIC interrupt to enable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
arch_eic_enable_int(
    UINT32 ic_line );	/* Interrupt source				*/


/************************************************************************
 *
 *                          arch_eic_disable_int
 *  Description :
 *  -------------
 *
 *  Disable EIC interrupt
 *
 *  Parameters :
 *  ------------
 *
 *  cpu_int: EIC interrupt to disable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
arch_eic_disable_int(
    UINT32 cpu_int );	/* Interrupt source				*/


/************************************************************************
 *
 *                          arch_eic_eoi
 *  Description :
 *  -------------
 *
 *  Perform EOI cycle for indicated interrupt on EIC
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
arch_eic_eoi(
    UINT32 index );


/************************************************************************
 *
 *                          EXCEP_init_reg_mask
 *  Description :
 *  -------------
 *  Determine what CP0/CP1 regs should be context switched.
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
void 
EXCEP_init_reg_mask( void );


/************************************************************************
 *
 *                          EXCEP_exc_handler_jump
 *  Description :
 *  -------------
 *
 *  Restore context and jump to function given by parameter.
 *  The function is responsible for handling the
 *  exception and possibly issue an eret (or deret
 *  in the case of an EJTAG exception).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
EXCEP_exc_handler_jump(
    t_EXCEP_esr handler );	/* ESR to be called			*/


/************************************************************************
 *
 *                          EXCEP_return
 *  Description :
 *  -------------
 *
 *  Entry point for code used for passing control from a registered
 *  ESR (Exception Service Routine) to the general exception handling
 *  in case the registered handler does not wish to process the exception.
 *  
 *  The address of this function is passed as a parameter in
 *  EXCEP_register_esr().
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
EXCEP_return( void );


#endif /* #ifndef EXCEP_H */

