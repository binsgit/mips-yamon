
/************************************************************************
 *
 *  fpu.c
 *
 *  fpu setup
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
#include <shell_api.h>
#include <shell.h>
#include <stdio.h>
//#include <varargs.h>
#include <stdarg.h>
#include <string.h>
#include <ArchDefs.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

#define CMD_NONE 0
#define CMD_ON   1
#define CMD_OFF  2
#define CMD_ONOFF (CMD_ON | CMD_OFF)
#define CMD_STAT 4
#define CMD_SCLR 8
#define CMD_SHOW 16
#define CMD_NOFPU 32

/************************************************************************
 *  Public variables
 ************************************************************************/

bool FPUEMUL_status;
bool FPUEMUL_soft_fr;
bool shell_fpu_warning;
extern UINT32 FPUEMUL_csr;

/************************************************************************
 *  Static variables
 ************************************************************************/
static bool emul_init;
static char *on_off[2] = { "off", "on" };


/************************************************************************
 *  Static function prototypes
 ************************************************************************/
extern int _doprntx(char *fmt0, va_list argp, void (*putc)(char, char **),
	char **pca);

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/
static void putch(char ch, char **pca)
{
    PUTCHAR(DEFAULT_PORT, ch);
}


static int myprintf(const char *fmt, ...)
{
    va_list args;
    int ret;

    if (emul_init)
	return 0;

    va_start(args,fmt);
    ret = _doprntx((unsigned char *)fmt, args, putch, NULL);
    va_end(args);
    return ret;
}


static void set_fs_fr(int fs_bit, int fr_bit)
{
    UINT32 cp0_status;
    UINT32 cp1_csr;

    FPUEMUL_soft_fr = fr_bit;
    FPUEMUL_csr |= M_FCSRFS;
    if (fs_bit == 0) FPUEMUL_csr ^= M_FCSRFS;

    if (sys_fpu) {
	cp0_status = sys_cp0_read32(R_C0_Status, R_C0_SelStatus);
	if (fr_bit)
	    cp0_status |= M_StatusFR;
	else
	    cp0_status &= ~M_StatusFR;
	sys_cp0_write32(R_C0_Status, R_C0_SelStatus, cp0_status);

	cp1_csr = sys_cp1_read(R_C1_FCSR);
	if (fs_bit)
	    cp1_csr |= M_FCSRFS;
	else
	    cp1_csr &= ~M_FCSRFS;
	sys_cp1_write(R_C1_FCSR, cp1_csr);
    }
}


/************************************************************************
 *                          parse_args
 ************************************************************************/
static UINT32 parse_args(UINT32 argc, char **argv,
                         int *fpucmd, int *emulcmd, int *fscmd, int *frcmd)
{
    if (argc == 1) return OK;
    argc--;
    while (argc) {
	argv++; argc--;
	if (strcmp(*argv, "emul") == 0) {
	    if (argc) {
		argv++; argc--;
		if (strcmp(*argv, on_off[0]) == 0) {
		    *emulcmd = (*emulcmd & ~CMD_ONOFF) | CMD_OFF;
		    continue;
		}
		if (strcmp(*argv, on_off[1]) == 0) {
		    *emulcmd = (*emulcmd & ~CMD_ONOFF) | CMD_ON;
		    continue;
		}
		if (strcmp(*argv, "stat") == 0) {
		    *emulcmd |= CMD_STAT;
		    continue;
		}
		if (strcmp(*argv, "clear") == 0) {
		    *emulcmd |= CMD_SCLR;
		    continue;
		}
		argv--; argc++;
	    }
	    *emulcmd |= CMD_SHOW;
	    continue;
	}

	if (strcmp(*argv, "fs") == 0) {
	    if (argc) {
		argv++; argc--;
		if (strcmp(*argv, on_off[0]) == 0) {
		    *fscmd = (*fscmd & ~CMD_ONOFF) | CMD_OFF;
		    continue;
		}
		if (strcmp(*argv, on_off[1]) == 0) {
		    *fscmd = (*fscmd & ~CMD_ONOFF) | CMD_ON;
		    continue;
		}
		argv--; argc++;
	    }
	    *fscmd |= CMD_SHOW;
	    continue;
	}

	if (strcmp(*argv, "fr") == 0) {
	    if (argc) {
		argv++; argc--;
		if (strcmp(*argv, on_off[0]) == 0) {
		    *frcmd = (*frcmd & ~CMD_ONOFF) | CMD_OFF;
		    continue;
		}
		if (strcmp(*argv, on_off[1]) == 0) {
		    *frcmd = (*frcmd & ~CMD_ONOFF) | CMD_ON;
		    continue;
		}
		argv--; argc++;
	    }
	    *frcmd |= CMD_SHOW;
	    continue;
	}

	if (strcmp(*argv, on_off[0]) == 0) {
	    *fpucmd = (*fpucmd & ~CMD_ONOFF) | CMD_OFF;
	    continue;
	}

	if (strcmp(*argv, on_off[1]) == 0) {
	    if (sys_fpu) {
	        *fpucmd = (*fpucmd & ~CMD_ONOFF) | CMD_ON;
		continue;
	    }
	    *fpucmd |= CMD_NOFPU;
	    continue;
	}
	return SHELL_ERROR_OPTION;
    }
    return OK;
}


/************************************************************************
 *                          fpu
 ************************************************************************/
static MON_FUNC(fpu)
{
    UINT32 cp0_status;
    UINT32 cp1_csr;
    int fr_bit, fs_bit;
    extern void FPUEMUL_stat_print();
    extern void FPUEMUL_stat_clear();
    int fpucmd, emulcmd, fscmd, frcmd;
    UINT32 rc;

    fpucmd = emulcmd = fscmd = frcmd = CMD_NONE;

    rc = parse_args(argc, argv, &fpucmd, &emulcmd, &fscmd, &frcmd);
    if (rc != OK) return rc;

    cp0_status = sys_cp0_read32(R_C0_Status, R_C0_SelStatus);
    if (sys_fpu && (cp0_status & M_StatusCU1) != 0) {
	cp1_csr = sys_cp1_read(R_C1_FCSR);
	fr_bit = (cp0_status & M_StatusFR) != 0;
	fs_bit = (cp1_csr & M_FCSRFS) != 0;
    }
    else {
	cp1_csr = FPUEMUL_csr;
	fr_bit = FPUEMUL_soft_fr;
	fs_bit = (cp1_csr & M_FCSRFS) != 0;
    }
    if (argc == 1) {
	if (sys_fpu) {
	    myprintf("FPU %s", on_off[(cp0_status & M_StatusCU1) != 0]);
	    myprintf(", emul %s", on_off[FPUEMUL_status]);
	    myprintf(", fs %s, fr %s\n", on_off[fs_bit], on_off[fr_bit]);
	}
	else if (FPUEMUL_status) {
	    myprintf("FPU emul on");
	    myprintf(", fs %s, fr %s\n", on_off[fs_bit], on_off[fr_bit]);
	}
	else {
	    myprintf("You don't have an FPU. Use \"fpu emul\" "
		    "to enable emulator\n");
	}
	return OK;
    }

    if (fpucmd != CMD_NONE) {
	if (fpucmd & CMD_NOFPU) {
	    myprintf("You don't have an FPU. Use \"fpu emul\" "
		    "to control emulator\n");
	    return OK;
	}
	if (fpucmd & CMD_OFF) {
	    cp0_status &= ~M_StatusCU1;
	    sys_cp0_write32(R_C0_Status, R_C0_SelStatus, cp0_status);
	    set_fs_fr(fs_bit, fr_bit);
	}
	else if (fpucmd & CMD_ON) {
	    cp0_status |= M_StatusCU1;
	    sys_cp0_write32(R_C0_Status, R_C0_SelStatus, cp0_status);
	    set_fs_fr(fs_bit, fr_bit);
	}
	else if (fpucmd & CMD_SHOW) {
	    myprintf("FPU %s\n", on_off[(cp0_status & M_StatusCU1) != 0]);
	}
    }

    if (emulcmd != CMD_NONE) {
	if (emulcmd & CMD_OFF) {
	    FPUEMUL_status = 0;
	}
	if (emulcmd & CMD_ON) {
	    FPUEMUL_status = 1;
	}
	if (emulcmd & CMD_STAT) FPUEMUL_stat_print(myprintf);
	if (emulcmd & CMD_SCLR) FPUEMUL_stat_clear();
	if (emulcmd & (CMD_ON | CMD_OFF | CMD_SHOW)) {
	    myprintf("FPU emul %s\n", on_off[FPUEMUL_status]);
	}
    }

    if (fscmd != CMD_NONE) {
	if (fscmd & CMD_OFF) {
	    fs_bit = 0;
	    set_fs_fr(fs_bit, fr_bit);
	}
	if (fscmd & CMD_ON) {
	    fs_bit = 1;
	    set_fs_fr(fs_bit, fr_bit);
	}
	myprintf("FPU fs %s\n", on_off[fs_bit]);
    }

    if (frcmd != CMD_NONE) {
	if (frcmd & CMD_OFF) {
	    fr_bit = 0;
	    set_fs_fr(fs_bit, fr_bit);
	}
	if (frcmd & CMD_ON) {
	    fr_bit = 1;
	    set_fs_fr(fs_bit, fr_bit);
	}
	myprintf("FPU fr %s\n", on_off[fr_bit]);
    }

    /* Determine whether fpu configuration is default:
     *  Without hw fpu: "fpuemul off",
     *  with hw fpu: "fpu on  fpuemul off  fs on  fr on".
     */
    shell_fpu_warning = FPUEMUL_status;
    if (sys_fpu) {
	shell_fpu_warning |= !(cp0_status & M_StatusCU1) || !fs_bit || !fr_bit;
    }

    return OK;
}


static bool decode(char *raw, void *decoded, UINT32 size)
{
    char line[80];
    char *argv[20];
    int argc;
    int retval;
    int fpucmd, emulcmd, fscmd, frcmd;

    strncpy(line, raw, sizeof(line)-1);
    argc = 0;
    argv[argc++] = NULL;
    argv[argc] = strtok(line, " ");
    while (argv[argc] && ++argc < 20) {
	argv[argc] = strtok(NULL, " ");
    }
    if (emul_init)
    {
	/* At initialization, execute an "fpu $fpu" command */
	retval = fpu(argc, argv);
    }
    else
    {
	/* Normally, just parse argument string */
	retval = parse_args(argc, argv, &fpucmd, &emulcmd, &fscmd, &frcmd);
    }
    return retval == OK;
}


/* Command definition for help */
static t_cmd cmd_def =
{
    "fpu",
     fpu,
    "fpu [on|off] [emul [on|off|stat|clear]] [fs [on|off]] [fr [on|off]]",
    "Controls the floating point unit and the FPU emulator. The above\n"
    "specified parameters can be set in the environment variable \"fpu\"\n"
    "to control settings at startup.\n"
    "The FPU emulator supports all FP operations specified in MIPS32/MIPS64s -\n"
    "except 64 bit addressing, Paired Single and extended (ASE) FP operations."
    "\n\nArguments are:\n\n"
    "<blank>    Show current status\n"
    "\n"
    "Applicable when FPU is present\n"
    "------------------------------\n"
    "on   Enable the hardware FPU.\n"
    "off  Disable the hardware FPU.\n"
    "\n"
    "Always applicable\n"
    "------------------\n"
    "emul        Show current status of emulator.\n"
    "emul on     Enable the emulator. If the system has a hardware FPU which\n"
    "            is enabled, the emulator will handle denormalized and out of\n"
    "            range numbers. If the system lacks an FPU or the FPU is\n"
    "            disabled, the emulator will handle all FP operations.\n"
    "emul off    Disable the emulator. If the system has a hardware FPU which\n"
    "            is enabled, FP operations on denormalized and out of range\n"
    "            numbers will cause an exception. If the system lacks an FPU\n"
    "            or the FPU is disabled, all FP operations will cause an\n"
    "            exception.\n"
    "emul stat   Show emulator statistics.\n"
    "emul clear  Clear emulator statistics.\n"
    "\n"
    "fs      Show current status of FS.\n"
    "fs on   Enable the flush to zero bit. When the FS bit is set,\n"
    "        denormalized numbers will be replaced by zero (This is not IEEE\n"
    "        compliant).\n"
    "fs off  Disable the flush to zero bit. Denormalized numbers must be\n"
    "        handled by the emulator.\n"
    "\n"
    "fr      Show current status of FR.\n"
    "fr on   Enable the FR bit. All 32 double-sized FP registers are exposed\n"
    "fr off  Disable the FR bit. Only 16 double-sized FP registers (even\n"
    "        numbers) are exposed.",
    NULL,
    0,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_fpuemul_init
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
shell_fpuemul_init( void )
{
    char *fpu_env;

    /* set up default values consistent with hw setup */
    FPUEMUL_soft_fr = TRUE;   /* corresponding hw value defined in init.h */
    FPUEMUL_csr |= M_FCSRFS;  /* corresponding hw value set up in cpu_s.S */

    emul_init = TRUE;
    if (!env_get("fpu", &fpu_env, NULL, 0) || !decode(fpu_env, NULL, 0))
	fpu_env = "";
    env_set("fpu", fpu_env, ENV_ATTR_RW, "", decode);
    emul_init = FALSE;
    return &cmd_def;
}
