
/************************************************************************
 *
 *  stty.c
 *
 *  Shell stty command
 *
 *  stty [-tty<0|1>] [-b|-u| [-p][<baudrate>][n|o|e][7|8][1|2][hw|none]]
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
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <shell_api.h>
#include <shell.h>
#include <syscon_api.h>
#include <env_api.h>
#include <string.h>
#include <stdio.h>
#include <serial_api.h>
#include <io_api.h>
#include <sys_api.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static MON_FUNC(stty);

static MON_FUNC(stty_sdb);

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    UINT8  *action,
#define ACTION_READ	0x01
#define ACTION_LIST	0x02
#define ACTION_UPDATE	0x04
#define ACTION_STORE	0x08
    UINT8  *port,
    UINT8  *baudrate,
    UINT8  *parity,
    UINT8  *databits,
    UINT8  *stopbits,
    UINT8  *flowctrl,
    bool   sdb );

static UINT32
do_stty(
    UINT8 action,
    UINT8 port,
    UINT8 baudrate,
    UINT8 parity,
    UINT8 databits,
    UINT8 stopbits,
    UINT8 flowctrl );

static void
re_init_port(
    UINT32 port );

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static UINT8 uart_count;


static char *help_text = 
    "Setup or view serial port setup. Default port is "
#if (DEFAULT_PORT == PORT_TTY0)
    "tty0"
#else
    "tty1"
#endif
    ".\n"
    "-b,-u,-p apply to the default port if no port is specified.\n"
    "\n"
    "The possible baudrates are generally 75-460800, but not all baudrates\n"
    "are supported by all platforms. Use 'stty -ttyx -b' to get a list of\n"
    "the supported baudrates for a specific port.\n"
    "\n"
    "Available parity settings are n (none), o (odd), e (even).\n"
    "Available databits are 7 and 8.\n"
    "Available stopbits are 1 and 2.\n"
    "Available flowctrl settings are hw and none.\n"
    "\n"
    "When changing the parameters for a tty which is being used (e.g. the\n"
    "console), some strange characters may appear as a result.\n"
    "\n"
    "Also note that stty does not by default modify the semi-permanent tty\n"
    "setting recorded in the environment variables. Use the '-p' option if\n"
    "you want to set the environment variable for the specific tty as well.";

static char *help_text_one_tty = 
    "Setup or view serial port setup.\n"
    "\n"
    "The possible baudrates are generally 75-460800, but not all baudrates\n"
    "are supported by all platforms. Use 'stty -b' to get a list of\n"
    "the supported baudrates.\n"
    "\n"
    "Available parity settings are n (none), o (odd), e (even).\n"
    "Available databits are 7 and 8.\n"
    "Available stopbits are 1 and 2.\n"
    "Available flowctrl settings are hw and none.\n"
    "\n"
    "When changing the parameters, some strange characters may appear as\n"
    "a result.\n"
    "\n"
    "Also note that stty does not by default modify the semi-permanent tty\n"
    "setting recorded in the environment variable. Use the '-p' option if\n"
    "you want to set the environment variable as well.";


/* OPTIONS */
static t_cmd_option options[] =
{ 
  { "u",     "Force environment settings for port to take effect" },
  { "p",     "Transfer current settings for port to environment" },
  { "b",     "List supported baud rates for port" },
  { "tty0",  "Setup port 0 - default" },
  { "tty1",  "Setup port 1" }
};
#define OPTION_COUNT	     (sizeof(options)/sizeof(t_cmd_option))
#define OPTION_COUNT_ONE_TTY (OPTION_COUNT - 2)

static char *syntax = 
    "stty [-tty<0|1>] [-b|-u|[-p][<baudrate>][n|o|e][7|8][1|2][hw|none]]";

static char *syntax_one_tty = 
    "stty [-b|-u|[-p][<baudrate>][n|o|e][7|8][1|2][hw|none]]";

/* Command definition for help */
static t_cmd cmd_def =
{
    "stty",
    stty,
    NULL,
    NULL,
    options,
    OPTION_COUNT,
    FALSE
};

static t_cmd cmd_def_sdb_lower =
{
    "b",
     stty_sdb,
    "b <baudrate>             (Microsoft SDB command)",
    "Setup baudrate of port "
#if (DEFAULT_PORT == PORT_TTY0)
    "tty0."
#else
    "tty1."
#endif
    " This command is equivalent to\n"
    "'stty -tty0 <baudrate>'. See 'help stty' for more information on\n"
    "available baudrates.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          stty
 ************************************************************************/
static MON_FUNC(stty)
{
    /* Options */
    UINT8   action, port, baudrate, parity, databits, stopbits, flowctrl;
    UINT32  rc;

    rc = get_options( argc, argv,
		      &action,
		      &port,
		      &baudrate,
		      &parity,
		      &databits,
		      &stopbits,
		      &flowctrl,
		      FALSE );

    if( rc != OK )
        return rc;
    else
    {
        return do_stty(action,port,baudrate,parity,databits,stopbits,flowctrl);
    }
}


/************************************************************************
 *                          stty_sdb
 ************************************************************************/
static MON_FUNC(stty_sdb)
{
    /* Options */
    UINT8  action, port, baudrate, parity, databits, stopbits, flowctrl;
    UINT32 rc;

    rc = get_options( argc, argv,
		      &action,
		      &port,
		      &baudrate,
		      &parity,
		      &databits,
		      &stopbits,
		      &flowctrl,
		      TRUE );

    if( rc != OK )
        return rc;
    else
    {
        return do_stty(action,port,baudrate,parity,databits,stopbits,flowctrl);
    }
}


/************************************************************************
 *                          do_stty
 ************************************************************************/
static UINT32
do_stty(
    UINT8 action,
    UINT8 port,
    UINT8 baudrate,
    UINT8 parity,
    UINT8 databits,
    UINT8 stopbits,
    UINT8 flowctrl )
{
    char    msg[200];
    char    *s;
    UINT32  rc;
    char    *mode;

    if( action & ACTION_LIST )
    {
	if( SHELL_PUTS( "\nAvailable baudrates are :\n\n" )) return OK;
	    
	for( baudrate = SERIAL_BAUDRATE_NOT_DEFINED + 1;
	     baudrate < SERIAL_BAUDRATE_MAX;
	     baudrate++ )
        {
            /*  Validate baudrate for specific port.
             *  If the following write operation fails, the baudrate is
	     *  not supported.
	     */
	    if( SYSCON_write( (port == PORT_TTY0) ?
		                  SYSCON_COM_TTY0_BAUDRATE_VERIFY_ID :
			          SYSCON_COM_TTY1_BAUDRATE_VERIFY_ID,
		              (void *)&baudrate, sizeof(UINT8) ) == OK )
	    {
	        env_baudrate_num2s( baudrate , &s );

		if( SHELL_PUTS( s )) return OK;
		if( SHELL_PUTC( '\n' )) return OK;
	    }
        }

	SHELL_PUTC( '\n' );
	
	return OK;
    }


    /**************************************************************
    ** Settings have been read from "CURRENT" or from command line
    */


    s = (port == PORT_TTY0) ? ENV_MODETTY0 : ENV_MODETTY1;

    if( action & ACTION_UPDATE )
    {
        /**********************************************
        ** Replace settings with those from environment
        */

        if( !env_get( s, &mode, NULL, 0 ) )
	    return SHELL_ERROR_FAILED;  /* TBD : error code */
	
	env_modetty_string2parts( mode,
                                  &baudrate,
				  &parity,
				  &databits,
				  &stopbits,
				  &flowctrl );
    }

    if( action & ACTION_STORE )
    {	
        /********************************
	** Store settings in environment
        */

        /* put together and store new mode */
        env_modetty_parts2string( &mode,
				  baudrate,
				  parity,
				  databits,
				  stopbits,
				  flowctrl );

        rc = env_set( s, mode, ENV_ATTR_RW, NULL, NULL );
	
	if( rc != OK )
	    return rc;
    }
    
    /*****************
    ** show settings
    */
 
    if( action & ACTION_STORE )
	sprintf( msg, "Stored: " );
    else
	*msg = '\0';

    strcat( msg, "baudrate=" );
    env_baudrate_num2s( baudrate, &s );
    strcat( msg, s );

    strcat( msg, ", parity=" );
    env_parity_num2s( parity, &s );
    strcat( msg, s );

    strcat( msg, ", databits=" );
    env_databits_num2s( databits, &s );
    strcat( msg, s );

    strcat( msg, ", stopbits=" );
    env_stopbits_num2s( stopbits, &s );
    strcat( msg, s );

    strcat( msg, ", flowcontrol=" );
    env_flowctrl_num2s( flowctrl, &s );
    strcat( msg, s );

    strcat( msg, "\n" );

    if( SHELL_PUTS( msg ) ) return OK;


    if( (action & ACTION_READ) == 0 )
    {	
        /******************************
        ** move settings to "CURRENT"
        */

	if( port == PORT_TTY0 )
	{
            SYSCON_write( SYSCON_COM_TTY0_BAUDRATE_ID,
		   	  (void *)&baudrate, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY0_PARITY_ID,
		   	  (void *)&parity, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY0_DATABITS_ID,
		   	  (void *)&databits, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY0_STOPBITS_ID,
		   	  (void *)&stopbits, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY0_FLOWCTRL_ID,
		   	  (void *)&flowctrl, 
			  sizeof( UINT8 ) );
	}
	else
	{
            SYSCON_write( SYSCON_COM_TTY1_BAUDRATE_ID,
	  	          (void *)&baudrate, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY1_PARITY_ID,
		   	  (void *)&parity, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY1_DATABITS_ID,
		   	  (void *)&databits, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY1_STOPBITS_ID,
		   	  (void *)&stopbits, 
			  sizeof( UINT8 ) );

            SYSCON_write( SYSCON_COM_TTY1_FLOWCTRL_ID,
		   	  (void *)&flowctrl, 
			  sizeof( UINT8 ) );
	}
	    
        /*******************************
        ** apply "CURRENT" settings 
        */

	re_init_port(port);
    }

    return OK;
}


/************************************************************************
 *                          re_init_port
 ************************************************************************/
static void
re_init_port(
    UINT32 port )
{
    /* Reinitialise device in order for the settings 
     * to take effect.
     */
 
    UINT32 major, minor;

    SYSCON_read( (port == PORT_TTY0) ? SYSCON_COM_TTY0_MAJOR : SYSCON_COM_TTY1_MAJOR,
	         (void *)(&major),
		 sizeof(UINT32) );

    SYSCON_read( (port == PORT_TTY0) ? SYSCON_COM_TTY0_MINOR : SYSCON_COM_TTY1_MINOR,
		 (void *)(&minor),
		 sizeof(UINT32) );

    if( port == DEFAULT_PORT )
    {
        SHELL_PUTS( "Reconfigure terminal program\n" );
        sys_wait_ms( 100 );
    }

    IO_init( major, minor, (void *)&port );
}


/************************************************************************
 *                          get_options
 *
 *  stty [-tty<0|1>] [-b|-u| [-p][<baudrate>][n|o|e][7|8][1|2][hw|none]]
 *  legal syntaxes:      Action:           Output:
 *    stty * -b          _READ | _LIST     baud rate possibilities
 *    stty * -u          _UPDATE           new settings + "Reconfigure..."
 *    stty *             _READ             settings
 *    stty * -p          _READ | _STORE    settings
 *    stty * -p <parm>   _STORE            new settings + "Reconfigure..."
 *    stty * <parm>      0                 "Reconfigure terminal program"
 *    b <baudrate>       0                 "Reconfigure terminal program"
 *
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    UINT8  *action,
    UINT8  *port,
    UINT8  *baudrate,
    UINT8  *parity,
    UINT8  *databits,
    UINT8  *stopbits,
    UINT8  *flowctrl,
    bool   sdb )
{
    char   *token;
    bool   ok             = TRUE;
    bool   baudrate_valid = FALSE;
    bool   parity_valid   = FALSE;
    bool   databits_valid = FALSE;
    bool   stopbits_valid = FALSE;
    bool   flowctrl_valid = FALSE;
    UINT32 arg;
    UINT32 error = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    *action   = ACTION_READ;
    *port     = DEFAULT_PORT;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
		  (token = argv[arg]);
         arg++ )
    {
	 if( env_baudrate_s2num( token, baudrate, sizeof(UINT8)))
	 {             *action &= ~ACTION_READ;
             ok = !baudrate_valid;
             baudrate_valid = TRUE;
	 }
	 else if( sdb )
	 {
            if( *token == '-' )
	    {
	        error            = SHELL_ERROR_OPTION;
		shell_error_data = token;
		ok		 = FALSE;
	    }
         }
	 else if( (uart_count > 1) &&
	          (strcmp( token, "-tty0" )) == 0 )
	 {
	     *port = PORT_TTY0;
	 }
	 else if( (uart_count > 1) &&
	          (strcmp( token, "-tty1" )) == 0 )
	 {
	     *port = PORT_TTY1;
	 }
	 else if( strcmp( token, "-b" ) == 0 )
	 {
	     *action |= ACTION_LIST;
	 }
         else if( strcmp( token, "-u" ) == 0 )
	 {
             /* ACTION_READ is cleared below - after syntax checking */
	     *action |= ACTION_UPDATE;
	 }
         else if( strcmp( token, "-p" ) == 0 )
	 {
	     *action |= ACTION_STORE;
	 }
	 else if( env_parity_s2num( token, parity, sizeof(UINT8)))
	 {
             *action &= ~ACTION_READ;
             ok = !parity_valid;
             parity_valid = TRUE;
	 }
	 else if( env_databits_s2num( token, databits, sizeof(UINT8)))
	 {
             *action &= ~ACTION_READ;
             ok = !databits_valid;
             databits_valid = TRUE;
	 }
	 else if( env_stopbits_s2num( token, stopbits, sizeof(UINT8)))
	 {
             *action &= ~ACTION_READ;
             ok = !stopbits_valid;
             stopbits_valid = TRUE;
	 }
	 else if( env_flowctrl_s2num( token, flowctrl, sizeof(UINT8)))
	 {
             *action &= ~ACTION_READ;
             ok = !flowctrl_valid;
             flowctrl_valid = TRUE;
	 }
	 else
	 {
	    ok = FALSE;
	   
            if( *token == '-' )
	    {
	        error            = SHELL_ERROR_OPTION;
		shell_error_data = token;
	    }
	 }
    }

    /* Syntax check: Allow only legal combinations of options */
    if( sdb )
    {
        if (!(*action == 0 ) )
            ok = FALSE;
    }
    else
    {
        if (!(*action == 0 ||
              *action == (ACTION_READ)  ||
              *action == (ACTION_READ|ACTION_LIST)  ||
              *action == (ACTION_READ|ACTION_STORE)  ||
              *action == (ACTION_READ|ACTION_UPDATE)  ||
              *action == (ACTION_STORE) ) )
            ok = FALSE;
    }

    /* modify temporary action value used for syntax check */
    if (*action == (ACTION_READ|ACTION_UPDATE))
        *action &= ~ACTION_READ;

    if( !ok )
        return error;

    if( baudrate_valid )
    {
        /*  Validate baudrate for specific port.
         *  If the following write operation fails, the baudrate is
	 *  not supported.
	 */
	if( SYSCON_write( (*port == PORT_TTY0) ?
		              SYSCON_COM_TTY0_BAUDRATE_VERIFY_ID :
			      SYSCON_COM_TTY1_BAUDRATE_VERIFY_ID,
		          (void *)baudrate, sizeof(UINT8) ) != OK )
        {
	    return SHELL_ERROR_BAUDRATE;
	}
    }
    else
    {
        SYSCON_read( (*port == PORT_TTY0) ?
		          SYSCON_COM_TTY0_BAUDRATE_ID :
		          SYSCON_COM_TTY1_BAUDRATE_ID,
		     (void *)baudrate, sizeof(UINT8) );
    }

    if( !parity_valid )
    {
        SYSCON_read( (*port == PORT_TTY0) ?
		          SYSCON_COM_TTY0_PARITY_ID :
		          SYSCON_COM_TTY1_PARITY_ID,
		     (void *)parity, sizeof(UINT8) );
    }

    if( !databits_valid )
    {
        SYSCON_read( (*port == PORT_TTY0) ?
		          SYSCON_COM_TTY0_DATABITS_ID :
		          SYSCON_COM_TTY1_DATABITS_ID,
		     (void *)databits, sizeof(UINT8) );
    }

    if( !stopbits_valid )
    {
        SYSCON_read( (*port == PORT_TTY0) ?
		          SYSCON_COM_TTY0_STOPBITS_ID :
		          SYSCON_COM_TTY1_STOPBITS_ID,
		     (void *)stopbits, sizeof(UINT8) );
    }
    
    if( !flowctrl_valid )
    {
        SYSCON_read( (*port == PORT_TTY0) ?
		          SYSCON_COM_TTY0_FLOWCTRL_ID :
		          SYSCON_COM_TTY1_FLOWCTRL_ID,
		     (void *)flowctrl, sizeof(UINT8) );
    }
    
    return ok ? OK : error;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_stty_init
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
shell_stty_init( void )
{
    /* Determine the number of serial ports */
    SYSCON_read( SYSCON_BOARD_UART_COUNT_ID, 
		 (void *)&uart_count, 
		 sizeof(UINT8) );

    if( uart_count > 1 )
    {
        cmd_def.syntax       = syntax;
	cmd_def.option_count = OPTION_COUNT;
	cmd_def.descr	     = help_text;
    }
    else
    {
        cmd_def.syntax       = syntax_one_tty;
	cmd_def.option_count = OPTION_COUNT_ONE_TTY;
	cmd_def.descr	     = help_text_one_tty;
    }

    return &cmd_def;
}

/************************************************************************
 *
 *                          shell_stty_sdb_init
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
shell_stty_sdb_init( void )
{
    return &cmd_def_sdb_lower;
}
