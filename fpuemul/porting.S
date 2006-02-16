/************************************************************************
 *
 *  porting.S
 *
 *  Miscellaneous functions to allow porting Linux FPU emulator to
 *  YAMON and other environments
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

#include <ArchDefs.h>

#ifdef EL
#define LW1	a0
#define UW1	a1
#define LW2	a2
#define UW2	a3
#else
#define LW1	a1
#define UW1	a0
#define LW2	a3
#define UW2	a2
#endif

	.text
	.set noreorder
	.set noat


  	.globl	verify_area
  	.ent	verify_area
verify_area:
	jr	ra
	move	v0, zero
  	.end	verify_area


  	.globl	flush_cache_sigtramp
  	.ent	flush_cache_sigtramp
flush_cache_sigtramp:
	la	v0, sys_64bit
	lb	v0, 0(v0)
	beqz	v0, 1f
	sub	sp, 32
#ifndef _GHS_
.set push
.set mips3
#endif
	dsll	LW1, 32
	dsrl	LW1, 32
	dsll	UW1, 32
	daddu	a0, LW1, UW1
	sw	ra, 24(sp)
	jal	sys_dcache_flush_addr
	sd	a0, 16(sp)
	lw	ra, 24(sp)
	ld	a0, 16(sp)
#ifndef _GHS_
.set pop
#endif
	li	a1, 1
	j	sys_icache_invalidate_addr
	add	sp, 32
1:	sw	ra, 24(sp)
	jal	sys_dcache_flush_addr
	sw	a0, 16(sp)
	lw	ra, 24(sp)
	lw	a0, 16(sp)
	li	a1, 1	
	j	sys_icache_invalidate_addr
	add	sp, 32
  	.end	flush_cache_sigtramp




