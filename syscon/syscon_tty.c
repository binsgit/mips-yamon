
/************************************************************************
 *
 *  syscon_tty.c
 *
 *  Non platform specific TTY parts of SYSCON module
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
#include <syscon_api.h>
#include <syscon.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Parameters for registered functions */
static UINT8 tty0 = 0;
static UINT8 tty1 = 1;

/* Current values of TTY0(0)/TTY1(1) parameters */
static UINT8  baudrate[2];
static UINT8  databits[2];
static UINT8  parity[2];
static UINT8  stopbits[2];
static UINT8  flowctrl[2];

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *  Implementation : Static functions registered for handling particular 
 *  SYSCON objects for particular platform(s)
 ************************************************************************/


/************************************************************************
 *  com_tty_baudrate_generic_read
 ************************************************************************/
static UINT32
com_tty_baudrate_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = baudrate[*(UINT8 *)data];
    return OK;
}


/************************************************************************
 *  com_tty_baudrate_generic_write
 ************************************************************************/
static UINT32
com_tty_baudrate_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    baudrate[*(UINT8 *)data] = *(UINT8 *)param;
    return OK;
}


/************************************************************************
 *  com_tty_databits_generic_read
 ************************************************************************/
static UINT32
com_tty_databits_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = databits[*(UINT8 *)data];
    return OK;
}


/************************************************************************
 *  com_tty_databits_generic_write
 ************************************************************************/
static UINT32
com_tty_databits_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    databits[*(UINT8 *)data] = *(UINT8 *)param;
    return OK;
}


/************************************************************************
 *  com_tty_parity_generic_read
 ************************************************************************/
static UINT32
com_tty_parity_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = parity[*(UINT8 *)data];
    return OK;
}


/************************************************************************
 *  com_tty_parity_generic_write
 ************************************************************************/
static UINT32
com_tty_parity_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    parity[*(UINT8 *)data] = *(UINT8 *)param;
    return OK;
}


/************************************************************************
 *  com_tty_stopbits_generic_read
 ************************************************************************/
static UINT32
com_tty_stopbits_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = stopbits[*(UINT8 *)data];
    return OK;
}


/************************************************************************
 *  com_tty_stopbits_generic_write
 ************************************************************************/
static UINT32
com_tty_stopbits_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    stopbits[*(UINT8 *)data] = *(UINT8 *)param;
    return OK;
}


/************************************************************************
 *  com_tty_flowctrl_generic_read
 ************************************************************************/
static UINT32
com_tty_flowctrl_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT8 *)param = flowctrl[*(UINT8 *)data];
    return OK;
}


/************************************************************************
 *  com_tty_flowctrl_generic_write
 ************************************************************************/
static UINT32
com_tty_flowctrl_generic_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    flowctrl[*(UINT8 *)data] = *(UINT8 *)param;
    return OK;
}



/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_generic_tty_init
 *  Description :
 *  -------------
 *
 *  Initialize generic TTY parts of SYSCON module.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_generic_tty_init( void )
{
    syscon_register_generic( SYSCON_COM_TTY0_BAUDRATE_ID,
			     com_tty_baudrate_generic_read,
			     (void *)&tty0,
			     com_tty_baudrate_generic_write,
			     (void *)&tty0 );

    syscon_register_generic( SYSCON_COM_TTY0_DATABITS_ID,
			     com_tty_databits_generic_read,
			     (void *)&tty0,
			     com_tty_databits_generic_write,
			     (void *)&tty0 );

    syscon_register_generic( SYSCON_COM_TTY0_PARITY_ID,
			     com_tty_parity_generic_read,
			     (void *)&tty0,
			     com_tty_parity_generic_write,
			     (void *)&tty0 );

    syscon_register_generic( SYSCON_COM_TTY0_STOPBITS_ID,
			     com_tty_stopbits_generic_read,
			     (void *)&tty0,
			     com_tty_stopbits_generic_write,
			     (void *)&tty0 );

    syscon_register_generic( SYSCON_COM_TTY0_FLOWCTRL_ID,
			     com_tty_flowctrl_generic_read,
			     (void *)&tty0,
			     com_tty_flowctrl_generic_write,
			     (void *)&tty0 );

    syscon_register_generic( SYSCON_COM_TTY1_BAUDRATE_ID,
			     com_tty_baudrate_generic_read,
			     (void *)&tty1,
			     com_tty_baudrate_generic_write,
			     (void *)&tty1 );

    syscon_register_generic( SYSCON_COM_TTY1_DATABITS_ID,
			     com_tty_databits_generic_read,
			     (void *)&tty1,
			     com_tty_databits_generic_write,
			     (void *)&tty1 );

    syscon_register_generic( SYSCON_COM_TTY1_PARITY_ID,
			     com_tty_parity_generic_read,
			     (void *)&tty1,
			     com_tty_parity_generic_write,
			     (void *)&tty1 );

    syscon_register_generic( SYSCON_COM_TTY1_STOPBITS_ID,
			     com_tty_stopbits_generic_read,
			     (void *)&tty1,
			     com_tty_stopbits_generic_write,
			     (void *)&tty1 );

    syscon_register_generic( SYSCON_COM_TTY1_FLOWCTRL_ID,
			     com_tty_flowctrl_generic_read,
			     (void *)&tty1,
			     com_tty_flowctrl_generic_write,
			     (void *)&tty1 );
}



