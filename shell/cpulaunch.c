/************************************************************************
 *
 *  cpulaunch.c
 *
 *  Monitor command for controlling CMP
 *  MIPSCMP
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
#include <gcmp.h>
#include <launch.h>
#include <malta.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/
extern bool gcmp_present;

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_HELP	0
  { "h", "help"	           },
};


/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          gcmp
 ************************************************************************/
static MON_FUNC(gcmp)
{
    return OK;
}


static t_cmd cmd_def =
{
    "gcmp",
    gcmp,

    "gcmp\n",

    "GCMP control\n"
    "\n"
    ,
    options,
    sizeof(options)/sizeof(options[0]),
    FALSE
};

#ifdef DEBUGLAUNCH
void
putn (int n)
{
    char buf[20];
    sprintf (buf, "%d", n);
    SHELL_PUTS(buf);
}
#endif

static void gcmp_requester(int i)
{
  cpulaunch_t *cpulaunch = (cpulaunch_t *)KSEG0(CPULAUNCH);
  unsigned int type;
  unsigned int gcmp = KSEG1(GCMPBASE); 
  unsigned int gcr = GCMPGCB(gcmp, GC);
  int timeout;

  GCMPCLCB(gcmp,OTHER) = i << GCMP_CCB_OTHER_CORENUM_SHF;
  type = (GCMPCOCB(gcmp,CFG) & GCMP_CCB_CFG_IOCUTYPE_MSK) >> GCMP_CCB_CFG_IOCUTYPE_SHF;
#ifdef DEBUGLAUNCH
  SHELL_PUTS ("Requester ");
  putn (i);
  SHELL_PUTS (": ");
#endif

  switch (type) {
  case GCMP_CCB_CFG_IOCUTYPE_CPU:
#ifdef DEBUGLAUNCH
    SHELL_PUTS ("CORE ");
#endif
    if (GCMPCOCB(gcmp,COHCTL) != 0) {
      /* Should not happen */
#ifdef DEBUGLAUNCH
      SHELL_PUTS ("[cleared COHCTL] ");
#endif
      GCMPCOCB(gcmp,COHCTL) = 0;
    }
    /* allow CPUi access to the GCR registers */
    GCMPGCB(gcmp, GCSRAP) |= 1 << (i + GCMP_GCB_GCSRAP_CMACCESS_SHF);
    /* start CPUi */
    GCMPCOCB(gcmp,RESETR) = 0;
    /* Poll the Coherence Control register waiting for it to join the domain */
#define TIMEOUT 100000
    /*
     * TIMEOUT should be independent of platform (LV, FPGA, EMULATION)
     * and may need tuning
     */
    for (timeout = TIMEOUT; (timeout >= 0) && (GCMPCOCB(gcmp,COHCTL) == 0); timeout--)
      ;
    if (timeout > 0) {
#ifdef DEBUGLAUNCH
      SHELL_PUTS ("started\n");
#endif
      /* could poll the ready bits for each VPE on this core here... */
    }
#ifdef DEBUGLAUNCH
    else
      SHELL_PUTS ("not responding\n");
#endif
    break;

  case GCMP_CCB_CFG_IOCUTYPE_NCIOCU:
#ifdef DEBUGLAUNCH
    SHELL_PUTS ("NIOCU\n");
#endif
    break;

  case GCMP_CCB_CFG_IOCUTYPE_CIOCU:
#ifdef DEBUGLAUNCH
    SHELL_PUTS ("CIOCU enabled\n");
#endif
    GCMPCOCB(gcmp,COHCTL) = 0xff;
    break;

  default:
#ifdef DEBUGLAUNCH
    SHELL_PUTS ("Unrecognised type\n");
#endif
    break;
  }
}

static void
gcmp_start_cores(void)
{
    unsigned int ncores, niocus;
    unsigned int nrequesters;

    unsigned int gcmp = KSEG1(GCMPBASE); 
    unsigned int gcr = GCMPGCB(gcmp, GC);
    int i;

    niocus = ((gcr & GCMP_GCB_GC_NUMIOCU_MSK) >> GCMP_GCB_GC_NUMIOCU_SHF);
    ncores = ((gcr & GCMP_GCB_GC_NUMCORES_MSK) >> GCMP_GCB_GC_NUMCORES_SHF) + 1;
    nrequesters = niocus + ncores;

#ifdef DEBUGLAUNCH
    putn (ncores);
    SHELL_PUTS (" core");
    if (ncores > 1)
      SHELL_PUTS ("s");
    SHELL_PUTS (", ");
      
    putn (niocus);
    SHELL_PUTS (" iocu");
    if (niocus > 1)
      SHELL_PUTS ("s");
    SHELL_PUTS ("\n");
#endif

    /* requester 0 is already in the coherency domain */
    for (i = 1; i < ncores; i++) {
      gcmp_requester(i);
    }
    for (i = 0; i < niocus; i++) {
      gcmp_requester(4+i);
    }
}

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
shell_cpu_init( void )
{
    cpulaunch_t *cpulaunch = (cpulaunch_t *)KSEG0(CPULAUNCH);

#ifdef DEBUGLAUNCH
    (void) SHELL_PUTS("Configuring secondary CPU's\n");	/* FIXME: debug */
#endif

    /* Clear the handshake locations before kicking off the secondary CPU's */
    memset (cpulaunch, 0, NCPULAUNCH*sizeof(cpulaunch_t));

    /* Fixup CPU#0 (us!) */
    cpulaunch->flags = LAUNCH_FREADY|LAUNCH_FGO|LAUNCH_FGONE;

    /* DEBUG: use S5-3 to select multicpu boot */
    if ((REGA(KSEG1(MALTA_STATUS)) & 0x04) == 0)
      return NULL;

    if (sys_mt) {
	/* Enable the remaining VPE's on this core */
	asm(
	    ".set push\n\t"
	    ".set mips32r2\n\t"
	    ".set mt\n\t"
	    "evpe\n\t"
	    "ehb\n\t"
	    ".set pop"
	    );
    }

    /* For GCMP systems we know how to start other cores */
    if (!gcmp_present)
      return NULL;

    gcmp_start_cores ();

    return &cmd_def;
}

int
cpu_present (int cpu)
{
  cpulaunch_t *cpulaunch = (cpulaunch_t *)KSEG0(CPULAUNCH);
  return cpulaunch[cpu].flags & LAUNCH_FREADY;
}
