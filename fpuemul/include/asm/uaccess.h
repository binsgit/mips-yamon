/* $Id: uaccess.h,v 1.5 2002/09/19 13:16:08 kjelde Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Kjeld Borch Egevang
 * Copyright (C) 2002 MIPS Technologies, Inc.  All rights reserved.
 */

#define __get_user(w, a)	((w = *(a))*0)
#define get_user		__get_user
#define __put_user(w, a)	((*(a) = w)*0)
#define put_user		__put_user

#define force_sig(x,y)
#define VERIFY_READ    0
#define VERIFY_WRITE   1
#define EFAULT	-1

#define verify_area(t,a,s)	0
