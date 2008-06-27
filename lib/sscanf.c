
/************************************************************************
 *
 *  sscanf.c
 *
 *  sscanf function
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

#include <string.h>
#include <ctype.h>
#include <stdio.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define	FLOAT	0
#define	NUL	0

#ifndef _fstrcpy
#define _fstrcpy strcpy
#endif

#define MAXLINE 64

typedef int  BOOL;
typedef int  TBOOL;

#define NEXT            TRUE    /* Forward direction for getting chars. */
#define BACK            FALSE   /* Reverse direction for getting chars. */
#define LONGTYP         TRUE    /* Value to signal long or double.      */
#define NORMTYP         FALSE   /* Value to denote normal width number. */
#define SHORTTYP        -TRUE   /* Value to denote short rather than int*/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char      *scanstr;
static BOOL      NOT_EOL;

static int       width;
static int       (*xgetc)();

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static int 
sgetc(
    int forward );

static BOOL 
eatblanks( void );

static long 
nxtnumber(
    int base );

#if FLOAT

static double
nxtfloat(
    char *buffer );

#endif

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *
 *                          sgetc
 *  Description :
 *  -------------
 *
 *
 *  Return values :
 *  ---------------
 *
 *  Character from the static scanstr.
 *  If forward is TRUE, get next character and advance; else, back up 
 *  and get one unless the end has been reached. Return EOF if so.
 *
 ************************************************************************/
static int
sgetc(
    BOOL forward )
{
	if (forward)
	{       if (*scanstr)
			return *scanstr++ & 0xff;

		else
			NOT_EOL = FALSE;
	}
	else if (NOT_EOL)
		return *--scanstr & 0xff;

	return EOF;
}



/************************************************************************
 *
 *                          eatblanks
 *  Description :
 *  -------------
 *
 *  Get characters from input medium until no longer whitespace.
 *
 *  Return values :
 *  ---------------
 *
 *  FALSE normally, or TRUE at end of line.
 *
 ************************************************************************/
static BOOL
eatblanks( void )     
{
    char c;
	
    while (isspace(c = (*xgetc)(NEXT)))
	;
    c = c; /* dummy */
    if ((*xgetc)(BACK) == EOF)
		return TRUE;
    else
		return FALSE;
}


#if	FLOAT

/************************************************************************
 *
 *                          nxtfloat
 *  Description :
 *  -------------
 *
 *  Get the next ascii e-form or f-form from the input medium, and 
 *  return its value. Store the ascii form in buffer.
 *
 *  Return values :
 *  ---------------
 *
 *  See description above.
 *
 ************************************************************************/
static double
nxtfloat(
    char *buffer )
{
	int c;
	char *s;
	TBOOL decmlpt, sign, exp;
	double atof();

	s = buffer;
	sign = exp = decmlpt = FALSE;
	while (width--)
	{       c = (*xgetc)(NEXT);
		if (!sign && (c == '-' || c == '+'))
			sign = TRUE;
		else if (!decmlpt && c == '.')
			sign = decmlpt = TRUE;
		else if (!exp && (c == 'e' || c == 'E'))
		{       sign = FALSE;
			exp = decmlpt = TRUE;
		}
		else if (!isdigit(c))
		{       (*xgetc)(BACK);
			break;
		}
		*s++ = c;
	}
	*s = 0;
	/*return atof(buffer);*/    
	return 0L;
}

#endif  /* #if FLOAT */


/************************************************************************
 *
 *                          nxtnumber
 *  Description :
 *  -------------
 *
 *  Get the ascii string of a number with given base from input
 *  medium, return its value.
 *
 *  Return values :
 *  ---------------
 *
 *  See description above.
 *
 ************************************************************************/
static long
nxtnumber(
    int base )
{
	int c;
	long value;
	BOOL minus;
	static char digits[] = "0123456789abcdef";

	if (width <= 0)
		return 0L;

	value = 0;
	minus = FALSE;
	if ((c = (*xgetc)(NEXT)) == '-')
	{       minus = TRUE;
		--width;
	}
	else if (c == '+')
		--width;
	else
		(*xgetc)(BACK);
	while (width--)
	{       c = (int)(index((char *)digits, tolower((*xgetc)(NEXT))) - (char *)digits);
		if (c < 0 || c >= base)
		{       (*xgetc)(BACK);
			break;
		}
		value *= base;
		value += c;
	}
	return (minus ? -value : value);
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          sscanf
 *  Description :
 *  -------------
 *
 *  Read string s using format fmt, and deposit information into 
 *  arguments in arg list.
 *
 *  Return values :
 *  ---------------
 *
 *  Return count of items read.
 *
 ************************************************************************/
int 
sscanf(
    char *s, 
    char *fmt, 
    ...)
{
	int unformat();

	int **arg = (int **) (&fmt) + 1;

	scanstr = s;
	NOT_EOL = TRUE;
	return unformat(sgetc, fmt, (int **) arg);
}



/************************************************************************
 *
 *                          unformat
 *  Description :
 *  -------------
 *
 *  Perform scanf() on getch input medium, using format fmt and argument 
 *  list pointed to by arglist. 
 *
 *  Return values :
 *  ---------------
 *
 *  Number of matched and assigned input items, or EOF if input finished 
 *  before fmt.
 *
 ************************************************************************/
int
unformat(
    int    (*getch)(),
    char   *fmt, 
    int    **arglist )
{
	int c;
#if	FLOAT
	double nxtfloat(), d;
#endif
	long val;
	int count, base;
	char *s;
	BOOL suppress, numtyp;
	char wkbuf[MAXLINE];

	count = 0;
	xgetc = getch;

	while ((c = *fmt++))
	{       if (c == '%')
		{       numtyp = NORMTYP;
			suppress = FALSE;
			width = MAXLINE;
			if (*fmt == '*')
			{       ++fmt;
				suppress = TRUE;
			}
			if (isdigit(*fmt))
			{       width = 0;
				do
				{       width *= 10;
					width += (*fmt - '0');
				} while (isdigit(*++fmt));
			}
			if (*fmt == 'l')
			{       numtyp = LONGTYP;
				++fmt;
			}
			if (*fmt == 'h')
			{       numtyp = SHORTTYP;
				++fmt;
			}
			if (index("DOXEF", c = *fmt++))
				numtyp = LONGTYP;
			switch (c = tolower(c))
			{   case ' ':
				fmt--;
				if (numtyp == SHORTTYP)
					c = 'd';
				else
					break;
			    case 'd':
			    case 'o':
			    case 'x':
				base = (c == 'd'? 10 : (c == 'o' ? 8 : 16));
				if (eatblanks())
					return (count ? count : EOF);

				val = nxtnumber(base);
				if (!suppress)
				{       if (numtyp == LONGTYP)
						*(long *)(*arglist++) = val;
					else if (numtyp == SHORTTYP)
						*(short *)(*arglist++) = val;
					else
						*(*arglist++) = (int)val;
					++count;
				}
				break;
#if	FLOAT
			    case 'e':
			    case 'f':
				if (eatblanks())
					return (count ? count : EOF);

				d = nxtfloat((char *)wkbuf);
				if (!suppress)
				{       if (numtyp == LONGTYP)
						*(double *)(*arglist++) = d;
					else
						*(float *)(*arglist++) = d;
					++count;
				}
				break;
#endif
			    case '[':
			    case 's':
				if (c == '[')
				{       if (*fmt == '^' || *fmt == '~')
					{       numtyp = NORMTYP;
						++fmt;
					}
					else
						numtyp = LONGTYP;
					for (s = (char *)wkbuf;(c = *fmt++) != ']';)
						*s++ = c;
					*s = (char)NUL;
				}
				else
				{       numtyp = NORMTYP;
					_fstrcpy((char *)wkbuf, " \t\n");
				}
				if (eatblanks())
					return (count ? count : EOF);

				if (!suppress)
					s = (char *) *arglist++;
				while (width--)
				{       if ((c = (*xgetc)(NEXT)) == EOF)
						break;
					if (numtyp == NORMTYP ?
					    (index((char *)wkbuf, c) != NUL) :
					    (index((char *)wkbuf, c)  ==  NUL))
					{       (*xgetc)(BACK);
						break;
					}
					if (!suppress)
						*s++ = c;
				}
				if (!suppress)
				{       *s = (char)NUL;
					++count;
				}
				break;

			    case 'c':
				if ((c = (*xgetc)(NEXT)) == EOF)
					return (count ? count : EOF);

				if (!suppress)
				{       *(char *)(*arglist++) = c;
					++count;
				}
				break;
			    case '%':
				goto match;
			}
		}
		else if (isspace(c))
		{       if (eatblanks())
				return (count ? count : EOF);
		}
		else
		{
match:                  if (eatblanks())
				return (count ? count : EOF);

			if ((*xgetc)(NEXT) != c)
				return count;
		}
	}
	return count;
}


/************************************************************************
 *
 *                          index
 *  Description :
 *  -------------
 *
 *  Return values :
 *  ---------------
 *
 *  Return pointer to char c in string s, if it occurs, NUL otherwise.
 *
 ************************************************************************/
char *
index(
    char *s, 
    int  c)
{
	int i;

	while ((i = *s) && c != i)
		s++;
	return (*s ? s : NUL);
}
