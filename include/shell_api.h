
/************************************************************************
 *
 *  shell_api.h
 *
 *  Shell module definitions
 *
 *  The user may choose to define the following symbols 
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


#ifndef SHELL_API_H
#define SHELL_API_H


#ifndef _ASSEMBLER_

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <excep_api.h>
#include <syscon_api.h>

/************************************************************************
 *  Definitions
*************************************************************************/

/* Monitor maximum lines per screen and maximum chars per line */
#define MON_DEF_LINEMAX		24
#define MON_DEF_LINEWIDTH	79

/* Shell command definitions */
#define MON_FUNC( name )   UINT32 name(UINT32 argc, char **argv)
typedef                    UINT32 (*t_func)(UINT32 argc, char **argv);

typedef struct
{
    char  *option;			/* Name of option		*/
    char  *descr;			/* Description of option	*/
}
t_cmd_option;

typedef struct
{
    char         *name;			/* Name of commands		*/
    t_func	 func;			/* Function implementing cmd	*/
    char	 *syntax;		/* Syntax of command		*/
    char	 *descr;		/* Detailed description of cmd	*/
    t_cmd_option *options;		/* Command options		*/
    UINT32	 option_count;		/* Number of options		*/
    bool	 secret;		/* if TRUE, help will ignore	*/
}
t_cmd;


#define SHELL_PUTS( s )			shell_puts( s, 0 )
#define SHELL_PUTS_INDENT( s, indent )  shell_puts( s, indent )
#define SHELL_PUTC( c )		        shell_putc( c, 0 )
#define SHELL_PUTC_INDENT( c, indent )	shell_putc( c, indent )
#define SHELL_DISABLE_MORE		shell_setmore( FALSE )
#define SHELL_ENABLE_MORE		shell_setmore( TRUE )


/* Error codes */
#define ERROR_SHELL                  0x00010000 /* for compile time check    */
#define SHELL_ERROR_SYNTAX              0x10000
#define SHELL_ERROR_OPTION              0x10001
#define SHELL_ERROR_PARSE_UNKNOWN_ENV   0x10002
#define SHELL_ERROR_PARSE_MISSING_QUOTE 0x10003
#define SHELL_ERROR_PARSE_ARGCOUNT      0x10004
#define SHELL_ERROR_PARSE_LONG_TOKEN    0x10005
#define SHELL_ERROR_PARSE_LONG_LINE     0x10006
#define SHELL_ERROR_PARSE_LONG_ENV      0x10007
#define SHELL_ERROR_ALIGN               0x10008
#define SHELL_ERROR_TLB                 0x10009
#define SHELL_ERROR_TLB_WP              0x1000a
#define SHELL_ERROR_TLB_INDEX           0x1000b
#define SHELL_ERROR_TLB_PAGESIZE        0x1000c
#define SHELL_ERROR_TLB_ASID            0x1000d
#define SHELL_ERROR_TLB_GLOBAL          0x1000e
#define SHELL_ERROR_TLB_C               0x1000f
#define SHELL_ERROR_TLB_D               0x10010
#define SHELL_ERROR_TLB_V               0x10011
#define SHELL_ERROR_OVERFLOW            0x10012
#define SHELL_ERROR_COMMAND_NOT_FOUND   0x10013
#define SHELL_ERROR_NOT_FOUND           0x10014
#define SHELL_ERROR_AMBIVALENT          0x10015
#define SHELL_ERROR_ARGV                0x10016
#define SHELL_ERROR_FAILED              0x10017
#define SHELL_ERROR_ADDRESS             0x10018
#define SHELL_ERROR_ADDRESS_UNKNOWN     0x10019
#define SHELL_ERROR_RAM_RANGE           0x1001a
#define SHELL_ERROR_LOAD                0x1001b
#define SHELL_ERROR_BOOTPROT            0x1001c
#define SHELL_ERROR_PORT                0x1001d
#define SHELL_ERROR_IP                  0x1001e
#define SHELL_ERROR_FILENAME            0x1001f
#define SHELL_ERROR_VAR_FLASH           0x10020
#define SHELL_ERROR_VAR_VALUE           0x10021
#define SHELL_ERROR_DATA_WIDTH          0x10022
#define SHELL_ERROR_UNKNOWN_CP0_REG     0x10023
#define SHELL_ERROR_UNKNOWN_CP1_REG     0x10024
#define SHELL_ERROR_RO_CP0_REG          0x10025
#define SHELL_ERROR_ILLEGAL_CACHE_CFG   0x10026
#define SHELL_ERROR_BAUDRATE            0x10027
#define SHELL_ERROR_CONTROL_C_DETECTED  0x10028
#define SHELL_ERROR_STRUCTURE           0x10029
#define SHELL_ERROR_ILLEGAL_MSG         0x1002a


/************************************************************************
 *  Public variables
 ************************************************************************/

/*  If error code is returned to shell, this pointer may be set to a
 *  string describing the error and possibly a hint.
 */
extern char  *shell_error_data;
extern char  *shell_error_hint;

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell_setup
 *  Description :
 *  -------------
 *
 *  Register commands and start shell
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
shell_setup(void);


/************************************************************************
 *
 *                          shell_register_cmd
 *  Description :
 *  -------------
 *
 *  Register a command for the shell (commands must be setup before
 *  starting shell)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
shell_register_cmd( 
    t_cmd *cmd );			/* Command to be registered	*/


/************************************************************************
 *
 *                          shell_arch
 *  Description :
 *  -------------
 *
 *  Platform specific setup of shell
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
shell_arch(void);


/************************************************************************
 *
 *                          shell_arch_info
 *  Description :
 *  -------------
 *
 *  Platform specifics to be printed by "info board" command
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE : No ctrl-c detected, FALSE : ctrl-c detected
 *
 ************************************************************************/
bool
shell_arch_info( 
    UINT32 indent );


/************************************************************************
 *
 *                          shell_reenter
 *  Description :
 *  -------------
 *  Reenters shell() after exception register dump.
 *  Any remaining counting and/or sub commands are aborted.
 *
 *  Return values :
 *  ---------------
 *  None, function never returns
 *
 ************************************************************************/
void
shell_reenter( bool go_uncached );


/************************************************************************
 *
 *                          shell_sysctrl_info
 *  Description :
 *  -------------
 *
 *  Platform specifics to be printed by "info sysctrl" command
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE : No ctrl-c detected, FALSE : ctrl-c detected
 *
 ************************************************************************/
bool
shell_sysctrl_info( 
    UINT32 indent );


/************************************************************************
 *
 *                          shell_puts
 *  Description :
 *  -------------
 *
 *  Print string to stdout
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> Ctrl^C was pressed
 *
 ************************************************************************/
bool
shell_puts(
    char   *string,
    UINT32 indent );


/************************************************************************
 *
 *                          shell_putc
 *  Description :
 *  -------------
 *
 *  Print char to stdout
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> Ctrl^C was pressed
 *
 ************************************************************************/
bool
shell_putc(
    char   ch,
    UINT32 indent );


/************************************************************************
 *
 *                          shell_setmore
 *  Description :
 *  -------------
 *
 *  Enable/disable 'more' control from shell_puts and shell_putc
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
shell_setmore(
    bool enable_more );


/************************************************************************
 *
 *                          shell_print_dot
 *  Description :
 *  -------------
 *
 *  Print dot on screen
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
bool
shell_print_dot(
    UINT32 *count );


/************************************************************************
 *
 *                          shell_info_init
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
shell_info_init(
    bool pci,			/* TRUE -> Board supports PCI		*/
    bool ide,			/* TRUE -> Board supports IDE		*/
    bool isa,			/* TRUE -> Board supports ISA bus	*/
    bool lan,			/* TRUE -> Board supports Ethernet	*/
    bool eeprom );		/* TRUE -> Boards supports EEPROM	*/


/************************************************************************
 *
 *                          shell_help_init
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
shell_help_init( 
    t_cmd  **cmd,
    UINT32 cmd_count );


/***** Shell commands ****/

t_cmd *shell_scpu_init(bool cache, bool mmu);
t_cmd *shell_cache_init(void);
t_cmd *shell_cksum_init(void);
t_cmd *shell_compare_init(void);
t_cmd *shell_copy_init(void);
t_cmd *shell_cp0_init(void);
t_cmd *shell_cp1_init(void);
t_cmd *shell_date_init(void);
t_cmd *shell_dis_init(void);
t_cmd *shell_disk_init(void);
t_cmd *shell_dump_init(void);
t_cmd *shell_dump_sdb_init(void);
t_cmd *shell_echo_init(void);
t_cmd *shell_edit_init(void);
t_cmd *shell_eeprom_init(void);
t_cmd *shell_erase_init(void);
t_cmd *shell_erase_sdb_init(void);
t_cmd *shell_fill_init(void);
t_cmd *shell_flush_init(void);
t_cmd *shell_fpuemul_init(void);
t_cmd *shell_fread_init(void);
t_cmd *shell_fwrite_init(void);
t_cmd *shell_gdb_init(void);
t_cmd *shell_go_init(void);
t_cmd *shell_info_sdb_init(void);
t_cmd *shell_load_init(void);
t_cmd *shell_load_sdb_init(void);
t_cmd *shell_off_init(void);
t_cmd *shell_pcicfg_init(void);
t_cmd *shell_ping_init(void);
t_cmd *shell_port_init(void);
t_cmd *shell_reset_init(void);
t_cmd *shell_search_init(void);
t_cmd *shell_setenv_init(void);
t_cmd *shell_sleep_init(void);
t_cmd *shell_stty_init(void);
t_cmd *shell_stty_sdb_init(void);
t_cmd *shell_test_init(void);
t_cmd *shell_test_sdb_init(void);
t_cmd *shell_tlb_init(void);
t_cmd *shell_unsetenv_init(void);
t_cmd *shell_cpu_init(void);

#endif /* #ifndef _ASSEMBLER_ */


/* Vector table address */
#define SHELL_VECTOR_ADDR  0x80001000

/* Shell table function codes */
#define SHELL_FUNC_EXIT_CODE		    0
#define SHELL_FUNC_PRINT_COUNT_CODE	    1
#define SHELL_FUNC_PRINT_CODE		    2
#define SHELL_FUNC_FLUSH_CODE		    3
#define SHELL_FUNC_REGISTER_CPU_ISR_CODE    4
#define SHELL_FUNC_DEREGISTER_CPU_ISR_CODE  5
#define SHELL_FUNC_REGISTER_IC_ISR_CODE	    6
#define SHELL_FUNC_DEREGISTER_IC_ISR_CODE   7
#define SHELL_FUNC_REGISTER_ESR_CODE	    8
#define SHELL_FUNC_DEREGISTER_ESR_CODE	    9
#define SHELL_FUNC_GETCHAR_CODE		    10
#define SHELL_FUNC_SYSCON_READ_CODE	    11
#define SHELL_FUNC_COUNT		    (SHELL_FUNC_SYSCON_READ_CODE + 1)

#endif /* #ifndef SHELL_API_H */


