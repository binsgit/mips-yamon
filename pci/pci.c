/************************************************************************
 *
 *  pci.c
 *
 *  Generic PCI code
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
#include <syserror.h>
#include <pci.h>
#include <pci_api.h>
#include <product.h>
#include <atlas.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <shell_api.h>
#include <string.h>
#include <stdio.h>

/************************************************************************
 *  Definitions
 ************************************************************************/


/* Determine whether devices is of type 0 or 1 based on header type */
#define DEV_PPB(dev)		( (dev)->ht == PCI_HEADERTYPE1 )


/* PCI bus definition */

typedef struct
{
    bool       fastb2b;
    t_pci_bar  *mem_largest_ptr;
    t_pci_bar  *io_largest_ptr;
    UINT32     start_mem;
    UINT32     start_io;
    bool       prefetch;
}
t_pci_cfg_bus;


/* Determine whether device IDs equal device data */
#define DEV_EQUAL( dev, vendor, device, func )		\
	(						\
	    (dev->vendorid == vendor) &&		\
	    (dev->devid    == device) &&		\
	    (dev->function == func)			\
	)

/* Determine whether a device is known to YAMON */
#define KNOWN_DEV( dev, known_dev )			\
	    DEV_EQUAL( dev, known_dev.vendorid, known_dev.devid, known_dev.function )

#define KNOWN_BAR( dev, pos, bar_req )		\
	    (KNOWN_DEV( dev, bar_req ) &&	\
	    (pos == bar_req.bar.pos))

/* Search for known device */
#define SEARCH_KNOWN( dev, loopvar )			\
	for( loopvar=0;					\
	       (loopvar < known_devs_count) &&		\
	       !KNOWN_DEV( dev, known_devs[loopvar] );	\
	     loopvar++ )				\
	{						\
	   ;						\
	}
	    
    
/************************************************************************
 *  Public variables
 ************************************************************************/

bool pci_alloc_err;

/************************************************************************
 *  Static variables
 ************************************************************************/

/* Variables for device and bus data */
static t_pci_cfg_dev  dev_data[PCI_MAX_DEV];
static t_pci_cfg_bus  bus_data[PCI_MAX_BUS];
static UINT32         dev_count;
static UINT8	      bus_count;

/* Variables for holding parameters used by several functions */
static t_known_dev    *known_devs;
static UINT32         known_devs_count;
static t_pci_bar_req  *bar_reqs;
static UINT32	      bar_reqs_count;

/*  Offsets :
 *    The system controller supports remapping of memory
 *    and IO ranges so that the addresses seen on the CPU side (SysAD) of
 *    the controller and the addresses seen on PCI are offset by some amount.
 *    The CPU side start address is found by adding the offset to the
 *    PCI side start address.
 */

static UINT32         pci_mem_offset;
static UINT32         pci_io_offset;

/* Error strings */
static char* error_strings[] =
{
    /* ERROR_PCI_MALLOC		*/ "Not enough space for autoconfiguration",
    /* ERROR_PCI_RANGE		*/ "Illegal ram range",
    /* ERROR_PCI_UNKNOWN_DEVICE */ "Unknown PCI device",
    /* ERROR_PCI_ABORT		*/ "PCI master or target abort",
    /* ERROR_PCI_STRUCTURE	*/ "Structural error in PCI module"
};


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32
detect_devices(
    UINT32 maxbus,
    UINT32 maxdev );

static UINT32
config_system( 
    UINT32 freemem_start,		/* First valid mem address	*/
    UINT32 freemem_end, 		/* First non valid mem address	*/
    UINT32 freeio_start,		/* First valid IO address	*/
    UINT32 freeio_end,			/* First non valid IO address	*/
    UINT32 maxbus );

static UINT32
config_dev( 
    t_pci_cfg_dev   *dev,		/* Device data			*/
    UINT32	    memlimit,		/* Max valid mem address	*/
    UINT32	    iolimit );		/* Max valid io address		*/

static UINT32
query_dev( 
    UINT32        bus,                  /* Bus number                   */
    UINT32        devnum,		/* Device number		*/
    UINT32	  function,		/* Device function		*/
    t_pci_cfg_dev *dev,			/* Device data			*/
    UINT32	  maxbus );

static bool
fixed_memory_alloc(
    t_pci_cfg_dev *dev,
    UINT32        bar_index );

static void
setup_fixed_memory_alloc( 
    t_pci_cfg_dev *dev );

static UINT32
alloc_space(
    UINT32 red_start,	/* redpage, 0 -> IO space, !=0 -> memory space	*/
    UINT32 red_end,
    UINT32 mem_start,
    UINT32 mem_end );

static UINT32
align(
    UINT32    addr,			/* Address to be aligned	*/
    UINT32    alignment );		/* Alignment requirement	*/

static INT32 
error_lookup( 
    t_sys_error_string *param );


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          pci_init
 *  Description :
 *  -------------
 *
 *  PCI module initialisation function
 *
 *  Return values :
 *  ---------------
 *
 *  OK if no error, else error code
 *
 ************************************************************************/
UINT32
pci_init( void )
{
    t_sys_error_lookup_registration registration;

    /* register error lookup function */
    registration.prefix = SYSERROR_DOMAIN( ERROR_PCI );
    registration.lookup = error_lookup;

    SYSCON_write( SYSCON_ERROR_REGISTER_LOOKUP_ID,
                  &registration,
                  sizeof( registration ) );

    /* Call platform specific configuration function */
    return pci_config();
}    


/************************************************************************
 *
 *                          pci_autoconfig
 *  Description :
 *  -------------
 *
 *  Configure PCI based on platform requirements.
 *
 *  Return values :
 *  ---------------
 *
 *     0: no pci error, else error
 *
 ************************************************************************/
UINT32
pci_autoconfig(
    t_pci_bar_req  *bar_req,		/* Special BAR requirements	*/
    UINT32	   bar_req_count,	/* Number of special BARs       */

    t_known_dev    *known_dev,		/* Array of known devices	*/
    UINT32	   known_dev_count )	/* Count of known devices	*/
{
    UINT32 rc, i;
    UINT32	  mem_start;	/* Start of PCI memory range		*/
    UINT32	  mem_size;	/* Size of PCI memory range		*/
    UINT32	  io_start;	/* Start of PCI io range		*/
    UINT32	  io_size;	/* Size of PCI io range			*/
    UINT32 maxbus, maxdev;

    /* Store data used by other functions */
    known_devs       = known_dev;
    known_devs_count = known_dev_count;
    bar_reqs	     = bar_req;
    bar_reqs_count   = bar_req_count;

    /*  Determine ranges :
     *
     *  1) Start of range as seen from PCI.
     *  2) Size of range.
     *  3) Offset of range. The system controller supports remapping of memory
     *     and IO ranges so that the addresses seen on the CPU side (SysAD) of
     *     the controller and the addresses seen on PCI are offset by some amount.
     *     The CPU side start address is found by adding the offset to the
     *     PCI side start address.
     */

    SYSCON_read( SYSCON_CORE_PCI_MEM_START, 
		 (void *)&mem_start, sizeof(UINT32) );

    SYSCON_read( SYSCON_CORE_PCI_MEM_SIZE,
		 (void *)&mem_size, sizeof(UINT32) );

    SYSCON_read( SYSCON_CORE_PCI_MEM_OFFSET, 
		 (void *)&pci_mem_offset, sizeof(UINT32) );

    SYSCON_read( SYSCON_CORE_PCI_IO_START, 
		 (void *)&io_start, sizeof(UINT32) );

    SYSCON_read( SYSCON_CORE_PCI_IO_SIZE,
		 (void *)&io_size, sizeof(UINT32) );

    SYSCON_read( SYSCON_CORE_PCI_IO_OFFSET,
		 (void *)&pci_io_offset, sizeof(UINT32) );


    /* Determine max number of busses and devices */
    maxbus = PCI_MAX_BUS;		/* Max number of PCI busses	*/
    maxdev = PCI_MAX_DEV;		/* Max number of PCI devices    */

    /* Detect devices */
    rc = detect_devices( maxbus, maxdev );
    if( rc != OK ) return rc;

    /* Configure system */
    rc = config_system( mem_start, mem_start + mem_size, 
			io_start,  io_start  + io_size,
			maxbus );
    if( rc != OK ) return rc;

    /* Configure devices */
    for(i=0; i<dev_count; i++)
    {
        rc = config_dev( &dev_data[i], 
			 mem_start + mem_size - 1,
			 io_start   + io_size - 1 );

        if( rc != OK ) return rc;
    }
    
    return OK;
}


/************************************************************************
 *
 *                          pci_config_write32/16/8
 *  Description :
 *  -------------
 *
 *  Low level pci configuration space write routines (32/16/8 bit)
 *
 *  Return values :
 *  ---------------
 *
 *  0 (No error) or ERROR_PCI_ABORT    
 *
 ************************************************************************/
UINT32 
pci_config_write32( 
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT32 data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   TRUE, sizeof(UINT32), (void *)&data );
}

UINT32 
pci_config_write16( 
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT16 data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   TRUE, sizeof(UINT16), (void *)&data );
}

UINT32 
pci_config_write8( 
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT8  data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   TRUE, sizeof(UINT8), (void *)&data );
}


/************************************************************************
 *
 *                          pci_config_read32/16/8
 *  Description :
 *  -------------
 *
 *  Low level pci configuration space read routines (32/16/8 bit)
 *
 *  Return values :
 *  ---------------
 *
 *  0 (No error) or ERROR_PCI_ABORT    
 *
 ************************************************************************/
UINT32
pci_config_read32(  
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT32 *data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   FALSE, sizeof(UINT32), (void *)data );
}

UINT32
pci_config_read16(  
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT16 *data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   FALSE, sizeof(UINT16), (void *)data );
}

UINT32
pci_config_read8(  
    UINT32 busnum,
    UINT32 devnum,
    UINT32 func,
    UINT32 reg,
    UINT8  *data )
{
    return arch_pci_config_access( busnum, devnum, func, reg,
				   FALSE, sizeof(UINT8), (void *)data );
}


/************************************************************************
 *
 *                          pci_lookup_bar
 *  Description :
 *  -------------
 *
 *  Determine base address (physical) of specific BAR of specific device.
 *
 *  Return values :
 *  ---------------
 *
 *  TRUE  : BAR found
 *  FALSE : BAR not found
 *
 ************************************************************************/
bool
pci_lookup_bar(
    UINT8         busnum,		/* Bus number			*/
    UINT8         devnum,		/* Device number		*/
    UINT8	  func,			/* Function number		*/
    UINT8	  bar,			/* Address of BAR register	*/
    void	  **param )		/* OUT : Base address of BAR	*/
{
    t_pci_cfg_dev *dev;
    UINT32	  i;

    /* Default */
    *param = NULL;

    for( i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];
	if (dev->bus == busnum && dev->dev == devnum && dev->function == func)
	    break;
    }
    if( i<dev_count )
    {
        for( i=0; i<dev->bar_count; i++ )
	{
	    if( dev->bar[i].pos == bar )
	    {
                *param = 
		    (void *) ( dev->bar[i].start +
			       (dev->bar[i].io ?
			           pci_io_offset :
			           pci_mem_offset) );
                return TRUE;
	    }
	}
    }

    return FALSE;
}


/************************************************************************
 *
 *                          pci_display
 *  Description :
 *  -------------
 *
 *  Display devices detected during autodetect and display the 
 *  autoconfiguration data (memory space and IO space allocations)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
pci_display(void)
{
    t_pci_cfg_dev *dev;
    t_pci_bar     *bar;
    UINT32	  i, t;
    char	  msg[200];
    UINT32	  freq_kHz;
    char	  *known_vendor, *known_device;
    UINT8	  slotnum;

    /* Determine bus frequency */
    SYSCON_read(
        SYSCON_BOARD_PCI_FREQ_KHZ_ID,
	(void *)&freq_kHz,
	sizeof(UINT32) );

    strcpy( msg , "PCI bus frequency = " );

    pci_busfreq_string( &msg[strlen(msg)], freq_kHz );

    if( SHELL_PUTS( msg ) ) return;
    
    if( SHELL_PUTS( "\n\nPCI devices:\n" ) ) return;

    sprintf( msg, "Bus count = %d, Device count = %d\n",
	          bus_count, dev_count );

    if( SHELL_PUTS( msg ) ) return;

    for( i=0; i<dev_count; i++ )
    {
	if( SHELL_PUTC( '\n' ) )
		return;

        dev = &dev_data[i];

	SEARCH_KNOWN( dev, t )

        if( t != known_devs_count )
	{
	    known_vendor = known_devs[t].vendor;
	    known_device = known_devs[t].device;
	}
	else
	{
	    known_vendor = NULL;
	    known_device = NULL;
	}

	sprintf( msg, "Bus = 0x%02x, Dev = 0x%02x", dev->bus, dev->dev );
        if(SHELL_PUTS( msg ) ) return;

	if( (dev->bus == PCI_BUS_LOCAL) && 
	    arch_pci_slot( dev->dev, &slotnum ) )
	{
	    sprintf( msg, " (PCI slot %hd)", slotnum );
	    if(SHELL_PUTS( msg )) return;
	}

	sprintf( msg, ", Function = 0x%02x", dev->function );
        if(SHELL_PUTS( msg ) ) return;

	if( dev->alloc_err )
	{
	    if(SHELL_PUTS( "  (NOT CONFIGURED)" ) ) return;
	}

	sprintf( msg, "\nVendor Id = 0x%04x", dev->vendorid );
        if(SHELL_PUTS( msg ) ) return;

	if( known_vendor )
	{
	    sprintf( msg, " (%s), Dev ID = 0x%04x (%s)\n", 
		     known_vendor, dev->devid, known_device );
	}		     		      
	else
	{
	    sprintf( msg, ", Dev ID = 0x%04x\n", dev->devid );
	}

        if(SHELL_PUTS( msg )) return;

	sprintf( msg, " Min Gnt = 0x%02x, Max Lat = 0x%02x, Lat Tim = 0x%02x\n",
		 dev->min_gnt, dev->max_lat, dev->lat_tim ); 
        if( SHELL_PUTS( msg ) ) return;
	
	sprintf( msg, " Int Pin = " );

	switch( dev->intpin )
	{
	  case PCI_MMII_INTPIN_A :
	    strcat( msg, "A" ); break;
	  case PCI_MMII_INTPIN_B :
	    strcat( msg, "B" ); break;
	  case PCI_MMII_INTPIN_C :
	    strcat( msg, "C" ); break;
	  case PCI_MMII_INTPIN_D :
	    strcat( msg, "D" ); break;
	  default :
	    strcat( msg, "None" ); break;
	}
        if( SHELL_PUTS( msg ) ) return;

	if( dev->intline == PCI_MMII_INTLINE_NONE )
	{
	    strcpy( msg, ", Int Line = None\n" );
	}
	else
	{
	    sprintf( msg, ", Int Line = 0x%02x\n", dev->intline );
	}

        if( SHELL_PUTS( msg ) ) return;

	sprintf( msg, " BAR count = 0x%02x\n", dev->bar_count );
	if( SHELL_PUTS( msg ) ) return;

	for( t=0; t<dev->bar_count; t++ )
	{
	    bar = &dev->bar[t];

	    if(SHELL_PUTS( ( bar->io == 1 ) ? "  IO: " : "  MEM:" )) return;

	    sprintf( msg, " Pos = 0x%02x", bar->pos );
	    if( SHELL_PUTS( msg ) ) return;
	    if (!dev->alloc_err)
	    {
		sprintf( msg, ", Base(CPU/PCI) = 0x%08x/0x%08x",
		     bar->start + (bar->io ? pci_io_offset : pci_mem_offset),
		     bar->start );
		if( SHELL_PUTS( msg ) ) return;
	    }
	    sprintf( msg, ", Size = 0x%08x\n", bar->size );
	    if( SHELL_PUTS( msg ) ) return;
	}
    }
}


/************************************************************************
 *
 *                          pci_busfreq_string
 *  Description :
 *  -------------
 *
 *  Format string containing PCI bus freqency in format xx.yy MHz
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
pci_busfreq_string(
    char   *msg,
    UINT32 freq_kHz )
{
    UINT32 freq_MHz = freq_kHz/1000;

    sprintf( msg, "%d MHz", freq_MHz );
    
    /* Calc remaining kHz */
    freq_kHz -= freq_MHz*1000;
    
    if( freq_kHz )
    {
        /* Adjust accuracy (e.g. 33330 kHz => 33.33 MHz, not 33.330 MHz) */
        while( (freq_kHz % 10) == 0 )
            freq_kHz /= 10;

	sprintf( &msg[strlen(msg)-4], ".%d MHz", freq_kHz );
    }
}


/************************************************************************
 *
 *                          pci_check_range
 *  Description :
 *  -------------
 *
 *  Validate PCI memory range
 *
 *  Return values :
 *  ---------------
 *
 *  OK    	    : Range OK
 *  ERROR_PCI_RANGE : Range not OK
 *
 ************************************************************************/
UINT32
pci_check_range( 
    UINT32 addr,			/* Start of range		*/
    UINT32 size,			/* Size of range		*/
    UINT32 *mask )			/* Mask corresponding to range	*/
{
    UINT32 n;

    /* Calc mask for bank size register ('1' at all don't care positions) */
    *mask = size-1;

    /* Check that address is naturally aligned */
    if( addr != (addr & ~(*mask)) )
        return ERROR_PCI_RANGE;

    return OK;
}


/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/


/************************************************************************
 *                          detect_devices
 ************************************************************************/
static UINT32
detect_devices( 
    UINT32 maxbus,
    UINT32 maxdev )
{
    UINT32 bus, dev, func;
    UINT32 rc;
    
    dev_count = 0;
    bus_count = 1;	/* The local bus */

    for( bus = 0; (bus < bus_count) && (bus < maxbus); bus++ )
    {
        for( dev = 0; dev <= 31; dev++ )
        {
	    /* Read function 0 */
	    rc = query_dev( bus, dev, 0, &dev_data[dev_count], maxbus );

	    if( rc == OK )
	    {
	        /* Found device */
	        dev_count++;

		if( dev_count == maxdev )
		    return OK;

	        if( dev_data[dev_count-1].multi )
		{
		    /* Multi function device */
		    for( func = 1; func < PCI_MAX_FUNC; func++ )
		    {
		        rc = query_dev( bus, dev, func, &dev_data[dev_count], maxbus );
			if( rc == OK )
			    dev_count++;

			if( dev_count == maxdev )
			    return OK;
		    }
		}
	    }
	}
    }

    return OK;
}


/************************************************************************
 *                          query_dev
 ************************************************************************/
static UINT32
query_dev( 
    UINT32        bus,                  /* Bus number                   */
    UINT32        devnum,		/* Device number		*/
    UINT32	  function,		/* Device function		*/
    t_pci_cfg_dev *dev,			/* Device data			*/
    UINT32	  maxbus )
{
    UINT32    data;
    t_pci_bar *bar;
    UINT32    pci_bar_max;
    UINT32    rc;
    UINT32    i;
    UINT8     type;

    /* Read configuration data common to all PCI devices */

    rc = pci_config_read32( bus, devnum, function, PCI_ID, &data );
    if( rc != OK ) return rc;
    if(data == 0xffffffff) return ERROR_PCI_ABORT;
    
    dev->devid    = (data & PCI_ID_DEVID_MSK)    >> PCI_ID_DEVID_SHF;
    dev->vendorid = (data & PCI_ID_VENDORID_MSK) >> PCI_ID_VENDORID_SHF;    

    rc = pci_config_read32( bus, devnum, function, PCI_SC, &data );
    if( rc != OK ) return rc;
    dev->status  = (data & PCI_SC_STATUS_MSK)  >> PCI_SC_STATUS_SHF;

    rc = pci_config_read32( bus, devnum, function, PCI_BHLC, &data );
    if( rc != OK ) return rc;
    dev->ht    = (data & PCI_BHLC_HT_MSK)    >> PCI_BHLC_HT_SHF;

    if( dev->ht > PCI_HEADERTYPE_MAX )
    {
        /* Unknown header type, ignore device */
        return ERROR_PCI_ABORT;
    }

    dev->multi = arch_pci_multi( dev, data );

    rc = pci_config_read32( bus, devnum, function, PCI_MMII, &data );
    if( rc != OK ) return rc;
    dev->intpin  = (data & PCI_MMII_INTPIN_MSK) >> PCI_MMII_INTPIN_SHF;
    /* max_lat and min_gnt actually only valid for header type 0 dev */
    dev->max_lat = (data & PCI_MMII_MAXLAT_MSK) >> PCI_MMII_MAXLAT_SHF;
    dev->min_gnt = (data & PCI_MMII_MINGNT_MSK) >> PCI_MMII_MINGNT_SHF;

    /* Read BARs */
    dev->bar_count = 0;

    pci_bar_max = DEV_PPB(dev) ? PCI_BAR_MAX_PPB : PCI_BAR_MAX;

    for(i = PCI_BAR_MIN; i <= pci_bar_max; i+= sizeof(UINT32) )
    {
	rc = pci_config_write32( bus, devnum, function, i, 0xFFFFFFFF );
        if( rc != OK )
            return rc;
	rc = pci_config_read32( bus, devnum, function, i, &data );
        if( rc != OK ) 
            return rc;	    

	if( data != 0 )
	{
	    bar = &dev->bar[dev->bar_count];
		
	    bar->pos = i;	
	    bar->io  = (data & PCI_BAR_IO_MSK) >> PCI_BAR_IO_SHF;

	    if(bar->io)
	    {
	        bar->size  = data & (1 + (~data | ~PCI_BAR_IOSIZE_MSK));
		dev->bar_count++;
	    }
	    else
	    {
		type = (data & PCI_BAR_TYPE_MSK) >> PCI_BAR_TYPE_SHF;

		if( type == PCI_BAR_TYPE_64 )
		{
		    /*  64 bit addressing. Ignore bar and increase
		     *  loop counter since bar field is 2 words 
		     */
			 
		    i += sizeof(UINT32);
		}
		else if( type != PCI_BAR_TYPE_RSVD )
		{
	            bar->size  = data & (1 + (~data | ~PCI_BAR_MEMSIZE_MSK));
		    bar->prefetch = (data & PCI_BAR_PREFETCH_MSK) >> 
		   	             PCI_BAR_PREFETCH_SHF;

		    dev->bar_count++;
		}
	    }
        }
    }

    dev->bus      = bus;
    dev->dev      = devnum;
    dev->function = function;

    if( DEV_PPB(dev) )
    {
        /* PCI-PCI Bridge */

	if( sys_platform == PRODUCT_ATLASA_ID && 
	    bus == PCI_BUS_LOCAL &&
	    devnum == PCI_IDSEL2DEVNUM(ATLAS_IDSEL_21150) &&
	    !arch_pci_system_slot() )
	{
	    /* The CompactPCI bridge on Atlas is NOT connected to back plane */
	    bus_count++;
	    return OK;
	}
	/* Configure bus numbers */
	data = (bus         << PCI_SSSP_PBN_SHF)    |
	       (bus_count++ << PCI_SSSP_SECBN_SHF ) |
	       ((maxbus-1)  << PCI_SSSP_SUBBN_SHF);
        rc = pci_config_write32( bus, devnum, function, PCI_SSSP, data );
        if( rc != OK ) return rc;	    

	if( bus == PCI_BUS_LOCAL &&
	    devnum == PCI_IDSEL2DEVNUM(ATLAS_IDSEL_21150) &&
	    arch_pci_system_slot() )
	{   
	    /* Enable backplane clocks (TBD : Use #define for 0x68) */
	    rc = pci_config_write32( bus, devnum, function, 0x68, 0 ); 
	    if( rc != OK ) return rc;	    

	    /* Pause for a while to allow target devices on PCI 
	     * backplane to boot.
	     * This is necessary when interfacing to the RIO
	     * board, which uses an i960 as PCI South bridge since
	     * the i960 needs to boot.
	     */
	    DISP_STR( "PCI WAIT" );
	    sys_wait_ms( 1000 );
	    DISP_STR( "PCI" );
	}
    }

    /* Everything OK */
    return OK;
}


/************************************************************************
 *                          config_system
 ************************************************************************/
static UINT32
config_system( 
    UINT32 freemem_start,		/* First valid mem address	*/
    UINT32 freemem_end, 		/* First non valid mem address	*/
    UINT32 freeio_start,		/* First valid IO address	*/
    UINT32 freeio_end,			/* First non valid IO address	*/
    UINT32 maxbus )
{
    t_pci_bar     **pptr;
    t_pci_bar	  *bar;
    t_pci_cfg_dev *dev;
    t_pci_cfg_bus *bus;
    UINT32	  rc;
    UINT32	  i,t;
    UINT8	  slotnum;

    /* Default bus settings */
    for( i=0; i<maxbus; i++ )
    {	 
        bus = &bus_data[i];

        bus->fastb2b	     = TRUE;
	bus->mem_largest_ptr = NULL;
	bus->io_largest_ptr  = NULL;
	bus->prefetch	     = (i == PCI_BUS_LOCAL) ? FALSE : TRUE;
    }

    /*  There are two types of BARs : 
     *
     *  1) Configurable (normal) BARs.
     *  2) Fixed BARS, ie BARs with fixed requirements for memory mapping.
     */

    /* Sort configurable BARs for each bus based on memory requirements */
    for( i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];
	bus = &bus_data[dev->bus];

	for( t=0; t<dev->bar_count; t++ )
        {	
	    if( !fixed_memory_alloc( dev, t ) )
	    {
	        /* Normal configurable BAR */
	        bar        = &dev->bar[t];
	        bar->fixed = FALSE;

	        /* Insert in ordered linked list */
	        pptr = bar->io ?
		           &bus->io_largest_ptr :
		           &bus->mem_largest_ptr;

                while( (*pptr != NULL) && ((*pptr)->size > bar->size) )
	            pptr = &((*pptr)->next);

	        bar->next = *pptr;
	        *pptr     = bar;
	    }
	}
    }

    /* Add fixed BARs to front of list */
    for( i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];
	bus = &bus_data[dev->bus];

        /* Fixed BARs */
        setup_fixed_memory_alloc( dev );

	for( t=0; t<dev->bar_count; t++ )
	{
	    if( fixed_memory_alloc( dev, t ) )
	    {
	        bar        = &dev->bar[t];
	        bar->fixed = TRUE;

	        /* Insert at front of list */
	        pptr = bar->io ?
		   &bus->io_largest_ptr :
		   &bus->mem_largest_ptr;
			   
	        bar->next = *pptr;
	        *pptr     = bar;
	    }
	}
    }

    /* BAR impact on bus prefetch */
    for(i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];
	bus = &bus_data[dev->bus];

	for(t=0; t<dev->bar_count; t++)
	{
	    bar = &dev->bar[t];

            if(!bar->prefetch)
	        bus->prefetch = FALSE;
	}
    }

    /* Alloc memory address space */
    rc = alloc_space( freeio_start+pci_io_offset, freeio_end+pci_io_offset, freemem_start, freemem_end );
    if( rc != OK ) return rc;

    /* Alloc I/O address space    */
    rc = alloc_space( 0,0, freeio_start, freeio_end );
    if( rc != OK ) return rc;

    /* Device impacts on bus settings and store offsets */
    for( i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];

	/* Determine device impact on bus fastb2b */
	if( !(dev->status & PCI_STATUS_FBB_BIT))
	{
	    bus_data[dev->bus].fastb2b = FALSE;
	}
    }

    /* Device private settings */
    for( i=0; i<dev_count; i++ )
    {
        dev = &dev_data[i];

	dev->lat_tim = arch_pci_lattim( dev );

        /* Determine interrupt line */
	if( dev->bus == PCI_BUS_LOCAL )
	{
	    /* Is it a PCI slot */
	    if( arch_pci_slot( dev->dev, &slotnum ) )
	    {
	        dev->intline = arch_pci_slot_intline( dev->dev, dev->intpin );
	    }
	    else
	    {
	        SEARCH_KNOWN( dev, t )

	        if( t == known_devs_count )
		{
	            return ERROR_PCI_UNKNOWN_DEVICE;
		}
	        else
                    dev->intline = known_devs[t].intline;
            }
	}
	else
	{
	    dev->intline = arch_pci_remote_intline( dev->intpin );
	}
    }

    return OK;
}

    
/************************************************************************
 *                          fixed_memory_alloc
 ************************************************************************/
static bool
fixed_memory_alloc(
    t_pci_cfg_dev *dev,
    UINT32        bar_index )
{
    /* Determine whether this BAR has fixed memory mapping requirements */
    UINT32 i;
    UINT32 pos;

    pos = dev->bar[bar_index].pos;

    for( i=0; i<bar_reqs_count; i++ )
    {
        if( KNOWN_BAR( dev, pos, bar_reqs[i] ) )
	{
	    return TRUE;
	}
    }

    return FALSE;
}


/************************************************************************
 *                          setup_fixed_memory_alloc
 ************************************************************************/
static void
setup_fixed_memory_alloc( 
    t_pci_cfg_dev *dev )
{
    UINT32 i, t;

    /* Find all BAR requests for device */
    for( i=0; i<bar_reqs_count; i++ )
    {
        if( KNOWN_DEV( dev, bar_reqs[i] ) )
	{
	    /*  Found a BAR request.
	     *  Now find BAR location in device data.
	     */
	     
	    for( t=0; 
		     (t < dev->bar_count) &&
	             (dev->bar[t].pos != bar_reqs[i].bar.pos);
		 t++ )
            {
	        ;
            }

	    if( t == dev->bar_count )
	    {
	        /*  Didn't find BAR (device did not report it).
		 *  Increase bar count so BAR may be setup.
		 */
	        dev->bar_count++;
	    }

            /*  Copy fixed memory allocation requirements from requirements 
	     *  array to the BAR in question.
	     */
	    memcpy( &dev->bar[t],
		    &bar_reqs[i].bar,
		    sizeof(t_pci_bar) );
	}
    }
}


/************************************************************************
 *                          config_dev
 ************************************************************************/
static UINT32
config_dev( 
    t_pci_cfg_dev   *dev,		/* Device data			*/
    UINT32	    memlimit,		/* Max valid mem address	*/
    UINT32	    iolimit )		/* Max valid io address		*/
{
    UINT32 data;
    UINT32 rc;
    UINT32 i;

    /* clear COMMAND field - some devices lock their BARs otherwise */

    if( dev->bus != PCI_BUS_LOCAL &&
        dev->dev != PCI_IDSEL2DEVNUM(ATLAS_IDSEL_CORE) )
    {
         pci_config_write32( dev->bus, dev->dev, dev->function, PCI_SC, 0);
    }


    /* Setup latency timer field */

    rc = pci_config_write8( dev->bus, dev->dev, dev->function,
			    PCI_LATTIM, dev->lat_tim );
    if( rc != OK ) return rc;


    /* Setup Interrupt Line field */

    rc = pci_config_write8( dev->bus, dev->dev, dev->function,
			    PCI_INTLINE, dev->intline );
    if( rc != OK ) return rc;


    /* Setup BARs */
    if (!dev->alloc_err)
    {
        for( i=0; i<dev->bar_count; i++ )
        { 
            t_pci_bar *bar;

            bar = &dev->bar[i];

	    rc = pci_config_write32( dev->bus, dev->dev, dev->function,
	                             bar->pos, bar->start );
	    if( rc != OK ) return rc;
        }
    }

    
    /* Setup power management (if available through extended capabilities) */
    if( dev->status & PCI_STATUS_CAP_BIT )
    {
        UINT8 cap_ptr, cap_id;

        /* Determine capability pointer */
	rc = pci_config_read8( dev->bus, dev->dev, dev->function,
			       PCI_CAP_PTR, &cap_ptr );
        if( rc != OK ) return rc;

	while( (cap_ptr != 0)         && 
	       ((cap_ptr & 0x3) == 0) &&   /* Must be 32 bit aligned */
	       (cap_ptr >= PCI_FIRST_NON_STANDARD) )
	{
	    /* Read capability ID */
	    rc = pci_config_read8( dev->bus, dev->dev, dev->function,
			           cap_ptr + PCI_EXT_CAP_ID_OFS, &cap_id );
            if( rc != OK ) return rc;

	    if( cap_id == PCI_EXT_CAP_ID_PWR_MGMT )
	    {
	        /*  Found power management capability.
		 *  Set power state to D0.
		 */
	        rc = pci_config_write16( dev->bus, dev->dev, dev->function,
			                 cap_ptr + PCI_EXT_CAP_PWR_MGMT_STATUS_OFS,
			                 PCI_EXT_CAP_PWR_MGMT_STATUS_D0 );
                if( rc != OK ) return rc;
	    }

	    /* Next capability */
	    rc = pci_config_read8( dev->bus, dev->dev, dev->function,
			           cap_ptr + PCI_EXT_CAP_NEXT_PTR_OFS,
			           &cap_ptr );
            if( rc != OK ) return rc;
	}
    }

    if( DEV_PPB(dev) )
    {
        t_pci_cfg_bus *bus;

        /**** PCI-PCI Bridge ****/

	/* Secondary latency timer */
        rc = pci_config_write8( dev->bus, dev->dev, dev->function,
			        PCI_LATTIM, dev->lat_tim );
        if( rc != OK ) return rc;	    

	if( sys_platform == PRODUCT_ATLASA_ID && 
	    dev->bus == PCI_BUS_LOCAL &&
	    dev->dev == PCI_IDSEL2DEVNUM(ATLAS_IDSEL_21150) &&
	    !arch_pci_system_slot() )
	{
            rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				     PCI_IO, 0x0000FFF0 );
            if( rc != OK ) return rc;	    
            rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				     PCI_MEM, 0x0000FFF0 );
            if( rc != OK ) return rc;	    
            rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				     PCI_PREFMEM, 0x0000FFF0 );
	    return rc;
	}

	/* Secondary bus data */
	bus = &bus_data[dev->bus + 1];

	/* Secondary I/O base and limit (bit 15:12) */
	data =  ((bus->start_io & 0xF000) >> 8) << PCI_IO_BASE_SHF;
	data |= ((iolimit       & 0xF000) >> 8) << PCI_IO_LIMIT_SHF;

        rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				 PCI_IO, data );
        if( rc != OK ) return rc;	    
		
	/* I/O base and limit (bit 31:16) */
	data = ((bus->start_io >> 16) << PCI_UPPERIO_BASE_SHF) |
	       ((iolimit       >> 16) << PCI_UPPERIO_LIMIT_SHF);

        rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				 PCI_UPPERIO, data );
        if( rc != OK ) return rc;	    


        /* Secondary memory base and limit      */
	if( bus->prefetch )
	{
	    /* Disable non-prefetchable memory */
	    data = (0xFFF0 << PCI_MEM_BASE_SHF) |
		   (0x0000 << PCI_MEM_LIMIT_SHF);
	}
	else
	{
	    /* Setup non-prefetchable memory (bit 31:20) */
	    data =  ((bus->start_mem & 0xFFF00000) >> 16) << PCI_MEM_BASE_SHF;
	    data |= ((memlimit       & 0xFFF00000) >> 16) << PCI_MEM_LIMIT_SHF;	
        }
        rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				 PCI_MEM, data );
        if( rc != OK ) return rc;	    


	/* Secondary prefetchable memory base and limit */
	if( bus->prefetch )
	{
	    /* Setup prefetchable memory (bit 31:20) */
	    data =  ((bus->start_mem & 0xFFF00000) >> 16) << PCI_PREFMEM_BASE_SHF;
	    data |= ((memlimit       & 0xFFF00000) >> 16) << PCI_PREFMEM_LIMIT_SHF;	
	}
	else
	{
    	    /* Disable prefetchable memory */
	    data = (0xFFF0 << PCI_PREFMEM_BASE_SHF) |
		   (0x0000 << PCI_PREFMEM_LIMIT_SHF);
	}
        rc = pci_config_write32( dev->bus, dev->dev, dev->function,
				 PCI_PREFMEM, data );
        if( rc != OK ) return rc;	    


	/* Bridge control */

        rc = pci_config_write16( dev->bus, dev->dev, dev->function,
				 PCI_BC,
				   (PCI_BCII_BC_PERR_BIT |
				   PCI_BCII_BC_SERR_BIT |
				   PCI_BCII_BC_MA_BIT   |
				   (bus->fastb2b ? PCI_BCII_BC_FBB_BIT : 0))>>16 );
        if( rc != OK ) return rc;
    }

    /* Setup COMMAND field */

    data = PCI_SC_CMD_IOS_BIT  |
	   PCI_SC_CMD_MS_BIT   |
	   PCI_SC_CMD_BM_BIT   |
	   PCI_SC_CMD_PERR_BIT |
	   PCI_SC_CMD_SERR_BIT;

    if( bus_data[dev->bus].fastb2b )
        data |= PCI_SC_CMD_FBB_BIT;

    if (dev->alloc_err)
	data = 0;

    rc = pci_config_write32( dev->bus, dev->dev, dev->function,
			     PCI_SC, data );
    return rc;
}	      


/************************************************************************
 *                          alloc_space
 ************************************************************************/
static UINT32
alloc_space(
    UINT32 red_start,	/* redpage, 0 -> IO space, !=0 -> memory space	*/
    UINT32 red_end,
    UINT32 mem_start,
    UINT32 mem_end )
{
    UINT32	  i;
    t_pci_cfg_bus *bus;
    t_pci_bar	  *bar;
    t_pci_bar	  *firstconfbar;
    UINT32        firstconfmem;

    for( i=0; i < bus_count; i++ )
    {
	bus = &bus_data[i];

	if( red_start == 0 )
	{
            mem_start = align( mem_start, PCI_ALIGN_IO );

	    bar = bus->io_largest_ptr;

	    bus->start_io =
	        ( bar && bar->fixed ) ?
		    bar->start :
		    mem_start;
	}
	else
	{
            mem_start = align( mem_start, PCI_ALIGN_MEM );

	    bar = bus->mem_largest_ptr;

	    bus->start_mem =
	        ( bar && bar->fixed ) ?
		    bar->start :
		    mem_start;
	}

	/* First the fixed */
	while( bar && bar->fixed )
	{
	    mem_start = MAX( mem_start, bar->start + bar->size );

	    if( mem_start > mem_end )
	        return ERROR_PCI_MALLOC;

	    bar = bar->next;
	}

	/* Now the configurable */

 	firstconfbar = bar;
	firstconfmem = mem_start;

	for( ;bar; bar= bar->next )
	{
	    if (bar->start == 0x00000001)
		continue;

	    /* Alignment requirement for BAR */
	    mem_start  =  align( mem_start, bar->size );
	    if (mem_start == red_start)
		mem_start = align( red_end, bar->size );
	    if( mem_start + bar->size > mem_end )
	    {
		/* PCI ressource allocation trouble.
		 * Something has to be removed, so we remove the
		 * biggest configurable and start over again.
		 */
		t_pci_cfg_dev *dev;
		int d, t;

		for( d=0; d<dev_count; d++ )
		{
		    dev = &dev_data[d];
		    if (firstconfbar < &dev->bar[0]) continue;
		    if (firstconfbar >= &dev->bar[dev->bar_count]) continue;

		    for( t=0; t<dev->bar_count; t++ )
		    {
			dev->bar[t].start = 0x00000001;
		    }
		    dev->alloc_err = TRUE;
		    break;
		}
		pci_alloc_err = TRUE;

		/* start over again */
		bar = firstconfbar;
		mem_start  = firstconfmem;
		firstconfbar = bar->next;
	    }
	    else
	    {
		bar->start =  mem_start;
		mem_start  += bar->size;
	    }
        }
    }

    return OK;
}


/************************************************************************
 *                          align
 ************************************************************************/
static UINT32
align(
    UINT32    addr,			/* Address to be aligned	*/
    UINT32    alignment )		/* Alignment requirement	*/
{
    return (addr + alignment - 1) & ~(alignment - 1);
}


/************************************************************************
 *                          error_lookup
 ************************************************************************/
static INT32 
error_lookup( 
    t_sys_error_string *param )
{
    UINT32 index;

    index = SYSERROR_ID( param->syserror );

    if( index < sizeof(error_strings)/sizeof(char*) )
    {
        param->strings[SYSCON_ERRORMSG_IDX] = error_strings[index];
        param->count = 1;
    }
    else
        param->count = 0;

    return OK;
}
