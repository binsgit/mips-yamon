/************************************************************************
 *
 *  fpustat.c
 *
 *  Functions to print out FPU emulator statistics
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

#ifdef ENABLE_SOFT_FPU
#include <asm/fpu_emulator.h>
#endif


static void (* printf)(char *fmt, ...);


static void printll(char *s, unsigned long long ll)
{
    printf("%s\t\t: %lu\n", s, (long)ll);
}


void FPUEMUL_stat_print(void (* print_func)(char *fmt, ...))
{
    printf = print_func;
#ifdef ENABLE_SOFT_FPU
    printll("FPU emulated", fpuemuprivate.stats.emulated);
    printll("FPU loads", fpuemuprivate.stats.loads);
    printll("FPU stores", fpuemuprivate.stats.stores);
    printll("FPU cp1ops", fpuemuprivate.stats.cp1ops);
    printll("FPU cp1xops", fpuemuprivate.stats.cp1xops);
    printll("FPU errors", fpuemuprivate.stats.errors);
#else
    printf("FPU emul not available\n");
#endif
}


void FPUEMUL_stat_clear(void (* putchar(char)))
{
#ifdef ENABLE_SOFT_FPU
    fpuemuprivate.stats.emulated = 0;
    fpuemuprivate.stats.loads = 0;
    fpuemuprivate.stats.stores = 0;
    fpuemuprivate.stats.cp1ops = 0;
    fpuemuprivate.stats.cp1xops = 0;
    fpuemuprivate.stats.errors = 0;
#endif
}
