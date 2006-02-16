
/************************************************************************
 *
 *      rtc.c
 *
 *      The 'RTC' module implements a device driver 
 *      for MC146818A/DS1685/DS1687 compatible RTCs.
 *
 *      IO services are handled the following way :
 *
 *        1) init  serial device:  configure and initialize RTC driver
 *        2) open  serial device:  not used
 *        3) close serial device:  not used
 *        4) read  serial device:  get RTC
 *        5) write serial device:  set RTC
 *        6) ctrl  serial device:  not used
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
 *      Include files
 ************************************************************************/

#include <sysdefs.h>
#include <syserror.h>
#include <sysdev.h>
#include <io_api.h>
#include <sys_api.h>
#include <syscon_api.h>
#include <rtc_api.h>
#include <rtc_api.h>

/************************************************************************
 *      Definitions
 ************************************************************************/

/* Register access */

#define SETREG(reg, data)		    \
	   if( size == sizeof(UINT8) )	    \
	       REG8(reg)  = (data);	    \
	   else				    \
	       REG32(reg) = (data);

#define GETREG(reg)			    \
	   ( ( size == sizeof(UINT8) ) ?    \
	       REG8(reg) : REG32(reg) )


/************************************************************************
 *  RTC device: Relative Register Addresses
 ************************************************************************/

#define RTC_SEC_OFS                  0x00    /* seconds register        */
#define RTC_SECALARM_OFS             0x01    /* seconds alarm register  */
#define RTC_MIN_OFS                  0x02    /* minutes register        */
#define RTC_MINALARM_OFS             0x03    /* minutes alarm register  */
#define RTC_HOUR_OFS                 0x04    /* hours register          */
#define RTC_HOURALARM_OFS            0x05    /* hours alarm register    */
#define RTC_DAYOFWEEK_OFS            0x06    /* day of week register    */
#define RTC_DAYOFMONTH_OFS           0x07    /* day of month register   */
#define RTC_MONTH_OFS                0x08    /* month register          */
#define RTC_YEAR_OFS                 0x09    /* year register           */
#define RTC_REGA_OFS                 0x0A    /* register A              */
#define RTC_REGB_OFS                 0x0B    /* register B              */
#define RTC_REGC_OFS                 0x0C    /* register C              */
#define RTC_REGD_OFS                 0x0D    /* register D              */

/* RTC Register A bit map definitions */
#define RTC_REGA_UIP_SHF	     7
#define RTC_REGA_UIP_MSK	     (MSK(1) << RTC_REGA_UIP_SHF)
#define RTC_REGA_UIP_BIT	     RTC_REGA_UIP_MSK

#define RTC_REGA_DCS_SHF	     4
#define RTC_REGA_DCS_MSK	     (MSK(3) << RTC_REGA_DCS_SHF)
#define RTC_REGA_DCS_NORMAL	     2	     /* Normal operation	*/
#define RTC_REGA_DCS_RESET	     6	     /* Divider reset		*/

#define RTC_REGA_RSB_SHF	     0
#define RTC_REGA_RSB_MSK	     (MSK(4) << RTC_REGA_RSB_SHF)
#define RTC_REGA_RSB_NONE	     0	     /* No periodic interrupt   */
#define RTC_REGA_RSB_3_9	     1	     /* 3.90625 ms		*/
#define RTC_REGA_RSB_7_8	     2	     /* 7.8125 ms		*/
#define RTC_REGA_RSB_122	     3	     /* 122.070 us		*/
#define RTC_REGA_RSB_244	     4	     /* 244.141 us		*/
#define RTC_REGA_RSB_488	     5	     /* 488.281 us		*/
#define RTC_REGA_RSB_976	     6	     /* 976.5625 us		*/
#define RTC_REGA_RSB_1_9	     7	     /* 1.953125 ms		*/
#define RTC_REGA_RSB_3_9A	     8	     /* 3.90625 ms		*/
#define RTC_REGA_RSB_7_8A	     9	     /* 7.8125 ms		*/
#define RTC_REGA_RSB_15		     10	     /* 15.625 ms		*/
#define RTC_REGA_RSB_31		     11	     /* 31.25 ms		*/
#define RTC_REGA_RSB_62		     12	     /* 62.5 ms			*/
#define RTC_REGA_RSB_125	     13	     /* 125 ms			*/
#define RTC_REGA_RSB_250	     14	     /* 250 ms			*/
#define RTC_REGA_RSB_500	     15	     /* 500 ms			*/


/* RTC Register B bit map definitions */
#define RTC_REGB_SET_SHF	     7 
#define RTC_REGB_SET_MSK	     (MSK(1) << RTC_REGB_SET_SHF)
#define RTC_REGB_SET_BIT	     RTC_REGB_SET_MSK

#define RTC_REGB_PIE_SHF	     6
#define RTC_REGB_PIE_MSK	     (MSK(1) << RTC_REGB_PIE_SHF)
#define RTC_REGB_PIE_BIT	     RTC_REGB_PIE_MSK

#define RTC_REGB_AIE_SHF	     5
#define RTC_REGB_AIE_MSK	     (MSK(1) << RTC_REGB_AIE_SHF)
#define RTC_REGB_AIE_BIT	     RTC_REGB_AIE_MSK

#define RTC_REGB_UIE_SHF	     4
#define RTC_REGB_UIE_MSK	     (MSK(1) << RTC_REGB_UIE_SHF)
#define RTC_REGB_UIE_BIT	     RTC_REGB_UIE_MSK

#define RTC_REGB_SQWE_SHF	     3
#define RTC_REGB_SQWE_MSK	     (MSK(1) << RTC_REGB_SQWE_SHF)
#define RTC_REGB_SQWE_BIT	     RTC_REGB_SQWE_MSK

#define RTC_REGB_DM_SHF		     2
#define RTC_REGB_DM_MSK		     (MSK(1) << RTC_REGB_DM_SHF)
#define RTC_REGB_DM_BIT		     RTC_REGB_DM_MSK
#define RTC_REGB_DM_BIN		     1
#define RTC_REGB_DM_BCD		     0

#define RTC_REGB_HF_SHF		     1
#define RTC_REGB_HF_MSK		     (MSK(1) << RTC_REGB_HF_SHF)
#define RTC_REGB_HF_BIT		     RTC_REGB_HF_MSK
#define RTC_REGB_HF_24		     1
#define RTC_REGB_HF_12		     0

#define RTC_REGB_DSE_SHF	     0	
#define RTC_REGB_DSE_MSK	     (MSK(1) << RTC_REGB_DSE_SHF)
#define RTC_REGB_DSE_BIT	     RTC_REGB_DSE_MSK


/* RTC Register C bit map definitions */
#define RTC_REGC_IRQF_SHF	     7
#define RTC_REGC_IRQF_MSK	     (MSK(1) << RTC_REGC_IRQF_SHF)
#define RTC_REGC_IRQF_BIT	     RTC_REGC_IRQF_MSK

#define RTC_REGC_PF_SHF		     6
#define RTC_REGC_PF_MSK		     (MSK(1) << RTC_REGC_PF_SHF)
#define RTC_REGC_PF_BIT		     RTC_REGC_PF_MSK

#define RTC_REGC_AF_SHF		     5
#define RTC_REGC_AF_MSK		     (MSK(1) << RTC_REGC_AF_SHF)
#define RTC_REGC_AF_BIT		     RTC_REGC_AF_MSK

#define RTC_REGC_UF_SHF		     4
#define RTC_REGC_UF_MSK		     (MSK(1) << RTC_REGC_UF_SHF)
#define RTC_REGC_UF_BIT		     RTC_REGC_UF_MSK


/* RTC Register D bit map definitions */
#define RTC_REGD_VRT_SHF	     7
#define RTC_REGD_VRT_MSK	     (MSK(1) << RTC_REGD_VRT_SHF)
#define RTC_REGD_VRT_BIT	     RTC_REGD_VRT_MSK

#define RTC_REGD_DA_SHF		     0
#define RTC_REGD_DA_MSK		     (MSK(6) << RTC_REGD_DA_SHF)


/************************************************************************
 *  Macro Definitions
*************************************************************************/


/************************************************************************
 *      Public variables
 ************************************************************************/



/************************************************************************
 *      Static variables
 ************************************************************************/

static UINT32 rtcaddr, rtcdata;
static UINT8  size;

/************************************************************************
 *      Static function prototypes
 ************************************************************************/


/************************************************************************
 *
 *                          RTC_init
 *  Description :
 *  -------------
 *  This service initializes the RTC driver.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   INOUT, not used
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static INT32 
RTC_init(
    UINT32 major,          /* IN: major device number			*/
    UINT32 minor,          /* IN: minor device number			*/
    void   *p_param );     /* INOUT: device parameter block		*/


/************************************************************************
 *
 *                          RTC_read
 *  Description :
 *  -------------
 *  This service reads the current value of the Real Time Clock.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, RTC variable of type, t_RTC_calendar.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static INT32 
RTC_read(
    UINT32         major,      /* IN: major device number		*/
    UINT32         minor,      /* IN: minor device number		*/
    t_RTC_calendar *p_param ); /* INOUT: current RTC value		*/


/************************************************************************
 *
 *                          RTC_write
 *  Description :
 *  -------------
 *  This service sets the current value of the Real Time Clock.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, RTC variable of type, t_RTC_calendar.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static INT32 
RTC_write(
    UINT32         major,      /* IN: major device number		*/
    UINT32	   minor,      /* IN: minor device number		*/
    t_RTC_calendar *p_param ); /* INOUT: current RTC value		*/


/************************************************************************
 *      Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          RTC_install
 *  Description :
 *  -------------
 *
 *  Installs the RTC device drivers services in the IO system 
 *  at the reserved device slot, found in the 'sys_dev.h' file, 
 *  which defines all major device numbers.
 *
 *  Note:
 *  This service is the only public declared interface function; all
 *  provided device driver services are static declared, but this
 *  function installs the function pointers in the io-system to
 *  enable the provided public driver services.
 *
 *  Parameters :
 *  ------------
 *
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *  'ERROR_IO_ILLEGAL_MAJOR':  Illegal major device number
 *  'ERROR_IO_NO_SPACE':       Device slot already allocated
 *
 ************************************************************************/
INT32 
RTC_install( void )
{
    /* pre-initialize local variables and install device services       */
    IO_install( SYS_MAJOR_RTC,             /* major device number   	*/
         (t_io_service) RTC_init,          /* 'init'  service       	*/
                        NULL,              /* 'open'  service  N/A	*/
                        NULL,              /* 'close' service  N/A	*/
         (t_io_service) RTC_read,          /* 'read'  service		*/
         (t_io_service) RTC_write,         /* 'write' service		*/
                        NULL ) ;           /* 'ctrl'  service  N/A	*/

    /* call our own 'init' service */
    return IO_init( SYS_MAJOR_RTC, 0, NULL);
}



/************************************************************************
 *      Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          RTC_init
 *  Description :
 *  -------------
 *  This service initializes the RTC driver.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    not used
 *  'p_param',   INOUT, not used
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 *
 ************************************************************************/
static INT32 
RTC_init(
    UINT32 major,          /* IN: major device number			*/
    UINT32 minor,          /* IN: minor device number			*/
    void   *p_param )      /* INOUT: device parameter block		*/
{
    bool   start12;
    UINT8  hour;

    /* Get addresses of ADR and DAT registers */
    SYSCON_read( SYSCON_BOARD_RTC_ADDR_ID, &rtcaddr, sizeof(rtcaddr) );
    SYSCON_read( SYSCON_BOARD_RTC_DATA_ID, &rtcdata, sizeof(rtcdata) );

    /* Get register size */
    SYSCON_read( SYSCON_BOARD_RTC_REG_SIZE_ID, &size, sizeof(UINT8) );

    rtcaddr = KSEG1( rtcaddr );
    rtcdata = KSEG1( rtcdata );

    /* initialize RTC,
     *  Register B:
     *    - 'disable update, temporary'
     *    - 'binary mode'
     *    - 'daylight saving on'
     *    - 24 hr mode
     *
     * If RTC is in 12 hr mode, update time.
     */

    SETREG( rtcaddr, RTC_REGB_OFS );

    start12 = 
        (REGFIELD( GETREG(rtcdata), RTC_REGB_HF ) == RTC_REGB_HF_12) ?
	    TRUE : FALSE;

    SETREG( rtcdata, RTC_REGB_SET_BIT			   |
		     (RTC_REGB_DM_BIN << RTC_REGB_DM_SHF)  |
		     RTC_REGB_DSE_BIT			   |
		     (RTC_REGB_HF_24 << RTC_REGB_HF_SHF) );

    if( start12) 
    {
        /* force 24h format */

        SETREG( rtcaddr, RTC_HOUR_OFS );

        hour            = GETREG( rtcdata ); 

        if (hour == 12)   hour = 0;
        if (hour == 0x8c) hour = 12;
        if (hour & 0x80)  hour = (hour & 0xf) + 12;

        SETREG( rtcdata, hour );
    } 

    /* Register A */
    SETREG( rtcaddr, RTC_REGA_OFS );

    SETREG( rtcdata, (RTC_REGA_DCS_NORMAL << RTC_REGA_DCS_SHF) |
		     (RTC_REGA_RSB_NONE   << RTC_REGA_RSB_SHF) );

    /* re-enable update */
    SETREG( rtcaddr, RTC_REGB_OFS );
    SETREG( rtcdata, GETREG(rtcdata) & ~RTC_REGB_SET_BIT );

    return( OK );
}


/************************************************************************
 *
 *                          RTC_read
 *  Description :
 *  -------------
 *  This service reads the current value of the Real Time Clock.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, RTC variable of type, t_RTC_calendar.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static INT32 
RTC_read(
    UINT32 major,               /* IN: major device number		 */
    UINT32 minor,               /* IN: minor device number		 */
    t_RTC_calendar *p_param )   /* INOUT: current RTC value		 */
{
    UINT8  data;
    UINT32 cp0_ie;
  
    /*  Read RTC value.
     *
     *  When the UIP bit is set, the clock will not tick for at least
     *  244 us.
     *  We need to perform the read operations before that happens.
     *  This should not be a problem unless an interrupt occurs. 
     *  So, we disable interrupts and poll the UIP bit until it is set.
     *  Then we read the data and restore the interrupt enable bit.
     */

    cp0_ie = sys_disable_int();
    
    do
    {
        SETREG( rtcaddr, RTC_REGA_OFS );
        data = GETREG( rtcdata );
    }
    while( data & RTC_REGA_UIP_MSK );

    SETREG( rtcaddr, RTC_SEC_OFS );
    p_param->second = GETREG( rtcdata ); 

    SETREG( rtcaddr, RTC_MIN_OFS );
    p_param->minute = GETREG( rtcdata ); 

    SETREG( rtcaddr, RTC_HOUR_OFS );
    p_param->hour   = GETREG( rtcdata ); 

    SETREG( rtcaddr, RTC_DAYOFWEEK_OFS ); /* CE:0-6 RTC:1-7 */
    p_param->dayofweek  = GETREG( rtcdata ); 

    SETREG( rtcaddr, RTC_DAYOFMONTH_OFS );
    p_param->dayofmonth = GETREG( rtcdata ); 

    SETREG( rtcaddr, RTC_MONTH_OFS );
    p_param->month  = GETREG( rtcdata ); 

    /*  We assume no century register, but instead assume:
     *  Year ranges: 0<=y<=69: 2000-2069.  70<=y<=255: 1970-2155.
     *  Future generations, please forgive us.
     */
    p_param->year   = 1900;

    SETREG( rtcaddr, RTC_YEAR_OFS );
    p_param->year += GETREG( rtcdata ); 
    if (p_param->year < 1970)
        p_param->year += 100;

    /* Reenable interrupts in case interrupts were enabled */
    if( cp0_ie )
        sys_enable_int();

    return( OK );
}


/************************************************************************
 *
 *                          RTC_write
 *  Description :
 *  -------------
 *  This service sets the current value of the Real Time Clock.
 *  
 *
 *  Parameters :
 *  ------------
 *
 *  'major',     IN,    major device number
 *  'minor',     IN,    minor device number for multi device drivers
 *  'p_param',   INOUT, RTC variable of type, t_RTC_calendar.
 *
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 *
 ************************************************************************/
static INT32 
RTC_write(
    UINT32         major,       /* IN: major device number		*/
    UINT32         minor,       /* IN: minor device number		*/
    t_RTC_calendar *p_param )   /* INOUT: current RTC value		*/
{
    /* Set RTC value */

    /* disable register update during setting of RTC value */
    SETREG( rtcaddr, RTC_REGB_OFS );
    SETREG( rtcdata, GETREG(rtcdata) | RTC_REGB_SET_BIT );

    SETREG( rtcaddr, RTC_SEC_OFS );
    SETREG( rtcdata, p_param->second );

    SETREG( rtcaddr, RTC_MIN_OFS );
    SETREG( rtcdata, p_param->minute ); 

    SETREG( rtcaddr, RTC_HOUR_OFS );
    SETREG( rtcdata, p_param->hour );

    SETREG( rtcaddr, RTC_DAYOFWEEK_OFS ); /* CE:0-6 RTC:1-7 */
    SETREG( rtcdata, p_param->dayofweek ); 

    SETREG( rtcaddr, RTC_DAYOFMONTH_OFS );
    SETREG( rtcdata, p_param->dayofmonth );

    SETREG( rtcaddr, RTC_MONTH_OFS );
    SETREG( rtcdata, p_param->month );

    SETREG( rtcaddr, RTC_YEAR_OFS );
    SETREG( rtcdata, p_param->year % 100 ); 

    /* re-enable update */
    SETREG( rtcaddr, RTC_REGB_OFS );
    SETREG( rtcdata, GETREG(rtcdata) & ~RTC_REGB_SET_BIT );

    return( OK );
}






