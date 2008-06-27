
/************************************************************************
 *
 *  syscon_platform_tty.c
 *
 *  Platform and TTY specific parts of SYSCON module
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
#include <syscon_api.h>
#include <syscon.h>
#include <syserror.h>
#include <mips.h>
#include <sysdev.h>
#include <serial_api.h>
#include <serial_ti16550_api.h>
#include <product.h>
#include <atlas.h>
#include <sead.h>
#include <malta.h>
#include <icta.h>	/* Atlas specific */

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Number of UARTs on board */
static UINT8 uart_count;

/* Select mapping of TTY1 on Malta (SuperIO device or discrete UART)	*/
static bool malta_tty1_on_texas	        = FALSE;  /* Default SuperIO	*/

/* Register size/spacing */
static UINT8  size8			= sizeof(UINT8);
static UINT8  size32			= sizeof(UINT32);
static UINT8  size64			= 2*sizeof(UINT32);

/**** Interrupt lines ****/

/* Atlas   */
static UINT32 intline_atlas_ti16550     = ICTA_INTSTATUS_SER_SHF;
static UINT32 intline_atlas_saa9730     = ICTA_INTSTATUS_CONINTBN_SHF;
/* SEAD    */
static UINT32 intline_sead_ti16550_0    = (S_StatusIM2 - S_StatusIM);
static UINT32 intline_sead_ti16550_1    = (S_StatusIM3 - S_StatusIM);
/* Malta */
static UINT32 intline_malta_ti16550_0   = MALTA_INTLINE_TTY0;
static UINT32 intline_malta_ti16550_1   = MALTA_INTLINE_TTY1;
static UINT32 intline_malta_ti16550_2   = (S_StatusIM4 - S_StatusIM);

/*  Major/minor device number mappings :
 *
 *	       TTY0         TTY1
 *     -----------------------------
 *     Atlas | TI16550(0)   SAA9730
 *     SEAD  | TI16550(0)   TI16550(1)
 *     Malta | TI16550(0)   TI16550(1)
 */

/* Atlas   */
static UINT32 atlas_major_tty0	        = SYS_MAJOR_SERIAL_TI16550;
static UINT32 atlas_minor_tty0	        = SERIAL_MINOR_TI16550_UART0;
static UINT32 atlas_major_tty1	        = SYS_MAJOR_SERIAL_SAA9730;
static UINT32 atlas_minor_tty1	        = SERIAL_MINOR_SAA9730_UART;
/* SEAD    */
static UINT32 sead_major_tty0	        = SYS_MAJOR_SERIAL_TI16550;
static UINT32 sead_minor_tty0	        = SERIAL_MINOR_TI16550_UART0;
static UINT32 sead_major_tty1	        = SYS_MAJOR_SERIAL_TI16550;
static UINT32 sead_minor_tty1	        = SERIAL_MINOR_TI16550_UART1;
/* Malta */
static UINT32 malta_major_tty0		= SYS_MAJOR_SERIAL_TI16550;
static UINT32 malta_minor_tty0		= SERIAL_MINOR_TI16550_UART0;
static UINT32 malta_major_tty1		= SYS_MAJOR_SERIAL_TI16550;
static UINT32 malta_minor_tty1		= SERIAL_MINOR_TI16550_UART1;


/*  Baudrate conversion tables for UART divisor latch.
 *
 *  This table is also used to compute minimum value of BRKRES,
 *  in order to avoid that 10 low bits in a row generates reset.
 *      A NULL sent with 8 bits, even parity generates
 *        Startbit                         Par Stopbit
 *               0  0  0  0  0  0  0  0  0  0  1
 *  At 75 baud, 10 bits equals 133 millisec,  and minimum value of
 *  BRKRES is computed to 'divisor value'>>4 (192 millisec)
 *
 *  Available baudrates are defined in serial_api.h :
 *
 *  SERIAL_BAUDRATE_075_BPS(1)
 *  SERIAL_BAUDRATE_110_BPS(2)
 *  SERIAL_BAUDRATE_150_BPS(3)
 *  SERIAL_BAUDRATE_300_BPS(4)
 *  SERIAL_BAUDRATE_600_BPS(5)
 *  SERIAL_BAUDRATE_1200_BPS(6)
 *  SERIAL_BAUDRATE_1800_BPS(7)
 *  SERIAL_BAUDRATE_2400_BPS(8)
 *  SERIAL_BAUDRATE_4800_BPS(9)
 *  SERIAL_BAUDRATE_7200_BPS(10)
 *  SERIAL_BAUDRATE_9600_BPS(11)
 *  SERIAL_BAUDRATE_14400_BPS(12)
 *  SERIAL_BAUDRATE_19200_BPS(13)
 *  SERIAL_BAUDRATE_38400_BPS(14)
 *  SERIAL_BAUDRATE_57600_BPS(15)
 *  SERIAL_BAUDRATE_115200_BPS(16)
 *  SERIAL_BAUDRATE_230400_BPS(17)
 *  SERIAL_BAUDRATE_460800_BPS(18)
 *  SERIAL_BAUDRATE_921600_BPS(19)
 */

/* TI16550C (Atlas TTY0, SEAD TTY0+TTY1) */
static t_SERIAL_baudrate_div serial_baudrate_div_ti16550c =
{
    SERIAL_ILLEGAL,   /*  0:  not defined   */
              3072,   /*  1:      75 baud   */
              2094,   /*  2:     110 baud   */
              1536,   /*  3:     150 baud   */
               768,   /*  4:     300 baud   */
               384,   /*  5:     600 baud   */
               192,   /*  6:   1.200 baud   */
               128,   /*  7:   1.800 baud   */
                96,   /*  8:   2.400 baud   */
                48,   /*  9:   4.800 baud   */
                32,   /* 10:   7.200 baud   */
                24,   /* 11:   9.600 baud   */
                16,   /* 12:  14.400 baud   */
                12,   /* 13:  19.200 baud   */
                 6,   /* 14:  38.400 baud   */
                 4,   /* 15:  57.600 baud   */
                 2,   /* 16: 115.200 baud   */
                 1,   /* 17: 230.400 baud   */
    SERIAL_ILLEGAL,   /* 18: 460.800 baud   */
    SERIAL_ILLEGAL    /* 19: 921.600 baud   */
};

/* SMSC (Malta TTY0+TTY1, i.e. SMSC SuperIO) */
static t_SERIAL_baudrate_div serial_baudrate_div_smsc =
{
    SERIAL_ILLEGAL,   /*  0:  not defined   */
              1536,   /*  1:      75 baud   */
              1047,   /*  2:     110 baud   */
               768,   /*  3:     150 baud   */
               384,   /*  4:     300 baud   */
	       192,   /*  5:     600 baud   */
	        96,   /*  6:   1.200 baud   */
		64,   /*  7:   1.800 baud   */
		48,   /*  8:   2.400 baud   */
		24,   /*  9:   4.800 baud   */
		16,   /* 10:   7.200 baud   */
		12,   /* 11:   9.600 baud   */
                 8,   /* 12:  14.400 baud   */
                 6,   /* 13:  19.200 baud   */
		 3,   /* 14:  38.400 baud   */
		 2,   /* 15:  57.600 baud   */
		 1,   /* 16: 115.200 baud   */
             32770,   /* 17: 230.400 baud   */
             32769,   /* 18: 460.800 baud   */
    SERIAL_ILLEGAL    /* 19: 921.600 baud   */
};

/* SAA9730 (Atlas TTY1) */
static t_SERIAL_baudrate_div serial_baudrate_div_saa9730 =
{
    SERIAL_ILLEGAL,   /*  0:  not defined   */
            0x0c04,   /*  1:      75 baud   */
            0x0831,   /*  2:     110 baud   */
            0x0601,   /*  3:     150 baud   */
            0x0300,   /*  4:     300 baud   */
            0x017f,   /*  5:     600 baud   */
               191,   /*  6:   1.200 baud   */
               127,   /*  7:   1.800 baud   */
                95,   /*  8:   2.400 baud   */
                47,   /*  9:   4.800 baud   */
                31,   /* 10:   7.200 baud   */
                23,   /* 11:   9.600 baud   */
                15,   /* 12:  14.400 baud   */
                11,   /* 13:  19.200 baud   */
                 5,   /* 14:  38.400 baud   */
                 3,   /* 15:  57.600 baud   */
                 1,   /* 16: 115.200 baud   */
                 0,   /* 17: 230.400 baud   */
    SERIAL_ILLEGAL,   /* 18: 460.800 baud   */
    SERIAL_ILLEGAL    /* 19: 921.600 baud   */
};


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


static UINT32
board_ti16c550c_base0_atlas_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)ATLAS_TI16C550_BASE;
    return OK;
}


static UINT32
board_ti16c550c_base0_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_TI16C550_DEV0_BASE;
    return OK;
}


static UINT32
board_ti16c550c_base0_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)MALTA_UART0ADR;
    return OK;
}


static UINT32
board_ti16c550c_base1_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = (void *)SEAD_TI16C550_DEV1_BASE;
    return OK;
}


static UINT32
board_ti16c550c_base1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(void **)param = 
        malta_tty1_on_texas ?
	    (void *)MALTA_TI16C550 :
	    (void *)MALTA_UART1ADR;

    return OK;
}


static UINT32
com_baudrate_div_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(t_SERIAL_baudrate_div **)param = (t_SERIAL_baudrate_div *)data;
    return OK;
}

static UINT32
com_baudrate_verify_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    /*  No write is actually performed. Object is only used to verify
     *  that baudrate is valid.
     *  Return OK if baudrate is valid, else ERROR_SYSCON_UNKNOWN_PARAM.
     */
    UINT8		  baudrate = *(UINT8 *)param; 
    t_SERIAL_baudrate_div *array   = (t_SERIAL_baudrate_div *)data;
 
    return    
        ( ( baudrate > SERIAL_BAUDRATE_921600_BPS ) ||
          ( baudrate < SERIAL_BAUDRATE_075_BPS    ) ||
	  ( (*array)[baudrate] == SERIAL_ILLEGAL  ) 
	) ?
	    ERROR_SYSCON_UNKNOWN_PARAM : OK;
}


static UINT32
board_reg_size1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        syscon_uint8_read( param, (void *)&size32, size ) :
        syscon_uint8_read( param, (void *)&size8,  size );
}


static UINT32
board_reg_space1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        syscon_uint8_read( param, (void *)&size64, size ) :
        syscon_uint8_read( param, (void *)&size8,  size );
}


static UINT32
board_reg_ic_sead_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01 ?
	syscon_true_read( param, data,  size ) :
	syscon_false_read( param, data, size );
}


static UINT32
board_reg_ic1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        syscon_false_read( param, data, size ) :
	syscon_true_read( param, data,  size );
}


static UINT32
board_baudrate1_div_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
	com_baudrate_div_read( param, (void *)&serial_baudrate_div_ti16550c, size) :
	com_baudrate_div_read( param, (void *)&serial_baudrate_div_smsc, size);
}


static UINT32
board_intline1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        syscon_uint32_read( param, &intline_malta_ti16550_2, size ) :
        syscon_uint32_read( param, &intline_malta_ti16550_1, size );
}


static UINT32
board_bigend1_malta_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        syscon_endian_big_read( param, data, size ) :
        syscon_false_read( param, data, size );
}


static UINT32
board_baudrate1_verify_malta_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    return malta_tty1_on_texas ?
        com_baudrate_verify_write(param, (void *)&serial_baudrate_div_ti16550c, size ) :
        com_baudrate_verify_write(param, (void *)&serial_baudrate_div_smsc, size );
}



/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_arch_tty_init
 *  Description :
 *  -------------
 *
 *  Initialize platform specific part of SYSCON TTY objects.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_arch_tty_init( void )
{
    /*  Number of serial ports supported by YAMON (TTY0/TTY1).
     *  YAMON supports max 2 ports.
     */
    uart_count = 2; /* Same for all currently supported platforms */

    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
    {
	intline_sead_ti16550_0 = SEAD_MSC01_INTLINE_TTY0;
	intline_sead_ti16550_1 = SEAD_MSC01_INTLINE_TTY1;
    }
 
    syscon_register_id_board( SYSCON_BOARD_TI16C550C_BASE0_ID,
			      /* Atlas */
			      board_ti16c550c_base0_atlas_read, NULL,
			      NULL,				NULL,
			      /* SEAD */
			      board_ti16c550c_base0_sead_read,	NULL,
			      NULL,				NULL,
			      /* Malta */
			      board_ti16c550c_base0_malta_read, NULL,
			      NULL,				NULL );

    syscon_register_id_board( SYSCON_BOARD_TI16C550C_BASE1_ID,
			      /* Atlas */
			      NULL,				NULL,
			      NULL,				NULL,
			      /* SEAD */
			      board_ti16c550c_base1_sead_read,  NULL,
			      NULL,				NULL,
			      /* Malta */
			      board_ti16c550c_base1_malta_read, NULL,
			      NULL,				NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_REG_SIZE_ID,
			      /* Atlas (TI16550)   */
			      syscon_uint8_read, &size32,
			      NULL,	         NULL,
			      /* SEAD (TI16550)    */
			      syscon_uint8_read, &size32,
			      NULL,	         NULL,
			      /* Malta (TI16550) */
			      syscon_uint8_read, &size8,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_REG_SIZE_ID,
			      /* Atlas (SAA9730)   */
			      NULL,		          NULL,
			      NULL,			  NULL,
			      /* SEAD (TI16550)    */
			      syscon_uint8_read,          &size32,
			      NULL,			  NULL,
			      /* Malta (TI16550) */
			      board_reg_size1_malta_read, NULL,
			      NULL,	                  NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_REG_SPACING_ID,
			      /* Atlas (TI16550)   */
			      syscon_uint8_read, &size64,
			      NULL,	         NULL,
			      /* SEAD (TI16550)    */
			      syscon_uint8_read, &size64,
			      NULL,	         NULL,
			      /* Malta (TI16550) */
			      syscon_uint8_read, &size8,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_REG_SPACING_ID,
			      /* Atlas (SAA9730)   */
			      NULL,			   NULL,
			      NULL,			   NULL,
			      /* SEAD (TI16550)    */
			      syscon_uint8_read,           &size64,
			      NULL,			   NULL,
			      /* Malta (TI16550) */
			      board_reg_space1_malta_read, NULL,
			      NULL,			   NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_IC_IN_USE_ID,
			      /* Atlas   */
			      syscon_true_read,  NULL,
			      NULL,	         NULL,
			      /* SEAD    */
			      board_reg_ic_sead_read, NULL,
			      NULL,	         NULL,
			      /* Malta */
			      syscon_true_read,  NULL,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_IC_IN_USE_ID,
			      /* Atlas    */
			      syscon_true_read,         NULL,
			      NULL,			NULL,
			      /* SEAD     */
			      board_reg_ic_sead_read,   NULL,
			      NULL,			NULL,
			      /* Malta  */
			      board_reg_ic1_malta_read, NULL,
			      NULL,	                NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_ADJUST_BREAK_ID,
			      /* Atlas   */
			      syscon_true_read,  NULL,
			      NULL,	         NULL,
			      /* SEAD    */
			      syscon_false_read, NULL, 
			      NULL,	         NULL,
			      /* Malta */
			      syscon_true_read,  NULL,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_ADJUST_BREAK_ID,
			      /* Atlas   */
			      syscon_false_read, NULL,
			      NULL,	         NULL,
			      /* SEAD    */
			      syscon_false_read, NULL,
			      NULL,	         NULL,
			      /* Malta */
			      syscon_false_read, NULL,
			      NULL,	         NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_BIGEND_ID,
			      /* Atlas   */
			      syscon_endian_big_read, NULL,
			      NULL,		      NULL,
			      /* SEAD    */
			      syscon_endian_big_read, NULL,
			      NULL,		      NULL,
			      /* Malta (PCI is always little endian) */
			      syscon_false_read,      NULL,
			      NULL,	              NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_BIGEND_ID,
			      /* Atlas (SAA9730 is PCI device and thus little endian) */
			      syscon_false_read,        NULL,
			      NULL,		        NULL,
			      /* SEAD    */
			      syscon_endian_big_read,   NULL,
			      NULL,		        NULL,
			      /* Malta (PCI is always little endian) */
			      board_bigend1_malta_read, NULL,
			      NULL,		        NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_INTLINE_ID,
			      /* Atlas   */
			      syscon_uint32_read, &intline_atlas_ti16550,
			      NULL,	          NULL,
			      /* SEAD    */
			      syscon_uint32_read, &intline_sead_ti16550_0,
			      NULL,	          NULL,
			      /* Malta */
			      syscon_uint32_read, &intline_malta_ti16550_0,
			      NULL,	          NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_INTLINE_ID,
			      /* Atlas   */
			      syscon_uint32_read,	 &intline_atlas_saa9730,
			      NULL,			 NULL,
			      /* SEAD    */
			      syscon_uint32_read,	 &intline_sead_ti16550_1,
			      NULL,	                 NULL,
			      /* Malta */
			      board_intline1_malta_read, NULL,
			      NULL,	                 NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_BAUDRATE_DIV_ID,
			      /* Atlas */
			      com_baudrate_div_read, (void *)&serial_baudrate_div_ti16550c,
			      NULL,		     NULL,
			      /* SEAD */
			      com_baudrate_div_read, (void *)&serial_baudrate_div_ti16550c,
			      NULL,		     NULL,
			      /* Malta */
			      com_baudrate_div_read, (void *)&serial_baudrate_div_smsc,
			      NULL,		     NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_BAUDRATE_DIV_ID,
			      /* Atlas */
			      com_baudrate_div_read, (void *)&serial_baudrate_div_saa9730,
			      NULL,		     NULL,
			      /* SEAD */
			      com_baudrate_div_read, (void *)&serial_baudrate_div_ti16550c,
			      NULL,		     NULL,
			      /* Malta */
			      board_baudrate1_div_malta_read, NULL,
			      NULL,			      NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_BAUDRATE_VERIFY_ID,
			      /* Atlas */
			      NULL,		         NULL,
			      com_baudrate_verify_write, (void *)&serial_baudrate_div_ti16550c,
			      /* SEAD */
			      NULL,		         NULL,
			      com_baudrate_verify_write, (void *)&serial_baudrate_div_ti16550c,
			      /* Malta */
			      NULL,		         NULL,
			      com_baudrate_verify_write, (void *)&serial_baudrate_div_smsc );

    syscon_register_id_board( SYSCON_COM_TTY1_BAUDRATE_VERIFY_ID,
			      /* Atlas */
			      NULL,			 NULL,
			      com_baudrate_verify_write, (void *)&serial_baudrate_div_saa9730,
			      /* SEAD */
			      NULL,			 NULL,
			      com_baudrate_verify_write, (void *)&serial_baudrate_div_ti16550c,
			      /* Malta */
			      NULL,				  NULL,
			      board_baudrate1_verify_malta_write, NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_MAJOR, 
			      /* Atlas   */
			      syscon_uint32_read, &atlas_major_tty0,
			      NULL,	          NULL,
			      /* SEAD    */
			      syscon_uint32_read, &sead_major_tty0,
			      NULL,	          NULL,
			      /* Malta */
			      syscon_uint32_read, &malta_major_tty0,
			      NULL,	          NULL );

    syscon_register_id_board( SYSCON_COM_TTY0_MINOR, 
			      /* Atlas   */
			      syscon_uint32_read, &atlas_minor_tty0,
			      NULL,	          NULL,
			      /* SEAD    */
			      syscon_uint32_read, &sead_minor_tty0,
			      NULL,	          NULL,
			      /* Malta */
			      syscon_uint32_read, &malta_minor_tty0,
			      NULL,	          NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_MAJOR, 
			      /* Atlas   */
			      syscon_uint32_read, &atlas_major_tty1,
			      NULL,	          NULL,
			      /* SEAD    */
			      syscon_uint32_read, &sead_major_tty1,
			      NULL,	          NULL,
			      /* Malta */
			      syscon_uint32_read, &malta_major_tty1,
			      NULL,	          NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_MINOR, 
			      /* Atlas   */
			      syscon_uint32_read, &atlas_minor_tty1,
			      NULL,	          NULL,
			      /* SEAD    */
			      syscon_uint32_read, &sead_minor_tty1,
			      NULL,	          NULL,
			      /* Malta */
			      syscon_uint32_read, &malta_minor_tty1,
			      NULL,	          NULL );

    syscon_register_id_board( SYSCON_COM_TTY1_ON_TEXAS, 
			      /* Atlas   */
			      NULL,	         NULL,
			      NULL,		 NULL,
			      /* SEAD    */
			      NULL,		 NULL,
			      NULL,		 NULL,
			      /* Malta */
			      syscon_bool_read,  (void *)&malta_tty1_on_texas,
			      syscon_bool_write, (void *)&malta_tty1_on_texas );

    syscon_register_generic(  SYSCON_BOARD_UART_COUNT_ID,
			      syscon_uint8_read, (void *)&uart_count,
			      NULL,	         NULL );

}
