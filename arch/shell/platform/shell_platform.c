
/************************************************************************
 *
 *  shell_platform.c
 *
 *  Platform specific setup of shell
 *
 * ######################################################################
 *
 * mips_start_of_legal_notice
 * 
 * Copyright (c) 2004 MIPS Technologies, Inc. All rights reserved.
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
#include <product.h>
#include <atlas.h>
#include <sead.h>
#include <shell_api.h>
#include <sys_api.h>
#include <syscon_api.h>
#include <stdio.h>
#include <string.h>

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
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_arch
 *  Description :
 *  -------------
 *
 *  Platform specific setup of shell
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
#ifdef PROMON
t_cmd *shell_ptest_init(void);
#endif
void 
shell_arch(void)
{
    bool cache_configurable, mmu_configurable, sys_mmu_tlb;
    t_cmd *cmd;

    /* Common commands */

#ifdef PROMON
    shell_register_cmd( shell_ptest_init()    );
#endif
    shell_register_cmd( shell_cksum_init()    );
    shell_register_cmd( shell_compare_init()  );
    shell_register_cmd( shell_copy_init()     );
    shell_register_cmd( shell_cp0_init()      );
    shell_register_cmd( shell_dis_init()      );
    shell_register_cmd( shell_dump_init()     );
    shell_register_cmd( shell_echo_init()     );
    shell_register_cmd( shell_edit_init()     );
    shell_register_cmd( shell_erase_init()    );
    shell_register_cmd( shell_fill_init()     );
    shell_register_cmd( shell_fpuemul_init()  );
    shell_register_cmd( shell_gdb_init()      );
    shell_register_cmd( shell_go_init()	      );
    shell_register_cmd( shell_load_init()     );
    shell_register_cmd( shell_port_init()     );
    shell_register_cmd( shell_search_init()   );
    shell_register_cmd( shell_setenv_init()   );
    shell_register_cmd( shell_sleep_init()    );
    shell_register_cmd( shell_stty_init()     );
    shell_register_cmd( shell_test_init()     );
    shell_register_cmd( shell_unsetenv_init() );
    shell_register_cmd( shell_eeprom_init()   );

    if( sys_fpu )
        shell_register_cmd( shell_cp1_init()  );

    /**** Platform specific commands ****/

    switch( sys_platform )
    {
      case PRODUCT_ATLASA_ID :
        shell_register_cmd( shell_off_init()	);
        shell_register_cmd( shell_date_init()   );
        shell_register_cmd( shell_pcicfg_init() );
        shell_register_cmd( shell_reset_init()  );
        shell_register_cmd( shell_ping_init()   );
        shell_register_cmd( shell_fread_init()  );
        shell_register_cmd( shell_fwrite_init() );
        shell_register_cmd( shell_info_init( TRUE, FALSE, FALSE, TRUE, TRUE ) );

	/*  Microsoft SDB commands
	 *  Note : shell_stty_sdb_init must be
	 *  called after shell_info_init.
	 */
        shell_register_cmd( shell_dump_sdb_init()  );
        shell_register_cmd( shell_erase_sdb_init() );
        shell_register_cmd( shell_info_sdb_init()  );
        shell_register_cmd( shell_load_sdb_init()  );
        shell_register_cmd( shell_stty_sdb_init()  );
        shell_register_cmd( shell_test_sdb_init()  );

	break;
      case PRODUCT_MALTA_ID :
        shell_register_cmd( shell_date_init()   );
	shell_register_cmd( shell_disk_init()   );
        shell_register_cmd( shell_pcicfg_init() );
        shell_register_cmd( shell_reset_init()  );
        shell_register_cmd( shell_ping_init()   );
        shell_register_cmd( shell_fread_init()  );
        shell_register_cmd( shell_fwrite_init() );
        shell_register_cmd( shell_info_init( TRUE, TRUE, TRUE, TRUE, TRUE ) );
	break;
      case PRODUCT_SEAD_ID :
        shell_register_cmd( shell_off_init() );
        shell_register_cmd( shell_info_init( FALSE, FALSE, FALSE, FALSE, FALSE ) );
	break;
      case PRODUCT_SEAD2_ID :
        shell_register_cmd( shell_reset_init() );
        shell_register_cmd( shell_info_init( FALSE, FALSE, FALSE, FALSE, FALSE ) );
	break;
      default :
        break;
    }

    /**** Other dependencies ****/

    cmd = shell_cache_init();
    if(cmd)
    {
        /* Cache available */
        shell_register_cmd( cmd );
        shell_register_cmd( shell_flush_init() );
    }

    SYSCON_read( SYSCON_CPU_CACHE_CONFIGURABLE_ID,
		 &cache_configurable,
		 sizeof(bool) );

    SYSCON_read( SYSCON_CPU_MMU_CONFIGURABLE_ID,
		 &mmu_configurable,
		 sizeof(bool) );

    if( cache_configurable || mmu_configurable )
    {
        shell_register_cmd( shell_scpu_init(
				cache_configurable,
				mmu_configurable ) );
    }

    SYSCON_read( SYSCON_CPU_TLB_AVAIL_ID,
	         (void *)&sys_mmu_tlb,
	         sizeof( bool ) );

    if( sys_mmu_tlb )
        shell_register_cmd( shell_tlb_init() );
}


/************************************************************************
 *
 *                          shell_sysctrl_info
 *  Description :
 *  -------------
 *  Platform specifics to be printed by "info sysctrl" command
 *
 *  Return values :
 *  ---------------
 *  TRUE : No ctrl-c detected, FALSE : ctrl-c detected
 *
 ************************************************************************/
bool
shell_sysctrl_info( 
    UINT32 indent )
{
    UINT32 wdata;
    UINT32 major, minor;
    char   msg[80];
    char   *name;

    switch( sys_platform )
    {
      case PRODUCT_MALTA_ID :
      case PRODUCT_ATLASA_ID :
sead_msc01:    
        /* System controller name/revision */
	if(SYSCON_read(SYSCON_SYSCTRL_NAME_ID, &name, sizeof(UINT32)) == 0)
	{
            if(SHELL_PUTS( "System controller/revision =" )) return FALSE;
            strcpy( msg, name );
	    if(SYSCON_read(SYSCON_SYSCTRL_REV_ID, &name, sizeof(UINT32)) == 0)
	    {
                strcat( msg," / " );
                strcat( msg, name );
	    }
            strcat( msg,"\r\n" );
	    if(SHELL_PUTS_INDENT( msg, indent )) return FALSE;
        }
    
	break;

      case PRODUCT_SEAD_ID :
      case PRODUCT_SEAD2_ID :
        if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01) goto sead_msc01;

        /* RTL ID/revision */      
        if(SYSCON_read(SYSCON_BOARD_RTL_ID, &wdata, sizeof(UINT32)) == 0)
        {
	    if(SHELL_PUTS( "RTL ID/revision =" )) return FALSE;

	    if( SYSCON_read( SYSCON_BOARD_RTL_NAME_ID, 
			 &name, sizeof(char *) ) == 0 )
	    {
	        sprintf( msg, "0x%02x (%s)", wdata, name );
            }
	    else
	    {
	        sprintf( msg, "0x%02x", wdata );
	    }

	    if( (SYSCON_read(SYSCON_BOARD_RTLREV_MAJOR_ID, 
			     &major, sizeof(UINT32)) == 0) &&
		(SYSCON_read(SYSCON_BOARD_RTLREV_MINOR_ID, 
			     &minor, sizeof(UINT32)) == 0) )
	    {
	        sprintf( &msg[strlen(msg)], " / %d.%d\n", 
		         major, minor );
	    }

	    if(SHELL_PUTS_INDENT( msg, indent )) return FALSE;
        }

	break;

      default :
        break;
    }
 
    return TRUE;
}


/************************************************************************
 *
 *                          shell_arch_info
 *  Description :
 *  -------------
 *  Platform specifics to be printed by "info board" command
 *
 *  Return values :
 *  ---------------
 *  TRUE : No ctrl-c detected, FALSE : ctrl-c detected
 *
 ************************************************************************/
bool
shell_arch_info( 
    UINT32 indent )
{
    UINT32 wdata;
    UINT32 major, minor;
    char   msg[80];
    char   *name;

    switch( sys_platform )
    {
      case PRODUCT_MALTA_ID :
      case PRODUCT_ATLASA_ID :
        /* Core board type/revision */
	if(SYSCON_read(SYSCON_BOARD_CORECARDID_ID, &wdata, sizeof(UINT32)) == 0)
	{
            if(SHELL_PUTS( "Core board type/revision =" )) return FALSE;

	    if( SYSCON_read( SYSCON_BOARD_CORECARD_NAME_ID, 
			 &name, sizeof(char *) ) == 0 )
	    {
                sprintf( msg, "0x%02x (%s)", wdata, name );
	    }
	    else
	    {
		sprintf( msg, "0x%02x", wdata );
	    }

	    if(SYSCON_read(SYSCON_BOARD_CORECARDREV_ID, &wdata, sizeof(UINT32)) == 0)
	    {
                sprintf( &msg[strlen(msg)], " / 0x%02x\n", wdata );
	    }

	    if(SHELL_PUTS_INDENT( msg, indent )) return FALSE;
        }
    
        /* System controller name/revision */
        if(shell_sysctrl_info( indent ) == FALSE) return FALSE;
    
        /* FPGA revision */
	if(SYSCON_read(SYSCON_BOARD_FPGAREV_ID, &wdata, sizeof(UINT32)) == 0)
	{
            if(SHELL_PUTS( "FPGA revision =" )) return FALSE;
            sprintf( msg, "0x%04x\n", wdata );
            if(SHELL_PUTS_INDENT( msg, indent )) return FALSE;
        }

	break;

      case PRODUCT_SEAD_ID :
      case PRODUCT_SEAD2_ID :
        /* RTL ID/revision */      
        if(shell_sysctrl_info( indent ) == FALSE) return FALSE;

	break;

      default :
        break;
    }

    return TRUE;
}



/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

