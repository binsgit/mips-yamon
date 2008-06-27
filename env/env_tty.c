
/************************************************************************
 *
 *  env_tty.c
 *
 *  TTY specific parts of ENV module
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
#include <sys_api.h>
#include <syscon_api.h>
#include <serial_api.h>
#include <env_api.h>
#include <string.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    UINT8 number;
    char  *s;
}
t_num_string_pair;

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Default values of environment variables */
static char   *default_modetty      = DEFAULT_SERIAL_ENV;
static char   *default_bootserport  = DEFAULT_BOOTPORT_ENV;

/*  Mappings between enumerated values and strings describing serial
 *  port parameters.
 */

static t_num_string_pair baudrate_num2s_array[] =
{
    { SERIAL_BAUDRATE_075_BPS,    "75"      },
    { SERIAL_BAUDRATE_110_BPS,	  "110"	    }, 
    { SERIAL_BAUDRATE_150_BPS,	  "150"	    },
    { SERIAL_BAUDRATE_300_BPS,	  "300"	    },
    { SERIAL_BAUDRATE_600_BPS,	  "600"	    },
    { SERIAL_BAUDRATE_1200_BPS,	  "1200"    },
    { SERIAL_BAUDRATE_1800_BPS,	  "1800"    },
    { SERIAL_BAUDRATE_2400_BPS,	  "2400"    },
    { SERIAL_BAUDRATE_4800_BPS,	  "4800"    },
    { SERIAL_BAUDRATE_7200_BPS,	  "7200"    },
    { SERIAL_BAUDRATE_9600_BPS,	  "9600"    },
    { SERIAL_BAUDRATE_14400_BPS,  "14400"   },
    { SERIAL_BAUDRATE_19200_BPS,  "19200"   },
    { SERIAL_BAUDRATE_38400_BPS,  "38400"   },
    { SERIAL_BAUDRATE_57600_BPS,  "57600"   },     
    { SERIAL_BAUDRATE_115200_BPS, "115200"  },
    { SERIAL_BAUDRATE_230400_BPS, "230400"  },
    { SERIAL_BAUDRATE_460800_BPS, "460800"  }
#if 0
    /* Not supported */
    { SERIAL_BAUDRATE_921600_BPS, "921600"  }
#endif    
};
#define BAUDRATE_NUM2S_COUNT \
	    (sizeof(baudrate_num2s_array) / sizeof(t_num_string_pair))

static t_num_string_pair databits_num2s_array[] =
{
    { SERIAL_DATABITS_7, "7" },
    { SERIAL_DATABITS_8, "8" }
};
#define DATABITS_NUM2S_COUNT \
	    (sizeof(databits_num2s_array) / sizeof(t_num_string_pair))

static t_num_string_pair parity_num2s_array[] =
{
    { SERIAL_PARITY_NONE, "n" },
    { SERIAL_PARITY_ODD,  "o" },
    { SERIAL_PARITY_EVEN, "e" }
};
#define PARITY_NUM2S_COUNT \
	    (sizeof(parity_num2s_array) / sizeof(t_num_string_pair))

static t_num_string_pair flowctrl_num2s_array[] =
{
    { SERIAL_FLOWCTRL_HARDWARE, "hw"      },
    { SERIAL_FLOWCTRL_NONE,	"none"    }
#if 0
    /* Not supported yet */
    { SERIAL_FLOWCTRL_XON_XOFF, "xonxoff" }
#endif
};
#define FLOWCTRL_NUM2S_COUNT \
	    (sizeof(flowctrl_num2s_array) / sizeof(t_num_string_pair))

static t_num_string_pair stopbits_num2s_array[] =
{
    { SERIAL_STOPBITS_10, "1"   },
    { SERIAL_STOPBITS_20, "2"   }
#if 0
    /* Not supported */
    { SERIAL_STOPBITS_15, "1.5" }
#endif
};
#define STOPBITS_NUM2S_COUNT \
	    (sizeof(stopbits_num2s_array) / sizeof(t_num_string_pair))

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static bool
search_s2num(
    char               *raw,
    void               *decoded,
    UINT32             size,
    t_num_string_pair *pairs,
    UINT32             count );

static bool
search_num2s( 
    UINT8	       param,
    char	       **s,
    t_num_string_pair  *pairs,
    UINT32             count );

static bool 
modetty_s2num(
    bool   verify_baud,
    UINT32 verify_id,
    char   *raw,
    void   *decoded,
    UINT32 size );

static bool 
modetty_s2num0(
    char   *raw,
    void   *decoded,
    UINT32 size );

static bool 
modetty_s2num1(
    char   *raw,
    void   *decoded,
    UINT32 size );

static void
register_initial( void );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          search_s2num
 *  Description :
 *  -------------
 *
 *  Search for string and return corresponding enumerated value (if
 *  buffer is not NULL and size matches).
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE if string is found, else FALSE
 *
 ************************************************************************/
static bool
search_s2num(
    char               *raw, 	 /* The string				*/
    void               *decoded, /* Decoded data			*/
    UINT32             size,	 /* Size of decoded data		*/
    t_num_string_pair *pairs,	 /* Array holding the str/number pairs  */
    UINT32             count )   /* Array size				*/
{
    UINT32 i;

    if( !raw ) return FALSE;

    for( i=0;
         (i < count) &&
	 (strcmp( raw, pairs[i].s ) != 0);
	 i++ );

    if( i == count ) return FALSE;

    if( decoded )
    {
        if( size != sizeof(UINT8) ) return FALSE;
	*(UINT8 *)decoded = pairs[i].number;
    }

    return TRUE;
}


/************************************************************************
 *
 *                          search_num2s
 *  Description :
 *  -------------
 *
 *  Search for enumerated value and return corresponding string.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE if value is found, else FALSE
 *
 ************************************************************************/
static bool
search_num2s( 
    UINT8	       param,	 /* Numeric value of parameter		*/
    char	       **s,	 /* env. string representing data	*/
    t_num_string_pair  *pairs,	 /* Array holding the str/number pairs  */
    UINT32             count )   /* Array size				*/
{
    UINT32 i;

    if( !s ) return FALSE;

    for( i=0;
         (i < count) &&
	 (param != pairs[i].number);
	 i++ );

    if( i == count ) return FALSE;

    *s = pairs[i].s;

    return TRUE;
}



/************************************************************************
 *
 *                          modetty_s2num/
 *			    modetty_s2num0/
 *			    modetty_s2num1/			    
 *  Description :
 *  -------------
 *
 *  Convert string describing serial port configuration (environment 
 *  variable format) to UINT32 containing the coded values (if buffer
 *  for this is available and format is correct)
 *
 *  modetty_s2num0/1 are the functions registered for TTY0 and TTY1.
 *  They call upon the common function modetty_s2num in order to
 *  perform the operation. Only difference is the SYSCON object used
 *  to verify the baudrate
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE if conversion is possible, else FALSE
 *
 ************************************************************************/

static bool 
modetty_s2num0( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return modetty_s2num( TRUE,
			  SYSCON_COM_TTY0_BAUDRATE_VERIFY_ID, 
			  raw, 
			  decoded, 
			  size );
}

static bool 
modetty_s2num1( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return modetty_s2num( TRUE,
			  SYSCON_COM_TTY1_BAUDRATE_VERIFY_ID, 
			  raw, 
			  decoded, 
			  size );
}

static bool 
modetty_s2num( 
    bool   verify_baud,		/* TRUE -> Verify baudrate		*/
    UINT32 verify_id,		/* SYSCON ID used to verify baudrate	*/
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
#define NUMTOKENS 5
    char  *myarg[NUMTOKENS];
    char  myraw[24];   /* "921600,n,8,1,xonxoff" (max) */
    char  c;
    int   n;
    char  *p;
    UINT8 baudrate, parity, databits, stopbits, flowctrl;

    /* Check that we have the string */
    if( !raw )
        return FALSE;

    /* Check size of variable for decoded data */
    if( decoded && (size != sizeof(UINT32)) )
        return FALSE;

    if( strlen(raw) > sizeof(myraw)-1 )
        return FALSE;

    /* make a local copy of string data */
    strcpy(myraw, raw);

    /* Get exactly NUMTOKENS ','-separated tokens from the ascii input */
    myarg[0] = myraw;                           /* first token is easy */
    for (n=0, p = myraw; (c= *p); p++)
    {
        if (c == ',')
        {
            if (n == NUMTOKENS-1) return FALSE; /* too many tokens */
            *p = '\0';                          /* token end   */
            myarg[++n] = p+1;                   /* next token */
        }
    }
    if (n != NUMTOKENS-1) return FALSE;         /* too few tokens */

    /*  Check validity of the 5 tokens and extract environment value
     *  for each.
     */


    if( !env_baudrate_s2num( myarg[0], &baudrate, sizeof(UINT8)) )
        return FALSE;

    /*  Port specific check.
     *  If the following write operation fails, the baudrate is
     *  not supported.
     */
    if( verify_baud &&
        (SYSCON_write( verify_id, &baudrate, sizeof(UINT8) ) != OK) )
    {
        return FALSE;
    }

    if( !env_parity_s2num(   myarg[1], &parity, sizeof(UINT8)) )
        return FALSE;

    if( !env_databits_s2num( myarg[2], &databits, sizeof(UINT8)) )
        return FALSE;

    if( !env_stopbits_s2num( myarg[3], &stopbits, sizeof(UINT8)) )
        return FALSE;

    if( !env_flowctrl_s2num( myarg[4], &flowctrl, sizeof(UINT8)) )
        return FALSE;

    if (decoded)
    {
        /* Construct environment value (byte) */
        *(UINT32 *)decoded = 
	     baudrate                |
             (parity<<8)             |
             (databits<<16)          |
             ((stopbits & 0x0f)<<24) |
             ((flowctrl & 0x0f)<<28);
    }

    return TRUE;
#undef NUMTOKENS
}


/************************************************************************
 *
 *                          register_initial
 *  Description :
 *  -------------
 *
 *  Determine initial serial port settings and register these with SYSCON
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
static void
register_initial( void )
{
    UINT8 baudrate0, parity0, databits0, stopbits0, flowctrl0;
    UINT8 baudrate1, parity1, databits1, stopbits1, flowctrl1;
    char  *raw;
    bool  init_from_env;

    SYSCON_read( SYSCON_BOARD_INIT_BASED_ON_ENV_ID,
		 (void *)&init_from_env, sizeof(bool) );

    if( init_from_env )
    {
        /* Get initial settings from environment variables */

	/* TTY 0 */
        if( !env_get( ENV_MODETTY0, &raw, NULL, 0 ) )
	    raw = default_modetty;

	env_modetty_string2parts( raw,
                                  &baudrate0,
				  &parity0,
				  &databits0,
				  &stopbits0,
				  &flowctrl0 );

	/* TTY 1 */
        if( !env_get( ENV_MODETTY1, &raw, NULL, 0 ) )
	    raw = default_modetty;	

	env_modetty_string2parts( raw,
                                  &baudrate1,
				  &parity1,
				  &databits1,
				  &stopbits1,
				  &flowctrl1 );
    }
    else
    {
        /* Fixed initial settings */

        baudrate0 = baudrate1 = DEFAULT_BAUDRATE;
        parity0   = parity1   = DEFAULT_PARITY;
	databits0 = databits1 = DEFAULT_DATABITS;
	stopbits0 = stopbits1 = DEFAULT_STOPBITS,
	flowctrl0 = flowctrl1 = DEFAULT_FLOWCTRL;
    }

    /**** Store initial settings ****/

    /* TTY0 */

    SYSCON_write( SYSCON_COM_TTY0_BAUDRATE_ID,
		  (void *)&baudrate0, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY0_PARITY_ID,
		  (void *)&parity0, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY0_DATABITS_ID,
		  (void *)&databits0, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY0_STOPBITS_ID,
		  (void *)&stopbits0, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY0_FLOWCTRL_ID,
		  (void *)&flowctrl0, sizeof(UINT8) );

    /* TTY 1 */

    SYSCON_write( SYSCON_COM_TTY1_BAUDRATE_ID,
		  (void *)&baudrate1, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY1_PARITY_ID,
		  (void *)&parity1, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY1_DATABITS_ID,
		  (void *)&databits1, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY1_STOPBITS_ID,
		  (void *)&stopbits1, sizeof(UINT8) );

    SYSCON_write( SYSCON_COM_TTY1_FLOWCTRL_ID,
		  (void *)&flowctrl1, sizeof(UINT8) );
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/*  Functions for converting an enumerated value to string describing
 *  the value
 */


/************************************************************************
 *		env_baudrate_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_baudrate_num2s( 
    UINT32 param,
    char   **s )
{  
    return search_num2s( param, s, 
		         baudrate_num2s_array, 
		         BAUDRATE_NUM2S_COUNT );
}


/************************************************************************
 *		env_databits_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_databits_num2s( 
    UINT32 param,
    char   **s)
{
    return search_num2s( param, s,
		         databits_num2s_array,
			 DATABITS_NUM2S_COUNT );
}


/************************************************************************
 *		env_parity_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_parity_num2s( 
    UINT32 param,
    char   **s )
{
    return search_num2s( param, s,
		         parity_num2s_array,
			 PARITY_NUM2S_COUNT );
}


/************************************************************************
 *		env_stopbits_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_stopbits_num2s( 
    UINT32 param,
    char   **s )
{
    return search_num2s( param, s,
		         stopbits_num2s_array,
			 STOPBITS_NUM2S_COUNT );
}


/************************************************************************
 *		env_flowctrl_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_flowctrl_num2s( 
    UINT32 param,
    char   **s )
{
    return search_num2s( param, s,
		         flowctrl_num2s_array,
			 FLOWCTRL_NUM2S_COUNT );
}


/*  Functions for converting string describing a serial port setting
 *  to corresponding enumerated value.
 */

/************************************************************************
 *		env_baudrate_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_baudrate_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return search_s2num( raw, decoded, size, 
		         baudrate_num2s_array,
			 BAUDRATE_NUM2S_COUNT );
}


/************************************************************************
 *		env_databits_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_databits_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return search_s2num( raw, decoded, size, 
			 databits_num2s_array,
			 DATABITS_NUM2S_COUNT );
}


/************************************************************************
 *		env_parity_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_parity_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return search_s2num( raw, decoded, size, 
			 parity_num2s_array,
			 PARITY_NUM2S_COUNT );
}


/************************************************************************
 *		env_stopbits_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_stopbits_s2num(
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return search_s2num( raw, decoded, size, 
			 stopbits_num2s_array,
			 STOPBITS_NUM2S_COUNT );
}


/************************************************************************
 *		env_flowctrl_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_flowctrl_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    return search_s2num( raw, decoded, size, 
			 flowctrl_num2s_array,
			 FLOWCTRL_NUM2S_COUNT );
}


/************************************************************************
 *
 *                          env_setup_env_tty
 *  Description :
 *  -------------
 *
 *  Create TTY specific system environment variables
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, else FALSE
 *
 ************************************************************************/
bool
env_setup_env_tty(
    bool default_switch )
{
    /* Setup system environment variables if this was not done
     * at a previous run.
     */

    UINT8  uart_count;
    char   *raw;
    bool   rc = TRUE;

    /* Determine the number of serial ports */
    SYSCON_read( SYSCON_BOARD_UART_COUNT_ID, 
		 (void *)&uart_count, 
		 sizeof(UINT8) );

    /* Modetty */

    if( uart_count > 0 )
    {
        if( default_switch || !env_get( ENV_MODETTY0, &raw, NULL, 0 ) )
            raw = default_modetty;		  /* Create new */

        if( env_set( ENV_MODETTY0, raw, ENV_ATTR_RW, 
	             default_modetty, modetty_s2num0 ) != OK )
        {
            rc = FALSE;
        }
    }

    if( uart_count > 1 )
    {
        if( default_switch || !env_get( ENV_MODETTY1, &raw, NULL, 0 ) )
            raw = default_modetty;		  /* Create new */

        if( env_set( ENV_MODETTY1, raw, ENV_ATTR_RW, 
	     	     default_modetty, modetty_s2num1 ) != OK )
        {
            rc = FALSE;
        }
    }

    /* Default boot port (ASC) */
    if( default_switch || !env_get( "bootserport", &raw, NULL, 0 ) )
	raw = default_bootserport;    /* Create new */
    if( env_set( "bootserport", raw, ENV_ATTR_RW, 
		 default_bootserport, env_decode_bootserport ) != OK )
    {
        rc = FALSE;
    }

    /* Initial settings */
    register_initial();
    
    return rc;
}


/************************************************************************
 *
 *                          env_modetty_string2parts
 *  Description :
 *  -------------
 *
 *  Convert string containing serial port settings (environment
 *  variable format) to values.
 *
 *  Return values :
 *  ---------------
 *
 *  Always TRUE
 *
 ************************************************************************/
bool
env_modetty_string2parts(
    char  *raw,		/* String with settings (e.g. "9600,n,8,1,hw" ) */
    UINT8 *baudrate,
    UINT8 *parity,
    UINT8 *databits,
    UINT8 *stopbits,
    UINT8 *flowctrl )
{
    UINT32 mode;

    modetty_s2num( FALSE, 0, raw, &mode, sizeof(UINT32) );

    *baudrate = mode & 0xff;
    *parity   = (mode >>  8) & 0xff;
    *databits = (mode >> 16) & 0xff;
    *stopbits = (mode >> 24) & 0x0f;
    *flowctrl = (mode >> 28) & 0x0f;

    return TRUE;
}


/************************************************************************
 *
 *                          env_modetty_parts2string
 *  Description :
 *  -------------
 *
 *  Converts serial port settings to string containing the serial port
 *  configuration. Returns pointer to this string in parameter **s.
 *
 *  Return values :
 *  ---------------
 *
 *  Always TRUE
 *
 ************************************************************************/
bool
env_modetty_parts2string(
    char  **s,		
    UINT8 baudrate,
    UINT8 parity,
    UINT8 databits,
    UINT8 stopbits,
    UINT8 flowctrl )
{
    char         *myp;
    static char  myraw[24];   /* "921600,n,8,1,xonxoff" (max) */
    int          n;
    
    /* Contruct the string from its serial parts */
    if ( !env_baudrate_num2s( baudrate, &myp ) ) return FALSE;
    n = strlen(myp) + 1;
    if (n > sizeof(myraw)-1) return FALSE;
    strcpy(myraw, myp);
    strcat(myraw, ",");

    if ( !env_parity_num2s( parity, &myp ) ) return FALSE;
    n += strlen(myp) + 1;
    if (n > sizeof(myraw)-1) return FALSE;
    strcat(myraw, myp);
    strcat(myraw, ",");

    if ( !env_databits_num2s( databits, &myp ) ) return FALSE;
    n += strlen(myp) + 1;
    if (n > sizeof(myraw)-1) return FALSE;
    strcat(myraw, myp);
    strcat(myraw, ",");

    if ( !env_stopbits_num2s( stopbits, &myp ) ) return FALSE;
    n += strlen(myp)+1;
    if (n > sizeof(myraw)-1) return FALSE;
    strcat(myraw, myp);
    strcat(myraw, ",");

    if ( !env_flowctrl_num2s( flowctrl, &myp ) ) return FALSE;
    n += strlen(myp);
    if (n > sizeof(myraw)-1) return FALSE;
    strcat(myraw, myp);

    *s = myraw;

    return TRUE;
}


/************************************************************************
 *
 *                          env_decode_bootserport
 *  Description :
 *  -------------
 *
 *  Decode boot serial port
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_decode_bootserport(
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    if( decoded && (size != sizeof(UINT32)) )
        return FALSE;

    if( strcmp( raw, "tty0" ) == 0 )
    {
        if( decoded )
            *(UINT32 *)decoded = PORT_TTY0;
	return TRUE;
    }
    else if( strcmp( raw, "tty1" ) == 0 )
    {
        if( decoded )
            *(UINT32 *)decoded = PORT_TTY1;
	return TRUE;
    }
    else
        return FALSE;
}

