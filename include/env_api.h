
/************************************************************************
 *
 *  env_api.h
 *
 *  API for ENV module
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

#ifndef ENV_API_H
#define ENV_API_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>

/************************************************************************
 *  Definitions
*************************************************************************/

/*  Max length of name of environment variable (not counting 
 *  terminating '\0'.
 */
#define ENV_VAR_MAXLEN_NAME 20

/************************************************************************
 *   ENV, ERROR completion codes
*************************************************************************/

#define ERROR_ENV           0x0000e000 /* for compile time check         */
#define ERROR_ENV_VAR_NAME_LEN  0xe000 /* Too long variable name         */
#define ERROR_ENV_VAR_RO        0xe001 /* Read only variable             */
#define ERROR_ENV_VAR_VALUE     0xe002 /* Illegal value	                 */
#define ERROR_ENV_VAR_OVERFLOW  0xe003 /* Too many environment variables */
#define ERROR_ENV_VAR_VAL_LEN   0xe004 /* Too long variable string       */


/* Environment variables for TTY0/TTY1 parameters */
#define ENV_MODETTY0   "modetty0"
#define ENV_MODETTY1   "modetty1"


/* Environment variable */
typedef struct
{
    char *name;
    char *val;
}
t_env_var;


/* Type of function used to decode/validate environment variable */
typedef   bool (* t_env_decode)( char *raw, void *decoded, UINT32 size );


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          env_init
 *  Description :
 *  -------------
 *
 *  Create System environment variables
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
env_init( void );


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
    char  *raw,
    UINT8 *baudrate,
    UINT8 *parity,
    UINT8 *databits,
    UINT8 *stopbits,
    UINT8 *flowctrl );


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
    UINT8 flowctrl );


/************************************************************************
 *
 *                          env_setup_cpuconfig
 *  Description :
 *  -------------
 *
 *  Setup string for "cpuconfig" environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
env_setup_cpuconfig(
    char		*s,		
    t_sys_cpu_decoded   *decoded );


/************************************************************************
 *
 *                          env_ip_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format x.y.z.w to 4 bytes
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_ip_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


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
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *
 *                          env_decode_bootprot
 *  Description :
 *  -------------
 *
 *  Decode boot protocol
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_decode_bootprot( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
#define PROTOCOL_TFTP	0
#define PROTOCOL_ASC	1
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *
 *                          env_decode_fileprot
 *  Description :
 *  -------------
 *  Decode file protocol
 *
 *  Return values :
 *  ---------------
 *  TRUE -> OK, FALSE -> Failed
 ************************************************************************/
bool 
env_decode_fileprot( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/*  Functions for converting an enumerated value to string describing
 *  the value
 */


/************************************************************************
 *		env_baudrate_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_baudrate_num2s( 
    UINT32 param,
    char   **s );


/************************************************************************
 *		env_databits_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_databits_num2s( 
    UINT32 param,
    char   **s);


/************************************************************************
 *		env_parity_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_parity_num2s( 
    UINT32 param,
    char   **s );


/************************************************************************
 *		env_stopbits_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_stopbits_num2s( 
    UINT32 param,
    char   **s );


/************************************************************************
 *		env_flowctrl_num2s
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_flowctrl_num2s( 
    UINT32 param,
    char   **s );


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
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *		env_databits_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_databits_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *		env_parity_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_parity_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *		env_stopbits_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_stopbits_s2num(
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *		env_flowctrl_s2num
 ************************************************************************/
bool				/* TRUE -> OK, FALSE -> Not OK		*/
env_flowctrl_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *
 *                          env_mac_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format xx.xx.xx.xx.xx.xx to 6 bytes
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
bool 
env_mac_s2num( 
    char *raw,			/* The string				*/
    void *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *
 *                          env_get
 *  Description :
 *  -------------
 *
 *  Lookup environment variable based on name
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> found, FALSE -> not found
 *
 ************************************************************************/
bool
env_get(
    char   *name,		/* Name of environment variable		*/
    char   **raw,		/* Raw text string			*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size );		/* Size of decoded data			*/


/************************************************************************
 *
 *                          env_set
 *  Description :
 *  -------------
 *
 *  Set (and possibly create) environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
env_set(
    char         *name,
    char	 *value,
    UINT8	 attr,
#define ENV_ATTR_USER	0
#define ENV_ATTR_RO	1
#define ENV_ATTR_RW	2
    char         *default_value,
    t_env_decode decode );


/************************************************************************
 *
 *                          env_unset
 *  Description :
 *  -------------
 *
 *  Delete environment variable
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
env_unset(
    char   *name );	/* Name of variable */


/************************************************************************
 *
 *                          env_print_all
 *  Description :
 *  -------------
 *
 *  Display all env. variables
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
env_print_all( void );


/************************************************************************
 *
 *                          env_remove
 *  Description :
 *  -------------
 *
 *  Remove all user and or system variables
 *
 *  Return values :
 *  ---------------
 *
 *  Error code (OK = No error)
 *
 ************************************************************************/
UINT32
env_remove(
    bool user,		/* TRUE -> remove user variables		*/
    bool system );	/* TRUE -> remove system variables		*/


/************************************************************************
 *
 *                          env_get_all
 *  Description :
 *  -------------
 *
 *  Get a pointer to the array of environment variables
 *
 *  Return values :
 *  ---------------
 *
 *  Pointer to array of env. variables
 *
 ************************************************************************/
t_env_var
*env_get_all( void );


/************************************************************************
 *
 *                          env_check
 *  Description :
 *  -------------
 *
 *  Determine whether env. variables have been corrupted (due to flash
 *  corruption). If this is the case, print warning.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> corrupted, FALSE -> not corrupted (normal state)
 *
 ************************************************************************/
bool
env_check(void);


#endif /* #ifndef ENV_API_H */





