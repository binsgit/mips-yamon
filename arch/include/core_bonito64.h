
/************************************************************************
 *
 *  core_bonito64.h
 *
 *  Register definitions for Bonito64 system controller
 *
 * ######################################################################
 *
 * mips_start_of_header
 * 
 *  $Id: core_bonito64.h,v 1.11 2002/03/14 14:35:54 eggerts Exp $
 * 
 * Copyright (c) [Year(s)] MIPS Technologies, Inc. All rights reserved.
 *
 * Unpublished rights reserved under U.S. copyright law.
 *
 * PROPRIETARY/SECRET CONFIDENTIAL INFORMATION OF MIPS TECHNOLOGIES,
 * INC. FOR INTERNAL USE ONLY.
 *
 * Under no circumstances (contract or otherwise) may this information be
 * disclosed to, or copied, modified or used by anyone other than employees
 * or contractors of MIPS Technologies having a need to know.
 *
 * 
 * mips_end_of_header
 *
 ************************************************************************/

#ifndef CORE_BONITO64_H
#define CORE_BONITO64_H

/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* CoreHI interrupt signal is taken from Bonito64 Int1 pin */
#define CORE_BONITO64_INT_HI		1

/************************************************************************
 *  Public variables
 ************************************************************************/

#ifndef _ASSEMBLER_

extern UINT32 _bonito;

#endif  /* #ifndef _ASSEMBLER_ */

/************************************************************************
 *  Public functions
 ************************************************************************/

#ifndef _ASSEMBLER_

/* Functions in flash for accessing specific Bonito64 registers */
UINT32 bonito64_write_iodevcfg( UINT32 data );
UINT32 bonito64_write_sdcfg(    UINT32 data );

#endif  /* #ifndef _ASSEMBLER_ */


#endif /* #ifndef CORE_BONITO64_H */




