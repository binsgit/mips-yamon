
/************************************************************************
 *
 *  gdb.c
 *
 *  Shell gdb command
 *
 *  gdb [-v][-c] [. <args>]
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
#include <syserror.h>
#include <shell_api.h>
#include <shell.h>
#include <gdb_stub.h>
#include <string.h>
#include <mips.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <excep_api.h>
#include <ctype.h>
#include <stdio.h>
#include <mips.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define GDB_OK			0
#define GDB_BREAK		1
#define GDB_ERROR_CSUM		2
#define GDB_CONTINUE		3
#define GDB_KILL		4
#define GDB_ERROR_GENERAL	5
#define GDB_ERROR_ADDRESS	6
#define GDB_APPL_DONE		7

/************************************************************************
 *  Public variables
 ************************************************************************/

extern t_gdb_regs   appl_context;

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_VERBOSE	        0
  { "v",  "Display messages from and to GDB host" },
#define OPTION_CHECKSUM_OFF	1
  { "c",  "Disable check of GDB checksum" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/* Port used for gdb connection to host */
static UINT32 gdb_port;

/* Arguments for application */
static char         *gdb_string = "gdb";

#define MAXBUF	2048
static char         input_buffer[MAXBUF];
static char         output_buffer[MAXBUF];

/* Single stepping */
static UINT32	    ss_count;
static UINT32	    ss_addr[2];
static UINT32	    ss_instr[2];      

/* Verbose or not */
static bool	    verbose;

/* Enable checksum verification */
static bool	    checksum_on;

/* Extended protocol */
static bool	    extended;

/* Error codes */
static char *msg_e01 = "$E01 : Illegal format.";
static char *msg_e02 = "$E02 : Illegal address.";
static char *msg_e03 = "$E03 : Structural error in GDB-stub.";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

static UINT64
*shell_get_epc(
    t_gdb_regs *regs );

static UINT32 
get_options(
    UINT32 argc,
    char   **argv );

static UINT32
get_packet(
   char *buf );

static void
init_data( void );

static UINT32
determine_reply(
    char *in_buf,
    char *out_buf,
    bool *ss );

static void
put_packet(
    char *buf );

static bool
setup_single_step( void );

static bool
cleanup_single_step( void );

static char
get_char(
    bool *ctrl_c );

static bool
valid_hex(
    char ch );

static UINT64
gethex( 
    char   *in_buf, 
    char   **ptr,
    UINT32 *rc,
    bool   endian_swap );

static UINT32
getval(
    char   **in_buf,
    UINT64 *val,
    UINT8  nibble_count,
    bool   strict,
    bool   endian_swap );

static void
setval(
    char   **out_buf,
    UINT64 val,
    UINT8  size,
    bool   endian_swap );

static UINT8
char2hex( 
    char ch );

static char
hex2char(
    UINT8 val );

static UINT32
write_regs(
    char   *in_buf,
    UINT32 size );

static void
read_regs(
    char   *out_buf,
    UINT32 size );

/************************************************************************
 *  				gdb
 ************************************************************************/
static MON_FUNC(gdb)
{
    bool   ss;
    UINT32 rc;
    bool   first;
    
    extended = FALSE;
    first    = TRUE;
    
    rc = get_options( argc, argv );

    if( rc != OK )
        return rc;

    printf( "Press Ctrl-C to return to YAMON\n" );

    /* Flush input UART fifo */
    while( GETCHAR( gdb_port, input_buffer ) );
    /*  Remote unit may have an output fifo, so     
     *  give it a chance to empty it and flush again.
     */
    sys_wait_ms(100);  
    while( GETCHAR( gdb_port, input_buffer ) );

    init_data();

    do
    {
        /* Get packet from GDB */ 
        rc = get_packet( input_buffer );

	switch( rc )
	{
	  case GDB_OK :

	    put_packet( "+" ); /* Transfer OK */

	    rc = determine_reply( input_buffer, 
			          output_buffer, 
			          &ss );

	    switch( rc )
	    {
	      case GDB_ERROR_GENERAL :
	        strcpy( output_buffer, msg_e01 );
		break;
	      case GDB_ERROR_ADDRESS :
	        strcpy( output_buffer, msg_e02 );
		break;
	      case GDB_KILL :
	      case GDB_OK   : 
	        /* output_buffer was written by determine_reply() */
	        break;
	      case GDB_CONTINUE :
	        /* Shift to user context */

	        if( shell_shift_to_user( TRUE, first ) )
		{
		    /* Application ended */
		    sprintf( output_buffer, "$W%02x", 
			     (UINT8)SYS_CPUREG(&appl_context, SYS_CPUREG_A0) );

		    rc = GDB_APPL_DONE;
		}
		else
		{ 
		    /* Exception */
		    if( ((UINT32)appl_context.cp0_cause & M_CauseExcCode) >> S_CauseExcCode ==
		            EX_BP )
		    {
		        /* Breakpoint */
 		        strcpy( output_buffer, "$S05" );
		    }
		    else
		    {
		        /* Other exception */
			strcpy( output_buffer, "$X0f" );
		    }
		}

                if( ss )
	        {
	            /* Break due to single step */
	            if( !cleanup_single_step() )
		    {
	                strcpy( output_buffer, msg_e02 );
		    }
	        }

		first = FALSE;
		
		break;
	      default : /* Should not happen */
	        strcpy( output_buffer, msg_e03 );
	        break;
	    }

	    break;

	  case GDB_ERROR_CSUM :
	  case GDB_ERROR_GENERAL :
	    
	    strcpy( output_buffer, "-$" );
	    break;

	  case GDB_BREAK :
	  default :  /* Should not happen */
	  
	    break;
        }

	if( rc != GDB_BREAK )
	{
	    /* Write reply */
	    put_packet( output_buffer );
	}
    }
    while( (rc != GDB_BREAK) && 
	   (rc != GDB_KILL)  && 
	   (rc != GDB_APPL_DONE) );

    if( verbose )
        printf( "\n" );

    return OK;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv )
{
    UINT32   arg;
    bool     ok	 = TRUE;
    UINT32   error = SHELL_ERROR_SYNTAX;
    bool     args = FALSE;
    char     *token;

    /* Setup defaults */
    verbose            = FALSE;
    checksum_on        = TRUE;
    shell_argc_appl    = 1;
    shell_argv_appl[0] = gdb_string;

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
	    if( strcmp( token, "-v" ) == 0 )
	    {
		verbose = TRUE;
            }
	    else if( strcmp( token, "-c" ) == 0 )
	    {
		checksum_on = FALSE;
            }
	    else if( *token == '-' )
	    {
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = token;
		ok		 = FALSE;
	    }
            else if( strcmp( token, "." ) == 0 )
	    {
	        args = TRUE;
	    }
	    else
	        ok = FALSE;
        }
    }

    return ok ? OK : error;
}

/************************************************************************
 *                          init_data
 ************************************************************************/
static void
init_data( void )
{
    /*  Setup initial context including EPC */ 
    shell_setup_default_cpu( &appl_context, 
			     shell_addr_go_valid ?
			         (UINT32)shell_addr_go :
				 0 );
}


/************************************************************************
 *                          get_packet
 ************************************************************************/
static UINT32
get_packet(
    char *buf )
{
    char   ch[2];
    char   *rcv;
    UINT8  csum_calc, csum_rcv;
    UINT8  i;
    bool   ctrl_c;
    UINT32 rc = GDB_OK;

    /* Wait for '$' */
    do
    {
        ch[0] = get_char( &ctrl_c );

	if( ctrl_c )
	{
	    rc = GDB_BREAK;
	    break;
	}
    }
    while( (ch[0] != '$') );

    /* Get packet contents, excluding cheksum bytes */
    if( rc == GDB_OK )
    {
        rcv       = buf;
        csum_calc = 0;

        do
        {
            ch[0] = get_char( &ctrl_c );

	    if( ctrl_c )
	    {
	        rc = GDB_BREAK;
		break;
	    }

	    switch( ch[0] )
	    {
	      case '$' :
	        /* Start again */
	        rcv       = buf;
	        csum_calc = 0;
	        break;
	      case '#' :
	        /* End */
	        *rcv = '\0';
	        break;
	      default :
	        /* Packet */
	        *rcv      = ch[0];
	        csum_calc += (UINT8)ch[0];
    	        rcv++;
	        break;
            }
        }
        while( ch[0] != '#' );
    }

    /* Get checksum bytes */
    if( rc == GDB_OK )
    {
	for(i=0; i<2; i++)
	{
            ch[i] = get_char( &ctrl_c );

	    if( ctrl_c )
	    {
	        rc = GDB_BREAK;
		break;
	    }
	}
    }

    /* Validate checksum (if not disabled by the -c command option) */
    if( (rc == GDB_OK) && checksum_on )
    {
        csum_rcv = 0;

        for(i=0; i<2; i++)
        {
            csum_rcv <<= 4;

	    if( !valid_hex( ch[i] ) )
	    {	
	        rc = GDB_ERROR_GENERAL;
		break;
	    }

            csum_rcv += char2hex( ch[i] );
        }
    }

    if( verbose )
    {
        if( rc == GDB_BREAK )
        {
	    printf( "(Break)" );
        }
        else
        {
	    printf( "(From Host : $%s#%c%c)", buf, ch[0], ch[1] );
	}
    }

    if( (rc == GDB_OK) && checksum_on && (csum_rcv != csum_calc) )
        rc = GDB_ERROR_CSUM;

    return rc;
}

/************************************************************************
 *                          determine_reply
 ************************************************************************/
static UINT32
determine_reply(
    char *in_buf,
    char *out_buf,
    bool *ss )
{
    int		i;
    UINT32	count;
    UINT64	val;
    char	ch;
    UINT32	address, addr_kseg0;
    char	*ptr;
    UINT32	reg, size;
    UINT32	rc;

    /* Defaults */
    *ss   = FALSE;
      
    /* Start sequence */
    strcpy( out_buf, "$" );
    out_buf+=1;

    ch = *in_buf;
    in_buf++;

    switch( ch )
    {
      case '!' :
        /* 
	 *  extended ops	   !	Use the extended remote protocol.
         *				Sticky -- only needs to be set once.
	 */

	extended = TRUE;
	strcpy( out_buf, "OK" );
	break;

      case '?' : 

	/* 
	 *  last signal     ?           Reply the current reason for stopping.
         *                              This is the same reply as is generated
	 *				for step or cont : SAA where AA is the
	 *				signal number.
	 */

	/* We always indicate TRAP */
	strcpy( out_buf, "S05" );
	break;

      case 'G' :

	/* 
	 *  write 32 bit regs  GXX..XX	Each byte of register data
	 *				is described by two hex digits.
	 *  reply	       OK	for success
	 *		       ENN	for an error
	 */

	rc = write_regs( in_buf, sizeof(UINT32) );
	if( rc != GDB_OK )
	    return rc;	

	strcpy( out_buf, "OK" );
	break;

      case 'H' :

	/* 
	 *  Set Thread (Hg | Hc) or "Write 64 bit regs" command.
	 *
	 *  Set Thread request is ignored.
	 *
	 *  Write 64 bit regs  HXX..XX	Each byte of register data
	 *				is described by two hex digits.
	 *  reply	       OK	for success
	 *		       ENN	for an error
	 */

	if( (*in_buf == 'g') || (*in_buf == 'c') )
	{
	    /* Possibly "set thread" command */
	    if( strlen( in_buf ) <= 18 )
	    {
	        *out_buf = '\0';
		break;
	    }
        }

	/* Write 64 bit regs command */

	rc = write_regs( in_buf, sizeof(UINT64) );
	if( rc != GDB_OK )
	    return rc;	

	strcpy( out_buf, "OK" );
	break;

      case 'g' :

	/* 
	 *  read registers  g[S]	Registers of size S
	 *  reply	    XX....X	Each byte of register data
	 *				is described by two hex digits.
	 *				Registers are in the internal order
	 *				for GDB, and the bytes in a register
	 *				are in the same order the machine uses.
	 *		    or ENN	for an error.
	 */

	if( *in_buf != '\0' )
	{
	    size = gethex( in_buf, &ptr, &rc, FALSE );

	    if( (rc != GDB_OK) || (*ptr != '\0') )
            {
	        return GDB_ERROR_GENERAL;
            }

	    if( (size != sizeof(UINT32)) &&
		(size != sizeof(UINT64)) )
	    {
		return GDB_ERROR_GENERAL;
	    }
        }
	else
	    size = sizeof(UINT32);

	read_regs( out_buf, size );
	break;

      case 'r' :
      
        /*
	 *  Read single register (SDE extension)   rNN[:S]    Register NN 
	 *						       of size S
  	 */

	reg = gethex( in_buf, &ptr, &rc, FALSE );

	if( (rc != GDB_OK) || ( (*ptr != '\0') && (*ptr != ':') ) )
        {
	    return GDB_ERROR_GENERAL;
        }
	else
        {
	    if( *ptr == ':' )
	    {
	        ptr++;

	        size = gethex( ptr, &ptr, &rc, FALSE );

		if( (rc != GDB_OK) || (*ptr != '\0') )
		{
		    return GDB_ERROR_GENERAL;
		}

		if( (size != sizeof(UINT32)) &&
		    (size != sizeof(UINT64)) )
		{
		    return GDB_ERROR_GENERAL;
		}
            }
	    else	
	        size = sizeof(UINT32);
        }

	setval( &out_buf, SYS_CPUREG(&appl_context, reg), size, TRUE );
	*out_buf = '\0';

	break;

      case 'R' :
         /*
	  *  Command depends on whether we are running extended
          *  protocol (due to '!' command) or not.
	  *
          *  Normal   : Identical to 'P', see below.
	  *  Extended : Restart (reinitialise registers)
	  */

         if(extended && (*in_buf == '\0') ) 
	 {
	     init_data();
	     strcpy( out_buf, "OK" );
	     break;
         } 

         /* Fallthrough !! */

      case 'P' :

        /*
	 *  Write single register (SDE extension)   PNN[:S]=XX    Register NN 
	 *						          of size S
  	 */

	reg = gethex( in_buf, &ptr, &rc, FALSE );

	if( rc != GDB_OK )
	    return GDB_ERROR_GENERAL;

	size = sizeof(UINT32);  /* Default */

        switch( *ptr )
	{
	  case ':' :

	    ptr++;
	    
	    size = gethex( ptr, &ptr, &rc, FALSE );

   	    if( (rc != GDB_OK)           ||
		(size != sizeof(UINT32)) ||
		(*ptr != '=') )
            {
		return GDB_ERROR_GENERAL;
	    }

	    if( (size != sizeof(UINT32)) &&
		(size != sizeof(UINT64)) )
	    {
		return GDB_ERROR_GENERAL;
	    }

  	    /* FALL THROUGH ! */

	  case '=' :

	    ptr++;

	    val = gethex( ptr, &ptr, &rc, TRUE );

	    if( size == sizeof(UINT32) )
	    {
	        /* Sign extend */
		val = (INT64)((INT32)((UINT32)val));
	    }

	    if( (rc != GDB_OK) || (*ptr != '\0') )
	    {
	        return GDB_ERROR_GENERAL;
	    }

	    break;
	  default  :

	    return GDB_ERROR_GENERAL;
        }

        SYS_CPUREG(&appl_context, reg) = val;
	strcpy( out_buf, "OK" );
	break;

      case 'm' :

        /*
	 *  read mem	mAA..AA,LLLL	AA..AA is address, LLLL is length.
	 *  reply	XX..XX		XX..XX is mem contents
	 *				Can be fewer bytes than requested
	 *				if able to read only part of the data.
	 *		or ENN		NN is errno
         */

	 /**** Fall through !! ****/

      case 'M' :

	/* 
	 *  write mem	MAA..AA,LLLL:XX..XX
	 *				AA..AA is address,
	 *				LLLL is number of bytes,
	 *				XX..XX is data
	 *  reply	OK		for success
	 *		ENN		for an error (this includes the case
	 *				where only part of the data was
	 *				written).
	 */

	/* Find the ',' */
	ptr = strchr( in_buf, ',' );

	if( ptr )
	{
	    *ptr = '\0';
	    ptr++;
	    
	    /* Get address */
	    address = gethex( in_buf, NULL, &rc, FALSE );

	    if( rc != GDB_OK )
	        return GDB_ERROR_GENERAL;

	    /* Get count */
	    count = gethex( ptr, NULL, &rc, FALSE );

	    if( rc != GDB_OK )
	        return GDB_ERROR_GENERAL;
	}

	if( !ptr )
	    return GDB_ERROR_GENERAL;
	else
	{
  	    /* Validate range */
	    if( sys_validate_range( address,
				    count,
				    sizeof(UINT8),
				    FALSE ) != OK )
	    {
	        return GDB_ERROR_ADDRESS;
	    }

	    if( ch == 'm' )
	    {
	        /* Read */
		for( i=0; i<count; i++ )
		{
		    setval( &out_buf, (UINT64)REG8(address),
			    sizeof(UINT8), FALSE );
		    address++;
		}
	    }
	    else
	    {
	        /* Write (M command) */
	        
		/* Find the ':' */
		ptr = strchr( ptr, ':' );

		if( !ptr )
		    return GDB_ERROR_GENERAL;
		else
		    ptr++;
		
		for( i=0; i<count; i++ )
		{
		    rc = getval( &ptr, &val, 2*sizeof(UINT8), FALSE, FALSE );
		    if( rc != GDB_OK )
		        return rc;

		    /* Convert to KSEG0 */
		    if( !sys_kseg0( address, &addr_kseg0 ) )
		        return GDB_ERROR_ADDRESS;

		    REG8(addr_kseg0) = (UINT8)val;

		    /*  Flush D-cache in order to store data in
		     *  physical memory.
		     *  Invalidate I-cache since new instructions may
		     *  have been stored;
		     */

		    sys_flush_cache_line( (void *)addr_kseg0 );

		    address++;
		}   

		strcpy( out_buf, "OK" );
		out_buf += 2;
	    }

	    *out_buf = '\0';
	}

	break;

      case 'C' :

	/*
	 *  continue with	Csig;AA..AA	Continue with signal sig (hex signal
	 *  signal				number).  If ;AA..AA is omitted, 
	 *					resume at same address.
	 */

      case 'S' :

	/* 
	 *  step with		Ssig;AA..AA	Like 'C' but step not continue.
	 *  signal
	 */

	 /* skip sig; sequence */

	 ptr = strchr( in_buf, ';' );

	 if( ptr )
	     in_buf = ptr + 1;

	 /**** Fall through !! ****/

      case 'c' :
	
	/* 
	 *  continue	cAA..AA  	AA..AA is address to resume
	 *				If AA..AA is omitted,
	 *				resume at same address.
         */

      case 's' :

        /* 
	 *  step	sAA..AA		AA..AA is address to resume
	 *				If AA..AA is omitted,
	 *				resume at same address.
	 */

	if( *in_buf != '\0' )
	{
	    /* Get address */
            address = gethex( in_buf, NULL, &rc, FALSE );

	    if( rc != GDB_OK )
	        return GDB_ERROR_GENERAL;
	    else
	    {
	        /* Store address in epc */
	        appl_context.cp0_epc = (INT64)((INT32)address);

	        /* Clear ERL and set EXL so that EPC will be used upon eret */
	        appl_context.cp0_status &= ~(UINT64)M_StatusERL;
	        appl_context.cp0_status |= (UINT64)M_StatusEXL;
	    }
	}

	if( (ch == 's') || (ch =='S') )
	{   
	    if( !setup_single_step() )
	        return GDB_ERROR_ADDRESS;
	    *ss = TRUE;
	}

	return GDB_CONTINUE;

      case 'D' :

	/* detach         D               Reply OK. */

      case 'k' :

	/* kill request	k */

	strcpy( out_buf, "OK" );	/* Reply ignored by GBD for kill */
	return GDB_KILL;

      default  : /* Other */
	*out_buf = '\0';
	break;
    }

    return GDB_OK;
}

/************************************************************************
 *                          put_packet
 ************************************************************************/
static void
put_packet(
    char *buf )
{
    UINT8 csum      = 0;
    bool  calc_csum = FALSE;
    char  ch[2];
    char  *s = buf;

    while( *buf != '\0' )
    {
        PUTCHAR( gdb_port, *buf );

	if( calc_csum )	
	    csum += (UINT8)(*buf);

	if( *buf == '$' )
	    calc_csum = TRUE;

	buf++;
    }

    if( calc_csum )
    {
        ch[0] = hex2char( csum >> 4 );
	ch[1] = hex2char( csum & 0xF );

        PUTCHAR( gdb_port, '#' );
        PUTCHAR( gdb_port, ch[0] );
        PUTCHAR( gdb_port, ch[1] );

        if( verbose )
        {
            printf( "(To Host : %s#%c%c)", s, ch[0], ch[1] );
        }
    }
    else
    {
        if( verbose )
        {
            printf( "(To Host : %s)", s );
        }
    }
}


/************************************************************************
 *				shell_get_epc
 ************************************************************************/
static UINT64
*shell_get_epc(
    t_gdb_regs *regs )
{
    return
        ( (UINT32)regs->cp0_status & M_StatusERL ) ?
	    &regs->cp0_errorepc :
	    &regs->cp0_epc;
}


/************************************************************************
 *                          setup_single_step
 ************************************************************************/
static bool
setup_single_step( void )
{
    UINT32 epc, instruction, opcode;
    UINT32 addr_kseg0;
    UINT32 i;
    bool   branch;
    INT32  offset;
    UINT32 target;
    bool   mips16e;
    bool   extend;
    bool   jump;
    UINT32 size;
    UINT32 reg;    

    /* Read EPC (only support for 32 bit) */
    epc	= (UINT32)*shell_get_epc(&appl_context);

    /* Determine mips16e, then clear lsb */
    mips16e =  (epc & 1);
    epc	    &= ~1;
    size    =  mips16e ? sizeof(UINT16) : sizeof(UINT32);

    if( mips16e && !sys_mips16e )
        return FALSE;
    
    /* Validate address */
    if( sys_validate_range( epc,
			    size,
			    size,
			    FALSE ) != OK )
    {
        return FALSE;
    }

    /* Convert to KSEG0 */
    if( !sys_kseg0( epc, &addr_kseg0 ) )
        return FALSE;

    /* Read next instruction */
    instruction = mips16e ? REG16( addr_kseg0 ) : REG32( addr_kseg0 );

    /* Defaults */
    branch     = FALSE;
    extend     = FALSE;
    ss_count   = 1;
    ss_addr[0] = epc + size;

    /* Determine where to set BREAK(s) */

    if( mips16e )
    {
        opcode = MIPS16E_OPCODE(instruction);

	if( (opcode == MIPS16E_OPC_EXTEND) ||
	    (opcode == MIPS16E_OPC_JAL_X) )
	{
	    extend = TRUE;

	    epc        += sizeof(UINT16);
            ss_addr[0] += sizeof(UINT16);

            /* Validate address of second half of instruction */
            if( sys_validate_range( epc,
			            sizeof(UINT16),
			            sizeof(UINT16),
			            FALSE ) != OK )
            {
                return FALSE;
            }

	    /* Convert to KSEG0 */
	    if( !sys_kseg0( epc, &addr_kseg0 ) )
	        return FALSE;

	    /* Read next instruction */
	    instruction = (instruction << 16) | REG16( addr_kseg0 );

	    if( opcode == MIPS16E_OPC_EXTEND )
                opcode = MIPS16E_OPCODE(instruction);
        }

	/* Default break must be MIPS16e */
        ss_addr[0] |= 1;

	switch( opcode )
	{
	  case MIPS16E_OPC_B :
	    if( extend )
	    {
 	        offset = (((instruction >> 16) & 0x1f) << 11) |
		         (((instruction >> 21) & 0x3f) <<  5) |
			 (((instruction >>  0) & 0x1f) <<  0);

		offset <<= 1;

		/* Sign extend */
		if( offset & 0x10000 ) 
		    offset |= 0xffff0000;
	    }
	    else
	    {
	        offset = (instruction & 0x7ff) << 1;

		/* Sign extend */
		if( offset & 0x800 )
		    offset |= 0xfffff000;  
	    }

	    ss_addr[0] =  (UINT32)(offset + (INT32)(epc + 2));
	    ss_addr[0] |= 1;
	    break;
	  case MIPS16E_OPC_BEQZ :
	  case MIPS16E_OPC_BNEZ :
	    branch = TRUE;
	    break;
	  case MIPS16E_OPC_I8 :
	    switch( MIPS16E_I8_FUNCTION( instruction ) )
	    {
	      case MIPS16E_I8_FUNC_BTEQZ :
	      case MIPS16E_I8_FUNC_BTNEZ :
	        branch = TRUE;
		break;
	    }
	    break;
	  case MIPS16E_OPC_JAL_X : 
	    /* JAL/JALX */
	    target = (((instruction >> 16) & 0x001f) << 21) |
		     (((instruction >> 21) & 0x001f) << 16) |
		     (((instruction >>  0) & 0xffff) <<  0);

	    target <<= 2;

	    ss_addr[0] = ((epc + 2) & 0xf0000000) | target;

	    if( MIPS16E_X( instruction ) != MIPS16E_X_JALX )
	        ss_addr[0] |= 1;
	    break;
	  case MIPS16E_OPC_RR :
	    if( MIPS16E_RR_FUNCTION( instruction ) ==
	            MIPS16E_RR_FUNC_JALRC )
	    {
	        jump = TRUE;

	        switch( MIPS16E_RY( instruction ) )
		{ 
		   case MIPS16E_RR_RY_JRRX :
		   case MIPS16E_RR_RY_JALR :
		   case MIPS16E_RR_RY_JRCRX :
		   case MIPS16E_RR_RY_JALRC :
		     switch( (instruction >> 8) & 0x7 )
		     {
		       case 0 : reg = SYS_CPUREG_S0; break;
		       case 1 : reg = SYS_CPUREG_S1; break;
		       case 2 : reg = SYS_CPUREG_V0; break;
		       case 3 : reg = SYS_CPUREG_V1; break;
		       case 4 : reg = SYS_CPUREG_A0; break;
		       case 5 : reg = SYS_CPUREG_A1; break;
		       case 6 : reg = SYS_CPUREG_A2; break;
		       case 7 : reg = SYS_CPUREG_A3; break;
		     }
		     break;
		   case MIPS16E_RR_RY_JRRA :
		   case MIPS16E_RR_RY_JRCRA :
		     reg = SYS_CPUREG_RA;
		     break;
		   default :
		     jump = FALSE;
		     break;
		}

		if( jump )
	             ss_addr[0] = (UINT32)SYS_CPUREG(&appl_context, reg);
	    }
	    
	    break;
	}

	if( branch )
	{
            /*  We need to set two breakpoints, one if the branch is
	     *  taken, and one if it is not taken.
	     */ 
	    if( extend )
	    {
 	        offset = (((instruction >> 16) & 0x1f) << 11) |
		         (((instruction >> 21) & 0x3f) <<  5) |
			 (((instruction >>  0) & 0x1f) <<  0);

		offset <<= 1;

		/* Sign extend */
		if( offset & 0x10000 ) 
		    offset |= 0xffff0000;
	    }
	    else
	    {
	        offset = (instruction & 0xff) << 1;

		/* Sign extend */
		if( offset & 0x100 )
		    offset |= 0xffffff00;
	    }

	    ss_addr[0] = epc + 2;
	    ss_addr[1] = (UINT32)(offset + (INT32)( epc + 2 ));

	    ss_addr[0] |= 1;
	    ss_addr[1] |= 1;
	    
            if( ss_addr[0] != ss_addr[1] )
	        ss_count = 2;
	}
    }
    else
    {
        opcode = MIPS_OPCODE(instruction);

        switch( opcode )
        {
          case OPC_SPECIAL :
	    switch( MIPS_FUNCTION( instruction ) )
	    {
	      case FUNC_JR :
	      case FUNC_JALR :
	        ss_addr[0] =
	            (UINT32)SYS_CPUREG(&appl_context, MIPS_RS(instruction));
	        break;
	    }

	    break;
          case OPC_REGIM :
	    switch( MIPS_RT( instruction ) )
	    {
              case RT_BLTZ :
              case RT_BGEZ :
              case RT_BLTZL :
              case RT_BGEZL :
              case RT_BLTZAL :
              case RT_BGEZAL :
              case RT_BLTZALL :
              case RT_BGEZALL :
	        branch = TRUE;
	        break;
	    }

	    break;
          case OPC_BEQ :
          case OPC_BNE :
          case OPC_BLEZ :
          case OPC_BGTZ :
          case OPC_BEQL :
          case OPC_BNEL :
          case OPC_BLEZL :
          case OPC_BGTZL :
            branch = TRUE;
	    break;
          case OPC_J :
          case OPC_JAL :
	  case OPC_JALX :
            ss_addr[0] = ((epc + 4) & (MSK(4) << 28)) |
	                 (MIPS_TARGET( instruction ) << 2);

	    if( opcode == OPC_JALX )
	        ss_addr[0] |= 1;

	    break;
	  case OPC_COP1 :
	    if( sys_fpu )
	    {
	        switch( MIPS_RS(instruction) )
	        {
                  case RS_BC1 :
	            /* Floating point branch */
	            branch = TRUE;
	            break;
	        }
	    }
	    break;
        }

        if( branch )
        {
            /*  We need to set two breakpoints, one if the branch is
	     *  taken, and one if it is not taken.
	     */ 
	    offset = (INT32)((INT16)MIPS_OFFSET(instruction)) * 4;

	    ss_addr[0] = epc + 2*4; /* Instruction after branch delay slot */
	    ss_addr[1] = (UINT32)(offset + (INT32)( epc + 4 ));

            if( ss_addr[0] != ss_addr[1] )
	        ss_count = 2;
        }
    }

    /* Set BREAK(s) */
    for( i=0; i < ss_count; i++ )
    {
        mips16e     =  (ss_addr[i] & 1);
        ss_addr[i]  &= ~1;
        size        =  mips16e ? sizeof(UINT16) : sizeof(UINT32);

        /* Validate address */
        if( sys_validate_range( ss_addr[i],
			        size,
			        size,
			        TRUE ) != OK )
        {
            return FALSE;
        }

        /* Convert to KSEG0 */
	if( !sys_kseg0( ss_addr[i], &addr_kseg0 ) )
	    return FALSE;

	/* Store original instruction and insert break */
	if( mips16e )
	{
            ss_instr[i]         = REG16( addr_kseg0 );
	    REG16( addr_kseg0 ) = MIPS16E_OPCODE_BREAK;
	}
	else
	{
            ss_instr[i]         = REG32( addr_kseg0 );
	    REG32( addr_kseg0 ) = OPCODE_BREAK;
	}

        /*  Flush D-cache in order to store data in
	 *  physical memory.
	 *  Invalidate I-cache since new instructions may
	 *  have been stored;
	 */

	sys_flush_cache_line( (void *)addr_kseg0 );

        if( mips16e )
            ss_addr[i] |= 1;
    }

    return TRUE;
}

/************************************************************************
 *                          cleanup_single_step
 ************************************************************************/
static bool
cleanup_single_step( void )
{  
    UINT32 i;
    UINT32 addr_kseg0;
    bool   mips16e;
    UINT32 size;

    /* Write back original instruction(s) */
    for( i=0; i<ss_count; i++)
    {
        mips16e     =  (ss_addr[i] & 1);
        ss_addr[i]  &= ~1;
        size        =  mips16e ? sizeof(UINT16) : sizeof(UINT32);

        if( mips16e && !sys_mips16e )
            return FALSE;

        /* Validate address */
        if( sys_validate_range( ss_addr[i],
			        size,
			        size,
			        TRUE ) != OK )
        {
            return FALSE;
        }

        /* Convert to KSEG0 */
	if( !sys_kseg0( ss_addr[i], &addr_kseg0 ) )
	    return FALSE;

	if( mips16e )
            REG16( addr_kseg0 ) = ss_instr[i];
	else
            REG32( addr_kseg0 ) = ss_instr[i];

        /*  Flush D-cache in order to store data in
	 *  physical memory.
	 *  Invalidate I-cache since new instructions may
	 *  have been stored;
	 */
	sys_flush_cache_line( (void *)addr_kseg0 );
    }

    return TRUE;
}



/************************************************************************
 *                          get_char
 ************************************************************************/
static char
get_char(
    bool *ctrl_c )
{
    char ch;

    do
    {
        *ctrl_c = GETCHAR_CTRLC( DEFAULT_PORT );
    }
    while( !(*ctrl_c) && !GETCHAR( gdb_port, &ch ) );

    return ch;
}



/************************************************************************
 *                          valid_hex
 ************************************************************************/
static bool
valid_hex(
    char ch )
{
    if( (ch >= '0') && (ch <= '9') )
        return TRUE;

    ch = tolower( ch );

    if( (ch >= 'a') && (ch <= 'f') )
        return TRUE;

    return FALSE;
}    


/************************************************************************
 *                          char2hex
 ************************************************************************/
static UINT8
char2hex( 
    char ch )
{
    if( (ch >= '0') && (ch <= '9') )
        return ch - '0';

    ch = tolower( ch );

    if( (ch >= 'a') && (ch <= 'f') )
        return ch - 'a' + 10;

    return 0;
}


/************************************************************************
 *                          hex2char
 ************************************************************************/
static char
hex2char(
    UINT8 val )
{
    if( val < 10 )
        return '0' + val;
    else
        return val - 10 + 'a';
}


/************************************************************************
 *                          getval
 ************************************************************************/
static UINT32
getval(
    char   **in_buf,
    UINT64 *val,
    UINT8  nibble_count,
    bool   strict,
    bool   endian_swap )
{
    UINT32 i;
    bool   valid;

    *val  = 0;
    valid = FALSE;

    for( i=0; i < nibble_count; i++ )
    {
        if( !valid_hex( **in_buf ) )
	{
	    if( strict )
	        return GDB_ERROR_GENERAL;
	    else
	    {
	        break;
	    }
	}
	else
	{
	    valid = TRUE;
            *val <<= 4;
	    *val += char2hex(**in_buf);
	    (*in_buf)++;
	}
    }

    if( !valid )
        return GDB_ERROR_GENERAL;
    else
    {
#ifdef EL
        if( endian_swap )
	{
            if( nibble_count == 2*sizeof(UINT32) )
                *val = SWAPEND32( (UINT32)(*val) );

            if( nibble_count == 2*sizeof(UINT64) )
                *val = SWAPEND64( *val );
        }
#endif	    

        return GDB_OK;
    }
}


/************************************************************************
 *                          setval
 ************************************************************************/
static void
setval(
    char   **out_buf,
    UINT64 val,
    UINT8  size,
    bool   endian_swap )
{
    UINT32 i;

#ifdef EL
    if( endian_swap )
    {
        if( size == sizeof(UINT32) )
            val = SWAPEND32( (UINT32)val );

        if( size == sizeof(UINT64) )
            val = SWAPEND64( val );
    }
#endif	    

    for( i = 2*size; i > 0; i-- )
    {
        (*out_buf)[i-1] = hex2char(val & 0xF);
	val >>= 4;
    }

    (*out_buf) += size * 2;
}


/************************************************************************
 *                          write_regs
 ************************************************************************/
static UINT32
write_regs(
    char   *in_buf,
    UINT32 size )
{
    UINT64 *regs;
    UINT64 val;
    UINT32 i;
    UINT32 rc;

    regs = (UINT64 *)&appl_context;

    for( i=0; i<GDB_REG_COUNT; i++ )
    {
        rc = getval( &in_buf, &val, 2*size, TRUE, TRUE );

	if( rc != GDB_OK )
	    return rc;
	else
	{
	    if( size == sizeof(UINT32) )
	    {
	        /* Sign extend */
		val = (INT64)((INT32)((UINT32)val));
	    }

	    regs[i] = val;
	}
    }

    return GDB_OK;
}


/************************************************************************
 *                          read_regs
 ************************************************************************/
static void
read_regs(
    char   *out_buf,
    UINT32 size )
{
    UINT64 *regs;
    UINT32 i;

    regs = (UINT64 *)&appl_context;

    for( i=0; i<GDB_REG_COUNT; i++ )
    {
	setval( &out_buf, regs[i], size, TRUE );
    }

    *out_buf = '\0';
}


/************************************************************************
 *                          gethex
 ************************************************************************/
static UINT64
gethex( 
    char   *in_buf, 
    char   **ptr,
    UINT32 *rc,
    bool   endian_swap )
{
    UINT64 val, tmp;
    UINT32 nibble_count;

    if( rc )
        *rc = GDB_ERROR_GENERAL;  /* Default */

    val          = 0;
    nibble_count = 0;

    while( valid_hex( *in_buf ) )
    {
	/* We have at least one valid nibble */     
        if( rc )
            *rc = GDB_OK;

        getval( &in_buf, &tmp, 1, FALSE, FALSE );

	val <<= 4;
	val += tmp;
	nibble_count++;
    }

#ifdef EL
    if( endian_swap )
    {
        if( nibble_count == 2*sizeof(UINT32) )
            val = SWAPEND32( (UINT32)(val) );

        if( nibble_count == 2*sizeof(UINT64) )
            val = SWAPEND64( val );
    }
#endif	    

    if( ptr )
        *ptr = in_buf;

    return val;
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "gdb",
     gdb,

    "gdb [-v][-c] [. <args>]",

    "Setup connection to GDB debugger on port ttyx.\n"
    "\n"
    "The Standard GDB remote protocol is used.\n"
    "\n"
    "If the user application is not currently running, and Ctrl-C\n"
    "is typed at the console, YAMON will leave GDB mode and\n"
    "return to the command prompt.\n"
    "\n"
    "<args> is broken up in substrings and passed to the application.\n"
    "The list of arguments to be passed must begin with a '.'.\n"
    "The '.' is not passed as an argument. The first argument (argv[0])\n"
    "will be the string 'gdb'.\n"
    "\n"
    "a0 is set to the number of substrings (argc).\n"
    "a1 is set to the address of an array of pointers to the substrings (argv).\n"
    "a2 is set to the address of the environment table.\n"
    "a3 is set to the memory size.\n"
    "ra holds the return address to YAMON.\n"
    "\n"
    "The application may return to YAMON by jumping to the address\n"
    "specified in ra or by calling the exit(rc) function supplied by YAMON.\n"
    "\n"
    "The verbose (-v) option will cause the commands from the\n"
    "GDB host and the responses from YAMON to be displayed on the\n"
    "console.\n"
    "\n"
    "The checksum off (-c) option will disable validation of\n"
    "the cheksum used in GDB commands. This is useful in case\n"
    "the user wishes to enter commands manually. Two checksum\n"
    "characters should still be used in the commands, but the\n"
    "values are don't care.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_gdb_init
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
shell_gdb_init( void )
{
    UINT8 uart_count;
    char  *ch;

    /* Determine the number of serial ports */
    SYSCON_read( SYSCON_BOARD_UART_COUNT_ID, 
		 (void *)&uart_count, 
		 sizeof(UINT8) );

    /* Select the port to use for GDB */
    gdb_port = (uart_count > 1) ? DEFAULT_GDB_PORT : PORT_TTY0;

    /* Setup port in help text */
    ch = strstr( cmd_def.descr, "ttyx" );
    if( ch )
        ch[3] = ( (gdb_port == PORT_TTY0) ? '0' : '1' );

    return &cmd_def;

}

