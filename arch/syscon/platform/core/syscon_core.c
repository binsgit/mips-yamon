/************************************************************************
 *
 *  syscon_core.c
 *
 *  Core card specific parts of SYSCON module.
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
#include <syscon_api.h>
#include <sys_api.h>
#include <syscon.h>
#include <syserror.h>
#include <stdio.h>
#include <product.h>
#include <malta.h>
#include <sead.h>
#include <gt64120.h>
#include <bonito64.h>
#include <core_bonito64.h>
#include <core_sys.h>
#include <pci.h>
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

/* Pointer to array of objects */
static t_syscon_obj       *syscon_objects;

/* Core card names */
static char   *name_qed_rm5261	      = "QED RM5261 Board";
static char   *name_core_lv	      = "CoreLV";
static char   *name_core_bonito	      = "CoreBonito64"; /* also "Bonito64" */
static char   *name_core_20k	      = "Core20K";
static char   *name_core_fpga	      = "CoreFPGA";
static char   *name_core_sys	      = "CoreSYS";
static char   *name_core_emul	      = "CoreEMUL";
static char   *name_core_fpga2        = "CoreFPGA-2";
static char   *name_core_fpga3        = "CoreFPGA-3";
static char   *name_core_24K          = "Core24K";
static char   *name_galileo           = "Galileo";
       char   *name_msc01             = "MIPS SOC-it 101\0       ";
static char   version_syscntrl[32]    = "unknown version";

/* North Bridge settings */
static UINT32 pci_mem_start;
static UINT32 pci_mem_size;
static UINT32 pci_mem_offset;
static UINT32 pci_io_start;
static UINT32 pci_io_size;
static UINT32 pci_io_offset;

static UINT32 ram_range_base;
static UINT32 ram_range_size;

/************************************************************************
 *  Static function prototypes
 ************************************************************************/



/************************************************************************
 *  Static functions
 ************************************************************************/


/************************************************************************
 *  Static functions registered for handling particular SYSCON objects 
 *  for particular core cards.
 ************************************************************************/


/************************************************************************
 *    board_systemram_refresh_ns_gt64120_read
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_gt64120_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    GT_L32( sys_nb_base, GT_SDRAM_CFG_OFS, regval);

    *(UINT32 *)param = 
        CYCLES2NS( REGFIELD( regval, GT_SDRAM_CFG_REFINT ) );

    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_gt64120_write 
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_gt64120_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval, refcount;

    NS2COUNT_ROUND_DOWN( *(UINT32 *)param, refcount );

    refcount = MIN( refcount,
	            GT_SDRAM_CFG_REFINT_MSK >> GT_SDRAM_CFG_REFINT_SHF );
    
    /* Read */
    GT_L32( sys_nb_base, GT_SDRAM_CFG_OFS, regval);

    /* Modify */
    regval &= ~GT_SDRAM_CFG_REFINT_MSK;
    regval |= (refcount << GT_SDRAM_CFG_REFINT_SHF);

    /* Write */
    sys_func_noram( (t_sys_func_noram)gt64120_write_reg, 
		    KSEG1(sys_nb_base + GT_SDRAM_CFG_OFS), 
		    regval,
		    0 );

    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_bonito64_read
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_bonito64_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 mult, clock_ns, cycles_per_refresh;

    /* Multiply factor (base refresh rate = 3.9 us) */
    mult = 1 << 
           ( (BONITO_SDCFG & BONITO_SDCFG_DRAMRFSHMULT) >> 
             BONITO_SDCFG_DRAMRFSHMULT_SHIFT );

    /* Bonito64 was informed on CPU clock period */
    clock_ns = (BONITO_IODEVCFG & BONITO_IODEVCFG_CPUCLOCKPERIOD) >>
	        BONITO_IODEVCFG_CPUCLOCKPERIOD_SHIFT;

    if( clock_ns == 0 )
    {
        clock_ns = BONITO_IODEVCFG_CPUCLOCKPERIOD >> 
		   BONITO_IODEVCFG_CPUCLOCKPERIOD_SHIFT;
    }

    cycles_per_refresh = mult * (UINT32)3900 / clock_ns;

    *(UINT32 *)param = CYCLES2NS( cycles_per_refresh );
   
    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_msc01_read
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 count;

    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        count = MC_GET_REG_FIELD(MC_GET_REG(TREF_ADDR),
					TREF_WIDTH, TREF_OFFSET);
	*(UINT32 *)param = CYCLES2NS( count );
	return OK;
    }

    count = REG(MSC01_MC_REG_BASE, MSC01_MC_TREFRESH);

    /* adjust count to bus cycles (inverse clk ratio) */
    switch (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_CLKRAT))
    {
      case 1: break;
      case 2: count = (count * 3) / 2; break;
      case 3: count *= 2; break;
      case 4: count *= 3; break;
      case 5: count *= 4; break;
    }

    *(UINT32 *)param = CYCLES2NS( count );
    return OK;
}


/************************************************************************
 *    board_systemram_refresh_ns_msc01_write 
 ************************************************************************/
static UINT32
board_systemram_refresh_ns_msc01_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 refcount;

    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
	/* Do nothing */
	return OK;
    }

    NS2COUNT_ROUND_DOWN( *(UINT32 *)param, refcount );

    /* adjust from bus cycle count to sdram clock cycle count */
    switch (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_CLKRAT))
    {
      case 1: break;
      case 2: refcount = (refcount * 2) / 3; break;
      case 3: refcount /= 2; break;
      case 4: refcount /= 3; break;
      case 5: refcount /= 4; break;
    }

    /* never decrease refresh counter */
    if (refcount > REG(MSC01_MC_REG_BASE, MSC01_MC_TREFRESH))
    {
        /* Write */
	REG(MSC01_MC_REG_BASE, MSC01_MC_TREFRESH) = refcount;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_gt64120_read
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_gt64120_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval );

    switch( REGFIELD( regval, GT_SDRAM_B0_SRASPRCHG ) )
    {
      case GT_SDRAM_B0_SRASPRCHG_2 :
        *(UINT32 *)param = 2;
	break;
      case GT_SDRAM_B0_SRASPRCHG_3 :
        *(UINT32 *)param = 3;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_gt64120_write  
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_gt64120_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;
  
    /* Read */
    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval);

    /* Modify */
    regval &= ~GT_SDRAM_B0_SRASPRCHG_MSK;

    regval |= ( ( ( *(UINT32 *)param <= 2 ) ? 
	           GT_SDRAM_B0_SRASPRCHG_2 : 
		   GT_SDRAM_B0_SRASPRCHG_3 ) << 
		      GT_SDRAM_B0_SRASPRCHG_SHF );

    /* Write */
    sys_func_noram( (t_sys_func_noram)gt64120_write_reg, 
		    KSEG1(sys_nb_base + GT_SDRAM_B0_OFS), 
		    regval,
		    0 );

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_bonito64_read
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_bonito64_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 
        ( BONITO_SDCFG & BONITO_SDCFG_EXTPRECH ) ?
	    3 : 2;

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_bonito64_write  
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_bonito64_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( *(UINT32 *)param <= 2 )
    {
        sys_func_noram( (t_sys_func_noram)bonito64_write_sdcfg,
		        BONITO_SDCFG & ~BONITO_SDCFG_EXTPRECH,
			0,0 );
    }
    else
    {
        sys_func_noram( (t_sys_func_noram)bonito64_write_sdcfg,
			BONITO_SDCFG | BONITO_SDCFG_EXTPRECH,
			0,0 );
    }

    return OK;
}


/************************************************************************
 *    board_systemram_srasprchg_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_srasprchg_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = MC_GET_REG_FIELD(MC_GET_REG(TRP_ADDR),
					TRP_WIDTH, TRP_OFFSET);
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_TIMPAR) &
                                               MSC01_MC_TIMPAR_TRP_MSK) >>
                                               MSC01_MC_TIMPAR_TRP_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_gt64120_read  
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_gt64120_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval );

    switch( REGFIELD( regval, GT_SDRAM_B0_SRAS2SCAS ) )
    {
      case GT_SDRAM_B0_SRAS2SCAS_2 :
        *(UINT32 *)param = 2;
	break;
      case GT_SDRAM_B0_SRAS2SCAS_3 :
        *(UINT32 *)param = 3;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_gt64120_write
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_gt64120_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;
  
    /* Read */
    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval);

    /* Modify */
    regval &= ~GT_SDRAM_B0_SRAS2SCAS_MSK;

    regval |= ( ( ( *(UINT32 *)param <= 2 ) ? 
	           GT_SDRAM_B0_SRAS2SCAS_2 : 
		   GT_SDRAM_B0_SRAS2SCAS_3 ) << 
		      GT_SDRAM_B0_SRAS2SCAS_SHF );

    /* Write */
    sys_func_noram( (t_sys_func_noram)gt64120_write_reg, 
		    KSEG1(sys_nb_base + GT_SDRAM_B0_OFS), 
		    regval,
		    0 );

    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_bonito64_read
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_bonito64_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 
        ( BONITO_SDCFG & BONITO_SDCFG_EXTRASCAS ) ?
	    3 : 2;

    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_bonito64_write  
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_bonito64_write(
    void   *param,
    void   *data,
    UINT32 size )
{
    if( *(UINT32 *)param <= 2 )
    {
        sys_func_noram( (t_sys_func_noram)bonito64_write_sdcfg,
			BONITO_SDCFG & ~BONITO_SDCFG_EXTRASCAS,
			0,0 );
    }
    else
    {
        sys_func_noram( (t_sys_func_noram)bonito64_write_sdcfg,
		        BONITO_SDCFG | BONITO_SDCFG_EXTRASCAS,
			0,0 );
    }

    return OK;
}


/************************************************************************
 *    board_systemram_sras2scas_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_sras2scas_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = MC_GET_REG_FIELD(MC_GET_REG(TRCD_INT_ADDR),
					TRCD_INT_WIDTH, TRCD_INT_OFFSET);
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_TIMPAR) &
                                               MSC01_MC_TIMPAR_TRCD_MSK) >>
                                               MSC01_MC_TIMPAR_TRCD_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_caslat_cycles_gt64120_read
 ************************************************************************/
static UINT32
board_systemram_caslat_cycles_gt64120_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    UINT32 regval;

    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval );

    switch( REGFIELD( regval, GT_SDRAM_B0_CASLAT ) )
    {
      case GT_SDRAM_B0_CASLAT_2 :
        *(UINT32 *)param = 2;
	break;
      case GT_SDRAM_B0_CASLAT_3 :
        *(UINT32 *)param = 3;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_caslat_cycles_bonito64_read
 ************************************************************************/
static UINT32
board_systemram_caslat_cycles_bonito64_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = 
        ( BONITO_SDCFG & BONITO_SDCFG_EXTRDDATA ) ?
	    3 : 2;

    return OK;
}


/************************************************************************
 *    board_systemram_caslat_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_caslat_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = MC_GET_REG_FIELD(MC_GET_REG(CASLAT_LIN_ADDR),
				CASLAT_LIN_WIDTH, CASLAT_LIN_OFFSET) / 2;
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_LATENCY) &
                                               MSC01_MC_LATENCY_CL_MSK) >>
                                               MSC01_MC_LATENCY_CL_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_rw_burstlen_gt64120_read
 ************************************************************************/
static UINT32
board_systemram_rw_burstlen_gt64120_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    /* Same read and write burstlength */

    UINT32 regval;

    GT_L32( sys_nb_base, GT_SDRAM_B0_OFS, regval );

    switch( REGFIELD( regval, GT_SDRAM_B0_BLEN ) )
    {
      case GT_SDRAM_B0_BLEN_4 :
        *(UINT32 *)param = 4;
	break;
      case GT_SDRAM_B0_BLEN_8 :
        *(UINT32 *)param = 8;
	break;
      default : /* Should not happen */
        return ERROR_SYSCON_UNKNOWN_PARAM;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_rw_burstlen_bonito64_read
 ************************************************************************/
static UINT32
board_systemram_rw_burstlen_bonito64_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    switch( (BONITO_SDCFG & BONITO_SDCFG_DRAMBURSTLEN) >>
	    BONITO_SDCFG_DRAMBURSTLEN_SHIFT )
    {
        case 0  : *(UINT32 *)param = 1; break;
	case 1  : *(UINT32 *)param = 2; break;
	case 2  : *(UINT32 *)param = 4; break;
	default : *(UINT32 *)param = 8; break;
    }

    return OK;
}


/************************************************************************
 *    board_systemram_rw_burstlen_msc01_read
 ************************************************************************/
static UINT32
board_systemram_rw_burstlen_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT)
	return !OK;

    *(UINT32 *)param = 
        ( REG(MSC01_MC_REG_BASE, MSC01_MC_HC_DDR) & MSC01_MC_HC_DDR_DDR_BIT ) ?
	    2 : 1;
    return OK;
}


/************************************************************************
 *    board_systemram_cslat_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_cslat_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = 999;
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_LATENCY) &
                                               MSC01_MC_LATENCY_CSL_MSK) >>
                                               MSC01_MC_LATENCY_CSL_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_wrlat_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_wrlat_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT)
	return !OK;

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_LATENCY) &
                                               MSC01_MC_LATENCY_WL_MSK) >>
                                               MSC01_MC_LATENCY_WL_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_rddel_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_rddel_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT)
	return (!OK);

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_RDDEL) &
                                               MSC01_MC_HC_RDDEL_RDDEL_MSK) >>
                                               MSC01_MC_HC_RDDEL_RDDEL_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_rasmin_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_rasmin_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
	*(UINT32 *)param = MC_GET_REG_FIELD(MC_GET_REG(TRAS_MIN_ADDR),
					TRAS_MIN_WIDTH, TRAS_MIN_OFFSET);
	return OK;
    }

    return !OK;
}


/************************************************************************
 *    board_systemram_ras2ras_cycles_msc01_read
 ************************************************************************/
static UINT32
board_systemram_ras2ras_cycles_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
	*(UINT32 *)param = MC_GET_REG_FIELD(MC_GET_REG(TRRD_ADDR),
					TRRD_WIDTH, TRRD_OFFSET);
	return OK;
    }

    return !OK;
}


/************************************************************************
 *    board_systemram_ddr_msc01_read
 ************************************************************************/
static UINT32
board_systemram_ddr_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = 999;
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_DDR) &
                                               MSC01_MC_HC_DDR_DDR_MSK) >>
                                               MSC01_MC_HC_DDR_DDR_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_fw_msc01_read
 ************************************************************************/
static UINT32
board_systemram_fw_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = 2;
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_FMDW) &
                                               MSC01_MC_HC_FMDW_FMDW_MSK) >>
                                               MSC01_MC_HC_FMDW_FMDW_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_clkrat_msc01_read
 ************************************************************************/
static UINT32
board_systemram_clkrat_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(char **)param = "unknown";
	return OK;
    }

    switch (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_CLKRAT))
    {
      case 1: *(char **)param = "1:1"; break;
      case 2: *(char **)param = "3:2"; break;
      case 3: *(char **)param = "2:1"; break;
      case 4: *(char **)param = "3:1"; break;
      case 5: *(char **)param = "4:1"; break;
      default: *(char **)param = "unknown"; break;
    }
    return OK;
}


/************************************************************************
 *    board_systemram_wc_msc01_read
 *    This "performance" bit controls automatic RAM/PCI syncronisation
 ************************************************************************/
static UINT32
board_systemram_wc_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REG(MSC01_BIU_REG_BASE, MSC01_SC_CFG) &
                                                MSC01_SC_CFG_WC_MSK) >>
                                                MSC01_SC_CFG_WC_SHF;
    return OK;
}


/************************************************************************
 *    sysctrl_sysid_msc01_read
 ************************************************************************/
static UINT32
sysctrl_sysid_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = REG(MSC01_BIU_REG_BASE, MSC01_SC_SYSID);
    return OK;
}


/************************************************************************
 *    sysctrl_pbcrev_maj_msc01_read
 ************************************************************************/
static UINT32
sysctrl_pbcrev_maj_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REG(MSC01_PBC_REG_BASE, MSC01_PBC_ID) &
                                                MSC01_PBC_ID_MAR_MSK) >>
                                                MSC01_PBC_ID_MAR_SHF;
    return OK;
}


/************************************************************************

 *    sysctrl_pbcrev_min_msc01_read
 ************************************************************************/
static UINT32
sysctrl_pbcrev_min_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REG(MSC01_PBC_REG_BASE, MSC01_PBC_ID) &
                                                MSC01_PBC_ID_MIR_MSK) >>
                                                MSC01_PBC_ID_MIR_SHF;
    return OK;
}


/************************************************************************
 *    sysctrl_pcirev_maj_msc01_read
 ************************************************************************/
static UINT32
sysctrl_pcirev_maj_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REG(MSC01_PBC_REG_BASE, MSC01_PCI_ID) &
                                                MSC01_PCI_ID_MAR_MSK) >>
                                                MSC01_PCI_ID_MAR_SHF;
    return OK;
}


/************************************************************************
 *    sysctrl_pcirev_min_msc01_read
 ************************************************************************/
static UINT32
sysctrl_pcirev_min_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = (REG(MSC01_PBC_REG_BASE, MSC01_PCI_ID) &
                                                MSC01_PCI_ID_MIR_MSK) >>
                                                MSC01_PCI_ID_MIR_SHF;
    return OK;
}


/************************************************************************
 *    board_systemram_parity_msc01_read
 ************************************************************************/
static UINT32
board_systemram_parity_msc01_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    if(sys_sysconid == MSC01_ID_SC_ROCIT) {
        *(UINT32 *)param = 0;
	return OK;
    }

    *(UINT32 *)param = (REG(MSC01_MC_REG_BASE, MSC01_MC_HC_PARITY) &
                                               MSC01_MC_HC_PARITY_PARITY_MSK) >>
                                               MSC01_MC_HC_PARITY_PARITY_SHF;
    return OK;
}


/************************************************************************
 *    board_sysctrl_regaddrbase_generic_read
 ************************************************************************/
static UINT32
board_sysctrl_regaddrbase_generic_read(
    void   *param,
    void   *data,
    UINT32 size )
{
    *(UINT32 *)param = PHYS( sys_nb_base );
    return OK;
}


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_arch_core_init
 *  Description :
 *  -------------
 *
 *  Initialize core card specific part of SYSCON
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_core_init( 
    t_syscon_obj *objects,		/* Array of SYSCON objects      */
    UINT32	 nb_ram_size,		/* Size of MAX RAM range	*/
    UINT32	 nb_pci_mem_start,      /* PCI memory range start	*/
    UINT32	 nb_pci_mem_size,	/* PCI memory range size 	*/
    UINT32	 nb_pci_mem_offset,	/* PCI memory range offset	*/
    UINT32	 nb_pci_io_start,	/* PCI I/O range start		*/
    UINT32	 nb_pci_io_size,	/* PCI I/O range size 		*/
    UINT32	 nb_pci_io_offset )	/* PCI I/O range offset 	*/
{
    UINT32 n;
    char  *s;
    syscon_objects = objects;

    ram_range_base = 0;	/* RAM must always reside at base address 0 */

    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	goto sead_msc01;

    /* Special handling of Bonito64 */
    if( (sys_corecard == MIPS_REVISION_CORID_BONITO64) ||
        (sys_corecard == MIPS_REVISION_CORID_CORE_20K) ||
        (sys_corecard == MIPS_REVISION_CORID_CORE_EMUL_20K) )
    {
	ram_range_size = BONITO_PCILO_BASE;

        pci_mem_start  = BONITO_PCILO_BASE;
	pci_mem_size   = BONITO_PCILO_SIZE;
	pci_mem_offset = 0;
	pci_io_start   = 0;
	pci_io_size    = BONITO_PCIIO_SIZE;
	pci_io_offset  = BONITO_PCIIO_BASE;

        /* System Controller Version String */

        for (n = 0; n < 31; n ++)
        {
            BONITO_BUILDINFO = n <<  BONITO_BUILDINFO_ADDR_MASK_SHIFT;
            version_syscntrl[n] = (char)BONITO_BUILDINFO;
        }
    }
    /* Special handling of System Controller */
    else if( sys_corecard == MIPS_REVISION_CORID_CORE_SYS ||
             sys_corecard == MIPS_REVISION_CORID_CORE_FPGA2 ||
             sys_corecard == MIPS_REVISION_CORID_CORE_FPGA3 ||
             sys_corecard == MIPS_REVISION_CORID_CORE_24K ||
             sys_corecard == MIPS_REVISION_CORID_CORE_EMUL_SYS )
    {
	int ix;
	int scid;
	ram_range_size = CORE_SYS_PCIMEM_BASE;

        pci_mem_start  = CORE_SYS_PCIMEM_BASE;
	pci_mem_size   = CORE_SYS_PCIIO_BASE - CORE_SYS_PCIMEM_BASE;

	/* On msc01 v1.0, pci_mem_size is limited */
	n = -(REG(MSC01_PCI_REG_BASE,  MSC01_PCI_SC2PMMSKL));
	if (n < pci_mem_size) pci_mem_size = n;

	pci_mem_offset = 0;
	pci_io_start   = 0;
	pci_io_size    = CORE_SYS_PCIIO_SIZE;
	pci_io_offset  = CORE_SYS_PCIIO_BASE;
sead_msc01:
        /* System Controller type */
	n = REG(MSC01_BIU_REG_BASE, MSC01_SC_ID);
	scid = (n & MSC01_SC_ID_ID_MSK) >> MSC01_SC_ID_ID_SHF;
	switch ( scid )
	{
	  case MSC01_ID_SC_EC32:
		strcat(name_msc01, " EC-32");
		break;
	  case MSC01_ID_SC_EC64:
		strcat(name_msc01, " EC-64");
		break;
	  case MSC01_ID_SC_MGB:
		strcat(name_msc01, " MGB");
		break;
	  case MSC01_ID_SC_MGBIIA36D64C0IO:
		strcat(name_msc01, " MGBII");
		break;
	  case MSC01_ID_SC_OCP:
		strcat(name_msc01, " OCP");
		break;
	  case MSC01_ID_SC_ROCIT:
		strcpy(name_msc01, "MIPS ROC-it");
		break;
	}

        /* System Controller Version String */
	n = REG(MSC01_BIU_REG_BASE, MSC01_SC_ID);
	ix = sprintf(version_syscntrl,"%d.%d   ", (n & MSC01_SC_ID_MAR_MSK)
	                                            >> MSC01_SC_ID_MAR_SHF,
	                                          (n & MSC01_SC_ID_MIR_MSK)
	                                            >> MSC01_SC_ID_MIR_SHF);
	if ( scid != MSC01_ID_SC_ROCIT ) {
		n = REG(MSC01_MC_REG_BASE, MSC01_MC_HC_DDR);
		ix += sprintf(&version_syscntrl[ix],
       	               (n & MSC01_MC_HC_DDR_DDR_BIT) ? "DDR" : "SDR");
		n = REG(MSC01_MC_REG_BASE, MSC01_MC_HC_FMDW);
		board_systemram_clkrat_msc01_read(&s, NULL, sizeof(s));
		sprintf(&version_syscntrl[ix],
		    ((n & MSC01_MC_HC_FMDW_FMDW_BIT) ? "-FW-%s" : "-HW-%s"), s);
	}
	else
	{
		ix += sprintf(&version_syscntrl[ix], "FW-1:1 (CLK_unknown)");
	}
    }
    else
    {
	ram_range_size = nb_ram_size;

        pci_mem_start  = nb_pci_mem_start;
	pci_mem_size   = nb_pci_mem_size;
	pci_mem_offset = nb_pci_mem_offset;
	pci_io_start   = nb_pci_io_start;
	pci_io_size    = nb_pci_io_size;
	pci_io_offset  = nb_pci_io_offset;

        /* System Controller Version String */

        pci_config_read32(0,0,0,PCI_CCREV,&n);
        n = (n & PCI_CCREV_REVID_MSK) >> PCI_CCREV_REVID_SHF;
        switch (n)
        {
          case 0x03: strcpy( version_syscntrl, "GT_64120-B-4" );
                     break;
          case 0x10: /* fall through */
          case 0x11: /* fall through */
          case 0x12: strcpy( version_syscntrl, "GT_64120A-B-x" );
                     version_syscntrl[12] = (n & 0xf) | '0';
                     break;
        }
    }

    /**** Register objects ****/

    /* Names of Core cards */

    if (sys_corecard != MIPS_REVISION_CORID_SEAD_MSC01)
    {
        syscon_register_id_core( SYSCON_BOARD_CORECARD_NAME_ID,
	     /* Galileo based */
	     syscon_string_read, 
	         (sys_corecard == MIPS_REVISION_CORID_CORE_LV) ?
		     name_core_lv :
	         (sys_corecard == MIPS_REVISION_CORID_CORE_FPGA ||
		  sys_corecard == MIPS_REVISION_CORID_CORE_FPGAr2) ?
		     name_core_fpga :
		     name_qed_rm5261,
	     NULL,	         NULL,
	     /* SysCtrl based */
	      syscon_string_read,
	         (sys_corecard == MIPS_REVISION_CORID_CORE_SYS) ?
		     name_core_sys :
	         (sys_corecard == MIPS_REVISION_CORID_CORE_FPGA2) ?
		     name_core_fpga2 :
		 (sys_corecard == MIPS_REVISION_CORID_CORE_FPGA3) ?
		     name_core_fpga3 :
		 (sys_corecard == MIPS_REVISION_CORID_CORE_24K) ?
		     name_core_24K : name_core_emul,
	     NULL,		 NULL,
	     /* Bonito64 based */
	     syscon_string_read, 
	         (sys_corecard == MIPS_REVISION_CORID_BONITO64) ?
		     name_core_bonito :
	         (sys_corecard == MIPS_REVISION_CORID_CORE_20K) ?
		     name_core_20k : name_core_emul,
	     NULL,		 NULL );
    }

    /* Names/revision of System controller */

    syscon_register_id_core( SYSCON_SYSCTRL_NAME_ID,
	     /* Galileo based */
	     syscon_string_read, (void *)name_galileo,
	     NULL,	         NULL,
	     /* SysCtrl based */
	     syscon_string_read, (void *)name_msc01,
	     NULL,		 NULL,
	     /* Bonito64 based */
	     syscon_string_read, (void *)&name_core_bonito[4],
	     NULL,		 NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_REV_ID,
	     /* Galileo based */
	     syscon_string_read, (void *)version_syscntrl,
	     NULL,	         NULL,
	     /* SysCtrl based */
	     syscon_string_read, (void *)version_syscntrl,
	     NULL,		 NULL,
	     /* Bonito64 based */
	     syscon_string_read, (void *)version_syscntrl,
	     NULL,		 NULL );

    /* SDRAM parameters */

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_BASE_ID,
	     syscon_uint32_read, (void *)&ram_range_base,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_BOARD_SYSTEMRAM_SIZE_ID,
	     syscon_uint32_read, (void *)&ram_range_size,
	     NULL,		 NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_CFG_ID,
	     /* Galileo based */
	     board_systemram_refresh_ns_gt64120_read,   NULL,
	     board_systemram_refresh_ns_gt64120_write,  NULL,
	     /* SysCtrl based */
	     board_systemram_refresh_ns_msc01_read,     NULL,
	     board_systemram_refresh_ns_msc01_write,    NULL,
	     /*  Bonito64 based
	      *  No write, since refresh is configured by setting :
	      *
	      *  1) CPUCLOCKPERIOD field of Bonito64 IODEVCFG register 
	      *  2) DRAMRFSHMULT   field of Bonito64 SDCFG regiter.
	      *  
	      *  This is done by initialisation code.
	      */
	     board_systemram_refresh_ns_bonito64_read,  NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_CYCLES_CFG_ID,
	     /* Galileo based */
	     board_systemram_srasprchg_cycles_gt64120_read,   NULL,
	     board_systemram_srasprchg_cycles_gt64120_write,  NULL,
	     /* SysCtrl based */
	     board_systemram_srasprchg_cycles_msc01_read,     NULL,
	     NULL,				              NULL,
	     /* Bonito64 based */
	     board_systemram_srasprchg_cycles_bonito64_read,  NULL,
	     board_systemram_srasprchg_cycles_bonito64_write, NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_CYCLES_CFG_ID,
	     /* Galileo based */
	     board_systemram_sras2scas_cycles_gt64120_read,   NULL,
	     board_systemram_sras2scas_cycles_gt64120_write,  NULL,
	     /* SysCtrl based */
	     board_systemram_sras2scas_cycles_msc01_read,     NULL,
	     NULL,				              NULL,
	     /* Bonito64 based */
	     board_systemram_sras2scas_cycles_bonito64_read,  NULL,
	     board_systemram_sras2scas_cycles_bonito64_write, NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_CASLAT_CYCLES_CFG_ID,
	     /* Galileo based */
	     board_systemram_caslat_cycles_gt64120_read,  NULL,
	     NULL,					  NULL,
	     /* SysCtrl based */
	     board_systemram_caslat_cycles_msc01_read,    NULL,
	     NULL,					  NULL,
	     /* Bonito64 based */
	     board_systemram_caslat_cycles_bonito64_read, NULL,
	     NULL,					  NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_READ_BURSTLEN_CFG_ID,
	     /* Galileo based */
	     board_systemram_rw_burstlen_gt64120_read,  NULL,
	     NULL,				        NULL,
	     /* SysCtrl based */
	     board_systemram_rw_burstlen_msc01_read,    NULL,
	     NULL,				        NULL,
	     /* Bonito64 based */
	     board_systemram_rw_burstlen_bonito64_read, NULL,
	     NULL,				        NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_WRITE_BURSTLEN_CFG_ID,
	     /* Galileo based */
	     board_systemram_rw_burstlen_gt64120_read,  NULL,
	     NULL,				        NULL,
	     /* SysCtrl based */
	     board_systemram_rw_burstlen_msc01_read,    NULL,
	     NULL,				        NULL,
	     /* Bonito64 based */
	     board_systemram_rw_burstlen_bonito64_read, NULL,
	     NULL,				        NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_CSLAT_CYCLES_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_cslat_cycles_msc01_read,	NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_WRLAT_CYCLES_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_wrlat_cycles_msc01_read,	NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_RDDEL_CYCLES_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_rddel_cycles_msc01_read,	NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_RASMIN_CYCLES_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_rasmin_cycles_msc01_read,	NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_RAS2RAS_CYCLES_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_ras2ras_cycles_msc01_read,	NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_DDR_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_ddr_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_FW_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_fw_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_CLKRAT_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_clkrat_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_BOARD_SYSTEMRAM_PARITY_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_parity_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_WC_CFG_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     board_systemram_wc_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_generic( SYSCON_SYSCTRL_REGADDR_BASE_ID,
	     board_sysctrl_regaddrbase_generic_read,	NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_SYSID_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     sysctrl_sysid_msc01_read,			NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_PBCREV_MAJOR_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     sysctrl_pbcrev_maj_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_PBCREV_MINOR_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     sysctrl_pbcrev_min_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    if (sys_corecard == MIPS_REVISION_CORID_SEAD_MSC01)
	return;

    syscon_register_id_core( SYSCON_SYSCTRL_PCIREV_MAJOR_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     sysctrl_pcirev_maj_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    syscon_register_id_core( SYSCON_SYSCTRL_PCIREV_MINOR_ID,
	     /* Galileo based */
	     NULL,				        NULL,
	     NULL,					NULL,
	     /* SysCtrl based */
	     sysctrl_pcirev_min_msc01_read,		NULL,
	     NULL,					NULL,
	     /* Bonito64 based */
	     NULL,				        NULL,
	     NULL,					NULL );

    /* PCI parameters */

    syscon_register_generic( SYSCON_CORE_PCI_MEM_START,
	     syscon_uint32_read, (void *)&pci_mem_start,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_CORE_PCI_MEM_SIZE,
	     syscon_uint32_read, (void *)&pci_mem_size,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_CORE_PCI_MEM_OFFSET,
	     syscon_uint32_read, (void *)&pci_mem_offset,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_CORE_PCI_IO_START,
	     syscon_uint32_read, (void *)&pci_io_start,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_CORE_PCI_IO_SIZE,
	     syscon_uint32_read, (void *)&pci_io_size,
	     NULL,		 NULL );

    syscon_register_generic( SYSCON_CORE_PCI_IO_OFFSET,
	     syscon_uint32_read, (void *)&pci_io_offset,
	     NULL,		 NULL );
}   


/************************************************************************
 *
 *                          syscon_register_id_core
 *  Description :
 *  -------------
 *
 *  Function used to register core card specific SYSCON object functions.
 *
 *  A read and/or write function may be registered for each core card.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed for the particular platform.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.

 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_id_core(
    t_syscon_ids   id,			  /* OBJECT ID from syscon_api.h */

    /* Galileo based */
    t_syscon_func  read_galileo,	  /* Core Galileo read function  */
    void	   *read_data_galileo,	  /* Registered data		 */
    t_syscon_func  write_galileo,	  /* Core Galileo write function */
    void	   *write_data_galileo,   /* Registered data		 */

    /* SysCtrl based */
    t_syscon_func  read_sysctl,	          /* Core SysCtrl read function	 */
    void	   *read_data_sysctl,     /* Registered data		 */
    t_syscon_func  write_sysctl,	  /* Core SysCtrl write function */
    void	   *write_data_sysctl,    /* Registered data		 */

    /* Bonito64 based */
    t_syscon_func  read_bonito64,	  /* Core Bonito64 read          */
    void	   *read_data_bonito64,	  /* Registered data		 */
    t_syscon_func  write_bonito64,	  /* Core Bonito64 write	 */
    void	   *write_data_bonito64 ) /* Registered data		 */
{
    t_syscon_obj *obj;

    obj = &syscon_objects[id];

    switch( sys_corecard )
    {
      case MIPS_REVISION_CORID_CORE_LV :
      case MIPS_REVISION_CORID_CORE_FPGA :
      case MIPS_REVISION_CORID_QED_RM5261 :
      case MIPS_REVISION_CORID_CORE_FPGAr2 :
        obj->read       = read_galileo;
	obj->read_data  = read_data_galileo;
	obj->write      = write_galileo;
	obj->write_data = write_data_galileo;
	break;

      case MIPS_REVISION_CORID_CORE_SYS :
      case MIPS_REVISION_CORID_CORE_FPGA2 :
      case MIPS_REVISION_CORID_CORE_FPGA3 :
      case MIPS_REVISION_CORID_CORE_24K :
      case MIPS_REVISION_CORID_CORE_EMUL_SYS :
      case MIPS_REVISION_CORID_SEAD_MSC01 :
        obj->read       = read_sysctl;
	obj->read_data  = read_data_sysctl;
	obj->write      = write_sysctl;
	obj->write_data = write_data_sysctl;
	break;

      case MIPS_REVISION_CORID_BONITO64 :
      case MIPS_REVISION_CORID_CORE_20K :
      case MIPS_REVISION_CORID_CORE_EMUL_20K :
        obj->read       = read_bonito64;
	obj->read_data  = read_data_bonito64;
	obj->write      = write_bonito64;
	obj->write_data = write_data_bonito64;
	break;

      /* Add new core cards here */

      default : /* Should not happen */
        obj->read  = NULL;
	obj->write = NULL;
	break;
    }    
}

