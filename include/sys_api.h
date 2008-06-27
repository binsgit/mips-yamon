/************************************************************************
 *
 *  sys_api.h
 *
 *  Header file for system functions
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

#ifndef SYS_API_H
#define SYS_API_H


#ifndef _ASSEMBLER_

/************************************************************************
 *  Include files
 ************************************************************************/

#include <serial_api.h>
#include <gdb_stub.h>

/************************************************************************
 *  Definitions
*************************************************************************/

#define MIPS_NAME	        "MIPS Technologies, Inc."
#define YAMON_REV_STRING	_REVMAJ_"."_REVMIN_
#define DEFAULT_PROMPT	        "YAMON\0\0\0"

/* Display macros (ASCII display) */
#define DISP( val )		sys_disp( val )
#define DISP_CH( pos, ch )	sys_disp_ch( pos, ch )
#define DISP_STR( s )		sys_disp_str( s )

#define PUTCHAR(port, ch)	sys_putchar(port, ch)
#define GETCHAR(port, ch)	sys_getchar(port, ch)
#define GETCHAR_CTRLC(port)	sys_getchar_ctrlc(port)
#define PUTS(port, s)		sys_puts(port, s)

#ifdef _DEBUG_
#define DEBUG(s)		PUTS(DEBUG_PORT, s)
#else
#define DEBUG(s)
#endif

/* Default baudrates for serial ports */
#define DEFAULT_BAUDRATE     SERIAL_BAUDRATE_38400_BPS
#define DEFAULT_DATABITS     SERIAL_DATABITS_8
#define DEFAULT_PARITY	     SERIAL_PARITY_NONE
#define DEFAULT_STOPBITS     SERIAL_STOPBITS_10
#define DEFAULT_FLOWCTRL     SERIAL_FLOWCTRL_HARDWARE

/* Default env. variable for serial port settings */
#define DEFAULT_SERIAL_ENV   "38400,n,8,1,hw"


/* Board Serial number type */
#define SYS_SN_BCD_SIZE	        5    /* BCD coded (eeprom format)       */
#define SYS_SN_ASCII_SIZE       11   /* ascii coded (syscon format ), 
				        including '\0' termination	*/
typedef UINT8    t_sn_bcd[SYS_SN_BCD_SIZE];
typedef char     t_sn_ascii[SYS_SN_ASCII_SIZE];


/* MAC address type (Ethernet address). */
#define SYS_MAC_ADDR_SIZE  	6
typedef UINT8  t_mac_addr[SYS_MAC_ADDR_SIZE];


/* Array with count type */
typedef struct
{
    UINT32 count;
    UINT32 *array;
}
t_sys_array;

/* Structure for holding data for cpu configuration */
typedef struct
{
    UINT32 i_bpw;
    UINT32 i_assoc;
    UINT32 d_bpw;
    UINT32 d_assoc;
    bool   mmu_tlb;  /* TRUE -> TLB, FALSE -> Fixed */
}
t_sys_cpu_decoded;


/* Function pointer passed to sys_func_noram() */
typedef  UINT32 (*t_sys_func_noram)(UINT32, UINT32, UINT32);

/************************************************************************
 *  Public variables
 ************************************************************************/

extern UINT32      sys_cpufreq_hz;
extern UINT32      sys_busfreq_hz;

extern UINT32	   sys_platform;
extern UINT32	   sys_processor;
extern UINT32	   sys_corecard;
extern UINT32      sys_sysconid;
extern UINT32	   sys_manid;
extern UINT32	   sys_manpd;

extern UINT32	   sys_nb_base;
extern UINT32	   sys_ramsize;
extern UINT32	   sys_freemem;

extern bool	   sys_64bit;
extern bool	   sys_mips32_64;
extern bool	   sys_mips16e;
extern bool	   sys_smallpage_support;
extern bool	   sys_ejtag;
extern bool	   sys_eicmode;
extern bool	   sys_mt;
extern bool	   sys_dsp;
extern UINT8	   sys_arch_rev; /* Reflects the C0_Config.AR field */
extern UINT32	   sys_fpu;	/* if set, contains cp1_fir value */

extern bool	   sys_cpu_cache_coherency;

extern bool	   sys_l2cache;
extern bool	   sys_l2cache_enabled;
extern bool	   sys_l2cache_exclusive;
extern UINT32	   sys_l2cache_lines;
extern UINT32	   sys_l2cache_linesize;
extern UINT32	   sys_l2cache_assoc;
extern UINT32	   sys_icache_linesize;
extern UINT32	   sys_icache_lines;
extern UINT32	   sys_icache_assoc;
extern UINT32	   sys_dcache_linesize;
extern UINT32	   sys_dcache_lines;
extern UINT32	   sys_dcache_assoc;
extern UINT32	   sys_dcache_antialias;

extern char        *sys_default_prompt;
extern char        sys_default_display[];

/************************************************************************
 *  Public functions
 ************************************************************************/

UINT32 sys_determine_icache_linesize_flash( UINT32 procid );
UINT32 sys_determine_icache_lines_flash(    UINT32 procid );
UINT32 sys_determine_icache_assoc_flash(    UINT32 procid );	
UINT32 sys_determine_dcache_linesize_flash( UINT32 procid );
UINT32 sys_determine_dcache_lines_flash(    UINT32 procid );
UINT32 sys_determine_dcache_assoc_flash(    UINT32 procid );
UINT32 sys_determine_dcache_antialias_flash(UINT32 procid );
UINT32 sys_determine_l2cache_linesize_flash(UINT32 procid );
UINT32 sys_determine_l2cache_lines_flash(   UINT32 procid );
UINT32 sys_determine_l2cache_assoc_flash(   UINT32 procid );

void 
sys_putchar( 
    UINT32 port,
    char   ch );

void
sys_puts( 
    UINT32 port, 
    char *s );

bool
sys_getchar(
    UINT32 port,
    char *ch );

bool
sys_getchar_ctrlc(
    UINT32 port );



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
void
sys_sync( void );


/************************************************************************
 *
 *                          sys_dma_enable
 *  Description :
 *  -------------
 *
 *  Platform specific handling of DMA (enable/disable)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_dma_enable(
    bool enable ); /* TRUE -> Enable DMA devices, FALSE -> Disable	*/


/************************************************************************
 *
 *                          sys_access_func
 *  Description :
 *  -------------
 *
 *  Call requested function without linker restrictions on address.
 *
 *  The 3 specified parameters are passed in CPU registers a0, a1, a2.
 *
 *  Return values :
 *  ---------------
 *
 *  UINT32 returned by requested function
 *
 ************************************************************************/
UINT32
sys_access_func( 
    UINT32 (*func)(UINT32, UINT32, UINT32),
    UINT32 parm1, 
    UINT32 parm2, 
    UINT32 parm3 );


/************************************************************************
 *
 *                          sys_func_noram
 *  Description :
 *  -------------
 *
 *  Access function while executing from flash.
 *  All RAM access will be disabled while accessing function.
 *
 *  Return values :
 *  ---------------
 *
 *  UINT32 returned by requested function
 *
 ************************************************************************/
UINT32
sys_func_noram(
    t_sys_func_noram func,
    UINT32	     parm1, 
    UINT32	     parm2, 
    UINT32	     parm3 );


/************************************************************************
 *
 *                          sys_legal_align
 *  Description :
 *  -------------
 *
 *  Determine if alignment of address is legal
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE -> Legal alignment, FALSE -> Illegal alignment
 *
 ************************************************************************/
bool
sys_legal_align(
    UINT32 address,
    UINT32 align );


/************************************************************************
 *
 *                          sys_disp
 *  Description :
 *  -------------
 *
 *  Display value in ASCII display
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_disp( 
    UINT32 val );
    

/************************************************************************
 *
 *                          sys_disp_ch
 *  Description :
 *  -------------
 *
 *  Display character in ASCII display
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_disp_ch( 
    UINT8  pos,				/* Position			*/
    char   ch );			/* Character			*/


/************************************************************************
 *
 *                          sys_disp_str
 *  Description :
 *  -------------
 *
 *  Display string in ASCII display
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_disp_str( 
    char *ch );				/* String			*/


/************************************************************************
 *
 *                          sys_poll
 *  Description :
 *  -------------
 *
 *  Poll drivers that need to be polled.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_poll( void );


/************************************************************************
 *
 *                          sys_poll_enable
 *  Description :
 *  -------------
 *
 *  Enable/disable non-critical polling done by function sys_poll
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_poll_enable( 
    bool enable );


/************************************************************************
 *
 *                          sys_cp0_read32
 *  Description :
 *  -------------
 *
 *  Read 32 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  Value read
 *
 ************************************************************************/
UINT32
sys_cp0_read32(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel );	/* sel field (0..7)	   */


/************************************************************************
 *
 *                          sys_cp0_read64
 *  Description :
 *  -------------
 *
 *  Read 64 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  Value read
 *
 ************************************************************************/
UINT64
sys_cp0_read64(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel );	/* sel field (0..7)	   */


/************************************************************************
 *
 *                          sys_cp0_write32
 *  Description :
 *  -------------
 *
 *  Write 32 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cp0_write32(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel,		/* sel field (0..7)	   */
    UINT32 value );	/* Value to be written	   */


/************************************************************************
 *
 *                          sys_cp0_write64
 *  Description :
 *  -------------
 *
 *  Write 64 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cp0_write64(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel,		/* sel field (0..7)	   */
    UINT64 value );	/* Value to be written	   */


/************************************************************************
 *
 *                          sys_cp0_mtread32
 *  Description :
 *  -------------
 *
 *  Read 32 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  Value read
 *
 ************************************************************************/
UINT32
sys_cp0_mtread32(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel,		/* sel field (0..7)	   */
    UINT32 tc		/* TC to use               */
    );

/************************************************************************
 *
 *                          sys_cp0_mtwrite32
 *  Description :
 *  -------------
 *
 *  Write 32 bit CP0 register
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cp0_mtwrite32(
    UINT32 number,	/* Register number (0..31) */
    UINT32 sel,		/* sel field (0..7)	   */
    UINT32 value,	/* Value to be written	   */
    UINT32 tc		/* TC to use               */
    );


/************************************************************************
 *
 *                          sys_cp1_read
 *  Description :
 *  -------------
 *
 *  Read CP1 register
 *
 *  Return values :
 *  ---------------
 *
 *  Value read
 *
 ************************************************************************/
UINT32
sys_cp1_read(
    UINT32 number );	/* Register number (0..31) */


/************************************************************************
 *
 *                          sys_cp1_write
 *  Description :
 *  -------------
 *
 *  Write CP1 register
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cp1_write(
    UINT32 number,	/* Register number (0..31) */
    UINT32 value );	/* Value to be written	   */


/************************************************************************
 *
 *                          sys_fpu_enable
 *  Description :
 *  -------------
 *
 *  Enable FPU
 *
 ************************************************************************/
void sys_fpu_enable( void );


/************************************************************************
 *                          sys_cpu_cache_bpw
 ************************************************************************/
void
sys_cpu_cache_bpw(
    bool         icache,	/* TRUE -> icache, FALSE -> dcache	*/
    t_sys_array  *sys_array );


/************************************************************************
 *                          sys_cpu_cache_assoc
 ************************************************************************/
void
sys_cpu_cache_assoc(
    bool         icache,	/* TRUE -> icache, FALSE -> dcache	*/
    t_sys_array  *sys_array );


/************************************************************************
 *
 *                          sys_scache_flush_index
 *  Description :
 *  -------------
 *
 *  Flush L2 cache line containing specified index
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_scache_flush_index( 
    UINT32 index );


/************************************************************************
 *
 *                          sys_icache_invalidate_index
 *  Description :
 *  -------------
 *
 *  Fill I cache line containing specified index
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_icache_invalidate_index(
    UINT32 index );


/************************************************************************
 *
 *                          sys_icache_invalidate_addr
 *  Description :
 *  -------------
 *
 *  Invalidate I cache line containing specified address.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_icache_invalidate_addr(
    UINT32 addr,		/* Address to be invalidated. Must be KSEG0/KSEG1 */
    bool   flush_pipeline );    /* TRUE => flush pipeline after invalidation */


/************************************************************************
 *
 *                          sys_dcache_flush_index
 *  Description :
 *  -------------
 *
 *  Flush D cache line containing specified index
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_dcache_flush_index(
    UINT32 index );


/************************************************************************
 *
 *                          sys_dcache_flush_addr
 *  Description :
 *  -------------
 *
 *  Flush D cache line containing specified address.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_dcache_flush_addr(
    UINT32 addr );   /* Must be KSEG0/KSEG1 */


/************************************************************************
 *
 *                          sys_dcache_flush_all
 *  Description :
 *  -------------
 *
 *  Flush entire DCACHE.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_dcache_flush_all( void );


/************************************************************************
 *
 *                          sys_dcache_exclusivel2_flush_all
 *  Description :
 *  -------------
 *
 *  Flush entire DCACHE in the presences of an L2 exclusive cache.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_dcache_exclusivel2_flush_all( void );


/************************************************************************
 *
 *                          sys_icache_invalidate_all
 *  Description :
 *  -------------
 *
 *  Invalidate entire ICACHE.
 *
 *  We bypass the cache operations if CPU is running uncached.
 *  (We assume Config.k0 == K_CacheAttrU is the
 *  only uncached mode used).
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_icache_invalidate_all( void );


/************************************************************************
 *
 *                          sys_flush_caches
 *  Description :
 *  -------------
 *
 *  First flush entire DCACHE, then invalidate entire ICACHE
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_flush_caches( void );


/************************************************************************
 *
 *                          sys_flush_pipeline
 *  Description :
 *  -------------
 *
 *  Flush pipeline
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_flush_pipeline( void );


/************************************************************************
 *
 *                          sys_flush_cache_line
 *  Description :
 *  -------------
 *
 *  First flush DCACHE line, then invalidate ICACHE line
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_flush_cache_line( 
    void *addr );


/************************************************************************
 *
 *                          sys_cpu_icache_config
 *  Description :
 *  -------------
 *
 *  Configure icache
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_cpu_icache_config( 
    UINT32 is,
    UINT32 il,
    UINT32 ia,
    UINT32 config1_reset );


/************************************************************************
 *
 *                          sys_cpu_dcache_config
 *  Description :
 *  -------------
 *
 *  Configure dcache
 *	
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_cpu_dcache_config( 
    UINT32 ds,
    UINT32 dl,
    UINT32 da,
    UINT32 config1_reset );


/************************************************************************
 *
 *                          sys_tlb_lookup
 *  Description :
 *  -------------
 *
 *  Probe TLB for matching entry
 *
 *  Return values :
 *  ---------------
 *
 *  SYS_TLB_NOTFOUND : No match
 *  SYS_TLB_NOTVALID : Match with valid bit cleared, i.e. not valid
 *  SYS_TLB_WP	     : Match with dirty bit cleared, i.e. write-protected
 *  SYS_TLB_OK       : Valid and Dirty entry found
 *
 ************************************************************************/
UINT32
sys_tlb_lookup(
    UINT32 vaddr,		/* Virtual address			*/
    UINT32 *phys,		/* OUT : Physical address		*/
    UINT32 *pagesize );		/* OUT : Pagesize (byte count)		*/


/************************************************************************
 *
 *                          sys_tlb_write
 *  Description :
 *  -------------
 *
 *  Write TLB
 *
 *  data = pointer to array of 5 words
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
void
sys_tlb_write(
    UINT32 *data );


/************************************************************************
 *
 *                          sys_kseg0
 *  Description :
 *  -------------
 *
 *  Determine KSEG0 address corresponding to input address.
 * 
 *  In case input address is TLB mapped, a lookup is performed in the
 *  TLB to determine the physical address. Then, the corresponding
 *  KSEG0 address is calculated.
 *
 *  In case input address is in KSEG1 range, it is converted to KSEG0.
 *
 *  In case of fixed mapping TLB, a KUSEG address is mapped to a
 *  corresponding KSEG0 address based on the ERL setting.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE if conversion was successfull, otherwise FALSE
 *
 ************************************************************************/
bool
sys_kseg0(
    UINT32 addr,		/* Address to be converted		*/
    UINT32 *kseg0addr );	/* OUT : Converted address		*/


/************************************************************************
 *
 *                          sys_tlb_read
 *  Description :
 *  -------------
 *
 *  Read TLB
 *
 *  data = pointer to array of 4 words. They will be filled with  the
 *         following data :
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
void
sys_tlb_read(
    UINT32 index,
    UINT32 *data );


/************************************************************************	
 *
 *                          sys_cpu_k0_config
 *  Description :
 *  -------------
 *
 *  Configure K0 field of CP0 CONFIG register
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_cpu_k0_config( 
    UINT32 k0 );		/* K0 field to be written		*/


/************************************************************************	
 *
 *                          sys_cpu_l2_enable
 *  Description :
 *  -------------
 *
 *  Enable/disable L2 cache
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cpu_l2_enable(
    bool enable );		/* FALSE -> disable, else enable	*/


/************************************************************************	
 *
 *                          sys_cpu_l2_enabled
 *  Description :
 *  -------------
 *
 *  Return status of L2 cache
 *
 *  Return values :
 *  ---------------
 *
 *  0: L2 cache disabled
 *  1: L2 cache enabled
 *
 ************************************************************************/
UINT32
sys_cpu_l2_enabled( void );


/************************************************************************
 *
 *                          sys_cpu_configurability
 *  Description :
 *  -------------
 *
 *  Determine cpu configurability
 *
 *  Return values :
 *  ---------------
 *
 *  Mask with the following bit definitions :
 *
 *  Bit 0 : Set if MMU may be configured (TLD -> Fixed)
 *  Bit 1 : Set if cache may be downsized
 *
 ************************************************************************/
UINT32 
sys_cpu_configurability( void );
#define SYS_CPU_CONFIGURABILITY_MMU	1
#define SYS_CPU_CONFIGURABILITY_CACHE	2


/************************************************************************
 *
 *                          sys_cpu_config
 *  Description :
 *  -------------
 *
 *  Configure CPU cache/mmu settings
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_cpu_config( 
    bool	       icache,
    bool	       dcache,
    bool	       mmu,
    t_sys_cpu_decoded  *setting );


/************************************************************************
 *
 *                          sys_cpu_mmu_config
 *  Description :
 *  -------------
 *
 *  Configure MMU
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_cpu_mmu_config(
    UINT8  c8 );


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
 *  sys_mt
 *  sys_dsp
 *  sys_arch_rev
 *  sys_fpu
 *  sys_cpu_cache_coherency
 *
 ************************************************************************/
void
sys_cpu_type( void );


/************************************************************************
 *
 *                          sys_wait_ms
 *  Description :
 *  -------------
 *
 *  Wait for the specified interval
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
sys_wait_ms(
    UINT32 ms );	/* Interval in milliseconds			*/


/************************************************************************
 *
 *                          sys_reg_addr
 *  Description :
 *  -------------
 *
 *  Calc address from base address, register spacing and register offset.
 *  Also, in case of device with different endianness than CPU, adjust
 *  for endianness.
 *
 *  In case the following is true, the 2 lsb of the address need
 *  to be inverted :
 *
 *     1) Endianness of target and CPU are not the same
 *     3) spacing is 1 byte
 *
 *  Supports spacings of 8 bit and 32 bit (not 16 bit)
 *
 *  Return values :
 *  ---------------
 *
 *  address (void pointer)
 *
 ************************************************************************/
void *
sys_reg_addr(
    bool   target_bigend,	/* TRUE  -> target is big endian.
			           FALSE -> target is little endian.	*/
    UINT8  spacing,		/* spacing of regs in bytes		*/
    void  *base,		/* Base address				*/
    UINT32 offset );		/* Offset scaled down by spacing	*/


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
void 
sys_enable_int( void );


/************************************************************************
 *
 *                          sys_enable_int_mask
 *  Description :
 *  -------------
 *
 *  Enable specific CPU interrupt.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_enable_int_mask( 
    UINT32 cpu_int );		/* CPU interrupt 0..7			*/


/************************************************************************
 *
 *                          sys_disable_int_mask
 *  Description :
 *  -------------
 *
 *  Disable specific CPU interrupt.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
sys_disable_int_mask( 
    UINT32 cpu_int );		/* CPU interrupt 0..7			*/


/************************************************************************
 *
 *                          sys_disable_int
 *  Description :
 *  -------------
 *
 *  Disable interrupt: clear IE in CP0-status.
 *  
 *  Return values :
 *  ---------------
 *
 *  Old IE bit
 *
 ************************************************************************/
UINT32 
sys_disable_int( void );



/************************************************************************
 *
 *                          sys_store_control_regs
 *  Description :
 *  -------------
 *
 *  Store the current values of CP0 and CP1 control registers.
 *  
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
UINT32 
sys_store_control_regs(
    t_gdb_regs *context ); 


/************************************************************************
 *
 *                          sys_decode_compid
 *  Description :
 *  -------------
 *
 *  Map Company ID field of CPO PrId register to string with company name.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
char *
sys_decode_compid( 
    UINT8 compid );	/* Company ID field of CP0 PRId register	*/


/************************************************************************
 *
 *                          sys_decode_procid
 *  Description :
 *  -------------
 *
 *  Map processor ID field to string hodling the name of the CPU
 *  
 *  Return values :
 *  ---------------
 *
 *  String holding name of CPU
 *
 ************************************************************************/
char *
sys_decode_procid( void );


/************************************************************************
 *
 *                          sys_validate_range
 *  Description :
 *  -------------
 *
 *  Validate address range (alignment, TLB if mapped address, RAM range).
 *
 *  Return values :
 *  ---------------
 *
 *  OK :		    No error
 *  SHELL_ERROR_ALIGN :     Alignment error
 *  SHELL_ERROR_OVERFLOW :  Range overflow
 *  SHELL_ERROR_TLB :       Mapped address with no match in TLB
 *  SHELL_ERROR_TLB_WP :    Write access to mapped write protected address
 *  SHELL_ERROR_RAM_RANGE : Address in unused RAM space
 *
 ************************************************************************/
UINT32
sys_validate_range(
    UINT32 addr,	/* Start address				*/
    UINT32 count,	/* Byte count					*/
    UINT8  size,	/* Access size (number of bytes)		*/
    bool   write );	/* Write access					*/



/************************************************************************
 *
 *                          sys_cp0_printreg_all
 *  Description :
 *  -------------
 *
 *  Print CP0 registers.
 *
 ************************************************************************/
UINT32
sys_cp0_printreg_all(
    UINT32 tc,		    // TC to use
    t_gdb_regs  *context ); // If not NULL, this holds the context to be dumped.
		  	    // If NULL, print current values of all CP0 regs.

/************************************************************************
 *
 *                          sys_cp0_printreg
 *  Description :
 *  -------------
 *
 *  Print value of CP0 register specified by name.
 *
 ************************************************************************/
UINT32
sys_cp0_printreg( 
    UINT32 tc,	        // TC to use
    char *reg_name );   // Name of register.


/************************************************************************
 *
 *                          sys_cp0_writereg
 *  Description :
 *  -------------
 *
 *  Write to CP0 register specified by name.
 *
 ************************************************************************/
UINT32
sys_cp0_writereg(
    UINT32 tc,	      // TC to use
    char   *reg_name, // Name of register.
    UINT64 value );   // Value to be written.

#else  /* #ifndef _ASSEMBLER */

EXTERN( sys_processor )
EXTERN( sys_platform )
EXTERN( sys_corecard )
EXTERN( sys_manid )
EXTERN( sys_manpd )

EXTERN( sys_ramsize )

EXTERN( sys_l2_cache )
EXTERN( sys_l2_cache_enabled )
EXTERN( sys_l2_cache_lines )
EXTERN( sys_l2_cache_linesize )
EXTERN( sys_l2_cache_assoc )
EXTERN( sys_icache_linesize )
EXTERN( sys_icache_lines )
EXTERN( sys_icache_assoc )
EXTERN( sys_dcache_linesize )
EXTERN( sys_dcache_lines )
EXTERN( sys_dcache_assoc )

EXTERN( sys_init_icache )
EXTERN( sys_init_dcache )
		
EXTERN( sys_ftext_init )
EXTERN( sys_etext_ram )
EXTERN( sys_fbss )
EXTERN( sys_freemem )

EXTERN( sys_64bit )
EXTERN( sys_mips32_64 )
EXTERN( sys_mips16e )
EXTERN( sys_smallpage_support )
EXTERN( sys_ejtag )
EXTERN( sys_mt )
EXTERN( sys_dsp )
EXTERN( sys_arch_rev )
EXTERN( sys_fpu )

#endif /* #ifndef _ASSEMBLER */


/* Return codes from sys_tlb_lookup() */

#define SYS_TLB_OK		        0
#define SYS_TLB_NOTFOUND		1
#define SYS_TLB_NOTVALID		2
#define SYS_TLB_WP			3

/* Stack size */
#define SYS_STACK_SIZE			0x5000
#define SYS_APPL_STACK_SIZE		0x5000

/*  RAM vector offsets (ie based at 0x80000000) 
 *  for CacheErr, EJTAG and NMI exceptions.
 */
# define SYS_CACHEERR_RAM_VECTOR_OFS     0x100
/*
 * YAMON 2.06 and earlier installed used these locations
 * to transfer NMI/EJTAG exceptions to an application
 */
# define SYS_OLD_EJTAG_RAM_VECTOR_OFS    0x300 /* chosen by YAMON */
# define SYS_OLD_NMI_RAM_VECTOR_OFS      0x380 /* chosen by YAMON */
/*
 * With the introduction of EIC support in 2.07 and later, these vectors
 * have been moved here.
 * For backwards compatibility, applications should install
 * handlers at both these addreses unless EIC mode is in use
 */
# define SYS_EJTAG_RAM_VECTOR_OFS        0xa00 /* chosen by YAMON */
# define SYS_NMI_RAM_VECTOR_OFS          0xa80 /* chosen by YAMON */

#endif /* #ifndef SYS_API_H */




