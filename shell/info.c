
/************************************************************************
 *
 *  info.c
 *
 *  Info command for shell
 *
 *  info [board | cpu | ide | mem | pci | isa | lan | boot | all]
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
#include <syserror.h>
#include <shell_api.h>
#include <pci_api.h>
#include <isa_api.h>
#include <mips.h>
#include <shell.h>
#include <sysdev.h>
#include <serial_api.h>
#include <sysenv_api.h>
#include <lan_api.h>
#include <syscon_api.h>
#include <sys_api.h>
#include <stdio.h>
#include <string.h>
#include <io_api.h>
#include <env_api.h>
#include <launch.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    char *name;
    bool (*func)(void);
}
t_item;

#define MAX_ITEM	12
#define INDENT		32

/* Messages */

#define SUPPORT		"support@mips.com"
#define UPGRADE	        "DIMM Socket J11: PC100 SDRAM 3.3V 64-128Mbyte"
#define SDBREV		"MIPS SDB Version "
#define SDBREV_UNKNOWN  "MIPS SDB non-approved version"

#define CONFIGURATION   "\
JP1: Position \"TERM\" : Enable SCSI termination.\n\
JP2: Position \"TPWR\" : Enable SCSI termination power.\n\
JP3: Position \"16\"   : PCI bus frequency = 16MHz.\n\
     Position \"33\"   : PCI bus frequency = 33MHz.\n\
\n\
Do no fit any other jumpers.\n\
\n\
All switches in S5 should be OFF (up).\n\
Front panel switch S1 functions as per Harp SDB spec."

#define SHELL_WELCOME_MSG				\
    "YAMON ROM Monitor, Revision "			\
    YAMON_REV_STRING					\
    ".\n"						\
    "Copyright (c) 1999-2007 "				\
    MIPS_NAME						\
    " - All Rights Reserved.\n\n"			\
    "For a list of available commands, type 'help'.\n"


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static char syntax_descr[500] = "Display information on the requested item (default boot).";

static char *info_board_name  = "board";
static char *info_sysctrl_name= "sysctrl";
static char *info_cpu_name    = "cpu";
static char *info_mem_name    = "memory";
static char *info_pci_name    = "pci";
static char *info_ide_name    = "ide";
static char *info_isa_name    = "isa";
static char *info_uart_name   = "uart";
static char *info_lan_name    = "lan";
static char *info_boot_name   = "boot";
static char *info_all_name    = "all";
static char *boot_name        = "boot";
#define MAX_NAME_LEN	      7

static char *info_board_descr = "Board properties";
static char *info_sysctrl_descr = "System Controller properties";
static char *info_cpu_descr   = "CPU properties";
static char *info_mem_descr   = "Memory properties";
static char *info_pci_descr   = "PCI autodiscovery/autoconfiguration";
static char *info_ide_descr   = "IDE configuration";
static char *info_isa_descr   = "ISA bus configuration";
static char *info_uart_descr  = "Serial ports statistics";
static char *info_lan_descr   = "Ethernet statistics";
static char *info_boot_descr  = "Info displayed after reset";
static char *info_all_descr   = "All info";

static char    syntax[100];
static t_item  item[MAX_ITEM];
static UINT32  item_count;
static bool    board_pci;
static bool    board_ide;
static bool    board_isa;
static bool    board_lan;
static bool    board_eeprom;
static bool    init_done = FALSE;
static bool    disp_all  = FALSE;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static bool
check_config1(void);

static bool
check_ip(void);

static bool
check_eeprom(void);

static void
init_command( void );

static UINT32 
get_options(
    UINT32 argc,
    char   **argv,
    char   **name );

static UINT32
determine_total_flash( void );

static bool
disp_prid(
    UINT32 prid );

static bool
info_boot(void);

static bool
info_board(void);

static bool
info_sysctrl(void);

static bool
info_cpu(void);

static bool
info_cpu_boot(void);

static bool
info_mem(void);

static bool
info_mem_boot(void);

static void
info_microsoft(void);

static bool
info_pci( void );

static bool
info_ide( void );

static bool
info_isa( void );

static bool
info_uart( void );

static bool
info_lan( void );

static void
determine_sdbrevision( 
    char *msg );

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          check_config1
 ************************************************************************/
static bool
check_config1(void)
{
    UINT32 config1, config1_reset;

    if( SYSCON_read( SYSCON_CPU_CP0_CONFIG1_ID, 
		     &config1, 
		     sizeof(UINT32) ) != OK )
    {
        return FALSE;
    }

    if( SYSCON_read( SYSCON_CPU_CP0_CONFIG1_RESET_ID,
		     &config1_reset,
		     sizeof(UINT32) ) != OK )
    {
        return FALSE;
    }

    if( config1 == config1_reset )
    {
        return FALSE;
    }

    printf("\nWARNING: Non default cache configuration!");
    return TRUE;
}

/************************************************************************
 *                          check_fpu
 ************************************************************************/
static bool
check_fpu(void)
{
    if (shell_fpu_warning)
    {
	printf("\nWARNING: Non default FPU configuration!");
	return TRUE;
    }
    return FALSE;
}

/************************************************************************
 *                          check_ip
 ************************************************************************/
static bool
check_ip(void)
{
    UINT32 wdata;
    bool   errflag = FALSE;

    if( board_lan )
    {
        if(
	    (SYSCON_read(SYSCON_COM_EN0_IP_ADDR_ID, 
			 &wdata, 
			 sizeof(UINT32)) != OK)    ||
            (wdata == 0) )
        {
            printf("\nWARNING: Environment variable 'ipaddr' is undefined!");
	    errflag = TRUE;
        }

        if(
	    (SYSCON_read(SYSCON_COM_EN0_IP_SUBNETMASK_ID, 
			 &wdata, 
			 sizeof(UINT32)) != OK)    ||
            (wdata == 0) )
        {
            printf("\nWARNING: Environment variable 'subnetmask' is undefined!");
	    errflag = TRUE;
        }
    }

    return errflag;
}

/************************************************************************
 *                          check_eeprom
 ************************************************************************/
static bool
check_eeprom(void)
{
    t_mac_addr	mac;
    t_sn_ascii  sn;
    bool        errflag = FALSE;

    if( board_eeprom )
    {
        if( SYSCON_read(SYSCON_BOARD_SN_ID, &sn, sizeof(sn)) != OK )
	    errflag = TRUE;

        if( board_lan )
	{
            if( SYSCON_read(SYSCON_COM_EN0_MAC_ADDR_ID, 
		            &mac, 
			    sizeof(mac)) != OK )
            {
	        errflag = TRUE;
            }
        }
    }

    if( errflag )
    {
        printf("\nWARNING: EEPROM content is not valid!");
    }

    return errflag;
}

/************************************************************************
 *                          check_pci_alloc
 ************************************************************************/
static bool
check_pci_alloc(void)
{
    bool bdata;
    if (SYSCON_read(SYSCON_BOARD_PCI_ALLOC_ERR_ID, &bdata, sizeof(bool)) == OK
        && bdata != FALSE )
    {
        printf("\nWARNING: PCI ressource overrun (see 'info pci')!");
	return TRUE;
    }
    return FALSE;
}

/************************************************************************
 *                          info
 ************************************************************************/
static MON_FUNC(info)
{
    /* Options */
    char   *name;
    UINT32 rc;

    rc = get_options( argc, argv, &name );

    if( rc != OK )
        return rc;
    else
        return shell_disp_info( name );
}


/************************************************************************
 *                          info_sdb
 ************************************************************************/
static MON_FUNC(info_sdb)
{
    /* Microsoft Windows CE validation */
  
    if( argc != 1 )
    {
        if( *(argv[1]) == '-' )
	{
	    shell_error_data = argv[1]; 
	    return SHELL_ERROR_OPTION;
        }
	else
	    return SHELL_ERROR_SYNTAX;
    }

    info_microsoft();
    SHELL_PUTC( '\n' );
    
    return OK;
}


/************************************************************************
 *                          register_item
 ************************************************************************/
static void
register_item( 
    char *name,
    char *descr,
    bool (*func)(void) )
{
    UINT32 i;

    if (item_count >= MAX_ITEM)
	return;

    if (item_count == 0)
    {
       strcat( syntax_descr, 
	       "\n\nThe following information displays can be requested:\n\n");
    }
    else
    {
	strcat( syntax, "|");
    }

    strcat( syntax, name );

    if( descr )
    {
       strcat( syntax_descr, "\n" );
       strcat( syntax_descr, name );
       strcat( syntax_descr, " : " );
       for( i = strlen(name); i < MAX_NAME_LEN; i++ )
           strcat( syntax_descr, " " );
       strcat( syntax_descr, descr );
    }

    item[item_count].name = name;
    item[item_count].func = func;
    item_count++;
}

/************************************************************************
 *                          determine_total_flash
 ************************************************************************/
static UINT32
determine_total_flash( void )
{
    UINT32 flash_size = 0;
    UINT32 wdata;

    if(SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,  
		    &wdata, sizeof(wdata) ) == OK)
    {
        flash_size += wdata;
    }

    if(SYSCON_read( SYSCON_BOARD_MONITORFLASH_SIZE_ID, &wdata,
	            sizeof(wdata) ) == OK)
    {
        flash_size += wdata;
    }

    if(SYSCON_read( SYSCON_BOARD_FILEFLASH_SIZE_ID, &wdata,
		        sizeof(wdata) ) == OK)
    {
        flash_size += wdata;
    }

    return flash_size;
}


/************************************************************************
 *                          disp_prid
 ************************************************************************/
static bool
disp_prid(
    UINT32 prid )
{
    UINT8 bdata;
    char  msg[80];
    char  *name;

    /* Processor Company ID/Options */
    if(SHELL_PUTS( "Processor Company ID/options =" )) return TRUE;

    bdata = (prid & M_PRIdCoID) >> S_PRIdCoID;
    sprintf( msg, "0x%02x", bdata );
    if(SHELL_PUTS_INDENT( msg, INDENT )) return TRUE;

    name = sys_decode_compid( bdata );
    if( name )
        sprintf( msg, " (%s) / ", name );
    else
        sprintf( msg, " / " );
    if(SHELL_PUTS( msg )) return TRUE;

    bdata = (prid & M_PRIdCoOpt) >> S_PRIdCoOpt;
    sprintf( msg, "0x%02x\n", bdata );
    if(SHELL_PUTS( msg )) return TRUE;

    /* Processor ID/Revision */
    if(SHELL_PUTS( "Processor ID/revision =" )) return TRUE;

    bdata = (prid & M_PRIdImp) >> S_PRIdImp;
    sprintf( msg, "0x%02x", bdata );
    if(SHELL_PUTS_INDENT( msg, INDENT )) return TRUE;

    name = sys_decode_procid();

    if( name )
    {
        sprintf( msg, " (%s)", name );
        if(SHELL_PUTS( msg )) return TRUE;
    }

    sprintf( msg, " / 0x%02x\n", ((prid & M_PRIdRev) >> S_PRIdRev) );

    return (SHELL_PUTS_INDENT( msg, INDENT ));
}


/************************************************************************
 *                          info_microsoft
 ************************************************************************/
static void
info_microsoft( void )
{
    UINT32 wdata;
    UINT32 flash_size;
    char   msg[80];

    if(SHELL_PUTS( "\nSDB Information List :" )) return;
    
    /* Company */
    if(SHELL_PUTS( "\n\n**** Company ****\n" )) return;
    if(SHELL_PUTS( MIPS_NAME )) return;

    /* Support */
    if(SHELL_PUTS( "\n\n**** Support ****\n" )) return;
    if(SHELL_PUTS( SUPPORT )) return;

    /* SDB Board version */
    determine_sdbrevision( msg );
    if(SHELL_PUTS( "\n\n**** SDB Board Version ****\n" )) return;
    if(SHELL_PUTS( msg )) return;

    /* Boot Monitor Revision Date */
    if(SHELL_PUTS( "\n\n**** Boot Monitor Revision Date ****\n" )) return;
    if(SHELL_PUTS( _shell_date )) return;         /* TBD MM/DD/YYYY */

    if(SYSCON_read( SYSCON_CPU_CP0_PRID_ID, &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "\n\n**** Processor ID/Version ****\n" )) return;

        /* ProcessorID */
	if( disp_prid( wdata ) )
	    return;
    }

    /* Upgrade Info */
    if(SHELL_PUTS( "\n\n**** Upgrade Info ****\n" )) return;
    if(SHELL_PUTS( UPGRADE )) return;

    /* Configuration Info */
    if(SHELL_PUTS( "\n\n**** Configuration Info ****\n" )) return;
    if(SHELL_PUTS( CONFIGURATION )) return;

    /**** Memory ****/

    if(SHELL_PUTS( "\n\n**** Memory Footprint ****\n" )) return;
 
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID, 
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "SDRAM size =" )) return;
        sprintf( msg, "%d MB\n", wdata/(1024*1024) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return;
    }

    flash_size = determine_total_flash();
    if(SHELL_PUTS( "Flash memory size =" )) return;
    sprintf( msg, "%d MB\n", flash_size/(1024*1024) );
    if(SHELL_PUTS_INDENT( msg, INDENT )) return;
}

static void
determine_sdbrevision( 
    char *msg )
{
    UINT32 brev, ctype, crev, frev;
  
    strcpy( msg, SDBREV );

    /*  We can assume this is an Atlas board since info_sdb is only
     *  registered in that case.
     */

    SYSCON_read(SYSCON_BOARD_PRODUCTREV_ID,  &brev,  sizeof(UINT32));
    SYSCON_read(SYSCON_BOARD_CORECARDID_ID,  &ctype, sizeof(UINT32));
    SYSCON_read(SYSCON_BOARD_CORECARDREV_ID, &crev,  sizeof(UINT32));
    SYSCON_read(SYSCON_BOARD_FPGAREV_ID,     &frev,  sizeof(UINT32));

    if(      (brev   == 0)	 &&
	     (ctype  == 0)	 &&
	     (crev   == 0)	 &&
	     (frev   <= 6) )
    {
 	strcat( msg, "1.0" );
    }
    else if( (brev   == 0)	 &&
	     (ctype  == 0)	 &&
	     (crev   == 0)	 &&
	     (frev   == 7) )
    {
 	strcat( msg, "1.1" );
    }
    else
    {
	strcpy( msg, SDBREV_UNKNOWN );
    }
}

/************************************************************************
 *                          info_uart
 ************************************************************************/

static bool
info_uart( void )
{
    char   msg[80];
    UINT32 myport;
    UINT32 mymajor;
    UINT32 myminor;
    t_SERIAL_ctrl_descriptor my_ioctl;

    my_ioctl.sc_command = SERIAL_CTRL_GET_STATISTICS;

    for ( myport = PORT_TTY0; myport <= PORT_TTY1; myport++ )
    {
	SYSCON_read( (myport == PORT_TTY0) ? SYSCON_COM_TTY0_MAJOR : SYSCON_COM_TTY1_MAJOR,
	             (void *)(&mymajor),
		     sizeof(UINT32) );

	SYSCON_read( (myport == PORT_TTY0) ? SYSCON_COM_TTY0_MINOR : SYSCON_COM_TTY1_MINOR,
	             (void *)(&myminor),
		     sizeof(UINT32) );

        sprintf( msg, "TTY%x:\n", myport );
        if(SHELL_PUTS( msg )) return FALSE;

        if (IO_ctrl(mymajor, myminor, &my_ioctl)) continue;

        if(SHELL_PUTS( " Bytes transmitted:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_tx_bytes);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Bytes received:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_bytes);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Receive overruns:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_overrun);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Receive parity errors:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_parity);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Receive framing errors:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_framing);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Receive breaks:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_break);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " Receive Interrupts:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_rx_irqs);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

        if(SHELL_PUTS( " No of resets:" )) return FALSE;
        sprintf( msg, "%d\n", my_ioctl.sc_arg.sc_statistics.ua_no_of_init);
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
    
    return TRUE;
}


/************************************************************************
 *                          info_lan
 ************************************************************************/

static bool
info_lan( void )
{
    /* TBD : should be able to break */

    INT32 rcode ;
    UINT32 en0_major_device ;
    t_LAN_CTRL_desc lan_ioctl;

    /* get a EN0 major device number */
    rcode = SYSCON_read( SYSCON_COM_EN0_MAJOR_DEVICE_ID,
                         &(en0_major_device),
                         sizeof(en0_major_device) );
    if (rcode)  return FALSE ;

    /* Display LAN device driver statistics */
    lan_ioctl.command = LAN_CTRL_DISPLAY_STATISTICS ;
    IO_ctrl(  en0_major_device, 0, &lan_ioctl ) ;

    return TRUE;
}


/************************************************************************
 *                          info_pci
 ************************************************************************/
static bool
info_pci( void )
{
    /* TBD : should be able to break */

    pci_display();

    return TRUE;
}


/************************************************************************
 *                          info_ide
 ************************************************************************/
static bool
info_ide( void )
{
    return (shell_ide_display() == OK ) ? TRUE : FALSE;
}


/************************************************************************
 *                          info_isa
 ************************************************************************/
static bool
info_isa( void )
{
    return isa_display();
}


/************************************************************************
 *                          info_cpu
 ************************************************************************/
static bool
info_cpu( void )
{
    char   msg[80];
    UINT32 wdata, wdata_cur;
    UINT8  bdata, bdata_cur;
    bool   tlb_avail;
    
    if( !info_cpu_boot() )
        return FALSE;

    /* ICACHE SIZE */
    if( (SYSCON_read( SYSCON_CPU_ICACHE_SIZE_ID, 
		      &wdata, 
		      sizeof(wdata)) == OK)	&&
        (SYSCON_read( SYSCON_CPU_ICACHE_SIZE_CURRENT_ID, 
		      &wdata_cur, 
		      sizeof(wdata_cur)) == OK) )
    {
        if(SHELL_PUTS( "ICACHE size =" )) return FALSE;

	if( wdata == wdata_cur )
	    sprintf( msg, "%d kByte\n", wdata_cur/1024 );
	else
	    sprintf( msg, "%d (%d) kByte\n", wdata_cur/1024, wdata/1024 );

	if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
    else
        wdata = 0; /* Should never happen */

    if( wdata != 0 ) /* Only if CPU has cache */
    {
        /* ICACHE LINESIZE */
        if( (SYSCON_read( SYSCON_CPU_ICACHE_LINESIZE_ID, 
		          &wdata, 
		          sizeof(wdata)) == OK)	   &&
	    (SYSCON_read( SYSCON_CPU_ICACHE_LINESIZE_CURRENT_ID, 
		          &wdata_cur, 
		          sizeof(wdata_cur)) == OK) )
        {
            if(SHELL_PUTS( "ICACHE line size =" )) return FALSE;

	    if( wdata == wdata_cur )
                sprintf( msg, "%d bytes\n", wdata_cur );
	    else
	        sprintf( msg, "%d (%d) bytes\n", wdata_cur, wdata );	

            if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
        }

        /* ICACHE ASSOC */
        if( (SYSCON_read( SYSCON_CPU_ICACHE_ASSOC_ID, 
			  &wdata, 
			  sizeof(wdata)) == OK)	   &&
	    (SYSCON_read( SYSCON_CPU_ICACHE_ASSOC_CURRENT_ID, 
		          &wdata_cur, 
		          sizeof(wdata_cur)) == OK) )
        {
            if(SHELL_PUTS( "ICACHE associativity =" )) return FALSE;

            if (wdata_cur == 1)
	        sprintf( msg, "direct mapped" );
	    else
   	        sprintf( msg, "%d-way", wdata_cur );

	    if( wdata != wdata_cur )
	    {
	        if( wdata == 1 )
	            sprintf( &msg[strlen(msg)], " (direct mapped)\n" );
	        else
	            sprintf( &msg[strlen(msg)], " (%d-way)\n", wdata );
	    }
	    else
	        sprintf( &msg[strlen(msg)], "\n" );

            if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
        }
    }

    /* DCACHE SIZE */
    if( (SYSCON_read( SYSCON_CPU_DCACHE_SIZE_ID, 
		      &wdata, 
		      sizeof(wdata)) == OK)	&&
        (SYSCON_read( SYSCON_CPU_DCACHE_SIZE_CURRENT_ID, 
		      &wdata_cur, 
		      sizeof(wdata_cur)) == OK) )
    {
        if(SHELL_PUTS( "DCACHE size =" )) return FALSE;

	if( wdata == wdata_cur )
	    sprintf( msg, "%d kByte\n", wdata_cur/1024 );
	else
	    sprintf( msg, "%d (%d) kByte\n", wdata_cur/1024, wdata/1024 );

	if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
    else
        wdata = 0; /* Should never happen */

    if( wdata ) /* Only if CPU has cache */
    {    
        /* DCACHE LINESIZE */
        if( (SYSCON_read( SYSCON_CPU_DCACHE_LINESIZE_ID, 
		          &wdata, 
		          sizeof(wdata)) == OK)	   &&
	    (SYSCON_read( SYSCON_CPU_DCACHE_LINESIZE_CURRENT_ID, 
		          &wdata_cur, 
		          sizeof(wdata_cur)) == OK) )
        {
            if(SHELL_PUTS( "DCACHE line size =" )) return FALSE;

	    if( wdata == wdata_cur )
                sprintf( msg, "%d bytes\n", wdata_cur );
	    else
	        sprintf( msg, "%d (%d) bytes\n", wdata_cur, wdata );	

            if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
        }

        /* DCACHE ASSOC */
        if( (SYSCON_read( SYSCON_CPU_DCACHE_ASSOC_ID, 
	   	          &wdata, 
		          sizeof(wdata)) == OK)	   &&
	    (SYSCON_read( SYSCON_CPU_DCACHE_ASSOC_CURRENT_ID, 
		          &wdata_cur, 
			  sizeof(wdata_cur)) == OK) )
        {
            if(SHELL_PUTS( "DCACHE associativity =" )) return FALSE;

            if (wdata_cur == 1)
	        sprintf( msg, "direct mapped" );
	    else
   	        sprintf( msg, "%d-way", wdata_cur );

	    if( wdata != wdata_cur )
	    {
	        if( wdata == 1 )
	            sprintf( &msg[strlen(msg)], " (direct mapped)\n" );
	        else
	            sprintf( &msg[strlen(msg)], " (%d-way)\n", wdata );
	    }
	    else
	        sprintf( &msg[strlen(msg)], "\n" );

            if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
        }
    }

    /* Secondary CACHE SIZE */
    if( (SYSCON_read( SYSCON_CPU_SCACHE_SIZE_ID, 
		     &wdata, 
		     sizeof(wdata)) == OK) &&
        (SYSCON_read( SYSCON_CPU_SCACHE_SIZE_CURRENT_ID, 
		      &wdata_cur, 
		      sizeof(wdata_cur)) == OK) )
    {
        if(SHELL_PUTS( "SCACHE size =" )) return FALSE;
	if( wdata == wdata_cur )
	    sprintf( msg, "%d kByte\n", wdata_cur/1024 );
	else
	    sprintf( msg, "%d (%d) kByte\n", wdata_cur/1024, wdata/1024 );
	if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
    
    /* Secondary LINESIZE */
    if( (SYSCON_read( SYSCON_CPU_SCACHE_LINESIZE_ID, 
		      &wdata, 
		      sizeof(wdata)) == OK ) &&
	(SYSCON_read( SYSCON_CPU_SCACHE_LINESIZE_CURRENT_ID, 
		     &wdata_cur, 
		      sizeof(wdata_cur)) == OK ) )
    {
        if(SHELL_PUTS( "SCACHE line size =" )) return FALSE;
	if( wdata == wdata_cur )
	    sprintf( msg, "%d bytes\n", wdata_cur );
	else
	    sprintf( msg, "%d (%d) bytes\n", wdata_cur, wdata );	
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Secondary CACHE ASSOC */
    if( (SYSCON_read( SYSCON_CPU_SCACHE_ASSOC_ID, 
		     &wdata, 
		     sizeof(wdata)) == OK ) &&
	(SYSCON_read( SYSCON_CPU_SCACHE_ASSOC_CURRENT_ID, 
		      &wdata_cur, 
		      sizeof(wdata_cur)) == OK ) )
    {
        if(SHELL_PUTS( "SCACHE associativity =" )) return FALSE;

        if (wdata_cur == 1)
	    sprintf( msg, "direct mapped" );
	else
   	    sprintf( msg, "%d-way", wdata_cur );

	if( wdata != wdata_cur )
	{
	    if( wdata == 1 )
		sprintf( &msg[strlen(msg)], " (direct mapped)\n" );
	    else
		sprintf( &msg[strlen(msg)], " (%d-way)\n", wdata );
	}
	else
	    sprintf( &msg[strlen(msg)], "\n" );

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }


    /* TLB */
    if( (SYSCON_read( SYSCON_CPU_TLB_AVAIL_ID,
		 (void *)&tlb_avail,
		 sizeof(bool) ) == OK )		&&

        (SYSCON_read( SYSCON_CPU_TLB_COUNT_ID,
		     (void *)&bdata_cur,
		     sizeof(UINT8) ) == OK)     &&

        (SYSCON_read( SYSCON_CPU_TLB_COUNT_RESET_ID,
		     (void *)&bdata,
		     sizeof(UINT8) ) == OK) )
    {
        if(SHELL_PUTS( "TLB entries =" )) return FALSE;

	if( tlb_avail )
	{
	    sprintf( msg, "%d\n", bdata_cur );
	}
	else
	{
	    if( bdata != 0 )
	        sprintf( msg, "None (%d)\n", bdata );
	    else
	        strcpy( msg, "None\n" );
	}

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    /* Register sets */
    if(sys_arch_rev >= K_ConfigAR_Rel2 &&
       (SYSCON_read( SYSCON_CPU_REGISTER_SETS_ID,
		     (void *)&bdata,
		     sizeof(UINT8) ) == OK ))
    {
        if(SHELL_PUTS( "Register Sets =" )) return FALSE;

	sprintf( msg, "%d\n", bdata );

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    if(SHELL_PUTS( "CPU type =")) return FALSE;   
    if(sys_mips32_64)
    {
        if(SHELL_PUTS_INDENT( 
	    sys_64bit ? "MIPS64\n" : "MIPS32\n", INDENT ))
	        return FALSE;

        if(SHELL_PUTS( "Architecture revision =" )) return FALSE;

	sprintf( msg, "%d\n", sys_arch_rev+1 );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
    else
    {
        if(SHELL_PUTS_INDENT( 
	    sys_64bit ? "64 Bit\n" : "32 Bit\n", INDENT )) 
	        return FALSE;
    }

    if(SHELL_PUTS( "MIPS16e implemented =")) return FALSE;
    if(SHELL_PUTS_INDENT( sys_mips16e ? "Yes\n" : "No\n", INDENT )) 
        return FALSE;

    if(SHELL_PUTS( "EJTAG implemented =")) return FALSE;
    if(SHELL_PUTS_INDENT( sys_ejtag  ? "Yes\n" : "No\n", INDENT )) 
        return FALSE;

    if(SHELL_PUTS( "FPU implemented =")) return FALSE;
    if(SHELL_PUTS_INDENT( sys_fpu    ? "Yes\n" : "No\n", INDENT )) 
        return FALSE;

    if (sys_mips32_64 && (sys_arch_rev > 0)) {
	if(SHELL_PUTS( "EIC mode =")) return FALSE;
	if(SHELL_PUTS_INDENT( sys_eicmode    ? "Yes\n" : "No\n", INDENT )) 
	    return FALSE;
    }

    if(SHELL_PUTS( "MT implemented =")) return FALSE;
    if(SHELL_PUTS_INDENT( sys_mt    ? "Yes\n" : "No\n", INDENT )) 
        return FALSE;
#if 0
    /* FIXME */
    if (sys_mt) {
	if ((SYSCON_read( SYSCON_CPU_MT_PVPE_ID, &bdata)) == OK) {
	    if(SHELL_PUTS( "VPE contexts =" )) return FALSE;
	    if( bdata != 0 )
	        sprintf( msg, "%d\n", bdata );
	    else
	        strcpy( msg, "None\n" );
	    if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
	}
	if ((SYSCON_read( SYSCON_CPU_MT_PTC_ID, &bdata)) == OK) {
	    if(SHELL_PUTS( "Thread contexts =" )) return FALSE;
	    sprintf( msg, "%d\n", bdata + 1);
	    if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
	}
    }
#endif
    if(SHELL_PUTS( "DSP implemented =")) return FALSE;
    if(SHELL_PUTS_INDENT( sys_dsp    ? "Yes\n" : "No\n", INDENT )) 
        return FALSE;

    return TRUE;
}


/************************************************************************
 *                          info_cpu_boot
 ************************************************************************/
static bool
info_cpu_boot( void )
{
    char   msg[80];
    UINT32 wdata;
    bool   bdata;
    UINT32 cpu_freq, bus_freq;
    char   *s;
    extern bool gcmp_present, io_coherent;
    int	   cpu;

    if(SYSCON_read( SYSCON_CPU_CP0_PRID_ID, &wdata, sizeof(wdata)) == OK)
    {
        if( disp_prid( wdata ) )
	    return FALSE;
    }

    /* Endianess */
    if(SYSCON_read(SYSCON_CPU_ENDIAN_BIG_ID, &bdata, sizeof(bdata)) == OK)
    {
        if(SHELL_PUTS("Endianness =" )) return FALSE;
        if(SHELL_PUTS_INDENT( bdata ? "Big\n" : "Little\n", INDENT )) return FALSE;
    }

    /* CPU and bus frequency */
    if(
        (SYSCON_read( SYSCON_BOARD_CPU_CLOCK_FREQ_ID, 
		      &cpu_freq, sizeof(cpu_freq)) == OK) &&
	(SYSCON_read( SYSCON_BOARD_BUS_CLOCK_FREQ_ID, 
		      &bus_freq, sizeof(bus_freq)) == OK) )

    {
        if(SHELL_PUTS( "CPU/Bus frequency =" )) return FALSE;

        if( cpu_freq < 1000*10 )
            sprintf( msg, "%d Hz", cpu_freq );
        else if( cpu_freq < 1000000*10 )
            sprintf( msg, "%d kHz", (cpu_freq+500)/1000 );
        else
            sprintf( msg, "%d MHz", (cpu_freq+500000)/1000000 );

	s = &msg[strlen(msg)];

	if( bus_freq < 1000*10 )
            sprintf( s, " / %d Hz\n", bus_freq );
        else if( bus_freq < 1000000*10 )
            sprintf( s, " / %d kHz\n", (bus_freq+500)/1000 );
        else
            sprintf( s, " / %d MHz\n", (bus_freq+500000)/1000000 );

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    if(SHELL_PUTS( "Coherency =" )) return FALSE;
    s = msg;
    if (gcmp_present) {
      s += sprintf (s, "Cache");
      if (io_coherent)
	s += sprintf (s, ",IO");
    }
    else {
      sprintf (s, "None");
    }
    *s++ = '\n';
    *s++ = '\0';
    if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

    if (gcmp_present) {
        if(SHELL_PUTS( "Running CPUs =" )) return FALSE;
	s = msg;
	for (cpu = 0; cpu < 8; cpu++) {
	  if (cpu_present(cpu)) {
	    if (s != msg)
	      *s++ = ',';
	    *s++ = '0' + cpu;
	  }
	}
	*s++ = '\n';
	*s = '\0';
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    return TRUE;
}


/************************************************************************
 *                          info_board
 ************************************************************************/
static bool
info_board( void )
{
    char       msg[80];
    UINT32     wdata;
    t_mac_addr mac;
    t_sn_ascii sn;
    char       *name;

    /* Base board type/revision */
    if(SYSCON_read(SYSCON_BOARD_PRODUCTID_ID, &wdata, sizeof(UINT32)) == OK)
    {
        if(SHELL_PUTS( "Board type/revision =" )) return FALSE;

	SYSCON_read( SYSCON_BOARD_PRODUCTNAME_ID, 
		     (void *)(&name),
		     sizeof(char *) );
	
	sprintf( msg, "0x%02x (%s)", wdata, name );
	
        if(SYSCON_read(SYSCON_BOARD_PRODUCTREV_ID, &wdata, sizeof(UINT32)) == OK)
        {
            sprintf( &msg[strlen(msg)], " / 0x%02x\n", wdata );
	}

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Platform specifics to be displayed */
    if( !shell_arch_info( INDENT ) )
        return FALSE;

    /* MAC address */
    if(SYSCON_read(SYSCON_COM_EN0_MAC_ADDR_ID, &mac, sizeof(mac)) == OK)
    {
	if(SHELL_PUTS( "MAC address =" )) return FALSE;

        sprintf( msg, "%02x.%02x.%02x.%02x.%02x.%02x\n",
	                 mac[0],
			 mac[1],
			 mac[2],
			 mac[3],
			 mac[4],
			 mac[5] );

        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Serial number */
    if(SYSCON_read(SYSCON_BOARD_SN_ID, &sn, sizeof(sn)) == OK)
    {
	if(SHELL_PUTS( "Board S/N =" )) return FALSE;
	sprintf( msg, "%s\n", sn  );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Determine bus frequency */
    if( SYSCON_read(
        SYSCON_BOARD_PCI_FREQ_KHZ_ID,
	(void *)&wdata,
	sizeof(UINT32) ) == OK )
    {
	if(SHELL_PUTS( "PCI bus frequency =" ) ) return FALSE;

	pci_busfreq_string( msg, wdata );
	strcat( msg, "\n" );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
  
    return TRUE;
}


/************************************************************************
 *                          info_sysctrl
 ************************************************************************/
static bool
info_sysctrl( void )
{
    char       msg[80];
    UINT32     wdata, minor, major;

    /* Platform specifics to be displayed */
    if( !shell_sysctrl_info( INDENT ) )
        return FALSE;

    /* Sysctrl Register window */
    if(SYSCON_read( SYSCON_SYSCTRL_REGADDR_BASE_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Register base address =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* MSC01 system ID */
    if(SYSCON_read( SYSCON_SYSCTRL_SYSID_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Sysid =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* MSC01 pbc revision */
    if(SYSCON_read( SYSCON_SYSCTRL_PBCREV_MAJOR_ID,
		    &major, sizeof(major)) == OK)
    {
        if(SHELL_PUTS("PBC rev. =" )) return FALSE;
        if(SYSCON_read( SYSCON_SYSCTRL_PBCREV_MINOR_ID,
		         &minor, sizeof(minor)) == OK)
            sprintf( msg, "%d.%d\n", major, minor );
        else
            sprintf( msg, "%d\n", major );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* MSC01 pci revision */
    if(SYSCON_read( SYSCON_SYSCTRL_PCIREV_MAJOR_ID,
		    &major, sizeof(major)) == OK)
    {
        if(SHELL_PUTS("PCI rev. =" )) return FALSE;
        if(SYSCON_read( SYSCON_SYSCTRL_PCIREV_MINOR_ID,
		         &minor, sizeof(minor)) == OK)
            sprintf( msg, "%d.%d\n", major, minor );
        else
            sprintf( msg, "%d\n", major );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* MSC01 "performance" bit */
    if(SYSCON_read( SYSCON_SYSCTRL_WC_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Write coherency =" )) return FALSE;
        sprintf( msg, "%d\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM data width */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_FW_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM external width =" )) return FALSE;
        strcpy( msg, wdata == 2 ? "Full (64" : wdata == 1
                     ? ( sys_64bit ? "Full (64" : "Full (32" )
                     : ( sys_64bit ? "Half (32" : "Half (16" ) );
	strcat( msg, " bit)\n" );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM parity */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_PARITY_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM parity check =" )) return FALSE;
        strcpy( msg, wdata == 0 ? "No\n" : "Yes\n" );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM refresh interval */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM refresh interval =" )) return FALSE;
        sprintf( msg, "%d ns\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM read delay */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_RDDEL_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM rddel =" )) return FALSE;
        sprintf( msg, "%d bus cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

#if 0
    /* System RAM clock ratio */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_CLKRAT_CFG_ID,
		    &sdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM bus:ram clock ratio =" )) return FALSE;
        sprintf( msg, "%s\n", sdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
#endif

    /* CAS latency */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_CASLAT_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM CAS latency =" )) return FALSE;
	switch (wdata)
	{
	  /* See board_systemram_caslat_cycles_msc01_read for these encodings... */
	  case 5: strcpy( msg, "1.5 ram cycles\n" ); break;
	  case 6: strcpy( msg, "2.5 ram cycles\n" ); break;
	  case 7: strcpy( msg, "0.5 ram cycles\n" ); break;
	  case 8: strcpy( msg, "3.5 ram cycles\n" ); break;
          default: sprintf( msg, "%d ram cycles\n", wdata ); break;
	}
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* SRAS precharge */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM min precharge delay =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* SRAS to SCAS delay */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM min RAS to CAS delay =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

#if 0
    /* System RAM CS delay */ 
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_CSLAT_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM Chip Select delay =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
#endif

    /* System RAM write latency */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_WRLAT_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM write latency =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM Write burst length */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_WRITE_BURSTLEN_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM write burst length =" )) return FALSE;
        sprintf( msg, "%d\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    /* System RAM Read burst length */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_READ_BURSTLEN_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM read burst length =" )) return FALSE;
        sprintf( msg, "%d\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    /* System RAM RAS min */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_RASMIN_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM RAS min =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    /* System RAM RAS to RAS */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_RAS2RAS_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM RAS to RAS =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
 
    /* System RAS Max */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_RASMAX_CYCLES_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM RAS max =" )) return FALSE;
        sprintf( msg, "%d ram cycles\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    return TRUE;
}
 

/************************************************************************
 *                          info_mem
 ************************************************************************/
static bool
info_mem( void )
{
    char   msg[80];
    UINT32 wdata;
    char  *sdata;

    /* System Flash base */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_BASE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("System flash phys base =" )) return FALSE;
        sprintf( msg, "0x%08x\n", PHYS(wdata) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System Flash size */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMFLASH_SIZE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("System flash size =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Monitor Flash base */
    if(SYSCON_read( SYSCON_BOARD_MONITORFLASH_BASE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Monitor flash phys base =" )) return FALSE;
        sprintf( msg, "0x%08x\n", PHYS(wdata) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Monitor Flash size */
    if(SYSCON_read( SYSCON_BOARD_MONITORFLASH_SIZE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Monitor flash size =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Environment Flash base */
    if(SYSCON_read( SYSCON_BOARD_FILEFLASH_BASE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Env. flash phys base =" )) return FALSE;
        sprintf( msg, "0x%08x\n", PHYS(wdata) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Environment Flash size */
    if(SYSCON_read( SYSCON_BOARD_FILEFLASH_SIZE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("Env. flash size =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM base */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_BASE_ID,  
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("\nSDRAM phys base =" )) return FALSE;
        sprintf( msg, "0x%08x\n", PHYS(wdata) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* System RAM size */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID, 
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM size =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

#if 0
    /* System RAM type */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_DDR_CFG_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM type =" )) return FALSE;
        strcpy( msg, wdata == 1 ? "DDR\n" : "SDR\n" );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }
#endif

    /* Free memory */
    if(SYSCON_read( SYSCON_BOARD_FREE_MEM_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "\nFirst free SDRAM address =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Stack size */
    if(SYSCON_read( SYSCON_BOARD_STACK_SIZE_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "Stack size =" )) return FALSE;
        sprintf( msg, "0x%x bytes\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Application stack size */
    if(SYSCON_read( SYSCON_BOARD_APPL_STACK_SIZE_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "Application stack size =" )) return FALSE;
        sprintf( msg, "0x%x bytes\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    return TRUE;
}

/************************************************************************
 *                          info_mem_boot
 ************************************************************************/
static bool
info_mem_boot( void )
{
    char   msg[80];
    UINT32 flash_size;
    UINT32 wdata;

    /* Flash size */
    flash_size = determine_total_flash();
    if(SHELL_PUTS("Flash memory size =" )) return FALSE;
    sprintf( msg, "%d MByte\n", flash_size / (1024*1024) );
    if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

    /* Ram size */
    if(SYSCON_read( SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID, 
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS("SDRAM size =" )) return FALSE;
        sprintf( msg, "%d MByte\n", wdata / (1024*1024) );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    /* Free memory */
    if(SYSCON_read( SYSCON_BOARD_FREE_MEM_ID,
		    &wdata, sizeof(wdata)) == OK)
    {
        if(SHELL_PUTS( "First free SDRAM address =" )) return FALSE;
        sprintf( msg, "0x%08x\n", wdata );
        if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;
    }

    return TRUE;
}


/************************************************************************
 *                          info_boot
 ************************************************************************/
static bool
info_boot( void )
{
    char msg[80];
    bool errflag;

    /* Welcome message */
    if(SHELL_PUTS( SHELL_WELCOME_MSG )) return FALSE;

    /* Compile data and time */
    if(SHELL_PUTS( "\nCompilation time =" )) return FALSE;
#ifdef _BUILD_USER_
    sprintf( msg, "%s  %s (%s)\n", _shell_date, _shell_time, _BUILD_USER_ );
#else
    sprintf( msg, "%s  %s\n", _shell_date, _shell_time );
#endif
    if(SHELL_PUTS_INDENT( msg, INDENT )) return FALSE;

    if( !disp_all )
    {
        if( !info_board() )
            return FALSE;

        if( !info_cpu_boot() )
            return FALSE;

        if( !info_mem_boot() )
            return FALSE;
    }

    /* Print out warnings for various things */

    errflag = FALSE;

    if ( SYSENV_check_state() ) errflag = TRUE;
    if ( !errflag )
    {
        if( check_ip() )        errflag = TRUE;
        if( check_eeprom() )    errflag = TRUE;
	if( env_check() )	errflag = TRUE;
	if( check_config1() )	errflag = TRUE;
	if( check_fpu() )	errflag = TRUE;
	if( check_pci_alloc() )	errflag = TRUE;
    }

    if (errflag)
	printf("\n");

    return TRUE;
}


/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32
get_options(
    UINT32 argc,
    char   **argv,
    char   **name )
{
    if( argc > 2 )
    {
        return SHELL_ERROR_SYNTAX;
    }

    *name = 
        ( argc != 1 ) ?
	    argv[1] : NULL;

    if( !(*name) )
        *name = boot_name;

    return OK;
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "info",
    info,
    NULL,
    NULL,

    NULL,
    0,
    FALSE
};

/* Command definitions for SDB 'i' command (secret command) */
static t_cmd cmd_def_sdb_lower =
{
    "i",
    info_sdb,
    "i                        (Microsoft SDB command)",
    "Identity command. Displays SDB information list.",

    NULL,
    0,
    TRUE
};


/************************************************************************
 *                          init_command
 ************************************************************************/
static void
init_command( void )
{
    item_count = 0;
    sprintf( syntax, "info [" );

    register_item( info_boot_name,  info_boot_descr,  info_boot );
    register_item( info_board_name, info_board_descr, info_board );
    register_item( info_cpu_name,   info_cpu_descr,   info_cpu );
    register_item( info_sysctrl_name, info_sysctrl_descr, info_sysctrl );
    register_item( info_mem_name,   info_mem_descr,   info_mem );
    register_item( info_uart_name,  info_uart_descr,  info_uart );
    register_item( info_all_name,   info_all_descr,   NULL );

    if( board_pci )
        register_item( info_pci_name, info_pci_descr, info_pci );

    if( board_ide )
        register_item( info_ide_name, info_ide_descr, info_ide );

    if( board_isa )
        register_item( info_isa_name, info_isa_descr, info_isa );

    if( board_lan )
        register_item( info_lan_name, info_lan_descr, info_lan );

    strcat( syntax, "]" );

    cmd_def.descr   = syntax_descr;    
    cmd_def.syntax  = syntax;

    init_done = TRUE;
} 


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_disp_info
 *  Description :
 *  -------------
 *
 *  Display info on the specified item
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
UINT32
shell_disp_info(
    char *name )
{
    t_item *item_data;
    bool   valid;
    UINT32 i;

    disp_all =
        (strcmp(name, info_all_name) == 0) ?
	    TRUE :
	    FALSE;

    if( disp_all )
    {
        SHELL_DISABLE_MORE;
    }

    /* Go through all registered items */

    for( i=0; i<item_count; i++ )
    {
	item_data = &item[i];
	valid     = FALSE;

	if( disp_all )
	    valid = TRUE;
	else
	{	    			  
	    if( strcmp(item_data->name, name) == 0 )
		valid = TRUE;
	}

        if( valid )
        {
	    if( item_data->func )
	    {
	        if( strcmp( name, info_boot_name ) != 0 )
		{
	            if(SHELL_PUTS( "\n**** Info " )) 
		        return OK;
	            if(SHELL_PUTS( item_data->name )) 
		        return OK;
	            if(SHELL_PUTS( " ****\n\n" )) 
		        return OK;
		}
		else
		{
	            if(SHELL_PUTS( "\n" )) 
		        return OK;
		}

	        if( !item_data->func() || !disp_all )
		{
 		    SHELL_PUTC( '\n' );
	            return OK;
		}

 		if( SHELL_PUTC( '\n' ) )
		    return OK;
            }
        }
    }
    
    if( !disp_all )
    {
        /* Not found */
	if( *name == '-' )
	{
	    shell_error_data = name;
	    return SHELL_ERROR_OPTION;
	}
	else
	    return SHELL_ERROR_SYNTAX;
    }

    return OK;
}


/************************************************************************
 *
 *                          shell_info_init
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
shell_info_init(
    bool pci,			/* TRUE -> Board supports PCI		*/
    bool ide,			/* TRUE -> Board supports IDE		*/
    bool isa,			/* TRUE -> Board supports ISA bus	*/
    bool lan,			/* TRUE -> Board supports Ethernet	*/
    bool eeprom )		/* TRUE -> Boards supports EEPROM	*/
{
    board_pci    = pci;
    board_ide    = ide;
    board_isa	 = isa;
    board_lan	 = lan;
    board_eeprom = eeprom;

    if( !init_done )
        init_command();
   
    return &cmd_def;
}


/************************************************************************
 *
 *                          shell_info_sdb_init
 *  Description :
 *  -------------
 *
 *  Register command for shell
 *
 *  Return values :
 *  ---------------
 *
 *  void
 *
 ************************************************************************/
t_cmd *
shell_info_sdb_init( void )
{
    if( !init_done )
        init_command();

    return &cmd_def_sdb_lower;
}

