
/************************************************************************
 *
 *  excep_s.S
 *
 *  First level exception handling functions.
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
#include <gdb_stub.h>
#include <sys_api.h>	
#include <product.h>	
#include <malta.h>	
	
/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public and static variables
 ************************************************************************/

	BSS
	
	/* 32 bit align */
	ALIGN(2)
	
	.globl EXCEP_shell_sp
EXCEP_shell_sp:
	.space	4

exc_handler_args:
	.space  8

ejtag_handler_arg:
	.space  4

excep_cp0_regs_mask:
	.space	4

excep_tmp:
	.space  4
	
exc_stack_low:
	.space  SYS_STACK_SIZE

	ALIGN(4) /* 16 bytes align */
exc_context:
	.space	EXCEP_SIZE_CONTEXT


ejtag_stack_low:
	.space  SYS_STACK_SIZE >> 4

	ALIGN(4) /* 16 bytes align */

	.globl ejtag_context
ejtag_context:
	.space	EXCEP_SIZE_CONTEXT


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

	.set noat
	.set noreorder
	
	.globl EXCEP_return
	
/************************************************************************
 *
 *                          fpuemul_putchar
 *  Description :
 *  -------------
 *
 *  This routine is called from fpu exception only.
 *  It sets up the minimum yamon context (gp) and calls sys_putchar.
 *  All registers except sp, gp, s0-s8, k0-k1  may be destroyed.
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = char to be written
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
LEAF( fpuemul_putchar )
	/* make room (must be 16 byte aligned) and save minimal context */
	addiu	sp, -8*4
	sw	gp, 16(sp)
	sw	ra, 20(sp)

	/* set up yamon minimal context */
	move	a1, a0
	jal	sys_putchar
	li	a0, DEFAULT_PORT
	
	/* restore minimal context */
	lw	ra, 20(sp)
	lw	gp, 16(sp)
	jr	ra
	addiu	sp, 8*4
END( fpuemul_putchar )


/************************************************************************
 *
 *                          EXCEP_init_reg_mask
 *  Description :
 *  -------------
 *  Determine what CP0/CP1 regs should be context switched.
 *
 *  Parameters :
 *  ------------
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( EXCEP_init_reg_mask )

    /* Determine availability of regs :
     *
     * Bit 0 = 32(1) or 64(0) bit CPU
     * Bit 1 = FPU
     * Bit 2 = TLB
     * Bit 3 = Watch registers
     * Bit 4 = MIPS32/MIPS64
     * ----- Following only apply to MIPS32/MIPS64
     * Bit 5 = EJTAG (MIPS32/64 only)
     * Bit 6 = MIPS32/MIPS64 Release 2
     * Bit 7 = SRSMap
     * Bit 8 = PageGrain
     */

    /* Check for 32 or 64 bit CPU */
    la		t0, sys_64bit
    lb		t0, 0(t0)
    bne		t0, zero, 1f
    move	t0, zero
    ori		t0, 0x1
1:
    /* Check for FPU */
    la		t1, sys_fpu
    lw		t1, 0(t1)
    beq		t1, zero, 1f
    nop
    ori		t0, 0x2
1:
    /* Check for MIPS32/MIPS64 */
    la		t1, sys_mips32_64
    lw		t1, 0(t1)
    bne		t1, zero, 1f
    nop
    b		non_mips32_64
    ori		t0, 0x4+0x8  /* Assume TLB + Watch for non MIPS32/MIPS64 */
1:
    ori		t0, 0x10

    /* Check for TLB based on Config.MT */
    MFC0_SEL_OPCODE( R_t1, R_C0_Config, R_C0_SelConfig )
    li		t2, M_ConfigMT
    and		t2, t1
    srl		t2, S_ConfigMT
    li		t3, K_ConfigMT_TLBMMU
    bne		t2, t3, 1f
    nop
    ori		t0, 0x4
1:
    /* Determine Release 2 based on Config.AR */
    li		t2, M_ConfigAR
    and		t1, t2
    srl		t1, S_ConfigAR
    beq		t1, zero, non_release2
    nop
    ori		t0, 0x40

    /* Determine SRSMap (Release 2 only) based on SRSCtl.HSS and Config3.VInt */
    MFC0_SEL_OPCODE( R_t1, R_C0_SRSCtl, R_C0_SelSRSCtl )
    li		t2, M_SRSCtlHSS
    and		t1, t2
    beq		t1, zero, 1f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config3, R_C0_SelConfig3 ) /* Since HSS!=0, Config3 is required */
    sll		t1, 31-S_Config3VInt
    bgez	t1, 1f
    nop
    ori		t0, 0x80
1:
    /* Determine PageGrain (Release 2 only) based on :
     *
     *   Config3.LPA (large physical addresses) 
     *   Config3.SP  (small page support)
     *
     *   TBD : SmartMIPS
     */
    MFC0_SEL_OPCODE( R_t1, R_C0_Config1, R_C0_SelConfig1 )
    sll		t1, 31-S_ConfigMore
    bgez	t1, 1f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config2, R_C0_SelConfig2 )
    sll		t1, 31-S_ConfigMore
    bgez	t1, 1f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config3, R_C0_SelConfig3 )
    li		t2, M_Config3LPA | M_Config3SP
    and		t1, t2
    beq		t1, zero, 1f
    nop
    ori		t0, 0x100
1:	
non_release2:
    /* Determine Watch registers based on Config1.WR */
    MFC0_SEL_OPCODE( R_t1, R_C0_Config1, R_C0_SelConfig1 )
    sll		t2, t1, 31-S_Config1WR
    bgez	t2, 1f
    nop
    ori		t0, 0x8
1:
    /* Determine EJTAG regs based on Config1.EP */
    sll		t1, 31-S_Config1EP
    bgez	t1, 1f
    nop
    ori		t0, 0x20
1:
non_mips32_64:
    /* Done. Store result and return */
    la		t1, excep_cp0_regs_mask
    jr 		ra
    sw		t0, 0(t1)

END( EXCEP_init_reg_mask )

		
/************************************************************************
 *
 *                          EXCEP_install_exc_in_ram
 *  Description :
 *  -------------
 *  Install jump instructions to exc handler at exception vector locations
 *
 *  Parameters :
 *  ------------
 *	
 *   a0: if set, install EIC vectors.
 *
 *  Return values :
 *  ---------------
 *  None
 *
 ************************************************************************/
LEAF( EXCEP_install_exc_in_ram )

	/* Install RAM exception vectors */

#define ADDR			a0  /* also used as arg to sys_cache..() */
#define STATUS_INT		t3
#define RA			t4
#define EICMODE			t5
#define EICOFFSET		t5
#define EICINCR			t6
#define EICCOUNT		t7	
		
	move	RA, ra
	move	EICMODE, a0

	/* Disable interrupts */
	MFC0(   t1, C0_Status )
	and	STATUS_INT, t1, M_StatusIE
	xor	t1,	    STATUS_INT	
	MTC0(   t1,	    C0_Status )
	
	/**** install yamon ram vectors ****/
	jal	setup_yamon_vector
	li	ADDR, 0x0	/* TLB refill     */

	jal	setup_yamon_vector
	li	ADDR, 0x80	/* XTLB refill    */

	la	t2, ram_vector_cacheerr
	jal	setup_yamon_vector_cacheerr
	li	ADDR, 0x100     /* Cache error    */
	
	la	t2, ram_vector_general
	jal	setup_yamon_vector_general
	li	ADDR, 0x180	/* General        */

	jal	setup_yamon_vector
	li	ADDR, 0x200	/* Int, CauseIV=1 */	

	beqz	EICMODE, noneicmode
	 nop
	
	jal	setup_yamon_vector_ejtag
	 li	ADDR, SYS_EJTAG_RAM_VECTOR_OFS

	jal	setup_yamon_vector
	 li	ADDR, SYS_NMI_RAM_VECTOR_OFS
	
	MFC0_SEL_OPCODE(R_t0, R_C0_IntCtl, R_C0_SelIntCtl)
	li	EICINCR, M_IntCtlVS
	and	EICINCR, t0
	
	li	EICCOUNT, 62
	move	EICOFFSET, EICINCR
	
1:	li	ADDR, 0x200
	jal	setup_yamon_vector
	 addu	ADDR, EICOFFSET
	addu	EICOFFSET, EICINCR
	bnez	EICCOUNT, 1b
	 subu	EICCOUNT, 1

	b	donevectors
	 nop

noneicmode:
	/*
	 * In non-EIC mode, install NMI/EJTAG compatibility
	 * so that old code will continue to work
	 */
	jal	setup_yamon_vector_ejtag
	 li	ADDR, SYS_OLD_EJTAG_RAM_VECTOR_OFS

	jal	setup_yamon_vector_ejtag_redirect
	 li	ADDR, SYS_EJTAG_RAM_VECTOR_OFS
	
	jal	setup_yamon_vector_nmi_forward
	 li	ADDR, SYS_OLD_NMI_RAM_VECTOR_OFS

	jal	setup_yamon_vector_nmi_redirect
	 li	ADDR, SYS_NMI_RAM_VECTOR_OFS
			
donevectors:	
	/* Clear BEV so that ram handlers are used */
	MFC0(   a0, C0_Status)
	li	t2, ~M_StatusBEV
	and	a0, t2
	MTC0(   a0, C0_Status)

	/* Restore interrupt enable state */
	or	a0, STATUS_INT
	MTC0(   a0, C0_Status )

	move	ra, RA

	/* Done */
	jr	ra
	nop

setup_yamon_vector_nmi_forward:
	.set	reorder
	la	t2, ram_vector_nmi_forward
	b	setup_yamon_vector_copy
	.set	noreorder
		
setup_yamon_vector_ejtag_redirect:
	.set	reorder
	la	t2, ram_vector_ejtag_redirect
	b	setup_yamon_vector_copy
	.set	noreorder
	
setup_yamon_vector_ejtag:
	la	t2, ram_vector_ejtag	

	/* Save existing data of exception vector location */
	/* Copy ram vector code to exception vector location */

setup_yamon_vector_copy:
	
	li	t1, KSEG0BASE
	or	ADDR, t1		/* Destination   */

	/* Install new yamon vector    */
	lw	t1, 0(t2)
	sw	t1, 0(ADDR)
	lw	t1, 4(t2)
	sw	t1, 4(ADDR)

	b	cache_flush_vector
	nop

setup_yamon_vector_nmi_redirect:
	.set	reorder
	la	t2, ram_vector_nmi_redirect
	b	setup_yamon_vector_general
	.set	noreorder
	
setup_yamon_vector:
	la	t2, ram_vector_normal

setup_yamon_vector_cacheerr:
setup_yamon_vector_general:
	/* Save existing data of exception vector location */
	/* Copy ram vector code to exception vector location */

	li	t1, KSEG0BASE
	or	ADDR, t1		/* Destination   */

	/* Install new yamon vector    */
	lw	t1, 0(t2)
	sw	t1, 0(ADDR)
	lw	t1, 4(t2)		/* li k1, 0x0000  */
	and	t2, ADDR, 0xffff
	or	t1, t2			/* li k1, vector_offset */
	sw	t1, 4(ADDR)


cache_flush_vector:

	/* Whenever an instruction has been stored to KSEG0, we must
	 * make sure the instruction has been flushed to physical memory
	 * and invalidate the corresponding I-Cache line.
	 *     sys_dcache_flush_addr modifies only t1.
	 *     sys_icache_invalidate_addr modifies only t1.
	 */

	move	t2, ra

	jal     sys_dcache_flush_addr
	nop
	jal	sys_icache_invalidate_addr
	move	a1, zero /* No need to flush pipeline */
			
	jr	t2
	nop

END( EXCEP_install_exc_in_ram )	


SLEAF( EXCEP_yamon_ram_vectors )
	
ram_vector_normal:
	/*  This code is copied to the various ram exception vector
	 *  locations (2 words are copied).
	 */
	j	exc_handler
	li	k0, 0		/* when copied: "li k0, vector_offset" */

ram_vector_cacheerr:
	/*  This code is copied to the cacheerr exception ram vector
	 *  location (2 words are copied).
	 */
	j	exc_handler_cacheerr
	li	k0, 0		/* when copied: "li k0, vector_offset" */

ram_vector_general:
	/*  This code is copied to the general exception ram vector
	 *  location (2 words are copied).
	 */
	j	exc_handler_general
	li	k0, 0		/* when copied: "li k0, vector_offset" */

ram_vector_ejtag:
	/*  This code is copied to the exception vector
	 *  location reserved (by YAMON) for EJTAG:
	 *  SYS_EJTAG_RAM_VECTOR_OFFS/SYS_OLD_EJTAG_RAM_VECTOR_OFFS
	 *  (2 words are copied).
	 *  This entry is software redirected from addr 0xbf000480.
	 *  Please see the file reset.S for a note about hazards.
	 */
	j	exc_handler_ejtag
        mtc0    k1, C0_DESAVE

ram_vector_ejtag_redirect:
	/*  This code is copied to the exception vector
	 *  location reserved (by YAMON) for EJTAG:
	 *  SYS_EJTAG_RAM_VECTOR_OFS
	 *  (2 words are copied).
	 *  This entry is software redirected from addr 0xbf000480.
	 *  Please see the file reset.S for a note about hazards.
	 */
	b	.-SYS_EJTAG_RAM_VECTOR_OFS+SYS_OLD_EJTAG_RAM_VECTOR_OFS
	 nop

ram_vector_nmi_forward:
	/*  This code is copied to the exception vector
	 *  location reserved (by YAMON) for EJTAG:
	 *  SYS_OLD_NMI_RAM_VECTOR_OFS
	 *  (2 words are copied).
	 *  This entry is software redirected from addr 0xbf000480.
	 *  Please see the file reset.S for a note about hazards.
	 */
	j	exc_handler_nmi
	 nop

ram_vector_nmi_redirect:
	/*  This code is copied to the exception vector
	 *  location reserved (by YAMON) for NMI:
	 *  SYS_NMI_RAM_VECTOR_OFS
	 *  (2 words are copied).
	 *  This entry is software redirected from addr 0xbf000480.
	 *  Please see the file reset.S for a note about hazards.
	 */
	b	.-SYS_NMI_RAM_VECTOR_OFS+SYS_OLD_NMI_RAM_VECTOR_OFS
	 li	k0, 0		/* when copied: "li k0, vector_offset" */
				
END( EXCEP_yamon_ram_vectors )


SLEAF( EXCEP_yamon_vector_handler )
/*********************************************************************
**      Special ejtag exception entry
*/
exc_handler_ejtag:

	/**** Store k0 (k1 has been stored in CP0) ****/
	/* Store sp and k1 in readable exception context */
	la	k1,   sys_64bit
	lb	k1,   0(k1)
	bne	k1,   zero, 64f
	nop

	la	k1,   ejtag_context
	sw	k0,   GDB_REG32_OFS(GDB_FR_REG26)(k1)
        mfc0    k0,   C0_DESAVE
	sw	k0,   GDB_REG32_OFS(GDB_FR_REG27)(k1)
	b	2f
	sw	$31,  GDB_REG32_OFS(GDB_FR_REG31)(k1)

64:
	la	k1,   ejtag_context
SET_MIPS3()
        sd	k0,   GDB_REG64_OFS(GDB_FR_REG26)(k1)
        mfc0    k0,   C0_DESAVE
        sd	k0,   GDB_REG64_OFS(GDB_FR_REG27)(k1)
        sd	$31,  GDB_REG64_OFS(GDB_FR_REG31)(k1)
SET_MIPS0()
2:

	/**** Store context ****/
	jal	EXCEP_store_gpr_but_k0k1ra
	nop
	jal	store_cp1_gpr_cp0_control
	nop

	/* Set up arguments for exception_ejtag() */
	la	a0, ejtag_handler_arg
	li	a1, 0
1:
	sw	a1, 0(a0)
	move	a0, k1

	/* call low level exception handler */

	jal	exception_ejtag /* It is imperative that k1 survives */
	addiu	sp, k1, -4*4

	/* If exception handler returns, we jump to restore code */
	b	EXCEP_exc_handler_ret
	move	a0, k1

rerun_ejtag:
	la	a0, ejtag_handler_arg
	lw	a1, 0(a0)
	b	1b
	add	a1, 1	/* count up exc_return_flag argument */


/*************************************************************
**      central entry for cacheerr exceptions
**      this code always runs in KSEG1
*/
exc_handler_cacheerr:
	/* Store unmodified C0_Config register in uncached memory */
	/* This value is put into exc_context in excep.c */
	la	k0, EXCEP_C0_Config_cacheerr
	li	k1, KSEG1BASE
	or	k1, k0
	mfc0	k0, C0_Config
	sw	k0, 0(k1)

	/* Change C0_Config to run uncached */
	or	k0, M_ConfigK0
	xor	k0, M_ConfigK0 ^ (K_CacheAttrU << S_ConfigK0)
	MTC0(	k0, C0_Config )

	/* Set up k0 again and jump to exc_handler */
	b	exc_handler
	li	k0, SYS_CACHEERR_RAM_VECTOR_OFS


/*************************************************************
**      central entry for NMI exceptions
*/
exc_handler_nmi:
	/* Determine processor manufacturer */
	MFC0(   k0, C0_PRId)
	li	k1, M_PRIdCoID
	and     k0, k1
	beqz	k0, NoMT		# Not a MIPS32/MIPS64 implementation
	 nop
	
	.set	push
	.set	mips32
	MFC0(	k0, C0_Config)		# Read Config0 for the M bit
	bgez	k0, NoMT		# Branch if no Config1
	 nop
	MFC0(	k0, C0_Config1)		# Read Config1 for the M bit
	bgez	k0, NoMT		# Branch if no Config2
	 nop
	MFC0(	k0, C0_Config2)		# Read Config2 for the M bit
	bgez	k0, NoMT		# Branch if no Config3
	 nop
	MFC0(	k0, C0_Config3)		# Finally, read what we really want
	andi	k0, M_Config3MT		# Is the MT ASE implemented?
	beqz	k0, NoMT		# Branch if so
	 nop

HaveMT:
	MFC0(	k0, C0_TCBind)		# Is this VPE0?
	andi	k0, M_TCBindCurVPE

/*
 * If any TC arrives here that is not running on VPE0, it sits in the following
 * tight loop waiting to be halted by the master VPE
 */

10:	bnez	k0, 10b			# Enter tight loop if not master processor
	 nop

/*
 * Here on VPE0.  Stop all other VPEs and threads
 */
	DVPE(R_zero)			# Stop all other VPEs
	DMT(R_zero)			# Stop all threads on this VPE
	ehb

NoMT:
	.set	pop	
	/* Reset k0 and jump to general handler */
	
	b	exc_handler
	 li	k0, SYS_NMI_RAM_VECTOR_OFS


/*************************************************************
**      central entry for general exceptions
*/
exc_handler_general:

	/* Store exception vector offset */
	la	k1, exc_handler_args
	sw	k0, 0(k1)
	
	/**** Test for FPU emulator ****/
	la	k1, FPUEMUL_status
	lb	k1, 0(k1)
	beqz	k1, exc_default
	nop
	la	k1, exc_default
	j	FPUEMUL_handler
	nop


/*************************************************************
**      central entry common to most exceptions
*/
exc_handler:

	/* Store exception vector offset */
	la	k1, exc_handler_args
	sw	k0, 0(k1)

exc_default:

	/**** Store context ****/
	j	EXCEP_store_gpr_but_k0k1_ss
	nop
	
exc_handler_after_gpr:
	
	jal	store_cp1_gpr_cp0_control
	nop

	/* Set up arguments for exception_sr() */

	/* Clear 'EXCEP_return_flag' */
	la	a1, exc_handler_args
	li	a3, 0
exc_rerun:	     
	sw	a3, 4(a1)						
	move	a2, k1
	lw	a1, 0(a1)

	/* Set a0 to the cause code field of the CAUSE register */
	MFC0(   t0, C0_Cause )
	li      t1, M_CauseExcCode
	and     a0, t0, t1
	srl     a0, S_CauseExcCode

	/* call low level exception handler */

	jal     exception_sr /* It is imperative that k1 survives */
	addiu	sp, k1, -4*4

	/* If exception handler returns, we jump to restore code */
	b	EXCEP_exc_handler_ret
	move	a0, k1
		
	/*  A registered exception handler is allowed to call
	 *  EXCEP_return (see below) in case it does not want to
	 *  handle the exception. 
	 *  The address of EXCEP_return was returned when the 
	 *  exception handler was registered.
	 *  We need to flag whether this is the initial handling
	 *  of an exception or such a return from a registered 
	 *  exception handler. This is done using the 
	 *  EXCEP_return_flag variable.
	 */
	
EXCEP_return :
	la	a2, ejtag_context
	beq	a2, k1, rerun_ejtag
	nop
	/* Set 'EXCEP_return_flag' */
	la	a1, exc_handler_args
	la	k1, exc_context
	lw	a3, 4(a1)
	b	exc_rerun
	add	a3, 1	/* count up exc_return_flag argument */
END( EXCEP_yamon_vector_handler )


LEAF( EXCEP_save_context )
	/* entry parameters
	** a0  pointer to context
	*/     
	/**** Store current stack pointer for reuse by appl_if ****/
	la	v0, EXCEP_shell_sp
	sw	sp, 0(v0)

	/**** Store context ****/
	li	v0, 1	    /* return value after context restore */
	la	k0, sys_64bit
	lb	k0, 0(k0)
	bne	k0, zero, 64f
	move	k1, a0
	sw	$31, GDB_REG32_OFS(GDB_FR_REG31)(k1)
	jal	EXCEP_store_gpr_but_k0k1ra
	nop
	jal	store_cp1_gpr_cp0_control
	nop
	/* restore ra and insert as CP0_EPC */
	lw	ra,  GDB_REG32_OFS(GDB_FR_REG31)(k1)
	b	32f
	sw	ra,  GDB_REG32_OFS(GDB_FR_EPC)(k1)
64:
SET_MIPS3()
        sd	$31,  GDB_REG64_OFS(GDB_FR_REG31)(k1)
	jal	EXCEP_store_gpr_but_k0k1ra
	nop
	jal	store_cp1_gpr_cp0_control
	nop
	/* restore ra and insert as CP0_EPC */
        ld	ra,   GDB_REG64_OFS(GDB_FR_REG31)(k1)
	sd	ra,   GDB_REG64_OFS(GDB_FR_EPC)(k1)
SET_MIPS0()
32:
	lw	v0,  GDB_REG32_OFS(GDB_FR_STATUS)(k1)
	or	v0,  M_StatusERL | M_StatusEXL
	xor	v0,  M_StatusERL
	sw	v0,  GDB_REG32_OFS(GDB_FR_STATUS)(k1)

	jr	ra
	li	v0, 0          /* return value after context save */

END( EXCEP_save_context )


LEAF( EXCEP_get_context_ptr )

        /* Retrieves pointer to current exception context. */

	la	v0, exc_context
	beq	v0, k1, 2f
	nop
	la	v0, ejtag_context
1:
	bne	v0, k1, 1b /* Stay here if k1 does not point to a known context */
	nop
2:
	jr	ra
	nop

END( EXCEP_get_context_ptr )


LEAF( EXCEP_exc_handler_ret_ss )

        /* Same functionality as EXCEP_exc_handler_ret()
         * except that for CPUs implementing MIPS32/MIPS64
         * Release 2, we first make sure to use shadowset 0.
         */

        la	t0, excep_cp0_regs_mask
        lw	t0, 0(t0)
        sll	t0, 31-6 /* Release 2 */
	bgez	t0, EXCEP_exc_handler_ret
	nop

	/* Clear ERL, BEV, but first set KSU=0 (kernel mode), so that
	 * we stay in Kernel mode independent of EXL. 
	 * We then clear SRSCtl and perform an ERET in order to start
	 * using shadowset 0.
	 */
	MFC0(	t0, C0_Status )
	li	t1, (M_StatusKSU | M_StatusIE)
	or	t0, t1
	xor	t0, t1
	MTC0(	t0, C0_Status )
	li	t1, (M_StatusERL | M_StatusBEV)
	or	t0, t1
	xor	t0, t1
	MTC0(	t0, C0_Status )

        MTC0_SEL_OPCODE( R_zero, R_C0_SRSCtl, R_C0_SelSRSCtl )	

	/* Copy a0 to previous shadow set (now 0) so that it survives eret */
	WRPGPR( R_a0, R_a0 )

	la	t0, EXCEP_exc_handler_ret
	MTC0(	t0, C0_EPC)

SET_MIPS3()
	eret
SET_MIPS0()

END( EXCEP_exc_handler_ret_ss )	
	
	
LEAF( EXCEP_exc_handler_ret )
	
	/*  Return context and return from exception.
	 *
	 *  a0 = Pointer to context to be restored.
 	 *  restore_control_gpr_but_k1() will perform jr k1 rather than jr ra.
	 */

	/* Determine ejtag return based on context pointer */
	la	k0, ejtag_context
	beq	a0, k0, return_from_ejtag
	nop

	/* Normal return from exception */
	la	k1, restore_done
	b	restore_control_gpr_but_k1
	move	k0, a0
restore_done:

SET_MIPS3()
	eret
SET_MIPS0()

return_from_ejtag:
	/* EJTAG return from exception */
	la	k1, restore_done_ejtag
	b	restore_control_gpr_but_k1
	move	k0, a0
restore_done_ejtag:
        mfc0    k1, C0_DESAVE			   /* "restore" k1 */
	DERET
	nop
	
END( EXCEP_exc_handler_ret )	

	
LEAF( EXCEP_exc_handler_jump )

	/*  Restore context and jump to function given by a0.
	 *  a0 = Pointer to function taking over (assume 32 bit)
	 *
	 *  Function is of type t_EXCEP_esr defined as :	
	 *    typedef void (*t_EXCEP_esr)(void);
	 *
	 *  This entry is invoked only for jumping to a handler
	 *  registered by EXCEP_register_esr(x,TRUE,x,x).
	 *  The a0 function is responsible for handling the
	 *  exception and possibly issue an eret (or deret
	 *  in the case of an EJTAG exception).
	 *
	 *  At this point in the exception processing only GPR
	 *  registers have modified, so there is no need to
	 *  restore any control or floating point registers.
	 * 
	 *  Caution: restoring control registers at this point
	 *  may cause an exception -> infinite exception loop.
	 *
	 */

	/*  a0 holds return address. Move this to k1, since
	 *  EXCEP_restore_gpr_but_k1() will perform a jr k1
	 */
	/* Set context to be restored */
	move	k0, k1

	/* Determine ejtag jump based on context pointer */
	la	k1, ejtag_context
	bne	k1, k0, EXCEP_restore_gpr_but_k1  /* branch normally taken */
	move	k1, a0

	/* jump in ejtag context - we need to restore ALL registers */
	la	k1, excep_tmp
	sw	a0, 0(k1)	/* a0 is type (*t_EXCEP_esr)() === UINT32 */
	la	k1, ejtag_jump_restore_done
	b	restore_control_gpr_but_k1
	nop
ejtag_jump_restore_done:
	/* we have only k1 left - "restore" return address */
	la	k1, excep_tmp
	lw	k1, 0(k1)
	jr	k1
        mfc0    k1, C0_DESAVE			   /* restore k1 */

END( EXCEP_exc_handler_jump )	


LEAF(EXCEP_store_gpr_but_k0k1ra)
	/**** Store context ****/
	la	k0, sys_64bit
	lb	k0, 0(k0)
	bne	k0, zero, store_gpr_64bit
	nop

	/* Store 32 bit CPU Registers */

	sw	$0,   GDB_REG32_OFS(GDB_FR_REG0 )(k1)
	sw	$1,   GDB_REG32_OFS(GDB_FR_REG1 )(k1)
	sw	$2,   GDB_REG32_OFS(GDB_FR_REG2 )(k1)
	sw	$3,   GDB_REG32_OFS(GDB_FR_REG3 )(k1)
	sw	$4,   GDB_REG32_OFS(GDB_FR_REG4 )(k1)
	sw	$5,   GDB_REG32_OFS(GDB_FR_REG5 )(k1)
	sw	$6,   GDB_REG32_OFS(GDB_FR_REG6 )(k1)
	sw	$7,   GDB_REG32_OFS(GDB_FR_REG7 )(k1)
	sw	$8,   GDB_REG32_OFS(GDB_FR_REG8 )(k1)
	sw	$9,   GDB_REG32_OFS(GDB_FR_REG9 )(k1)
	sw	$10,  GDB_REG32_OFS(GDB_FR_REG10)(k1)
	sw	$11,  GDB_REG32_OFS(GDB_FR_REG11)(k1)
	sw	$12,  GDB_REG32_OFS(GDB_FR_REG12)(k1)
	sw	$13,  GDB_REG32_OFS(GDB_FR_REG13)(k1)
	sw	$14,  GDB_REG32_OFS(GDB_FR_REG14)(k1)
	sw	$15,  GDB_REG32_OFS(GDB_FR_REG15)(k1)
	sw	$16,  GDB_REG32_OFS(GDB_FR_REG16)(k1)
	sw	$17,  GDB_REG32_OFS(GDB_FR_REG17)(k1)
	sw	$18,  GDB_REG32_OFS(GDB_FR_REG18)(k1)
	sw	$19,  GDB_REG32_OFS(GDB_FR_REG19)(k1)
	sw	$20,  GDB_REG32_OFS(GDB_FR_REG20)(k1)
	sw	$21,  GDB_REG32_OFS(GDB_FR_REG21)(k1)
	sw	$22,  GDB_REG32_OFS(GDB_FR_REG22)(k1)
	sw	$23,  GDB_REG32_OFS(GDB_FR_REG23)(k1)
	sw	$24,  GDB_REG32_OFS(GDB_FR_REG24)(k1)
	sw	$25,  GDB_REG32_OFS(GDB_FR_REG25)(k1)
	/* Not k0, k1 = $26, $27 */
	sw	$28,  GDB_REG32_OFS(GDB_FR_REG28)(k1)
	sw	$29,  GDB_REG32_OFS(GDB_FR_REG29)(k1)
	sw	$30,  GDB_REG32_OFS(GDB_FR_REG30)(k1)
        /* Not ra = $31 (saved separately) */

	jr	ra
	nop

store_gpr_64bit :

	/* Store 64 bit CPU Registers */

SET_MIPS3()

        sd	$0,   GDB_REG64_OFS(GDB_FR_REG0 )(k1)
        sd	$1,   GDB_REG64_OFS(GDB_FR_REG1 )(k1)
        sd	$2,   GDB_REG64_OFS(GDB_FR_REG2 )(k1)
        sd	$3,   GDB_REG64_OFS(GDB_FR_REG3 )(k1)
        sd	$4,   GDB_REG64_OFS(GDB_FR_REG4 )(k1)
        sd	$5,   GDB_REG64_OFS(GDB_FR_REG5 )(k1)
        sd	$6,   GDB_REG64_OFS(GDB_FR_REG6 )(k1)
        sd	$7,   GDB_REG64_OFS(GDB_FR_REG7 )(k1)
        sd	$8,   GDB_REG64_OFS(GDB_FR_REG8 )(k1)
        sd	$9,   GDB_REG64_OFS(GDB_FR_REG9 )(k1)
        sd	$10,  GDB_REG64_OFS(GDB_FR_REG10)(k1)
        sd	$11,  GDB_REG64_OFS(GDB_FR_REG11)(k1)
        sd	$12,  GDB_REG64_OFS(GDB_FR_REG12)(k1)
        sd	$13,  GDB_REG64_OFS(GDB_FR_REG13)(k1)
        sd	$14,  GDB_REG64_OFS(GDB_FR_REG14)(k1)
        sd	$15,  GDB_REG64_OFS(GDB_FR_REG15)(k1)
        sd	$16,  GDB_REG64_OFS(GDB_FR_REG16)(k1)
        sd	$17,  GDB_REG64_OFS(GDB_FR_REG17)(k1)
        sd	$18,  GDB_REG64_OFS(GDB_FR_REG18)(k1)
        sd	$19,  GDB_REG64_OFS(GDB_FR_REG19)(k1)
        sd	$20,  GDB_REG64_OFS(GDB_FR_REG20)(k1)
        sd	$21,  GDB_REG64_OFS(GDB_FR_REG21)(k1)
        sd	$22,  GDB_REG64_OFS(GDB_FR_REG22)(k1)
        sd	$23,  GDB_REG64_OFS(GDB_FR_REG23)(k1)
        sd	$24,  GDB_REG64_OFS(GDB_FR_REG24)(k1)
        sd	$25,  GDB_REG64_OFS(GDB_FR_REG25)(k1)
	/* Not k0, k1 = $26, $27 */
        sd	$28,  GDB_REG64_OFS(GDB_FR_REG28)(k1)
        sd	$29,  GDB_REG64_OFS(GDB_FR_REG29)(k1)
        sd	$30,  GDB_REG64_OFS(GDB_FR_REG30)(k1)
        /* Not ra = $31 (saved separately) */
SET_MIPS0()

	jr	ra
	nop

END(EXCEP_store_gpr_but_k0k1ra)


LEAF( sys_store_control_regs )

    /* Store the CP0 registers that are available for the CPU in use.
     * 
     * We skip the following registers, since YAMON doesn't depend
     * on them and GDB doesn't request them :
     *
     * LLAddr
     * Xcontext
     * TraceControl
     * TraceControl2
     * UserTraceData
     * TraceBPC
     * PerfCnt
     * ErrCtl
     * CacheErr
     * TagLo
     * DataLo
     * TagHi
     * DataHi
     * DESAVE
     */

    /* Get availability of regs :
     *
     * Bit 0 = 32(1) or 64(0) bit CPU
     * Bit 1 = FPU
     * Bit 2 = TLB
     * Bit 3 = Watch registers
     * Bit 4 = MIPS32/MIPS64
     * ----- Following only apply to MIPS32/MIPS64
     * Bit 5 = EJTAG (MIPS32/64 only)
     * Bit 6 = MIPS32/MIPS64 Release 2
     * Bit 7 = SRSMap
     * Bit 8 = PageGrain
     */

    la		t0, excep_cp0_regs_mask
    lw		t0, 0(t0)

    /* 32 bit or 64 bit */	
    sll		t1, t0, 31-0
    bgez	t1, store_control_64bit
    nop

    /* Status register (always available) */
    MFC0(       t1, C0_Status)
    sw		t1, GDB_REG32_OFS(GDB_FR_STATUS)(a0)
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG12)(a0)

    /* Check for FPU */
    sll		t2, t0, 31-1
    bgez	t2, 1f
    nop
    /* Check that FPU is enabled */
    sll		t1, 31-S_StatusCU1
    bgez	t1, 1f
    nop

    /* Store FPU registers */
    cfc1        t1, $0
    cfc1	t2, $31
    sw		t1, GDB_REG32_OFS(GDB_FR_FIR)(a0)
    sw		t2, GDB_REG32_OFS(GDB_FR_FSR)(a0)
1:
    /* Registers available for all MIPS CPUs */
    mflo	t1
    sw		t1, GDB_REG32_OFS(GDB_FR_LO)(a0)

    mfhi	t1
    sw		t1, GDB_REG32_OFS(GDB_FR_HI)(a0)		

    MFC0(       t1, C0_BadVAddr )
    sw		t1, GDB_REG32_OFS(GDB_FR_BADVADDR)(a0)
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG8)(a0)	

    MFC0(	t1, C0_Count )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG9)(a0)

    MFC0(	t1, C0_Compare )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG11)(a0)

    MFC0(	t1, C0_Cause )
    sw		t1, GDB_REG32_OFS(GDB_FR_CAUSE)(a0)
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG13)(a0)

    MFC0(	t1, C0_EPC )
    sw		t1, GDB_REG32_OFS(GDB_FR_EPC)(a0)
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_REG14)(a0)

    MFC0(	t1, C0_PRId )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_PRID)(a0)

    MFC0(	t1, C0_ErrorEPC )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_ERROREPC)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_Config, R_C0_SelConfig )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG)(a0)

    /* Check for TLB */
    sll		t1, t0, 31-2
    bgez	t1, 1f
    nop

    /* TLB */
    MFC0(       t1, C0_Index )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_INDEX)(a0)

    MFC0(	t1, C0_Random )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_RANDOM)(a0)

    MFC0(	t1, C0_EntryLo0 )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYLO0)(a0)
		
    MFC0(	t1, C0_EntryLo1 )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYLO1)(a0)

    MFC0(	t1, C0_Context )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_CONTEXT)(a0)

    MFC0(	t1, C0_PageMask )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_PAGEMASK)(a0)

    MFC0(	t1, C0_Wired )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_WIRED)(a0)

    MFC0(	t1, C0_EntryHi )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYHI)(a0)
1:
    /* Check for Watch registers */
    sll		t1, t0, 31-3
    bgez	t1, 1f
    nop

    /* Watch registers */
    MFC0(       t1, C0_WatchLo )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_WATCHLO)(a0)

    MFC0(       t1, C0_WatchHi )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_WATCHHI)(a0)
1:
    /* Check for MIPS32/MIPS64 */
    sll		t1, t0, 31-4
    bgez	t1, 1f
    nop

    /* MIPS32/MIPS64 */
    
    /* Config1 (required for MIPS32/MIPS64 CPUs) */
    MFC0_SEL_OPCODE( R_t1, R_C0_Config1, R_C0_SelConfig1 )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG1)(a0)

    /* Config2 depends on M bit of Config1 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config2, R_C0_SelConfig2 )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG2)(a0)

    /* Config3 depends on M bit of Config2 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config3, R_C0_SelConfig3 )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG3)(a0)
2:
    /* Check for EJTAG */
    sll		t1, t0, 31-5
    bgez	t1, 2f
    nop

    /* EJTAG regs */
    MFC0(	t1, C0_Debug )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_DEBUG)(a0)

    MFC0(	t1, C0_DEPC )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_DEPC)(a0)
2:
    /* Check for Release 2 */
    sll		t1, t0, 31-6
    bgez	t1, 2f
    nop

    /* Release 2 */
    MFC0(       t1, C0_HWREna )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_HWRENA)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_IntCtl, R_C0_SelIntCtl )
    sw		t1, GDB_REG32_OFS(GDB_FR_CP0_INTCTL)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_SRSCtl, R_C0_SelSRSCtl )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_SRSCTL)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_EBase, R_C0_SelEBase )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_EBASE)(a0)

    /* SRSMap */
    sll		t1, t0, 31-7
    bgez	t1, 2f
    nop

    MFC0_SEL_OPCODE( R_t1, R_C0_SRSMap, R_C0_SelSRSMap )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_SRSMAP)(a0)
2:
    /* PageGrain */
    sll		t1, t0, 31-8
    bgez	t1, 1f
    nop

    MFC0_SEL_OPCODE( R_t1, R_C0_PageGrain, R_C0_SelPageGrain )
    sw	        t1, GDB_REG32_OFS(GDB_FR_CP0_PAGEGRAIN)(a0)
1:
    /* Done */
    jr ra
    nop

store_control_64bit :

SET_MIPS3()
	
    /* Status register (always available) */
    MFC0(       t1, C0_Status)
    sd		t1, GDB_REG64_OFS(GDB_FR_STATUS)(a0)
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG12)(a0)

    /* Check for FPU */
    sll		t2, t0, 31-1
    bgez	t2, 1f
    nop
    /* Check that FPU is enabled */
    sll		t1, 31-S_StatusCU1
    bgez	t1, 1f
    nop

    /* Store FPU registers */
    cfc1        t1, $0
    cfc1	t2, $31
    sd		t1, GDB_REG64_OFS(GDB_FR_FIR)(a0)
    sd		t2, GDB_REG64_OFS(GDB_FR_FSR)(a0)
1:
    /* Registers available for all MIPS CPUs */
    mflo	t1
    sd		t1, GDB_REG64_OFS(GDB_FR_LO)(a0)

    mfhi	t1
    sd		t1, GDB_REG64_OFS(GDB_FR_HI)(a0)		

    DMFC0(      t1, C0_BadVAddr )
    sd		t1, GDB_REG64_OFS(GDB_FR_BADVADDR)(a0)
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG8)(a0)	

    MFC0(	t1, C0_Count )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG9)(a0)

    MFC0(	t1, C0_Compare )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG11)(a0)

    MFC0(	t1, C0_Cause )
    sd		t1, GDB_REG64_OFS(GDB_FR_CAUSE)(a0)
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG13)(a0)

    DMFC0(	t1, C0_EPC )
    sd		t1, GDB_REG64_OFS(GDB_FR_EPC)(a0)
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_REG14)(a0)

    MFC0(	t1, C0_PRId )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_PRID)(a0)

    DMFC0(	t1, C0_ErrorEPC )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_ERROREPC)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_Config, R_C0_SelConfig )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG)(a0)

    /* Check for TLB */
    sll		t1, t0, 31-2
    bgez	t1, 1f
    nop

    /* TLB */
    MFC0(       t1, C0_Index )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_INDEX)(a0)

    MFC0(	t1, C0_Random )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_RANDOM)(a0)

    DMFC0(	t1, C0_EntryLo0 )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYLO0)(a0)
		
    DMFC0(	t1, C0_EntryLo1 )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYLO1)(a0)

    DMFC0(	t1, C0_Context )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_CONTEXT)(a0)

    MFC0(	t1, C0_PageMask )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_PAGEMASK)(a0)

    MFC0(	t1, C0_Wired )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_WIRED)(a0)

    DMFC0(	t1, C0_EntryHi )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYHI)(a0)
1:
    /* Check for Watch registers */
    sll		t1, t0, 31-3
    bgez	t1, 1f
    nop

    /* Watch registers */
    DMFC0(      t1, C0_WatchLo )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_WATCHLO)(a0)

    DMFC0(      t1, C0_WatchHi )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_WATCHHI)(a0)
1:
    /* Check for MIPS32/MIPS64 */
    sll		t1, t0, 31-4
    bgez	t1, 1f
    nop

    /* MIPS32/MIPS64 */
    
    /* Config1 (required for MIPS32/MIPS64 CPUs) */
    MFC0_SEL_OPCODE( R_t1, R_C0_Config1, R_C0_SelConfig1 )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG1)(a0)

    /* Config2 depends on M bit of Config1 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config2, R_C0_SelConfig2 )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG2)(a0)

    /* Config3 depends on M bit of Config2 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    MFC0_SEL_OPCODE( R_t1, R_C0_Config3, R_C0_SelConfig3 )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG3)(a0)
2:
    /* Check for EJTAG */
    sll		t1, t0, 31-5
    bgez	t1, 2f
    nop

    /* EJTAG regs */
    MFC0(	t1, C0_Debug )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_DEBUG)(a0)

    DMFC0(	t1, C0_DEPC )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_DEPC)(a0)
2:
    /* Check for Release 2 */
    sll		t1, t0, 31-6
    bgez	t1, 2f
    nop

    /* Release 2 */
    MFC0(       t1, C0_HWREna )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_HWRENA)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_IntCtl, R_C0_SelIntCtl )
    sd		t1, GDB_REG64_OFS(GDB_FR_CP0_INTCTL)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_SRSCtl, R_C0_SelSRSCtl )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_SRSCTL)(a0)

    MFC0_SEL_OPCODE( R_t1, R_C0_EBase, R_C0_SelEBase )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_EBASE)(a0)

    /* SRSMap */
    sll		t1, t0, 31-7
    bgez	t1, 2f
    nop

    MFC0_SEL_OPCODE( R_t1, R_C0_SRSMap, R_C0_SelSRSMap )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_SRSMAP)(a0)
2:
    /* PageGrain */
    sll		t1, t0, 31-8
    bgez	t1, 1f
    nop

    MFC0_SEL_OPCODE( R_t1, R_C0_PageGrain, R_C0_SelPageGrain )
    sd	        t1, GDB_REG64_OFS(GDB_FR_CP0_PAGEGRAIN)(a0)
1:

SET_MIPS0()
	
    /* Done */	
    jr ra
    nop
	
END( sys_store_control_regs )

	

LEAF( EXCEP_restore_gpr_but_k1 )

        /* k0 points to context, k1 = return address */

	la	t6, sys_64bit
	lb	t6, 0(t6)
	bne	t6, zero, 64f
	nop

	/* Restore 32 bit CPU register bank */	
	lw	$0,   GDB_REG32_OFS(GDB_FR_REG0 )(k0)
	lw	$1,   GDB_REG32_OFS(GDB_FR_REG1 )(k0)
	lw	$2,   GDB_REG32_OFS(GDB_FR_REG2 )(k0)
	lw	$3,   GDB_REG32_OFS(GDB_FR_REG3 )(k0)
	lw	$4,   GDB_REG32_OFS(GDB_FR_REG4 )(k0)
	lw	$5,   GDB_REG32_OFS(GDB_FR_REG5 )(k0)
	lw	$6,   GDB_REG32_OFS(GDB_FR_REG6 )(k0)
	lw	$7,   GDB_REG32_OFS(GDB_FR_REG7 )(k0)
	lw	$8,   GDB_REG32_OFS(GDB_FR_REG8 )(k0)
	lw	$9,   GDB_REG32_OFS(GDB_FR_REG9 )(k0)
	lw	$10,  GDB_REG32_OFS(GDB_FR_REG10)(k0)
	lw	$11,  GDB_REG32_OFS(GDB_FR_REG11)(k0)
	lw	$12,  GDB_REG32_OFS(GDB_FR_REG12)(k0)
	lw	$13,  GDB_REG32_OFS(GDB_FR_REG13)(k0)
	lw	$14,  GDB_REG32_OFS(GDB_FR_REG14)(k0)
	lw	$15,  GDB_REG32_OFS(GDB_FR_REG15)(k0)
	lw	$16,  GDB_REG32_OFS(GDB_FR_REG16)(k0)
	lw	$17,  GDB_REG32_OFS(GDB_FR_REG17)(k0)
	lw	$18,  GDB_REG32_OFS(GDB_FR_REG18)(k0)
	lw	$19,  GDB_REG32_OFS(GDB_FR_REG19)(k0)
	lw	$20,  GDB_REG32_OFS(GDB_FR_REG20)(k0)
	lw	$21,  GDB_REG32_OFS(GDB_FR_REG21)(k0)
	lw	$22,  GDB_REG32_OFS(GDB_FR_REG22)(k0)
	lw	$23,  GDB_REG32_OFS(GDB_FR_REG23)(k0)
	lw	$24,  GDB_REG32_OFS(GDB_FR_REG24)(k0)
	lw	$25,  GDB_REG32_OFS(GDB_FR_REG25)(k0)
	/* Not k0 and k1 = $26, $27 */	
	lw	$28,  GDB_REG32_OFS(GDB_FR_REG28)(k0)
	lw	$29,  GDB_REG32_OFS(GDB_FR_REG29)(k0)
	lw	$30,  GDB_REG32_OFS(GDB_FR_REG30)(k0)
	lw	$31,  GDB_REG32_OFS(GDB_FR_REG31)(k0)

	jr	k1
	lw	$26,  GDB_REG32_OFS(GDB_FR_REG26)(k0) /* "restore" k0 */
	
64:

SET_MIPS3()
	
	/* Restore 64 bit CPU register bank */	
        ld      $0,  GDB_REG64_OFS(GDB_FR_REG0 )(k0)
        ld      $1,  GDB_REG64_OFS(GDB_FR_REG1 )(k0)
        ld      $2,  GDB_REG64_OFS(GDB_FR_REG2 )(k0)
        ld      $3,  GDB_REG64_OFS(GDB_FR_REG3 )(k0)
        ld      $4,  GDB_REG64_OFS(GDB_FR_REG4 )(k0)
        ld      $5,  GDB_REG64_OFS(GDB_FR_REG5 )(k0)
        ld      $6,  GDB_REG64_OFS(GDB_FR_REG6 )(k0)
        ld      $7,  GDB_REG64_OFS(GDB_FR_REG7 )(k0)
        ld      $8,  GDB_REG64_OFS(GDB_FR_REG8 )(k0)
        ld      $9,  GDB_REG64_OFS(GDB_FR_REG9 )(k0)
        ld      $10, GDB_REG64_OFS(GDB_FR_REG10)(k0)
        ld      $11, GDB_REG64_OFS(GDB_FR_REG11)(k0)
        ld      $12, GDB_REG64_OFS(GDB_FR_REG12)(k0)
        ld      $13, GDB_REG64_OFS(GDB_FR_REG13)(k0)
        ld      $14, GDB_REG64_OFS(GDB_FR_REG14)(k0)
        ld      $15, GDB_REG64_OFS(GDB_FR_REG15)(k0)
        ld      $16, GDB_REG64_OFS(GDB_FR_REG16)(k0)
        ld      $17, GDB_REG64_OFS(GDB_FR_REG17)(k0)
        ld      $18, GDB_REG64_OFS(GDB_FR_REG18)(k0)
        ld      $19, GDB_REG64_OFS(GDB_FR_REG19)(k0)
        ld      $20, GDB_REG64_OFS(GDB_FR_REG20)(k0)
        ld      $21, GDB_REG64_OFS(GDB_FR_REG21)(k0)
        ld      $22, GDB_REG64_OFS(GDB_FR_REG22)(k0)
        ld      $23, GDB_REG64_OFS(GDB_FR_REG23)(k0)
        ld      $24, GDB_REG64_OFS(GDB_FR_REG24)(k0)
        ld      $25, GDB_REG64_OFS(GDB_FR_REG25)(k0)
	/* Not k0, k1 = $26, $27 */
        ld      $28, GDB_REG64_OFS(GDB_FR_REG28)(k0)
        ld      $29, GDB_REG64_OFS(GDB_FR_REG29)(k0)
        ld      $30, GDB_REG64_OFS(GDB_FR_REG30)(k0)
        ld      $31, GDB_REG64_OFS(GDB_FR_REG31)(k0)

	jr	k1
        ld      $26, GDB_REG64_OFS(GDB_FR_REG26)(k0) /* "restore" k0 */

SET_MIPS0()

END( EXCEP_restore_gpr_but_k1 )

	
/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

	
SLEAF(store_cp1_gpr_cp0_control)

	la	t1, sys_fpu
	lw	t1, 0(t1)
	beq	t1, zero, store_control
	nop

	MFC0(   t1, C0_Status)
	sll	t0, t1, 31-S_StatusCU1
	bgez	t0, store_control
	sll	t0, t1, 31-S_StatusFR
	bgez	t0, store_fpu_gpr_32bit
	nop

	/* Store 64 bit FPU registers */	
	SDC1(   R_fp0,   R_k1, GDB_REG64_OFS(GDB_FR_FPR0 ) )
	SDC1(   R_fp1,   R_k1, GDB_REG64_OFS(GDB_FR_FPR1 ) )
	SDC1(   R_fp2,   R_k1, GDB_REG64_OFS(GDB_FR_FPR2 ) )
	SDC1(   R_fp3,   R_k1, GDB_REG64_OFS(GDB_FR_FPR3 ) )
	SDC1(   R_fp4,   R_k1, GDB_REG64_OFS(GDB_FR_FPR4 ) )
	SDC1(   R_fp5,   R_k1, GDB_REG64_OFS(GDB_FR_FPR5 ) )
	SDC1(   R_fp6,   R_k1, GDB_REG64_OFS(GDB_FR_FPR6 ) )
	SDC1(   R_fp7,   R_k1, GDB_REG64_OFS(GDB_FR_FPR7 ) )
	SDC1(   R_fp8,   R_k1, GDB_REG64_OFS(GDB_FR_FPR8 ) )
	SDC1(   R_fp9,   R_k1, GDB_REG64_OFS(GDB_FR_FPR9 ) )
	SDC1(   R_fp10,  R_k1, GDB_REG64_OFS(GDB_FR_FPR10) )
	SDC1(   R_fp11,  R_k1, GDB_REG64_OFS(GDB_FR_FPR11) )
	SDC1(   R_fp12,  R_k1, GDB_REG64_OFS(GDB_FR_FPR12) )
	SDC1(   R_fp13,  R_k1, GDB_REG64_OFS(GDB_FR_FPR13) )
	SDC1(   R_fp14,  R_k1, GDB_REG64_OFS(GDB_FR_FPR14) )
	SDC1(   R_fp15,  R_k1, GDB_REG64_OFS(GDB_FR_FPR15) )
	SDC1(   R_fp16,  R_k1, GDB_REG64_OFS(GDB_FR_FPR16) )
	SDC1(   R_fp17,  R_k1, GDB_REG64_OFS(GDB_FR_FPR17) )
	SDC1(   R_fp18,  R_k1, GDB_REG64_OFS(GDB_FR_FPR18) )
	SDC1(   R_fp19,  R_k1, GDB_REG64_OFS(GDB_FR_FPR19) )
	SDC1(   R_fp20,  R_k1, GDB_REG64_OFS(GDB_FR_FPR20) )
	SDC1(   R_fp21,  R_k1, GDB_REG64_OFS(GDB_FR_FPR21) )
	SDC1(   R_fp22,  R_k1, GDB_REG64_OFS(GDB_FR_FPR22) )
	SDC1(   R_fp23,  R_k1, GDB_REG64_OFS(GDB_FR_FPR23) )
	SDC1(   R_fp24,  R_k1, GDB_REG64_OFS(GDB_FR_FPR24) )
	SDC1(   R_fp25,  R_k1, GDB_REG64_OFS(GDB_FR_FPR25) )
	SDC1(   R_fp26,  R_k1, GDB_REG64_OFS(GDB_FR_FPR26) )
	SDC1(   R_fp27,  R_k1, GDB_REG64_OFS(GDB_FR_FPR27) )
	SDC1(   R_fp28,  R_k1, GDB_REG64_OFS(GDB_FR_FPR28) )
	SDC1(   R_fp29,  R_k1, GDB_REG64_OFS(GDB_FR_FPR29) )
	SDC1(   R_fp30,  R_k1, GDB_REG64_OFS(GDB_FR_FPR30) )
	SDC1(   R_fp31,  R_k1, GDB_REG64_OFS(GDB_FR_FPR31) )

	b	store_control
	nop

store_fpu_gpr_32bit :	
		
	/* Store 32 bit FPU registers */
	swc1    $f0,   GDB_REG32_OFS(GDB_FR_FPR0 )(k1)
	swc1    $f1,   GDB_REG32_OFS(GDB_FR_FPR1 )(k1)
	swc1    $f2,   GDB_REG32_OFS(GDB_FR_FPR2 )(k1)	
	swc1    $f3,   GDB_REG32_OFS(GDB_FR_FPR3 )(k1)			
	swc1    $f4,   GDB_REG32_OFS(GDB_FR_FPR4 )(k1)
	swc1    $f5,   GDB_REG32_OFS(GDB_FR_FPR5 )(k1)
	swc1    $f6,   GDB_REG32_OFS(GDB_FR_FPR6 )(k1)	
	swc1    $f7,   GDB_REG32_OFS(GDB_FR_FPR7 )(k1)			
	swc1    $f8,   GDB_REG32_OFS(GDB_FR_FPR8 )(k1)
	swc1    $f9,   GDB_REG32_OFS(GDB_FR_FPR9 )(k1)
	swc1    $f10,  GDB_REG32_OFS(GDB_FR_FPR10)(k1)	
	swc1    $f11,  GDB_REG32_OFS(GDB_FR_FPR11)(k1)			
	swc1    $f12,  GDB_REG32_OFS(GDB_FR_FPR12)(k1)
	swc1    $f13,  GDB_REG32_OFS(GDB_FR_FPR13)(k1)
	swc1    $f14,  GDB_REG32_OFS(GDB_FR_FPR14)(k1)	
	swc1    $f15,  GDB_REG32_OFS(GDB_FR_FPR15)(k1)			
	swc1    $f16,  GDB_REG32_OFS(GDB_FR_FPR16)(k1)
	swc1    $f17,  GDB_REG32_OFS(GDB_FR_FPR17)(k1)
	swc1    $f18,  GDB_REG32_OFS(GDB_FR_FPR18)(k1)	
	swc1    $f19,  GDB_REG32_OFS(GDB_FR_FPR19)(k1)			
	swc1    $f20,  GDB_REG32_OFS(GDB_FR_FPR20)(k1)
	swc1    $f21,  GDB_REG32_OFS(GDB_FR_FPR21)(k1)
	swc1    $f22,  GDB_REG32_OFS(GDB_FR_FPR22)(k1)	
	swc1    $f23,  GDB_REG32_OFS(GDB_FR_FPR23)(k1)			
	swc1    $f24,  GDB_REG32_OFS(GDB_FR_FPR24)(k1)
	swc1    $f25,  GDB_REG32_OFS(GDB_FR_FPR25)(k1)
	swc1    $f26,  GDB_REG32_OFS(GDB_FR_FPR26)(k1)	
	swc1    $f27,  GDB_REG32_OFS(GDB_FR_FPR27)(k1)			
	swc1    $f28,  GDB_REG32_OFS(GDB_FR_FPR28)(k1)
	swc1    $f29,  GDB_REG32_OFS(GDB_FR_FPR29)(k1)
	swc1    $f30,  GDB_REG32_OFS(GDB_FR_FPR30)(k1)	
	swc1    $f31,  GDB_REG32_OFS(GDB_FR_FPR31)(k1)			

store_control:
	
	/* Store CP0/CP1 control registers */
	b	sys_store_control_regs
	move	a0, k1

END(store_cp1_gpr_cp0_control)

	
SLEAF( EXCEP_store_gpr_but_k0k1_ss )

        /* Just like EXCEP_store_gpr_but_k0k1ra() with the following exception :
	 * 
	 * If this is a MIPS32/MIPS64 Release 2 CPU including
	 * shadowsets, we store the GPR values of the shadowset
	 * that was in use when the exception was taken. In this case
	 * we also write k0, k1, ra.
	 */

	la	k1,  exc_context
		
	/* In case of MIPS32/MIPS64 Release 2 and
	 * BEV=0 and ERL=0, we read GPR from previous shadowset.
	 */
        la	k0, excep_cp0_regs_mask
        lw	k0, 0(k0)
        sll	k0, 31-6	/* Release 2 */
	bgez	k0, 1f
	nop
	MFC0(   k0, C0_Status )
	sll     k0, 31 - S_StatusBEV
	bltz	k0, 1f
	sll	k0, S_StatusBEV - S_StatusERL
	bgez	k0, 2f
	nop
1:
	la	k0,  sys_64bit
	lb	k0,  0(k0)
	beq	k0,  zero, 32f
	sw	$31, GDB_REG32_OFS(GDB_FR_REG31)(k1)
SET_MIPS3()
        sd	$31, GDB_REG64_OFS(GDB_FR_REG31)(k1)
SET_MIPS0()
32:
	jal	EXCEP_store_gpr_but_k0k1ra
	nop
	j	exc_handler_after_gpr
	nop	
2:
	/* Release 2 (and BEV=0 and ERL=0) */

        /* 32 bit or 64 bit */
	la	k0, sys_64bit
	lb	k0, 0(k0)
	bne	k0, zero, copy_pss_64bit
	nop

#define MOVEGPR( number )				\
	RDPGPR( R_k0, number );				\
	sw	k0,   GDB_REG32_OFS(GDB_GPR_OFS(number))(k1)

	MOVEGPR( 1  )
	MOVEGPR( 2  )
	MOVEGPR( 3  )
	MOVEGPR( 4  )
	MOVEGPR( 5  )
	MOVEGPR( 6  )
	MOVEGPR( 7  )
	MOVEGPR( 8  )
	MOVEGPR( 9  )
	MOVEGPR( 10 )
	MOVEGPR( 11 )
	MOVEGPR( 12 )
	MOVEGPR( 13 )
	MOVEGPR( 14 )
	MOVEGPR( 15 )
	MOVEGPR( 16 )
	MOVEGPR( 17 )
	MOVEGPR( 18 )
	MOVEGPR( 19 )
	MOVEGPR( 20 )
	MOVEGPR( 21 )
	MOVEGPR( 22 )
	MOVEGPR( 23 )
	MOVEGPR( 24 )
	MOVEGPR( 25 )
	MOVEGPR( 26 )
	MOVEGPR( 27 )
	MOVEGPR( 28 )
	MOVEGPR( 29 )
	MOVEGPR( 30 )
	MOVEGPR( 31 )

	j	exc_handler_after_gpr
	nop	

#undef MOVEGPR

copy_pss_64bit:

SET_MIPS3()
	
#define MOVEGPR( number )			\
	RDPGPR( R_k0, number );			\
	sd	k0,   GDB_REG64_OFS(GDB_GPR_OFS(number))(k1)

	MOVEGPR( 1  )
	MOVEGPR( 2  )
	MOVEGPR( 3  )
	MOVEGPR( 4  )
	MOVEGPR( 5  )
	MOVEGPR( 6  )
	MOVEGPR( 7  )
	MOVEGPR( 8  )
	MOVEGPR( 9  )
	MOVEGPR( 10 )
	MOVEGPR( 11 )
	MOVEGPR( 12 )
	MOVEGPR( 13 )
	MOVEGPR( 14 )
	MOVEGPR( 15 )
	MOVEGPR( 16 )
	MOVEGPR( 17 )
	MOVEGPR( 18 )
	MOVEGPR( 19 )
	MOVEGPR( 20 )
	MOVEGPR( 21 )
	MOVEGPR( 22 )
	MOVEGPR( 23 )
	MOVEGPR( 24 )
	MOVEGPR( 25 )
	MOVEGPR( 26 )
	MOVEGPR( 27 )
	MOVEGPR( 28 )
	MOVEGPR( 29 )
	MOVEGPR( 30 )
	MOVEGPR( 31 )

SET_MIPS0()

	j	exc_handler_after_gpr
	nop	

END( EXCEP_store_gpr_but_k0k1_ss )	
	
	
SLEAF( restore_control_gpr_but_k1 )

    /* Restore the CP0 registers that are available for the CPU in use.
     * 
     * k0 points to context, k1 = return address.
     *
     * We skip the following registers, since YAMON doesn't depend
     * on them and GDB doesn't request them :
     *
     * LLAddr
     * Xcontext
     * TraceControl
     * TraceControl2
     * UserTraceData
     * TraceBPC
     * PerfCnt
     * ErrCtl
     * CacheErr
     * TagLo
     * DataLo
     * TagHi
     * DataHi
     * DESAVE
     */

    /* Get availability of regs :
     *
     * Bit 0 = 32(1) or 64(0) bit CPU
     * Bit 1 = FPU
     * Bit 2 = TLB
     * Bit 3 = Watch registers
     * Bit 4 = MIPS32/MIPS64
     * ----- Following only apply to MIPS32/MIPS64
     * Bit 5 = EJTAG (MIPS32/64 only)
     * Bit 6 = MIPS32/MIPS64 Release 2
     * Bit 7 = SRSMap
     * Bit 8 = PageGrain
     */
    la		t0, excep_cp0_regs_mask
    lw		t0, 0(t0)

    /* 32 bit or 64 bit */	
    sll		t1, t0, 31-0
    bgez	t1, restore_control_64bit
    nop

    /* Registers available for all MIPS CPUs */
    lw		t1, GDB_REG32_OFS(GDB_FR_LO)(k0)
    mtlo	t1

    lw		t1, GDB_REG32_OFS(GDB_FR_HI)(k0)		
    mthi	t1

    lw		t1, GDB_REG32_OFS(GDB_FR_CAUSE)(k0)
    MTC0(	t1, C0_Cause )

    lw		t1, GDB_REG32_OFS(GDB_FR_EPC)(k0)
    MTC0(	t1, C0_EPC )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_ERROREPC)(k0)
    MTC0(	t1, C0_ErrorEPC )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_Config, R_C0_SelConfig )

    /* Check for TLB */
    sll		t1, t0, 31-2
    bgez	t1, 1f
    nop

    /* TLB */
    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_INDEX)(k0)
    MTC0(       t1, C0_Index )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYLO0)(k0)
    MTC0(	t1, C0_EntryLo0 )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYLO1)(k0)
    MTC0(	t1, C0_EntryLo1 )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_CONTEXT)(k0)
    MTC0(	t1, C0_Context )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_PAGEMASK)(k0)
    MTC0(	t1, C0_PageMask )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_WIRED)(k0)
    MTC0(	t1, C0_Wired )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_ENTRYHI)(k0)
    MTC0(	t1, C0_EntryHi )
1:
    /* Check for Watch registers */
    sll		t1, t0, 31-3
    bgez	t1, 1f
    nop

    /* Watch registers */
    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_WATCHLO)(k0)
    MTC0(       t1, C0_WatchLo )

    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_WATCHHI)(k0)
    MTC0(       t1, C0_WatchHi )
1:
    /* Check for MIPS32/MIPS64 */
    sll		t1, t0, 31-4
    bgez	t1, 1f
    nop

    /* MIPS32/MIPS64 */
    
    /* Config1 (required for MIPS32/MIPS64 CPUs) */
    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG1)(k0)
    /* Config2 depends on M bit of Config1 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_CONFIG2)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_Config2, R_C0_SelConfig2 )
2:
    /* Check for EJTAG */
    sll		t1, t0, 31-5
    bgez	t1, 2f
    nop

    /* EJTAG regs */
#if 0 /* TBD : Consider DEBUG */
    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_DEBUG)(k0)
    MTC0(	t1, C0_Debug )
#endif

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_DEPC)(k0)
    MTC0(	t1, C0_DEPC )
2:
    /* Check for Release 2 */
    sll		t1, t0, 31-6
    bgez	t1, 2f
    nop

    /* Release 2 */
    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_HWRENA)(k0)
    MTC0(       t1, C0_HWREna )

    lw		t1, GDB_REG32_OFS(GDB_FR_CP0_INTCTL)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_IntCtl, R_C0_SelIntCtl )

    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_SRSCTL)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_SRSCtl, R_C0_SelSRSCtl )
	
    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_EBASE)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_EBase, R_C0_SelEBase )

    /* SRSMap */
    sll		t1, t0, 31-7
    bgez	t1, 2f
    nop

    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_SRSMAP)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_SRSMap, R_C0_SelSRSMap )
2:
    /* PageGrain */
    sll		t1, t0, 31-8
    bgez	t1, 1f
    nop

    lw	        t1, GDB_REG32_OFS(GDB_FR_CP0_PAGEGRAIN)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_PageGrain, R_C0_SelPageGrain )
1:
    /* Status register (always available) */
    lw		t1, GDB_REG32_OFS(GDB_FR_STATUS)(k0)
    MTC0(       t1, C0_Status)

    /* Check for FPU */
    sll		t2, t0, 31-1
    bgez	t2, EXCEP_restore_gpr_but_k1
    nop
    /* Check that FPU is enabled */
    sll		t2, t1, 31-S_StatusCU1
    bgez	t2, EXCEP_restore_gpr_but_k1
    nop

    /* Store FPU registers */
    cfc1        t2, $31 /* Empty pipeline */
    lw	        t2, GDB_REG32_OFS(GDB_FR_FSR)(k0)
    ctc1        t2, $31

    b		restore_cp1_gpr
    nop   

restore_control_64bit :	

SET_MIPS3()

    /* Registers available for all MIPS CPUs */
    ld		t1, GDB_REG64_OFS(GDB_FR_LO)(k0)
    mtlo	t1

    ld		t1, GDB_REG64_OFS(GDB_FR_HI)(k0)		
    mthi	t1

    ld		t1, GDB_REG64_OFS(GDB_FR_CAUSE)(k0)
    MTC0(	t1, C0_Cause )

    ld		t1, GDB_REG64_OFS(GDB_FR_EPC)(k0)
    DMTC0(	t1, C0_EPC )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_ERROREPC)(k0)
    DMTC0(	t1, C0_ErrorEPC )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_Config, R_C0_SelConfig )

    /* Check for TLB */
    sll		t1, t0, 31-2
    bgez	t1, 1f
    nop

    /* TLB */
    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_INDEX)(k0)
    MTC0(       t1, C0_Index )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYLO0)(k0)
    DMTC0(	t1, C0_EntryLo0 )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYLO1)(k0)
    DMTC0(	t1, C0_EntryLo1 )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_CONTEXT)(k0)
    DMTC0(	t1, C0_Context )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_PAGEMASK)(k0)
    MTC0(	t1, C0_PageMask )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_WIRED)(k0)
    MTC0(	t1, C0_Wired )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_ENTRYHI)(k0)
    DMTC0(	t1, C0_EntryHi )
1:
    /* Check for Watch registers */
    sll		t1, t0, 31-3
    bgez	t1, 1f
    nop

    /* Watch registers */
    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_WATCHLO)(k0)
    DMTC0(      t1, C0_WatchLo )

    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_WATCHHI)(k0)
    DMTC0(      t1, C0_WatchHi )
1:
    /* Check for MIPS32/MIPS64 */
    sll		t1, t0, 31-4
    bgez	t1, 1f
    nop

    /* MIPS32/MIPS64 */
    
    /* Config1 (required for MIPS32/MIPS64 CPUs) */
    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG1)(k0)
    /* Config2 depends on M bit of Config1 */
    sll		t1, 31-S_ConfigMore
    bgez	t1, 2f
    nop
    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_CONFIG2)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_Config2, R_C0_SelConfig2 )
2:
    /* Check for EJTAG */
    sll		t1, t0, 31-5
    bgez	t1, 2f
    nop

    /* EJTAG regs */
#if 0 /* TBD : Consider DEBUG */
    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_DEBUG)(k0)
    MTC0(	t1, C0_Debug )
#endif

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_DEPC)(k0)
    DMTC0(	t1, C0_DEPC )
2:
    /* Check for Release 2 */
    sll		t1, t0, 31-6
    bgez	t1, 2f
    nop

    /* Release 2 */
    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_HWRENA)(k0)
    MTC0(       t1, C0_HWREna )

    ld		t1, GDB_REG64_OFS(GDB_FR_CP0_INTCTL)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_IntCtl, R_C0_SelIntCtl )

    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_SRSCTL)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_SRSCtl, R_C0_SelSRSCtl )

    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_EBASE)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_EBase, R_C0_SelEBase )

    /* SRSMap */
    sll		t1, t0, 31-7
    bgez	t1, 2f
    nop

    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_SRSMAP)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_SRSMap, R_C0_SelSRSMap )
2:
    /* PageGrain */
    sll		t1, t0, 31-8
    bgez	t1, 1f
    nop

    ld	        t1, GDB_REG64_OFS(GDB_FR_CP0_PAGEGRAIN)(k0)
    MTC0_SEL_OPCODE( R_t1, R_C0_PageGrain, R_C0_SelPageGrain )
1:
    /* Status register (always available) */
    ld		t1, GDB_REG64_OFS(GDB_FR_STATUS)(k0)
    MTC0(       t1, C0_Status)

    /* Check for FPU */
    sll		t2, t0, 31-1
    bgez	t2, EXCEP_restore_gpr_but_k1
    nop
    /* Check that FPU is enabled */
    sll		t2, t1, 31-S_StatusCU1
    bgez	t2, EXCEP_restore_gpr_but_k1
    nop

    /* Store FPU registers */
    cfc1        t2, $31 /* Empty pipeline */
    ld	        t2, GDB_REG64_OFS(GDB_FR_FSR)(k0)
    ctc1        t2, $31

SET_MIPS0()

restore_cp1_gpr:

	/* t1 contains cp0_status */
	sll	t1, 31-S_StatusFR
	bgez	t1, restore_fpu_gpr_32bit
	nop

	/* Restore 64 bit FPU registers */	
	LDC1(   R_fp0,  R_k0, GDB_REG64_OFS(GDB_FR_FPR0 ))
	LDC1(   R_fp1,  R_k0, GDB_REG64_OFS(GDB_FR_FPR1 ))
	LDC1(   R_fp2,  R_k0, GDB_REG64_OFS(GDB_FR_FPR2 ))
	LDC1(   R_fp3,  R_k0, GDB_REG64_OFS(GDB_FR_FPR3 ))
	LDC1(   R_fp4,  R_k0, GDB_REG64_OFS(GDB_FR_FPR4 ))
	LDC1(   R_fp5,  R_k0, GDB_REG64_OFS(GDB_FR_FPR5 ))
	LDC1(   R_fp6,  R_k0, GDB_REG64_OFS(GDB_FR_FPR6 ))
	LDC1(   R_fp7,  R_k0, GDB_REG64_OFS(GDB_FR_FPR7 ))
	LDC1(   R_fp8,  R_k0, GDB_REG64_OFS(GDB_FR_FPR8 ))
	LDC1(   R_fp9,  R_k0, GDB_REG64_OFS(GDB_FR_FPR9 ))
	LDC1(   R_fp10, R_k0, GDB_REG64_OFS(GDB_FR_FPR10))
	LDC1(   R_fp11, R_k0, GDB_REG64_OFS(GDB_FR_FPR11))
	LDC1(   R_fp12, R_k0, GDB_REG64_OFS(GDB_FR_FPR12))
	LDC1(   R_fp13, R_k0, GDB_REG64_OFS(GDB_FR_FPR13))
	LDC1(   R_fp14, R_k0, GDB_REG64_OFS(GDB_FR_FPR14))
	LDC1(   R_fp15, R_k0, GDB_REG64_OFS(GDB_FR_FPR15))
	LDC1(   R_fp16, R_k0, GDB_REG64_OFS(GDB_FR_FPR16))
	LDC1(   R_fp17, R_k0, GDB_REG64_OFS(GDB_FR_FPR17))
	LDC1(   R_fp18, R_k0, GDB_REG64_OFS(GDB_FR_FPR18))
	LDC1(   R_fp19, R_k0, GDB_REG64_OFS(GDB_FR_FPR19))
	LDC1(   R_fp20, R_k0, GDB_REG64_OFS(GDB_FR_FPR20))
	LDC1(   R_fp21, R_k0, GDB_REG64_OFS(GDB_FR_FPR21))
	LDC1(   R_fp22, R_k0, GDB_REG64_OFS(GDB_FR_FPR22))
	LDC1(   R_fp23, R_k0, GDB_REG64_OFS(GDB_FR_FPR23))
	LDC1(   R_fp24, R_k0, GDB_REG64_OFS(GDB_FR_FPR24))
	LDC1(   R_fp25, R_k0, GDB_REG64_OFS(GDB_FR_FPR25))
	LDC1(   R_fp26, R_k0, GDB_REG64_OFS(GDB_FR_FPR26))
	LDC1(   R_fp27, R_k0, GDB_REG64_OFS(GDB_FR_FPR27))
	LDC1(   R_fp28, R_k0, GDB_REG64_OFS(GDB_FR_FPR28))
	LDC1(   R_fp29, R_k0, GDB_REG64_OFS(GDB_FR_FPR29))
	LDC1(   R_fp30, R_k0, GDB_REG64_OFS(GDB_FR_FPR30))
	LDC1(   R_fp31, R_k0, GDB_REG64_OFS(GDB_FR_FPR31))

	b	EXCEP_restore_gpr_but_k1
	nop
	
restore_fpu_gpr_32bit :	
		
	/* Restore 32 bit FPU registers */	
	lwc1    $f0,   GDB_REG32_OFS(GDB_FR_FPR0 )(k0)
	lwc1    $f1,   GDB_REG32_OFS(GDB_FR_FPR1 )(k0)
	lwc1    $f2,   GDB_REG32_OFS(GDB_FR_FPR2 )(k0)	
	lwc1    $f3,   GDB_REG32_OFS(GDB_FR_FPR3 )(k0)			
	lwc1    $f4,   GDB_REG32_OFS(GDB_FR_FPR4 )(k0)
	lwc1    $f5,   GDB_REG32_OFS(GDB_FR_FPR5 )(k0)
	lwc1    $f6,   GDB_REG32_OFS(GDB_FR_FPR6 )(k0)	
	lwc1    $f7,   GDB_REG32_OFS(GDB_FR_FPR7 )(k0)			
	lwc1    $f8,   GDB_REG32_OFS(GDB_FR_FPR8 )(k0)
	lwc1    $f9,   GDB_REG32_OFS(GDB_FR_FPR9 )(k0)
	lwc1    $f10,  GDB_REG32_OFS(GDB_FR_FPR10)(k0)	
	lwc1    $f11,  GDB_REG32_OFS(GDB_FR_FPR11)(k0)			
	lwc1    $f12,  GDB_REG32_OFS(GDB_FR_FPR12)(k0)
	lwc1    $f13,  GDB_REG32_OFS(GDB_FR_FPR13)(k0)
	lwc1    $f14,  GDB_REG32_OFS(GDB_FR_FPR14)(k0)	
	lwc1    $f15,  GDB_REG32_OFS(GDB_FR_FPR15)(k0)			
	lwc1    $f16,  GDB_REG32_OFS(GDB_FR_FPR16)(k0)
	lwc1    $f17,  GDB_REG32_OFS(GDB_FR_FPR17)(k0)
	lwc1    $f18,  GDB_REG32_OFS(GDB_FR_FPR18)(k0)	
	lwc1    $f19,  GDB_REG32_OFS(GDB_FR_FPR19)(k0)			
	lwc1    $f20,  GDB_REG32_OFS(GDB_FR_FPR20)(k0)
	lwc1    $f21,  GDB_REG32_OFS(GDB_FR_FPR21)(k0)
	lwc1    $f22,  GDB_REG32_OFS(GDB_FR_FPR22)(k0)	
	lwc1    $f23,  GDB_REG32_OFS(GDB_FR_FPR23)(k0)			
	lwc1    $f24,  GDB_REG32_OFS(GDB_FR_FPR24)(k0)
	lwc1    $f25,  GDB_REG32_OFS(GDB_FR_FPR25)(k0)
	lwc1    $f26,  GDB_REG32_OFS(GDB_FR_FPR26)(k0)	
	lwc1    $f27,  GDB_REG32_OFS(GDB_FR_FPR27)(k0)			
	lwc1    $f28,  GDB_REG32_OFS(GDB_FR_FPR28)(k0)
	lwc1    $f29,  GDB_REG32_OFS(GDB_FR_FPR29)(k0)
	lwc1    $f30,  GDB_REG32_OFS(GDB_FR_FPR30)(k0)	
	lwc1    $f31,  GDB_REG32_OFS(GDB_FR_FPR31)(k0)

	b	EXCEP_restore_gpr_but_k1
	nop
	
END( restore_control_gpr_but_k1 )
