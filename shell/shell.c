
/***********************************************************************
 *
 *  shell.c
 *
 *  Implementation of command shell with command line history and
 *  command line editing. VT100 codes used.
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
#include <sysdefs.h>
#include <syserror.h>
#include <shell.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <mips.h>
#include <syscon_api.h>
#include <env_api.h>
#include <sys_api.h>
#include <excep_api.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* Maximum number of parsed tokens in a command */
#define MAX_ARGC           15

typedef struct
{
    char   *name;
    UINT32 base;
}
t_sh_prefix;

/*  States for state event machine used for receiving characters. */
typedef enum
{
    STATE_RCV_START,
    STATE_RCV_ESC,
    STATE_RCV_CSI,
    STATE_RCV_BRACKET,
    STATE_RCV_DONE
}
t_rcv_state;

/*  States for state event machine used to expand env. variables of
 *  command line.
 */
typedef enum
{
    STATE_EXP_NORMAL,
    STATE_EXP_SPECIAL,
    STATE_EXP_QUOTE_SINGLE,
    STATE_EXP_QUOTE_SINGLE_SPECIAL,
    STATE_EXP_QUOTE_DOUBLE,
    STATE_EXP_QUOTE_DOUBLE_SPECIAL,
    STATE_EXP_QUOTE_DOUBLE_ENV,
    STATE_EXP_ENV,
    STATE_EXP_DONE
}
t_expand_state;

/*  States for state event machine used tokenize command line. */
typedef enum
{
    STATE_TOKENIZE_SEARCH,
    STATE_TOKENIZE_SPECIAL,
    STATE_TOKENIZE_QUOTE_DOUBLE,
    STATE_TOKENIZE_QUOTE_SINGLE,
    STATE_TOKENIZE_DONE,
    STATE_TOKENIZE_WORD,
    STATE_TOKENIZE_QUOTE_DOUBLE_SPECIAL,
    STATE_TOKENIZE_QUOTE_SINGLE_SPECIAL,
    STATE_TOKENIZE_WORD_SPECIAL
}
t_tokenize_state;

/************************************************************************
 *  Public variables
 ************************************************************************/

char *shell_msg_ctrl_c  = "Press Ctrl-C to break";
char *shell_error_data  = NULL;
char *shell_error_hint  = NULL;
bool shell_dot		= FALSE;

/************************************************************************
 *  Static variables
 ************************************************************************/

static t_gdb_regs shell_restart_context;
static bool       display_freeze;

static char continue_msg[] = "Press any key "
			     "(Ctrl-C to break, Enter to singlestep)";

/* Stored parameters to shell() */
static t_cmd   **command_list;
static UINT32  command_count;

/* Number prefixes */
static t_sh_prefix sh_prefix[] =
{
    { "0x",  16 },
    { "0X",  16 },
    { "16/", 16 },
    { "10/", 10 },
    { "8/",   8 }
};
#define SHELL_PREFIX_NUMBER_COUNT    (sizeof(sh_prefix)/sizeof(t_sh_prefix))

/* Max line length after prompt */
static UINT32 max_line_len;

/* Shell error messages */
static char *(shell_err[]) = SHELL_ERR_MSG;
#define SHELL_ERROR_MSG_COUNT	(sizeof(shell_err)/sizeof(char *))

/* Array for error strings */
#define MAX_NUMBER_OF_ERROR_STRINGS 100
static UINT8* err_strings[MAX_NUMBER_OF_ERROR_STRINGS];

/* Line parsing */
static UINT32 argc;
static char   *(argv[MAX_ARGC]);
static char   arg_buf[MAX_ARGC][SHELL_MAX_TOKEN_LEN+1];

/* Display data */
static UINT32 linenum;
static bool   more = FALSE;
static UINT32 indent_prev;

/*  String containing value of environment variable "start", which 
 *  is automatically executed after a reset unless cancelled by user,
 *  or unless environment variable "startdelay" exists with value 0.
 */
static char *start;

/*  Flag for monitoring whether CTRL-C is pressed during command
 *  execution.
 */
static bool ctrl_c_flag;

/*  Flag for monitoring whether a '\n' has been issued after
 *  the command prompt.
 */
static bool newline_after_prompt;

/* Error message */
#define MAX_SHELL_ERR_MSG	400
static char shell_error_msg[MAX_SHELL_ERR_MSG];

/* Failing command */
static t_cmd *failing_cmd;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
receive( 
    char *ch );

static void
moveleft(
    UINT32 count );

static void
moveright(
    char    *line,
    UINT32  cursor,
    UINT32  count );

static void
print_prompt(
#define PROMPT_TARGET_TERMINAL	0
#define PROMPT_TARGET_DISPLAY	1
    UINT32 target,
#define PROMPT_TEXT_ENV		0
#define PROMPT_TEXT_CONT	1
    UINT32 text ); 

static bool
get_previous( 
    t_shell_line_buf *buf, 
    UINT32	     *current_from_buf, 
    char	     *line );

static bool
get_next( 
    t_shell_line_buf *buf, 
    UINT32	     *current_from_buf, 
    char	     *line );

static void 
copybuf(
    char   *dst,
    char   *src,
    UINT32 pos,
    UINT32 bufsize );

static bool
decode_option(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token );

static bool
decode_ip(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token );

static void
command_loop(
    t_shell_line_buf *buf );

static void
insert_char( 
    char   *line, 
    UINT32 *cursor,
    UINT32 *eol,
    char   ch );

static void
delete_char( 
    char   *line, 
    UINT32 *cursor, 
    UINT32 *eol );

static void
delete_from_cursor(
    UINT32 cursor, 
    UINT32 *eol);

static void
delete_line(
    UINT32 *cursor,
    UINT32 *eol );

static void
add2buf( 
    t_shell_line_buf *buf,
    char	     *line );

static UINT32
execute_line( 
    char *line,
    bool first );

static UINT32
execute_command( 
    t_cmd  **cmd );

static void
remove_space( 
    char *line );

static void
separate(
    char   *line,
    UINT32 *subcommand_count );

static bool
get_repeat(
    char   *line,
    UINT32 *loop_count );

static UINT32
expand_line( 
    char *line );

static UINT32
add_env(
    char   *env_name,
    UINT32 env_len,
    char   *expanded,
    UINT32 *len_expanded );

static UINT32
tokenize(
    char *line );

static INT32 
error_lookup( 
    t_sys_error_string *param );

static void
add2error(
    char *s );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell
 *  Description :
 *  -------------
 *  Implements the shell
 *
 *  Return values :
 *  ---------------
 *  None, function never returns
 *
 ************************************************************************/
void 
shell(
    t_cmd  **cmd_list,		/* Array of pointers to shell commands  */
    UINT32 cmd_count )		/* Number of shell commands		*/
{
    t_shell_line_buf		     buf;
    UINT32			     i;
    UINT32			     default_switch;
    bool			     ctrl_c;
    UINT32			     delay;
    char			     *startdelay;
    t_sys_error_lookup_registration  registration;
    UINT32			     old_ie;

    /* Store parameters */
    command_list  = cmd_list;
    command_count = cmd_count;

    /* register error lookup function */
    registration.prefix = SYSERROR_DOMAIN( ERROR_SHELL );
    registration.lookup = error_lookup;

    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    /* Print prompt on display */
    print_prompt( PROMPT_TARGET_DISPLAY, PROMPT_TEXT_ENV );
    printf( "\n" );
    DEBUG( "Shell Init\n" );

    /* Init */
    for(i=0; i<MAX_ARGC; i++)
        argv[i] = arg_buf[i];

    /* Display info */
    printf( "\n" );
    shell_disp_info( "boot" );

    /* Check default switch */
    SYSCON_read( SYSCON_BOARD_USE_DEFAULT_ID,
		 (void *)&default_switch,
		 sizeof(UINT32) );

    if( default_switch )
    {
        printf(
	"Environment default switch is in the 'on' position. The environment\n"
	"has been set to default values. If the switch is not set to 'off',\n"
	"the environment will be cleared at every reset.\n"
	"\n"
	"Please set the switch to the 'off' position or type Ctrl-C to continue.\n" );

        do
	{		  
            SYSCON_read( SYSCON_BOARD_USE_DEFAULT_ID,
		         (void *)&default_switch,
		         sizeof(UINT32) );

	    ctrl_c = GETCHAR_CTRLC( DEFAULT_PORT );
	}
        while( default_switch && (!ctrl_c) );

        printf("\n");
    }

    /* Initialise buffer */
    buf.first = BUFFER_NU;
    buf.free  = 0;

    /* Check for "start" command" */
    if( env_get( "start", &start, NULL, 0 ) && (*start != '\0') )
    {
	if (env_get("startdelay",&startdelay,NULL,0) && (*startdelay != '\0'))
	{
	    delay = (UINT32)strtoul( startdelay, &startdelay, 10 );
	    if( *startdelay != '\0' || delay > 0x100000 )
	    {
		delay = 2;
	    }
	    if (delay == 0) start = NULL;
	}
	else delay = 2;
    }
    else
        start = NULL;

    if (start)
    {
        printf( "Environment variable 'start' exists. After %d seconds\n"
		"it will be interpreted as a YAMON command and executed.\n"
		"Press Ctrl-C to bypass this.\n", delay );

        for (delay *= 10; delay--;)
	{
	    ctrl_c = GETCHAR_CTRLC( DEFAULT_PORT );
	    
	    if( ctrl_c )
	    {
		start = NULL;
		printf( "\n" );
	        break;
	    }
	    else
	        sys_wait_ms( 100 );
        }
    }

    /**** Save YAMON shell context ****/

    /* Store exception handler settings */
    EXCEP_store_handlers( EXCEP_HANDLERS_SHELL ); 

    old_ie = sys_disable_int();

    /* Store context */
    EXCEP_save_context(&shell_restart_context); 

    /* This is where also shell_reenter() returns to ! */

    /* Flush context to memory. Needed in case we reenter
     * shell in uncached mode (using shell_reenter())
     * after a cache error exception.
     */
    sys_dcache_flush_all();

    if(old_ie)
        sys_enable_int();

    command_loop( &buf );
}


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
shell_reenter( bool go_uncached )
{
    if ( (shell_restart_context.cp0_status & M_StatusEXL) == 0)
    {
        /* shell has not been started yet, so loop forever */
        while(1);
    }

    /* abort start command line should it still be active */
    start = NULL;

    /* Freeze display until operator intervention occurs */
    display_freeze = TRUE;

    /* After cacheerr exception shell is forced to run uncached */
    if ( go_uncached )
    {
	shell_restart_context.cp0_config &= ~M_ConfigK0;
	shell_restart_context.cp0_config |= K_CacheAttrU << S_ConfigK0;
    }

    printf( "\n" );

    /* Restore shell setup (incl. ESR settings) */
    shell_restore();
    
    /* re-initialize context and make a return to the bottom of shell() */
    EXCEP_exc_handler_ret_ss( &shell_restart_context );
}


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
shell_restore( void )
{
    /* Restore registered exception handlers */
    EXCEP_set_handlers( EXCEP_HANDLERS_SHELL );

    /* clean up ram vectors modified by application */
    EXCEP_install_exc_in_ram(sys_eicmode);

    /* Restart DMA */
    sys_dma_enable( TRUE );
}


/************************************************************************
 *                          shell_command_error
 ************************************************************************/
void
shell_command_error(
    t_cmd  *cmd,			/* Command definition or NULL   */
    UINT32 err )			/* Error code			*/
{
    int i;
    t_sys_error_string error_string;

    failing_cmd = cmd;

    /* lookup syserror */
    error_string.syserror = err;
    error_string.count    = MAX_NUMBER_OF_ERROR_STRINGS;
    error_string.strings  = err_strings;
    
    for (i=0; i<3; i++)
    {
        error_string.strings[i] = NULL;
    }
    
    SYSCON_read( SYSCON_ERROR_LOOKUP_ID,
                 &error_string,
                 sizeof( error_string ) );

    /* CTRL-C is not an error */
    if( err != SHELL_ERROR_CONTROL_C_DETECTED )
        printf( "Error : " );

    /* Check for user defined error message */
    if( error_string.strings[SYSCON_ERRORMSG_IDX] )
    {
        printf( "%s\n", error_string.strings[SYSCON_ERRORMSG_IDX] );
    }

    /* Check for user defined diagnose message */
    if( error_string.strings[SYSCON_DIAGMSG_IDX] )
    {
        printf( "Diag  : %s\n",
	        error_string.strings[SYSCON_DIAGMSG_IDX] );
    }

    /* Check for user defined hint message */
    if( error_string.strings[SYSCON_HINTMSG_IDX] )
    {
        printf( "Hint  : %s\n", 
	        error_string.strings[SYSCON_HINTMSG_IDX] );
    }
}


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
    t_shell_option *decode )  /* Decoded value				*/
{
    if( !decode_option(       decode, type, token) ) 
    if( !shell_decode_number( decode, type, token) )
    if( !decode_ip(           decode, type, token) )
    {
	/* String */
        decode->string = token;
        *type          = SHELL_TOKEN_STRING;
    }

    return TRUE;
}


/************************************************************************
 *                          shell_decode_number
 ************************************************************************/
bool
shell_decode_number(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token )
{
    UINT32 len, base;
    UINT32 len_temp;
    char   *endp;
    UINT32 i;

    /* Lets see if it is a number */
	  
    len  = 0;   /* In case of no radix prefix */
    base = SHELL_RADIX_DEFAULT;

    for( i=0; i < SHELL_PREFIX_NUMBER_COUNT; i++ )
    {
        len_temp = strlen( sh_prefix[i].name );

        if( strncmp( token, sh_prefix[i].name, len_temp ) == 0 )
	{   
	   base  = sh_prefix[i].base;
	   len   = len_temp;
	   break;
	}
    }

    errno = 0;

    if( sys_64bit )
    {
        decode->number64 = (UINT64)strtoull( &token[len], &endp, base );

        if( (*endp == '\0') && (errno == 0) )
        {
	    if( decode->number64 > MAXUINT(sizeof(UINT32)) )
	    {
                if( type )
                    *type = SHELL_TOKEN_NUMBER64;
	    }
	    else
	    {
	        decode->number = (UINT32)decode->number64;

                if( type )
                    *type = SHELL_TOKEN_NUMBER;
	    }

	    return TRUE;
        }
    }
    else
    {
        decode->number = (UINT64)strtoul( &token[len], &endp, base );

        if( (*endp == '\0') && (errno == 0) )
        {
            if( type )
                *type = SHELL_TOKEN_NUMBER;

	    return TRUE;
        }
    }

    return FALSE;
}


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
    UINT32 cmd_count )			/* Number of commands		*/
{
    t_cmd  *cmd, *cmd_found;
    UINT32 i;
    char   ch;

    cmd_found = NULL;

    if(ambivalent)
        *ambivalent = FALSE;

    /* Hack in order to handle case insensitivity of 
     * Microsoft SDB commands.
     */
    if( strlen( name ) == 1 )
    {
        ch    = *name;
        *name = tolower(*name);
    }

    /* Try to find a match */
    for( i=0; i < cmd_count; i++ )
    {
        cmd = cmd_list[i];

        if( strstr( cmd->name, name ) == cmd->name )
        {
            /* Found match */

            if( strcmp( cmd->name, name ) == 0 )
            {
                /* Exact match */
                if(len)
                    *len = strlen( cmd->name );

                cmd_found = cmd;
		break;
            }

            if( cmd_found )
            {
                /* Got one already */
                UINT32 t;

                /* Determine the common characters */
                for( t=0; cmd_found->name[t] == cmd->name[t]; t++ );

                if(len)
                    *len = MIN(t, *len);

                if(ambivalent)
		    *ambivalent = TRUE;
            }
            else
            {
	        if( strlen(name) > 1 ) /* Min 2 char for autocompletion */
		{
                    cmd_found = cmd;

                    if(len)
		        *len = strlen(cmd->name);
                }
            }
        } 
    }

    /* Hack in order to handle case insensitivity of 
     * Microsoft SDB commands.
     */
    if( strlen( name ) == 1 )
    {
        *name = ch;
    }

    return cmd_found;
}


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
    UINT32 *count )
{
    PUTCHAR( DEFAULT_PORT, '.' );

    if( ((++(*count) % 40) == 0) )
    {
        PUTCHAR( DEFAULT_PORT, '\n' );
    }

    return GETCHAR_CTRLC( DEFAULT_PORT );
}


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
    bool enable_more )
{
    more        = enable_more;
    linenum     = 0;
    indent_prev = 0;
}


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
    UINT32 indent )
{
    UINT32 count, len;
    UINT32 first_newline;
    bool   nl;
    char   ch;
    bool   rc;
    UINT32 linemax, linewidth;
    char *s;

    /* Check for "linewidth" environment variable" */
    if (env_get ("linewidth", &s, NULL, 0) && (*s != '\0'))
    {
	linewidth = (UINT32)strtoul (s, &s, 10);
	if (*s != '\0')
	    linewidth = MON_DEF_LINEWIDTH;
    }
    else 
	linewidth = MON_DEF_LINEWIDTH;
    /* Check for "linemax" environment variable" */
    if (env_get ("linemax", &s, NULL, 0) && (*s != '\0'))
    {
	linemax = (UINT32)strtoul (s, &s, 10);
	if (*s != '\0')
	    linemax = MON_DEF_LINEMAX;
    }
    else 
	linemax = MON_DEF_LINEMAX;
    if (linemax)
	linemax -= 1;

    if( linewidth <= indent )		/* This is an error		*/
        return TRUE;

    while( *string != '\0' )
    {
        if( linemax && (linenum == linemax) )
	{
	   /* Print 'press any key...' message */
	   PUTS( DEFAULT_PORT, continue_msg );

	   while( !GETCHAR( DEFAULT_PORT, &ch ) );
	   
	   /* Remove 'press any key...' message */
	   len = strlen( continue_msg );
	   moveleft( len );
	   for(count=0; count<len; count++)
	   {
 	       PUTCHAR( DEFAULT_PORT, ' ' );
	   }
	   moveleft( len );

	   if( ch == CTRL_C )
	   {
	       linenum     = 1; /* Keep last line */
	       ctrl_c_flag = TRUE;
	       return TRUE;
           }
	   else
	   {
	       linenum = 
	           ( (ch == CR) || (ch == LF) ) ?
		       linemax - 1 :
		       1;   /* Keep last line */
	       indent_prev = 0;
	   }
	}

        /* Indent */
	if( indent )
	{
            if (indent < indent_prev)
                indent = indent_prev;
	    count        = indent - indent_prev;
	    indent_prev += count;
	}
	else	  
	    count = 0;

        while(count--)
        {
            PUTCHAR( DEFAULT_PORT, ' ' );
        }

        first_newline = strcspn( string, "\n" );

	count = ( first_newline + indent <= linewidth ) ?
		     first_newline + 1 : 
		     linewidth - indent;

	while( count-- && (*string != '\0') )
	{
	    nl = (*string == '\n');
	    PUTCHAR( DEFAULT_PORT, *(string++) );
	    indent_prev++;
	    
	    if(nl) 
	    {
	        indent_prev = 0;
	        if( more ) 
		    linenum++;
	    }
        }

	if( (*string != '\0') && (!nl) )
	{
            PUTCHAR( DEFAULT_PORT, '\n' );
	    indent_prev = 0;
	    if( more )
	        linenum++;
	}
    }

    rc = GETCHAR_CTRLC( DEFAULT_PORT );

    if( rc )
        ctrl_c_flag = TRUE;

    return rc;
}


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
    UINT32 indent )
{
    char s[2];
    
    s[0] = ch;
    s[1] = '\0';

    return shell_puts( s, indent );
}


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
    t_shell_line_buf *buf ) /* Command stack (only used if full == TRUE)	 */
{
    UINT32  cursor, eol;
    char    ch;
    UINT32  slash_count;
    bool    line_shift, line_done;
    UINT32  len;
    t_cmd   *cmd;
    UINT32  current_from_buf;

    if( start )
    {
        /* Use $start as the first command line */
        strcpy( line, "$start" );
	return TRUE;
    }

    *line            = '\0';
    cursor	     = 0;
    eol		     = 0;
    line_done	     = FALSE;
    line_shift	     = FALSE;
    
    if( full && !buf )   /* Should not happen */
        return FALSE;

    if( full )
        current_from_buf = buf->free;

    while( !line_done && !line_shift )
    {
        receive( &ch );

	switch( ch )
	{
          case CR :
          case LF :
	    /*  End line unless last characters were an
	     *  odd number of '\' chars, in which 
	     *  case input continues on new line.
	     */

	    for( slash_count = 0;
		   (eol - slash_count > 0) &&
		   (line[eol - slash_count - 1] == '\\');
		 slash_count++ )
	    {
	         ;
	    }
		
	    if( (slash_count % 2) == 1 )
	        line_shift = TRUE;
	    else
	        line_done = TRUE;
	    break;

          case CTRL_C :
	    /* Cancel line */
	    return FALSE;

          case CTRL_A :
	    /* Move cursor to the far left */
	    moveleft( cursor );
	    cursor = 0;
	    break;

          case CTRL_E :
	    /* Move cursor to the far right */
	    moveright( line, cursor, eol - cursor );
	    cursor = eol;
	    break;

          case CTRL_B :     
	    /* ^B or left arrow : Move cursor left */
            if( cursor > 0 ) 
            { 
                moveleft(1);
                cursor--;
            }
            break;
          
          case CTRL_F : 
	    /* ^F or right arrow : Move cursor right */
            if( cursor < eol )
            {
                moveright(line, cursor, 1);
                cursor++;
            }
            break;

          case CTRL_D :
	    /* Delete character after cursor */
            if( cursor < eol )
            {
                delete_char( line, &cursor, &eol );
            }
            break;

          case CTRL_H :     /* Same as DEL */
          case DEL    :
	    /* Delete character at cursor */
            if( cursor > 0 )
            {
                moveleft(1);
                cursor--;
                delete_char(line, &cursor, &eol );
            }
            break;

          case CTRL_K :
	    /* Delete everything after cursor */
	    delete_from_cursor(cursor, &eol);
	    line[eol] = '\0';
	    break;

          case CTRL_U :
	    /* Delete line */
	    delete_line( &cursor, &eol );
	    line[eol] = '\0';
	    break;
	    
          case CTRL_P :
	    /* Previous command in command stack (first line only) */

            if( full && first_line && 
		get_previous( buf, &current_from_buf, line ) )
            {
	        /* Delete line */
	        delete_line( &cursor, &eol );

		/* Update settings and print line */
		cursor = eol = strlen( line );
		printf( "%s",line );
            }

            break;      

          case CTRL_N :
	    /* Next command in command stack (first line only) */

	    if( full && first_line && 
		get_next( buf, &current_from_buf, line ) )
	    {
	        /* Delete line */
	        delete_line( &cursor, &eol );

		/* Update settings and print line */
		cursor = eol = strlen( line );
		printf( "%s",line );
	    }

	    break;

          case TAB :

            /* Command line completion (first line only) */
	    if( full && first_line )
	    {
                cmd = shell_lookup_cmd( line, NULL, &len,
					command_list, command_count );

                if( cmd )
                {
		    /* Delete old line */
	            delete_line( &cursor, &eol );
		
		    /* Insert new line */    
                    strncpy( line, cmd->name, len );
                        line[len] = '\0';
		    cursor = len;
		    eol    = len;

		    /* Print line */
		    printf( "%s",line );
                }
	    }
            break;

          default :
            if( isprint(ch) && (eol < MIN(maxchars, max_line_len )) )
                insert_char( line, &cursor, &eol, ch );
            break;      
	}
    }

    if( line_shift )
    {
        print_prompt( PROMPT_TARGET_TERMINAL, PROMPT_TEXT_CONT );

	eol--;  /* Remove '\' */
	return shell_get_line( &line[eol], maxchars - eol, FALSE, full, buf );
    }

    return TRUE;
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          print_prompt
 ************************************************************************/
static void
print_prompt(
    UINT32 target,
    UINT32 text )
{
    char *s;
    bool  b;

    if( text == PROMPT_TEXT_ENV )
    {
        if( !env_get( "prompt", &s, NULL, 0 ) )
            s = sys_default_prompt;
    }
    else
    {
        s = "\n?";
    }

    if( target == PROMPT_TARGET_DISPLAY )
    {
        /* Freeze display for the first prompt after shell is reentered */
        if (display_freeze)
	{
            display_freeze = FALSE;

            /* Acknowledge NMI (doesn't harm if nmi is not active) */
            SYSCON_write(SYSCON_BOARD_NMI_ACK_ID, &b, sizeof(b));
            return;
        }

        if (!strcmp(s, sys_default_prompt))
        {
            /* Prompt is default - show default display */
	    s = sys_default_display;
	}

        DISP_STR( s );
    }
    else
    {
        printf( "%s",s );
        printf( (text == PROMPT_TEXT_ENV) ? "> " : " " );

	max_line_len = SHELL_MAX_LINE_LEN - strlen(s) - 2;
    }
}


/************************************************************************
 *                          receive
 ************************************************************************/
static void
receive( 
    char *ch )
{
    t_rcv_state state = STATE_RCV_START;

    while( state != STATE_RCV_DONE )
    {
        while( !GETCHAR( DEFAULT_PORT, ch ) );

        switch( state )
        {
          case STATE_RCV_START :
            if     ( *ch == ESC )		   state = STATE_RCV_ESC;
            else if( *(unsigned char *)ch == CSI ) state = STATE_RCV_CSI;
            else				   state = STATE_RCV_DONE;
            break;
          case STATE_RCV_ESC :
            if( *ch == '[' ) state = STATE_RCV_BRACKET;
            else             state = STATE_RCV_START;
            break;
          case STATE_RCV_CSI :
          case STATE_RCV_BRACKET :
            state = STATE_RCV_DONE;
            switch( *ch )
            {
              case 'A':
                *ch = CTRL_P; break;
              case 'B':
                *ch = CTRL_N; break;
              case 'C':
                *ch = CTRL_F; break;
              case 'D':
                *ch = CTRL_B; break;
              default :
                state = STATE_RCV_START;
            }
            break;
          default :
            break;
       }
    }
}


/************************************************************************
 *                          moveleft
 ************************************************************************/
static void
moveleft(
    UINT32 count )
{
    while( count-- )
    {
        PUTCHAR( DEFAULT_PORT, '\b' );
    }       
}


/************************************************************************
 *                          moveright
 ************************************************************************/
static void
moveright(
    char    *line,
    UINT32  cursor,
    UINT32  count )
{
    while( count-- )
    {
        PUTCHAR( DEFAULT_PORT, line[cursor++] );
    }
}


/************************************************************************
 *                          get_previous
 ************************************************************************/
static bool
get_previous( 
    t_shell_line_buf *buf, 
    UINT32	     *current_from_buf, 
    char	     *line )
{
    if( buf->first == BUFFER_NU )
        return FALSE;

    if( *current_from_buf == buf->first )
        return FALSE;

    /* Move backward */

    DECWRAP(*current_from_buf, BUFFERSIZE);    
    do
    {
        DECWRAP(*current_from_buf, BUFFERSIZE);    
    }
    while( (buf->data[*current_from_buf] != '\0') &&
           (*current_from_buf != buf->first) );

    if( *current_from_buf != buf->first )
    {
        /* One too far, so move increase by 1 */
        INCWRAP(*current_from_buf, BUFFERSIZE);
    }

    copybuf( line, buf->data, *current_from_buf, BUFFERSIZE );

    return TRUE;
}


/************************************************************************
 *                          get_next
 ************************************************************************/
static bool
get_next( 
    t_shell_line_buf *buf, 
    UINT32	     *current_from_buf, 
    char	     *line )
{
    if( buf->first == BUFFER_NU )
        return FALSE;

    if( *current_from_buf == buf->free )
        return FALSE;

    /* Move forward */
    do
    {
        INCWRAP(*current_from_buf, BUFFERSIZE);
    }
    while( buf->data[*current_from_buf] != '\0' );
    INCWRAP(*current_from_buf, BUFFERSIZE);

    if( *current_from_buf == buf->free )
    {
        line[0] = '\0';
    }
    else
    {   
        /* Do the copy */
        copybuf( line, buf->data, *current_from_buf, BUFFERSIZE );
    }

    return TRUE;
}


/************************************************************************
 *                          copy_buf
 ************************************************************************/
static void 
copybuf(
    char   *dst,
    char   *src,
    UINT32 pos,
    UINT32 bufsize )
{
    do
    {
        *dst = src[pos];
        INCWRAP(pos, bufsize );
    }
    while( *(dst++) != '\0' );
}


/************************************************************************
 *                          decode_option
 ************************************************************************/
static bool
decode_option(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token )
{
    if( *token == '-' )
    {
        decode->option = &token[1];
	*type	       = SHELL_TOKEN_OPTION;
	return TRUE;
    }
    else
       return FALSE;    
}


/************************************************************************
 *                          decode_ip
 ************************************************************************/
static bool
decode_ip(
    t_shell_option *decode,
    UINT32	   *type,
    char           *token )
{
    if( env_ip_s2num( token, (void *)&decode->ip, sizeof(decode->ip)) )
    {
	*type      = SHELL_TOKEN_IP;
	return TRUE;
    }
    else
        return FALSE;
}


/************************************************************************
 *                          insert_char
 ************************************************************************/
static void
insert_char( 
    char   *line, 
    UINT32 *cursor,
    UINT32 *eol,
    char   ch )
{
    UINT32 i;
  
    /* Move characters after cursor and insert new character */
    for( i = *eol; i > *cursor; i-- )
        line[i] = line[i-1];
    line[i] = ch;

    /* Increase line length and add new termination */
    (*eol)++;
    line[*eol] = '\0';    

    /* Print line from cursor  */
    printf("%s", &line[*cursor] );

    /* Increase cursor */
    (*cursor)++;

    /* Move cursor back */
    moveleft(*eol - *cursor);
}


/************************************************************************
 *                          delete_char
 ************************************************************************/
static void
delete_char( 
    char   *line, 
    UINT32 *cursor, 
    UINT32 *eol )
{
    UINT32 i;
  
    /* Move characters after cursor and insert new character */
    for( i = *cursor; i < *eol; i++ )
        line[i] = line[i+1];

    /* Print line from cursor, add space */
    printf( "%s ", &line[*cursor] );

    /* Move cursor back */
    moveleft( *eol - *cursor );

    /* Decrease line length */
    (*eol)--;
}


/************************************************************************
 *                          delete_from_cursor
 ************************************************************************/
static void
delete_from_cursor(
    UINT32 cursor, 
    UINT32 *eol)
{
    char   spaces[SHELL_MAX_LINE_LEN + 1];
    UINT32 i;

    /* Replace characters to the right of cursor with spaces */    
    for(i=0; i < (*eol - cursor); i++)
        spaces[i] = ' ';
    spaces[i] = '\0';
    printf( spaces );

    /* Move cursor back */
    moveleft( *eol - cursor );
    
    /* Move eol to cursor location */
    *eol = cursor;
}


/************************************************************************
 *                          delete_line
 ************************************************************************/
static void
delete_line(
    UINT32 *cursor,
    UINT32 *eol )
{
    moveleft( *cursor );
    *cursor = 0;
    delete_from_cursor(*cursor, eol );
}


/************************************************************************
 *                          add2buf
 ************************************************************************/
static void
add2buf( 
    t_shell_line_buf *buf,
    char	     *line )
{
    UINT32 current_from_buf;
    char   previous_line[SHELL_MAX_COMMAND_LEN + 1];
    UINT32 len, i;

    current_from_buf = buf->free;

    /* Check for identical command on stack */
    if( get_previous( buf, &current_from_buf, previous_line ) &&
        strcmp( line, previous_line ) == 0 )
    {
        return;
    }

    /* Copy line to buffer */
    for( i=0, len = strlen(line); i<=len; i++ )
    {
        /* Before inserting character, we check if we are about
         * to overwrite an old line.
         */
        if( buf->free == buf->first )
        {
            /* We are about to overwrite an old line,
	     * so obsolete the old line
	     */
            while(buf->data[buf->first] != '\0')
            {
                INCWRAP(buf->first, BUFFERSIZE);
            }
            INCWRAP(buf->first, BUFFERSIZE);
        }

        /* Now we are ready to insert the character */
        buf->data[buf->free] = line[i];
        INCWRAP(buf->free, BUFFERSIZE);

        /* After moving the free pointer forward, we need to check
	 * again if 'free' has caught up with 'first'.
	 */
        if( buf->free == buf->first )
        {
            /* free did catch up with first, so obsolete the old line. */
            while(buf->data[buf->first] != '\0')
            {
                INCWRAP(buf->first, BUFFERSIZE);
            }
            INCWRAP(buf->first, BUFFERSIZE);
        }
    }

    /* Buffer is no longer unused */
    if( buf->first == BUFFER_NU ) 
        buf->first = 0;
}


/************************************************************************
 *                          command_loop
 ************************************************************************/
static void
command_loop(
    t_shell_line_buf *buf )
{
    char   line[SHELL_MAX_COMMAND_LEN + 1];
    UINT32 current_from_buf;
    bool   execute;

    while( TRUE )
    {
        print_prompt( PROMPT_TARGET_DISPLAY, PROMPT_TEXT_ENV );

	if( !start )
            print_prompt( PROMPT_TARGET_TERMINAL, PROMPT_TEXT_ENV );

        if( !shell_get_line( line, SHELL_MAX_COMMAND_LEN, TRUE, TRUE, buf ) )
	{
	    printf( "\n" );
	}
	else
	{
	    /* Remove excess spaces */
	    remove_space( line );

	    if( strlen(line) == 0 )
	        printf( "\n" );
	    else
	    {
	        execute = TRUE;

	        if( strcmp( line, "." ) == 0 )
		{
		    shell_dot = TRUE;

		    current_from_buf = buf->free;

		    if(!get_previous( buf, &current_from_buf, line ))
		    {
		        execute = FALSE;
		        printf( "\n" );
		    }
		}
		else
		{
		    shell_dot = FALSE;

	            /* Add line to stack (only "single" lines) */
		    if( !start && (strlen( line ) <= max_line_len) )
	                add2buf( buf, line );
		}

		if( execute )
		{    
		    /*  Recursively split line in ";" separated
		     *  commands and expand environment variables.
		     *  Execute command(s).
		     */
		    newline_after_prompt = FALSE;
		    execute_line( line, TRUE );
		}
            }
        }

	start = NULL;
    }
}


/************************************************************************
 *                          remove_space
 ************************************************************************/
static void
remove_space( 
    char *line )
{
    char   clean[SHELL_MAX_COMMAND_LEN + 1];
    UINT32 len = 0;
    bool   skip;
    bool   found_char   = FALSE;
    char   *start       = line;
    bool   quote_double = FALSE;
    bool   quote_single = FALSE;
    bool   special	= FALSE;

    while( line[0] != '\0' )
    {
        skip = FALSE;

        /*  Remove excess whitespace, unless within single or
	 *  double quotes.
	 */
        if( line[0] == ' ' )
	{
	    if( !found_char        ||  /* Leading spaces  */
	        (line[1] == ' ')   ||  /* Multiple spaces */
		(line[1] == '\0') )    /* Trailing spaces */
            {
	        /* Keep spaces within quotes */
	        if( !quote_double && !quote_single )
		{
	            skip = TRUE;
		}
	    }
        }
	else if( line[0] == '\\' )
	{
	    special = !special;
	}
	else if( (line[0] == '\'') && !special )
	{
	    if( !quote_double )
	    {
	        quote_single = !quote_single;
	    }
	}
	else if( (line[0] == '"') && !special )
	{
	    if( !quote_single )
	    {
	        quote_double = !quote_double;
	    }
	}
	else
	    special = FALSE;

	if( !skip )
	{
	    clean[len++] = line[0];
	    found_char   = TRUE;
        }

	line++;
    }

    clean[len] = '\0';
    strcpy( start, clean );
}


/************************************************************************
 *                          separate
 ************************************************************************/
static void
separate(
    char   *line,
    UINT32 *subcommand_count )
{
    bool   quote_double = FALSE;
    bool   quote_single = FALSE;
    bool   special	= FALSE;

    *subcommand_count = 1;

    /*  Find ';' not within single or double quotes and substitute
     *  with '\0'
     */

    while( line[0] != '\0' )
    {
        if( (*line == ';') && !quote_double && !quote_single )
	{
            *line = '\0';
	    (*subcommand_count)++;
        }
	else if( *line == '\\' )
	{
	    special = !special;
	}
	else if( (*line == '\'') && !special )
	{
	    if( !quote_double )
	    {
	        quote_single = !quote_single;
	    }
	}
	else if( (*line == '"') && !special )
	{
	    if( !quote_single )
	    {
	        quote_double = !quote_double;
	    }
	}
	else
	    special = FALSE;

	line++;
    }
}


/************************************************************************
 *                          get_repeat
 ************************************************************************/
static bool
get_repeat(
    char   *line,
    UINT32 *loop_count )
{
    t_shell_option decode;
    UINT32	   len;

    if( *line == '+' )
    {
        line++;

	/* Remove trailing spaces */
	len = strlen( line );
	
	while( len && line[len-1] == ' ' )
	{
	    line[len-1] = '\0';
	    len--;
	}

	/* Decode number */
        if( shell_decode_number(
	            &decode, 
		    NULL,
	            line ) )
	{
	    *loop_count = decode.number;
	    return TRUE;
	}
    }

    return FALSE;
}


/************************************************************************
 *                          expand_line
 ************************************************************************/
static UINT32
expand_line( 
    char *line )
{
    char            env_name[SHELL_MAX_ENV_LEN + 2];
    char            expanded[SHELL_MAX_COMMAND_LEN + 1];
    UINT32          len, env_len;
    t_expand_state  state;
    char	    *ch;
    UINT32	    rc;
		  
    state   = STATE_EXP_NORMAL;
    ch      = line;
    len	    = 0;
    env_len = 0;
    rc	    = OK;

    do
    {
        switch( state )
	{
	  case STATE_EXP_NORMAL :
	    switch( *ch )
	    {
	      case '\0' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_DONE;
		break;
	      case '$'  :
	        env_len = 0;
	        state = STATE_EXP_ENV;
		break;
	      case '\\' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_SPECIAL;
		break;
	      case '\'' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_QUOTE_SINGLE;
		break;
	      case '"' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_QUOTE_DOUBLE;
		break;
	      default   :
	        expanded[len++] = *ch;
		state = STATE_EXP_NORMAL;
		break;
	    }
	    break;
	  case STATE_EXP_ENV :
	    switch( *ch )
	    {
	      case '\0' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;
		state = STATE_EXP_DONE;
		break;
	      case '{' :
		state = STATE_EXP_ENV;
		break;
	      case '}' :
		rc = add_env( env_name, env_len, expanded, &len );
		state = STATE_EXP_NORMAL;
		break;
	      case '$'  :
		rc = add_env( env_name, env_len, expanded, &len );
		env_len = 0;
		state = STATE_EXP_ENV;
		break;
	      case '\\' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;
		state = STATE_EXP_SPECIAL;
		break;
	      case '\'' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;		
		state = STATE_EXP_QUOTE_SINGLE;
		break;
	      case '"' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;		
		state = STATE_EXP_QUOTE_DOUBLE;
		break;
	      default   :
		if (isalnum(*ch) || *ch == '_') {
		    env_name[env_len++] = *ch;
		    state = STATE_EXP_ENV;
		}
		else {
		    rc = add_env( env_name, env_len, expanded, &len );
		    expanded[len++] = *ch;		
		    state = STATE_EXP_NORMAL;
		}
		break;
	    }
	    break;
	  case STATE_EXP_SPECIAL :
	    switch( *ch )
	    {
	      case '\0' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_DONE;
		break;
	      case '$'  :
	      case '\\' :
	      case '\'' :
	      case '"'  :
	      case ' '  :
	      default   :
	        expanded[len++] = *ch;
		state = STATE_EXP_NORMAL;
		break;
	    }
	    break;
	  case STATE_EXP_QUOTE_SINGLE :
	    switch( *ch )
	    {
	      case '\0' :
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '\\' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_QUOTE_SINGLE_SPECIAL;
		break;
	      case '\'' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_NORMAL;
		break;
	      case '"' : 
	      case '$' :
	      case ' ' :
	      default  :
	        expanded[len++] = *ch;
		state = STATE_EXP_QUOTE_SINGLE;
		break;
	    }
	    break;
	  case STATE_EXP_QUOTE_DOUBLE :
	    switch( *ch )
	    {
	      case '\0' :
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '$'  :
	        env_len = 0;
	        state = STATE_EXP_QUOTE_DOUBLE_ENV;
		break;
	      case '\\' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_QUOTE_DOUBLE_SPECIAL;
		break;
	      case '"' :
	        expanded[len++] = *ch;
	        state = STATE_EXP_NORMAL;
		break;
	      case '\'' :
	      case ' '  :
	      default   :
	        expanded[len++] = *ch;
		state = STATE_EXP_QUOTE_DOUBLE;
		break;
	    }
	    break;
	  case STATE_EXP_QUOTE_SINGLE_SPECIAL :
	    switch( *ch )
	    {
	      case '\0' :
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '$' :
	      case '\\':
	      case '\'':
	      case '"' :
	      case ' ' :
	      default  :
	        expanded[len++] = *ch;
		state = STATE_EXP_QUOTE_SINGLE;
		break;
	    }
	    break;
	  case STATE_EXP_QUOTE_DOUBLE_SPECIAL :
	    switch( *ch )
	    {
	      case '\0' :
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '$' :
	      case '\\':
	      case '\'':
	      case '"' :
	      case ' ' :
	      default  :
	        expanded[len++] = *ch;
		state = STATE_EXP_QUOTE_DOUBLE;
		break;
	    }
	    break;
	  case STATE_EXP_QUOTE_DOUBLE_ENV :
	    switch( *ch )
	    {
	      case '\0' :
		rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '{' :
		state = STATE_EXP_QUOTE_DOUBLE_ENV;
		break;
	      case '}' :
		rc = add_env( env_name, env_len, expanded, &len );
		state = STATE_EXP_QUOTE_DOUBLE;
		break;
	      case '$'  :
		rc = add_env( env_name, env_len, expanded, &len );
		env_len = 0;
		state = STATE_EXP_QUOTE_DOUBLE_ENV;
		break;
	      case '\\' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;
		state = STATE_EXP_QUOTE_DOUBLE_SPECIAL;
		break;
	      case '"' :
		rc = add_env( env_name, env_len, expanded, &len );
		expanded[len++] = *ch;		
		state = STATE_EXP_NORMAL;
		break;
	      default   :
		if (isalnum(*ch) || *ch == '_') {
		    env_name[env_len++] = *ch;
		    state = STATE_EXP_QUOTE_DOUBLE_ENV;
		}
		else {
		    rc = add_env( env_name, env_len, expanded, &len );
		    expanded[len++] = *ch;		
		    state = STATE_EXP_QUOTE_DOUBLE;
		}
		break;
	    }
	    break;
	  default : /* Should not happen */
	    rc = SHELL_ERROR_STRUCTURE;
	    break;
	}

	if( (len == SHELL_MAX_COMMAND_LEN + 1) && (state != STATE_EXP_DONE) )
	    rc = SHELL_ERROR_PARSE_LONG_LINE;

	if( env_len == SHELL_MAX_ENV_LEN + 1 )
	    rc = SHELL_ERROR_PARSE_LONG_ENV;

	ch++;
    }
    while( (rc == OK) && (state != STATE_EXP_DONE) );
    
    if( rc != OK )
        return rc;
    else
    {
        strcpy( line, expanded );
        return OK;
    }
}


/************************************************************************
 *                          add_env
 ************************************************************************/
static UINT32
add_env(
    char   *env_name,
    UINT32 env_len,
    char   *expanded,
    UINT32 *len_expanded )
{
    char   *env_val;

    env_name[env_len]       = '\0';
    expanded[*len_expanded] = '\0';

    /* Lookup env variable */
    if( env_get( env_name, &env_val, NULL, 0 ) )
    {
        *len_expanded += strlen(env_val);

        if( *len_expanded > SHELL_MAX_COMMAND_LEN )
            return SHELL_ERROR_PARSE_LONG_LINE;
	else
	{
            strcat( expanded, env_val );
	    return OK;
	}
    }
    else
        return SHELL_ERROR_PARSE_UNKNOWN_ENV;
}


/************************************************************************
 *                          tokenize
 ************************************************************************/
static UINT32
tokenize( 
    char *line )
{
    t_tokenize_state  state;
    char	      *s;
    UINT32	      len;
    UINT32	      rc;

    state = STATE_TOKENIZE_SEARCH;
    argc  = 0;
    len   = 0;
    s     = argv[argc];
    rc	  = OK;

    do
    {
        switch( state )
	{
	  case STATE_TOKENIZE_SEARCH :
	    switch( *line )
	    {
	      case '\\' :
	        state = STATE_TOKENIZE_SPECIAL;
		break;
	      case '\'' :
    	        state = STATE_TOKENIZE_QUOTE_SINGLE;
		break;
	      case '"'  :
    	        state = STATE_TOKENIZE_QUOTE_DOUBLE;
		break;
	      case ' ' :
	        state = STATE_TOKENIZE_SEARCH;
		break;
	      case '\0' :
	        state = STATE_TOKENIZE_DONE;
		break;
	      default :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_WORD;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_SPECIAL :
	    switch( *line )
	    {
	      case '\0' :
	        state = STATE_TOKENIZE_DONE;
		break;
	      case '\\' :
	      case '\'' :
	      case '"'  :
	      default   :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_WORD;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_QUOTE_DOUBLE :
	    switch( *line )
	    {
	      case '\\' :
	        state = STATE_TOKENIZE_QUOTE_DOUBLE_SPECIAL;
		break;
	      case '"'  :
	        state = STATE_TOKENIZE_WORD;
		break;
	      case '\0' :  /* Should not happen */
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case ' ' :
	      case '\'' :
	      default  :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_QUOTE_DOUBLE;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_QUOTE_SINGLE :
	    switch( *line )
	    {
	      case '\\' :
	        state = STATE_TOKENIZE_QUOTE_SINGLE_SPECIAL;
		break;
	      case '\''  :
	        state = STATE_TOKENIZE_WORD;
		break;
	      case '\0' :  /* Should not happen */
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case ' ' :
	      case '"' :
	      default  :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_QUOTE_SINGLE;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_QUOTE_DOUBLE_SPECIAL :
	    switch( *line )
	    {
	      case '\0' :  /* Should not happen */
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '\\' :
	      case '\'' :
	      case '"'  :
	      default   :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_QUOTE_DOUBLE;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_QUOTE_SINGLE_SPECIAL :
	    switch( *line )
	    {
	      case '\0' :  /* Should not happen */
	        rc = SHELL_ERROR_PARSE_MISSING_QUOTE;
		break;
	      case '\\' :
	      case '\'' :
	      case '"'  :
	      default   :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_QUOTE_SINGLE;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_WORD :
	    switch( *line )
	    {
	      case '\\' :
	        state = STATE_TOKENIZE_WORD_SPECIAL;
		break;
	      case '\'' :
	        state = STATE_TOKENIZE_QUOTE_SINGLE;
		break;
	      case '"'  :
	        state = STATE_TOKENIZE_QUOTE_DOUBLE;
		break;
	      case '\0' :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else
		{
	            *s = *line;
		    argc++;
	            state = STATE_TOKENIZE_DONE;
		}
		break;
	      case ' ' :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else
		{
	            *s  = '\0';
		    argc++;
		    s     = argv[argc];
		    len   = 0;
		    state = STATE_TOKENIZE_SEARCH;
		}
		break;
	      default :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_WORD;
		}
		break;
	    }
	    break;
	  case STATE_TOKENIZE_WORD_SPECIAL :
	    switch( *line )
	    {
	      case '\0' :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else
		{
	            *s = *line;
		    argc++;
	            state = STATE_TOKENIZE_DONE;
		}
		break;
	      case '\\' :
	      case '\'' :
	      case '"'  :
	      default   :
	        if( argc == MAX_ARGC )
		    rc = SHELL_ERROR_PARSE_ARGCOUNT;
		else if( len == SHELL_MAX_TOKEN_LEN )
		    rc = SHELL_ERROR_PARSE_LONG_TOKEN;
		else
		{	      
	            *s = *line;
		    s++;
		    len++;
	            state = STATE_TOKENIZE_WORD;
		}
		break;
	    }
	    break;
	  default : /* Should not happen */
	    rc = SHELL_ERROR_STRUCTURE;
	    break;
	}

	line++;
    }
    while( (rc == OK) && (state != STATE_TOKENIZE_DONE) );

    return rc;
}


/************************************************************************
 *                          execute_command
 ************************************************************************/
static UINT32
execute_command( 
    t_cmd  **cmd )
{
    bool   ambivalent;
    UINT32 rc;
    
    if( argc != 0 )
    {
        *cmd = shell_lookup_cmd( argv[0], &ambivalent, NULL,
				 command_list, command_count );

        if( !(*cmd) )
        {
            shell_error_data = argv[0];
            return SHELL_ERROR_COMMAND_NOT_FOUND;
        }

        if( ambivalent )
	{
            shell_error_data = argv[0];
	    return SHELL_ERROR_AMBIVALENT;
        }

        SHELL_ENABLE_MORE;

        if( !newline_after_prompt )
	{
	    SHELL_PUTC( '\n' );
	    newline_after_prompt = TRUE;
	}

        if( (strlen((*cmd)->name) == 1) && (*(*cmd)->name != '.') )
        {
            /* Notify that this is a special Microsoft
	     * SDB command.
	     */
            SHELL_PUTS( "Microsoft SDB Command\n" );
        }

        /* We detect whether ctrl-c is typed during command execution */
        ctrl_c_flag = FALSE;
 
        /* Execute command */
        rc = (*cmd)->func( argc, &argv[0] );

        SHELL_DISABLE_MORE;

        if( rc != OK )
	    return rc;
    }
    else
        ctrl_c_flag = FALSE;

    /*  Test for CTRL-C in case command did not call shell_putc/s. */
    if( !ctrl_c_flag )
        ctrl_c_flag = GETCHAR_CTRLC( DEFAULT_PORT );

    return ctrl_c_flag ? SHELL_ERROR_CONTROL_C_DETECTED : OK;
}


/************************************************************************
 *                          execute_line
 ************************************************************************/
static UINT32
execute_line( 
    char *line,		/* Command line					*/
    bool first )	/* First call (ie not recursively called)	*/
{
    char   buffer[SHELL_MAX_COMMAND_LEN + 1];
    char   subcommand[SHELL_MAX_COMMAND_LEN + 1];
    char   *line_copy;
    char   *first_subcommand;
    UINT32 count, subcommand_count;
    UINT32 repeat_count = 1;
    UINT32 rc;
    UINT32 len;
    t_cmd  *cmd;

    line_copy = buffer;

    strcpy( line_copy, line );

    /* Separate line in subcommands based on ';' characters */
    separate( line_copy, &subcommand_count );

    if( first )
    {
        if( subcommand_count > 1 )
	{
	    /* Determine repeat (+<n>) count */
	    len = strlen(line_copy);
	  
	    if( get_repeat( line_copy, &repeat_count ) )
	    {
	        line_copy = &line_copy[len + 1];
	        subcommand_count--;
	    }
	}
    }

    first_subcommand = line_copy;

    do		     /* Loop based on repeat (+<n>) count */
    {
        line_copy = first_subcommand;
	count     = subcommand_count;

	do	     /* Perform each subcommand */
	{
            strcpy( subcommand, line_copy );
	    cmd = NULL;

	    if( first )
	    {
	        /* Perform expansion of environment variables */
	        rc = expand_line( subcommand );

		if( rc == OK ) 
		{
		    /* Execute the line */
		    rc = execute_line( subcommand, FALSE );

		    if( rc != OK ) 
		        return rc;
		}
	    }
	    else
	    {
		rc = tokenize( subcommand );

		if( rc == OK )
		    rc = execute_command( &cmd );
            }
	    
	    if( rc != OK )
	    {
	        if( !newline_after_prompt )
		    printf( "\n" );

                shell_command_error( cmd, rc );
                shell_error_data = NULL;
		shell_error_hint = NULL;
	    }
	    
	    line_copy = &line_copy[strlen(line_copy) + 1];
	}
	while( --count && (rc == OK) );
    }
    while( ((repeat_count == 0) || --repeat_count) && (rc == OK) );

    return rc;
}


/************************************************************************
 *                          error_lookup
 ************************************************************************/
static INT32 
error_lookup( 
    t_sys_error_string *param )
{
    UINT32 index = SYSERROR_ID( param->syserror );

    if( index >= SHELL_ERROR_MSG_COUNT )
        index = SHELL_ERROR_MSG_COUNT - 1;

    shell_error_msg[0] = '\0';

    add2error( shell_err[index] );

    if( shell_error_data )
    {
        add2error( " : " );
	add2error( shell_error_data );
	shell_error_data = NULL;
    }
	
    if( (param->syserror == SHELL_ERROR_SYNTAX) && failing_cmd && (failing_cmd->syntax) )
    {
	add2error( "\nSyntax :\n" );
	add2error( failing_cmd->syntax );
    }

    param->strings[SYSCON_ERRORMSG_IDX] = shell_error_msg;

    if( shell_error_hint )
    {
        param->strings[SYSCON_DIAGMSG_IDX] = NULL;
        param->strings[SYSCON_HINTMSG_IDX] = shell_error_hint;       
        param->count = 2;
    }
    else
        param->count = 1;

    return OK;
}


/************************************************************************
 *                          add2error
 ************************************************************************/
static void
add2error(
    char *s )
{
    UINT32 len, count;

    len   = strlen( shell_error_msg );
    count = strlen(s);

    if( len + count + 1 > MAX_SHELL_ERR_MSG )
        count = MAX_SHELL_ERR_MSG - len - 1;

    strncpy( &shell_error_msg[len], s, count );

    shell_error_msg[len+count] = '\0';
}
