
/************************************************************************
 *
 *  help.c
 *
 *  Help command for shell
 *
 *  help [-a | <command>]
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


/************************************************************************
 *  Include files
 ************************************************************************/

#include <shell_api.h>
#include <shell.h>
#include <sysdefs.h>
#include <string.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Indentations */

#define HELP_SYNTAX	12
#define HELP_INDENT	4
#define HELP_OPTION	14


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static t_cmd  **commands;
static UINT32 command_count;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static MON_FUNC(help);

static void
print_help(
    t_cmd  *cmd );


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          help
 ************************************************************************/
static MON_FUNC(help)
{
    t_cmd  *cmd;
    bool   ambivalent;
    bool   disp_all    = TRUE;
    bool   disp_secret = FALSE;
    UINT32 i;

    /* Get additional parameter, if any */

    if( argc > 2 )
        return SHELL_ERROR_SYNTAX;

    if( argc != 1 )
    {
        if( strcmp( argv[1], "-a" ) == 0 )
	{
	    disp_secret = TRUE;
	}
	else
	{
	    disp_all = FALSE;

            cmd = shell_lookup_cmd( argv[1], &ambivalent, NULL, 
				    commands, command_count );

	    if( cmd )
	    {
	        if( !ambivalent )    
		{
                    print_help( cmd );
		    SHELL_PUTC( '\n' );
		}
	        else
		    return SHELL_ERROR_AMBIVALENT;
            }
	    else
	    {
                if( *(argv[1]) == '-' )
		{
		    shell_error_data = argv[1];
		    return SHELL_ERROR_OPTION;
		}
		else
		    return SHELL_ERROR_NOT_FOUND;
	    }
        }
    }

    if( disp_all )
    {
        if(SHELL_PUTS( "\nAvailable commands :\n" )) 
	    return OK;

	if(SHELL_PUTS( "\n." )) return OK;
	if(SHELL_PUTS_INDENT( ". (repeat last command line)", HELP_SYNTAX )) 
	      return OK;
	      
	if(SHELL_PUTS( "\n;" )) return OK;
	if(SHELL_PUTS_INDENT( "; (command separator)", HELP_SYNTAX )) 
	      return OK;
	      
	if(SHELL_PUTS( "\n+" )) return OK;
	if(SHELL_PUTS_INDENT( "+ <repeat count> ;", HELP_SYNTAX )) 
	      return OK;
	      
	for( i=0; i<command_count; i++ )
	{
	  if( commands[i]->secret && !disp_secret )
	      continue;		 /* Secret command */
	   
	  if(SHELL_PUTC( '\n' )) 
	      return OK;
	  if(SHELL_PUTS( commands[i]->name )) 
	      return OK;
	  if(SHELL_PUTS_INDENT( commands[i]->syntax, HELP_SYNTAX )) 
	      return OK;
	}
        
        if(SHELL_PUTS( "\n\n" )) 
	    return OK;
    }

    return OK;
}

/* Shell command 'help' */

static t_cmd help_cmd =
{
    "help", 
    help, 
    "help [<command>]",

    "'help' with no parameter shows a list of all the available commands.\n"
    "To get more detailed help on a particular command, specify the\n"
    "command name as an argument to 'help'.\n"
    "\n"
    "When specifying a command as an argument to help, command completion\n"
    "will be performed if at least two characters of the command name have\n"
    "been specified.",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *                          print_help
 ************************************************************************/
static void
print_help(
    t_cmd  *cmd )
{
    UINT32 indent;
    UINT32 i;

    if(SHELL_PUTS( "\nNAME\n" )) return;
    if(SHELL_PUTS_INDENT( cmd->name, HELP_INDENT )) return;

    if(SHELL_PUTS( "\n\nSYNOPSIS\n" )) return;
    if(SHELL_PUTS_INDENT( cmd->syntax, HELP_INDENT )) return;

    if(SHELL_PUTS( "\n\nDESCRIPTION\n" )) return;
    if(SHELL_PUTS_INDENT( cmd->descr, HELP_INDENT )) return;

    if(SHELL_PUTS( "\n\nOPTIONS" )) return;
    
    for( i=0; i<cmd->option_count; i++ )
    {
	if(SHELL_PUTC( '\n' )) return;

	indent = HELP_INDENT;
        
	if(SHELL_PUTC_INDENT( '-', indent )) return;
	indent++;
	
        if(SHELL_PUTS_INDENT( cmd->options[i].option, indent )) return;
	indent += strlen( cmd->options[i].option );

        if(SHELL_PUTS_INDENT( cmd->options[i].descr, 
		   	      MAX( HELP_OPTION, indent + 1 ) )) return;
    }

    if(SHELL_PUTC( '\n' )) return;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

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
    UINT32 cmd_count )
{
    commands      = cmd;
    command_count = cmd_count;
    
    return &help_cmd;
}
