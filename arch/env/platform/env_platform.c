
/************************************************************************
 *
 *  env_platform.c
 *
 *  Platform specific part of ENV module (shell environment variables)
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
#include <env_api.h>
#include <env.h>
#include <product.h>
#include <eeprom_api.h>
#include <syserror.h>
#include <stdio.h>
#include <string.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

UINT32 env_ipaddr;
UINT32 env_subnetmask;
UINT32 env_gateway;

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Default settings of env. variables */
static char  *default_ip           = "0.0.0.0";
static char  *default_subnetmask;
static char  *default_gateway;
static char  *default_bootserver;
static char  *default_bootprot     = "tftp";
static char  *default_bootfile     = "";
static char  *default_softendian   = "Hardware";
static char  *env_mac              = "ethaddr";
static char  *env_sn		   = "baseboardserial";

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          ipaddr_ip_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format x.y.z.w to 4 bytes.
 *  In case of success, the result is stored in a global variable.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool 
ipaddr_ip_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    bool rc;

    if (decoded)
    {
        rc = env_ip_s2num(raw, decoded, size);
        if (rc == TRUE)
        {
	    env_ipaddr = *(UINT32*)decoded;
	}
    }
    return env_ip_s2num(raw, &env_ipaddr, sizeof(env_ipaddr));
}


/************************************************************************
 *
 *                          gateway_ip_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format x.y.z.w to 4 bytes.
 *  In case of success, the result is stored in a global variable.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool 
gateway_ip_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    bool rc;

    if (decoded)
    {
        rc = env_ip_s2num(raw, decoded, size);
        if (rc == TRUE)
        {
	    env_gateway = *(UINT32*)decoded;
	}
    }
    return env_ip_s2num(raw, &env_gateway, sizeof(env_gateway));
}


/************************************************************************
 *
 *                          subnetmask_ip_s2num
 *  Description :
 *  -------------
 *
 *  Decode a string of format x.y.z.w to 4 bytes.
 *  In case of success, the result is stored in a global variable.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/
static bool 
subnetmask_ip_s2num( 
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    bool rc;

    if (decoded)
    {
        rc = env_ip_s2num(raw, decoded, size);
        if (rc == TRUE)
        {
	    env_subnetmask = *(UINT32*)decoded;
	}
	return rc;
    }
    return env_ip_s2num(raw, &env_subnetmask, sizeof(env_subnetmask));
}


/************************************************************************
 *
 *                          validate_softendian
 *  Description :
 *  -------------
 *
 *  Ensure that only three options are valid for the user endian preference
 *  and be intelligent about input and return values: if there's not enough
 *  space for an ASCII string, use 0 for Little, 1 for Big endian, and 2 to
 *  take the value of S5-2 as the desired endianess (ie: do nothing)
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, FALSE -> Failed
 *
 ************************************************************************/

static bool
validate_softendian(
    char   *raw,		/* The string				*/
    void   *decoded,		/* Decoded data				*/
    UINT32 size )		/* Size of decoded data			*/
{
    char   *ptr;
    UINT32 se=0;
    UINT32 endian=3;

    if (!raw)
        return FALSE; 

    if ( SYSCON_read( SYSCON_BOARD_SOFTEND_VALID_ID,
         (void *)&se, sizeof(UINT32) ) != OK )
        return FALSE;

    if (strlen(raw) == 1)
    {
        switch (raw[0]) {
            case '0':
                endian=0;
                break;

	    case '1':
	        endian=1;
                break;

            case '2':
                endian=2;
                break;
	}
    }
    else
    {
        if ((strlen(raw) != 3) && (strlen(raw) != 6) && (strlen(raw) != 8))
            return FALSE;

	ptr=&raw[1];
        switch (raw[0]) {
	    case 'L':
	    case 'l':
                if (strcmp(ptr, "ittle") == 0)
                    endian = 0;
	        break;

	    case 'B':
	    case 'b':
                if (strcmp(ptr, "ig") == 0)
                    endian = 1;
	        break;

	    case 'H':
	    case 'h':
                if (strcmp(ptr, "ardware") == 0)
                    endian = 2;
	        break;
	}
    }

    if (endian > 2)
        return FALSE;

    if ((se == 0) && (endian != 2))
        return FALSE;

    if (decoded)
    {
	if (!size)
	    return FALSE;

        if (size >= 9)
	{
	    switch (endian) {
	        case 0:
	            sprintf(decoded, "Little");
		    break;
	        case 1:
	            sprintf(decoded, "Big");
		    break;
	        case 2:
	            sprintf(decoded, "Hardware");
		    break;
            }
	}
	else if (sizeof(UINT32) == size)
	{
	    *(UINT32 *)decoded = endian;
	}
    }

    return TRUE;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          env_setup_env_board
 *  Description :
 *  -------------
 *
 *  Create board specific system environment variables
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> OK, else FALSE
 *
 ************************************************************************/
bool
env_setup_env_board(
    bool default_switch )
{
    char       msg[40];
    char       *raw;
    t_sn_ascii sn;
    t_mac_addr mac_addr;
    UINT32     rc = TRUE;
    UINT32     se = 0;

    default_subnetmask = default_ip;
    default_gateway    = default_ip;
    default_bootserver = default_ip;

    switch( sys_platform )
    {
      case PRODUCT_MALTA_ID :
	/* Soft Endianness */
	if( SYSCON_read( SYSCON_BOARD_SOFTEND_VALID_ID,
			 (void *)&se,
			 sizeof(UINT32) ) == OK )
	{
            if( default_switch || !env_get( "softendian", &raw, NULL, 0 ) )
                raw = default_softendian;
            if( env_set( "softendian", raw, ENV_ATTR_RW, 
	        default_softendian, validate_softendian ) != OK )
            {
	        rc = FALSE;
            }
	}
	else
	{
	    if( env_unset( "softendian" ) != OK )
	        rc = FALSE;
	}
	/* NB: no break here - the following code is common */

      case PRODUCT_ATLASA_ID :

	/* MAC address */
	if( SYSCON_read( SYSCON_COM_EN0_MAC_ADDR_ID,
			 (void *)&mac_addr,
			 sizeof(t_mac_addr) ) == OK )
	{
            sprintf( msg, "%02x.%02x.%02x.%02x.%02x.%02x",
		          mac_addr[0],
		          mac_addr[1],
		          mac_addr[2],
		          mac_addr[3],
		          mac_addr[4],
		          mac_addr[5] );

            if( env_set( env_mac, msg, ENV_ATTR_RO, NULL,
			 env_mac_s2num ) != OK )
	    {
		rc = FALSE;
	    }
        }
	else
	{
	    if( env_unset( env_mac ) != OK )
	        rc = FALSE;
        }

        /* Serial number */
	if( SYSCON_read( SYSCON_BOARD_SN_ID,
			 (void *)&sn,
			 sizeof(t_sn_ascii) ) == OK )
	{
            if( env_set( env_sn, (char *)sn, ENV_ATTR_RO, NULL, NULL ) != OK )
	        rc = FALSE;
        }
        else	
	{
	    if( env_unset( env_sn ) != OK )
	        rc = FALSE;
        }

	/* IP address */
        if( default_switch || !env_get( "ipaddr", &raw, NULL, 0 ) )
	     raw = default_ip;		  /* Create new */
        if( env_set( "ipaddr", raw, ENV_ATTR_RW, 
		     default_ip, ipaddr_ip_s2num ) != OK )
	{
	    rc = FALSE;
	}

	/* Subnet mask */
        if( default_switch || !env_get( "subnetmask", &raw, NULL, 0 ) )
	     raw = default_subnetmask;	  /* Create new */
        if( env_set( "subnetmask", raw, ENV_ATTR_RW, 
		     default_subnetmask, subnetmask_ip_s2num ) != OK )
	{
	    rc = FALSE;
	}

	/* Default gateway */
        if( default_switch || !env_get( "gateway", &raw, NULL, 0 ) )
	     raw = default_gateway;	  /* Create new */
        if( env_set( "gateway", raw, ENV_ATTR_RW, 
		     default_gateway, gateway_ip_s2num ) != OK )
	{
	    rc = FALSE;
	}

	/* Default boot protocol */
	if( default_switch || !env_get( "bootprot", &raw, NULL, 0 ) )
	    raw = default_bootprot;       /* Create new */
        if( env_set( "bootprot", raw, ENV_ATTR_RW, 
		     default_bootprot, env_decode_bootprot ) != OK )
	{
	    rc = FALSE;
	}

	/* Default boot server (TFTP) */
        if( default_switch || !env_get( "bootserver", &raw, NULL, 0 ) )
            raw = default_bootserver;	  /* Create new */
        if( env_set( "bootserver", raw, ENV_ATTR_RW, 
		     default_bootserver, env_ip_s2num ) != OK )
	{
	    rc = FALSE;
	}

	/* Default boot file */
	if( default_switch || !env_get( "bootfile", &raw, NULL, 0 ) )
	    raw = default_bootfile;       /* Create new */
        if( env_set( "bootfile", raw, ENV_ATTR_RW, 
		     default_bootfile, NULL ) != OK )
	{
	    rc = FALSE;
	}

	break;

      case PRODUCT_SEAD_ID  :
      case PRODUCT_SEAD2_ID :
      
        break;

      default :
        break;
    }

    return rc;
}


/************************************************************************
 *
 *                          env_decode_bootprot
 *  Description :
 *  -------------
 *
 *  Decode boot protocol (currently tftp or asc)
 *  (not all protocols legal on all platforms)
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
    UINT32 size )		/* Size of decoded data			*/
{
    if( decoded && (size != sizeof(UINT32)) )
        return FALSE;

    if( strcmp( raw, "tftp" ) == 0 )
    {
        if( (sys_platform == PRODUCT_SEAD_ID) ||
            (sys_platform == PRODUCT_SEAD2_ID) )
	{
	    return FALSE;
        }
        else
	{
	    if( decoded )
                *(UINT32 *)decoded = PROTOCOL_TFTP;
	    return TRUE;
        }
    }
    else if( strcmp( raw, "asc" ) == 0 )
    {
        if( decoded )
            *(UINT32 *)decoded = PROTOCOL_ASC;
	return TRUE;
    }
    else
        return FALSE;
}

