
/************************************************************************
 *
 *  shell.h
 *
 *  Private shell module definitions
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


#ifndef _SHELL_H
#define _SHELL_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <shell_api.h>
#include <sysdefs.h>
#include <sysenv_api.h>
#include <syscon_api.h>
#include <excep_api.h>
#include <env_api.h>
#include <gdb_stub.h>
#include <syserror.h>

/************************************************************************
 *  Definitions
*************************************************************************/


/* Error text strings (correspond to the error codes defined in shell_api.h) */

#define SHELL_ERR_MSG \
{\
    /* SHELL_ERROR_SYNTAX */\
    "Illegal syntax",\
\
    /* SHELL_ERROR_OPTION */\
    "Illegal option",\
\
    /* SHELL_ERROR_PARSE_UNKNOWN_ENV */\
    "Unknown environment variable",\
\
    /* SHELL_ERROR_PARSE_MISSING_QUOTE */\
    "Unmatched quote",\
\
    /* SHELL_ERROR_PARSE_ARGCOUNT */\
    "Too many tokens",\
\
    /* SHELL_ERROR_PARSE_LONG_TOKEN */\
    "Token too long",\
\
    /* SHELL_ERROR_PARSE_LONG_LINE */\
    "Line too long",\
\
    /* SHELL_ERROR_PARSE_LONG_ENV */\
    "Environment variable too long",\
\
    /* SHELL_ERROR_ALIGN */\
    "Illegal alignment",\
\
    /* SHELL_ERROR_TLB */\
    "No match in TLB for mapped address",\
\
    /* SHELL_ERROR_TLB_WP */\
    "TLB page write-protected",\
\
    /* SHELL_ERROR_TLB_INDEX */\
    "Illegal TLB index",\
\
    /* SHELL_ERROR_TLB_PAGESIZE */\
    "Illegal TLB pagesize",\
\
    /* SHELL_ERROR_TLB_ASID */\
    "Illegal TLB ASID",\
\
    /* SHELL_ERROR_TLB_GLOBAL */\
    "Illegal TLB Global setting",\
\
    /* SHELL_ERROR_TLB_C */\
    "Illegal Cache algorithm",\
\
    /* SHELL_ERROR_TLB_D */\
    "Illegal TLB Dirty setting",\
\
    /* SHELL_ERROR_TLB_V */\
    "Illegal TLB Valid setting",\
\
    /* SHELL_ERROR_OVERFLOW */\
    "Data overflow",\
\
    /* SHELL_ERROR_COMMAND_NOT_FOUND */\
    "Command not found",\
\
    /* SHELL_ERROR_NOT_FOUND */\
    "Unknown command",\
\
    /* SHELL_ERROR_AMBIVALENT */\
    "Ambivalent command",\
\
    /* SHELL_ERROR_ARGV */\
    "Too many arguments",\
\
    /* SHELL_ERROR_FAILED */\
    "Operation failed",\
\
    /* SHELL_ERROR_ADDRESS */\
    "Illegal address",\
\
    /* SHELL_ERROR_ADDRESS_UNKNOWN */\
    "Address unknown",\
\
    /* SHELL_ERROR_RAM_RANGE */\
    "Illegal RAM range",\
\
    /* SHELL_ERROR_LOAD */\
    "Load failed",\
\
    /* SHELL_ERROR_BOOTPROT */\
    "Illegal protocol",\
\
    /* SHELL_ERROR_PORT */\
    "Illegal port",\
\
    /* SHELL_ERROR_IP */\
    "Illegal IP address",\
\
    /* SHELL_ERROR_FILENAME */\
    "Illegal filename",\
\
    /* SHELL_ERROR_VAR_FLASH */\
    "Environment variable could not be accessed",\
\
    /* SHELL_ERROR_VAR_VALUE */\
    "Illegal value",\
\
    /* SHELL_ERROR_DATA_WIDTH */\
    "Illegal data/width combination",\
\
    /* SHELL_ERROR_UNKNOWN_CP0_REG */\
    "Unknown CP0 register",\
\
    /* SHELL_ERROR_UNKNOWN_CP1_REG */\
    "Unknown CP1 register",\
\
    /* SHELL_ERROR_RO_CP0_REG */\
    "Read-only CP0 register",\
\
    /* SHELL_ERROR_ILLEGAL_CACHE_CFG */\
    "Illegal cache configuration",\
\
    /* SHELL_ERROR_BAUDRATE */\
    "Baudrate not supported",\
\
    /* SHELL_ERROR_CONTROL_C_DETECTED */\
    "Stopped due to Ctrl-C",\
\
    /* SHELL_ERROR_STRUCTURE */\
    "Structural error",\
\
    /* SHELL_ERROR_ILLEGAL_MSG */\
    "Illegal error msg number"\
}

typedef union
{
#define SHELL_TOKEN_NUMBER	0
    UINT32 number;
#define SHELL_TOKEN_NUMBER64	1
    UINT64 number64;
#define SHELL_TOKEN_OPTION	2
    char   *option;
#define SHELL_TOKEN_STRING	3
    char   *string;
#define SHELL_TOKEN_IP		4
    UINT32 ip;
#define SHELL_TOKEN_UNKNOWN	5
}
t_shell_option;


#define SHELL_RADIX_DEFAULT	16


/*  Maximum length of single line including prompt (commands may be
 *  continued on next line by ending the line with a '\' character.
 */
#define SHELL_MAX_LINE_LEN	    79

/*  Maximum total command length (not including terminating '\0').
 *  (multi line)
 */
#define SHELL_MAX_COMMAND_LEN	    (4 * SHELL_MAX_LINE_LEN)

/* Maximum length of parsed token (not including terminating '\0') */
#define SHELL_MAX_TOKEN_LEN         (2 * SHELL_MAX_LINE_LEN)

/* Maximum length of environment variable (not including '\0') */
#define SHELL_MAX_ENV_LEN	    ENV_VAR_MAXLEN_NAME


/* Cyclical buffer definitions */
#define BUFFERSIZE	            1000
#define BUFFER_NU		    0xFFFFFFFF

typedef struct
{
    UINT32 first;
    UINT32 free;
    char   data[BUFFERSIZE];
}
t_shell_line_buf;

/* Max arguments to user application */
#define SHELL_APPL_MAX_ARGS	     20


typedef struct
{
    char  *name;
    char  *descr;
    UINT8 val;
#define SHELL_CACHE_CONFIG_L2_ENABLE	0xFF
#define SHELL_CACHE_CONFIG_L2_DISABLE	0xFE
}
t_shell_cache_config_name_val;

/************************************************************************
 *  Public variables
 ************************************************************************/

extern void   *shell_addr_go;
extern bool   shell_addr_go_valid;
extern char   *shell_msg_ctrl_c;
extern char   *shell_msg_stopped;
extern char   *_shell_date, *_shell_time;
extern char   *(shell_argv_appl[SHELL_APPL_MAX_ARGS]);
extern UINT32 shell_argc_appl; 
extern bool   shell_dot;
extern bool   shell_fpu_warning;

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell
 *  Description :
 *  -------------
 *
 *  Implements the shell
 *
 *  Return values :
 *  ---------------
 *
 *  None, function never returns
 *
 ************************************************************************/
void 
shell(
    t_cmd  **cmd_list,		/* Array of pointers to shell commands  */
    UINT32 cmd_count );		/* Number of shell commands		*/


/************************************************************************
 *
 *                          shell_lookup_cmd
 *  Description :
 *  -------------
 *
 *  Search command line for (possibly partial) match in command array. 
 *  Also to be used for command completion.
 *
 *  Return values :
 *  ---------------
 *
 *  Pointer to command found, if any (else NULL)
 *
 ************************************************************************/
t_cmd *
shell_lookup_cmd(
    char   *name,			/* Command name			*/
    bool   *ambivalent,			/* TRUE -> match is not unique  */
    UINT32 *len,			/* Number of valid chars	*/
    t_cmd  **cmd_list,			/* array of ptr to commands	*/
    UINT32 cmd_count );			/* Number of commands		*/


/************************************************************************
 *                          shell_command_error
 ************************************************************************/
void
shell_command_error(
    t_cmd  *cmd,			/* Command definition or NULL   */
    UINT32 err );			/* Error code			*/


/************************************************************************
 *
 *                          shell_decode_token
 *  Description :
 *  -------------
 *
 *  Decode token
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed (never happens)
 *
 ************************************************************************/
bool
shell_decode_token(
    char	   *token,    /* Token to be decoded			*/
    UINT32	   *type,     /* Type of token				*/
    t_shell_option *decode ); /* Decoded value				*/


/************************************************************************
 *                          shell_decode_number
 ************************************************************************/
bool
shell_decode_number(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token );


/************************************************************************
 *
 *                          shell_disp_info
 *  Description :
 *  -------------
 *
 *  Display info on the specified item
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
UINT32
shell_disp_info(
    char *name );


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
   bool first );   /* TRUE -> Start application (only FALSE for GDB reentry) */


/************************************************************************
 *
 *                          shell_setup_default_cpu
 *  Description :
 *  -------------
 *
 *  Setup CP0 registers in context structure so that they contain
 *  values suiting an application.
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
    UINT32     epc );


/************************************************************************
 *
 *                          shell_get_line
 *  Description :
 *  -------------
 *
 *  Get command line with handling of special characters
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE -> Ctrl^C was pressed
 *
 ************************************************************************/
bool
shell_get_line(
    char       *line,	    /* Line buffer                                       */
    UINT32     maxchars,    /* Max number of chars not counting terminating '\0' */
    bool       first_line,  /* TRUE -> First line (ie not a \CR continuation)	 */
    bool       full,	    /* TRUE -> Handle command stack and autocompletion   */
    t_shell_line_buf *buf );/* Command stack (only used if full == TRUE)	 */


/************************************************************************
 *
 *                          shell_arch_cache_config
 *  Description :
 *  -------------
 *
 *  Get CPU specific cache options (used by shell cache command)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
shell_arch_cache_config(
    t_shell_cache_config_name_val **mapping,
    UINT32			  *config_count );


/************************************************************************
 *
 *                          shell_ide_display
 *  Description :
 *  -------------
 *
 *  Display IDE configuration
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
UINT32
shell_ide_display(void);


/************************************************************************
 *
 *                          shell_restore
 *  Description :
 *  -------------
 *  Restore shell setup to state before application was started.
 *  Does not restore cpu registers.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
void 
shell_restore( void );


/* appl_if.S functions */
void  shell_return( void );
void shell_api_vector_setup( UINT32 vector_addr, UINT32 shell_func_code);

#endif /* #ifndef _SHELL_H */


