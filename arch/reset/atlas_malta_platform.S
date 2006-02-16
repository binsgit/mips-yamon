
/************************************************************************
 *
 *  atlas_malta_platform.S
 *
 *  Atlas and Malta specific initialisation
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
#include <pci_api.h>
#include <i2c.h>
#include <init.h>
#include <product.h>
#include <atlas.h>
#include <malta.h>
#include <saa9730.h>  /* Atlas specific */

/************************************************************************
 *  Definitions
 ************************************************************************/

/******** Definitions for I2C controller on Malta ********/

#define SCL_TX		MALTA_I2COE_I2CSCL_BIT
#define SCL_RX		0
#define SDA_TX		MALTA_I2COE_I2CSDA_BIT
#define SDA_RX		0

#define SCL_HI		MALTA_I2COUT_I2CSCL_BIT
#define SCL_LO		0
#define SDA_HI		MALTA_I2COUT_I2CSDA_BIT
#define SDA_LO		0

/*  Loop counter for PAUSE loop :
 *
 *  Each iteration of loop consists of two instructions, i.e. two
 *  processor clock cycles (assuming cached operation).
 *
 *  A pause loop should delay the processor half an I2C clock cycle.
 *
 *  We assume a max. processor frequency of 1000MHz.
 */
#define PAUSE_COUNT (1000000 / MAX_I2C_FREQ_KHZ  / 2 / 2)

#define PAUSE				          \
	    li    t7, PAUSE_COUNT;		  \
10:						  \
	    bne   t7, zero, 10b;		  \
	    addiu t7, -1

#define I2C_DIRECTION( scl, sda )	          \
	    li t7, KSEG1(MALTA_I2COE);		  \
	    li t6, (scl) | (sda);		  \
	    sw t6, 0(t7);			  \
	    PAUSE

#define I2C_TX( scl, sda )		          \
	    li t7, KSEG1(MALTA_I2COUT);		  \
	    li t6, (scl) | (sda);		  \
	    sw t6, 0(t7);			  \
	    PAUSE

#define I2C_RX( reg )				  \
	    li   t7, KSEG1(MALTA_I2CINP);	  \
	    lw   reg, 0(t7);			  \
	    andi reg, MALTA_I2CINP_I2CSDA_MSK;	  \
	    srl  reg, MALTA_I2CINP_I2CSDA_SHF;	  \
	    PAUSE

#define I2C_FPGA_ENABLE		                  \
	    li t7, KSEG1(MALTA_I2COE);		  \
	    li t6, (SCL_RX) | (SDA_RX);		  \
	    sw t6, 0(t7);			  \
	    li t7, KSEG1(MALTA_I2COUT);		  \
	    li t6, (SCL_HI) | (SDA_HI);		  \
	    sw t6, 0(t7);			  \
	    li t7, KSEG1(MALTA_I2CSEL);		  \
	    li t6, MALTA_I2CSEL_FPGA_BIT;	  \
	    sw t6, 0(t7);			  \
	    PAUSE

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          sys_init_atlas_malta
 *  Description :
 *  -------------
 *
 *  Determine SDRAM parameters and configure North Bridge.
 *
 *  SDRAM configuration is conservative since we do not know the bus
 *  frequency yet. However, after this function has been called,
 *  a memory test can be performed. Later, when we know the bus
 *  frequency, the SDRAM configuration may be optimised.
 *
 *  Parameters :
 *  ------------
 *
 *  k0 = Platform ID
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 = no error)
 *  v1 = RAM size in bytes
 *
 ************************************************************************/
LEAF(sys_init_atlas_malta)

	.set noreorder

#define PCIMEM_DEFAULT	    s0
#define RA		    gp
#define MEMSIZE		    sp

	move		    RA, ra

	/* Get default base address for PCI memory access */
	jal	sys_core_get_pcimem_base
	nop
	move    PCIMEM_DEFAULT, v0

	li	t0, PRODUCT_ATLASA_ID
	bne	t0, k0, setup_sdram
	nop

	/*************************************************************
	** On Atlas, enable read of SPD eeprom via SAA9730 PCI device
	*/
	/* Setup SAA9730 memory mapping */
	li	a0, ATLAS_DEVNUM_SAA9730
	li	a1, PCI_BAR(1)
	move	a2, PCIMEM_DEFAULT
	li	a3, KSEG1(ATLAS_CORECTRL_BASE)
	jal	sys_core_config_write
	nop
	bne	v0, zero, error_init
	nop

	/*  Enable SAA9730 memory space */
	li	a2, PCI_SC_CMD_MS_BIT | PCI_SC_CMD_SERR_BIT
	li	a0, ATLAS_DEVNUM_SAA9730
	li	a1, PCI_SC
	li	a3, KSEG1(ATLAS_CORECTRL_BASE)
	jal	sys_core_config_write
	nop
	bne	v0, zero, error_init
	nop

        /* Init I2C controller in SAA9730 (run as slow as possible) */
	li	t0, SAA9730_I2CSC_I2CCC_6400 << SAA9730_I2CSC_I2CCC_SHF
	/* Base address of SAA9730 */
	KSEG1A( PCIMEM_DEFAULT )
	sw	t0, SAA9730_I2CSC_OFS( PCIMEM_DEFAULT )

setup_sdram:

	/* Configure North Bridge SDRAM parameters */
	li      a0, ATLAS_CPUFREQ_LOWEST_MHZ
	li	a1, KSEG1(ATLAS_CORECTRL_BASE)
	li	a2, ATLAS_SYSTEMRAM_SIZE
	jal	sys_core_configure_sdram
	nop
	bne	v0, zero, error_init
	nop
	move	MEMSIZE, v1

	/* Configure North Bridge memory map */
	li      a0, ATLAS_PCI_MEM_BASE
	li	a1, ATLAS_PCI_MEM_SIZE
	li	a2, ATLAS_PCI_IO_BASE
	li	a3, ATLAS_PCI_IO_SIZE
	li	t0, ATLAS_SYSTEMFLASH_BASE
	li	t1, ATLAS_SYSTEMFLASH_SIZE + ATLAS_MONITORFLASH_SIZE
	li	t2, KSEG1(ATLAS_CORECTRL_BASE)
	jal	sys_core_setup_decode
	nop
	bne	v0, zero, error_init
	nop

	/*  If Malta, remap PCI I/O.
	 *  We need to do this since the PIIX4 device on Malta requires
	 *  IO mapping to start at address 0.
	 */
	li	t0, PRODUCT_MALTA_ID
	bne	t0, k0, 1f
	nop
	li	a0, KSEG1(ATLAS_CORECTRL_BASE)
	jal     sys_core_remap_pci_io
	nop
1:

	/* Setup return parameters */
        move    v0, zero
	move	v1, MEMSIZE

error_init:

        jr      RA
	nop

#undef PCIMEM_DEFAULT
#undef RA
#undef MEMSIZE

END(sys_init_atlas_malta)


/************************************************************************
 *
 *                          sys_spd_read
 *  Description :
 *  -------------
 *
 *  Function for reading data byte from SPD (or other serial EEPROM device).
 *
 *  Parameters :
 *  ------------
 *
 *  a0 = sub-address
 *  k0 = Platform ID
 *
 *  Return values :
 *  ---------------
 *
 *  v0 = error code (0 if OK)
 *  v1 = byte read
 *
 ************************************************************************/
LEAF(sys_spd_read)

#define SPD_ADDR	t0
#define SUB_ADDR	t1
#define RA		t2

	move	RA,   ra

	DISP_STR( msg_spd )

	/* Setup SPD address and sub-address */
	li	SPD_ADDR,  (ATLAS_EEPROM_IICADR_SPD000 << 1)
	move	SUB_ADDR,  a0

	/* Branch on platform */
	li	t3, PRODUCT_ATLASA_ID
	beq	t3, k0, eeprom_atlas
	nop

	/* Malta */

eeprom_malta:

	/*  Set I2C registers in FPGA to a well-defined state (not
	 *  really necessary).
	 *  Macros use t7..t6.
	 */
	I2C_FPGA_ENABLE

	jal     iic_malta_start
	nop

	ori	a0, SPD_ADDR, I2C_WRITE_BIT    /* SPD address + write bit */
	jal	iic_malta_write
	nop
	li	a0, I2C_ACK_BIT
	bne	v0, a0, error_eeprom
	li	v0, ERROR_SPD

	move	a0, SUB_ADDR		       /* Subaddress		  */
	jal	iic_malta_write
	nop
	li	a0, I2C_ACK_BIT
	bne	v0, a0, error_eeprom
	li	v0, ERROR_SPD

	jal	iic_malta_start
	nop

	ori     a0, SPD_ADDR, I2C_READ_BIT     /* SPD address + read bit */
	jal     iic_malta_write
	nop
	li	a0, I2C_ACK_BIT
	bne	v0, a0, error_eeprom
	li	v0, ERROR_SPD

	/* Read data */
	jal	iic_malta_read		       /* Returns data in v0     */
	nop
	move	v1, v0

	jal	iic_malta_stop
	nop

	b	eeprom_done
	nop

iic_malta_start :

	I2C_DIRECTION( SCL_TX, SDA_TX )

	I2C_TX( SCL_HI, SDA_HI )
	I2C_TX( SCL_HI, SDA_LO )	/* Hi to Lo transition -> START */
	I2C_TX( SCL_LO, SDA_LO )

	jr	ra
	nop

iic_malta_write :

	/*  a0 =  byte to write
	 *  v0 := ACK/NACK bit
	 */

	I2C_DIRECTION( SCL_TX, SDA_TX )

	li	t3, 7		      /* 8 bit loop counter */
1:
	/* Determine data[t3] (MSb to be transmitted first) */
	srlv	t4, a0, t3
	andi	t4, 1
	beq	t4, zero, 2f
	nop

	I2C_TX( SCL_LO, SDA_HI )	/* Set SDA	    */
	I2C_TX( SCL_HI, SDA_HI )	/* Clock Hi	    */
	I2C_TX( SCL_LO, SDA_HI )	/* Clock Lo	    */
	b	3f
	nop
2:
	I2C_TX( SCL_LO, SDA_LO )	/* Clear SDA	    */
	I2C_TX( SCL_HI, SDA_LO )	/* Clock Hi	    */
	I2C_TX( SCL_LO, SDA_LO )	/* Clock lo	    */

3:
	bne	t3, zero, 1b
	addiu	t3, -1

	/*  Get acknowledge.
	 *
	 *  Note : Between the time that clock was driven low (see above)
	 *         and the direction of SDA is changed (see below),
	 *         there may be a conflict where both the I2C master and
	 *         the I2C slave are driving SDA. Hardware allows for
	 *         this (open drain implementation).
	 */

	I2C_DIRECTION( SCL_TX, SDA_RX )

	I2C_TX( SCL_HI, SDA_HI )	/* Clock Hi, dummy SDA */
	I2C_RX( v0 )			/* Read SDA	       */
	I2C_TX( SCL_LO, SDA_HI )	/* Clock Lo, dummy SDA */

	jr      ra
	nop

iic_malta_read :

	/* v0 := data read */

	I2C_DIRECTION( SCL_TX, SDA_RX )

	move	v0, zero	        /* Init result register */
	li	t3, 8			/* 8 bit loop counter   */
1:
	sll	v0, 1

	I2C_TX( SCL_HI, SDA_HI )	/* Clock Hi, dummy SDA	*/
	I2C_RX( t4 )			/* Read SDA		*/
	I2C_TX( SCL_LO, SDA_HI )	/* Clock Lo, dummy SDA	*/

	addiu	t3, -1
	bne	t3, zero, 1b
	or	v0, t4

	/* Generate NACK */

	I2C_TX( SCL_LO, I2C_NACK_BIT )
	I2C_DIRECTION( SCL_TX, SDA_TX )
	I2C_TX( SCL_HI, I2C_NACK_BIT )
	I2C_TX( SCL_LO, I2C_NACK_BIT )

	jr	ra
	nop

iic_malta_stop :

	I2C_DIRECTION( SCL_TX, SDA_TX )

	I2C_TX( SCL_LO, SDA_LO )
	I2C_TX( SCL_HI, SDA_LO )	/* Lo to Hi transition -> STOP */
	I2C_TX( SCL_HI, SDA_HI )
	I2C_TX( SCL_LO, SDA_HI )

	jr	ra
	nop

	/* ATLAS */

eeprom_atlas:

	/* Write EEPROM address, sub-address, EEPROM address */

	/* Byte1 : Sub-address */
	sll	a0, SUB_ADDR, SAA9730_I2CTFR_BYTE1_SHF
	ori	a0, SAA9730_I2CTFR_ATTR_CONT << SAA9730_I2CTFR_BYTE1ATTR_SHF

	/* Byte2 : EEPROM address */
	ori	t3, SPD_ADDR, I2C_WRITE_BIT		/* Last bit indicates write */
	sll	t3, SAA9730_I2CTFR_BYTE2_SHF
	ori	t3, t3, SAA9730_I2CTFR_ATTR_START << SAA9730_I2CTFR_BYTE2ATTR_SHF
	or	a0, t3

	/* Byte0 : EEPROM address again */
	ori	t3, SPD_ADDR, I2C_READ_BIT
	sll	t3, SAA9730_I2CTFR_BYTE0_SHF
	ori	t3, t3, SAA9730_I2CTFR_ATTR_START << SAA9730_I2CTFR_BYTE0ATTR_SHF
	or	a0, t3

	/*  Get default base address for PCI memory access, which is the
	 *  also the address used for the SAA9730 device.
	 */
	jal	sys_core_get_pcimem_base
	nop
	move	a1, v0
	KSEG1A( a1 )

	/* Perform operation */
	jal	i2c_atlas
	nop

	/* Read byte */

	/* Byte2 : data will be read hereto */
	li	a0, SAA9730_I2CTFR_ATTR_STOP << SAA9730_I2CTFR_BYTE2ATTR_SHF

	/* Perform operation */
	jal	i2c_atlas
	nop

	/*  Byte2 now contains the data read. Read it and shift it,
	 *  placing the result in v1.
	 */

	lw	v1, SAA9730_I2CTFR_OFS(a1)
	and	v1, SAA9730_I2CTFR_BYTE2_MSK
	srl	v1, SAA9730_I2CTFR_BYTE2_SHF

	b	eeprom_done
	nop

i2c_atlas :

	/* a0 = data, a1 = base address of SAA9730 */

	/* Request write operation */
	sw	a0, SAA9730_I2CTFR_OFS(a1)

	/* Poll Busy flag */
	li	a0, SAA9730_I2CTFR_BUSY_MSK
1:
	lw	t3, SAA9730_I2CTFR_OFS(a1)
	and	t3, a0
	bne	t3, zero, 1b
	nop

	/* Check error flag */
	li      a0, SAA9730_I2CTFR_ERR_MSK
	lw	t3, SAA9730_I2CTFR_OFS(a1)
	and	t3, a0
	bne	t3, zero, error_eeprom
	li	v0, ERROR_SPD

	jr	ra
	nop

	/* Done */

eeprom_done:

	move	v0, zero

error_eeprom:

	jr	RA
	nop

END(sys_spd_read)


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/* Messages */
	.text

MSG( msg_spd,   "SPD"   )

