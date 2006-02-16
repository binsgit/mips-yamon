
/************************************************************************
 *
 *  sys.c
 *
 *  System functions
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
#include <io_api.h>
#include <syscon_api.h>
#include <net_api.h>
#include <serial_api.h>
#include <syserror.h>
#include <shell_api.h>
#include <sys_api.h>
#include <product.h>
#include <stdio.h>
#include <mips.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char   msg[30];

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static bool
determine_dev( 
    UINT32 port, 
    UINT32 *major, 
    UINT32 *minor );

static UINT32
sys_validate_ram_range(
    UINT32 start,		/* Start address (physical)		*/
    UINT32 last );		/* Last address (physical)		*/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *                          sys_putchar
 ************************************************************************/
void 
sys_putchar( 
    UINT32 port,
    char   ch )
{
    UINT32 major, minor;

    /* LF -> CR+LF due to some terminal programs */ 
    if( ch == LF )
        sys_putchar( port, CR );

    if( determine_dev( port, &major, &minor ) )
    {
        IO_write( major, minor, (UINT8 *)(&ch) );
    }
}


/************************************************************************
 *                          sys_puts
 ************************************************************************/
void
sys_puts( 
    UINT32 port, 
    char *s )
{
    while( *s != '\0' )
    {
        sys_putchar( port, *(s++) );
    }
} 


/************************************************************************
 *                          sys_getchar
 ************************************************************************/
bool			  /* TRUE -> OK, FALSE -> FAILED		*/
sys_getchar(
    UINT32 port,
    char   *ch )
{
    UINT32 major, minor;
    INT32  rc;

    if( port == PORT_NET )
    {
        *ch = net_getchar();
	return TRUE;
    }

    if( determine_dev( port, &major, &minor ) )
    {
        /* Poll whatever needs to be polled (platform specific) */
        sys_poll();
	
	rc = IO_read( major, minor, (UINT8 *)(ch) );

        if ( (rc == OK) || (rc == ERROR_SERIAL_COMM_BREAK) )
        {
            return TRUE;
        }

        if ( rc == ERROR_SERIAL_COMM_ERROR )
        {
            *ch = UART_ERROR;
            return TRUE;
        }

        return FALSE;
    }
    else
        return FALSE;
}


/************************************************************************
 *                          sys_getchar_ctrlc
 ************************************************************************/
bool			  /* TRUE -> CTRL-C received			*/
sys_getchar_ctrlc(
    UINT32 port )
{
    UINT32		     major, minor;
    t_SERIAL_ctrl_descriptor descr;

    descr.sc_command = SERIAL_CTRL_POLL_BREAK;

    if( determine_dev( port, &major, &minor ) )
    {
        return 
	   (IO_ctrl( major, minor, (void *)&descr ) ==
		     ERROR_SERIAL_COMM_BREAK) ?
		         TRUE :
			 FALSE;
    }  
    else
        return FALSE;
}


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
void sys_disp( 
    UINT32 val )
{
    SYSCON_write( SYSCON_BOARD_ASCIIWORD_ID,
		  (void *)&val, sizeof(UINT32) );
}


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
void sys_disp_ch( 
    UINT8  pos, 
    char   ch )
{
    t_sys_alphanumeric data;
  
    data.posid  = pos;
    data.string = (UINT8 *)&ch;

    SYSCON_write( SYSCON_BOARD_ASCIICHAR_ID,
		  (void *)&data, sizeof(t_sys_alphanumeric) );
}


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
void sys_disp_str( 
    char *s )
{
    t_sys_alphanumeric data;
    
    data.posid  = 0;
    data.string = (UINT8 *)s;

    SYSCON_write( SYSCON_BOARD_ASCIISTRING_ID,\
		  (void *)&data, sizeof(t_sys_alphanumeric) );
}


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
    UINT32 ms )	/* Interval in milliseconds			*/
{
    UINT32 first, current, last;
    UINT32 cycle_per_count;

    SYSCON_read(
        SYSCON_CPU_CYCLE_PER_COUNT_ID,
	&cycle_per_count,
	sizeof(UINT32) );

    /* read start value */
    SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		 (void *)(&first),
		 sizeof(UINT32) );

    last = first + ((sys_cpufreq_hz / 1000) * ms / cycle_per_count);


    if (last >= first)
    do
    {
	/* no wrap - continue while between first and last */
        SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		     (void *)(&current),
		     sizeof(UINT32) );
    }
    while( current < last && current >= first );
    else
    do
    {
	/* wrap - continue until between first and last */
        SYSCON_read( SYSCON_CPU_CP0_COUNT_ID,
		     (void *)(&current),
		     sizeof(UINT32) );
    }
    while( current < last || current >= first );
}


/************************************************************************
 *
 *                          sys_reg_addr
 *  Description :
 *  -------------
 *
 *  Calc address from base address, register spacing and register offset.
 *  Also, in case of device with different endianness than CPU, adjust
 *  for endianness (only supported for 1 byte spacings).
 *
 *  In case the following is true, the 2 lsb of the address need
 *  to be inverted :
 *
 *     1) Endianness of target and CPU are not the same
 *     3) spacing is 1 byte
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
    UINT32 offset )		/* Offset scaled down by spacing	*/
{    
    UINT32 addr = (UINT32)base + offset * spacing;
  
    if( spacing == sizeof(UINT8) )
    {
        addr = target_bigend ?
	           SWAP_BYTEADDR_EB( addr ) :
                   SWAP_BYTEADDR_EL( addr );
    }

    return (void *)addr;
}


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
    bool   write )	/* Write access					*/
{
    UINT32  last, pagesize;
    bool    sys_mmu_tlb;
    UINT32  phys;
    UINT32  rc;
    UINT8   range_start;
    UINT32  erl;

#define R_KUSEG    0
#define R_KSEG0	   1
#define R_KSEG1	   2
#define R_KSSEG	   3
#define R_KSEG3	   4

    if( !sys_legal_align( addr, size ) || !sys_legal_align( count, size ) )
        return SHELL_ERROR_ALIGN;

    if( count == 0 )
        return OK;

    last = addr + (count - size);
 
    if( last < addr )
        return SHELL_ERROR_OVERFLOW;

    SYSCON_read( SYSCON_CPU_TLB_AVAIL_ID,
	         (void *)&sys_mmu_tlb,
	         sizeof( bool ) );

    if(      KUSEG( addr ) == addr ) range_start = R_KUSEG;
    else if( KSEG0( addr ) == addr ) range_start = R_KSEG0;
    else if( KSEG1( addr ) == addr ) range_start = R_KSEG1;
    else if( KSSEG( addr ) == addr ) range_start = R_KSSEG;
    else			     range_start = R_KSEG3;

    switch( range_start )
    {
      case R_KUSEG :
      case R_KSSEG :
      case R_KSEG3 :
        if( !sys_mmu_tlb )
	{
	    /* Assume Fixed Mapping MMU */

	    /* Determine ERL setting */
	    SYSCON_read( SYSCON_CPU_CP0_STATUS_ID,
			 (void *)&erl,
			 sizeof(UINT32) );

	    erl &= M_StatusERL;

            if( last < KSEG0BASE )
	    {
                /* Range contained within KUSEG */
		if( !erl )
		{
		    addr += 0x40000000;
		    last += 0x40000000;
		}

   	        return sys_validate_ram_range( addr, last );
	    }
	    else if( addr >= KSSEGBASE )
	    {
	        /* Range contained within KSSEG,KSEG3 */
   	        return sys_validate_ram_range( addr, last );
	    }
	    else
	    {
                /* Range overflows KUSEG */
	        rc = sys_validate_ram_range( 
		         erl ? addr : addr + 0x40000000, 
			 erl ? KSEG0BASE- size : KSEG0BASE - size + 0x40000000 );
	      
	        return ( rc != OK ) ?
	            rc :
	            /* Continue in KSEG0 */
		    sys_validate_range( KSEG0BASE,
				        count - (KSEG0BASE - addr),
				        size,
				        write );
	    }
	}
	else
	{
	    /* MMU = TLB */
	    switch( sys_tlb_lookup( addr, &phys, &pagesize ) )
            {
	      case SYS_TLB_WP :
		if( write )
		{
                    sprintf( msg, "Address = 0x%08x", addr );
	            shell_error_data = msg;
		    return SHELL_ERROR_TLB_WP;
		}
		/* Fallthrough !! */   
	      case SYS_TLB_OK :
		rc = sys_validate_ram_range( phys, 
					     phys + pagesize - size );

		if( rc != OK )
		   return rc;

		addr += pagesize;

		return ( addr > last ) ?
		    OK :
		    sys_validate_range( addr,
					count - pagesize,
					size,
					write );
		break;
	      case SYS_TLB_NOTFOUND :
	      case SYS_TLB_NOTVALID :
	      default : /* Should not happen */
                sprintf( msg, "Address = 0x%08x", addr );
                shell_error_data = msg;
	        return SHELL_ERROR_TLB;
            }
	}
	break;
      case R_KSEG0 :
        if( KSEG0( last ) == last )
	{
            /* Range contained within KSEG0 */
   	    return sys_validate_ram_range( PHYS(addr), PHYS(last) );
        }
	else
	{
            /* Range overflows KSEG0 */
	    rc = sys_validate_ram_range( PHYS(addr), PHYS(KSEG1BASE-size) );

	    return ( rc != OK ) ?
	        rc :
	        /* Continue in KSEG1 */
		sys_validate_range( KSEG1BASE,
				    count - (KSEG1BASE - addr),
				    size,
				    write );
	}
	break;
      case R_KSEG1 :
        if( KSEG1( last ) == last )
	{
            /* Range contained within KSEG1 */
   	    return sys_validate_ram_range( PHYS(addr), PHYS(last) );
        }
	else
	{
            /* Range overflows KSEG1 */
	    rc = sys_validate_ram_range( PHYS(addr), PHYS(KSSEGBASE-size) );

	    return ( rc != OK ) ?
	        rc :
	        /* Continue in KSSEG */
		sys_validate_range( KSSEGBASE,
				    count - (KSSEGBASE - addr),
				    size,
				    write );
	}
	break;
    }

    return OK; /* Should never reach this point */
}


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
    UINT32 *kseg0addr )		/* OUT : Converted address		*/
{
    UINT32 phys, pagesize;
    bool   sys_mmu_tlb;
    UINT32 erl;

    SYSCON_read( SYSCON_CPU_TLB_AVAIL_ID,
                 (void *)&sys_mmu_tlb,
	         sizeof( bool ) );

    if( sys_mmu_tlb )
    {
        /* MMU = TLB */
        if( (KSEG0( addr ) == addr) || (KSEG1( addr ) == addr) )
        {
            *kseg0addr = KSEG0(addr);
	    return TRUE;
        }
	else
	{
	    if( sys_tlb_lookup( addr, &phys, &pagesize ) == SYS_TLB_OK )
	    {
	        *kseg0addr = KSEG0( phys );
		return TRUE;
	    }
	    else
	        return FALSE;
	}
    }
    else
    {
	/* Assume Fixed Mapping MMU */
        if(      KUSEG( addr ) == addr )
        {
	    /* Determine ERL setting */
	    SYSCON_read( SYSCON_CPU_CP0_STATUS_ID,
			 (void *)&erl,
			 sizeof(UINT32) );

	    if( (erl & M_StatusERL) == 0 )
	        return FALSE; /* Can't be mapped to KSEG0 */

	    if( (addr & KSEG_MSK) != addr )
	        return FALSE; /* Can't be mapped to KSEG0 */
	
	    *kseg0addr = KSEG0(addr);
	    
	    return TRUE;
        }
        else if( KSEG0( addr ) == addr )
        {
            *kseg0addr = addr;
	    return TRUE;
        }
        else if( KSEG1( addr ) == addr )
        {
            *kseg0addr = KSEG0(addr);
	    return TRUE;
        }
        else
        {
            /* KSSEG/KSEG3, can't be mapped to KSEG0 */
	    return FALSE;
        }
    }
}


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
    UINT32 align )
{
    return (address == (address & ~(align - 1))) ? TRUE : FALSE;
}


/************************************************************************
 *  Implementation : static functions
 ************************************************************************/


/************************************************************************
 *                          determine_dev
 ************************************************************************/
static bool
determine_dev( 
    UINT32 port, 
    UINT32 *major, 
    UINT32 *minor )
{
    UINT32 id_major, id_minor;

    if( port == PORT_TTY0 )
    {
	id_major = SYSCON_COM_TTY0_MAJOR;
	id_minor = SYSCON_COM_TTY0_MINOR;
    }
    else if( port == PORT_TTY1 )
    {	
	id_major = SYSCON_COM_TTY1_MAJOR;
	id_minor = SYSCON_COM_TTY1_MINOR;
    }
    else
	return FALSE;

    SYSCON_read( id_major,
	         (void *)(major),
		 sizeof(UINT32) );

    SYSCON_read( id_minor,
	         (void *)(minor),
	         sizeof(UINT32) );
		     
    return TRUE;
}


/************************************************************************
 *
 *                          sys_validate_ram_range
 *  Description :
 *  -------------
 *
 *  Perform range check relating to RAM use.
 *
 *  The range defined by parameters 'start' and 'last' is checked with
 *  relation to RAM use.
 *
 *  The memory map allocates a range for RAM. In case the RAM
 *  module does not occupy this entire range, there will be an
 *  "empty" RAM range. We detect whether the specified range
 *  hits this empty RAM range. If so, we return an error code. 
 *  Otherwise, OK is returned.
 *
 *  In case of a hit in the unused range, shell_err_data is set to a 
 *  text string stating the illegal address. This may be used by 
 *  error handling.
 *
 *  Return values :
 *  ---------------
 *
 *  OK :		    No error
 *  SHELL_ERROR_RAM_RANGE : Error detected
 *
 ************************************************************************/
static UINT32
sys_validate_ram_range(
    UINT32 start,		/* Start address (physical)		*/
    UINT32 last )		/* Last address (physical)		*/
{
    static void    *ram_base;
    static UINT32  ram_actual_size, ram_size;
    static UINT32  ram_range_unused_start;
    static UINT32  ram_range_unused_last;
    static bool    first = TRUE;
    
    if( first )
    {
        /* Self-initialisation */

        first = FALSE;

        SYSCON_read( SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID,
		     (void *)&ram_actual_size,
		     sizeof(UINT32) );

        SYSCON_read( SYSCON_BOARD_SYSTEMRAM_BASE_ID,
		     (void *)&ram_base,
		     sizeof(void *) );

        SYSCON_read( SYSCON_BOARD_SYSTEMRAM_SIZE_ID,
		     (void *)&ram_size,
		     sizeof(UINT32) );

        ram_range_unused_start = (UINT32)ram_base + ram_actual_size;
        ram_range_unused_last  = (UINT32)ram_base + ram_size - 1;
    }

    if( ram_actual_size == ram_size )
        return OK;  /* No unused RAM space */

    if( start <= ram_range_unused_last )
    {
        if( start >= ram_range_unused_start )
	{
            sprintf( msg, "Address = 0x%08x", start );
	    shell_error_data = msg;

	    return SHELL_ERROR_RAM_RANGE;
	}

        if( last >= ram_range_unused_start )
	{
            sprintf( msg, "Address = 0x%08x", ram_range_unused_start );
	    shell_error_data = msg;

	    return SHELL_ERROR_RAM_RANGE;
	}
    }

    return OK;
}


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
    UINT32	     parm3 )
{
    UINT32 old_ie;
    UINT32 rc;
    
    /* Disable interrupts */
    old_ie = sys_disable_int();

    /* Stop DMA */
    sys_dma_enable( FALSE );
    
    /* Call function */
    rc = (*func)(parm1, parm2, parm3);

    /* Restart DMA */
    sys_dma_enable( TRUE );

    /* Restore interrupt enable setting */
    if(old_ie)
        sys_enable_int();

    return rc;
}


