
/************************************************************************
 *
 *  tlb.c
 *
 *  Monitor command for displaying and setting TLB.
 *
 *  tlb -i [-s]    (-s only for MIPS32/64 Release 2 CPUs)
 *
 *     or
 *
 *  tlb [ <index> <pagesize> 
 *	  <va>  <g>  <asid>
 *	  <pa0> <c0> <d0> <v0>
 *	  <pa1> <c1> <d1> <v1> ]
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
#include <sys_api.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <shell_api.h>
#include <syscon_api.h>
#include <mips.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    bool   read;
    UINT8  index;
    UINT32 pagemask;
    UINT32 vpn;
    UINT32 asid;
    bool   global;
    UINT32 pfn[2];
    UINT8  cache[2];
    bool   dirty[2];
    bool   valid[2];
}
t_tlb_setup;

#define TLB_MAX_ASID		(M_EntryHiASID >> S_EntryHiASID)
#define TLB_MAX_C		(M_EntryLoC   >> S_EntryLoC)
#define TLB_MAX_D		(M_EntryLoD   >> S_EntryLoD)
#define TLB_MAX_V		(M_EntryLoV   >> S_EntryLoV)
#define TLB_MAX_GLOBAL		(M_EntryLoG   >> S_EntryLoG)


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_INIT	  0
  { "i", "Initialise TLB"	           },
#define OPTION_SMALLPAGE  1
  { "s", "Toggle small (1kB) page support" }
};
#define OPTION_COUNT_REL2     (sizeof(options)/sizeof(t_cmd_option))
#define OPTION_COUNT_NOT_REL2 (sizeof(options)/sizeof(t_cmd_option)-1)

static UINT32 option_count;

static UINT8  tlb_entries;
static bool   smallpages;
static UINT32 pagegrain;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32      argc,
    char        **argv,
    bool	*init,
    bool	*toggle_smallpages,
    t_tlb_setup *tlb_setup );

static bool
set_flag(
    bool	   *flag,
    t_shell_option *decode );

static bool
get_size(
    UINT32	   *pagesize, 
    t_shell_option *decode );

static bool
size2mask(
    UINT32 pagesize,
    UINT32 *mask );

static bool
mask2size(
    UINT32 mask,
    UINT32 *pagesize );

static void
do_init( 
    bool toggle_smallpages );

static void
do_tlb( 
    t_tlb_setup *tlb_setup );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          tlb
 ************************************************************************/
static MON_FUNC(tlb)
{
    UINT32      rc;
    bool	tlb_avail;
    t_tlb_setup tlb_setup;
    bool	init, toggle_smallpages;

    SYSCON_read( SYSCON_CPU_TLB_AVAIL_ID,
		 (void *)&tlb_avail,
		 sizeof(bool) );

    if( !tlb_avail )
    {
        printf( "No TLB\n" );
	return OK;
    }
    else
    {
        SYSCON_read( SYSCON_CPU_TLB_COUNT_ID,
		     (void *)&tlb_entries,
		     sizeof(UINT8) );
    }

    /* Determine whether small pages are currently enabled */
    if( sys_smallpage_support )
    {
        rc = SYSCON_read( SYSCON_CPU_CP0_PAGEGRAIN_ID,
		          (void *)&pagegrain,
		          sizeof(UINT32) );

        if( rc == OK )
	{
	    smallpages = (pagegrain & M_PageGrainESP) ? TRUE : FALSE;
	}
    }

    rc = get_options( argc, argv, &init, &toggle_smallpages, &tlb_setup );

    if( rc != OK )
        return rc;
    else
    {
        if( init )
	{
	    do_init( toggle_smallpages );
        }
	else
	{
	    do_tlb( &tlb_setup );
	}
	return OK;
    }
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32      argc,
    char        **argv,
    bool	*init,
    bool	*toggle_smallpages,
    t_tlb_setup *tlb_setup )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    UINT32	   pagesize;
    UINT32	   error      = SHELL_ERROR_SYNTAX;
    bool	   ok         = TRUE;
    bool	   got_value  = FALSE;
    bool	   got_option = FALSE;
    UINT32	   i;

    /* Default */
    *init              = FALSE;
    *toggle_smallpages = FALSE;

    if( argc == 1)
    {
        tlb_setup->read = TRUE;
	return OK;
    }
    else
        tlb_setup->read = FALSE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :

	    if( got_value )
	    {
	        ok = FALSE;
	    }
	    else
	    {
	        got_option = TRUE;

	        /* Find match */
	        for(i=0; 
		    (i<option_count) &&
		    (strcmp(decode.option, options[i].option) != 0);
		    i++) ;

	        if( i == option_count )
	            i = 0xff; /* Make sure we hit default in below switch */

	        switch(i)
	        {
	          case OPTION_INIT      : *init              = TRUE; break;
	          case OPTION_SMALLPAGE : *toggle_smallpages = TRUE; break;
	          default :
		    error	     = SHELL_ERROR_OPTION;
		    shell_error_data = argv[arg];
		    ok		     = FALSE;
		    break;		      
	        }
            }

	    break;

	  case SHELL_TOKEN_STRING :

	    if( got_option )
	    {
	        ok = FALSE;
	    }
	    else
	    {
	        got_value = TRUE;

	        switch( arg )
	        {
	          case 2 : 
	            /* Pagesize */
		    ok = get_size( &pagesize, &decode );

		    if( ok && !size2mask( pagesize, &tlb_setup->pagemask ) )
		    {
		        error = SHELL_ERROR_TLB_PAGESIZE;
		        ok    = FALSE;
		    }
		    break;
	          case 4 :
	            /* Global */
		    ok = set_flag( &tlb_setup->global, &decode );
		    if( !ok )
		        error = SHELL_ERROR_TLB_GLOBAL;
		    break;
	          case 8 :
	            /* D0 */
		    ok = set_flag( &tlb_setup->dirty[0], &decode );
		    if( !ok )
		        error = SHELL_ERROR_TLB_D;
		    break;
	          case 9 :
	            /* V0 */
		    ok = set_flag( &tlb_setup->valid[0], &decode );
		    if( !ok )
		        error = SHELL_ERROR_TLB_V;
		    break;
	          case 12 :
	            /* D1 */
		    ok = set_flag( &tlb_setup->dirty[1], &decode );
		    if( !ok )
		        error = SHELL_ERROR_TLB_D;
		    break;
	          case 13 :
	            /* V1 */
		    ok = set_flag( &tlb_setup->valid[1], &decode );
		    if( !ok )
		        error = SHELL_ERROR_TLB_V;
		    break;
	          default :
	            ok = FALSE;
		    break;
	        }
            }

	    break;

	  case SHELL_TOKEN_NUMBER :

	    if( got_option )
	    {
	        ok = FALSE;
	    }
	    else
	    {
	        got_value = TRUE;

	        switch( arg )
	        {
	          case 1 : 
	            /* Index */
		    if( argc != 14 )
		        ok = FALSE;
		    else if( decode.number >= tlb_entries )
		    {
		        error = SHELL_ERROR_TLB_INDEX;
		        ok    = FALSE;
		    }
		    else
	                tlb_setup->index = decode.number;
		    break;
	          case 2 : 
	            /* Pagesize */
		    pagesize = decode.number;
		
		    if( !size2mask( pagesize, &tlb_setup->pagemask ) )
		    {
		        error = SHELL_ERROR_TLB_PAGESIZE;
		        ok    = FALSE;
		    }
		    break;
	          case 3 : 
	            /*  VA :
		     *
		     *  The Virtual address base (VA) must be aligned 
		     *  to pagesize * 2.
		     *  We multiply by two since the VA specified must be
		     *  the even page of the even/odd pair that will be
		     *  setup.
		     */
		    if( decode.number & tlb_setup->pagemask )
		    {
		        error = SHELL_ERROR_ALIGN;
		        ok    = FALSE;
		    }
		    else
                        tlb_setup->vpn = decode.number;
		    break;
	          case 5 :
	            /* ASID */
		    if( decode.number > TLB_MAX_ASID )
		    {
		        error = SHELL_ERROR_TLB_ASID;
		        ok    = FALSE;
		    }
		    else
		        tlb_setup->asid = decode.number;
		    break;
	          case 6 :
	            /*  PFN0 :
		     *
		     *  The Physical address base must be aligned to pagesize.
		     */  
		    if( decode.number & (tlb_setup->pagemask >> 1) )
		    {
		        error = SHELL_ERROR_ALIGN;
		        ok    = FALSE;
		    }
		    else
	                tlb_setup->pfn[0] = 
			    decode.number >> (smallpages ? 10 : 12);
		    break;
	          case 7 :
	            /* C0 */
		    if( decode.number > TLB_MAX_C )
		    {
		        error = SHELL_ERROR_TLB_C;
		        ok    = FALSE;
		    }
		    else
		        tlb_setup->cache[0] = decode.number;
		    break;
	          case 10 :
	            /*  PFN1 :
		     *
		     *  The Physical address base must be aligned to pagesize.
		     */  
		    if( decode.number & (tlb_setup->pagemask >> 1) )
		    {
		        error = SHELL_ERROR_ALIGN;
		        ok    = FALSE;
		    }
		    else
	                tlb_setup->pfn[1] = 
			    decode.number >> (smallpages ? 10 : 12);
		    break;
	          case 11 :
	            /* C1 */
		    if( decode.number > TLB_MAX_C )
		    {
		        error = SHELL_ERROR_TLB_C;
		        ok    = FALSE;
		    }
		    else
		        tlb_setup->cache[1] = decode.number;
		    break;
	          default :
	            ok = FALSE;
		    break;
                }
            }

	    break;

	  default :
	    ok = FALSE;
	    break;
        }
    }

    if( *toggle_smallpages && !(*init) )
        ok = FALSE;

    return ok ? OK : error;
}


/************************************************************************
 *                          set_flag
 ************************************************************************/
static bool
set_flag(
    bool	   *flag,
    t_shell_option *decode )
{
    char ch;

    if( strlen( decode->string ) != 1 )
        return FALSE;

    ch = tolower( *decode->string );

    if( ch == 'y' )
    {
        *flag = TRUE;
	return TRUE;
    }

    if( ch == 'n' )
    {
        *flag = FALSE;
	return TRUE;
    }

    return FALSE;
}


/************************************************************************
 *                          get_size
 ************************************************************************/
static bool
get_size(
    UINT32	   *pagesize, 
    t_shell_option *decode )
{
    if( smallpages && (strcmp( decode->string, "1kB" ) == 0 ) )
    {
        *pagesize = 0x400;    
    }
    else if( strcmp( decode->string, "4kB" )   == 0 )
    {
        *pagesize = 0x1000;    
    }
    else if( strcmp( decode->string, "16kB" )  == 0 )
    {
        *pagesize =  0x4000;    
    }
    else if( strcmp( decode->string, "64kB" )  == 0 )
    {
        *pagesize =  0x10000;   
    }
    else if( strcmp( decode->string, "256kB" ) == 0 )
    {
        *pagesize =  0x40000;  
    }
    else if( strcmp( decode->string, "1MB" )   == 0 )
    {
        *pagesize =  0x100000;  
    }
    else if( strcmp( decode->string, "4MB" )   == 0 )
    {
        *pagesize =  0x400000; 
    }
    else if( strcmp( decode->string, "16MB" )  == 0 )
    {
        *pagesize =  0x1000000; 
    }
    else	
        return FALSE;

    return TRUE;
}


/************************************************************************
 *                          size2mask
 ************************************************************************/
static bool
size2mask(
    UINT32 pagesize,
    UINT32 *mask )
{
    *mask = MSK( S_PageMaskMask );

    switch( pagesize )
    {
      case 0x400 :
        /* 1k */
	if( smallpages )
	{
	    *mask = MSK( S_PageMaskMaskX );
	    return TRUE;
	}
	else
	    return FALSE;
      case 0x1000 :
        /* 4k */
        *mask |= K_PageMask4K << S_PageMaskMask;
	return TRUE;
      case 0x4000 :
        /* 16k */
        *mask |= K_PageMask16K << S_PageMaskMask;;
	return TRUE;
      case 0x10000 :
        /* 64k */
        *mask |= K_PageMask64K << S_PageMaskMask;;
	return TRUE;
      case 0x40000 :
        /* 256k */
        *mask |= K_PageMask256K << S_PageMaskMask;;
	return TRUE;
      case 0x100000 :
        /* 1M */
        *mask |= K_PageMask1M << S_PageMaskMask;;
	return TRUE;
      case 0x400000 :
        /* 4M */
        *mask |= K_PageMask4M << S_PageMaskMask;;
	return TRUE;
      case 0x1000000 :
        /* 16M */
        *mask |= K_PageMask16M << S_PageMaskMask;;
	return TRUE;
      default :
        return FALSE;
    }
}


/************************************************************************
 *                          mask2size
 ************************************************************************/
static bool
mask2size(
    UINT32 mask,
    UINT32 *pagesize )
{
    if( smallpages )
    {
        if( (mask >> S_PageMaskMaskX) == 0 )
	{
            /* 1k */
	    *pagesize = 0x400;
	    return TRUE;
        }
	else if( (mask & M_PageMaskMaskX) != M_PageMaskMaskX )
	{
	    return FALSE;
        }
	else
            mask >>= S_PageMaskMask;
    }
    else
        mask >>= S_PageMaskMask;

    switch( mask )
    {
      case K_PageMask4K :
        /* 4k */
	*pagesize = 0x1000;
	return TRUE;
      case K_PageMask16K :
        /* 16k */
	*pagesize = 0x4000;
	return TRUE;
      case K_PageMask64K :
        /* 64k */
	*pagesize = 0x10000;
	return TRUE;
      case K_PageMask256K :
        /* 256k */
	*pagesize = 0x40000;
	return TRUE;
      case K_PageMask1M :
        /* 1M */
	*pagesize = 0x100000;
	return TRUE;
      case K_PageMask4M :
        /* 4M */
	*pagesize = 0x400000;
	return TRUE;
      case K_PageMask16M :
        /* 16M */
	*pagesize = 0x1000000;
	return TRUE;
      default :
        return FALSE;
    }
}


/************************************************************************
 *                          do_init
 ************************************************************************/
static void
do_init( 
    bool toggle_smallpages )
{
    /* Initialise TLB. We make sure all entries have different VPN2 */
    UINT32 i;
    UINT32 data[5];
    UINT32 value;

    if( toggle_smallpages )
    {
        value = 0;
        SYSCON_write( SYSCON_CPU_CP0_ENTRYLO0_ID, (void *)&value, sizeof(UINT32) );
        SYSCON_write( SYSCON_CPU_CP0_ENTRYLO1_ID, (void *)&value, sizeof(UINT32) );
        SYSCON_write( SYSCON_CPU_CP0_ENTRYHI_ID,  (void *)&value, sizeof(UINT32) );
	value = M_PageMaskMaskX;
        SYSCON_write( SYSCON_CPU_CP0_PAGEMASK_ID, (void *)&value, sizeof(UINT32) );

        if( smallpages )
	    pagegrain ^= M_PageGrainESP;
	else
	    pagegrain |= M_PageGrainESP;

        SYSCON_write( SYSCON_CPU_CP0_PAGEGRAIN_ID,
		      (void *)&pagegrain,
		      sizeof(UINT32) );
    }

    for( i=0; i<tlb_entries; i++ )
    {
	/* Index */
	data[0] = i;

        /* Pagemask */
	data[1] = 0;

	/*  EntryHi (Don't use 0 value since this is typically
	 *  the reset value, and we don't want collisions).
	 */
	data[2] = (i+1) << S_EntryHiVPN2;

	/* EntryLo0 */
	data[3] = 0;

	/* EntryLo1 */
	data[4] = 0;

	sys_tlb_write( data );
    }
}


/************************************************************************
 *                          do_tlb
 ************************************************************************/
static void
do_tlb( 
    t_tlb_setup *tlb_setup )
{
    UINT32 data[5];
    UINT32 pagesize, pa;
    UINT32 i;
    char   msg[80];
    
    if( tlb_setup->read )
    {
        if( sys_smallpage_support )
	{
	    if( smallpages )
	    {
	        if( SHELL_PUTS( "Small pages enabled !" ) )
		    return;
	    }
	    else
	    {
	        if( SHELL_PUTS( "Small pages not enabled" ) )
		    return;
	    }
	}

	if( SHELL_PUTS( 
	        "\n"
		"Index Page  VA         G ASID PA0        C0 D0 V0 PA1        C1 D1 V1\n"
		"---------------------------------------------------------------------\n" ) )
	{
	    return;
        }    
	
	for( i=0; i<tlb_entries; i++ )
	{
	    sys_tlb_read( i, data );

	    /* Index */
	    sprintf( msg, "0x%x", i );
	    if(SHELL_PUTS( msg )) return;

	    /* Pagesize */
	    if( mask2size( data[0], &pagesize ) )
	    {
	        if( pagesize < 1024*1024 )
	        {   
		    sprintf( msg, "%dkB", pagesize / 1024 );
	            if(SHELL_PUTS_INDENT( msg, 6 )) return;
	        }
	        else
	        {
	            sprintf( msg, "%dMB", pagesize / 1024 / 1024 );
 	            if(SHELL_PUTS_INDENT( msg, 6 )) return;
	        }
	    }
	    else
	    {
                if(SHELL_PUTS_INDENT( "?", 6 )) return;
	    }

	    /* Virtual address */
	    if( smallpages )
	        sprintf( msg, "0x%08x", data[1] & (M_EntryHiVPN2 | M_EntryHiVPN2X) );
	    else
	        sprintf( msg, "0x%08x", data[1] & M_EntryHiVPN2 );
	    if( SHELL_PUTS_INDENT( msg, 12 )) return;

	    /* Global */
	    sprintf( msg, "%c", 
		     ( (data[2] & M_EntryLoG) &&
		       (data[3] & M_EntryLoG) ) ?
		         'y' : 'n' );
	    if( SHELL_PUTS_INDENT( msg, 23 )) return;

	    /* ASID */
	    sprintf( msg, "0x%02x", (data[1] & M_EntryHiASID) >> S_EntryHiASID );
	    if( SHELL_PUTS_INDENT( msg, 25 )) return;

	    /* Physical address, even */
	    pa  = ( (data[2] & M_EntryLoPFN) >> S_EntryLoPFN ) << (smallpages ? 10 : 12);
	    pa &= ~(pagesize - 1);
	    sprintf( msg, "0x%08x", pa );
	    if( SHELL_PUTS_INDENT( msg, 30 )) return;

	    /* Cache Algorithm, even */
	    sprintf( msg, "%d", (data[2] & M_EntryLoC) >> S_EntryLoC );
	    if( SHELL_PUTS_INDENT( msg, 41 )) return;

	    /* Dirty setting, even */
	    sprintf( msg, "%c", 
		     (data[2] & M_EntryLoD) >> S_EntryLoD ?
		         'y' : 'n' );
	    if( SHELL_PUTS_INDENT( msg, 44 )) return;

	    /* Valid setting, even */
	    sprintf( msg, "%c", 
		     (data[2] & M_EntryLoV) >> S_EntryLoV ?
		         'y' : 'n' );
	    if( SHELL_PUTS_INDENT( msg, 47 )) return;

	    /* Physical address, odd */
	    pa  = ( (data[3] & M_EntryLoPFN) >> S_EntryLoPFN ) << (smallpages ? 10 : 12);
	    pa &= ~(pagesize - 1);
	    sprintf( msg, "0x%08x", pa );
	    if( SHELL_PUTS_INDENT( msg, 50 )) return;

	    /* Cache Algorithm, odd */
	    sprintf( msg, "%d", (data[3] & M_EntryLoC) >> S_EntryLoC );
	    if( SHELL_PUTS_INDENT( msg, 61 )) return;

	    /* Dirty setting, odd */
	    sprintf( msg, "%c", 
		     (data[3] & M_EntryLoD) >> S_EntryLoD ?
		         'y' : 'n' );
	    if( SHELL_PUTS_INDENT( msg, 64 )) return;

	    /* Valid setting, odd */
	    sprintf( msg, "%c", 
		     (data[3] & M_EntryLoV) >> S_EntryLoV ?
		         'y' : 'n' );
	    if( SHELL_PUTS_INDENT( msg, 67 )) return;

	    if( SHELL_PUTS( "\n" ) ) return;
        }	    

        if( SHELL_PUTS( "\n" ) ) return;
    }
    else
    {
        /**** TLB write ****/

	/* Index */
	data[0] = tlb_setup->index;

        /* Pagemask */
	data[1] = tlb_setup->pagemask;

	/* EntryHi */
	data[2]  = tlb_setup->vpn | (tlb_setup->asid << S_EntryHiASID);

	/* EntryLo0 */
	data[3] =  (tlb_setup->pfn[0]		  << S_EntryLoPFN) |
		   (tlb_setup->cache[0]		  << S_EntryLoC)	  |
		   ((tlb_setup->dirty[0] ? 1 : 0) << S_EntryLoD)	  |
		   ((tlb_setup->valid[0] ? 1 : 0) << S_EntryLoV)	  |
		   ((tlb_setup->global   ? 1 : 0) << S_EntryLoG);

 	/* EntryLo1 */
	data[4] =  (tlb_setup->pfn[1]		  << S_EntryLoPFN) |
		   (tlb_setup->cache[1]		  << S_EntryLoC)	  |
		   ((tlb_setup->dirty[1] ? 1 : 0) << S_EntryLoD)	  |
		   ((tlb_setup->valid[1] ? 1 : 0) << S_EntryLoV)	  |
		   ((tlb_setup->global   ? 1 : 0) << S_EntryLoG);

	sys_tlb_write( data );
    }
}

/* Command definition for dump */
static t_cmd cmd_def =
{
    "tlb",
    tlb,

    "tlb ( -i [-s] ) | ( <index> <pagesize> <va> <g> <asid>\n"
    "                    <pa0> <c0> <d0> <v0>\n"
    "                    <pa1> <c1> <d1> <v1> )",

    "Display or edit TLB.\n"
    "\n"
    "Some CPUs (MIPS32/MIPS64 Release 2 or later only) may support\n"
    "small pages (1kB).\n"
    "The '-s' option is only available when running on such a CPU.\n"
    "'-s' is used to toggle between enabling and disabling small pages.\n"
    "It may only be used in conjunction with '-i' since toggling this state\n"
    "requires the TLB to be reinitialised.\n"
    "\n"
    "In case there are no parameters, the contents of the TLB is displayed.\n"
    "If small pages are available, the state of this feature (enabled/disabled)\n"
    "is also displayed.\n"
    "\n"
    "In case (all) parameters are available, the TLB entry at the requested\n"
    "index is written.\n"
    "\n"
    "The number of TLB entries is CPU specific.\n"
    "\n"
    "Available settings of 'pagesize' are :\n"

    "  0x400     |   1kB (if small pages are enabled)\n"
    "  0x1000    |   4kB\n"
    "  0x4000    |  16kB\n"
    "  0x10000   |  64kB\n"
    "  0x40000   | 256kB\n"
    "  0x100000  |   1MB\n"
    "  0x400000  |   4MB\n"
    "  0x1000000 |  16MB\n"
    "\n"    
    "Available settings of c0/c1 (cache algorithm for even/odd page) are\n"
    "processor specific. However, the values 2 and 3 are typically reserved\n"
    "for Uncached (2) and Cacheable (3) modes. Values 0..7 are available.\n"
    "\n"
    "Other parameters are :\n"
    "  va     : Virtual base address of even/odd pair of pages.\n"
    "           The va specified is the one used for the even page, so\n"
    "           it must be aligned to pagesize * 2.\n"
    "  g      : GLOBAL setting ('n' -> ASID is used, 'y' -> Ignore ASID).\n"
    "  asid   : ASID setting (only relevant if g = 'n').\n"
    "  pa0    : Physical base address of even page.\n"
    "           Must be aligned to pagesize.\n"
    "  d0     : DIRTY setting of even page ('y' -> write enabled,\n"
    "                                       'n' -> write protected).\n"
    "  v0     : VALID setting of even page ('y' -> valid,\n"
    "                                       'n' -> not valid).\n"
    "  pa1    : Physical base address of odd page.\n"
    "           Must be aligned to pagesize.\n"
    "  d1     : DIRTY setting of odd page  ('y' -> write enabled,\n"
    "                                       'n' -> write protected).\n"
    "  v1     : VALID setting of odd page  ('y' -> valid,\n"
    "                                       'n' -> not valid).\n"
    "\n"
    "Example :\n"
    "\n"
    "   TLB index       = 2\n"
    "   Pagesize        = 4kB\n"
    "   Global mapping (i.e. ASID ignored)\n"
    "   ASID            = 0xff (but ignored)\n"
    "   Cache algorithm = 3 (Cacheable)\n"
    "   Both pages valid\n"
    "\n"
    "   Virtual address  Physical address  Dirty (i.e. write enabled)\n"
    "   -------------------------------------------------------------\n"
    "   0x00000000       0x00200000        Yes\n"
    "   0x00001000       0x00300000        No\n"
    "\n"
    "tlb 2 4kB 0 y ff 200000 3 y y 300000 3 n y",

    options,
    0,
    FALSE
};

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_tlb_init
 *  Description :
 *  -------------
 *
 *  Initialise command
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
t_cmd *
shell_tlb_init( void )
{
    if( !sys_smallpage_support )
        smallpages = FALSE;

    option_count = 
        sys_smallpage_support ?
	    OPTION_COUNT_REL2 : OPTION_COUNT_NOT_REL2;

    cmd_def.option_count = option_count;

    return &cmd_def;
}



