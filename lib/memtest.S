
/************************************************************************
 *
 *  memtest.S
 *
 *  Memory test
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

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

		.globl	errordump

		.section .sbss
	
		ALIGN(2)

errordump:	.space	64	// 2 * 8 registers * 4 bytes

teststart:	.word 0
testend:	.word 0
testdummy:	.word 0
seed:		.word 0
retaddr:	.word 0

s0save:		.word 0
s1save:		.word 0
s2save:		.word 0
s3save:		.word 0
s4save:		.word 0
s5save:		.word 0
s6save:		.word 0
s7save:		.word 0
s8save:		.word 0


/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


#ifdef EB
#  define B0	3	/* byte 0 of word has address 3			*/
#  define B1	2	/* byte 1 of word has address 2			*/
#  define B2	1	/* byte 2 of word has address 1			*/
#  define B3	0	/* byte 3 of word has address 0			*/
#  define H0	2	/* halfword 0 of word has byte address 2	*/
#  define H1	0	/* halfword 1 of word has byte address 0	*/
#else
#  define B0	0	/* byte 0 of word has address 0	*/
#  define B1	1	/* byte 1 of word has address 1	*/
#  define B2	2	/* byte 2 of word has address 2	*/
#  define B3	3	/* byte 3 of word has address 3	*/
#  define H0	0	/* halfword 0 of word has byte address 0	*/
#  define H1	2	/* halfword 1 of word has byte address 2	*/
#endif

	
LEAF( mem_pattern_write )

                                // parameters:
				//	a0: teststart
				//	a1: testdummy
				//         (Entry: Address seed. Exit: new seed)
				//	a2: testend
				//	a3: seed
	sw	ra, retaddr
	sw	a2, testend
	sw	s8, s8save

	lw	t9, 0(a1)	// address seed

fill:
	jal	generate		// Fill all of memory
	jal	write0
	addu	a0, 32
	jal	generate
	jal	write1
	addu	a0, 32
	jal	generate
	jal	write2
	addu	a0, 32
	jal	generate
	jal	write3
	addu	a0, 32
	jal	generate
	jal	write4
	addu	a0, 32
	jal	generate
	jal	write5
	addu	a0, 32
	jal	generate
	jal	write6
	addu	a0, 32
	jal	generate
	jal	write7
	lw	t0, testend
	addu	a0, 32
	bne	a0, t0, fill
	nop
	sw	a3, 0(a1)		// save seed

	lw	s8, s8save
	lw	ra,retaddr
	j	ra			// All done

END( mem_pattern_write )


LEAF( mem_pattern_verify )

	sw	ra, retaddr
	sw	a0, teststart		// Save parameters
	sw	a1, testdummy		// a1: Entry: Addr seed. Exit: new seed
	sw	a2, testend
	sw	a3, seed

	sw	s0, s0save
	sw	s1, s1save
	sw	s2, s2save
	sw	s3, s3save
	sw	s4, s4save
	sw	s5, s5save
	sw	s6, s6save
	sw	s7, s7save
	sw	s8, s8save

	li	a2, 0x5a5a5a5a		// Dummy write pattern
	move	t9, a0			// address seed

verify:	jal	generate		// Test all of memory
	jal	read7
	jal	check
	addu	a0, 32
	jal	generate
	jal	read6
	jal	check
	addu	a0, 32
	jal	generate
	jal	read5
	jal	check
	addu	a0, 32
	jal	generate
	jal	read4
	jal	check
	addu	a0, 32
	jal	generate
	jal	read3
	jal	check
	addu	a0, 32
	jal	generate
	jal	read2
	jal	check
	addu	a0, 32
	jal	generate
	jal	read1
	jal	check
	addu	a0, 32
	jal	generate
	jal	read0
	jal	check
	addu	a0, 32
	lw	t0, testend
	bne	a0, t0, verify
	move	v0,zero			// All ok!
	lw	a1, testdummy
	sw	a3, 0(a1)		// save seed

return:	lw	s0, s0save		// Return with value in V0
	lw	s1, s1save
	lw	s2, s2save
	lw	s3, s3save
	lw	s4, s4save
	lw	s5, s5save
	lw	s6, s6save
	lw	s7, s7save
	lw	s8, s8save

	lw	ra,retaddr
	j	ra			// All done


 // Generate routine. Generates 8 pseudo-random numbers in t0-t7.
 // The numbers are generated from the 'address' t9 and the 'seed'
 // a3. a3 is modified upon return to contain the new seed.

generate:
	li	s8, 99991	// Multiplier constant
	li	t7, 1234567	// Adder constant

	multu	a3, s8		// Generate each of the eight numbers
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t0, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t1, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t2, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t3, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t4, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t5, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t6, a3

	multu	a3, s8
	mflo	a3
	addu	a3, t7
	xor	a3, t9
	move	t7, a3

	addu	t9, 32

	j	ra


 // Checks the numbers in t0-t7 against s0-s7. If they differ, an
 // errormessage is printed.

check:
	.set	noreorder
	bne	t0, s0, cerror
	nop
	bne	t1, s1, cerror
	nop
	bne	t2, s2, cerror
	nop
	bne	t3, s3, cerror
	nop
	bne	t4, s4, cerror
	nop
	bne	t5, s5, cerror
	nop
	bne	t6, s6, cerror
	nop
	bne	t7, s7, cerror
	nop
	j	ra
	nop
	.set	reorder

cerror:	la	v0, errordump
	sw	t0,  0(v0)
	sw	s0,  4(v0)
	sw	t1,  8(v0)
	sw	s1, 12(v0)
	sw	t2, 16(v0)
	sw	s2, 20(v0)
	sw	t3, 24(v0)
	sw	s3, 28(v0)
	sw	t4, 32(v0)
	sw	s4, 36(v0)
	sw	t5, 40(v0)
	sw	s5, 44(v0)
	sw	t6, 48(v0)
	sw	s6, 52(v0)
	sw	t7, 56(v0)
	sw	s7, 60(v0)
	move	v0, a0
	j	return


 // Writeroutines. Each of the following eight routines places the
 // values in t0-t7 in memory in eight different ways. The destination
 // area is 0(a0) up til 28(a0). Dummy writes are performed at 0(a1).

	.set	noreorder

write0:	sw	t0,  0(a0)
	sw	t1,  4(a0)

 //	sw	t2,  8(a0)	// Single write performed as four
	move	v0, t2		//  bytewrites
	srl	v1, v0, 8
	sb	v0, (8+B0)(a0)
	sb	v1, (8+B1)(a0)
	srl	v1, 8
	sb	v1, (8+B2)(a0)
	srl	v1, 8
	sb	v1, (8+B3)(a0)

	sw	t3, 12(a0)

 //	sw	t4, 16(a0)	// Write performed as two halfword writes
	sh	t4, (16+H0)(a0)
	srl	v0, t4, 16
	sh	v0, (16+H1)(a0)

	sw	t5, 20(a0)

 //	sw	t6, 24(a0)	// Write performed as two halfword writes
	srl	v0, t6, 16
	sh	v0, (24+H1)(a0)
	sh	t6, (24+H0)(a0)

	sw	t7, 28(a0)
	j	ra
	nop

write1:	sw	t0,  0(a0)
	nop
	sw	t6, 24(a0)
	nop
	sw	t2,  8(a0)
	nop
	sw	t5, 20(a0)
	nop
	sw	t7, 28(a0)
	nop
	sw	t3, 12(a0)
	nop
	sw	t1,  4(a0)
	j	ra
	sw	t4, 16(a0)

write2:	sw	t0,  0(a0)
	nop
	nop
	sw	t1,  4(a0)
	nop
	nop
	sw	t2,  8(a0)
	nop
	nop
	sw	t3, 12(a0)
	nop
	nop
	sw	t4, 16(a0)
	nop
	nop
	sw	t5, 20(a0)
	nop
	nop
	sw	t6, 24(a0)
	nop
	j	ra
	sw	t7, 28(a0)

write3:	sw	t0,  0(a0)
	lw	a2,  0(a1)
	sw	t1,  4(a0)
	lw	a2,  0(a1)
	sw	t2,  8(a0)
	lw	a2,  0(a1)
	sw	t3, 12(a0)
	lw	a2,  0(a1)
	sw	t4, 16(a0)
	lw	a2,  0(a1)
	sw	t5, 20(a0)
	lw	a2,  0(a1)
	sw	t6, 24(a0)
	lw	a2,  0(a1)
	sw	t7, 28(a0)
	j	ra
	nop

write4:	sw	t0,  0(a0)
	sw	t1,  4(a0)
	lw	a2,  0(a1)
	sw	t2,  8(a0)
	sw	t3, 12(a0)
	lw	a2,  0(a1)
	sw	t4, 16(a0)
	sw	t5, 20(a0)
	lw	a2,  0(a1)
	sw	t6, 24(a0)
	sw	t7, 28(a0)
	j	ra
	nop

write5:	sw	t0,  0(a0)
	lw	a2,  0(a1)
	nop
	sw	t1,  4(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t2,  8(a0)
	nop
	lw	a2,  0(a1)
	sw	t3, 12(a0)
	sw	t4, 16(a0)
	lw	a2,  0(a1)
	nop
	sw	t5, 20(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t6, 24(a0)
	nop
	lw	a2,  0(a1)
	sw	t7, 28(a0)
	j	ra
	nop

write6:	sw	t0,  0(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t3, 12(a0)
	sw	t4, 16(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t1,  4(a0)
	sw	t2,  8(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t5, 20(a0)
	sw	t7, 28(a0)
	lw	a2,  0(a1)
	lw	a2,  0(a1)
	sw	t6, 24(a0)
	j	ra
	nop

write7:	sw	t0,  0(a0)
	lw	a2,  0(a1)
	nop
	nop
	nop
	sw	t1,  4(a0)
	lw	a2,  0(a1)
	nop
	nop
	lw	a2,  0(a1)
	sw	t2,  8(a0)
	nop
	nop
	nop
	lw	a2,  0(a1)
	sw	t3, 12(a0)
	nop
	nop
	sw	t4, 16(a0)
	lw	a2,  0(a1)
	nop
	nop
	nop
	sw	t5, 20(a0)
	lw	a2,  0(a1)
	nop
	nop
	lw	a2,  0(a1)
	sw	t6, 24(a0)
	nop
	nop
	nop
	lw	a2,  0(a1)
	sw	t7, 28(a0)
	j	ra
	nop

	.set	reorder

 // Readroutines. Each of the following eight routines reads data
 // in memory to s0-s7 in eight different ways. The source area 
 // is 0(a0) up til 28(a0). Dummy writes are performed at 0(a1).

	.set	noreorder

read0:	lw	s0,  0(a0)
	lw	s1,  4(a0)

 //	lw	s2,  8(a0)	// Read performed as four byte-reads
	lbu	s2, (8+B3)(a0)
	lbu	v0, (8+B2)(a0)
	sll	s2, 24
	sll	v0, 16
	lbu	v1, (8+B1)(a0)
	addu	s2, v0
	sll	v1, 8
	lbu	v0, (8+B0)(a0)
	addu	s2, v1
	addu	s2, v0

	lw	s3, 12(a0)

 //	lw	s4, 16(a0)	// Read performed as two halfword reads
	lhu	s4, (16+H1)(a0)
	nop
	lhu	v0, (16+H0)(a0)
	sll	s4, 16
	addu	s4, v0

	lw	s5, 20(a0)

 //	lw	s6, 24(a0)	// Read performed as two halfword reads
	lhu	v0, (24+H0)(a0)
	lhu	s6, (24+H1)(a0)
	nop
	sll	s6, 16
	addu	s6, v0

	lw	s7, 28(a0)
	j	ra
	nop

read1:	lw	s0,  0(a0)
	nop
	lw	s6, 24(a0)
	nop
	lw	s2,  8(a0)
	nop
	lw	s5, 20(a0)
	nop
	lw	s7, 28(a0)
	nop
	lw	s3, 12(a0)
	nop
	lw	s1,  4(a0)
	j	ra
	lw	s4, 16(a0)

read2:	lw	s0,  0(a0)
	nop
	nop
	lw	s1,  4(a0)
	nop
	nop
	lw	s2,  8(a0)
	nop
	nop
	lw	s3, 12(a0)
	nop
	nop
	lw	s4, 16(a0)
	nop
	nop
	lw	s5, 20(a0)
	nop
	nop
	lw	s6, 24(a0)
	nop
	j	ra
	lw	s7, 28(a0)

read3:	lw	s0,  0(a0)
	sw	a2,  0(a1)
	lw	s1,  4(a0)
	sw	a2,  0(a1)
	lw	s2,  8(a0)
	sw	a2,  0(a1)
	lw	s3, 12(a0)
	sw	a2,  0(a1)
	lw	s4, 16(a0)
	sw	a2,  0(a1)
	lw	s5, 20(a0)
	sw	a2,  0(a1)
	lw	s6, 24(a0)
	sw	a2,  0(a1)
	lw	s7, 28(a0)
	j	ra
	nop

read4:	lw	s0,  0(a0)
	lw	s1,  4(a0)
	sw	a2,  0(a1)
	lw	s2,  8(a0)
	lw	s3, 12(a0)
	sw	a2,  0(a1)
	lw	s4, 16(a0)
	lw	s5, 20(a0)
	sw	a2,  0(a1)
	lw	s6, 24(a0)
	lw	s7, 28(a0)
	j	ra
	nop

read5:	lw	s0,  0(a0)
	sw	a2,  0(a1)
	nop
	lw	s1,  4(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s2,  8(a0)
	nop
	sw	a2,  0(a1)
	lw	s3, 12(a0)
	lw	s4, 16(a0)
	sw	a2,  0(a1)
	nop
	lw	s5, 20(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s6, 24(a0)
	nop
	sw	a2,  0(a1)
	lw	s7, 28(a0)
	j	ra
	nop

read6:	lw	s0,  0(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s3, 12(a0)
	lw	s4, 16(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s1,  4(a0)
	lw	s2,  8(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s5, 20(a0)
	lw	s7, 28(a0)
	sw	a2,  0(a1)
	sw	a2,  0(a1)
	lw	s6, 24(a0)
	j	ra
	nop

read7:	lw	s0,  0(a0)
	sw	a2,  0(a1)
	nop
	nop
	nop
	lw	s1,  4(a0)
	sw	a2,  0(a1)
	nop
	nop
	sw	a2,  0(a1)
	lw	s2,  8(a0)
	nop
	nop
	nop
	sw	a2,  0(a1)
	lw	s3, 12(a0)
	nop
	nop
	lw	s4, 16(a0)
	sw	a2,  0(a1)
	nop
	nop
	nop
	lw	s5, 20(a0)
	sw	a2,  0(a1)
	nop
	nop
	sw	a2,  0(a1)
	lw	s6, 24(a0)
	nop
	nop
	nop
	sw	a2,  0(a1)
	lw	s7, 28(a0)
	j	ra
	nop

END( mem_pattern_verify )


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/
