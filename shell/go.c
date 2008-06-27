
/************************************************************************
 *
 *  go.c
 *
 *  Shell go command
 *
 *  go [?|.|<address> [<args>]]",
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
#include <syscon_api.h>
#include <string.h>
#include <stdio.h>
#include <shell_api.h>
#include <yamon_api.h>
#include <string.h>
#include <mips.h>
#include <sys_api.h>
#include <excep_api.h>
#include <env_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/


/************************************************************************
 *  Public variables
 ************************************************************************/

UINT32 shell_argc_appl; 
char   *(shell_argv_appl[SHELL_APPL_MAX_ARGS]);
t_gdb_regs   shell_context;
t_gdb_regs   appl_context;
UINT32       appl_shell_func_code;

/************************************************************************
 *  Static variables
 ************************************************************************/

static char *go_string = "go";
static bool  shift_gdb_retval;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    void   **address,
    bool   *disp_addr );

static void
appl_exception( void );


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          go
 ************************************************************************/
static MON_FUNC(go)
{
    void   *address;
    bool   disp_addr;
    UINT32 rc;

    /* Options */
    rc = get_options( argc, argv, &address, &disp_addr );

    if( rc != OK )
        return rc;

    if( disp_addr )
    {
	if( shell_addr_go_valid )
	    printf( "Address = 0x%08x\n", (UINT32)shell_addr_go );
	else
	    printf( "Address unknown\n" );
    }
    else
    {
        /* Store start address */
        shell_addr_go_valid = TRUE;
        shell_addr_go       = address;

        /* Setup startup conditions */
	shell_setup_default_cpu( &appl_context, (UINT32)address );

	/* Shift to user application */
        shell_shift_to_user( FALSE, TRUE );
    }

    return OK;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    void   **address,
    bool   *disp_addr )
{
    char	   *token;
    bool           ok   = TRUE;
    bool	   args = FALSE;
    bool	   addr_valid;
    t_shell_option decode;
    UINT32	   arg;
    UINT32	   error = SHELL_ERROR_SYNTAX;

    /* Defaults */
    if( shell_addr_go_valid )
    {
        *address   = shell_addr_go;
	addr_valid = TRUE;
    }
    else
        addr_valid = FALSE;

    shell_argc_appl    = 1;
    shell_argv_appl[0] = go_string;
    *disp_addr         = FALSE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
		  (token = argv[arg]);
         arg++ )
    {
        if( args )
	{
	    if( shell_argc_appl == SHELL_APPL_MAX_ARGS )
	    {
	        return SHELL_ERROR_ARGV;
	    }
	    else
	    {
	        shell_argv_appl[shell_argc_appl] = token;
	        shell_argc_appl++;
	    }
        }
	else
	{
	    if( *token == '-' )
	    {
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = token;
		ok		 = FALSE;
	    }
	    else if( strcmp( token, "?" ) == 0 )
	    {
		if( argc != 2 )
		    ok = FALSE;
		else
	            *disp_addr = TRUE;
	    }
            else if( strcmp( token, "." ) == 0 )
	    {
	        args = TRUE;
	    }
            else
	    {
		ok = shell_decode_number( &decode, NULL, token );

		if( ok )
		{
	            *address   = (void *)decode.number;
		    addr_valid = TRUE;
	            args       = TRUE;
		}
	    }
	}
    }

    if( !ok )
    {
	return error;
    }

    if( *disp_addr )
        return OK;
    
    if( !addr_valid )
    {
	return SHELL_ERROR_ADDRESS_UNKNOWN;
    }

    if( sys_mips16e && ((UINT32)(*address) & 1) )
    {
        return sys_validate_range( (UINT32)(*address) & ~1, 
			           sizeof(UINT16), 
			           sizeof(UINT16),
			           FALSE );
    }
    else
    {
        return sys_validate_range( (UINT32)(*address), 
			           sizeof(UINT32), 
			           sizeof(UINT32),
			           FALSE );
    }
}


/************************************************************************
 *                          appl_exception
 ************************************************************************/
static void
appl_exception( void )
{
    /* Called from exception_sr()
     *  (normal or ejtag exception context holds context)
     */

    t_gdb_regs *context = EXCEP_get_context_ptr();

    if( shift_gdb_retval )
    {
	/* return value of gdb_shift_to_appl() in case of exception */
	shift_gdb_retval = FALSE;

	memcpy( &appl_context, context, sizeof(t_gdb_regs ) );
    }
    else
    {
	/* Print context */
	EXCEP_print_context( context, NULL, TRUE, FALSE );
	printf( "\n" );
    }

    EXCEP_exc_handler_ret_ss( &shell_context );
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "go",
     go,
    "go [?|.|<address> [<args>]]",

    "Execute application code.\n"
    "\n"
    "If a target address is not specified, the address obtained from the last\n"
    "successful 'load' or 'go' command (if any) is used as the target address.\n"
    "This address may be determined by issuing a 'go ?' command. The application\n"
    "will not be executed in this case.\n"
    "\n"
    "If arguments for the user program need to be specified, the default\n"
    "execution address can be referenced by a '.'.\n"
    "\n"
    "<args> is broken up in substrings and passed to the application.\n"
    "The first argument (argv[0]) will be the string 'go'.\n"
    "\n"
    "a0 is set to the number of substrings (argc).\n"
    "a1 is set to the address of an array of pointers to the substrings (argv).\n"
    "a2 is set to the address of the environment table.\n"
    "a3 is set to the memory size.\n"
    "ra holds the return address to YAMON.\n"
    "\n"
    "The application may return to YAMON by jumping to the address\n"
    "specified in ra or by calling the exit(rc) function supplied by YAMON.",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *                          shell_shift_to_user
 ************************************************************************/

/************************************************************************
 *
 *                          shell_shift_to_user
 *  Description :
 *  -------------
 *
 *  Shift/return to user context
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> application ended.
 *
 ************************************************************************/
bool
shell_shift_to_user( 
   bool from_gdb,  /* TRUE -> invoked from GDB */
   bool first )	   /* TRUE -> Start application (only FALSE for GDB reentry) */
{
    UINT32 old_ie;

    /* also used as switch in appl_exception() */
    shift_gdb_retval = from_gdb;

    /* Stop DMA */
    sys_dma_enable( FALSE );

    if( first )
    {
        /* Register default exc handler */
        EXCEP_register_esr( EXCEP_DEFAULT_HANDLER,
			    FALSE,
			    appl_exception,
			    NULL,
			    NULL );
        if( from_gdb )
        {
            /* GDB stub needs to own BREAK exception */
            EXCEP_register_esr( EX_BP,
			        FALSE,
			        appl_exception,
			        NULL,
			        NULL );
        }
    }
    else
    {
        /*  GDB only (otherwise 'first' is always TRUE).
	 *  Restore ESR settings as they were at last breakpoint. 
	 */
        EXCEP_set_handlers( EXCEP_HANDLERS_GDB );
    }

    /* Save our (yamon) shell context as if an exception had occurred */
    old_ie = sys_disable_int();

    if ( EXCEP_save_context(&shell_context) == 0 )
    {
	/* first return - after context save */
	/* flush dcache to memory in case application initialises caches */
	sys_dcache_flush_all();
	/* Shift to application */
	EXCEP_exc_handler_ret( &appl_context );
	/* dead end */
    }
    else
    {
	/* second return - after application has ended (or GDB break happens) */

        if( from_gdb)
	{
	    /* GDB breakpoint : Store current ESR settings */
	    EXCEP_store_handlers( EXCEP_HANDLERS_GDB );
	}

        /* Restore shell setup (incl. ESR settings) */
        shell_restore();

	if (old_ie) sys_enable_int();
    }

    return shift_gdb_retval;
}


/************************************************************************
 *				appl_shell_func
 ************************************************************************/
UINT32
appl_shell_func( UINT32 arg0, UINT32 arg1, UINT32 arg2, UINT32 arg3 )
{
    bool   exit2yamon;
    char   *s;
    UINT32 i, rc;
    UINT64 epc;

    /* Called from appl_if.S */

    exit2yamon = FALSE;	/* Default */

    switch( appl_shell_func_code )
    {
      case SHELL_FUNC_EXIT_CODE :
	/* Normal return / exit() function */

	printf("User application returned with code = 0x%08x\n",
		      (UINT32)SYS_CPUREG(&appl_context, SYS_CPUREG_A0) );
	exit2yamon = TRUE;
	break;

      case SHELL_FUNC_PRINT_COUNT_CODE :
        /*  a0 = Output port (ignored).
	 *  a1 = String pointer.
	 *  a2 = Number of characters.
	 */

	s = (char *)arg1;
	for( i = arg2; i; i-- )
	{
	    sys_putchar( DEFAULT_PORT, *s );
	    s++;
        }
	rc = OK;
	break;

      case SHELL_FUNC_PRINT_CODE :
	/*  a0 = Output port (ignored).
	 *  a1 = Null terminated string.
	 */

	sys_puts( DEFAULT_PORT, (char *)arg1 );
	rc = OK;
	break;

      case SHELL_FUNC_GETCHAR_CODE :
	/*  a0 = Input port (ignored).
	 *  a1 = Pointer to char.
	 */
	
	rc = sys_getchar( DEFAULT_PORT, (char *)arg1 );
	break;

      case SHELL_FUNC_SYSCON_READ_CODE :
	/*  a0 = SYSCON id.
	 *  a1 = Pointer to param.
	 *  a2 = size of param.
	 */
	
	rc = SYSCON_read( (t_syscon_ids) arg0,
		                (void *) arg1,
		                         arg2 );
	break;

      case SHELL_FUNC_FLUSH_CODE :
	/*  a0 = 
	 *     YAMON_FLUSH_ICACHE : Flush ICACHE
	 *     YAMON_FLUSH_DCACHE : Flush DCACHE
	 */
	if( arg0 == YAMON_FLUSH_ICACHE )
	    sys_icache_invalidate_all();
	else
	    sys_dcache_flush_all();
	rc = OK;
	break;

      case SHELL_FUNC_REGISTER_CPU_ISR_CODE :
	/*  a0 = CPU interrupt number
	 *  a1 = Pointer to ISR
	 *  a2 = Pointer to data
	 *  a3 = Pointer to ref
	 */

        EXCEP_register_cpu_isr( arg0,
	           (t_EXCEP_isr)arg1,
                        (void *)arg2,
                 (t_EXCEP_ref *)arg3 );

	/* return value is set to the contents of the STATUS register */
        SYSCON_read(
            SYSCON_CPU_CP0_STATUS_ID,
	    (void *)&rc,
	    sizeof(UINT32) );	
        break;

      case SHELL_FUNC_DEREGISTER_CPU_ISR_CODE :
	/* a0 = ref */

        EXCEP_deregister_cpu_isr( (t_EXCEP_ref)arg0 );

	/* return value is set to the contents of the STATUS register */
        SYSCON_read(
            SYSCON_CPU_CP0_STATUS_ID,
	    (void *)&rc,
	    sizeof(UINT32) );	
        break;

      case SHELL_FUNC_REGISTER_IC_ISR_CODE :
	/*  a0 = IC line number
	 *  a1 = Pointer to ISR
	 *  a2 = Pointer to data
	 *  a3 = Pointer to ref
	 */

        rc = EXCEP_register_ic_isr( arg0,
                       (t_EXCEP_isr)arg1,
                            (void *)arg2,
                     (t_EXCEP_ref *)arg3 );

        break;

      case SHELL_FUNC_DEREGISTER_IC_ISR_CODE :
	/* a0 = ref */

        rc = EXCEP_deregister_ic_isr( (t_EXCEP_ref)arg0 );
        break;

      case SHELL_FUNC_REGISTER_ESR_CODE :
	/*  a0 = Exception code
	 *  a1 = Pointer to ESR
	 *  a2 = Pointer to ref
	 *  a3 = Pointer to retfunc
	 */

	rc = EXCEP_register_esr( arg0,
			         TRUE,
                    (t_EXCEP_esr)arg1,
                  (t_EXCEP_ref *)arg2,
              (t_EXCEP_retfunc *)arg3 );
        break;

      case SHELL_FUNC_DEREGISTER_ESR_CODE :
	/* a0 = ref */

	rc = EXCEP_deregister_esr( (t_EXCEP_ref)arg0 );
        break;

      default :	/* Should not happen */
	printf( "appl_shell_func: unknown argument = 0x%08x\n",appl_shell_func_code);
	exit2yamon = TRUE;
	break;
    }

    if( exit2yamon )
    {
	/* return to (yamon) shell */
	EXCEP_exc_handler_ret_ss( &shell_context );
    }
    else
    {
	/* return to application. */
	SYS_CPUREG(&appl_context, SYS_CPUREG_V0) = rc;
    }
}


/************************************************************************
 *
 *                          shell_setup_default_cpu
 *  Description :
 *  -------------
 *
 *  Setup CPU (incl. CP0) registers in context structure so that they 
 *  contain values suiting an application.
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
void
shell_setup_default_cpu(
    t_gdb_regs *regs,
    UINT32     epc )
{
    /* Clear data */
    memset( regs, 0, sizeof(t_gdb_regs) );

    /* ra (make sure jr ra is handled the same way as exit()) */
    SYS_CPUREG(regs, SYS_CPUREG_RA) = (UINT64)shell_return;

    /* sp (4 words below top according to C-calling convention) */
    SYS_CPUREG(regs, SYS_CPUREG_SP) = (UINT64)(sys_freemem - 4 * sizeof(UINT32));

    /* fp (set equal to sp) */
    SYS_CPUREG(regs, SYS_CPUREG_FP) = SYS_CPUREG(regs, SYS_CPUREG_SP);

    /* frame_ptr field (Dummy GDB reg, should not be used) */
    regs->frame_ptr = 0;

    /* a0 */
    SYS_CPUREG(regs, SYS_CPUREG_A0) = (UINT64)shell_argc_appl;

    /* a1 */
    SYS_CPUREG(regs, SYS_CPUREG_A1) = (INT64)((INT32)shell_argv_appl);

    /* a2 points to environment variables */
    SYS_CPUREG(regs, SYS_CPUREG_A2) = (INT64)((INT32)env_get_all());

    /* a3 is set to the memory size */
    SYS_CPUREG(regs, SYS_CPUREG_A3) = (UINT64)sys_ramsize;

    /* Retrieve current setting of CP0/CP1 control registers */
    sys_store_control_regs( regs );
    
    /*  CP0 status :
     *    Clear ERL and set EXL so that EPC will be used upon eret.
     *    Disable interrupts.
     */
    regs->cp0_status &= ~(M_StatusERL | M_StatusIE);
    regs->cp0_status |= M_StatusEXL;

    /* EPC (sign extended) */
    regs->cp0_epc = (INT64)((INT32)epc);
}


/************************************************************************
 *
 *                          shell_go_init
 *  Description :
 *  -------------
 *
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
t_cmd *
shell_go_init( void )
{
    UINT32 code;
    for (code = 0; code < SHELL_FUNC_COUNT; code++)
    {
	/* set up vectorized calls to appl_shell_func() */
	shell_api_vector_setup(SHELL_VECTOR_ADDR | (code<<3), code);
    }
    return &cmd_def;
}
