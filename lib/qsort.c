/************************************************************************
 *
 *  qsort.c
 *
 *  Quicksort
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

/************************************************************************
 *  Definitions
 ************************************************************************/

#define		THRESH		4		/* threshold for insertion */
#define		MTHRESH		6		/* threshold for median */

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static  int		(*qcmp)();		/* the comparison routine */
static  int		qsz;			/* size of each record */
static  int		thresh;			/* THRESHold in chars */
static  int		mthresh;		/* MTHRESHold in chars */

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static void
qst(
    char *base, 
    char *max );

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          qsort
 *  Description :
 *  -------------
 *
 *  qsort:
 *  First, set up some global parameters for qst to share.  Then, quicksort
 *  with qst(), and then a cleanup insertion sort ourselves.  Sound simple?
 *  It's not...
 *
 *  Perform quicksort
 *
 ************************************************************************/
void
qsort(
char *base, 
int  n, 
int  size,
int  (*compar)() )
{
	register char c, *i, *j, *lo, *hi;
	char *min, *max;

	if (n <= 1)
		return;
	qsz = size;
	qcmp = compar;
	thresh = qsz * THRESH;
	mthresh = qsz * MTHRESH;
	max = base + n * qsz;
	if (n >= THRESH) {
		qst(base, max);
		hi = base + thresh;
	} else {
		hi = max;
	}
	/*
	 * First put smallest element, which must be in the first THRESH, in
	 * the first position as a sentinel.  This is done just by searching
	 * the first THRESH elements (or the first n if n < THRESH), finding
	 * the min, and swapping it into the first position.
	 */
	for (j = lo = base; (lo += qsz) < hi; )
		if (qcmp(j, lo) > 0)
			j = lo;
	if (j != base) {
		/* swap j into place */
		for (i = base, hi = base + qsz; i < hi; ) {
			c = *j;
			*j++ = *i;
			*i++ = c;
		}
	}
	/*
	 * With our sentinel in place, we now run the following hyper-fast
	 * insertion sort.  For each remaining element, min, from [1] to [n-1],
	 * set hi to the index of the element AFTER which this one goes.
	 * Then, do the standard insertion sort shift on a character at a time
	 * basis for each element in the frob.
	 */
	for (min = base; (hi = min += qsz) < max; ) {
		while (qcmp(hi -= qsz, min) > 0)
			/* void */;
		if ((hi += qsz) != min) {
			for (lo = min + qsz; --lo >= min; ) {
				c = *lo;
				for (i = j = lo; (j -= qsz) >= hi; i = j)
					*i = *j;
				*i = c;
			}
		}
	}
}

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *
 *                          <function name>
 *  Description :
 *  -------------
 *
 *  qst:
 *  Do a quicksort
 *  First, find the median element, and put that one in the first place as the
 *  discriminator.  (This "median" is just the median of the first, last and
 *  middle elements).  (Using this median instead of the first element is a big
 *  win).  Then, the usual partitioning/swapping, followed by moving the
 *  discriminator into the right place.  Then, figure out the sizes of the two
 *  partions, do the smaller one recursively and the larger one via a repeat of
 *  this code.  Stopping when there are less than THRESH elements in a partition
 *  and cleaning up with an insertion sort (in our caller) is a huge win.
 *  All data swaps are done in-line, which is space-losing but time-saving.
 *  (And there are only three places where this is done).
 *
 ************************************************************************/
static void
qst(
    char *base, 
    char *max )
{
	register char c, *i, *j, *jj;
	register int ii;
	char *mid, *tmp;
	int lo, hi;

	/*
	 * At the top here, lo is the number of characters of elements in the
	 * current partition.  (Which should be max - base).
	 * Find the median of the first, last, and middle element and make
	 * that the middle element.  Set j to largest of first and middle.
	 * If max is larger than that guy, then it's that guy, else compare
	 * max with loser of first and take larger.  Things are set up to
	 * prefer the middle, then the first in case of ties.
	 */
	lo = max - base;		/* number of elements as chars */
	do	{
		mid = i = base + qsz * ((lo / qsz) >> 1);
		if (lo >= mthresh) {
			j = (qcmp((jj = base), i) > 0 ? jj : i);
			if (qcmp(j, (tmp = max - qsz)) > 0) {
				/* switch to first loser */
				j = (j == jj ? i : jj);
				if (qcmp(j, tmp) < 0)
					j = tmp;
			}
			if (j != i) {
				ii = qsz;
				do	{
					c = *i;
					*i++ = *j;
					*j++ = c;
				} while (--ii);
			}
		}
		/*
		 * Semi-standard quicksort partitioning/swapping
		 */
		for (i = base, j = max - qsz; ; ) {
			while (i < mid && qcmp(i, mid) <= 0)
				i += qsz;
			while (j > mid) {
				if (qcmp(mid, j) <= 0) {
					j -= qsz;
					continue;
				}
				tmp = i + qsz;	/* value of i after swap */
				if (i == mid) {
					/* j <-> mid, new mid is j */
					mid = jj = j;
				} else {
					/* i <-> j */
					jj = j;
					j -= qsz;
				}
				goto swap;
			}
			if (i == mid) {
				break;
			} else {
				/* i <-> mid, new mid is i */
				jj = mid;
				tmp = mid = i;	/* value of i after swap */
				j -= qsz;
			}
		swap:
			ii = qsz;
			do	{
				c = *i;
				*i++ = *jj;
				*jj++ = c;
			} while (--ii);
			i = tmp;
		}
		/*
		 * Look at sizes of the two partitions, do the smaller
		 * one first by recursion, then do the larger one by
		 * making sure lo is its size, base and max are update
		 * correctly, and branching back.  But only repeat
		 * (recursively or by branching) if the partition is
		 * of at least size THRESH.
		 */
		i = (j = mid) + qsz;
		if ((lo = j - base) <= (hi = max - i)) {
			if (lo >= thresh)
				qst(base, j);
			base = i;
			lo = hi;
		} else {
			if (hi >= thresh)
				qst(i, max);
			max = j;
		}
	} while (lo >= thresh);
}
