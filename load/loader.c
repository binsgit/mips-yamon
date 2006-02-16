/************************************************************************
 *
 *  load.c
 *
 *  load using serial port or Ethernet
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

#include <sysdefs.h>
#include <sysdev.h>
#include <syserror.h>
#include <syscon_api.h>
#include <string.h>
#include <stdio.h>
#include <loader_api.h>
#include <sys_api.h>
#include <io_api.h>
#include <flash_api.h>
#include <net_api.h>
#include <shell_api.h>
#include <ctype.h>

extern UINT32 net_last_error ;

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    char   *id;
    UINT32 (*func)(
	       UINT32	      port,
	       UINT32	      ip,
	       char           *filename,
	       void	      **addr,
	       t_image_format *imageformat,
	       UINT32	      *error_pos,
	       UINT32	      *raw_error );
}
t_load_format;

static t_load_format formats[] =
{
    { "S",         NULL }
//    { "\x7f""ELF", NULL }
};
#define LOAD_FORMATS	(sizeof(formats)/sizeof(t_load_format))

#define COUNT_DISP_INTERVAL  0x200
#define MAX_LINE_SIZE   200

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* context, keping current s-record being read and processed */
static UINT8  loader_diag_msg[MAX_LINE_SIZE+24];
static UINT32 line_number ;
static bool   eol_was_cr ;
static UINT8  sdata[MAX_LINE_SIZE+4];

static char* loader_error_string[] =
{
    /* ERROR_LOAD_NONE              */ NULL,
    /* ERROR_LOAD_UNSUPPORTED_PORT  */ "Port not supported",
    /* ERROR_LOAD_AMBIGUOUS_TYPE    */ "Load format is ambigous",
    /* ERROR_LOAD_UNKNOWN_TYPE      */ "Load format is unknown",
    /* ERROR_LOAD_ILLEGAL_FORMAT    */ "Load format not compliant",
    /* ERROR_LOAD_CHECKSUM          */ "Invalid checksum",
    /* ERROR_LOAD_LINE_TOO_LONG     */ "Line too long",
    /* ERROR_LOAD_NET               */ "Load error from net",
    /* ERROR_LOAD_STORE             */ "Load error by store of data",
    /* ERROR_LOAD_BREAK             */ "Load was interrupted by user",
    /* ERROR_LOAD_NO_S7_RECORD      */ "Termination record (S7,S8 or S9) is lacking",
    /* ERROR_LOAD_ILLEGAL_CHARACTER */ "Illegal character in line",
    /* ERROR_LOAD_UART_COMM_ERROR   */ "UART communication error",
} ;

static char* loader_error_hint_string[] =
{
    /* ERROR_LOAD_NONE              */ NULL,
    /* ERROR_LOAD_UNSUPPORTED_PORT  */ "Use help",
    /* ERROR_LOAD_AMBIGUOUS_TYPE    */ NULL,
    /* ERROR_LOAD_UNKNOWN_TYPE      */ NULL,
    /* ERROR_LOAD_ILLEGAL_FORMAT    */ NULL,
    /* ERROR_LOAD_CHECKSUM          */ NULL,
    /* ERROR_LOAD_LINE_TOO_LONG     */ NULL,
    /* ERROR_LOAD_NET               */ NULL,
    /* ERROR_LOAD_STORE             */ NULL,
    /* ERROR_LOAD_BREAK             */ NULL,
    /* ERROR_LOAD_NO_S7_RECORD      */ "Add S7 termination record as last line of file",
    /* ERROR_LOAD_ILLEGAL_CHARACTER */ NULL,
    /* ERROR_LOAD_UART_COMM_ERROR   */ NULL,
} ;

#define     NO_HEX          128
#define     VALID_SPACE     129
static const UINT8 char2val[256] =
{
    /*   0: NUL  */    VALID_SPACE,  /* space */
    /*   1: SOH  */    NO_HEX,
    /*   2: STX  */    NO_HEX,
    /*   3: ETX  */    NO_HEX,
    /*   4: EOT  */    NO_HEX,
    /*   5: ENQ  */    NO_HEX,
    /*   6: ACK  */    NO_HEX,
    /*   7: BEL  */    NO_HEX,
    /*   8: BS   */    NO_HEX,
    /*   9: HT   */    VALID_SPACE,  /* space */
    /*  10: LF   */    VALID_SPACE,  /* space */
    /*  11: VT   */    NO_HEX,
    /*  12: NP   */    NO_HEX,
    /*  13: CR   */    VALID_SPACE,  /* space */
    /*  14: SO   */    NO_HEX,
    /*  15: SI   */    NO_HEX,
    /*  16: DLE  */    NO_HEX,
    /*  17: DC1  */    NO_HEX,
    /*  18: DC2  */    NO_HEX,
    /*  19: DC3  */    NO_HEX,
    /*  20: DC4  */    NO_HEX,
    /*  21: NAK  */    NO_HEX,
    /*  22: SYN  */    NO_HEX,
    /*  23: ETB  */    NO_HEX,
    /*  24: CAN  */    NO_HEX,
    /*  25: EM   */    NO_HEX,
    /*  26: SUB  */    NO_HEX,
    /*  27: ESC  */    NO_HEX,
    /*  28: FS   */    NO_HEX,
    /*  29: GS   */    NO_HEX,
    /*  30: RS   */    NO_HEX,
    /*  31: US   */    NO_HEX,
    /*  32: SP   */    VALID_SPACE,  /* space */
    /*  33: !    */    NO_HEX,
    /*  34: "    */    NO_HEX,
    /*  35: #    */    NO_HEX,
    /*  36: $    */    NO_HEX,
    /*  37: %    */    NO_HEX,
    /*  38: &    */    NO_HEX,
    /*  39: '    */    NO_HEX,
    /*  40: (    */    NO_HEX,
    /*  41: )    */    NO_HEX,
    /*  42: *    */    NO_HEX,
    /*  43: +    */    NO_HEX,
    /*  44: ,    */    NO_HEX,
    /*  45: -    */    NO_HEX,
    /*  46: .    */    NO_HEX,
    /*  47: sl   */    NO_HEX,
    /*  48: 0    */    0,
    /*  49: 1    */    1,
    /*  50: 2    */    2,
    /*  51: 3    */    3,
    /*  52: 4    */    4,
    /*  53: 5    */    5,
    /*  54: 6    */    6,
    /*  55: 7    */    7,
    /*  56: 8    */    8,
    /*  57: 9    */    9,
    /*  58: :    */    NO_HEX,
    /*  59: ;    */    NO_HEX,
    /*  60: <    */    NO_HEX,
    /*  61: =    */    NO_HEX,
    /*  62: >    */    NO_HEX,
    /*  63: ?    */    NO_HEX,
    /*  64: @    */    NO_HEX,
    /*  65: A    */    10,
    /*  66: B    */    11,
    /*  67: C    */    12,
    /*  68: D    */    13,
    /*  69: E    */    14,
    /*  70: F    */    15,
    /*  71: G    */    NO_HEX,
    /*  72: H    */    NO_HEX,
    /*  73: I    */    NO_HEX,
    /*  74: J    */    NO_HEX,
    /*  75: K    */    NO_HEX,
    /*  76: L    */    NO_HEX,
    /*  77: M    */    NO_HEX,
    /*  78: N    */    NO_HEX,
    /*  79: O    */    NO_HEX,
    /*  80: P    */    NO_HEX,
    /*  81: Q    */    NO_HEX,
    /*  82: R    */    NO_HEX,
    /*  83: S    */    NO_HEX,
    /*  84: T    */    NO_HEX,
    /*  85: U    */    NO_HEX,
    /*  86: V    */    NO_HEX,
    /*  87: W    */    NO_HEX,
    /*  88: X    */    NO_HEX,
    /*  89: Y    */    NO_HEX,
    /*  90: Z    */    NO_HEX,
    /*  91: [    */    NO_HEX,
    /*  92: bsl  */    NO_HEX,
    /*  93: ]    */    NO_HEX,
    /*  94: ^    */    NO_HEX,
    /*  95: _    */    NO_HEX,
    /*  96: `    */    NO_HEX,
    /*  97: a    */    10,
    /*  98: b    */    11,
    /*  99: c    */    12,
    /* 100: d    */    13,
    /* 101: e    */    14,
    /* 102: f    */    15,
    /* 103: g    */    NO_HEX,
    /* 104: h    */    NO_HEX,
    /* 105: i    */    NO_HEX,
    /* 106: j    */    NO_HEX,
    /* 107: k    */    NO_HEX,
    /* 108: l    */    NO_HEX,
    /* 109: m    */    NO_HEX,
    /* 110: n    */    NO_HEX,
    /* 111: o    */    NO_HEX,
    /* 112: p    */    NO_HEX,
    /* 113: q    */    NO_HEX,
    /* 114: r    */    NO_HEX,
    /* 115: s    */    NO_HEX,
    /* 116: t    */    NO_HEX,
    /* 117: u    */    NO_HEX,
    /* 118: v    */    NO_HEX,
    /* 119: w    */    NO_HEX,
    /* 120: x    */    NO_HEX,
    /* 121: y    */    NO_HEX,
    /* 122: z    */    NO_HEX,
    /* 123: {    */    NO_HEX,
    /* 124: |    */    NO_HEX,
    /* 125: }    */    NO_HEX,
    /* 126: ~    */    NO_HEX,
    /* 127: DEL  */    NO_HEX,
    /* 128:      */    NO_HEX,
    /* 129:      */    NO_HEX,
    /* 130:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 135:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 140:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 145:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 150:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 155:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 160:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 165:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 170:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 175:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 180:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 185:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 190:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 195:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 200:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 205:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 210:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 215:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 220:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 225:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 230:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 235:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 240:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 245:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX,
    /* 250:      */    NO_HEX, NO_HEX, NO_HEX, NO_HEX, NO_HEX

} ;
/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 loader_readline( UINT32  port ) ;
static INT32  loader_error_lookup( t_sys_error_string *p_param ) ;

static UINT32
load_srec(
    UINT32	   port,		/* Port number			*/
    UINT32	   ip,			/* IP address (valid for NET)   */
    char           *filename,		/* valid for net		*/
    void	   **addr,		/* Start address		*/
    t_image_format *imageformat,	/* Output : Format		*/
    UINT32	   *error_pos,  	/* Output : Position of error   */
    UINT32	   *raw_error );	/* Output : Raw error of subsys */

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          loader_init
 *  Description :
 *  -------------
 *
 *  Init load module
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
loader_init( void )
{
    t_sys_error_lookup_registration registration ;

    /* register lookup syserror */
    registration.prefix = SYSERROR_DOMAIN( ERROR_LOADER ) ;
    registration.lookup = loader_error_lookup ;
    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    formats[0].func = load_srec;
}

/************************************************************************
 *
 *                          loader_image
 *  Description :
 *  -------------
 *
 *  Load image from serial port or Ethernet
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code (see loader_api.h)
 *
 ************************************************************************/
UINT32
loader_image(
    UINT32	   port,		/* Port number			*/
    UINT32	   ip,			/* IP addr (valid for PORT_NET) */
    char           *filename,		/* valid for PORT_NET		*/
    void	   **addr,		/* Output : Start address	*/
    t_image_format *imageformat,	/* Output : Format		*/
    UINT32	   *error_pos, 		/* Output : Position of error	*/
    UINT32	   *raw_error )		/* Output : Raw error from subs	*/
{
    bool          fit[LOAD_FORMATS];
    UINT32        pos     = 0;
    UINT32        maxid   = 0;
    t_load_format *format = NULL;
    UINT32        f, rc;
    char	  ch;

    /* clear error context */
    sdata[0] = 0 ;
    line_number = 0 ;
    eol_was_cr = FALSE ;
    *raw_error = OK ;  /* default completion */

    if( (port != PORT_TTY0) && (port != PORT_TTY1) && (port != PORT_NET) )
    {
        *raw_error = ERROR_LOAD_UNSUPPORTED_PORT ;
        return ERROR_LOAD_UNSUPPORTED_PORT;
    }
    
    /* First identify the format */
    for(f=0; f<LOAD_FORMATS; f++)
    {
        maxid  = MAX(maxid, strlen(formats[f].id));
        fit[f] = TRUE;
    }

    /* If we are loading from net, open file */
    if (port==PORT_NET)
    {
        *raw_error = net_open( ip, filename ) ;
        if (*raw_error != OK)
        {
            if (*raw_error == ERROR_NET_USER_BREAK)
                return( ERROR_LOAD_BREAK ) ;

            return( ERROR_LOAD_NET );
        }
    }

    /* read first line */
    *raw_error = loader_readline( port ) ;
    *error_pos = line_number ;
    if ( *raw_error != OK ) return( *raw_error ) ;

    /* check format */
    ch = toupper(sdata[0]) ;
    for(f=0; f<LOAD_FORMATS; f++)
    {
        if( (strlen(formats[f].id) > pos) && 
            (ch != formats[f].id[pos]) )
        {
            fit[f] = FALSE;
        }
    }

    for(f=0; f<LOAD_FORMATS; f++)
    {
        if( fit[f] )
	{
	    if( format )
            {
                *raw_error = ERROR_LOAD_AMBIGUOUS_TYPE ;
	        return ERROR_LOAD_AMBIGUOUS_TYPE;
            }
	    else
            {
	        format = &formats[f];
            }
        }
    }

    if( !format || !format->func )
    {
        *raw_error = ERROR_LOAD_UNKNOWN_TYPE ;
        return ERROR_LOAD_UNKNOWN_TYPE;
    }
    else
    {
	if( (port == PORT_TTY0) || (port == PORT_TTY1) )
	{
	    /*  Disable any not critical but time consuming 
	     *  polling operations while we are loading in
	     *  order not to risk overflow in UART buffers.
	     */
	     sys_poll_enable( FALSE );
        }

        /* Flush and invalidate DCACHE, so that
	 * old writeback data will not corrupt the new
	 * image
	 */
        sys_dcache_flush_all();

	/* Run the load protocol */
        rc = format->func(port,
			  ip,
			  filename,
			  addr,
			  imageformat,
			  error_pos,
			  raw_error );
	
	/* Flush caches */
	sys_flush_caches();

	if( (port == PORT_TTY0) || (port == PORT_TTY1) )
	{
	    /*  Reenable polling */
	     sys_poll_enable( TRUE );
        }

        /* Hack to return net derived error */
        if (port==PORT_NET)
        {
            if (net_last_error != OK)
            {
                *raw_error = net_last_error ;
                if (*raw_error == ERROR_NET_USER_BREAK)
                    return( ERROR_LOAD_BREAK ) ;

                return( ERROR_LOAD_NET );
            }
        }

	return rc;
    }
}

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          load_srec
 ************************************************************************/
static UINT32
load_srec(
    UINT32	   port,		/* Port number			*/
    UINT32	   ip,			/* IP address (valid for NET)   */
    char           *filename,		/* valid for net		*/
    void	   **addr,		/* Start address		*/
    t_image_format *imageformat,	/* Output : Format		*/
    UINT32	   *error_pos,  	/* Output : Position of error   */
    UINT32	   *raw_error ) 	/* Output : Raw error of subsys */
{
#define OFFSET_COUNT_37	3
#define OFFSET_COUNT_28	4
#define OFFSET_COUNT_19	5
#define OFFSET_ADDRESS  4
#define OFFSET_DATA	8


    UINT32 type, ch;
    UINT32 checksum;
    UINT32 rc, offset, offset_count, bytecount, my_addr;
    UINT8  data[MAX_LINE_SIZE];	/* Must be UINT32 alligned */
    UINT32 val;
    t_FLASH_write_descriptor flash_write ;
    UINT32 count     = 0;
    UINT32 dot_count = 0;
    UINT32 ix;

    *imageformat = MOTOROLA_S3;

    addr[0] = (UINT32*)0 ;
    addr[1] = (UINT32*)0xffffffff ;
    addr[2] = (UINT32*)0 ;

    /*  Layout for S3
     *
     *  0..2         : Not used
     *  3	     : count
     *  4..7         : address
     *  8..8+count-1 : data
     *  8+count      : checksum
     */

    SHELL_DISABLE_MORE;

    while(TRUE)
    {	
        if( !(count % COUNT_DISP_INTERVAL) )
	{
	    if( port == PORT_NET )
	    {
	        if(!dot_count)
                    printf("\n") ;
	        if(shell_print_dot( &dot_count ))
		{
		    rc = ERROR_LOAD_BREAK;
		    goto done;
		}
	    }
	}
	count++;
	
        /* Read type */
        type = sdata[1] ;

	/* Read line */
	if      (type == '3') offset_count = OFFSET_COUNT_37;
	else if (type == '7') offset_count = OFFSET_COUNT_37;
	else if (type == '2') offset_count = OFFSET_COUNT_28;
	else if (type == '8') offset_count = OFFSET_COUNT_28;
	else if (type == '1') offset_count = OFFSET_COUNT_19;
	else if (type == '9') offset_count = OFFSET_COUNT_19;
	else if (type == '0') offset_count = 0;
	else if (type == '5') offset_count = 0;
	else
	{
	    rc = ERROR_LOAD_ILLEGAL_FORMAT;
	    goto done;
	}
	checksum    = 0;
	offset = offset_count;
	ix = 2;

        /* Convert line */
	while (TRUE)
	{
            ch = sdata[ix++] ; /* get next character */
            val = char2val[ch] ; /* convert character */

	    if ( val & NO_HEX )
	    {
                if ( val == NO_HEX )
                {
                    rc = ERROR_LOAD_ILLEGAL_CHARACTER ;
	            goto done;
                }

                /* check rest of line, must contain VALID_SPACE only */
                while (ch)
                {
                    ch = sdata[ix++] ; /* get char */
                    if ( char2val[ch] != VALID_SPACE )
		    {
			rc = ERROR_LOAD_ILLEGAL_FORMAT;
			goto done;
		    }
                }
	        break;
	    }

            ch = sdata[ix++] ; /* get next character */
            ch = char2val[ch] ; /* convert character */

	    if ( ch & NO_HEX )
	    {
                rc = ( ch == NO_HEX )
                     ? ERROR_LOAD_ILLEGAL_CHARACTER
	             : ERROR_LOAD_ILLEGAL_FORMAT;
	        goto done;
	    }

            /* Store value */
	    val            = (val << 4) | ch ;
            data[offset++] = val;
            checksum       += val;
	}

	/* Do various checks (don't perform checks for S0/S5 since
	* they are discarded anyway.
	*/

	if( offset_count )
	{
	    /* Check count */
	    if( offset - offset_count  - 1 != data[offset_count] )
	    {
	        rc = ERROR_LOAD_ILLEGAL_FORMAT;
	        goto done;
	    }

	    /* Check that enough data is available */
	    if( offset < OFFSET_DATA + 1 )
	    {
	        rc = ERROR_LOAD_ILLEGAL_FORMAT;
	        goto done;
	    }

	    /* Check checksum */
	    if( (checksum & 0xff) != 0xff)
            {
                rc = ERROR_LOAD_CHECKSUM ;
	        goto done;
            }

	    /* Address */
	    if ( type != '3' )
	    {
                if ( type == '2' )
                {
                    /* 3-byte address is kept in 5-7 */
                    data[OFFSET_ADDRESS] = 0 ;
                }
                else if ( type == '8' )
                {
                    /* 3-byte address is kept in 5-7 */
                    data[OFFSET_ADDRESS] = 0 ;
                }
                else if ( type == '1' )
                {
                    /* 2-byte address is kept in 6-7 */
                    data[OFFSET_ADDRESS] = 0 ;
                    data[OFFSET_ADDRESS+1] = 0 ;
                }
                else if ( type == '9' )
                {
                    /* 2-byte address is kept in 6-7 */
                    data[OFFSET_ADDRESS] = 0 ;
                    data[OFFSET_ADDRESS+1] = 0 ;
                }
            }
#ifdef EL
	    my_addr = SWAPEND32( *(UINT32*)&data[OFFSET_ADDRESS] );
#else
            my_addr = (*(UINT32 *)&data[OFFSET_ADDRESS]);
#endif

	    /* Decode line */

            bytecount = offset - OFFSET_DATA - 1;
	    
  	    if (type <= '3')
	    {
	        /* S3, S2 or S1 record (data record) */
	        if(bytecount)
	        {
		    /* Write data to RAM/FLASH
		     *   
		     * dst addr  = (UINT32)*addr
		     * src addr  = &data[OFFSET_DATA]
		     */

                    /* check range context to return this at load end */
                    if ( (UINT32)addr[1] > my_addr ) 
                        addr[1] = (void *)my_addr  ;
                    if ( (UINT32)addr[2] < (my_addr+bytecount-1) ) 
                        addr[2] = (void *)(my_addr+bytecount-1)  ;

		    /* Validate address */
		    rc = sys_validate_range( 
		             my_addr,
			     bytecount,
			     sizeof(UINT8),
			     TRUE );

		    if (rc != OK) goto done;

                    flash_write.adr     = my_addr;
                    flash_write.length  = bytecount ;
                    flash_write.buffer  = &data[OFFSET_DATA] ;
                    rc = IO_write( SYS_MAJOR_FLASH_STRATA, 0, &flash_write ) ;
                    if (rc != OK) goto done;

                    /* indicate address in alpha-display */
                    DISP( my_addr ) ;
	        }
	    }
	    else
	    {
	        /* S7, S8 or S9 record (end record) */
	        if( port == PORT_NET )
	        {
                    /* This just to guarantee proper report */
                    printf("\n") ;
	        }
                if (bytecount != 0 )
		{
		    rc = ERROR_LOAD_ILLEGAL_FORMAT;
		    goto done;
		}

		/* return last address read */
		addr[0] = (void *)my_addr;
		rc = OK;
		goto done;
	    }

        }

        /* read next line */
        rc = loader_readline( port ) ;
        if ( rc != OK ) goto done;
        ch = sdata[0] ;
	if( toupper(ch) != 'S' )
	{
	    rc = ERROR_LOAD_ILLEGAL_FORMAT;
	    goto done;
	}
    }
done:
    *error_pos = line_number ;
    *raw_error = rc ;
    return rc;
}

/************************************************************************
 *                          loader_readline
 ************************************************************************/
static UINT32 loader_readline( UINT32  port ) 
{
    UINT8  ch8;
    UINT32 ch ;
    UINT32 sid ;
    UINT32 rc ;

    /* adjust line number counter */
    /* line number is increased here and by leading LF */
    if (eol_was_cr) 
    {
	/* previous line ended with CR, so await leading LF */
	eol_was_cr = FALSE ; 
    }
    else
    {
	line_number++;
    }

    /* clear actual line */
    sid = 0 ;

    /* do read lines until any not empty found */
    rc = OK;
    while ( sid == 0 )
    {
        do
        {
            while( !GETCHAR( port, &ch8 ) )
            {
                /* check for user break */
                if (port != DEFAULT_PORT )
                {
                    if (GETCHAR_CTRLC( DEFAULT_PORT ))
		    {
			rc = ERROR_LOAD_BREAK;
			goto out;
		    }
                }
            }
	    ch = ch8;
    
	    if ( char2val[ch] & NO_HEX )
	    {
                /* check for user break */
                if (( ch == CTRL_C ) && (port != PORT_NET))
	        {
                    rc = ERROR_LOAD_BREAK ;
		    goto out;
	        }

                if ( ch == (EOF_SREC & 0xff) )
                {
                    /* check for End Of File; used by TFTP */
                    sid = 0 ; /* clear current line */
                    rc = ERROR_LOAD_NO_S7_RECORD;
		    goto out;
                }

                if ( ch == (UART_ERROR & 0xff) )
                {
                    /* UART communication error */
                    rc = ERROR_LOAD_UART_COMM_ERROR;
		    goto out;
                }

                /* Check for NULL chars */
                if ( ch == 0 )
                {
                    /* raw null characters are never expected */
                    /* turned into '?' to make them printable */
                    ch = '?';
                }

                /* Check for unexpected chars */
	        if ( char2val[ch] == NO_HEX )
                {
		    /* turn not-printable into '?' */
                    if (!isprint(ch))
                    {
                        ch = '?';
                    }
                }

                if ( sid == 0 )
                {
		    /* line is still empty */
                    if ( ch == LF )
                    {
                        /* leading NL increments line counter */
                        line_number++;
                    }

		    if ( char2val[ch] == VALID_SPACE )
		    {
                        /* ignore lading white space characters */
		        continue;
		    }
                }
	    }

            /* check line size */
            if (sid >= MAX_LINE_SIZE)
            {
                /* terminate line and return error */
                rc = ERROR_LOAD_LINE_TOO_LONG;
		goto out;
            }
             
            /* save character */
            sdata[sid++] = ch ;

	    if ( ch == CR )
	    {
		/* We got CR as eol - return and make ready for next call */
		eol_was_cr = TRUE ; 
		break;
	    }
        }
        while( ch != LF ) ;

    }
    rc = OK;
out:
    sdata[sid] = 0 ; /* remember to allocate space for the NUL-termination */
    return rc;
}


static
INT32 loader_error_lookup( t_sys_error_string *p_param )
{
    UINT32 t, i ;

    i = 0 ;
    p_param->count = 0 ;
    t = SYSERROR_ID( p_param->syserror ) ;
    /* check for recognized error code */
    if (t < sizeof(loader_error_string)/sizeof(char*) )
    {
        /* fill in mandatory error message string */
        p_param->strings[SYSCON_ERRORMSG_IDX] = loader_error_string[t] ;
        i++ ;

        /* check for diagnose message */
        if ( sdata[0] != 0 )
        {
            /* fill in optional diagnose message string */
            sprintf( loader_diag_msg,"Line %d, %s", line_number, sdata ) ;
            p_param->strings[SYSCON_DIAGMSG_IDX] = loader_diag_msg ;
            i++ ;
        }

        /* check for hint message */
        if ( loader_error_hint_string[t] != NULL)
        {
            /* fill in optional hint message string */
            p_param->strings[SYSCON_HINTMSG_IDX] = loader_error_hint_string[t] ;
            i++ ;
        }
    }
    p_param->count      = i ;

    return(OK) ;
}
