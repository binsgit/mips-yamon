
/************************************************************************
 *
 *  date.c
 *
 *  Shell date command
 *
 *  date [[cc]yy]mmddHHMM[.ss]       (HH = 00-23)
 *
 *   argument possible   length
 *       mmddHHMM           8
 *       mmddHHMM.ss       11
 *     yymmddHHMM          10
 *     yymmddHHMM.ss       13
 *   ccyymmddHHMM          12
 *   ccyymmddHHMM.ss       15
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
#include <shell_api.h>
#include <shell.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <io_api.h>
#include <rtc_api.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static MON_FUNC(shell_date);

static bool
mydate(char   *args);

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    char   **parg1 );


/************************************************************************
 *  Static variables
 ************************************************************************/

static UINT16 mdays[13] =
           {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static UINT16 leapdays[13] =
           {  0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

static char dayname[][4] = {
  { '?','?','?', 0 }, { 'S','u','n', 0 }, { 'M','o','n', 0 }, { 'T','u','e', 0 },
  { 'W','e','d', 0 }, { 'T','h','u', 0 }, { 'F','r','i', 0 }, { 'S','a','t', 0 }
};
#define MAXDAY       7
#define BADDAY_INDEX 0


static char monname[][4] = {
  { '?','?','?', 0 },
  { 'J','a','n', 0 }, { 'F','e','b', 0 }, { 'M','a','r', 0 }, { 'A','p','r', 0 },
  { 'M','a','y', 0 }, { 'j','u','n', 0 }, { 'J','u','l', 0 }, { 'A','u','g', 0 },
  { 'S','e','p', 0 }, { 'O','c','t', 0 }, { 'N','o','v', 0 }, { 'D','e','c', 0 }
};
#define MAXMONTH 12
#define BADMONTH_INDEX 0

/* Command definition for help */
static t_cmd cmd_def =
{
    "date",
     shell_date,
    "date [ [[cc]yy]mmddHHMM[.ss] ]   (HH in 24 hour format)",

    "Set or read date and time.\n"
    "If no argument is given, the present date and time is output.\n"
    "If an argument is specified, it is used to set the date and time\n"
    "on the real time clock.\n"
    "\n"
    "The argument must have the format [[cc]yy]mmddHHMM[.ss] where\n"
    "the hours HH are represented in 24h format (00-23).\n"
    "The output is shown as Day Mon dd HH:MM:SS ccyy.",

    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

static bool
mydate( char *arg1 )
{
    char *endptr;
    t_RTC_calendar mycaldr;
    UINT32 year;
    UINT32 days;
    int n;
    bool leapyear;


/*****************************************************
**  read current date (default for year and seconds)
*/
    IO_read( SYS_MAJOR_RTC, 0, (UINT8 *)(&mycaldr) );
    year = mycaldr.year;

/*****************************************************
**  parse input
*/
    if (arg1 == 0) goto prepare_print;

    n = strlen(arg1);
    if (n < 8) return FALSE;

    if (n & 1)
    {
        if (n < 11) return FALSE;
        if (arg1[n-3] != '.') return FALSE;
        mycaldr.second = strtoul(&arg1[n-2], &endptr, 10);
        if (endptr != &arg1[n]) return FALSE;
        n-= 3;
        arg1[n] = 0;
    }

    mycaldr.minute = strtoul(&arg1[n-2], &endptr, 10);
    if (endptr != &arg1[n]) return FALSE;
    n-= 2;
    arg1[n] = 0;

    mycaldr.hour = strtoul(&arg1[n-2], &endptr, 10);
    if (endptr != &arg1[n]) return FALSE;
    n-= 2;
    arg1[n] = 0;

    mycaldr.dayofmonth = strtoul(&arg1[n-2], &endptr, 10);
    if (endptr != &arg1[n]) return FALSE;
    n-= 2;
    arg1[n] = 0;

    mycaldr.month = strtoul(&arg1[n-2], &endptr, 10);
    if (endptr != &arg1[n]) return FALSE;
    n-= 2;
    arg1[n] = 0;

    if (n >= 2)
    {
        year = strtoul(&arg1[n-2], &endptr, 10);
        if (endptr != &arg1[n]) return FALSE;
        n-= 2;
        arg1[n] = 0;

        if (n == 2)
        {
            year += strtoul(&arg1[n-2], &endptr, 10) * 100;
            if (endptr != &arg1[n]) return FALSE;
            n-= 2;
        } else
            year += (year >= 70) ? 1900 : 2000;
    }

    if (n != 0) return FALSE;

/***********************************************************
**  syntax OK, check content of input
*/
    if ( mycaldr.month  ==  0 ||
         mycaldr.month  >  12 ||
         mycaldr.hour   >= 24 ||
         mycaldr.minute >= 60 ||
         mycaldr.second >= 60 ) return FALSE;

    leapyear= ((year % 4) == 0 &&
               ((year % 100) != 0 || (year % 400) == 0));

    if (leapyear) 
        days= leapdays[mycaldr.month] - leapdays[mycaldr.month-1];
    else
        days= mdays[mycaldr.month] - mdays[mycaldr.month-1];

    if ( mycaldr.dayofmonth ==    0 ||
         mycaldr.dayofmonth >  days ) return FALSE;
    
/***********************************************************
**  content OK, compute weekday and write back to RTC
*/
    /* compute day of week (no good before 17xx) */
    if (year != 0) {
        mycaldr.year = year;

        /* days before current year */
        year--;
        days = year*365 + year/4 - year/100 + year/400;
        year++;

        if (leapyear)
            days += leapdays[mycaldr.month-1];
        else
            days += mdays[mycaldr.month-1];
        days += mycaldr.dayofmonth;

        /* 01010001 => 1 */
        mycaldr.dayofweek = (days + 0) % 7;  /* unix: sunday=0, saturday=6) */
        mycaldr.dayofweek++;                 /* RTC:  sunday=1, saturday=7) */
    }

    IO_write( SYS_MAJOR_RTC, 0, (UINT8 *)(&mycaldr) );
   
    /* now read RTC again for print-out */
    IO_read( SYS_MAJOR_RTC, 0, (UINT8 *)(&mycaldr) );

prepare_print:
    /* array boundaries */
    if (mycaldr.dayofweek > MAXDAY)
        mycaldr.dayofweek = BADDAY_INDEX;
   
    if (mycaldr.month > MAXMONTH)
        mycaldr.month = BADMONTH_INDEX;

    /* "Fri Nov 26 11:50:32 1999" */

    printf ("%s %s %2d %02d:%02d:%02d %04d\n",
               dayname[mycaldr.dayofweek],
                  monname[mycaldr.month],
                     mycaldr.dayofmonth,
                         mycaldr.hour,
                              mycaldr.minute,
                                   mycaldr.second,
                                        mycaldr.year);

    return TRUE;
}

/************************************************************************
 *                          shell_date
 ************************************************************************/
static MON_FUNC(shell_date)
{
    /* Options */
    char *parg;
    UINT32 rc;

    rc = get_options( argc, argv, &parg );

    if( rc != OK )
        return rc;

    return mydate(parg) ?
        OK : SHELL_ERROR_SYNTAX;
}

/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    char   **parg1 )
{
    bool   ok    = TRUE;
    UINT32 error = SHELL_ERROR_SYNTAX;
    
    if( argc == 1 )
    {
        *parg1 = NULL;
    }
    else if( argc == 2 )
    {
        *parg1 = argv[1];

	if( **parg1 == '-' )
	{
	    error	     = SHELL_ERROR_OPTION;
	    shell_error_data = *parg1;
	    ok		     = FALSE;
	}
    }
    else
    {
        ok = FALSE;
    }

    return ok ? OK : error;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_date_init
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
shell_date_init( void )
{
    return &cmd_def;
}
