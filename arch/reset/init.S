
/************************************************************************
 *
 *  init.S
 *
 *  Startup code for bootprom
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

#include <sysdefs.h>
#include <mips.h>
#include <product.h>
#include <init.h>
#include <sys_api.h>
#include <gcmp.h>
#include <launch.h>
					
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

/*  Public address for returning from call to 
 *  sys_platform_early (see below) 
 */
.globl sys_platform_early_done
	
/************************************************************************
 *  reset_handler
 ************************************************************************/
LEAF(__reset_handler)

/* This is the entry point of the endian dependent code. It is called
 * from the reset handler (reset.S) located at bfc00000. Processor is running
 * at uncached addresses, but this code was linked cached. This function
 * will shift to cached when caches have been initialised.
 */	
	.set noreorder
	.set noat

	/* Don't touch cause if we got here from an NMI */
	MFC0(   k0, C0_Status)
	sll	k0, (31-S_StatusNMI)
	bltz	k0, 1f
	
	/* Setup cause register (clear cause code)  */
	li	k0, M_CauseIV
	MTC0(	k0, C0_Cause)

	/*  Early platform specific initialisation.
	 *  Also detects NMI.
	 *  Can't do jal since we don't want to modify ra register
	 *  in case an NMI has occurred.
	 *  Instead, sys_platform_early shall jump to
	 *  sys_platform_early_done when it is done.
	 *
	 *  sys_platform_early will set k0 = Product ID.
	 */
1:	
	j       sys_platform_early
	 nop
	.set	at

sys_platform_early_done:
	
	/*  From now on, we can use the display macro
	 *  (if platform supports it) 
	 */

	bne	v0, zero, error
	nop
	
	/* Determine processor */
	MFC0(   k1, C0_PRId)
	li	t0, M_PRIdImp | M_PRIdCoID
	and     k1, t0

	/**** k1 now holds the ProcessorID field ****/

	/* Perform CPU specific initialisation */

	DISP_STR( msg_cpu )
		
	jal     sys_init_processor
	nop
1:
	bne	v0, zero, error
	nop

	/*  Initialise caches. If compiled for simulation, we
	 *  skip this since caches can be assumed to be invalidated.
	 */
	
#ifndef _SIMULATE_

	jal     sys_init_cache
	nop
1:
	bne	v0, zero, error
	nop
	
#endif /* #ifndef _SIMULATE_ */
	
	/*
	 * MIPSCMP
	 * Time to start up the coherency manager if present
	 */
#define GCMP	t0
	li	GCMP,KSEG1(GCMPBASE)
	lw	t1,GCMPGCBOFS(GCMPB)(GCMP)
	and	t1,GCMP_GCB_GCMPB_GCMPBASE_MSK
	or	t2,t1,KSEG1BASE
	bne	t2,GCMP,nogcmp		/* No GCMP present so skip the initialisation */ 
	 nop

	/*
	 * Change the CCA from non-coherent to coherent
	 * Assumption: GCMP => coherent CCA is usable
	 */
	mfc0	t2,C0_Config
	and	t3,t2,M_ConfigK0
	xori	t3,K_CacheAttrCN<<S_ConfigK0
	bnez	t3,1f
	 nop
	and	t2,~M_ConfigK0
	or	t2,K_CacheAttrCCS<<S_ConfigK0
	mtc0	t2, C0_Config
1:

	/*
	 * Primary Core needs to initialise GCMP
	 */
	bnez	v1, 1f
	 nop
	
	/* Initialise CM */
	
	/* Rewrite the GCMPB register to initialise CM_DEFAULT_TARGET */
	or	t1,GCMP_GCB_GCMPB_CMDEFTGT_MEM<<GCMP_GCB_GCMPB_CMDEFTGT_SHF
	sw	t1,GCMPGCBOFS(GCMPB)(GCMP)

	/* Disable the CM regions */
	lui	t1,0xffff
	sw	t1,GCMP_GCB_CMxBASE_OFS(0)(GCMP)
	sw	t1,GCMP_GCB_CMxMASK_OFS(0)(GCMP)
	sw	t1,GCMP_GCB_CMxBASE_OFS(1)(GCMP)
	sw	t1,GCMP_GCB_CMxMASK_OFS(1)(GCMP)
	sw	t1,GCMP_GCB_CMxBASE_OFS(2)(GCMP)
	sw	t1,GCMP_GCB_CMxMASK_OFS(2)(GCMP)
	sw	t1,GCMP_GCB_CMxBASE_OFS(3)(GCMP)
	sw	t1,GCMP_GCB_CMxMASK_OFS(3)(GCMP)

1:
	/* Allow interventions from all other cores (including self) */
	li	t1,0xff
	sw	t1, GCMPCLCBOFS(COHCTL)(GCMP)
	EHB	/* FIXME: Other hazards? */

nogcmp:
	/* Caches have now been initialised, so shift to kseg0 */
	la	t0, 1f
	KSEG0A( t0)
	j	t0
	nop
1:
	/**** From now on, we are executing cached ****/
	
	/*
         * MIPSMT
	 * Check for MT capable cores
	 */

	MFC0(	t0,C0_Config)
	srl	t0,S_ConfigAR
	andi	t0,M_ConfigAR>>S_ConfigAR
	beqz	t0,notmtcapable
	 nop

	/* Config1 is required for MIPS32 */
	MFC0_SEL_OPCODE(R_t0, R_C0_Config1, R_C0_SelConfig1)
#if S_Config1M!=31
	sll	t0,31 - S_Config1M
#endif
	bgez	t0,notmtcapable		/* No Config2 register */
	 nop
	MFC0_SEL_OPCODE(R_t0, R_C0_Config2, R_C0_SelConfig2)
#if S_Config2M!=31
	sll	t0,31 - S_Config2M
#endif
	bgez	t0,notmtcapable		/* No Config3 register */
	 nop
	MFC0_SEL_OPCODE(R_t0, R_C0_Config3, R_C0_SelConfig3)
	and	t0,M_Config3MT
	beqz	t0,notmtcapable
	 nop

	/*
	 * This core supports MT ASE
	 * Start up secondary VPE's
	 */
	.set	push
	.set	mips32r2
	.set	mt


startmtconfig:
	/* Enter config mode */
	mfc0	t0,C0_MVPCtl
	or	t0,M_MVPCtlVPC
	mtc0	t0,C0_MVPCtl
	
	/* Make sure VPEConf0.MVP is set */
	mfc0	t0,C0_VPEConf0
	or	t0,M_VPEConf0MVP
	mtc0	t0,C0_VPEConf0
	ehb

#define NTCS	t2
#define NVPES	t3
#define TC	t4

	/* Get number of TC's and VPE's */
	mfc0	t0,C0_MVPConf0
	srl	NTCS,t0,S_MVPConf0PTC
	andi	NTCS,M_MVPConf0PTC>>S_MVPConf0PTC
	srl	NVPES,t0,S_MVPConf0PVPE
	andi	NVPES,M_MVPConf0PVPE>>S_MVPConf0PVPE

	/* Initialise TC's/VPE's */
	move	TC,zero
nexttc:
	/* Select TCn */
	mfc0	t0,C0_VPECtl
	and	t0,~M_VPECtlTargTC
	sll	t1,TC,S_VPECtlTargTC
	or	t0,t1
	mtc0	t0,C0_VPECtl
	ehb

	/* Bind TC to next VPE */
	beqz	TC,nextvpe		/* Don't rebind TC0 */
	 nop
	bgt	TC,NVPES,2f		/* Bind spare TC's to VPElast */
	 move	t1,NVPES
	
	/* Set XTC for active TC's */
	mftc0	t0,C0_VPEConf0
	and	t0,~M_VPEConf0XTC
	sll	t1,TC,S_VPEConf0XTC
	or	t0,t1
	mttc0	t0,C0_VPEConf0
	move	t1,TC
2:
	/* Bind TC to a VPE */
	mftc0	t0,C0_TCBind
	and	t0,~M_TCBindCurVPE
	sll	t1,S_TCBindCurVPE
	or	t0,t1
	mttc0	t0,C0_TCBind

	/* mark TC not allocated, not dynamically allocatable and interrupt exempt */
	mftc0	t0,C0_TCStatus
	and	t0,~(M_TCStatusA|M_TCStatusDA)
	or	t0,M_TCStatusIXMT
	mttc0	t0,C0_TCStatus

	li	t0,M_TCHaltH		/* Halt all TC's */
	mttc0	t0,C0_TCHalt

nextvpe:
	bgt	TC,NVPES,donevpe	/* No more VPE's */
	 nop

	/* Disable multi-threading with TC's */
	mftc0	t0,C0_VPECtl
	and	t0,~M_VPECtlTE
	mttc0	t0,C0_VPECtl

	beqz	TC,1f
	 nop

	/* For VPE1..n */
	/* Clear VPA and set master VPE */
	mftc0	t0,C0_VPEConf0
	and	t0,~M_VPEConf0VPA
	or	t0,M_VPEConf0MVP
	mttc0	t0,C0_VPEConf0

	mfc0	t0,C0_Status
	mttc0	t0,C0_Status

	li	t0,0x12345678
	mttc0	t0,C0_EPC
	
	mttc0	zero,C0_Cause

	mfc0	t0,C0_Config
	mttc0	t0,C0_Config	

	mftc0	t0,C0_EBase
	srl	t0,S_EBaseCPUNum
	andi	t0,M_EBaseCPUNum
	mttgpr	t0,v1

	/* Finally... arrange for other VPE's to continue from othercores */
	la	t0,othercores
	mttc0	t0,C0_TCRestart

	/* Yes.. this is undoing all of the work done above... :) */
	mftc0	t0,C0_TCStatus
	and	t0,~M_TCStatusIXMT
	or	t0,M_TCStatusA
	mttc0	t0,C0_TCStatus

	mttc0	zero,C0_TCHalt

	mftc0	t0,C0_VPEConf0
	ori	t0,M_VPEConf0VPA
	mttc0	t0,C0_VPEConf0

1:
	
donevpe:
	addu	TC,1
	bleu	TC,NTCS,nexttc
	 nop

	/* Exit config mode */
	mfc0	t0,C0_MVPCtl
	and	t0, ~M_MVPCtlVPC
	mtc0	t0,C0_MVPCtl
	ehb

	/*
	 * Enable MT for cores other than 0
	 * MT on Core0 will be enabled later in shell_cpu_init
	 */
	beqz	v1,1f
	 nop
	evpe
	ehb
1:	
#undef NTCS
#undef NVPES
#undef TC
	.set	pop

notmtcapable:
	/*
	 * MIPSCMP
	 * Only Core0 carries on from here
	 * Everybody else waits...
	 */
	beqz	v1,finish_initialisation
	 nop

othercores:
 	/* FIXME any other per-CPU initialisation required? */

	li	t0,KSEG0(CPULAUNCH)
	sll	t1,v1,LOG2CPULAUNCH
	addu	t0,t1

	/*
	 * Set CPU online flag
	 */
	lw	t1,LAUNCH_FLAGS(t0)
	or	t1,LAUNCH_FREADY
	sw	t1,LAUNCH_FLAGS(t0)

	/* enable count interrupt in mask, but don't enable interrupts */
	mfc0	t2,C0_Status
	li	t1,M_StatusIM7 /* FIXME should calculate dynamically from Config3.ippti */
	or	t1,t2
	mtc0	t1,C0_Status

	MFC0_SEL_OPCODE(R_t3, R_C0_Config7, R_C0_SelConfig7)
	and	t3,M_Config7WII	/* Wakeup from wait with interrupts disabled? */

	/* Execute the startcode out of a safe RAM area */	
	li	t1,KSEG0(SCRLAUNCH)
	j	t1
	 nop
	
waitcode_start:
	/*
	 * Poll CPU go flag
	 */
1:	mfc0	t1,C0_Count
	addu	t1,LAUNCHPERIOD
	mtc0	t1,C0_Compare

	bnez	t3,hwwait
	 nop


swwait:	/* Software wait */
	mfc0	t4,C0_Count
	subu	t4,t1
	bltz	t4,swwait
	 nop
	b	checklaunch
	 nop


hwwait:	/* Hardware wait */
	.set	push
	.set	mips32
	wait
	.set	pop

checklaunch:
#define LEDDBG	
#ifdef LEDDBG
	/*
	 * This is a bad idea...
	 * It's Malta specific and causes contention on this cache line
	 * but is amusing for now
	 */
#define LEDBITS	KSEG0(CPULAUNCH+LAUNCH_FLAGS-4)
#define LEDBAR	KSEG1(0x1f000408)
	
	/* Only do this every 512 iterations */
	srl	t6,t5,9
	beqz	t6,twiddledone
	 addu	t5,1
	li	t5,1
	sll	t5,v1
	/* update the CPU bits atomically */
	li	t4,LEDBITS
twiddleledbits:
	ll	t6,(t4)
	xor	t6,t5
	sc	t6,(t4)
	beqz	t6,twiddleledbits
	 nop
	/* made our change, now go with the flow */
	lw	t6,(t4)
	li	t4,LEDBAR
	sw	t6,(t4)
	/* reset our counter */
	move	t5,zero
twiddledone:
#endif

	lw	t1,LAUNCH_FLAGS(t0)
	and	t1,LAUNCH_FGO
	beqz	t1, 1b
	 nop

	/* Reset the counter and interrupts to give naive clients a chance */
	mtc0	t2,C0_Status
	mfc0	t2,C0_Count
	subu	t2,1
	mtc0	t2,C0_Compare

	/* we're off */	
	lw	t2,LAUNCH_PC(t0)
	lw	gp,LAUNCH_GP(t0)
	lw	sp,LAUNCH_SP(t0)
	lw	a0,LAUNCH_A0(t0)
	move	a1,zero
	move	a2,zero
	move	a3,zero
	ori	t1,LAUNCH_FGONE
	jr	t2
	 sw	t1,LAUNCH_FLAGS(t0)
waitcode_end:


finish_initialisation:	
	DISP_STR( msg_platform )
	
	/* Perform platform specific initialisation.
	 *
	 * This does not include memory test/clear.
	 * The function returns the following parameters :
	 * v0 = error code (0 = no error)
	 * v1 = RAM size in bytes
	 */

	jal	sys_init_platform
	nop
1:
	bne	v0, zero, error
	nop
	
	move	s0, v1

	/**** s0 now holds RAM size in bytes ****/

	/* Now perform memory test/clear */

	DISP_STR( msg_ram )
	
	move	a0, s0
	jal	sys_memory_setup	/* v0 != 0 on error */
	nop
	bne	v0, zero, error
	nop

	/* Copy code to RAM */

	DISP_STR( msg_copy_code )
	
#ifndef _SIMULATE_
		
	la	a0, _etext_init		/* SRC  */
	la	a1, _ftext_ram		/* DST  */
	la	a2, _etext_ram		/* STOP */
	bal	copy_text
	 nop
	
#endif /* #ifndef _SIMULATE_ */

	/* Copy the wait code to memory so that it is not running from flash */
	la	a0, waitcode_start	/* SRC */
	li	a1, SCRLAUNCH		/* DST */
	/* Grr... */
	li	a2, SCRLAUNCH+(waitcode_end-waitcode_start) /* STOP */
	bal	copy_text
	 nop
	
	/* Copy initialised data to RAM */

copy_data:

	DISP_STR( msg_copy_data )
	
	la	t0, _etext		/* SRC  */
	la	t1, _fdata		/* DST  */
	la	t2, _fbss		/* STOP */
	KSEG0A( t1 )
	KSEG0A( t2 )
	beq	t1, t2, setup_sp
	nop
	addiu	t2, -4	
1:
	lw	t3, 0(t0)
	add	t0, 4
	sw	t3, 0(t1)
	bne	t1, t2, 1b
	add	t1, 4

#ifdef VERIFY_COPYTEXT
	/* Verify the data copied OK */
	la	t0, _etext		/* SRC  */
	la	t1, _fdata		/* DST  */
	la	t2, _fbss		/* STOP */
	KSEG0A( t1 )
	KSEG0A( t2 )
	addiu	t2, -4	
1:
	lw	t3, 0(t0)
	lw	t4, 0(t1)
	bne	t3, t4, 2f		/* Oh dear! */
	 add	t0, 4
	bne	t1, t2, 1b
	 add	t1, 4
	b	4f
	 nop
2:
	DISP_STR( msg_verify )
3:	b	3b
	 nop
4:
#endif /* VERIFY_COPYTEXT */

setup_sp :

	/* Setup sp (at top of memory) */

	DISP_STR( msg_sp )
	
	la	 sp, _freemem
	li	 t0, SYS_STACK_SIZE
	addu	 sp, t0
	/* Align sp to 16 byte boundary (required by Cygnus) */
	li	 t0, ~0xf
	and	 sp, t0

 	/* Store system info */

	DISP_STR( msg_info )
	
	la	t0, sys_processor
	sw	k1, 0(t0)
	la	t0, sys_ramsize
	sw	s0, 0(t0)
	
	/* Redetermine cache info and store it */

	DISP_STR( msg_cache_info )
	
	move	a0, k1

	/* L2 Cache */
	jal     sys_determine_l2cache_avail_flash
	nop
	la      t0, sys_l2cache
	sb	v0, 0(t0)
        la      t0, sys_l2cache_enabled
	sb	v1, 0(t0)
	li	v1, 1		/* FIXME: Assume L2 is exclusive */
	la      t0, sys_l2cache_exclusive
	sb	v1, 0(t0)
	
	beq	v0, zero, 1f
	nop

	jal	sys_determine_l2cache_linesize_flash
	nop
	la      t0, sys_l2cache_linesize
	sw	v0, 0(t0)
	
	jal     sys_determine_l2cache_lines_flash
	nop
	la	t0, sys_l2cache_lines
	sw	v0, 0(t0)
	
	jal	sys_determine_l2cache_assoc_flash
	nop
	la	t0, sys_l2cache_assoc
	sw	v0, 0(t0)
1:
	/* L1 Cache */
	jal	sys_determine_icache_assoc_flash
	nop
	la	t0, sys_icache_assoc
	sw	v0, 0(t0)
	
	jal	sys_determine_icache_lines_flash
	nop
	la	t0, sys_icache_lines
	sw	v0, 0(t0)

	jal	sys_determine_icache_linesize_flash
	nop
	la	t0, sys_icache_linesize
	sw	v0, 0(t0)

	jal	sys_determine_dcache_assoc_flash
	nop
	la	t0, sys_dcache_assoc
	sw	v0, 0(t0)
				
	jal	sys_determine_dcache_lines_flash
	nop
	la	t0, sys_dcache_lines
	sw	v0, 0(t0)
		
	jal	sys_determine_dcache_linesize_flash
	nop
	la	t0, sys_dcache_linesize
	sw	v0, 0(t0)

	jal	sys_determine_dcache_antialias_flash
	 nop
	la	t0, sys_dcache_antialias
	sw	v0, 0(t0)

        /*  Calc first free RAM address. We add the dcache line size
	 *  to avoid cache writeback problem if user executes uncached
	 *  code.
	 */
	li	t0, SYS_APPL_STACK_SIZE
	addu	t0, sp
	addu	t0, v0
	la	t1, sys_freemem
	sw	t0, 0(t1)
		 
	/*  C-functions can now be called (memory ready and sp,gp have been set).
	 *  Interrupts are disabled .
	 *  We are using the exception handlers of the bootprom.
	 */

	/*  Call the first C-function. It should never return. If it does, we
	 * consider it an error, and expect v0 to hold the error code.
	 */

	DISP_STR( msg_main )

	li	t0, 4*4
	subu	sp, t0	
	la	t0, c_entry
	jalr	t0
	nop

error:
	/*  Error handling. Display error code (if device for this is available) and
	 *  enter an infinite loop (from init.h)
	 */

	ERROR_HANDLING	
	
END(__reset_handler)
	

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

SLEAF(copy_text)	
	move	t0, a0
	KSEG0A2(t1, a1)
	KSEG0A2(t2, a2)
	beq	t1, t2, copy_text_done
	 nop
	addiu	t2, -4
	
	/*  Whenever an instruction has been stored to KSEG0, we must
	 *  make sure the instruction has been flushed to physical memory
	 *  and invalidate the corresponding I-Cache line.
	 *
	 *  We bypass the cache operations if CPU is running uncached.
	 *  (We assume Config.k0 == K_CacheAttrU is the
	 *  only uncached mode used).
	 */
	
	MFC0( t4, C0_Config )
	and   t4, M_ConfigK0
	srl   t4, S_ConfigK0
	xor   t4, K_CacheAttrU

	/*
	 * L2 cache may also be present
	 */
	move	t3, ra
	jal     sys_determine_l2cache_avail_flash
	 move	a0, k1
	or	t5, v0, v1	/* L2 cache available and enabled */
	move	ra, t3

1:	
	lw	t3, 0(t0)
	sw	t3, 0(t1)

	beqz	t4, 2f	
	 addu	t0, 4

	SET_MIPS3()
	cache   DCACHE_ADDR_HIT_WRITEBACK, 0(t1)
	sync
	.set	noat
	ICACHE_ADDR_INVALIDATE_OP(t1,AT)
	.set	at
	SET_MIPS0()
2:	
	beqz	t5, 2f
	 nop
	
	SET_MIPS3()
	cache   SCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0(t1)
	SET_MIPS0()
	
2:
	bne	t1, t2, 1b
	 addu	t1, 4

copy_text_done:
	j	ra
	 nop
END(copy_text)	
		
/************************************************************************
 *  sys_memory_setup
 ************************************************************************/
SLEAF(sys_memory_setup)

	/*  a0 = RAM size */

	/*  Test first, middle and last address by writing magic values and
	 *  reading values back.
	 *  Test for wraparound by writing boundary locations
	 */
	DISP_STR( msg_ramtest_hilo )

	/* First calc start, middle and end addresses	*/
	li	t0, KSEG1(0)	/* First	   	*/
	add	t2, t0, a0	/* First non valid 	*/

	/* Setup values */
#define MAGIC_LO	0xFEEDFACE
#define MAGIC_MI	0x5A5A5A00
#define MAGIC_HI	0xDEADBEEF
#define BLACK_MAGIC	0x0BADF00D

	li	t3, MAGIC_LO
	li	t4, MAGIC_MI
	li	t5, MAGIC_HI
	li	t6, BLACK_MAGIC

	/* Perform the writes */
	sw	t3, 0(t0)	/* first */
	sw	t3, 4(t0)	/* first+4 */
	sw	t6, 8(t0)
	sw	t6, 12(t0)

	srl	t1, a0, 1
	addu	t1, t0
	sw	t4, -8(t1)	/* (last/2)-8 */
	add	t4, 1
	sw	t4, -4(t1)	/* (last/2)-4 */
	add	t4, 1
	sw	t6, -16(t1)
	sw	t6, -12(t1)

	srl	t1, a0, 2
	addu	t1, t0
	sw	t4, -8(t1)	/* (last/4)-8 */
	add	t4, 1
	sw	t4, -4(t1)	/* (last/4)-4 */
	add	t4, 1
	sw	t6, -16(t1)
	sw	t6, -12(t1)

	srl	t1, a0, 3
	addu	t1, t0
	sw	t4, -8(t1)	/* (last/8)-8 */
	add	t4, 1
	sw	t4, -4(t1)	/* (last/8)-4 */
	add	t4, 1
	sw	t6, -16(t1)
	sw	t6, -12(t1)

	srl	t1, a0, 1
	srl	t6, a0, 2
	addu	t1, t6
	addu	t1, t0
	sw	t4, -8(t1)	/* (last/2) + (last/4) - 8 */
	add	t4, 1
	sw	t4, -4(t1)	/* (last/2) + (last/4) - 4 */
	add	t4, 1
	sw	t6, -16(t1)
	sw	t6, -12(t1)

	srl	t1, a0, 1
	srl	t6, a0, 2
	addu	t1, t6
	srl	t6, a0, 3
	addu	t1, t6
	addu	t1, t0
	sw	t4, -8(t1)	/* (last/2) + (last/4) + (last/8) - 8 */
	add	t4, 1
	sw	t4, -4(t1)	/* (last/2) + (last/4) + (last/8) - 4 */
	add	t4, 1
	sw	t6, -16(t1)
	sw	t6, -12(t1)

	sw	t5, -8(t2)	/* last-8 */
	sw	t5, -4(t2)	/* last-4 */
	sw	t6, -16(t2)
	sw	t6, -12(t2)

	/* Perform the reads and compares */
	li	t4, MAGIC_MI
	lw	v0, 0(t0)	/* first */
	bne	v0, t3, error
	 li	v0, ERROR_RAM_LO
	not	v0, t3		/* Ensure nothing lingering on the bus */
	sw	v0, 8(t0)
	lw	v0, 4(t0)	/* first+4 */
	bne	v0, t3, error
	 li	v0, ERROR_RAM_LO
	not	v0, t3
	sw	v0, 12(t0)

	srl	t1, a0, 1
	addu	t1, t0
	lw	v0, -8(t1)	/* (last/2) - 8 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -16(t1)
	lw	v0, -4(t1)	/* (last/2) - 4 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -12(t1)

	srl	t1, a0, 2
	addu	t1, t0
	lw	v0, -8(t1)	/* (last/4) - 8 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -16(t1)
	lw	v0, -4(t1)	/* (last/4) - 4 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -12(t1)

	srl	t1, a0, 3
	addu	t1, t0
	lw	v0, -8(t1)	/* (last/8) - 8 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -16(t1)
	lw	v0, -4(t1)	/* (last/8) - 4 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -12(t1)

	srl	t1, a0, 1
	srl	t6, a0, 2
	addu	t1, t6
	addu	t1, t0
	lw	v0, -8(t1)	/* (last/2) + (last/4) - 8 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -16(t1)
	lw	v0, -4(t1)	/* (last/2) + (last/4) - 4 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -12(t1)

	srl	t1, a0, 1
	srl	t6, a0, 2
	addu	t1, t6
	srl	t6, a0, 3
	addu	t1, t6
	addu	t1, t0
	lw	v0, -8(t1)	/* (last/2) + (last/4) + (last/8) - 8 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -16(t1)
	lw	v0, -4(t1)	/* (last/2) + (last/4) + (last/8) - 4 */
	bne	v0, t4, error
	 li	v0, ERROR_RAM_MI
	add	t4, 1
	not	v0, t4
	sw	v0, -12(t1)

	lw	v0, -8(t2)
	bne	v0, t5, error
	 li	v0, ERROR_RAM_HI
	not	v0, t5
	sw	v0, -16(t1)
	lw	v0, -4(t2)
	bne	v0, t5, error
	 li	v0, ERROR_RAM_HI
	not	v0, t5
	sw	v0, -12(t1)

	/* ok */
	move	v0, zero

	/* Perform test of yamon RAM */

	DISP_STR( msg_ramtest_yamon )

	li	a0, KSEG1(0)	        /* start	*/
#ifdef _SIMULATE_
	li	a1, 0x0400		/* only test 1KByte */
#else
	la	a1, _freemem
	li	t0, SYS_STACK_SIZE
	addu	a1, t0 
	PHYSA( a1 )			/* size		*/
#endif
	move    a2, zero		/* no callback  */
	move	s1, ra
	jal	sys_memory_test
	nop
	move	ra, s1
	bne	v0, zero, error
	nop
	move	v0, zero

	/* Perform memory clear */

mem_clear:	
	
	DISP_STR( msg_ram_clear )
	
	la     t0, _fbss
	la     t1, _end
	addiu  t1, -4
	KSEG0A(  t0 )
	KSEG0A(  t1 )
1:
	sw      zero,  0(t0)
	bne	t0, t1, 1b
	add	t0, 4

mem_setup_done :		
	jr	ra
	nop
		
END(sys_memory_setup)	


/************************************************************************
 *
 *                          sys_memory_test
 *  Description :
 *  -------------
 *  Perform simple memory test
 *
 *  Parameters :
 *  ------------
 *  a0 = RAM start address
 *  a1 = RAM size
 *
 *  Return values :
 *  ---------------
 *  0 If OK, error code != 1 if error
 *
 ************************************************************************/
SLEAF(sys_memory_test)

#define ADDR	t0
#define LAST	t1
#define STRIDE	4

	KSEG1A( a0 )

	/* Default to no error */
	li	v0, 0

	addu	LAST, a0, a1
	subu	LAST, STRIDE

	/* Write */
	move	ADDR, a0
1: 	sw	ADDR, 0(ADDR)
	bne	ADDR, LAST, 1b
	 addiu	ADDR, STRIDE

	/* Read */
	move	ADDR, a0
1:
	lw	t5, 0(ADDR)
	beq	t5, ADDR, 2f
	 nop
	li	v0, ERROR_MEMTEST_WORD
	lui	t5,0xbfc0
	sw	t5,(t5)
		
2:	bne	ADDR, LAST, 1b
	 addiu	ADDR, STRIDE
	
	/**** DONE ****/

	jr	ra
	 nop
END(sys_memory_test)


/******* Messages ********/

	.text

MSG( msg_cpu,	        "CPU" )
MSG( msg_platform,      "BOARD" )	
MSG( msg_ram,	        "RAM" )
MSG( msg_copy_code,     "COPYTEXT" )
MSG( msg_copy_data,     "COPYDATA" )		
MSG( msg_ramtest_hilo,  "RAM_HILO" )
MSG( msg_ramtest_yamon, "RAM_TEST" )
MSG( msg_ram_clear,	"CLEAR" )
MSG( msg_sp,		"STACK" )
MSG( msg_info,		"INFO" )
MSG( msg_cache_info,    "CINFO" )
MSG( msg_main,		"FIRSTC" )
#ifdef VERIFY_COPYTEXT
MSG( msg_verify,	"BAD_COPY" )
#endif /* VERIFY_COPYTEXT */

/* Error messages (see init.h) */
ERROR_MESSAGES
	
