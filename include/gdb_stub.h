
/************************************************************************
 *
 *  gdb_stub.h
 *
 *  Definition of layout of register structure for context save
 *  This is how GDB expects it.
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


#ifndef GDB_STUB_H
#define GDB_STUB_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>

/************************************************************************
 *  Definitions
*************************************************************************/

/* Max size of CPU registers (actual size depends on 32/64 bit processor) */
#define REGSIZE			8

/* Offset to 32 bit word in 64 bit storage area */
#ifdef EB
#define GDB_OFS_END		4
#else
#define GDB_OFS_END		0
#endif

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/


/**** Registers defined by GDB ****/


/* CPU registers */
#define GDB_FR_REG0		(0)				  /* 0 */
#define GDB_FR_REG1		((GDB_FR_REG0)         + REGSIZE) /* 1 */
#define GDB_FR_REG2		((GDB_FR_REG1)	       + REGSIZE) /* 2 */
#define GDB_FR_REG3		((GDB_FR_REG2)	       + REGSIZE) /* 3 */
#define GDB_FR_REG4		((GDB_FR_REG3)	       + REGSIZE) /* 4 */
#define GDB_FR_REG5		((GDB_FR_REG4)	       + REGSIZE) /* 5 */
#define GDB_FR_REG6		((GDB_FR_REG5)	       + REGSIZE) /* 6 */
#define GDB_FR_REG7		((GDB_FR_REG6)	       + REGSIZE) /* 7 */
#define GDB_FR_REG8		((GDB_FR_REG7)	       + REGSIZE) /* 8 */
#define GDB_FR_REG9	        ((GDB_FR_REG8)	       + REGSIZE) /* 9 */
#define GDB_FR_REG10		((GDB_FR_REG9)	       + REGSIZE) /* 10 */
#define GDB_FR_REG11		((GDB_FR_REG10)	       + REGSIZE) /* 11 */
#define GDB_FR_REG12		((GDB_FR_REG11)	       + REGSIZE) /* 12 */
#define GDB_FR_REG13		((GDB_FR_REG12)	       + REGSIZE) /* 13 */
#define GDB_FR_REG14		((GDB_FR_REG13)	       + REGSIZE) /* 14 */
#define GDB_FR_REG15		((GDB_FR_REG14)	       + REGSIZE) /* 15 */
#define GDB_FR_REG16		((GDB_FR_REG15)	       + REGSIZE) /* 16 */
#define GDB_FR_REG17		((GDB_FR_REG16)	       + REGSIZE) /* 17 */
#define GDB_FR_REG18		((GDB_FR_REG17)	       + REGSIZE) /* 18 */
#define GDB_FR_REG19		((GDB_FR_REG18)	       + REGSIZE) /* 19 */
#define GDB_FR_REG20		((GDB_FR_REG19)	       + REGSIZE) /* 20 */
#define GDB_FR_REG21		((GDB_FR_REG20)	       + REGSIZE) /* 21 */
#define GDB_FR_REG22		((GDB_FR_REG21)	       + REGSIZE) /* 22 */
#define GDB_FR_REG23		((GDB_FR_REG22)	       + REGSIZE) /* 23 */
#define GDB_FR_REG24		((GDB_FR_REG23)	       + REGSIZE) /* 24 */
#define GDB_FR_REG25		((GDB_FR_REG24)	       + REGSIZE) /* 25 */
#define GDB_FR_REG26		((GDB_FR_REG25)	       + REGSIZE) /* 26 */
#define GDB_FR_REG27		((GDB_FR_REG26)	       + REGSIZE) /* 27 */
#define GDB_FR_REG28		((GDB_FR_REG27)	       + REGSIZE) /* 28 */
#define GDB_FR_REG29		((GDB_FR_REG28)	       + REGSIZE) /* 29 */
#define GDB_FR_REG30		((GDB_FR_REG29)	       + REGSIZE) /* 30 */
#define GDB_FR_REG31		((GDB_FR_REG30)	       + REGSIZE) /* 31 */
	
/* Saved special registers */
#define GDB_FR_STATUS		((GDB_FR_REG31)        + REGSIZE) /* 32 */
#define GDB_FR_LO		((GDB_FR_STATUS)       + REGSIZE) /* 33 */
#define GDB_FR_HI		((GDB_FR_LO)	       + REGSIZE) /* 34 */
#define GDB_FR_BADVADDR		((GDB_FR_HI)	       + REGSIZE) /* 35 */
#define GDB_FR_CAUSE		((GDB_FR_BADVADDR)     + REGSIZE) /* 36 */
#define GDB_FR_EPC		((GDB_FR_CAUSE)	       + REGSIZE) /* 37 */

/* Saved floating point registers */
#define GDB_FR_FPR0		((GDB_FR_EPC)          + REGSIZE) /* 38 */
#define GDB_FR_FPR1		((GDB_FR_FPR0)	       + REGSIZE) /* 39 */
#define GDB_FR_FPR2		((GDB_FR_FPR1)	       + REGSIZE) /* 40 */
#define GDB_FR_FPR3		((GDB_FR_FPR2)	       + REGSIZE) /* 41 */
#define GDB_FR_FPR4		((GDB_FR_FPR3)	       + REGSIZE) /* 42 */
#define GDB_FR_FPR5		((GDB_FR_FPR4)	       + REGSIZE) /* 43 */
#define GDB_FR_FPR6		((GDB_FR_FPR5)	       + REGSIZE) /* 44 */
#define GDB_FR_FPR7		((GDB_FR_FPR6)	       + REGSIZE) /* 45 */
#define GDB_FR_FPR8		((GDB_FR_FPR7)	       + REGSIZE) /* 46 */
#define GDB_FR_FPR9		((GDB_FR_FPR8)	       + REGSIZE) /* 47 */
#define GDB_FR_FPR10		((GDB_FR_FPR9)	       + REGSIZE) /* 48 */
#define GDB_FR_FPR11		((GDB_FR_FPR10)	       + REGSIZE) /* 49 */
#define GDB_FR_FPR12		((GDB_FR_FPR11)	       + REGSIZE) /* 50 */
#define GDB_FR_FPR13		((GDB_FR_FPR12)	       + REGSIZE) /* 51 */
#define GDB_FR_FPR14		((GDB_FR_FPR13)	       + REGSIZE) /* 52 */
#define GDB_FR_FPR15		((GDB_FR_FPR14)	       + REGSIZE) /* 53 */
#define GDB_FR_FPR16		((GDB_FR_FPR15)	       + REGSIZE) /* 54 */
#define GDB_FR_FPR17		((GDB_FR_FPR16)	       + REGSIZE) /* 55 */
#define GDB_FR_FPR18		((GDB_FR_FPR17)	       + REGSIZE) /* 56 */
#define GDB_FR_FPR19		((GDB_FR_FPR18)	       + REGSIZE) /* 57 */
#define GDB_FR_FPR20		((GDB_FR_FPR19)	       + REGSIZE) /* 58 */
#define GDB_FR_FPR21		((GDB_FR_FPR20)	       + REGSIZE) /* 59 */
#define GDB_FR_FPR22		((GDB_FR_FPR21)	       + REGSIZE) /* 60 */
#define GDB_FR_FPR23		((GDB_FR_FPR22)	       + REGSIZE) /* 61 */
#define GDB_FR_FPR24		((GDB_FR_FPR23)	       + REGSIZE) /* 62 */
#define GDB_FR_FPR25		((GDB_FR_FPR24)	       + REGSIZE) /* 63 */
#define GDB_FR_FPR26		((GDB_FR_FPR25)	       + REGSIZE) /* 64 */
#define GDB_FR_FPR27		((GDB_FR_FPR26)	       + REGSIZE) /* 65 */
#define GDB_FR_FPR28		((GDB_FR_FPR27)	       + REGSIZE) /* 66 */
#define GDB_FR_FPR29		((GDB_FR_FPR28)	       + REGSIZE) /* 67 */
#define GDB_FR_FPR30		((GDB_FR_FPR29)	       + REGSIZE) /* 68 */
#define GDB_FR_FPR31		((GDB_FR_FPR30)	       + REGSIZE) /* 69 */

#define GDB_FR_FSR		((GDB_FR_FPR31)        + REGSIZE) /* 70 */
#define GDB_FR_FIR		((GDB_FR_FSR)	       + REGSIZE) /* 71 */
#define GDB_FR_FRP		((GDB_FR_FIR)	       + REGSIZE) /* 72 */

#define GDB_FR_DUMMY		((GDB_FR_FRP)          + REGSIZE) /* 73 (unused) */

/* Again, CP0 registers */
#define GDB_FR_CP0_INDEX	((GDB_FR_DUMMY)        + REGSIZE) /* 74 */
#define GDB_FR_CP0_RANDOM	((GDB_FR_CP0_INDEX)    + REGSIZE) /* 75 */
#define GDB_FR_CP0_ENTRYLO0	((GDB_FR_CP0_RANDOM)   + REGSIZE) /* 76 */
#define GDB_FR_CP0_ENTRYLO1	((GDB_FR_CP0_ENTRYLO0) + REGSIZE) /* 77 */
#define GDB_FR_CP0_CONTEXT	((GDB_FR_CP0_ENTRYLO1) + REGSIZE) /* 78 */
#define GDB_FR_CP0_PAGEMASK	((GDB_FR_CP0_CONTEXT)  + REGSIZE) /* 79 */
#define GDB_FR_CP0_WIRED	((GDB_FR_CP0_PAGEMASK) + REGSIZE) /* 80 */
#define GDB_FR_CP0_REG7		((GDB_FR_CP0_WIRED)    + REGSIZE) /* 81 */
#define GDB_FR_CP0_REG8		((GDB_FR_CP0_REG7)     + REGSIZE) /* 82 */
#define GDB_FR_CP0_REG9		((GDB_FR_CP0_REG8)     + REGSIZE) /* 83 */
#define GDB_FR_CP0_ENTRYHI	((GDB_FR_CP0_REG9)     + REGSIZE) /* 84 */
#define GDB_FR_CP0_REG11	((GDB_FR_CP0_ENTRYHI)  + REGSIZE) /* 85 */
#define GDB_FR_CP0_REG12	((GDB_FR_CP0_REG11)    + REGSIZE) /* 86 */
#define GDB_FR_CP0_REG13	((GDB_FR_CP0_REG12)    + REGSIZE) /* 87 */
#define GDB_FR_CP0_REG14	((GDB_FR_CP0_REG13)    + REGSIZE) /* 88 */
#define GDB_FR_CP0_PRID		((GDB_FR_CP0_REG14)    + REGSIZE) /* 89 */

/* Number of registers used by GDB */
#define GDB_REG_COUNT	        90

/* Registers not used by GDB, but used for context storage */
#define GDB_FR_CP0_ERROREPC	((GDB_FR_CP0_PRID)     + REGSIZE) /* 90  */
#define GDB_FR_CP0_CONFIG	((GDB_FR_CP0_ERROREPC) + REGSIZE) /* 91  */
#define GDB_FR_CP0_CONFIG1	((GDB_FR_CP0_CONFIG)   + REGSIZE) /* 92  */
#define GDB_FR_CP0_CONFIG2	((GDB_FR_CP0_CONFIG1)  + REGSIZE) /* 93  */
#define GDB_FR_CP0_CONFIG3	((GDB_FR_CP0_CONFIG2)  + REGSIZE) /* 94  */
#define GDB_FR_CP0_WATCHLO	((GDB_FR_CP0_CONFIG3)  + REGSIZE) /* 95  */
#define GDB_FR_CP0_WATCHHI	((GDB_FR_CP0_WATCHLO)  + REGSIZE) /* 96  */
#define GDB_FR_CP0_DEBUG	((GDB_FR_CP0_WATCHHI)  + REGSIZE) /* 97  */
#define GDB_FR_CP0_DEPC		((GDB_FR_CP0_DEBUG)    + REGSIZE) /* 98  */
/* MIPS32/64 Relase 2 registers not used by GDB, but used for context storage */
#define GDB_FR_CP0_HWRENA	((GDB_FR_CP0_DEPC)     + REGSIZE) /* 99  */
#define GDB_FR_CP0_EBASE	((GDB_FR_CP0_HWRENA)   + REGSIZE) /* 100 */
#define GDB_FR_CP0_INTCTL	((GDB_FR_CP0_EBASE)    + REGSIZE) /* 101 */
#define GDB_FR_CP0_SRSCTL	((GDB_FR_CP0_INTCTL)   + REGSIZE) /* 102 */
#define GDB_FR_CP0_SRSMAP	((GDB_FR_CP0_SRSCTL)   + REGSIZE) /* 103 */
#define GDB_FR_CP0_PAGEGRAIN	((GDB_FR_CP0_SRSMAP)   + REGSIZE) /* 104 */

/* Size of context structure */
#define EXCEP_SIZE_CONTEXT	((GDB_FR_CP0_PAGEGRAIN)     + REGSIZE)


/* Macro for calculating offset of specific GPR in context structure */
#define GDB_GPR_OFS( number )	(GDB_FR_REG0 + (number) * REGSIZE)

/* Macros for adjusting offset (due to endianness) of specific
 * register in context structure.
 */
#define GDB_REG32_OFS( offset )	( offset + GDB_OFS_END )
#define GDB_REG64_OFS( offset )	( offset )


#ifndef _ASSEMBLER_

/*
 * This is the same as above, but for the high-level
 * part of the GDB stub.
 */

typedef struct
{
	/* CPU registers */
	UINT64	reg0,  reg1,  reg2,  reg3,  reg4,  reg5,  reg6,  reg7;
	UINT64	reg8,  reg9,  reg10, reg11, reg12, reg13, reg14, reg15;
	UINT64	reg16, reg17, reg18, reg19, reg20, reg21, reg22, reg23;
	UINT64	reg24, reg25, reg26, reg27, reg28, reg29, reg30, reg31;

	/* Special registers */
	UINT64	cp0_status;
	UINT64	lo;
	UINT64	hi;
	UINT64	cp0_badvaddr;
	UINT64	cp0_cause;
	UINT64	cp0_epc;

	/* Floating point registers */
	UINT64	fpr0,  fpr1,  fpr2,  fpr3,  fpr4,  fpr5,  fpr6,  fpr7;
	UINT64	fpr8,  fpr9,  fpr10, fpr11, fpr12, fpr13, fpr14, fpr15;
	UINT64	fpr16, fpr17, fpr18, fpr19, fpr20, fpr21, fpr22, fpr23;
	UINT64	fpr24, fpr25, fpr26, fpr27, fpr28, fpr29, fpr30, fpr31;

	UINT64	cp1_fsr;
	UINT64	cp1_fir;

	/* Frame pointer */
	UINT64	frame_ptr;
	UINT64  dummy;		/* unused */
	
	/* CP0 registers */
	UINT64	cp0_index;
	UINT64	cp0_random;
	UINT64	cp0_entrylo0;
	UINT64	cp0_entrylo1;
	UINT64	cp0_context;
	UINT64	cp0_pagemask;
	UINT64	cp0_wired;
	UINT64	cp0_reg7;
	UINT64	cp0_reg8;
	UINT64	cp0_reg9;
	UINT64	cp0_entryhi;
	UINT64	cp0_reg11;
	UINT64	cp0_reg12;
	UINT64	cp0_reg13;
	UINT64	cp0_reg14;
	UINT64	cp0_prid;

	/* CP0 registers not used by GDB */
	UINT64  cp0_errorepc;
	UINT64	cp0_config;
	UINT64	cp0_config1;
	UINT64	cp0_config2;
	UINT64  cp0_config3;
	UINT64  cp0_watchlo;
	UINT64  cp0_watchhi;
	UINT64  cp0_debug;
	UINT64  cp0_depc;
	/* MIPS32/64 Relase 2 CP0 registers not used by GDB */
	UINT64  cp0_hwrena;
	UINT64	cp0_ebase;
	UINT64  cp0_intctl;
	UINT64  cp0_srsctl;
	UINT64  cp0_srsmap;
	UINT64  cp0_pagegrain;
}
t_gdb_regs;


/* Macro for access to CPU register in context structure */
#define SYS_CPUREG(pcontext, number)    (*(&(pcontext)->reg0 + (number)))

/* Macro for access to FP register in context structure */
#define SYS_FPREG(pcontext, number)     (*(&(pcontext)->fpr0 + (number)))

/* Macros for access to register in context structure.
 * Register given by offset in context structure.
 */
#define GDB_REG32(pcontext, offset )    (UINT32 *)((UINT32)pcontext + GDB_REG32_OFS(offset))
#define GDB_REG64(pcontext, offset )	(UINT64 *)((UINT32)pcontext + GDB_REG64_OFS(offset))

#endif /* _ASSEMBLER_ */

#endif /* #ifndef GDB_STUB_H */

