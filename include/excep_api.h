
#ifndef EXCEP_API_H
#define EXCEP_API_H

/************************************************************************
 *
 *  excep_api.h
 *
 *  The 'EXCEP' module implements a first level exception and
 *  interrupt handling and an API to register user defined
 *  exception and interrupt service routines (ESR & ISR).
 *
 *  Interrupt Service Routine registration is possible at
 *
 *    a) The MIPS CPU SW/HW interrupt level (CPU-ISR) : 0..7
 *    b) The Interrupt controller level     (IC-ISR)  : (platform specific)
 *
 *  Registration of ISR's imply enabling of the corresponding
 *  interrupt MASK bit in the CPU or the interrupt controller.
 *
 *  Interrupt Service Routine handling is executed in a prioritized
 *  scheme. First, registered handlers at CPU interrupt level are
 *  called, second, registered handlers at interrupt controller
 *  level are called.
 *
 *  Public API offers:
 *
 *  1) EXCEP_init()
 *  2) EXCEP_register_esr()
 *  3) EXCEP_register_cpu_isr()
 *  4) EXCEP_register_ic_isr()
 *  5) EXCEP_print_context()
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


/************************************************************************
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <gdb_stub.h>


#ifndef _ASSEMBLER_


/************************************************************************
 *  System Definitions
*************************************************************************/

/*  Number used by the below registration functions in order to register
 *  a default handler.
 */
#define EXCEP_DEFAULT_HANDLER	0xfffffff0

/*  Number used by the below registration functions in order to register
 *  an EJTAG debug exception handler.
 */
#define EXCEP_EJTAG_HANDLER	0xfffffff1

/* Registered ISR */
typedef void (*t_EXCEP_isr)(void *data);

/* Registered ESR */
typedef void (*t_EXCEP_esr)(void);

/* Function registered to be called by first level exception handler */
typedef void (*t_EXCEP_first_level_excep)(UINT32 code, bool ejtag);

/* Entry point called by ESRs wishing to pass control back to YAMON */
typedef void (*t_EXCEP_retfunc)(void);

/* Handle used for deregistration of ISR/ESR */
typedef void *t_EXCEP_ref;

/************************************************************************
 *   EXCEP ERROR completion codes
*************************************************************************/

#define ERROR_EXCEP                0x0000b000 /* for compile time check    */
#define ERROR_EXCEP_ILLEGAL_EXCEPTION  0xb000 /* Exception ID is unknown   */
#define ERROR_EXCEP_ILLEGAL_HANDLER    0xb001 /* Handler reference is NULL */
#define ERROR_EXCEP_ALREADY_REGISTERED 0xb002 /* Exception has been registered */
#define ERROR_EXCEP_ILLEGAL_INTERRUPT  0xb003 /* Interrupt source unknown  */
#define ERROR_EXCEP_NO_SPACE           0xb004 /* Too many handlers registered */


/************************************************************************
 *    Public variables
 ************************************************************************/

extern bool	   EXCEP_nmi;

/************************************************************************
 *   Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          EXCEP_init
 *  Description :
 *  -------------
 *
 *  Initializes the 'EXCEP' module.
 *  
 *  Parameters :
 *  ------------
 *
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 ************************************************************************/
INT32 
EXCEP_init( void );


/************************************************************************
 *
 *                          EXCEP_register_esr
 *  Description :
 *  -------------
 *
 *  Registers an exception handler, also known as an "Exception Service
 *  Routine" (ESR) for the specified exception.
 *  
 *  Two special exception IDs are defined :
 *      EXCEP_DEFAULT_HANDLER used for a default handler.
 *      EXCEP_EJTAG_HANDLER used for EJTAG exceptions.
 *	
 *  The default handler is called if no other handler is registered
 *  for an exception. If no default handler is registered, a static
 *  (i.e. not registered) "super default" function is invoked.
 *  This function prints out the registers and halts.
 *
 *  Deregistration of a handler may be be done by calling this function 
 *  with 'handler' set to NULL.
 *  Handlers can also be deregistered using EXCEP_deregister_esr.
 *
 *  A handler may be registered even if a previously registered 
 *  handler has not been deregistered. In this case the previously
 *  registered handler is lost.
 *
 *  In case an ESR does not want to handle the exception, it may
 *  call the return function passed in the 'retfunc' parameter.
 *
 *  Case 1 : 'retfunc' called by ESR registered for the INTERRUPT exception.
 *
 *  We assume an application has registered this ESR and wants
 *  YAMON to process the (remaining) interrupts.
 *  So, we call the ESR, which is used by YAMON for handling 
 *  interrupt exceptions, i.e. interrupt_sr().
 *
 *  Case 2 :  'retfunc' called by an ESR registered for a specific
 *	      exception (not INTERRUPT).
 *
 *  Default handling will be done.
 *
 *  Case 3 : 'retfunc' is called by the ESR registered as default handler.
 *
 *  The exception will be handled as though no ESR is registered 
 *  (i.e. the "super default" function is called).
 *
 *  Parameters :
 *  ------------
 *
 *  'exception', IN,    Exception code (C0_CAUSE_CODE_xxx).
 *			or EXCEP_DEFAULT_HANDLER for a default handler
 *			or EXCEP_EJTAG_HANDLER for ejtag exceptions.
 *  'raw',	 IN     If TRUE, ESR will get called with registers
 *			in the state they were when the exception occurred.
 *			This includes all CP0 registers and CPU registers
 *			$0..$31, except for k0,k1 ($26,$27).
 *  'handler',   IN,    Function pointer for ESR.
 *  'ref',	 OUT,   Handle used for deregistration of handler.
 *  'retfunc',	 OUT,   Pointer to function pointer for the return
 *			function described above.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_EXCEPTION':  Exception ID is unknown
 *
 ************************************************************************/
INT32 
EXCEP_register_esr(
    UINT32          exception,	     /* Exception identification	*/
    bool	    raw,	     /* Pass on unmodified registers	*/
    t_EXCEP_esr     handler,	     /* ESR to be registered		*/
    t_EXCEP_ref     *ref,	     /* OUT : Handle for deregistration */
    t_EXCEP_retfunc *retfunc );      /* OUT : Return function		*/


/************************************************************************
 *
 *                          EXCEP_deregister_esr
 *  Description :
 *  -------------
 *
 *  Deregisters exception handler
 *  
 *  Parameters :
 *  ------------
 *
 *  'ref',	 IN,   Handle obtained when calling EXCEP_register_esr
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_HANDLER':  ref is unknown
 *
 ************************************************************************/
INT32 
EXCEP_deregister_esr(
    t_EXCEP_ref ref );	             /* In : Handle for deregistration */


/************************************************************************
 *
 *                          EXCEP_register_cpu_isr
 *  Description :
 *  -------------
 *
 *  Registers an interrupt handler for the specified CPU interrupt.
 *  The highest service priority is attached to HW-INT5, which is
 *  connected to the CPU-built-in CP0-timer. SW_INT0 gets the lowest
 *  service priority. During registration, the interrupt mask field
 *  in the CPU CP0-status register is updated to enable interrupts
 *  from the specified interrupt source.
 *
 *  A special ID is defined :
 *      EXCEP_DEFAULT_HANDLER used for a default handler.
 *	
 *  The default handler is called if no other handler is registered
 *  for a CPU interrupt.
 *
 *  Deregistration of the default handler may be done by calling
 *  this function with 'handler' set to NULL.
 *  Also, a new default handler may be registered even if a 
 *  previously registered handler has not been deregistered.
 *  Handlers for specific CPU interrupts must be deregistered using 
 *  EXCEP_deregister_cpu_isr.
 *
 *  Parameters :
 *  ------------
 *
 *  'cpu_int',      IN,    CPU interrupt (C0_STATUS_IM_HW[0..5] |
 *					  C0_STATUS_IM_SW[0..1])
 *			   or EXCEP_DEFAULT_HANDLER for a default handler.
 *  'handler',      IN,    Function pointer for user defined handler
 *  'data',         IN,    Data pointer, may be NULL
 *  'ref',	    OUT,   Handle used for deregistration of handler.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_INTERRUPT': CPU interrupt source is unknown
 *  'ERROR_EXCEP_ILLEGAL_HANDLER':   Handler reference is NULL
 *  'ERROR_EXCEP_NO_SPACE':          Too many handlers have been registered
 *
 ************************************************************************/
INT32 
EXCEP_register_cpu_isr(
    UINT32      cpu_int,	     /* CPU interrupt, 0..7		*/
    t_EXCEP_isr handler,	     /* ISR to be registered		*/
    void        *data,   	     /* Data reference to be registered */
    t_EXCEP_ref *ref );	             /* OUT : Handle for deregistration */


/************************************************************************
 *
 *                          EXCEP_deregister_cpu_isr
 *  Description :
 *  -------------
 *
 *  Deregisters interrupt handler
 *  
 *  Parameters :
 *  ------------
 *
 *  'ref',	 IN,   Handle obtained when calling EXCEP_register_cpu_isr
 *  It is OK to call this function even if we are in interrupt context.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_HANDLER':  ref is unknown
 *
 ************************************************************************/
INT32 
EXCEP_deregister_cpu_isr(
    t_EXCEP_ref ref );	             /* In : Handle for deregistration */


/************************************************************************
 *
 *                          EXCEP_register_ic_isr
 *  Description :
 *  -------------
 *
 *  Registers an interrupt handler for the specified source in the
 *  interrupt controller. 
 *
 *  A special ID is defined :
 *      EXCEP_DEFAULT_HANDLER used for a default handler.
 *	
 *  The default handler is called if no other handler is registered
 *  for an interrupt.
 *
 *  Deregistration of the default handler may be done by calling
 *  this function with 'handler' set to NULL.
 *  Also, a new default handler may be registered even if a 
 *  previously registered handler has not been deregistered.
 *  Handlers for specific interrupts must be deregistered using 
 *  EXCEP_deregister_ic_isr.

 *  Parameters :
 *  ------------
 *
 *  'ic_int_line', IN,    Interrupt source line in Int. Controller
 *			  or EXCEP_DEFAULT_HANDLER for a default handler.
 *  'handler',     IN,    Function pointer for user defined handler
 *  'data',        IN,    Data pointer; may be NULL
 *  'ref',	   OUT,   Handle used for deregistration of handler.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_INTERRUPT': Int. source line is unknown
 *  'ERROR_EXCEP_ILLEGAL_HANDLER':   Handler reference is NULL
 *  'ERROR_EXCEP_NO_SPACE':          Too many handlers have been registered
 *
 ************************************************************************/
INT32 
EXCEP_register_ic_isr(
    UINT32      ic_int_line,	    /* Exception identification		*/
    t_EXCEP_isr handler,	    /* ISR to be registered		*/
    void        *data,		    /* Data reference to be registered  */
    t_EXCEP_ref *ref );		    /* OUT : Handle for deregistration  */


/************************************************************************
 *
 *                          EXCEP_deregister_ic_isr
 *  Description :
 *  -------------
 *
 *  Deregisters interrupt handler for source in interrupt controller
 *  It is OK to call this function even if we are in interrupt context.
 *  
 *  Parameters :
 *  ------------
 *
 *  'ref',	 IN,   Handle obtained when calling EXCEP_register_ic_isr
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_EXCEP_ILLEGAL_HANDLER':  ref is unknown
 *
 ************************************************************************/
INT32 
EXCEP_deregister_ic_isr(
    t_EXCEP_ref ref );	             /* In : Handle for deregistration */


/************************************************************************
 *
 *                          EXCEP_store_handlers
 *  Description :
 *  -------------
 *
 *  The current setup of registered exception handlers is copied to
 *  a table so that it may later be restored using EXCEP_set_handlers().
 *
 ************************************************************************/
void 
EXCEP_store_handlers(
    UINT32 set ); /* table where setup is copied */
#define EXCEP_HANDLERS_SHELL	0
#define EXCEP_HANDLERS_GDB	1


/************************************************************************
 *
 *                          EXCEP_set_handlers
 *  Description :
 *  -------------
 *
 *  The setup of registered exception handlers is restored to the
 *  state stored previously using EXCEP_store_handlers().
 *
 ************************************************************************/
void 
EXCEP_set_handlers(
    UINT32 set ); /* table where setup is copied */


/************************************************************************
 *
 *                          EXCEP_install_exc_in_ram
 *  Description :
 *  -------------
 *  Install jump instructions to exc handler at exception vector locations.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
void
EXCEP_install_exc_in_ram( void );


/************************************************************************
 *
 *                          EXCEP_save_context
 *  Description :
 *  -------------
 *  Saves current context, as done by an exception
 *  Will return 0 when context is saved.
 *  Will return 1 second time, when context is restored.
 *
 *  Parameters :
 *  ------------
 *  'context',	 IN,   Pointer to context structure to save to
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
UINT32
EXCEP_save_context(
    t_gdb_regs *context );


/************************************************************************
 *
 *                          EXCEP_get_context_ptr
 *  Description :
 *  -------------
 *  Retrieves pointer to current exception context.
 *
 *  Return values :
 *  ---------------
 *  Pointer to register structure saved at exception entry.
 *  If a valid pointer cannot be returned, the routine does not return!
 ************************************************************************/
t_gdb_regs *
EXCEP_get_context_ptr( void );


/************************************************************************
 *
 *                          EXCEP_exc_handler_ret
 *  Description :
 *  -------------
 *
 *  Restore context and return from exception
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
EXCEP_exc_handler_ret( 
    t_gdb_regs *context );


/************************************************************************
 *
 *                          EXCEP_exc_handler_ret_ss
 *  Description :
 *  -------------
 *
 * Same functionality as EXCEP_exc_handler_ret()
 * except that for CPUs implementing MIPS32/MIPS64
 * Release 2, we first make sure to use shadowset 0.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
EXCEP_exc_handler_ret_ss( 
    t_gdb_regs *context );


/************************************************************************
 *
 *                          EXCEP_print_context
 *  Description :
 *  -------------
 *
 *  Print context
 *
 *  Note : Printout is done by printf() through the IO module, so if
 *         this routine is entered before serial drivers are installed,
 *         no printout will take place - and no harm is done.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
EXCEP_print_context( 
    t_gdb_regs *context,	/* Context to print			*/
    char       *msg,		/* Special msg to be printed (or NULL)	*/
    bool       user,		/* In user context			*/
    bool       display );	/* TRUE -> Print EPC in ASCII display	*/


/************************************************************************
 *
 *                          EXCEP_run_default_esr_handler
 *  Description :
 *  -------------
 *  Runs default exception handler.
 *  Called by North Bridge Interrupt handlers (init_core.c)
 *  to print out context and possibly continue (go.c).
 *
 *  Return values :
 *  ---------------
 *  None - may not return
 *
 ************************************************************************/
void
EXCEP_run_default_esr_handler( void );


#else /* #ifndef _ASSEMBLER_ */

EXTERN(EXCEP_install_exc_in_ram)
EXTERN(EXCEP_exc_handler_ret)

#endif /* #ifndef _ASSEMBLER_ */


#endif /* #ifndef EXCEP_API_H */
