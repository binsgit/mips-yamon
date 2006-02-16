
/************************************************************************
 *
 *  cpu.S
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
#include <mips.h>
#include <sys_api.h>

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
 *                          sys_tlb_lookup
 *  Description :
 *  -------------
 *
 *  Probe TLB for matching entry
 *
 *  a0 holds the virtual address.
 *  a1 holds pointer to an UINT32, which is set to the mapped address.	 
 *  a2 holds pointer to an UINT32, which is set to the pagesize if a
 *     match is found.
 *	
 *  Return values :
 *  ---------------
 *
 *  SYS_TLB_NOTFOUND : No match
 *  SYS_TLB_NOTVALID : Match with valid bit cleared, i.e. not valid
 *  SYS_TLB_WP	     : Match with dirty bit cleared, i.e. write-protected
 *  SYS_TLB_OK	     : Valid and Dirty entry found
 *
 ************************************************************************/
LEAF(sys_tlb_lookup)

	/* Determine whether small pages (1kB pages) are enabled
	 * (MIPS32/64 Release 2 CPUs only).
	 */
	la	t0, sys_smallpage_support
	lb	t0, 0(t0)
	beq	t0, zero, 1f
	nop
	MFC0_SEL_OPCODE( 8 /* t0 */, R_C0_PageGrain, R_C0_SelPageGrain )
	sll	  t0, 31 - S_PageGrainESP
	bltz      t0, sys_tlb_lookup_small
	nop

1:	
	/**** Setup EntryHi ****/

	/* VPN2 */
	li	t0, MSK(19) << 13
	and	t1, a0, t0

	/* ASID */
	MFC0(   t0, C0_EntryHi )
	li	t2, M_EntryHiASID
	and	t0, t2

	or	t1, t0

	MTC0(   t1, C0_EntryHi )

	/**** Probe ****/
	tlbp
	NOPS
	MFC0(   t0, C0_Index )
	
	bgez    t0, entry_found
	nop
	
	/* Not found */		
	li      v0, SYS_TLB_NOTFOUND
	jr      ra
	nop

entry_found:	

	/* Read entry */
	tlbr    
	NOPS

	/* Determine page size */
	MFC0(   t0, C0_PageMask )
	li	t1, MSK( S_PageMaskMask )
	or	t0, t1
	addiu	t0, 1
	srl	t0, t0, 1

	/* Store result */
	sw	t0, 0(a2)

	/**** Determine mapped address ****/
	
	/*  Even/odd page based on Virtual Address bit N,
	 *  where 2^N = pagesize (currently in t0 register).
	 */

	and	t1, t0, a0
	beq	t1, zero, even_page
	nop

odd_page:
	MFC0(   t1, C0_EntryLo1 )
	b	calc_pfn
	nop

even_page:
	MFC0(   t1, C0_EntryLo0 )		

calc_pfn:
	/* Determine PFN */
	srl	t2, t1, S_EntryLoPFN
	sll     t2, t2, 12

	/*  Least Significant Bits are taken from virtual address.
	 *  t0 = pagesize, t2 = PFN, a0 = virtual address
	 */
	addiu   t0, -1        /* Mask with ones for the lsbits */
	nor	t3, t0, zero  /* Mask with ones for the msbits */
	and	t2, t3	      /* Clear lsbits		       */
	and	t0, a0	      /* Get lsbits from virtual addr  */
	or	t2, t0	      /* Setup lsbits		       */

	/* Store result */
	sw	t2, 0(a1)

	/* Determine setting of Valid bit */
	li	t0, M_EntryLoV
	and	t2, t1, t0
	bne	t2, zero, tlb_valid
	nop

	/* Not valid */
	li      v0, SYS_TLB_NOTVALID
	jr	ra 
	nop

tlb_valid :
	/* Determine setting of Dirty bit */
	li	t0, M_EntryLoD
	and	t2, t1, t0
	bne	t2, zero, tlb_ok
	nop

	/* Write protected */
	li      v0, SYS_TLB_WP
	jr	ra 
	nop

tlb_ok :
	li	v0, SYS_TLB_OK	
	jr	ra
	nop

END(sys_tlb_lookup)

	
/************************************************************************
 *
 *                          sys_tlb_lookup_small
 *  Description :
 *  -------------
 *
 *  Same as sys_tlb_lookup except this is used when small pages (1kB)
 *  are enabled (MIPS32/MIPS64 Release 2 CPU only).
 *
 *  Probe TLB for matching entry
 *
 *  a0 holds the virtual address.
 *  a1 holds pointer to an UINT32, which is set to the mapped address.	 
 *  a2 holds pointer to an UINT32, which is set to the pagesize if a
 *     match is found.
 *	
 *  Return values :
 *  ---------------
 *
 *  SYS_TLB_NOTFOUND : No match
 *  SYS_TLB_NOTVALID : Match with valid bit cleared, i.e. not valid
 *  SYS_TLB_WP	     : Match with dirty bit cleared, i.e. write-protected
 *  SYS_TLB_OK	     : Valid and Dirty entry found
 *
 ************************************************************************/
SLEAF(sys_tlb_lookup_small)

	/**** Setup EntryHi ****/

	/* VPN2 */
	li	t0, MSK(21) << 11
	and	t1, a0, t0

	/* ASID */
	MFC0(   t0, C0_EntryHi )
	li	t2, M_EntryHiASID
	and	t0, t2

	or	t1, t0

	MTC0(   t1, C0_EntryHi )

	/**** Probe ****/
	tlbp
	NOPS
	MFC0(   t0, C0_Index )
	
	bgez    t0, entry_found_small
	nop
	
	/* Not found */		
	li      v0, SYS_TLB_NOTFOUND
	jr      ra
	nop

entry_found_small:	

	/* Read entry */
	tlbr    
	NOPS

	/* Determine page size */
	MFC0(   t0, C0_PageMask )
	li	t1, MSK( S_PageMaskMaskX )
	or	t0, t1
	addiu	t0, 1
	srl	t0, t0, 1

	/* Store result */
	sw	t0, 0(a2)

	/**** Determine mapped address ****/
	
	/*  Even/odd page based on Virtual Address bit N,
	 *  where 2^N = pagesize (currently in t0 register).
	 */

	and	t1, t0, a0
	beq	t1, zero, even_page_small
	nop

odd_page_small:
	MFC0(   t1, C0_EntryLo1 )
	b	calc_pfn_small
	nop

even_page_small:
	MFC0(   t1, C0_EntryLo0 )		

calc_pfn_small:
	/* Determine PFN */
	srl	t2, t1, S_EntryLoPFN
	sll     t2, t2, 10

	/*  Least Significant Bits are taken from virtual address.
	 *  t0 = pagesize, t2 = PFN, a0 = virtual address
	 */
	addiu   t0, -1        /* Mask with ones for the lsbits */
	nor	t3, t0, zero  /* Mask with ones for the msbits */
	and	t2, t3	      /* Clear lsbits		       */
	and	t0, a0	      /* Get lsbits from virtual addr  */
	or	t2, t0	      /* Setup lsbits		       */

	/* Store result */
	sw	t2, 0(a1)

	/* Determine setting of Valid bit */
	li	t0, M_EntryLoV
	and	t2, t1, t0
	bne	t2, zero, tlb_valid_small
	nop

	/* Not valid */
	li      v0, SYS_TLB_NOTVALID
	jr	ra 
	nop

tlb_valid_small :
	/* Determine setting of Dirty bit */
	li	t0, M_EntryLoD
	and	t2, t1, t0
	bne	t2, zero, tlb_ok_small
	nop

	/* Write protected */
	li      v0, SYS_TLB_WP
	jr	ra 
	nop

tlb_ok_small :
	li	v0, SYS_TLB_OK	
	jr	ra
	nop

END(sys_tlb_lookup_small)
		
	
/************************************************************************
 *
 *                          sys_tlb_write
 *  Description :
 *  -------------
 *
 *  Write TLB
 *
 *  a0 = pointer to array of 5 words
 * 
 *  array[0] = index
 *  array[1] = pagemask
 *  array[2] = entryhi
 *  array[3] = entrylo0
 *  array[4] = entrylo1
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_tlb_write)

	lw    t0, 0(a0)
	MTC0( t0, C0_Index )

	lw    t0, 4(a0)
	MTC0( t0, C0_PageMask )

	lw    t0, 8(a0)
	MTC0( t0, C0_EntryHi )

	lw    t0, 12(a0)
	MTC0( t0, C0_EntryLo0 )

	lw    t0, 16(a0)
	MTC0( t0, C0_EntryLo1 )

	tlbwi

	/* Done */
	jr	ra
	nop

END(sys_tlb_write)


/************************************************************************
 *
 *                          sys_tlb_read
 *  Description :
 *  -------------
 *
 *  Read TLB
 *
 *  a0 = index
 *  a1 = pointer to array of 5 words. They will be filled with the
 *       following data :
 *
 *  array[0] = pagemask
 *  array[1] = entryhi
 *  array[2] = entrylo0
 *  array[3] = entrylo1
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_tlb_read)

	MTC0( a0, C0_Index )

	tlbr

	MFC0( t0, C0_PageMask )
	sw    t0, 0(a1)

	MFC0( t0, C0_EntryHi )
	sw    t0, 4(a1)

	MFC0( t0, C0_EntryLo0 )
	sw    t0, 8(a1)

	MFC0( t0, C0_EntryLo1 )
	sw    t0, 12(a1)

	/* Done */
	jr	ra
	nop

END(sys_tlb_read)
	
			
/************************************************************************
 *
 *                          sys_icache_invalidate_index
 *  Description :
 *  -------------
 *
 *  Invalidate I cache line containing specified index
 *
 *  a0 holds the index
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_icache_invalidate_index)

SET_MIPS3()
	ICACHE_INDEX_INVALIDATE_OP(a0,a1)
SET_MIPS0()

	jr	ra
	nop
	
END(sys_icache_invalidate_index)


/************************************************************************
 *
 *                          sys_icache_invalidate_addr
 *  Note
 *  ------
 *  This function is called from user2yamon() in application context,
 *  possibly in 64 bit mode and with invalid gp.
 *  if a1 == 0, sys_icache_invalidate_addr modifies only t1.
 *
 *  Description :
 *  -------------
 *  Invalidate I cache line containing specified address.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *	
 *  a0 holds the address
 *  a1 != 0 => flush pipeline after the invalidation
 *
 *  Address MUST be KSEG0/KSEG1
 *	
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF(sys_icache_invalidate_addr)

	MFC0( t1, C0_Config )
	andi  t1, M_ConfigK0
	xori  t1, K_CacheAttrU << S_ConfigK0
	beq   t1, zero, 2f
	nop
	
	la	t1, sys_l2cache_enabled
	lb	t1, 0(t1)
	beq	t1, zero, 1f
	nop

SET_MIPS3()
	SCACHE_ADDR_HIT_WB_INVALIDATE_OP(a0)
	sync
1:		
	ICACHE_ADDR_INVALIDATE_OP(a0,t1)
SET_MIPS0()
2:
	bne	a1, zero, sys_flush_pipeline
	nop
	jr      ra
	nop	

END(sys_icache_invalidate_addr)


/************************************************************************
 *
 *                          sys_flush_pipeline
 *  Description :
 *  -------------
 *
 *  Flush pipeline
 *
 *  a0 holds the index
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_flush_pipeline)

	MFC0(	t0, C0_Status )
	
	/* Set STATUS in a well defined state so that we can perform
	 * a controlled eret (one without modifying shadow registers
	 * in case of a MIPS32/MIPS64 release 2 CPU).
	 *
	 * Note that we may end up here due a user application calling
	 * e.g. SYSCON through the application interface. So, STATUS
	 * is not necessarily in a well known state at this point.
	 *
	 * We need to make sure STATUS has the following settings :
	 *
	 * ERL = 0  (so EPC is used rather than ErrorEPC)
	 * BEV = 1  (so shadowsets are not shifted)
	 * IE  = 0  (so interrupts are disabled)
	 * KSU = 00 (so that we end up in kernel mode after eret)
	 */


	/* First set KSU=00, IE=0 */
	ori	t1, t0, (M_StatusKSU | M_StatusIE)
	xori	t1,     (M_StatusKSU | M_StatusIE)
	MTC0(	t1, C0_Status )
	/* Now set BEV */	
	li	t2, M_StatusBEV
	or	t1, t2
	MTC0(	t1, C0_Status )
	/* Finally, clear ERL */
	ori	t1, M_StatusERL
	xori	t1, M_StatusERL
	MTC0(	t1, C0_Status )

	la	t1, sys_64bit
	lb	t1, 0(t1)
	bne	t1, zero, 1f
	nop

	/* 32 bit CPU */
	MFC0(   t1, C0_EPC ) /* Store EPC */

	/* Now setup EPC and perform eret */	
	la	t2, after_eret32
	MTC0(	t2, C0_EPC )

SET_MIPS3()
	eret
SET_MIPS0()

after_eret32 :
	MTC0(   t1, C0_EPC ) /* Restore EPC */
	b	2f
	nop
	
1:	
	/* 64 bit CPU */
SET_MIPS3()
	DMFC0(  t1, C0_EPC ) /* Store EPC */

	/* Now setup EPC and perform eret */	
	la	t2, after_eret64
	DMTC0(	t2, C0_EPC )

	eret

after_eret64 :
	DMTC0(  t1, C0_EPC ) /* Restore EPC */
SET_MIPS0()
	
2:	
	/* Restore STATUS and return */
	MTC0(	t0, C0_Status )
	jr	ra
	nop

END(sys_flush_pipeline)
	
	
/************************************************************************
 *
 *                          sys_dcache_flush_index
 *  Description :
 *  -------------
 *
 *  Flush D cache line containing specified index
 *
 *  a0 holds the index
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_dcache_flush_index)

SET_MIPS3()
	cache	DCACHE_INDEX_WRITEBACK_INVALIDATE, 0(a0)
SET_MIPS0()
	sync
	jr ra
	nop	

END(sys_dcache_flush_index)


/************************************************************************
 *
 *                          sys_dcache_flush_addr
 *  Note
 *  ------
 *  This function is called from user2yamon() in application context,
 *  possibly in 64 bit mode and with invalid gp.
 *  sys_dcache_flush_addr modifies only t1.
 *
 *  Description :
 *  -------------
 *  Flush D cache line containing specified address.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  a0 holds the address 
 *
 *  Address MUST be KSEG0/KSEG1
 *	
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF(sys_dcache_flush_addr)

	MFC0( t1, C0_Config )
	andi  t1, M_ConfigK0
	xori  t1, K_CacheAttrU << S_ConfigK0
	beq   t1, zero, 2f
	nop
	
	la	t1, sys_l2cache_enabled
	lb	t1, 0(t1)
	beq	t1, zero, 1f
	nop

	/*  This secondary cache operation will also flush $D,
	 *  so we don't need to flush $D.
	 */
SET_MIPS3()
	cache	DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0(a0)
	sync
	SCACHE_ADDR_HIT_WB_INVALIDATE_OP(a0)
SET_MIPS0()
	b	2f
	nop
1:		
SET_MIPS3()
	cache	DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0(a0)
SET_MIPS0()
2:
	sync
	jr	ra
	lb	zero, 0(a0)

END(sys_dcache_flush_addr)


/************************************************************************
 *
 *                          sys_scache_flush_index
 *  Description :
 *  -------------
 *
 *  Flush L2 cache line containing specified index
 *
 *  a0 holds the index
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_scache_flush_index )

SET_MIPS3()
	SCACHE_INDEX_WRITEBACK_INVALIDATE_OP(a0)
SET_MIPS0()
	sync
	jr ra
	nop	

END( sys_scache_flush_index )


/************************************************************************
 *
 *                          sys_enable_int
 *  Description :
 *  -------------
 *
 *  Enable interrupt: set IE in CP0-status.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_enable_int)

    MFC0(   v0, C0_Status)
    or      v0, M_StatusIE
    MTC0(   v0, C0_Status)
    j       ra
    nop

END(sys_enable_int)


/************************************************************************
 *
 *                          sys_disable_int
 *  Description :
 *  -------------
 *
 *  UINT32 sys_disable_int( void )
 *
 *  Disable interrupt: clear IE in CP0-status.
 *  
 *  Return values :
 *  ---------------
 *
 *  Old IE bit
 *
 ************************************************************************/
LEAF(sys_disable_int)

    MFC0(   v0, C0_Status)
    li	    t0, ~M_StatusIE
    nop
    and     v1, v0, t0
    MTC0(   v1, C0_Status)
    li	    t0, M_StatusIE
    j       ra
    and	    v0, t0

END(sys_disable_int)


/************************************************************************
 *
 *                          sys_enable_int_mask
 *  Description :
 *  -------------
 *
 *  Enable specific interrupt: set IM[x] bit in CP0-status.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_enable_int_mask)

    MFC0(   v0, C0_Status)
    li	    t0, 1
    addiu   a0, S_StatusIM
    sllv    t0, t0, a0
    or	    v0, t0
    MTC0(   v0, C0_Status)
    j       ra
    nop

END(sys_enable_int_mask)


/************************************************************************
 *
 *                          sys_disable_int_mask
 *  Description :
 *  -------------
 *
 *  Disable specific interrupt: set IM[x] bit in CP0-status.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF(sys_disable_int_mask)

    MFC0(   v0, C0_Status)
    li	    t0, 1
    addiu   a0, S_StatusIM
    sllv    t0, t0, a0
    li	    t1, 0xffffffff
    xor	    t0, t1
    and	    v0, t0
    MTC0(   v0, C0_Status)
    j       ra
    nop

END(sys_disable_int_mask)

	
/************************************************************************	
 *
 *                          sys_cpu_k0_config
 *  Description :
 *  -------------
 *
 *  Configure K0 field of CP0 CONFIG register
 *
 *  a0 holds the requested K0 setting
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cpu_k0_config )

	/* Reserve space on stack */
	addiu	sp, -8*4

	/* Store registers */
	sw	ra, 0(sp)
	sw	s0, 4(sp)
	sw	s1, 8(sp)	

	/* Copy registers to preserved registers */
	move	s0, a0
	
	/* Disable interrupts */
	MFC0(   s1, C0_Status )	
	li	t0, ~M_StatusIE
	and	t0, s1
	MTC0(   t0, C0_Status )

	/* Flush D-Cache */
	addiu	sp, -4*4
	la      t0, sys_dcache_flush_all
	jal	t0
	nop
	addiu	sp, 4*4

        /* Invalidate I-Cache */
	addiu	sp, -4*4
	la      t0, sys_icache_invalidate_all
	jal	t0
	nop
	addiu	sp, 4*4

	/* Shift to uncached */
	la	t0, sys_cpu_k0_config_uncached
	li	t1, KSEG1BASE
	or	t0, t1
	jr	t0
	nop

sys_cpu_k0_config_uncached :

	/* Set K0 field */
	sll	s0, S_ConfigK0
	MFC0(   t1, C0_Config )
	li	t0, ~M_ConfigK0
	and	t1, t0
	or	t1, s0
	MTC0(   t1, C0_Config )

	/* Restore CP0 STATUS */
	MTC0(   s1, C0_Status )

	/* Restore registers */
	lw	ra, 0(sp)
	lw	s0, 4(sp)
	lw	s1, 8(sp)
	
	jr	ra
	addiu	sp, 8*4
	
END( sys_cpu_k0_config )
	

/************************************************************************
 *
 *                          sys_sync
 *  Description :
 *  -------------
 *
 *  Issue "sync" instruction
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_sync )

        sync
	jr	ra
	nop

END( sys_sync )


/************************************************************************
 *
 *                          sys_fpu_enable
 *  Description :
 *  -------------
 *
 *  Enable FPU
 *
 ************************************************************************/
LEAF( sys_fpu_enable )
	
	/* Enable FPU */
	MFC0(   t0, C0_Status )
	li	t1, M_StatusCU1
	or	t0, t1
	MTC0(   t0, C0_Status )

	/* Set up FPU Control/Status register */
	li	t0, M_FCSRFS	/* Set FS (flush to zero) bit */
	ctc1	t0, C1_FCSR

	jr	ra
	nop
	
END( sys_fpu_enable )

	
		
/******* Functions for access to CP1 (FPU) registers ******/
	

/************************************************************************
 *
 *                          sys_cp1_read
 *  Description :
 *  -------------
 *  Read CP1 register
 *
 *  a0 holds register number
 *
 *  Return values :
 *  ---------------
 *  v0 = Value read
 *
 ************************************************************************/
LEAF( sys_cp1_read )

	addiu	sp, -8
	sw	ra, 0(sp)
	
	/* Be sure to make CU1 usable */
	MFC0(   t2, C0_Status)
	sw	t2, 4(sp)
	li	v0, M_StatusCU1
	or	v0, t2
	or	v0, M_StatusIE  /* and disable interrupts */
	xor	v0, M_StatusIE
	MTC0(   v0, C0_Status)

        /*  Calc specific CFC1 opcode :
	 *
	 *  CFC1 rt, fs
	 *
	 *  010001 00010 rt fs 00000000000
	 *
	 *  rt[4:0]  = v0 ($2) 
	 *  fs[4:0]  = Value of a0
         */
        li      t0, 0x44400000 | (0x2 << 16)
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, cfc1_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr		/* modifies t1 only */
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */

cfc1_location :
	nop	/* Placeholder for CFC1 operation */

	/* CFC1 has now been performed, so restore cp0_status and return */
	lw	t2, 4(sp)
	MTC0(   t2, C0_Status)

	lw	ra, 0(sp)
	jr	ra
	addiu	sp, 8

END( sys_cp1_read )	


/************************************************************************
 *
 *                          sys_cp1_write
 *  Description :
 *  -------------
 *  Write CP1 register
 *
 *  a0 holds CP1 register number
 *  a1 holds value to be written
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( sys_cp1_write )

	addiu	sp, -12
	sw	ra, 0(sp)
	sw	a1, 4(sp)
	
	/* Be sure to make CU1 usable */
	MFC0(   t2, C0_Status)
	sw	t2, 8(sp)	
	li	v0, M_StatusCU1
	or	v0, t2
	or	v0, M_StatusIE  /* and disable interrupts */
	xor	v0, M_StatusIE
	MTC0(   v0, C0_Status)

        /*  Calc specific CTC1 opcode :
	 *
	 *  CTC1 rt, fs 
	 *
	 *  010001 00110 rt fs 00000000000
	 *
	 *  rt[4:0]  = a1 ($5)
	 *  fs[4:0]  = Value of a0
         */
	li      t0, 0x44c00000 | (0x5 << 16)
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, ctc1_location
	sw 	t0, 0(a0)
	
	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr		/* modifies t1 only */
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */

	/* Restore a1 (value to be written) */
	lw	a1, 4(sp)
	
ctc1_location :
	nop	/* Placeholder for CTC1 operation */

	/* CTC1 has now been performed, so restore cp0_status and return */
	lw	t2, 8(sp)
	MTC0(   t2, C0_Status)

	lw	ra, 0(sp)
	jr	ra
	addiu	sp, 12

END( sys_cp1_write )	


/******* Functions for access to CP0 registers ******/

	
/************************************************************************
 *
 *                          sys_cp0_read32
 *  Description :
 *  -------------
 *
 *  Read 32 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = Value read
 *
 ************************************************************************/
LEAF( sys_cp0_read32 )

	/* Look if a0,a1 happen to be C0_Status, sel0 */
	li	t0, R_C0_Status
	bne	a0, t0, 1f
	nop
	bne	a1, zero, 1f
	nop
	MFC0(   v0, C0_Status)
	jr	ra
	nop
	
1:
	/* Reserve space on stack and store ra, C0_Status */
	addiu	sp, -2*4
	sw	ra, 0(sp)

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 4(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

        /*  Calc specific MFC0 opcode :
	 *
	 *  MFC0 rt, rd 
	 *
	 *  010000 00000 rt rd 00000000 sel
	 *
	 *  rt[4:0]  = v0 ($2) 
	 *  rd[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
         */
	
	li      t0, 0x40000000 | (0x2 << 16)
	or  	t0, a1   /* sel */
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, mfc0_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */	

mfc0_location :
	nop	/* Placeholder for MFC0 operation */

	/* MFC0 has now been performed, so restore cp0_status and return */
	lw	t2, 4(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	addiu	sp, 2*4

END( sys_cp0_read32 )	


/************************************************************************
 *
 *                          sys_cp0_read64
 *  Description :
 *  -------------
 *
 *  Read 64 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *
 *  Return values (Note that C-code is compiled for 32 bit !) :
 *  -----------------------------------------------------------
 *
 *  Little endian :
 *  v0 = 32 lsb of value read
 *  v1 = 32 msb of value read 
 *
 *  Big endian :
 *  v0 = 32 msb of value read
 *  v1 = 32 lsb of value read
 *
 ************************************************************************/
LEAF( sys_cp0_read64 )

	/* Reserve space on stack and store ra, C0_Status */
	addiu	sp, -2*4
	sw	ra, 0(sp)

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 4(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

        /*  Calc specific DMFC0 opcode :
	 *
	 *  DMFC0 rt, rd 
	 *
	 *  010000 00001 rt rd 00000000 sel
	 *
	 *  rt[4:0]  = v0 ($2) 
	 *  rd[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
         */
	
	li      t0, 0x40200000 | (0x2 << 16)
	or  	t0, a1   /* sel */
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, dmfc0_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */		

dmfc0_location :
	nop	/* Placeholder for DMFC0 operation */

SET_MIPS3()
#ifdef EB
        move	v1, v0
	dsrl	v0, v1, 32
#else
	dsrl	v1, v0, 32
#endif	
SET_MIPS0()
	
	/* DMFC0 has now been performed, so restore cp0_status and return */
	lw	t2, 4(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	addiu	sp, 2*4

END( sys_cp0_read64 )	
	

/************************************************************************
 *
 *                          sys_cp0_write32
 *  Description :
 *  -------------
 *
 *  Write 32 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *  a2 holds value to be written
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cp0_write32 )

	/* Look if a0,a1 happen to be C0_Status, sel0 */
	li	t0, R_C0_Status
	bne	a0, t0, 1f
	nop
	bne	a1, zero, 1f
	nop
	MTC0(   a2, C0_Status)
	jr	ra
	nop
	
1:
	/* Reserve space on stack and store ra, a2, C0_Status */
	addiu	sp, -3*4
	sw	ra, 0(sp)
	sw	a2, 4(sp)	

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 8(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

        /*  Calc specific MTC0 opcode :
	 *
	 *  MTC0 rt, rd 
	 *
	 *  010000 00100 rt rd 00000000 sel
	 *
	 *  rt[4:0]  = a2 ($6)
	 *  rd[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
         */
	
	li      t0, 0x40800000 | (0x6 << 16)
	or  	t0, a1   /* sel */
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, mtc0_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */

	/* Restore a2 (value to be written) */
	lw	a2, 4(sp)		

mtc0_location :
	nop	/* Placeholder for MTC0 operation */

	/* MTC0 has now been performed, so restore cp0_status and return */
	lw	t2, 8(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	addiu	sp, 3*4

END( sys_cp0_write32 )	


/************************************************************************
 *
 *                          sys_cp0_write64
 *  Description :
 *  -------------
 *
 *  Write 64 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *
 *  Little endian (Note that C-code is compiled for 32 bit !) :
 *
 *  a2 holds 32 lsb of value to be written
 *  a3 holds 32 msb of value to be written
 *
 *  Big endian :
 *
 *  a2 holds 32 msb of value to be written
 *  a3 holds 32 lsb of value to be written
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cp0_write64 )

SET_MIPS3()
		
	/* Reserve space on stack and store ra, a2, a3, C0_Status */
	addiu	sp, -4*4
	sw	ra, 0(sp)
	sw	a2, 4(sp)
	sw	a3, 8(sp)

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 12(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

        /*  Calc specific DMTC0 opcode :
	 *
	 *  DMTC0 rt, rd 
	 *
	 *  010000 00101 rt rd 00000000 sel
	 *
	 *  rt[4:0]  = a2 ($6)
	 *  rd[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
         */
	
	li      t0, 0x40a00000 | (0x6 << 16)
	or  	t0, a1   /* sel */
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, dmtc0_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	li	a1, 1				/* Flush pipeline */	

	/* Restore a2, a3 (value to be written) */
	lw	a2, 4(sp)
	lw	a3, 8(sp)

#ifdef EB
	dsll	a3, 32
	dsrl	a3, 32 
	dsll	a2, 32
	or	a2, a3
#else
	dsll	a2, 32
	dsrl	a2, 32
	dsll	a3, 32
	or	a2, a3
#endif

dmtc0_location :
	nop	/* Placeholder for DMTC0 operation */

	/* DMTC0 has now been performed, so restore cp0_status and return */
	lw	t2, 12(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	addiu	sp, 4*4

SET_MIPS0()
		
END( sys_cp0_write64 )


/************************************************************************
 *
 *                          sys_cp0_mtread32
 *  Description :
 *  -------------
 *
 *  Read 32 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *  a2 holds tc
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = Value read
 *
 ************************************************************************/
LEAF( sys_cp0_mtread32 )
	
	/* Look if a0,a1,a2 happen to be C0_Status, sel0, TC0 */
	li	t0, R_C0_Status
	bne	a0, t0, 1f
	 nop
	bne	a1, zero, 1f
	 nop
	bne	a2, zero, 1f
	 nop
	MFC0(   v0, C0_Status)
	jr	ra
	nop
	
1:
#define MTREAD32SIZE	4
	/* Reserve space on stack and store ra, C0_Status, MVPCtl, VPECtl */
	addiu	sp, -MTREAD32SIZE*4
	sw	ra, 0(sp)

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 4(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

	DVPE(R_t3)
	sw	t3, 8(sp)
		
	/* Select target thread */
	MFC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)
	sw	t4, 12(sp)
	and	t4, ~M_VPECtlTargTC
	or	t4, a2
	MTC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)

        /*  Calc specific MFTR opcode :
	 *
	 *  MFTR rd, rt, u, sel, h
	 *
	 *  010000 01000 rt rd 00000uh0 sel
	 *
	 *  rd[4:0]  = v0 ($2) 
	 *  rt[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
	 *  u = 0
	 *  h = 0
         */
	
	li      t0, 0x41000000 | (0x2 << 11)
	or  	t0, a1   /* sel */
	sll 	a0, 16
	or  	t0, a0

	/* Store instruction */
	la  	a0, mftr_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	 nop
	jal	sys_icache_invalidate_addr
	 li	a1, 1				/* Flush pipeline */

	# align to cacheline boundary		
	.align	3		
mftr_location :
	nop	/* Placeholder for MFTR operation */

	/* restore VPECtl */
	lw	t4, 12(sp)
	MTC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)
	
	/* reenable VPE's if needed */
	lw	t3, 8(sp)
	and	t3, M_MVPCtlEVP
	beqz	t3, 1f
	 nop
	EVPE(R_zero)
1:		
	/* MFTR has now been performed, so restore cp0_status and return */
	lw	t2, 4(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	 addiu	sp, MTREAD32SIZE*4

END( sys_cp0_mtread32 )	


/************************************************************************
 *
 *                          sys_cp0_mtwrite32
 *  Description :
 *  -------------
 *
 *  Write 32 bit CP0 register
 *
 *  a0 holds register number
 *  a1 holds sel field
 *  a2 holds value to be written
 *  a3 holds tc
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( sys_cp0_mtwrite32 )

	/* Look if a0,a1,a3 happen to be C0_Status, sel0, TC0 */
	li	t0, R_C0_Status
	bne	a0, t0, 1f
	 nop
	bne	a1, zero, 1f
	 nop
	bne	a3, zero, 1f
	 nop
	MTC0(   a2, C0_Status)
	jr	ra
	 nop
	
1:
#define MTWRITE32SIZE	5
	/* Reserve space on stack and store ra, a2, C0_Status, MVPCtl, VPECtl */
	addiu	sp, -MTWRITE32SIZE*4
	sw	ra, 0(sp)
	sw	a2, 4(sp)	

	/* Disable interrupts */
	MFC0(   t2, C0_Status)
	sw	t2, 8(sp)
	or	t2, M_StatusIE
	xor	t2, M_StatusIE
	MTC0(   t2, C0_Status)

	DVPE(R_t3)
	sw	t3, 12(sp)
	
	/* Select target thread */
	MFC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)
	sw	t4, 16(sp)
	and	t4, ~M_VPECtlTargTC
	or	t4, a3
	MTC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)
	
        /*  Calc specific MTTR opcode :
	 *
	 *  MTTR rt, rd, u, sel, h
	 *
	 *  010000 01100 rt rd 00000uh0 sel
	 *
	 *  rt[4:0]  = a2 ($6)
	 *  rd[4:0]  = Value of a0
	 *  sel[2:0] = Value of a1
         */
	
	li      t0, 0x41800000 | (0x6 << 16)
	or  	t0, a1   /* sel */
	sll 	a0, 11
	or  	t0, a0

	/* Store instruction */
	la  	a0, mttr_location
	sw 	t0, 0(a0)

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 */
	jal     sys_dcache_flush_addr
	 nop
	jal	sys_icache_invalidate_addr
	 li	a1, 1				/* Flush pipeline */

	/* Restore a2 (value to be written) */
	lw	a2, 4(sp)		

	# align to cacheline boundary		
	.align 3
mttr_location :
	nop	/* Placeholder for MTTR operation */

	ehb
	
	/* restore VPECtl */
	lw	t4, 16(sp)
	MTC0_SEL_OPCODE(R_t4, R_C0_VPECtl, R_C0_SelVPECtl)
	
	/* reenable VPE's if needed */		
	lw	t3, 12(sp)
	and	t3, M_MVPCtlEVP
	beqz	t3, 1f
	 nop
	EVPE(R_zero)
1:		
	/* MTTR has now been performed, so restore cp0_status and return */
	lw	t2, 8(sp)
	MTC0(   t2, C0_Status)

	lw      ra, 0(sp)
	jr	ra
	 addiu	sp, MTWRITE32SIZE*4

END( sys_cp0_mtwrite32 )	
