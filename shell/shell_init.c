
/************************************************************************
 *
 *  shell_init.c
 *
 *  Register help commands and start shell
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

#include <stdlib.h>
#include <shell_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <mips.h>
#include <string.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Maximum number of shell commands */
#define MAX_COMMANDS	50

#define R_t0  8
#define OPC_ADDIU       0x9
#define OPCODE_LI_T0(imm) ( (OPC_ADDIU << 26) | (R_t0<<16) | ((imm) & 0xFFFF) )

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Shell commands (array and count) */
static t_cmd  *(commands[MAX_COMMANDS]);
static UINT32 command_count = 0;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static int
compare(
   t_cmd **x,
   t_cmd **y );


/************************************************************************
 *  Implementation : Public functions
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
shell_setup(void)
{
    UINT32 code;

    /* Register commands except help */
    shell_arch();
    
    /* Register help command (must be the last one) */   
    shell_register_cmd( shell_help_init( commands, command_count + 1 ) );


    /* Sort the table */
    qsort( (char *)commands,
           (int)command_count,
	   (int)sizeof(t_cmd *),
	   compare );

    /* Start shell (never returns) */    
    shell( commands, command_count );
}


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
    t_cmd *cmd )			/* Command to be registered	*/
{
  if(cmd && (command_count < MAX_COMMANDS))
        commands[command_count++] = cmd;
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *  			compare
 ************************************************************************/
static int
compare(
   t_cmd **x,
   t_cmd **y )
{
   if( !(*x)->name )
       return 1;
   if( !(*y)->name )
       return -1;

   return strcmp( (*x)->name, (*y)->name );
}



