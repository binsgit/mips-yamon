
/************************************************************************
 *
 *  freq_platform.c
 *
 *  FREQ module
 *
 *  Determines CPU and BUS frequencies and performs optimisations
 *  based on these.
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
#include <mips.h>
#include <init.h>
#include <sys_api.h>
#include <syscon_api.h>
#include <io_api.h>
#include <sysdev.h>
#include <rtc_api.h>
#include <serial_ti16550_api.h>
#include <product.h>
#include <atlas.h>
#include <sead.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

UINT32 sys_cpufreq_hz;
UINT32 sys_busfreq_hz;

/************************************************************************
 *  Static variables
 ************************************************************************/

static UINT32 cycle_per_count;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
rtc_estimate_cpufreq( void );

static void
ti16550_estimate_cpufreq( 
    UINT32 ti16550base );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          rtc_estimate_cpufreq
 ************************************************************************/
static void
rtc_estimate_cpufreq( void )
{
    t_RTC_calendar rtc1, rtc2;
    UINT32         data = 0;

    /* Read the seconds */
    IO_read( SYS_MAJOR_RTC, 0, (UINT8 *)(&rtc1) );

    /* Wait for second to shift */
    do
    {
        IO_read( SYS_MAJOR_RTC, 0, (UINT8 *)(&rtc2) );
    }
    while( rtc1.second == rtc2.second );

    /* Set counter to 0 */
    SYSCON_write( SYSCON_CPU_CP0_COUNT_ID,
		  (void *)(&data),
		  sizeof(UINT32) );

    /* Wait for second to shift */
    do
    {
        IO_read( SYS_MAJOR_RTC, 0, (UINT8 *)(&rtc1) );   
    }
    while( rtc1.second == rtc2.second );

    /* Read counter and adjust for processor */
    SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		 (void *)(&data),
		 sizeof(UINT32) );

    sys_cpufreq_hz = data * cycle_per_count;
}


/************************************************************************
 *
 *                          ti16550_estimate_cpufreq
 *  Description :
 *  -------------
 *
 *  The default tty port is used to estimate the CPU frequency as follows:
 *
 *  2000 characters are printed to the UART (in loopback mode) with
 *  flowcontrol turned off, and the time needed for this is measured. 
 *  The elapsed time is scaled with a constant, to yield the estimated
 *  CPU frequency.
 *  NOTE: This function is called before the serial ports are initalized
 *  for general use. It is therefore not necessarry to restore the previous
 *  TI16550 settings.
 *
 ************************************************************************/
static void
ti16550_estimate_cpufreq( 
    UINT32 ti16550base )
{
    UINT32	uncached_base = UNCACHED(ti16550base);
    INT32	i;
    UINT32      data = 0;

#define UART( base, ofs )    REG32( (base) + 8*(ofs) )

    /* Setup this UART to 38400, no flow control, 1 stop bit, no parity,
     * no auto flowcontrol, loopback mode.
     * Note that we cannot rely on using SERIAL_TI16550_init(), because it
     * contains default settings that may change in the future.
     */
    
    /* Set baudrate to 38400 */
    UART(uncached_base, SERIAL_TI16550_LCR_OFS) = SERIAL_LCR_DLAB;
    UART(uncached_base, SERIAL_TI16550_DLL_OFS) = 0x06;
    UART(uncached_base, SERIAL_TI16550_DLM_OFS) = 0x00;
    UART(uncached_base, SERIAL_TI16550_LCR_OFS) = 0x00;

    /* Init LCR */
    UART(uncached_base, SERIAL_TI16550_LCR_OFS) =
			SERIAL_LCR_DATA8 |
			SERIAL_LCR_STOP1 |
			SERIAL_LCR_PARITYNONE;
    /* Init MCR */
    UART(uncached_base, SERIAL_TI16550_MCR_OFS) =
			SERIAL_MCR_DTR	|
			SERIAL_MCR_RTS	|
			SERIAL_MCR_LOOP	;	/* no autoflow control */
	

    /* Set counter to 0 */
    SYSCON_write( SYSCON_CPU_CP0_COUNT_ID,
		  (void *)(&data),
		  sizeof(UINT32) );

    for (i = 0;  i < 2000;  i++)
    {
	while ( !(UART(uncached_base, SERIAL_TI16550_LSR_OFS)
		  & SERIAL_LSR_THRE) )	; /* poll until tx hold empty */

    	UART(uncached_base, SERIAL_TI16550_THR_OFS) = 'M';
    }

    while ( !(UART(uncached_base, SERIAL_TI16550_LSR_OFS)
	      & SERIAL_LSR_TEMT) ) ; /* poll until transmitter empty (idle)*/

    /* Read counter and adjust for processor */
    SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		 (void *)(&data),
		 sizeof(UINT32) );

    sys_cpufreq_hz = (data * cycle_per_count / 5204) * 10000;

    /* Empty read register, due to loopback */			
    while ( (UART(uncached_base, SERIAL_TI16550_LSR_OFS)
	      & SERIAL_LSR_DR) ) 
	*((volatile INT32*) &i) = UART(uncached_base, SERIAL_TI16550_RBR_OFS);
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          freq_init
 *  Description :
 *  -------------
 *
 *  Main module function called during system initialisation
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
freq_init( void )
{
    UINT32 old_ie;

    /* Determine number of CPU cycles per CPU counter tick */
    SYSCON_read(
        SYSCON_CPU_CYCLE_PER_COUNT_ID,
	&cycle_per_count,
	sizeof(UINT32) );

    /* Estimate CPU clock frequency */
    DISP_STR("FREQCPU");

    /* We disable interrupts in order to avoid precision loss */
    old_ie = sys_disable_int();

    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :
      case PRODUCT_MALTA_ID :

	/* Esitmate CPU frequency using RTC as reference */
        rtc_estimate_cpufreq();

	/* Estimate external bus (SysAD) clock frequency */
        DISP_STR("FREQBUS");
        sys_busfreq_hz = arch_core_estimate_busfreq();

        break;
      case PRODUCT_SEAD_ID  :
      case PRODUCT_SEAD2_ID :

	/*  Estimate CPU frequency using UART baudrate as reference 
	 *  (since there is no RTC).
	 */
        ti16550_estimate_cpufreq(SEAD_TI16C550_DEV0_BASE);

	/* Bus frequency equals CPU frequency */
	sys_busfreq_hz = sys_cpufreq_hz;

        break;
      default :  /* Should never happen */ 
        break;
    }

    /* Restore interrupt enable setting */
    if(old_ie)
        sys_enable_int();
}






