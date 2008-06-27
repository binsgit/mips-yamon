
/************************************************************************
 *
 *  gic.c
 *
 *  GIC support
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

#include <sysdefs.h>
#include <gic.h>
#include <core_sys.h>
#include <syscon_api.h>
#include <shell_api.h>
#include <gcmp.h>

extern bool gic_present;
extern bool gcmp_present;

/*
 * We assume that ROCit-2 implies that the GIC is present; now validate this.
 */
int gic_probe(void)
{
    if (gcmp_present) {
	GCMPGCB(GCMPBASE,GICBA) = GIC_BASE_ADDR | GCMP_GCB_GICBA_EN_MSK;
	return (1);
    }
    else {
	/* Check if the GIC is present */
	UINT32 r = (REG(MSC01_BIU_REG_BASE, MSC01_SC_CFG) & 
		    MSC01_SC_CFG_GICPRES_MSK) >> MSC01_SC_CFG_GICPRES_SHF;
	return (r);
    }
}

static int setup_eic(void)
{
	UINT32 data;

	/* Map Interrupt sources */

	/* Ask GIC to switch to EIC mode */
	GICREAD(GIC_REG(VPE_LOCAL, GIC_VPE_CTL), data);
	data |= GIC_VPE_CTL_EIC_MODE_MSK;
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_CTL), data);

	return (0);
}

static void setup_polarities(UINT32 numintrs)
{
	/* Setup polarity for the interrupts of interest */
	GICWRITE(GIC_REG(SHARED,GIC_SH_POL_31_0), 0x7FFF);
}

static unsigned int setup_triggers(UINT32 numintrs)
{
	GICWRITE(GIC_REG(SHARED,GIC_SH_TRIG_31_0), 0);
}

static void setup_pin_maps(UINT32 numintrs)
{
	UINT32 r;

	/* GIC Global Src 3 -> CPU Int 0 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(3)),  GIC_MAP_TO_PIN_MSK | 0);
	/* GIC Global Src 4 -> CPU Int 1 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(4)),  GIC_MAP_TO_PIN_MSK | 1);
	/* GIC Global Src 5 -> CPU Int 2 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(5)),  GIC_MAP_TO_PIN_MSK | 2);
	/* GIC Global Src 6 -> CPU Int 3 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(6)),  GIC_MAP_TO_PIN_MSK | 3);
	/* GIC Global Src 7 -> CPU Int 4 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(7)),  GIC_MAP_TO_PIN_MSK | 4);
	/* GIC Global Src 8 -> CPU Int 3 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(8)),  GIC_MAP_TO_PIN_MSK | 3);
	/* GIC Global Src 9 -> CPU Int 3 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(9)),  GIC_MAP_TO_PIN_MSK | 3);
	/* GIC Global Src 12 -> CPU Int 3 */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(12)), GIC_MAP_TO_PIN_MSK | 3);
	/* GIC Global Src 13 -> NMI */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(13)), GIC_MAP_TO_NMI_MSK);
	/* GIC Global Src 14 -> NMI */
	GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(14)), GIC_MAP_TO_NMI_MSK);
}

/*
 * MAP_TO_PIN[MAP_TO_YQ] must be clear for every interrupt, both
 * global and local (which is the default reset state, right?)
 * in order to map to VPEs rather than cores.
 */
static void setup_vpe_maps(UINT32 numintrs)
{
	UINT32 intr;

	/* Map All interrupts to VPE 0 */
	for (intr = 0; intr < numintrs; intr++) {
	    GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_VPE_REG_OFF(intr, 0)),
		     GIC_SH_MAP_TO_VPE_REG_BIT(0));
	}
}

static int vpe_local_setup(int numvpes) 
{
    int i;
    UINT32 timer_interrupt = 5, perf_interrupt = 5;
    UINT32 data;

    /*
     * If we wanted to be fancy, the interrupt values
     * could be overridden from the environment
     */

    /*
     * Setup the default performance and counter timer
     * interrupts for all VPE's
     */
    for (i = 0; i < numvpes; i++) {
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);

	/* Are Interrupts locally routable? */
	GICREAD(GIC_REG(VPE_OTHER, GIC_VPE_CTL), data);
	if (data & GIC_VPE_CTL_TIMER_RTBL_MSK)
	    GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_TIMER_MAP), 
		     GIC_MAP_TO_PIN_MSK | timer_interrupt);

	GICREAD(GIC_REG(VPE_OTHER, GIC_VPE_CTL), data);
	if (data & GIC_VPE_CTL_PERFCNT_RTBL_MSK)
	    GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_PERFCTR_MAP), 
		     GIC_MAP_TO_PIN_MSK | perf_interrupt);
    }

    return (0);
}

static void setup_maps(int numintrs, int numvpes)
{
	/* Map to Pins */
	setup_pin_maps(numintrs);
	vpe_local_setup(numvpes);

	/* Map to VPEs */
	setup_vpe_maps(numintrs);
}

static void set_interrupt_masks(void)
{
	/* Unmask only the interrupts we're interested in */
	GICWRITE(GIC_REG(SHARED, GIC_SH_SMASK_31_0),    0x00007fff);
}

int gic_init(void)
{
	UINT32 gicconfig, numintrs, numvpes, data;

	if (gic_present) {
		DISP_STR("GIC_PRSNT");
	}
	else {
		DISP_STR("GIC_ABSNT");
		return (0);
	}

	/* EIC? */
	if (sys_eicmode) {
		setup_eic();
	}

	GICREAD(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
	numvpes = (gicconfig & GIC_SH_CONFIG_NUMVPES_MSK) >> GIC_SH_CONFIG_NUMVPES_SHF;

	/* How many external Interrupts in the system ? */
	numintrs = (gicconfig & GIC_SH_CONFIG_NUMINTRS_MSK) >> GIC_SH_CONFIG_NUMINTRS_SHF;
	numintrs = ((numintrs + 1) * 8);

	/* Setup Polarities and triggers */
	setup_polarities(numintrs);
	setup_triggers(numintrs);
	setup_maps(numintrs, numvpes);
	set_interrupt_masks();

	/* Some CMP enabled FPGA's still need this bit set to route interrupts correctly */
	if (!gcmp_present || 1) {
	    /* Enable the GIC */
	    data = REG(MSC01_BIU_REG_BASE, MSC01_SC_CFG);
	    REG(MSC01_BIU_REG_BASE, MSC01_SC_CFG) =
		(data | (0x1 << MSC01_SC_CFG_GICENA_SHF));
	}

	return 0;
}
