
/************************************************************************
 *
 *  excep.c
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
 *  3) EXCEP_deregister_esr()
 *  4) EXCEP_register_cpu_isr()
 *  5) EXCEP_deregister_cpu_isr()
 *  6) EXCEP_register_ic_isr()
 *  7) EXCEP_deregister_ic_isr()
 *  8) EXCEP_print_context()
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
 *      Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <mips.h>
#include <excep_api.h>
#include <excep.h>
#include <syscon_api.h>
#include <shell_api.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

/* max. number of interrupt sources from the CPU */
#define MAX_INTERRUPTS        (C0_STATUS_IM_MAX + 1)

/* max. number of exception codes from the CPU */
#define MAX_EXCEPTIONS        (EX_MCHECK + 1)

/* max. number of interrupt sources from interrupt controller */
#define MAX_IC		      32

/*  Basic definition for registered interrupt handler */
typedef struct int_handler
{
   t_EXCEP_isr handler;
   void        *data;
} t_INT_handler;

/*  Basic definition for registered exception handler */
typedef struct excep_handler
{
   t_EXCEP_esr handler;
   bool	       raw;
} t_EXCEP_handler;


/*  Element struct to be used for 8-bit-set of int.
 *  pending to int. handler index list.
 */
typedef struct excep_set_to_list
{
   UINT8 list_size;
   UINT8 index[8];
} t_EXCEP_set_to_list;

/* max. number of interrupt handlers per line */
#define INT_HANDLERS_PER_LINE 4

/*  Element struct to be used for keeping a configurable
 *  number of int. handlers per line.
 */
typedef struct excep_int_handler_list
{
   UINT8         list_size;
   t_INT_handler int_handler[INT_HANDLERS_PER_LINE];
} t_EXCEP_int_handler_list;

/* max. number of interrupt control handlers per line */
#define CTRL_HANDLERS_PER_LINE 4

/*  Element struct to be used for keeping a configurable
 *  number of int. controller handlers per line.
 */
typedef struct excep_ctrl_handler_list
{
   UINT8         list_size;
   t_INT_handler ctrl_handler[CTRL_HANDLERS_PER_LINE];
} t_EXCEP_ctrl_handler_list;


/* Struct for holding exception handlers */
typedef struct
{
    /* Exception handler table. */
    t_EXCEP_handler           exception[MAX_EXCEPTIONS]; 

    /* EJTAG exception handler */
    t_EXCEP_handler	      ejtag;

    /* CPU Interrupt handler table. */
    t_EXCEP_int_handler_list  interrupt[MAX_INTERRUPTS];

    /* Int. controller handler table. */
    t_EXCEP_ctrl_handler_list controller[MAX_IC];

    /* Default handlers */
    t_EXCEP_handler	      default_exception;
    t_INT_handler	      default_int;
    t_INT_handler	      default_ctrl;
}
t_table;

/************************************************************************
 *      Public variables
 ************************************************************************/

bool   EXCEP_nmi = FALSE;
UINT32 EXCEP_C0_Config_cacheerr;

extern t_gdb_regs ejtag_context;

/************************************************************************
 *      Static variables
 ************************************************************************/

static bool EXCEP_cacheerr = FALSE;

/* Number of interrupts handled by interrupt controller */
static UINT32 excep_ic_count;

/* HW interrupt used by interrupt controller */
static UINT32 excep_ic_int;

/***********************************************************************
 *
 *      Handler function tables
 *
 ************************************************************************/

static t_table table;
static t_table table_shell, table_gdb;

/***********************************************************************
 *
 *      Private lookup tables
 *
 ************************************************************************/

/*
 * An 8-bit set is converted to a list of indices
 * by sequentially using and clearing the most significant bit number.
 */
static const UINT8 byte2msbit[256] =
{
   0,
   0,
   1,1,
   2,2,2,2,
   3,3,3,3, 3,3,3,3,
   4,4,4,4, 4,4,4,4, 4,4,4,4, 4,4,4,4,

   5,5,5,5, 5,5,5,5, 5,5,5,5, 5,5,5,5,
   5,5,5,5, 5,5,5,5, 5,5,5,5, 5,5,5,5,

   6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,
   6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,
   6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,
   6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,

   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7,
   7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7
};


/* Textual descriptions of cause codes */
static char *(cause_names[]) =
{
  /* EX_INT    */ "Interrupt",
  /* EX_MOD    */ "TLB modification",
  /* EX_TLBL   */ "TLB (load or instruction fetch)",
  /* EX_TLBS   */ "TLB (store)",
  /* EX_ADEL   */ "Address error (load or instruction fetch)",
  /* EX_ADES   */ "Address error (store)",
  /* EX_IBE    */ "Bus error (instruction fetch)",
  /* EX_DBE    */ "Bus error (data reference: load or store)",
  /* EX_SYS    */ "Syscall",
  /* EX_BP     */ "Breakpoint",
  /* EX_RI     */ "Reserved instruction",
  /* EX_CPU    */ "Coprocessor Unusable",
  /* EX_OV     */ "Arithmetic Overflow",
  /* EV_TR     */ "Trap",
  /* 14 (reserved)        */ NULL,
  /* EX_FPE    */ "Floating point",
  /* 16		          */ NULL,
  /* 17		          */ NULL,
  /* 18		          */ NULL,
  /* 19		          */ NULL,
  /* 20		          */ NULL,
  /* 21		          */ NULL,
  /* 22		          */ NULL,
  /* EX_WATCH  */ "Reference to WatchHi/WatchLo address",
  /* EX_MCHECK */ "Machine check",
  /* 25		          */ NULL,
  /* 26		          */ NULL,
  /* 27		          */ NULL,
  /* 28		          */ NULL,
  /* 29		          */ NULL,
  /* 30		          */ NULL,
  /* 31		          */ NULL
};


/* CPU register names */
static const char *(reg_names[]) =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

#define REG_NAMES_COUNT		(sizeof(reg_names)/sizeof(char *))


/************************************************************************
 *      Static function prototypes
 ************************************************************************/


static void 
interrupt_sr( void ); 

static void 
controller_sr( 
    void *data );

static void
default_handler(
    char *msg,
    t_gdb_regs *p_context );

static void
cacheerr_handler(
    t_gdb_regs *p_context );


/************************************************************************
 *      Implementation : Public functions
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
EXCEP_init( void ) 
{
    /* Initialize interrupt controller */
    arch_excep_init_intctrl(&excep_ic_count, &excep_ic_int);

    if( excep_ic_count > MAX_IC )	     /* Should not happen	*/
        while(1);

    /**** Initialize exception handler structure ****/
    memset((void*)&table, 0, sizeof(table));

    /* Register ESR for interrupt exception */
    EXCEP_register_esr( EX_INT,
			FALSE,
			interrupt_sr,
			NULL,
			NULL );

    /* Register interrupt control handler if controller is available */
    if( excep_ic_count > 0 )
        EXCEP_register_cpu_isr( excep_ic_int, controller_sr, NULL, NULL );

    /* Determine what CP0/CP1 registers exist and must be context shifted */
    EXCEP_init_reg_mask();

    /* set up ram vectors and clear cp0 status BEV */
    EXCEP_install_exc_in_ram();

    return OK;
}


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
    t_EXCEP_retfunc *retfunc )       /* OUT : Return function		*/
{
    t_EXCEP_handler *h;

    if( exception == EXCEP_DEFAULT_HANDLER )
    {
        h = &table.default_exception;
    }
    else if( exception == EXCEP_EJTAG_HANDLER )
    {
        h = &table.ejtag;
    }
    else if( exception >= MAX_EXCEPTIONS )
    {
        return ERROR_EXCEP_ILLEGAL_EXCEPTION;
    }
    else
    {
        /* Specific exception */
        h = &table.exception[exception];
    }

    /* register/deregister exception handler */
    h->handler = handler;
    h->raw     = raw;

    if( ref )
        *ref = (t_EXCEP_ref)h;

    if( retfunc )
        *retfunc = EXCEP_return;

    return OK;
}


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
    t_EXCEP_ref ref )	             /* In : Handle for deregistration */
{
    t_EXCEP_handler *h = (t_EXCEP_handler *)ref;

    if( h )
    {
        h->handler = NULL;
        return OK;
    }
    else
        return ERROR_EXCEP_ILLEGAL_HANDLER;
}


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
    t_EXCEP_ref *ref )	             /* OUT : Handle for deregistration */
{
    UINT32        old_ie;
    t_INT_handler *h = NULL;
    UINT32        rc = OK;
    UINT32        i;
    bool	  enable = FALSE;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    if( cpu_int == EXCEP_DEFAULT_HANDLER )
    {
        h = &table.default_int;
    }
    else if( cpu_int >= MAX_INTERRUPTS )
    {
        /* Interrupt specified is unknown */
        rc = ERROR_EXCEP_ILLEGAL_INTERRUPT;
    }
    else
    {
        /* Specific interrupt */

        if( handler == NULL )
        {
            rc = ERROR_EXCEP_ILLEGAL_HANDLER;
        }
        else if( table.interrupt[cpu_int].list_size == INT_HANDLERS_PER_LINE )
        {
            /* Too many handlers have been registered */
            rc = ERROR_EXCEP_NO_SPACE;
        }
        else
        {
            /* Find unused entry in array */
            for( i = 0, h = &table.interrupt[cpu_int].int_handler[0];
	         (i < INT_HANDLERS_PER_LINE) && (h->handler != NULL);
	         i++, h++) 
            {
	        ;
	    }
	    
            table.interrupt[cpu_int].list_size++;
	    enable = TRUE;
        }
    }

    if( rc == OK )
    {
        h->handler = handler;	
	h->data    = data;

        if( ref )
            *ref = (t_EXCEP_ref)h;

        /* Enable interrupt mask */
	if( enable )
	{
            sys_enable_int_mask( cpu_int );
	}
    }

    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();

    return rc;
}


/************************************************************************
 *
 *                          EXCEP_deregister_cpu_isr
 *  Description :
 *  -------------
 *
 *  Deregisters interrupt handler. 
 *  It is OK to call this function even if we are in interrupt context.
 *  
 *  Parameters :
 *  ------------
 *
 *  'ref',	 IN,   Handle obtained when calling EXCEP_register_cpu_isr
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
    t_EXCEP_ref ref )	             /* In : Handle for deregistration */
{
    UINT32		      old_ie;
    t_INT_handler             *h = (t_INT_handler *)ref;
    t_EXCEP_int_handler_list  *list;
    INT32		      rc = ERROR_EXCEP_ILLEGAL_HANDLER;
    UINT32		      i, t;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    /* Find and deregister handler pointed to by ref (h) */

    if( h == &table.default_int )
    {
        h->handler = NULL;
	rc	   = OK;
    }
    else
    {
        for( i=0; (i<MAX_INTERRUPTS) && (rc != OK); i++ )
        {
            list = &table.interrupt[ i ];
	
	    for( t=0; (t<INT_HANDLERS_PER_LINE) && (rc != OK); t++ )
	    {
	        if( &list->int_handler[t] == h )
	        {
	            (list->list_size)--;
	  	    h->handler = NULL;

		    if( list->list_size == 0 )
		    {
		        /* Disable interrupt mask */
			sys_disable_int_mask( i );
		    }

		    rc = OK;
	        }
	    }
        }
    }

    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();

    return rc;
}


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
    t_EXCEP_ref *ref )		    /* OUT : Handle for deregistration  */
{
    UINT32        old_ie;
    t_INT_handler *h = NULL;
    bool	  enable = FALSE;
    UINT32        rc     = OK;
    int		  i;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    if( ic_int_line == EXCEP_DEFAULT_HANDLER )
    {
        h = &table.default_ctrl;
    }
    else if( ic_int_line >= excep_ic_count )
    {
        /* Interrupt specified is unknown */
	rc = ERROR_EXCEP_ILLEGAL_INTERRUPT;
    }
    else
    {
        /* Specific interrupt */

        if( handler == NULL )
        {
	    rc = ERROR_EXCEP_ILLEGAL_HANDLER;
        }
        else if( table.controller[ic_int_line].list_size == CTRL_HANDLERS_PER_LINE )
        {
            /* Too many handlers have been registered */
            rc = ERROR_EXCEP_NO_SPACE;
        }
        else
        {
            /* Find unused entry in array */
            for( i = 0, h = &table.controller[ic_int_line].ctrl_handler[0];
	         (i < CTRL_HANDLERS_PER_LINE) && (h->handler != NULL);
	         i++, h++) 
            {
	        ;
	    }
	    
            table.controller[ic_int_line].list_size++;
	    enable = TRUE;
        }
    }

    if( rc == OK )
    {
        h->handler = handler;
	h->data    = data;

        if( ref )
            *ref = (t_EXCEP_ref)h;

        /* add this source to enable mask of interrupt controller */
	if( enable )
            arch_excep_enable_int(ic_int_line);
    }
    
    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();

    return rc;
}


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
    t_EXCEP_ref ref )	             /* In : Handle for deregistration */
{
    UINT32		      old_ie;
    t_INT_handler             *h = (t_INT_handler *)ref;
    t_EXCEP_ctrl_handler_list *list;
    INT32		      rc = ERROR_EXCEP_ILLEGAL_HANDLER;
    UINT32		      i, t;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    /* Find and deregister handler pointed to by ref (h) */

    if( h == &table.default_ctrl )
    {
        h->handler = NULL;
	rc	   = OK;
    }
    else
    {
        for( i=0; (i<MAX_IC) && (rc != OK); i++ )
        {
            list = &table.controller[ i ];
	
	    for( t=0; (t<CTRL_HANDLERS_PER_LINE) && (rc != OK); t++ )
	    {
	        if( &list->ctrl_handler[t] == h )
	        {
	            (list->list_size)--;
	  	    h->handler = NULL;

		    if( list->list_size == 0 )
		    {
		        /* Disable interrupt mask */
                        arch_excep_disable_int( i );
		    }

		    rc = OK;
	        }
	    }
        }
    }

    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();

    return rc;
}


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
    UINT32 set ) /* table where setup is copied */
{
    UINT32 old_ie;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    switch( set )
    {
      case EXCEP_HANDLERS_SHELL :
        memcpy( &table_shell, &table, sizeof(t_table) );
	break;
      case EXCEP_HANDLERS_GDB :
        memcpy( &table_gdb,   &table, sizeof(t_table) );
	break;
      default : /* Should not happen */
    }

    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();
}


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
    UINT32 set ) /* table where setup is copied */
{
    UINT32 old_ie;

    /* Disable interrupts */
    old_ie = sys_disable_int();

    switch( set )
    {
      case EXCEP_HANDLERS_SHELL :
        memcpy( &table, &table_shell, sizeof(t_table) );
	break;
      case EXCEP_HANDLERS_GDB :
        memcpy( &table, &table_gdb,   sizeof(t_table) );
	break;
      default : /* Should not happen */
    }

    /* Restore interrupt enable status */
    if(old_ie)
        sys_enable_int();
}


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
    bool       display )	/* TRUE -> Print EPC in ASCII display	*/
{
    char   *s;
    UINT64 *regs;
    UINT64 reg64;
    UINT32 i,j;
    UINT32 werr;
    UINT32 shadowset;

    if (!context)
	context = EXCEP_get_context_ptr();

    if( msg )
        printf( "\n* %s *\n", msg );
    else	
    {
        printf( "\n* Exception " );
        printf( user ? "(user) : " : ": " );
    }

    if( EXCEP_nmi )
    {
	s = "NMI";
	if( SYSCON_read( SYSCON_BOARD_NMI_WERR_ID, 
		          (void *)&werr,
		          sizeof(UINT32) ) == OK )
	{
	    if (werr != 0)
	    {
	        s = "NMI - write access error";
	    }
	}	
    }
    else if( EXCEP_cacheerr )
    {
	s = "CacheErr";
    }
    else
    {
        if( display )
            DISP( (UINT32)context->cp0_epc );
	s = cause_names[ ((UINT32)context->cp0_cause & M_CauseExcCode) >> S_CauseExcCode ];
    }

    if( msg == NULL )
    {
        if (context == &ejtag_context)
	{
            printf( "EJTAG" );
	}
        else
        {
            if( s )
	        printf("%s", s); 
            else
	        printf("Unknown exception code : 0x%02x",
		       ((UINT32)context->cp0_cause & M_CauseExcCode) >> S_CauseExcCode );
        }
    
        printf( " *\n" );
    }

    /* CP0 registers */
    sys_cp0_printreg_all( context );

    printf( "\n" );
    
    if( sys_fpu && (UINT32)context->cp0_status & M_StatusCU1 )
    {
        regs = (UINT64 *)&context->fpr0;

        for(i=0; i<32; i++)
	{
	    if( (UINT32)context->cp0_status & M_StatusFR )
	    {
	        /* All 32 floating point registers are 64 bit wide */
	        j = ((i & 1) << 4) + (i >> 1);

	        reg64 = regs[j];

                printf("f%-2d:0x%08x%08x%s", j,
		        HI32(reg64), LO32(reg64),
		        (i & 1) == 1 ? "\n" : "  ");
	    }
	    else
	    {
	        j = ((i & 3) << 3) + (i >> 2);

                printf("f%-2d:0x%08x%s", j, (UINT32)regs[j],
		        (i & 3) == 3 ? "\n" : "  ");
            }
	}

        printf( "\n" );
    }

    shadowset = 
        ( (UINT32)context->cp0_status & (M_StatusBEV | M_StatusERL) ) ?
            ((UINT32)context->cp0_srsctl & M_SRSCtlCSS) >> S_SRSCtlCSS :
            ((UINT32)context->cp0_srsctl & M_SRSCtlPSS) >> S_SRSCtlPSS;

    if( shadowset != 0 )
	printf( "* Register set %d *\n", shadowset );

    regs = (UINT64 *)&context->reg0;

    for(i=0; i<REG_NAMES_COUNT; i++)
    {
	if( sys_64bit )
	{
	    j = ((i & 1) << 4) + (i >> 1);

	    reg64 = regs[j];

            printf("$%2d(%s):0x%08x%08x%s", j, reg_names[j], 
		    HI32(reg64), LO32(reg64),
		    (i & 1) == 1 ? "\n" : "  ");
	}
	else
	{
	    j = ((i & 3) << 3) + (i >> 2);

            printf("$%2d(%s):0x%08x%s", j, reg_names[j], (UINT32)regs[j],
		    (i & 3) == 3 ? "\n" : "  ");
        }		   
    }
}


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
EXCEP_run_default_esr_handler( void )
{
    if( table.default_exception.handler == NULL )
    {
	/*  No default ESR has been registered, so call our own */
	default_handler( NULL, NULL );    /*  (never returns).  */
    }

    table.default_exception.handler();
}



/************************************************************************
 *
 *                          exception_sr
 *  Description :
 *  -------------
 *
 *  This function implements the exception handler, which gets called
 *  by the first-level exception handler. According to the parameter
 *  'exc_code', any user registered exception handler for the asserted
 *  exception, will be called from this function. 
 *  
 *  Parameters :
 *  ------------
 *
 *  exc_code         the exception code from the 5-bit ExcCode-field in
 *                   the CP0-status register.
 * 
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
exception_sr( 
    UINT32     exc_code,
    UINT32     exc_vector_offs,
    t_gdb_regs *p_exc_context,
    UINT32     exc_return_flag )
{
    t_EXCEP_handler *reg_esr;
    bool	    default_handling;
    t_EXCEP_handler int_esr;

    /* First check for NMI and CacheErr */
    if (exc_vector_offs == SYS_NMI_RAM_VECTOR_OFS)
    {
        /* EXCEP_nmi is cleared by SYSCON_write(SYSCON_BOARD_NMI_ACK_ID) */
        EXCEP_nmi = TRUE;
	default_handler( NULL, p_exc_context );
	return;	/* default_handler never returns */
    }

    if (exc_vector_offs == SYS_CACHEERR_RAM_VECTOR_OFS)
    {
	cacheerr_handler( p_exc_context );
	return;	/* cacheerr_handler never returns */
    }

    if( exc_code < MAX_EXCEPTIONS )
    {
	reg_esr = &table.exception[exc_code];
        default_handling = (reg_esr->handler == NULL);
    }
    else
        default_handling = TRUE;

    if( exc_return_flag )
    {
        /*  A registered exception handler was invoked for this
	 *  exception, but it called EXCEP_return().
	 *  This means that it does not want to handle the exception,
	 *  so we must handle it.
	 *
	 *  Case 1 : If the exception handler was the ESR registered
	 *	     for interrupts, we assume an application has
	 *	     registered an ESR for the interrupt exception.
	 *	     So, we call the ESR, which is used by YAMON
	 *	     for handling interrupt exceptions, i.e.
	 *	     interrupt_sr().
	 *
	 *  Case 2 : If the exception handler was a specific one,
	 *	     other than interrupt, we try default handling.
	 *
	 *  Case 3 : If the exception handler was the default ESR, 
	 *           we must call default_handler().
	 */

	 if( exc_return_flag != 1)
	 {
	     /* nested calls to EXCEP_return() */
	     default_handler( NULL, p_exc_context );
	     return;	/* default_handler never returns */
	 }
	 if( exc_code == EX_INT )
	 {
	     /* Case 1 */
	     interrupt_sr();
	     return;
	 }
	 else if( !default_handling )
	 {
	     /* Case 2 */
	     default_handling = TRUE;
	 }
	 else
	 {
	     /* Case 3 */
	     default_handler( NULL, p_exc_context );
	     return;	/* default_handler never returns */
	 }
    }	     

    if( default_handling )
    {
        /*  Either EXCEP_return() was called (se above), or
	 *  there is no registered ESR for the specific exception. 
	 *  Check if there is a registered default ESR.
	 */

	reg_esr = &table.default_exception;
	
        if( reg_esr->handler == NULL )
        {
	    /*  No default ESR has been registered, so call our own */
	    default_handler( NULL, p_exc_context );
	    return;	/* default_handler never returns */
        }
    }

    if( reg_esr->raw )
        EXCEP_exc_handler_jump( reg_esr->handler );
    else
        reg_esr->handler();
}



/************************************************************************
 *
 *                          exception_ejtag
 *  Description :
 *  -------------
 *  This function implements the ejtag exception handler, called from
 *  the low-level ejtag exception handler. A user registered exception
 *  handler for the specific exception will be called from this function. 
 *  A user registered default ESR handler will NOT be called, since there
 *  is no way to distinguish an ejtag exception.
 *  
 *  Parameters :
 *  ------------
 *  none
 * 
 *  Return values :
 *  ---------------
 *  None
 ************************************************************************/
void 
exception_ejtag(
    t_gdb_regs *p_exc_context,
    UINT32     exc_return_flag )
{
    t_EXCEP_handler *reg_esr;

    reg_esr = &table.ejtag;

    if( exc_return_flag || reg_esr->handler == NULL )
    {
	default_handler( NULL, p_exc_context );
	return;	/* default_handler never returns */
    }

    if( reg_esr->raw )
        EXCEP_exc_handler_jump( reg_esr->handler );
    else
        reg_esr->handler();
}


/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/



/************************************************************************
 *
 *                          interrupt_sr
 *  Description :
 *  -------------
 *
 *  This function implements the CPU interrupt handler, which is
 *  registered for the interrupt exception.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
static void 
interrupt_sr( void )
{
    UINT32                   h, index;
    UINT32		     int_pending;
    t_EXCEP_int_handler_list *table_interrupt;
    t_INT_handler	     *int_handler;
    t_gdb_regs               *p_context;

    /*  Determine the pending interrupts based on CAUSE register IP field
     *  masked with STATUS register IM (Interrupt mask) field.
     */
    p_context = EXCEP_get_context_ptr();

    int_pending = ((p_context->cp0_cause & M_CauseIP) >> S_CauseIP) &
	          ((p_context->cp0_status & M_StatusIM) >> S_StatusIM);

    /* validate 'int_pending' */
    if(int_pending <= (M_CauseIP >> S_CauseIP) )
    {
        /* Handle the 8 possible interrupts: 0..7 */

        /* do call handlers for all pending interrupts */
        for (; int_pending; int_pending ^= 1 << index)
        {
            /*  Handle the interrupts in a sequence, 7..0 */
            index = byte2msbit[int_pending];

            /*  Now, use the derived index to call into the
             *  handler table (index = interrupt line).
             */

	    table_interrupt = &table.interrupt[index];

	    if( table_interrupt->list_size == 0 )
	    {
	        /* No registered handler, so use default */

                if( table.default_int.handler )
		{
		    /* A default handler has been registered, so call it */
                    table.default_int.handler(table.default_int.data);
		}
                else
                {
		    /* No default handler has been registered, so call our own */
		    default_handler( "Unregistered CPU interrupt occurred", NULL );
	            return;	/* default_handler never returns */
                }
	    }
	    else
	    {
                /* For one int. line, more handlers may be registered */
                for( h=0; h < INT_HANDLERS_PER_LINE; h++)
                {
		    int_handler = &table_interrupt->int_handler[h];

                    if( int_handler->handler != NULL)
                        (*int_handler->handler)(int_handler->data);
                }
            }
        }
    }
    else
    {
        /* Should not happen */
	while(1);
    }
}


/************************************************************************
 *
 *                          controller_sr
 *  Description :
 *  -------------
 *
 *  This function implements the generic interrupt handler for the 
 *  interrupt controller. It gets called by the second-level 
 *  interrupt handler (interrupt_sr), when the HW-INT-line of 
 *  the interrupt controller is asserted.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
static void 
controller_sr( 
    void *data )     /* Data pointer set during registration (NULL)	*/
{
    UINT32 pending, mask, int_set;
    UINT32 h, j, index;
    char   msg[80];

    /* get the pending controller interrupts */

    while( (pending = arch_excep_pending()) != 0)
    {
        mask = pending;

        /* handle the max. int. sources in rounds of 8 */
        j = 0;

        while( j<MAX_IC )
        {
            /* select this round's max. 8 int. sources */
            int_set = (pending & 0x000000ff);

            /* check for any set */
            for (; int_set; int_set ^= 1 << (index & 7))
            {
                /*  Handle the interrupts in a sequence, 7..0 */
                index = byte2msbit[int_set] + j;

		/* index == interrupt line */

                /*  Now, use the derived index to call into the
                 *  handler table.
		 */
	        if( table.controller[index].list_size == 0 )
	        {
		    /* No registered handler, so use default */

		    if( table.default_ctrl.handler )
		    {
			/* A default handler has been registered, so call it */
			table.default_ctrl.handler(table.default_int.data);
		    }			
		    else
		    {
		        /* No default handler has been registered, so call our own */
		        sprintf( msg, "Unregistered interrupt controller event occurred,"
		    		  " mask = 0x%08x", mask );
    		        default_handler( msg, NULL );
		        return;	/* default_handler never returns */
		    }
	        }
		else
		{
                    for( h=0; h < CTRL_HANDLERS_PER_LINE; h++)
			{
                        /* For one int. line, more handlers may be registered */
			if(table.controller[index].ctrl_handler[h].handler != NULL)
			{
                            /* call the handlers in sequence */
			    (*table.controller[index].ctrl_handler[h].handler)
                               (table.controller[index].ctrl_handler[h].data);
                        }
                    }
		}
		    
		/* Issue End Of Interrupt (EOI) */
		arch_excep_eoi(index);
            }

            pending /= 256;
            j       += 8;
        }
    }
}


/************************************************************************
 *
 *                          default_handler
 *  Description :
 *  -------------
 *
 *  Default exception handler
 *
 ************************************************************************/
static void
default_handler(
    char *msg,
    t_gdb_regs *p_context )
{
    /* Print context */
    EXCEP_print_context( p_context, msg, FALSE, TRUE );

    /* Take a breath */
    sys_wait_ms(1000);

    /* re-initialize shell context and enter shell prompt */
    shell_reenter( FALSE );
}


/************************************************************************
 *
 *                          cacheerr_handler
 *  Description :
 *  -------------
 *
 *  CacheErr exception handler
 *
 ************************************************************************/
static void
cacheerr_handler(
    t_gdb_regs *p_context )
{
    /* Change modified context->C0_config to its unmodified value */
    p_context->cp0_config = (INT64)EXCEP_C0_Config_cacheerr;

    /* Display error type */
    DISP_STR( "CacheErr" );

    /* Print context */
    EXCEP_cacheerr = TRUE;
    EXCEP_print_context( p_context, NULL, FALSE, FALSE );
    EXCEP_cacheerr = FALSE;

    /* Take a breath */
    sys_wait_ms(1000);

    /* re-initialize shell context and enter shell prompt */
    shell_reenter( TRUE );
}
