
/************************************************************************
 *
 *  sys_cpu_s.S
 *
 *  cpu functions
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
#include <sys_api.h>
#include <mips.h>
#include <qed.h>
	
/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

	.set noreorder
			
/************************************************************************
 *
 *                          sys_cpu_icache_config
 *  Description :
 *  -------------
 *
 *  Configure icache
 *
 *  a0 holds the requested sets per way field
 *  a1 holds the requested line size (0 or hardware default) field
 *  a2 holds the requested associativity field
 *  a3 holds the reset value of CONFIG1
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cpu_icache_config )

	/* Reserve space on stack */
	addiu	sp, -8*4

	/* Store return address, s0..s4 */
	sw	ra, 0(sp)
	sw	s0, 4(sp)
	sw	s1, 8(sp)
	sw	s2, 12(sp)
	sw	s3, 16(sp)
	sw	s4, 20(sp)
	sw	s5, 24(sp)
	sw	s6, 28(sp)

	/* Copy registers to preserved registers */
	move	s0, a0
	move	s1, a1
	move	s2, a2
	move	s3, a3
	MFC0(   s4, C0_Status )
	MFC0(   s5, C0_Config )
	MFC0_SEL_OPCODE( R_s6, R_C0_Config1, R_C0_SelConfig1 )

	/* Disable interrupts */
	li	t0, ~M_StatusIE
	and	t0, s4, t0
	MTC0(   t0, C0_Status )
	
	/* Shift to uncached */
	la	t0, sys_cpu_icache_config_uncached
	KSEG1A( t0)
	jr	t0
	nop

sys_cpu_icache_config_uncached :
			       
	/* Enable write access to config1 */
	li	t0, C0_CONFIG_WC_BIT
	or	t0, s5, t0
	MTC0(	t0, C0_Config )

	/* Set Icache settings to reset values */
	li	t0, (M_Config1IS | M_Config1IL | M_Config1IA)
	and	t0, s3, t0
	li	t1, ~(M_Config1IS | M_Config1IL | M_Config1IA)
	and	t1, s6, t1
	or	t1, t0

	/* Write CONFIG1 register */
	MTC0_SEL_OPCODE( R_t1, R_C0_Config1, R_C0_SelConfig1 )

	/* Empty pipeline */
	nop; nop; nop; nop; nop; nop

	/* Prepare for initialising cache */
	
	/* a1 := line size (bytes) */
	la      a1, sys_icache_linesize
	lw	a1, 0(a1)

	/* a0 := cache size (bytes) */
	la      a0, sys_icache_lines
	lw	a0, 0(a0)
	multu	a0, a1
	mflo    a0

	/* Initialise */
	la	t0, sys_init_icache
	KSEG1A( t0)		/* Make it uncached */
	jal	t0
	nop

	/* Set Icache settings to requested values */
	li	t0, ~(M_Config1IS | M_Config1IL | M_Config1IA)
	and	t0, s6, t0

	li	t1, S_Config1IS
	sllv	t1, s0, t1
	or	t0, t1

	li	t1, S_Config1IL
	sllv	t1, s1, t1
	or	t0, t1

	li	t1, S_Config1IA
	sllv	t1, s2, t1
	or	t0, t1

	/* Write CONFIG1 register */
	MTC0_SEL_OPCODE( R_t0, R_C0_Config1, R_C0_SelConfig1 )

	/* Disable write access to config1 */
	MTC0(	s5, C0_Config )

	/* Empty pipeline */
	nop; nop; nop; nop; nop; nop

	/* Restore CP0 STATUS */
	MTC0(   s4, C0_Status )

	/* Return */
	lw	ra, 0(sp)
	lw	s0, 4(sp)
	lw	s1, 8(sp)
	lw	s2, 12(sp)
	lw	s3, 16(sp)
	lw	s4, 20(sp)
	lw	s5, 24(sp)
	lw	s6, 28(sp)

	jr	ra
	addiu	sp, 8*4

END( sys_cpu_icache_config )


		
/************************************************************************
 *
 *                          sys_cpu_dcache_config
 *  Description :
 *  -------------
 *
 *  Configure dcache
 *
 *  a0 holds the requested sets per way field
 *  a1 holds the requested line size (0 or hardware default) field
 *  a2 holds the requested associativity field
 *  a3 holds the reset value of CONFIG1
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cpu_dcache_config )

	/* Reserve space on stack */
	addiu	sp, -8*4

	sw	ra, 0(sp)
	sw	s0, 4(sp)
	sw	s1, 8(sp)
	sw	s2, 12(sp)
	sw	s3, 16(sp)
	sw	s4, 20(sp)
	sw	s5, 24(sp)
	sw	s6, 28(sp)

	/* Copy registers to preserved registers */
	move	s0, a0
	move	s1, a1
	move	s2, a2
	move	s3, a3
	MFC0(   s4, C0_Status )
	MFC0(   s5, C0_Config )
	MFC0_SEL_OPCODE( R_s6, R_C0_Config1, R_C0_SelConfig1 )
	
	/* Disable interrupts */
	li	t0, ~M_StatusIE
	and	t0, s4, t0
	MTC0(   t0, C0_Status )
		
	/* Flush D-Cache */
	addiu	sp, -4*4
	la      t0, sys_dcache_flush_all
	jal	t0
	nop
	addiu	sp, 4*4

	/* Shift to uncached */
	la	t0, sys_cpu_dcache_config_uncached
	KSEG1A( t0)
	jr	t0
	nop

sys_cpu_dcache_config_uncached :

	/* Enable write access to config1 */
	li	t0, C0_CONFIG_WC_BIT
	or	t0, s5, t0
	MTC0(	t0, C0_Config )

	/* Set Dcache settings to reset values */
	li	t0, (M_Config1DS | M_Config1DL | M_Config1DA)
	and	t1, s3, t0
	li	t0, ~(M_Config1DS | M_Config1DL | M_Config1DA)
	and	t2, s6, t0
	or	t2, t1

	/* Write CONFIG1 register */
	MTC0_SEL_OPCODE( R_t2, R_C0_Config1, R_C0_SelConfig1 )

	/* Empty pipeline */
	nop; nop; nop; nop; nop; nop

	/* Prepare for initialising cache */

	/* a2 := processor type */
	la	a2, sys_processor
	lw	a2, 0(a2)

	/* a1 := line size (bytes) */
	la      a1, sys_dcache_linesize
	lw	a1, 0(a1)

	/* a0 := cache size (bytes) */
	la      a0, sys_dcache_lines
	lw	a0, 0(a0)
	multu	a0, a1
	mflo    a0

	/* Initialise */
	la	t0, sys_init_dcache
	KSEG1A( t0)		/* Make it uncached */
	jal	t0
	nop
	
	/* Set Dcache settings to requested values */
	li	t0, ~(M_Config1DS | M_Config1DL | M_Config1DA)
	and	t0, s6, t0

	li	t1, S_Config1DS
	sllv	t1, s0, t1
	or	t0, t1

	li	t1, S_Config1DL
	sllv	t1, s1, t1
	or	t0, t1

	li	t1, S_Config1DA
	sllv	t1, s2, t1
	or	t0, t1

	/* Write CONFIG1 register */
	MTC0_SEL_OPCODE( R_t0, R_C0_Config1, R_C0_SelConfig1 )

	/* Disable write access to config1 */
	MTC0(	s5, C0_Config )

	/* Empty pipeline */
	nop; nop; nop; nop; nop; nop

	/* Restore CP0 STATUS */
	MTC0(   s4, C0_Status )

	/* Return */
	lw	ra, 0(sp)
	lw	s0, 4(sp)
	lw	s1, 8(sp)
	lw	s2, 12(sp)
	lw	s3, 16(sp)
	lw	s4, 20(sp)
	lw	s5, 24(sp)
	lw	s6, 28(sp)

	jr	ra
	addiu	sp, 8*4
	
END( sys_cpu_dcache_config )
		


/************************************************************************
 *
 *                          sys_cpu_mmu_config
 *  Description :
 *  -------------
 *
 *  Configure MMU
 *
 *  a0 holds the required setting of Config0[8]
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cpu_mmu_config )

	/* Disable interrupts (store old value of CP0 STATUS) */
	MFC0(   t0, C0_Status )
	li	t1, ~M_StatusIE
	and	t1, t0, t1
	MTC0(   t1, C0_Status )

	/* Enable write access to Config0[8] */
	MFC0(   t1, C0_Config )
	li	t2, C0_CONFIG_WC_BIT
	or	t1, t1, t2
	MTC0(	t1, C0_Config )

	/* Set bit 8 */
	li	t2, ~(1 << 8)
	and	t1, t2
	sll	a0, 8
	or	t1, a0
	MTC0(   t1, C0_Config )

	/* Disable write access to Config1[8] */
	li	t2, ~C0_CONFIG_WC_BIT
	and	t1, t2
	MTC0(   t1, C0_Config )	

	/* Empty pipeline */
	nop; nop; nop; nop; nop; nop

	/* Restore CP0 STATUS */
	MTC0(   t0, C0_Status )

	/* Return */
	jr	ra
	nop
		
END( sys_cpu_mmu_config )



/************************************************************************
 *
 *                          sys_cpu_configurability
 *  Description :
 *  -------------
 *
 *  Determine cpu configurability
 *
 *  This feature is present specifically to support configuration
 *  testing of the core in a lead vehicle, and is not supported
 *  in any other environment.  Attempting to use this feature
 *  outside of the scope of a lead vehicle is a violation of the
 *  MIPS Architecture, and may cause unpredictable operation of
 *  the processor.
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = mask with the following bit definitions :
 *
 *  Bit 0 : Set if MMU may be configured (TLD -> Fixed)
 *  Bit 1 : Set if cache may be downsized
 *
 ************************************************************************/
LEAF( sys_cpu_configurability )

	/* Reserve space on stack */
	addiu	sp, -2*4

	/* Store return address */
	sw	ra, 0(sp)

	/* Disable interrupts (store old value of CP0 STATUS) */
	MFC0(   t0, C0_Status )
	sw	t0, 4(sp)
	li	t1, ~M_StatusIE
	and	t1, t0, t1
	MTC0(   t1, C0_Status )
		
	/* Shift to uncached */
	la	t0, sys_cpu_configurability_uncached
	KSEG1A( t0)
	jr	t0
	nop

sys_cpu_configurability_uncached:

	/* Set default = Not configurable */
	move    v0, zero

	/* Check that Config0[WC] bit is cleared */
	MFC0(   t1, C0_Config )
	li	t2, C0_CONFIG_WC_BIT
	and	t3, t1, t2
	bne	t3, zero, done
	nop

	/* Set Config0[WC] bit */
	or      t1, t2
	MTC0(   t1, C0_Config )

	/* Check Config0[8] : MMU configurability */
	li	t2, (1 << 8)
	and	t3, t1, t2
	bne	t3, zero, 1f
	nop
	/*  Config0[8] == 0 => TLB.
	 *  Try to set bit and read it back.
	 */
	or	t3, t1, t2
	MTC0(   t3, C0_Config )
	MFC0(   t3, C0_Config )
	and	t3, t3, t2
	beq	t3, zero, 1f
	nop
	/*  Bit was set, so MMU may be configured.
	 *  Clear bit and set mask.
	 */	
	MTC0(   t1, C0_Config )
	ori	v0, 1

1:
	/* Determine cache configurability
	 *
	 * Read CONFIG1 register, which holds implementation data
	 */
	MFC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )     
#define config1	t9

	/*  Check IL (I-cache line size) field.
	 *  If 0, there is no (I)cache.
	 */
	li      t2, M_Config1IL
	and	t3, t9, t2
	beq	t3, zero, done
	nop

	/* Try to clear IL and read it back */
	li      t2, ~M_Config1IL
	and	t3, t9, t2
	MTC0_SEL_OPCODE( R_t3, R_C0_Config1, R_C0_SelConfig1 )
	MFC0_SEL_OPCODE( R_t3, R_C0_Config1, R_C0_SelConfig1 )     
	li	t2, M_Config1IL
	and	t3, t2
	bne	t3, zero, done
	nop
	/*  IL was cleared, so CACHE may be configured.
	 *  Restore setting and set mask.
	 */
	MTC0_SEL_OPCODE( R_t9, R_C0_Config1, R_C0_SelConfig1 )
	ori     v0, 2

done:
	/* Clear WC bit */
	li	t2, ~C0_CONFIG_WC_BIT	
	and	t1, t2
	MTC0(   t1, C0_Config )

	/***** Reinitialise I-cache *****/

	/* a1 := line size (bytes) */
	la      a1, sys_icache_linesize
	lw	a1, 0(a1)

	/* a0 := cache size (bytes) */
	la      a0, sys_icache_lines
	lw	a0, 0(a0)
	multu	a0, a1
	mflo    a0

	/* Initialise */
	la	t0, sys_init_icache
	KSEG1A( t0)		/* Make it uncached */
	jal	t0
	nop
	
	/* Restore CP0 STATUS */
	lw	t0, 4(sp)
	MTC0(   t0, C0_Status )

	/* Return */
	lw	ra, 0(sp)
	jr	ra
	addiu	sp, 2*4

END( sys_cpu_configurability )
	

/************************************************************************	
 *
 *                          sys_cpu_type
 *  Description :
 *  -------------
 *
 *  Determine whether we have :
 *
 *  a) 32 or 64 bit CPU
 *  b) MIPS32/MIPS64 CPU or other type.
 *  c) Support for MIPS16e ASE or not.
 *  d) Support for EJTAG or not.
 *  e) Support for FPU or not.
 *  f) Support for hardware cache coherency or not.
 *
 *  Parameters :
 *  ------------
 *
 *  None

 *  Return values :
 *  ---------------
 *
 *  Following global variables are set :
 *
 *  sys_64bit
 *  sys_mips32_64
 *  sys_mips16e
 *  sys_ejtag
 *  sys_arch_rev
 *  sys_fpu
 *  sys_cpu_cache_coherency
 *  sys_eicmode
 *  sys_mt
 *
 ************************************************************************/
LEAF(sys_cpu_type)

	/* Determine processor */
	MFC0(   t0, C0_PRId)
	li	t1, M_PRIdImp | M_PRIdCoID
	and     t0, t1

	/* MIPS 20Kc and 25Kf have support for hw cache coherency */
	li	t1, MIPS_20Kc
	beq	t1, t0, 1f
	li	t2, 1
	li	t1, MIPS_25Kf
	beq	t1, t0, 1f
	nop
	move	t2, zero
1:
	la	t1, sys_cpu_cache_coherency
	sb	t2, 0(t1)
		
	/* Check if it is a MIPS32/64 processor */	
	li      t1, M_PRIdCoID
	and	t1, t0
	srl	t1, S_PRIdCoID
	li	t2, C0_PRID_COMP_NOT_MIPS32_64
	bne	t1, t2, mips32_64
	nop

	/* Not a MIPS32/64 processor */
	la      t1, sys_mips32_64
	sb	zero, 0(t1)
	la	t1, sys_mips16e
	sb	zero, 0(t1)
	la	t1, sys_ejtag
	sb	zero, 0(t1)
	la	t1, sys_arch_rev
	sb	zero, 0(t1)
	la	t1, sys_smallpage_support
	sb	zero, 0(t1)
	la	t1, sys_mt
	sb	zero, 0(t1)
	la	t1, sys_dsp
	sb	zero, 0(t1)
	
	li	t1, QED_RM52XX
	beq	t1, t0, 1f
	li	t2, 1
	li	t1, QED_RM70XX
	beq	t1, t0, 1f
	nop
	move	t2, zero	/* 32 bit, no FPU by default */
1:
	la	t0, sys_64bit
	sb	t2, 0(t0)
	b	mips_fir_in_t1
	move	t1,t2

mips32_64:

	/* MIPS32/MIPS64 processor */
	la      t1, sys_mips32_64
	li	t0, 1
	sb	t0, 0(t1)

	MFC0(   t0, C0_Config )

	/* Determine architecture revision level */
	li	t1, M_ConfigAR
	and	t1, t0
	srl	t1, S_ConfigAR
	la	t2, sys_arch_rev
	sb	t1, 0(t2)
	
	/* Determine 32/64 based on AT field of CONFIG register */	
	li	t1, M_ConfigAT
	and	t0, t1
	srl	t0, S_ConfigAT
	li	t1, K_ConfigAT_MIPS32
	beq	t0, t1, 1f
	move	t0, zero
	li	t0, 1
1:
	la	t1, sys_64bit
	sb	t0, 0(t1)

	/* Read CONFIG1 register, which holds implementation data */
	MFC0_SEL_OPCODE( R_t0, R_C0_Config1, R_C0_SelConfig1 )     

	/* Determine mips16e, ejtag, fpusupport based on CONFIG1 */
	srl     t1, t0, S_Config1CA
	and	t1, 1
	la	t2, sys_mips16e
	sb	t1, 0(t2)

	srl     t1, t0, S_Config1EP
	and	t1, 1
	la	t2, sys_ejtag
	sb	t1, 0(t2)

	/* Determine small page, EIC & MT support (requires CONFIG3 to be there) */
	move	t4, zero
	move	t5, zero
	sll	t1, t0, 31 - S_Config1M
	move	t3, zero
	bgez	t1, 1f
	 move	t1, zero
	MFC0_SEL_OPCODE( R_t2, R_C0_Config2, R_C0_SelConfig2 )     
	sll	t1, t2, 31 - S_Config2M
	bgez	t1, 1f
	 move	t1, zero
	MFC0_SEL_OPCODE( R_t2, R_C0_Config3, R_C0_SelConfig3 )     
	srl	t1, t2, S_Config3SP
	and	t1, 1
	srl	t3, t2, S_Config3VEIC
	and	t3, 1
	srl	t4, t2, S_Config3MT
	and	t4, 1
	srl	t5, t2, S_Config3DSPP
	and	t5, 1
		
1:
	la	t2, sys_smallpage_support
	sb	t1, 0(t2)

	la	t2, sys_eicmode
	sb	t3, 0(t2)

	la	t2, sys_mt
	sb	t4, 0(t2)

	la	t2, sys_dsp
	sb	t5, 0(t2)
		
	/* FPU */
	srl     t1, t0, S_Config1FP
	and	t1, 1

mips_fir_in_t1:
	bne	t1, zero, 2f
	nop

	/* FPU not implemented: Clear CP0.status.FR bit */
	MFC0(   t0, C0_Status)
	li	t1, ~M_StatusFR
	and	t1, t0
	MTC0(   t1, C0_Status)
	b	1f
	move	t1, zero

2:
	/* FPU implemented: Read CP1 fir value */
	MFC0(   t0, C0_Status)
	li	t1, M_StatusCU1
	or	t1, t0
	MTC0(   t1, C0_Status)

	cfc1    t1, $0

	/* restore status reg */
	MTC0(   t0, C0_Status)
	bne	t1,zero, 1f
	nop
	or	t1, 1		/* make sure that fir != 0 */
1:
	la	t2, sys_fpu
	sw	t1, 0(t2)

        jr	ra
	nop

END(sys_cpu_type)

				
/************************************************************************	
 *
 *                          sys_cpu_l2_enable
 *  Description :
 *  -------------
 *
 *  Enable/disable L2 cache
 *
 *  a0 = 0 -> disable, else enable
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cpu_l2_enable )

	MFC0(   t1, C0_PRId)
	and     t0, t1, (M_PRIdCoID|M_PRIdImp)
	beq	t0, QED_RM70XX, 1f
	 nop
	
	and     t0, t1, M_PRIdCoID
	/* Check if it is a MIPS32/64 processor */	
	bne	t0, (C0_PRID_COMP_NOT_MIPS32_64 << S_PRIdCoID), 1f
	 nop
	
	/* No L2 enable/disable support */
	jr	ra
	 nop

1:	
	/* Reserve space on stack */
	subu	sp, 4*4

	/* Store registers */
	sw	ra, 0(sp)
	sw	s0, 4(sp)
	sw	s1, 8(sp)
	sw	s2, 12(sp)

	/* Copy registers to preserved registers */
	move	s0, a0
	
	/* Disable interrupts */
	MFC0(   s1, C0_Status )	
	and	t0, s1, ~M_StatusIE
	MTC0(   t0, C0_Status )

	move	s2, t1		# PRID
	
	/* Flush D-Cache */
	subu	sp, 4*4
	la      t0, sys_dcache_flush_all
	jal	t0
	 nop
	addu	sp, 4*4

	/* Get L2 cache size */
	lw	a0, sys_l2cache_lines
	lw	a1, sys_l2cache_linesize
	multu	a0, a1
	mflo	a0
	
	/* Shift to uncached */
	la	t0, sys_cpu_l2_enable_uncached
	KSEG1A( t0)
	jr	t0
	 nop

sys_cpu_l2_enable_uncached :

	beq	s0, zero, 2f
	 nop

	and	t0, s2,(M_PRIdCoID|M_PRIdImp)
	beq	t0, QED_RM70XX, sys_cpu_l2_enable_rm7061
	 nop
	
sys_cpu_l2_enable_mips:
	/* Enable L2 cache */
	MFC0_SEL_OPCODE( R_t0, R_C0_Config2, R_C0_SelConfig2 )     
	and	t0, ~M_Config2L2B
	MTC0_SEL_OPCODE( R_t0, R_C0_Config2, R_C0_SelConfig2 )
	b	sys_cpu_l2_enable_init
	 nop

sys_cpu_l2_enable_rm7061:	
	/* Enable L2 cache */
	MFC0(   t0, C0_Config )
	or	t0, C0_CONFIG_QED_RM70XX_SE_BIT
	MTC0(   t0, C0_Config )	
	
sys_cpu_l2_enable_init:	
	/* Initialise L2 cache */
	la	t0, sys_init_l2cache_flash
	KSEG1A( t0)
	jalr	t0
	 nop

	b	3f
	 nop

2:		
	and	t0, s2,(M_PRIdCoID|M_PRIdImp)
	beq	t0, QED_RM70XX, sys_cpu_l2_disable_rm7061
	 nop
	
sys_cpu_l2_disable_mips:
	/* Disable L2 cache */
	MFC0_SEL_OPCODE( R_t0, R_C0_Config2, R_C0_SelConfig2 )     
	or	t0, M_Config2L2B
	MTC0_SEL_OPCODE( R_t0, R_C0_Config2, R_C0_SelConfig2 )
	b	3f
	 nop
	
sys_cpu_l2_disable_rm7061:	
	/* Disable L2 cache */
	MFC0(   t0, C0_Config )
	and	t0, ~C0_CONFIG_QED_RM70XX_SE_BIT
	MTC0(   t0, C0_Config )	
	
3:
	/* Restore CP0 STATUS */
	MTC0(   s1, C0_Status )

	/* Restore registers */
	lw	ra, 0(sp)
	lw	s0, 4(sp)
	lw	s1, 8(sp)
	lw	s2, 12(sp)

	jr	ra
	addiu	sp, 4*4
	
END( sys_cpu_l2_enable )	

/************************************************************************	
 *
 *                          sys_cpu_l2_enabled
 *  Description :
 *  -------------
 *
 *  Return enable status of L2 cache
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = 0 -> disabled, else enabled
 *
 ************************************************************************/
LEAF( sys_cpu_l2_enabled )

	MFC0(   t1, C0_PRId)
	and     t0, t1, (M_PRIdCoID|M_PRIdImp)
	beq	t0, QED_RM70XX, sys_cpu_l2_enabled_rm7061
	 nop
	
	and     t0, t1, M_PRIdCoID
	/* Check if it is a MIPS32/64 processor */	
	bne	t0, (C0_PRID_COMP_NOT_MIPS32_64 << S_PRIdCoID), sys_cpu_l2_enabled_mips
	 nop
	
	/* No L2 enable/disable support, assume it is enabled*/
	li	v0, 1
	jr	ra
	 nop

sys_cpu_l2_enabled_mips:
	MFC0_SEL_OPCODE( R_t0, R_C0_Config2, R_C0_SelConfig2 )     
	and	t0, M_Config2L2B
	j	ra
	 seq	v0, t0, zero

sys_cpu_l2_enabled_rm7061:	
	MFC0(   t0, C0_Config )
	and	t0, C0_CONFIG_QED_RM70XX_SE_BIT
	jr	ra
	 sne	v0, t0, zero
	
END( sys_cpu_l2_enabled )
	

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/




	
