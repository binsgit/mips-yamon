
/************************************************************************
 *
 *  dis.c
 *
 *  Monitor command for disassembling an address range
 *
 *  Disassembles MIPS64/MIPS32 instructions.
 *  Includes MIPS-3D(TM) and MIPS16e(TM) ASEs.
 *
 *  dis [-m] [-16] <address> [<count>]
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
#include <shell.h>
#include <mips.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

typedef struct
{
    UINT8 type;
    char *name;
}
t_opc;

#define DIS_DEFAULT_COUNT    16

#define IMM( n, msb, lsb )   (((n) >> (lsb)) & MSK((msb)-(lsb)+1))
#define ZERO2EIGHT( n )	     (((n) == 0) ? 8 : (n))
#define ZERO2_128( n )	     (((n) == 0) ? 128 : (n))
#define SIGN( n, bits )      ( (n) |\
		                   (((n) & (1 << ((bits) - 1))) ?\
				       (MSK(32-(bits)) << (bits)) : 0))

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

/* OPTIONS */
static t_cmd_option options[] =
{ 
#define OPTION_MORE	0
  { "m",  "Prompt user for keypress after each screen of data" },
#define OPTION_MIPS16E  1
  { "16", "Disassemble MIPS16e code" }
};
#define OPTION_COUNT	(sizeof(options)/sizeof(t_cmd_option))

/* Options */
static bool   more;
static UINT32 address, count;
static bool   mips16e;
static UINT32 rc = OK;

/************************************************************************
 *  Static variables - Opcode and register tables
 ************************************************************************/

static t_opc rsvd = {0};

/************************************************************************
 *		    MIPS64/32, MIPS-3D ASE
 ************************************************************************/

/* Encoding of opcode field (31..26) */
static t_opc opcode[64] = {
  {0,         }, {0,          }, {23, "j"    }, {23, "jal"  },
  {7,  "beq"  }, {7,  "bne"   }, {8,  "blez" }, {8,  "bgtz" },
  {3,  "addi" }, {3,  "addiu" }, {3,  "slti" }, {3,  "sltiu"},
  {15, "andi" }, {15, "ori"   }, {15, "xori" }, {28, "lui"  },
  {0,         }, {0,          }, {14, "cop2" }, {0,         },
  {7,  "beql" }, {7,  "bnel"  }, {8,  "blezl"}, {8,  "bgtzl"},
  {3,  "daddi"}, {3,  "daddiu"}, {25, "ldl"  }, {25, "ldr"  },
  {0,         }, {23, "jalx"  }, {18, "mdmx" }, {0,         },
  {25, "lb"   }, {25, "lh"    }, {25, "lwl"  }, {25, "lw"   },
  {25, "lbu"  }, {25, "lhu"   }, {25, "lwr"  }, {25, "lwu"  },
  {25, "sb"   }, {25, "sh"    }, {25, "swl"  }, {25, "sw"   },
  {25, "sdl"  }, {25, "sdr"   }, {25, "swr"  }, {11, "cache"},
  {25, "ll"   }, {26, "lwc1"  }, {29, "lwc2" }, {36, "pref" },
  {25, "lld"  }, {26, "ldc1"  }, {29, "ldc2" }, {25, "ld"   },
  {25, "sc"   }, {26, "swc1"  }, {29, "swc2" }, {0,         },
  {25, "scd"  }, {26, "sdc1"  }, {29, "sdc2" }, {25, "sd"   }
};

/*  SPECIAL opcode (opcode field = 0). 
 *  Encoding of function field (5..0)  
 */
static t_opc special[64] = {
  {0,           }, {0,          }, {0,          }, {21, "sra"   },
  {22, "sllv"   }, {0,          }, {0,          }, {22, "srav"  },
  {0,           }, {0,          }, {1,  "movz"  }, {1,  "movn"  },
  {9,  "syscall"}, {9,  "break" }, {0,          }, {18, "sync"  },
  {32, "mfhi"   }, {24, "mthi"  }, {32, "mflo"  }, {24, "mtlo"  },
  {22, "dsllv"  }, {0,          }, {22, "dsrlv" }, {22, "dsrav" },
  {17, "mult"   }, {17, "multu" }, {17, "div"   }, {17, "divu"  },
  {17, "dmult"  }, {17, "dmultu"}, {17, "ddiv"  }, {17, "ddivu" },
  {1,  "add"    }, {1,  "addu"  }, {1,  "sub"   }, {1,  "subu"  },
  {1,  "and"    }, {1,  "or"    }, {1,  "xor"   }, {1,  "nor"   },
  {0,           }, {0,          }, {1,  "slt"   }, {1,  "sltu"  },
  {1,  "dadd"   }, {1,  "daddu" }, {1,  "dsub"  }, {1,  "dsubu" },
  {17, "tge"    }, {17, "tgeu"  }, {17, "tlt"   }, {17, "tltu"  },
  {17, "teq"    }, {0,          }, {17, "tne"   }, {0,          },
  {21, "dsll"   }, {0,          }, {21, "dsrl"  }, {21, "dsra"  },
  {21, "dsll32" }, {0,          }, {21, "dsrl32"}, {21, "dsra32"}
};

/*  jr,jalr,srl,srlv (SPECIAL opcode, functions = 2,6,8,9)
 */
static t_opc jr[2] = {       /* Encoding of msb of hint field (bit 10) */
  {24, "jr"}, {24, "jr.hb"}
};

static t_opc jalr[2] = {     /* Encoding of msb of hint field (bit 10) */
  {13, "jalr"}, {13, "jalr.hb"}
};

static t_opc srl[2] = {      /* Encoding of bit 21 */
  {21, "srl"}, {21,"rotr"}
};

static t_opc srlv[2] = {     /* Encoding of bit 6 */
  {22, "srlv"}, {22, "rotrv"}
};

/*  REGIMM opcode (opcode field = 1). 
 *  Encoding of rt field (20..16) 
 */
static t_opc regimm[32] = {
  {8, "bltz"     }, {8, "bgez"  }, {8, "bltzl"  }, {8,  "bgezl"  },
  {0,            }, {0,         }, {0,          }, {0,           },
  {4, "tgei"     }, {4, "tgeiu" }, {4, "tlti"   }, {4,  "tltiu"  },
  {4, "teqi"     }, {0,         }, {4, "tnei"   }, {0,           },
  {8, "bltzal"   }, {8, "bgezal"}, {8, "bltzall"}, {8,  "bgezall"},
  {0,            }, {0,         }, {0,          }, {0,           },
  {48, "bposge32"}, {0,         }, {0,          }, {0,           },
  {0,            }, {0,         }, {0,          }, {42, "synci"  }
};

/*  SPECIAL2 opcode (opcode field = 28). 
 *  Encoding of function field (5..0)
 */
static t_opc special2[64] = {
  {17, "madd"}, {17, "maddu"}, {1,  "mul"}, {0,         },
  {17, "msub"}, {17, "msubu"}, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {18, "UDI" }, {18, "UDI"  }, {18, "UDI"}, {18, "UDI"  },
  {18, "UDI" }, {18, "UDI"  }, {18, "UDI"}, {18, "UDI"  },
  {18, "UDI" }, {18, "UDI"  }, {18, "UDI"}, {18, "UDI"  },
  {18, "UDI" }, {18, "UDI"  }, {18, "UDI"}, {18, "UDI"  },
  {13, "clz" }, {13, "clo"  }, {0,       }, {0,         },
  {13, "dclz"}, {13, "dclo" }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {0,         },
  {0,        }, {0,         }, {0,       }, {9,  "sdbbp"}
};

/*  SPECIAL3 opcode (opcode field = 31).
 *  MIPS32/64 Release 2 instructions.
 *  Encoding of function field (5..0)
 */
static t_opc special3[64] = {
  {38, "ext" }, {0,         }, {0,   }, {0,         },
  {39, "ins" }, {0,         }, {0,   }, {0,         },
  {1,  "fork"}, {46, "yield"}, {0,   }, {0,         },
  {53, "insv"}, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {0,         },
  {0,        }, {0,         }, {0,   }, {40, "rdhwr"},
  {0,        }, {0,         }, {0,   }, {0,         }
};

/*  COP1X opcode (opcode field = 19). 
 *  Encoding of function field (5..0)
 */
static t_opc cop1x[64] = {
  {27, "lwxc1"  }, {27, "ldxc1"  }, {0,            }, {0,         },
  {0,           }, {27, "luxc1"  }, {0,            }, {0,         },
  {2,  "swxc1"  }, {2,  "sdxc1"  }, {0,            }, {0,         },
  {0,           }, {2,  "suxc1"  }, {0,            }, {31, "prefx"},
  {0,           }, {0,           }, {0,            }, {0,         },
  {0,           }, {0,           }, {0,            }, {0,         },
  {0,           }, {0,           }, {0,            }, {0,         },
  {0,           }, {0,           }, {5,  "alnv.ps" }, {0,         },
  {30, "madd.s" }, {30, "madd.d" }, {0,            }, {0,         },
  {0,           }, {0,           }, {30, "madd.ps" }, {0,         },
  {30, "msub.s" }, {30, "msub.d" }, {0,            }, {0,         },
  {0,           }, {0,           }, {30, "msub.ps" }, {0,         },
  {30, "nmadd.s"}, {30, "nmadd.d"}, {0,            }, {0,         },
  {0,           }, {0,           }, {30, "nmadd.ps"}, {0,         },
  {30, "nmsub.s"}, {30, "nmsub.d"}, {0,            }, {0,         },
  {0,           }, {0,           }, {30, "nmsub.ps"}, {0,         }
};

/*  MOVCI opcodes (SPECIAL opcode, function = 1)
 *  Encoding of tf field (bit 16)
 */
static t_opc movci[2] = {
  {33, "movf"}, {33, "movt"}
};

/*  COP0 opcode (opcode field = 16)
 *  Encoding of rs field (25..21) for rs < 16 
 */
static t_opc cop0[16] = {
  {20, "mfc0"}, {20, "dmfc0"}, {0,          }, {0,   },
  {20, "mtc0"}, {20, "dmtc0"}, {0,          }, {0,   },
  {61, "mftr"}, {0          }, {41, "rdpgpr"}, {0,   },
  {47, "mttr"}, {0,         }, {41, "wrpgpr"}, {0,   }
};

/* COP0.MFMC0 opcodes (COP0 opcode, rs == 11, rd == 0)
 * Encoding of sc field (bit 5)
 */
static t_opc mfmc0_0[2] = {
  {43, "dvpe"}, {43, "evpe"}
};

/* COP0.MFMC0 opcodes (COP0 opcode, rs == 11, rd == 1)
 * Encoding of sc field (bit 5)
 */
static t_opc mfmc0_1[2] = {
  {43, "dmt"}, {43, "emt"}
};

/* COP0.MFMC0 opcodes (COP0 opcode, rs == 11, rd == 12)
 * Encoding of sc field (bit 5)
 */
static t_opc mfmc0_12[2] = {
  {43, "di"}, {43, "ei"}
};

/*  COP0 opcode (opcode field = 16)
 *  Encoding of function field (5..0) for rs >= 16
 */

static t_opc cop0co[64] = {
  {0,        }, {18, "tlbr"}, {18, "tlbwi"}, {0,         },
  {0,        }, {0,        }, {18, "tlbwr"}, {0,         },
  {18, "tlbp"}, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {18, "eret"}, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {18, "deret"},
  {18, "wait"}, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         },
  {0,        }, {0,        }, {0,         }, {0,         }
};

/*  COP1 opcode (opcode field = 17)
 *  Encoding of rs field (25..21) for rs < 16
 */
static t_opc cop1[16] = {
  {12, "mfc1"}, {12, "dmfc1"}, {37, "cfc1"}, {12, "mfhc1"},
  {12, "mtc1"}, {12, "dmtc1"}, {37, "ctc1"}, {12, "mthc1"},
  {0,        }, {0,         }, {0,        }, {0,    },
  {0,        }, {0,         }, {0,        }, {0,    }
}; 

/*  BC1 opcode (opcode field = 17) for rs = 8
 *  Encoding of rt field (17..16) 
 */
static t_opc bc1[4] = {
  {6, "bc1f" }, {6, "bc1t" },
  {6, "bc1fl"}, {6, "bc1tl"}
};

/*  BC1ANY2 opcodes (opcode field = 17) for rs = 9
 *  Encoding of tf field (bit 16)
 */
static t_opc bc1any2[2] = {
  {6, "bc1any2f"}, {6, "bc1any2t"}
};

/*  BC1ANY4 opcodes (opcode field = 17) for rs = 10
 *  Encoding of tf field (bit 16)
 */
static t_opc bc1any4[2] = {
  {6, "bc1any4f"}, {6, "bc1any4t"}
};

/*  COP1 opcode (opcode field = 17) for rs=16 (S) 
 *  Encoding of function field (5..0)
 */
static t_opc cop1s[64] = {
  {19, "add.s"    }, {19, "sub.s"    }, {19, "mul.s"    }, {19, "div.s"    },
  {16, "sqrt.s"   }, {16, "abs.s"    }, {16, "mov.s"    }, {16, "neg.s"	   },
  {16, "round.l.s"}, {16, "trunc.l.s"}, {16, "ceil.l.s" }, {16, "floor.l.s"},
  {16, "round.w.s"}, {16, "trunc.w.s"}, {16, "ceil.w.s" }, {16, "floor.w.s"},
  {0,             }, {0,             }, {35, "movz.s"   }, {35, "movn.s"   },
  {0,             }, {16, "recip.s"  }, {16, "rsqrt.s"  }, {0,    	   },
  {0,             }, {0,             }, {0,    	        }, {0,    	   },
  {19, "recip2.s" }, {16, "recip1.s" }, {16, "rsqrt1.s" }, {19,  "rsqrt2.s"},
  {0,             }, {16, "cvt.d.s"  }, {0,    	        }, {0,             },
  {16, "cvt.w.s"  }, {16, "cvt.l.s"  }, {19,  "cvt.ps.s"}, {0,             },
  {0,             }, {0,    	     }, {0,    	        }, {0,             },
  {0,             }, {0,    	     }, {0,             }, {0,             },
  {10, "c.f.s"    }, {10, "c.un.s"   }, {10, "c.eq.s"   }, {10, "c.ueq.s"  },
  {10, "c.olt.s"  }, {10, "c.ult.s"  }, {10, "c.ole.s"  }, {10, "c.ule.s"  },
  {10, "c.sf.s"   }, {10, "c.ngle.s" }, {10, "c.seq.s"  }, {10, "c.ngl.s"  },
  {10, "c.lt.s"   }, {10, "c.nge.s"  }, {10, "c.le.s"   }, {10, "c.ngt.s"  }
};

/*  COP1 opcode (opcode field = 17) for rs=17 (D) 
 *  Encoding of function field (5..0)
 */
static t_opc cop1d[64] = {
  {19, "add.d"    }, {19, "sub.d"    }, {19, "mul.d"   }, {19, "div.d"    },
  {16, "sqrt.d"   }, {16, "abs.d"    }, {16, "mov.d"   }, {16, "neg.d"	  },
  {16, "round.l.d"}, {16, "trunc.l.d"}, {16, "ceil.l.d"}, {16, "floor.l.d"},
  {16, "round.w.d"}, {16, "trunc.w.d"}, {16, "ceil.w.d"}, {16, "floor.w.d"},
  {0,             }, {0,             }, {35, "movz.d"  }, {35, "movn.d"	  },
  {0,             }, {16, "recip.d"  }, {16, "rsqrt.d" }, {0,    	  },
  {0,    	  }, {0,    	     }, {0,            }, {0,    	  },
  {19, "recip2.d" }, {16, "recip1.d" }, {16, "rsqrt1.d"}, {19, "rsqrt2.d" },
  {16, "cvt.s.d"  }, {0,    	     }, {0,            }, {0,    	  },
  {16, "cvt.w.d"  }, {16, "cvt.l.d"  }, {0,    	       }, {0,    	  },
  {0,    	  }, {0,    	     }, {0,    	       }, {0,    	  },
  {0,    	  }, {0,    	     }, {0,    	       }, {0,    	  },
  {10, "c.f.d"	  }, {10, "c.un.d"   }, {10, "c.eq.d"  }, {10, "c.ueq.d"  },
  {10, "c.olt.d"  }, {10, "c.ult.d"  }, {10, "c.ole.d" }, {10, "c.ule.d"  },
  {10, "c.sf.d"	  }, {10, "c.ngle.d" }, {10, "c.seq.d" }, {10, "c.ngl.d"  },
  {10, "c.lt.d"	  }, {10, "c.nge.d"  }, {10, "c.le.d"  }, {10, "c.ngt.d"  }
};

/*  COP1 opcode (opcode field = 17) for rs=20 (W)
 *  Encoding of function field (5..0)
 */
static t_opc cop1w[64] = {
  {0,           }, {0,           }, {0,             }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {16, "cvt.s.w"}, {16, "cvt.d.w"}, {0,    	    }, {0,   },
  {0,           }, {0,           }, {16, "cvt.ps.pw"}, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   }
};

/*  COP1 opcode (opcode field = 17) for rs=21 (L)
 *  Encoding of function field (5..0)
 */
static t_opc cop1l[64] = {
  {0,           }, {0,           }, {0,             }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {16, "cvt.s.l"}, {16, "cvt.d.l"}, {0,    	    }, {0,   },
  {0,           }, {0,           }, {16, "cvt.ps.pw"}, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   },
  {0,           }, {0,           }, {0,    	    }, {0,   }
};

/*  COP1 opcode (opcode field = 17) for rs=22 (PS)
 *  Encoding of function field (5..0)
 */
static t_opc cop1ps[64] = {
  {19, "add.ps"   }, {19, "sub.ps"   }, {19, "mul.ps"   }, {0,             },
  {0,    	  }, {16, "abs.ps"   }, {16, "mov.ps"   }, {16, "neg.ps"   },
  {0,    	  }, {0,    	     }, {0,    		}, {0,    	   },
  {0,    	  }, {0,    	     }, {0,    		}, {0,    	   },
  {0,    	  }, {0,    	     }, {35, "movz.ps"  }, {35, "movn.ps"  },
  {0,    	  }, {0,    	     }, {0,    		}, {0,    	   },
  {19, "addr.ps"  }, {0,    	     }, {19, "mulr.ps"  }, {0,    	   },
  {19, "recip2.ps"}, {16, "recip1.ps"}, {16, "rsqrt1.ps"}, {19, "rsqrt2.ps"},
  {16, "cvt.s.pu" }, {0,    	     }, {0,    		}, {0,             },
  {16, "cvt.pw.ps"}, {0,    	     }, {0,    		}, {0,    	   },
  {16, "cvt.s.pl" }, {0,    	     }, {0,    		}, {0,    	   },
  {19, "pll.ps"   }, {19, "plu.ps"   }, {19, "pul.ps"	}, {19, "puu.ps"   },
  {10, "c.f.ps"	  }, {10, "c.un.ps"  }, {10, "c.eq.ps"  }, {10, "c.ueq.ps" },
  {10, "c.olt.ps" }, {10, "c.ult.ps" }, {10, "c.ole.ps" }, {10, "c.ule.ps" },
  {10, "c.sf.ps"  }, {10, "c.ngle.ps"}, {10, "c.seq.ps" }, {10, "c.ngl.ps" },
  {10, "c.lt.ps"  }, {10, "c.nge.ps" }, {10, "c.le.ps"  }, {10, "c.ngt.ps" }
};

/*  MOVCF opcode (opcode field = 17) for rs = 16 (S) and
    function = 17.
 *  Encoding of tf field (bit 16)
 */
static t_opc movcfs[2] = {
  {34, "movf.s"}, {34, "movt.s"}
};

/*  MOVCF opcode (opcode field = 17) for rs = 17 (D) and
    function = 17.
 *  Encoding of tf field (bit 16)
 */
static t_opc movcfd[2] = {
  {34, "movf.d"}, {34, "movt.d"}
};

/*  MOVCF opcode (opcode field = 17) for rs = 22 (PS) and
    function = 17.
 *  Encoding of tf field (bit 16)
 */
static t_opc movcfps[2] = {
  {34, "movf.ps"}, {34, "movt.ps"}
};

/*  COP2 opcode (opcode field = 18)
 *  Encoding of rs field (25..21)
 */
static t_opc cop2[32] = {
  {44, "mfc2"}, {0, }, {45, "cfc2"}, {44, "mfhc2"},
  {44, "mtc2"}, {0, }, {45, "ctc2"}, {44, "mthc2"},
  {0,        }, {0, }, {0,        }, {0,         },
  {0,        }, {0, }, {0,        }, {0,         },
  {0,        }, {0, }, {0,        }, {0,         },
  {0,        }, {0, }, {0,        }, {0,         },
  {0,        }, {0, }, {0,        }, {0,         },
  {0,        }, {0, }, {0,        }, {0,         }
};

/*  BC2 opcode (opcode field = 18) for rs = 8
 *  Encoding of rt field (17..16) 
 */
static t_opc bc2[4] = {
  {6, "bc2f" }, {6, "bc2t" },
  {6, "bc2fl"}, {6, "bc2tl"}
};

/*  SPECIAL3.LX (opcode field = 31, function field=10).
 *  Encoding of op function field (10..6)
 */

static t_opc lx[32] = {
  {52, "lwx"}, {0, }, {0, }, {0, },
  {52, "lhx"}, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
  {0,       }, {0, }, {0, }, {0, },
};

/*  SPECIAL3.ADDUQB (opcode field = 31, function field=16).
 *  Encoding of op function field (10..6)
 */

static t_opc adduqb[32] = {
  {1,  "addu.qb"      }, {1, "subu.qb"      }, {1, "muleu.ph.qbl"  }, {1, "muleu.ph.qbr"  },
  {1,  "addu_s.qb"    }, {1, "subu_s.qb"    }, {1, "muleu_s.ph.qbl"}, {1, "muleu_s.ph.qbr"},
  {0,                 }, {0,                }, {1, "addq.ph"       }, {1, "subq.ph"       },
  {0,                 }, {0,                }, {1, "addq_s.ph"     }, {1, "subq_s.ph"     },
  {1,  "addsc"        }, {1, "addwc"        }, {1, "modsub"        }, {0,                 },
  {13, "raddu.w.qb"   }, {0,                }, {1, "addq_s.w"      }, {1, "subq_s.w"      },
  {1,  "muleq.w.phl"  }, {1, "muleq.w.phr"  }, {0,                 }, {1, "mulq.ph"       },
  {1,  "muleq_s.w.phl"}, {1, "muleq_s.w.phr"}, {0,                 }, {1, "mulq_rs.ph"    }
};

/*  SPECIAL3.ADDUQB (opcode field = 31, function field=17).
 *  Encoding of op function field (10..6)
 */

static t_opc cmpueqqb[32] = {
  {17, "cmpu.eq.qb"  }, {17, "cmpu.lt.qb"   }, {17, "cmpu.le.qb"}, {1, "pick.qb"        },
  {0,                }, {0,                 }, {0,              }, {0,                  },
  {17, "cmp.eq.ph"   }, {17, "cmp.lt.ph"    }, {17, "cmp.le.ph" }, {1, "pick.ph"        },
  {1,  "precrq.qb.ph"}, {0,                 }, {0,              }, {1, "precrqu_s.qb.ph"},
  {0,                }, {0,                 }, {0,              }, {0,                  },
  {1,  "precrq.ph.w" }, {1, "precrq_rs.ph.w"}, {0,              }, {1, "precrqu_s.ph.w" },
  {0,                }, {0,                 }, {0,              }, {0,                  },
  {0,                }, {0,                 }, {0,              }, {0,                  }
};

/*  SPECIAL3.ABSQPH (opcode field = 31, function field=18).
 *  Encoding of op function field (10..6)
 */

static t_opc absqph[32] = {
  {0,                  }, {0,                  }, {49, "repl.qb"        }, {41, "replv.qb"       },
  {41, "precequ.ph.qbl"}, {41, "precequ.ph.qbr"}, {41, "precequ.ph.qbla"}, {41, "precequ.ph.qbra"},
  {41, "absq.ph"       }, {41, "absq_s.ph"     }, {50, "repl.ph"        }, {41, "replv.ph"       },
  {41, "preceq.w.phl"  }, {41, "preceq.w.phr"  }, {0,                   }, {0,                   },
  {0,                  }, {41, "absq_s.w"      }, {0,                   }, {0,                   },
  {0,                  }, {0,                  }, {0,                   }, {0,                   },
  {0,                  }, {0,                  }, {0,                   }, {0,                   },
  {0,                  }, {0,                  }, {0,                   }, {41, "bitrev"         }
};

/*  SPECIAL3.SLLQB (opcode field = 31, function field=19).
 *  Encoding of op function field (10..6)
 */

static t_opc sllqb[32] = {
  /* We cheat a bit here and bits 25..21 for the qb/ph shll/shrl variants */
  {51, "shll.qb"  }, {51, "shrl.qb"  }, {22, "shllv.qb"  }, {22, "shrlv.qb"  },
  {0,             }, {0,             }, {0,              }, {0,              },
  {51, "shll.ph"  }, {51, "shra.ph"  }, {22, "shllv.ph"  }, {22, "shrav.ph"  },
  {51, "shll_s.ph"}, {51, "shra_r.ph"}, {22, "shllv_s.ph"}, {22, "shrav_r.ph"},
  {0,             }, {0,             }, {0,              }, {0,              },
  {51, "shll_s.w" }, {51, "shra_r.w" }, {22, "shllv_s.w" }, {22, "shrav_r.w" },
  {0,             }, {0,             }, {0,              }, {0,              },
  {0,             }, {0,             }, {0,              }, {0,              },
};

/*  SPECIAL3.BSHFL (opcode field = 31, function field=32).
 *  Encoding of bshfl function field (10..6)
 */

static t_opc bshfl[32] = {
  {0,       }, {41, "wsbh"}, {0,   }, {0,   },
  {0,       }, {0,        }, {0,   }, {0,   },
  {0,       }, {0,        }, {0,   }, {0,   },
  {0,       }, {0,        }, {0,   }, {0,   },
  {41, "seb"}, {0,        }, {0,   }, {0,   },
  {0,       }, {0,        }, {0,   }, {0,   },
  {41, "seh"}, {0,        }, {0,   }, {0,   },
  {0,       }, {0,        }, {0,   }, {0,   }
};

/*  SPECIAL3.DPAQWPH (opcode field = 31, function field=48).
 *  Encoding of opb function field (10..6)
 */

static t_opc dpaqwph[32] = {
  {54, "dpaq.w.ph"  }, {54, "dpsq.w.ph"  }, {54, "mulsaq.w.ph"  }, {54,  "dpau.h.qbl" },
  {54, "dpaq_s.w.ph"}, {54, "dpsq_s.w.ph"}, {54, "mulsaq_s.w.ph"}, {54,  "dpau.h.qbr" },
  {54, "dpaq.l.w"   }, {54, "dpsq.l.w"   }, {0,                 }, {54,  "dpsu.h.qbl" },
  {54, "dpaq_sa.l.w"}, {54, "dpsq_sa.l.w"}, {0,                 }, {54,  "dpsu.h.qbr" },
  {54, "maq.w.phl"  }, {0,               }, {54, "maq.w.phr"    }, {0,                },
  {54, "maq_s.w.phl"}, {0,               }, {54, "maq_s.w.phr"  }, {0,                },
  {0,               }, {0,               }, {0,                 }, {0,                },
  {0,               }, {0,               }, {0,                 }, {0,                },
};

/*  SPECIAL3.EXTRW (opcode field = 31, function field=56).
 *  Encoding of opb function field (10..6)
 */

static t_opc extrw[32] = {
  {55, "extr.w"  }, {55, "extrv.w"  }, {55, "extp"     }, {56, "extpv"      },
  {55, "extr_r.w"}, {55, "extrv_r.w"}, {55, "extr_rs.w"}, {55, "extrv_rs.w" },
  {55, "extl.w"  }, {56, "extlv.w"  }, {55, "extpdp"   }, {56, "extpdpv"    },
  {55, "extl_s.w"}, {56, "extlv_s.w"}, {0,             }, {0,               },
  {0,            }, {0,             }, {49, "rddsp"    }, {57, "wrdsp"      },
  {0,            }, {0,             }, {0,             }, {0,               },
  {0,            }, {0,             }, {58, "shilo"    }, {59, "shilov"     },
  {0,            }, {0,             }, {0,             }, {60, "mthlip"     },
};

/* SLL instruction and special cases of this opcode */
static t_opc opc_sll   = {21, "sll"  };
static t_opc opc_nop   = {18, "nop"  };
static t_opc opc_ssnop = {18, "ssnop"};
static t_opc opc_ehb   = {18, "ehb"  };

/* Register names */
static char *regs[32] = {
  "zero","at",  "v0",  "v1",  "a0",  "a1",  "a2",  "a3",
  "t0",  "t1",  "t2",  "t3",  "t4",  "t5",  "t6",  "t7",
  "s0",  "s1",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
  "t8",  "t9",  "k0",  "k1",  "gp",  "sp",  "s8",  "ra"
};

/* CP0 Register names */
static char *cp0regs[8][32] = 
{
  /* SEL = 0 */
  {
    "Index",   "Random", "EntryLo0","EntryLo1","Context", "PageMask","Wired",   "HWREna",
    "BadVAddr","Count",  "EntryHi", "Compare", "Status",  "Cause",   "EPC",     "PRId",
    "Config",  "LLAddr", "WatchLo", "WatchHi", "XContext","$21,0",   "$22,0",   "Debug",
    "DEPC",    "PerfCnt","ErrCtl",  "CacheErr","TagLo",   "TagHi",   "ErrorEPC","DESAVE"
  },
  /* SEL = 1 */
  {
    "MVPControl", "VPEControl", "TCStatus", "$3,1", "ContextConfig","PageGrain","SRSConf0", "$7,1",
    "$8,1",   "$9,1", "$10,1","$11,1","IntCtl",       "$13,1",    "$14,1","EBase",
    "Config1","$17,1","$18,1","$19,1","$20,1",        "$21,1",    "$22,1","TraceControl",
    "$24,1",  "$25,1","$26,1","$27,1","DataLo",       "DataHi",   "$30,1","$31,1"
  },
  /* SEL = 2 */
  {
    "MVPConf0", "VPEConf0", "TCBind", "$3,2", "$4,2",  "$5,2", "SRSConf1", "$7,2",
    "$8,2",   "$9,2", "$10,2","$11,2","SRSCtl","$13,2","$14,2","$15,2",
    "Config2","$17,2","$18,2","$19,2","$20,2", "$21,2","$22,2","TraceControl2",
    "$24,2",  "$25,2","$26,2","$27,2","$28,2", "$29,2","$30,2","$31,2"
  },
  /* SEL = 3 */
  {
    "MVPConf1", "VPEConf1", "TCRestart", "$3,3", "$4,3","$5,3", "SRSConf2", "$7,3",
    "$8,3",   "$9,3", "$10,3","$11,3","SRSMap","$13,3","$14,3","$15,3",
    "Config3","$17,3","$18,3","$19,3","$20,3", "$21,3","$22,3","UserTraceData",
    "$24,3",  "$25,3","$26,3","$27,3","$28,3", "$29,3","$30,3","$31,3"
  },
  /* SEL = 4 */
  {
    "$0,4", "YQMask", "TCHalt", "$3,4", "$4,4", "$5,4", "SRSConf3", "$7,4",
    "$8,4", "$9,4", "$10,4","$11,4","$12,4","$13,4","$14,4","$15,4",
    "$16,4","$17,4","$18,4","$19,4","$20,4","$21,4","$22,4","TraceBPC",
    "$24,4","$25,4","$26,4","$27,4","$28,4","$29,4","$30,4","$31,4"
  },
  /* SEL = 5 */
  {    
    "$0,5", "VPESchedule", "TCContext", "$3,5", "$4,5", "$5,5", "SRSConf4", "$7,5",
    "$8,5", "$9,5", "$10,5","$11,5","$12,5","$13,5","$14,5","$15,5",
    "$16,5","$17,5","$18,5","$19,5","$20,5","$21,5","$22,5","$23,5",
    "$24,5","$25,5","$26,5","$27,5","$28,5","$29,5","$30,5","$31,5"
  },
  /* SEL = 6 */
  {
    "$0,6", "VPEScheFBack", "TCSchedule", "$3,6", "$4,6", "$5,6", "$6,6", "$7,6",
    "$8,6", "$9,6", "$10,6","$11,6","$12,6","$13,6","$14,6","$15,6",
    "$16,6","$17,6","$18,6","$19,6","$20,6","$21,6","$22,6","$23,6",
    "$24,6","$25,6","$26,6","$27,6","$28,6","$29,6","$30,6","$31,6"
  },
  /* SEL = 7 */
  {
    "$0,7", "$1,7", "TCScheFBack", "$3,7", "$4,7", "$5,7", "$6,7", "$7,7",
    "$8,7", "$9,7", "$10,7","$11,7","$12,7","$13,7","$14,7","$15,7",
    "$16,7","$17,7","$18,7","$19,7","$20,7","$21,7","$22,7","$23,7",
    "$24,7","$25,7","$26,7","$27,7","$28,7","$29,7","$30,7","$31,7"
  } 
};

/* FPU Register names */
static char *fpregs[32] = {
  "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
  "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
  "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
  "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
};

/* DSP accumulator register names */
static char *acregs[4] = {
    "ac0", /* aka hi/lo */
    "ac1",
    "ac2",
    "ac3"
};

/************************************************************************
 *			MIPS16e ASE
 ************************************************************************/

static t_opc opc16[32] = {
  { 1,    "addiu"      },
  { 2,	  "addiu"      },
  { 3,	  "b"	       },
  { 0xff, 	       },
  { 4,	  "beqz"       },
  { 4,	  "bnez"       },
  { 0xff, 	       },
  { 5,	  "ld"	       },
  { 0xff, 	       },
  { 6,	  "addiu"      },
  { 7,	  "slti"       },
  { 7,	  "sltiu"      },
  { 0xff, 	       },
  { 8,	  "li"	       },
  { 8,	  "cmpi"       },
  { 5,	  "sd"	       },
  { 9,	  "lb"	       },
  { 10,	  "lh"	       },
  { 11,	  "lw"	       },
  { 12,	  "lw"	       },
  { 9,	  "lbu"	       },
  { 10,	  "lhu"	       },
  { 13,	  "lw"	       },
  { 12,	  "lwu"	       },
  { 9,	  "sb"	       },
  { 10,	  "sh"	       },
  { 11,	  "sw"	       },
  { 12,	  "sw"	       },
  { 0xff, 	       },
  { 0xff, 	       },
  { 0xff, 	       },
  { 0xff, 	       }
};

static t_opc opc16_jalx[2] = {
  { 14,   "jal"        },
  { 14,	  "jalx"       }
};

static t_opc opc16_shift[4] = {
  { 15,   "sll"        },
  { 15,	  "dsll"       },
  { 15,	  "srl"	       },
  { 15,	  "sra"	       }
};

static t_opc opc16_rria[2] = {
  { 16,   "addiu"      },
  { 16,	  "daddiu"     }
};

static t_opc opc16_i8[8] = {
  { 17,   "bteqz"      },
  { 17,	  "btnez"      },
  { 18,	  "sw"	       },
  { 19,	  "addiu"      },
  { 0xff, 	       },
  { 20,	  "move"       },
  { 0xff, 	       },
  { 21,	  "move"       }
};

static t_opc opc16_rrr[4] = {
  { 22,   "daddu"      },
  { 22,	  "addu"       },
  { 22,	  "dsubu"      },
  { 22,	  "subu"       }
};

static t_opc opc16_rr[32] = {
  { 0xff, 	       },
  { 23,   "sdbbp"      },
  { 24,	  "slt"	       },
  { 24,	  "sltu"       },
  { 25,	  "sllv"       },
  { 23,	  "break"      },
  { 25,	  "srlv"       },
  { 25,	  "srav"       },
  { 26,	  "dsrl"       },
  { 0xff, 	       },
  { 24,	  "cmp"	       },
  { 24,	  "neg"	       },
  { 24,	  "and"	       },
  { 24,	  "or"	       },
  { 24,	  "xor"	       },
  { 24,	  "not"	       },
  { 27,	  "mfhi"       },
  { 0xff, 	       },
  { 27,	  "mflo"       },
  { 26,	  "dsra"       },
  { 25,	  "dsllv"      },
  { 0xff, 	       },
  { 25,	  "dsrlv"      },
  { 25,	  "dsrav"      },
  { 24,	  "mult"       },
  { 24,	  "multu"      },
  { 24,	  "div"	       },
  { 24,	  "divu"       },
  { 24,	  "dmult"      },
  { 24,	  "dmultu"     },
  { 24,	  "ddiv"       },
  { 24,	  "ddivu"      }
};

static t_opc opc16_i64[8] = {
  { 28,   "ld"         },
  { 28,	  "sd"	       },
  { 29,	  "sd"	       },
  { 19,	  "daddiu"     },
  { 30,	  "ld"	       },
  { 31,	  "daddiu"     },
  { 32,	  "daddiu"     },
  { 33,	  "daddiu"     }
};

static t_opc opc16_svrs[2] = {
  { 34,   "restore"    },
  { 34,   "save"       }
};

static t_opc opc16_jalrc[8] = {
  { 27,   "jr"         },
  { 35,	  "jr"	       },
  { 36,	  "jalr"       },
  { 0xff, 	       },
  { 27,	  "jrc"	       },
  { 35,	  "jrc"	       },
  { 36,	  "jalrc"      },
  { 0xff, 	       }
};

static t_opc opc16_cnvt[8] = {
  { 27,   "zeb"        },
  { 27,	  "zeh"	       },
  { 27,	  "zew"	       },
  { 0xff, 	       },
  { 27,	  "seb"	       },
  { 27,	  "seh"	       },
  { 27,	  "sew"	       },
  { 0xff, 	       }
};

static t_opc opc16_asmacro =
  { 0, "asmacro"       };

static t_opc opc16_nop = 
  { 0, "nop"           };

/* Register names */
static char *regs16[8] = {
  "s0","s1", "v0", "v1", "a0", "a1", "a2", "a3" 
};

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

static UINT32 
get_options(
    UINT32 argc,
    char   **argv );

static void
do_dis( void );

static bool
disassemble(
    char    *dest,		/* line address		*/
    UINT32  *addr );		/* virtual address	*/

static bool
disassemble16(
    char    *dest,		/* line address		*/
    UINT32  *addr );		/* virtual address	*/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *                          dis
 ************************************************************************/
static MON_FUNC(dis)
{
    UINT32 size;

    if( !shell_dot )
        rc = get_options( argc, argv );

    size = mips16e ? sizeof(UINT16) : sizeof(UINT32);

    if( rc == OK )    
        rc = sys_validate_range( address, count * size, size, FALSE ); 

    if( rc == OK )
        do_dis();

    return rc;
}

/************************************************************************
 *                          get_options
 ************************************************************************/
static UINT32 
get_options(
    UINT32 argc,
    char   **argv )
{
    t_shell_option decode;
    UINT32	   type;
    UINT32	   arg;
    UINT32	   i;
    bool	   address_valid = FALSE;
    bool	   count_valid   = FALSE;
    bool	   ok		 = TRUE;
    UINT32	   error         = SHELL_ERROR_SYNTAX;

    /* Setup defaults */
    more    = FALSE;
    count   = DIS_DEFAULT_COUNT;
    mips16e = FALSE;

    for( arg = 1; 
	          ok && 
	          (arg < argc) && 
                  shell_decode_token( argv[arg], &type, &decode );
         arg++ )
    {
	switch( type )
	{
	  case SHELL_TOKEN_OPTION :
	    /* Find match */
	    for(i=0; 
		(i<OPTION_COUNT) &&
		(strcmp(decode.option, options[i].option) != 0);
		i++) ;

	    switch(i)
	    {
	      case OPTION_MORE :
		more  = TRUE; 
		break;
              case OPTION_MIPS16E :
                mips16e = TRUE;
                break;
	      default :
	        error		 = SHELL_ERROR_OPTION;
		shell_error_data = argv[arg];
		ok		 = FALSE;
		break;		      
	    }
	    break;
	  case SHELL_TOKEN_NUMBER :
	    if( !address_valid )
	    {
		address_valid = TRUE;
		address       = decode.number;
	    }
	    else if( !count_valid )
	    {
		count_valid = TRUE;
		count	    = decode.number;
	    }
	    else
		ok = FALSE;
	    break;
	  default :
	    ok = FALSE;
	    break;
        }
    }

    if(!address_valid)
        ok = FALSE;

    return ok ? OK : error;
}

/************************************************************************
 *                          do_dis
 ************************************************************************/
static void
do_dis( void )
{
    UINT32  i;
    char    line[80];	// must be long enough to hold disassembled inst line
    bool    rc = TRUE;

    if (SHELL_PUTC( '\n' )) return;

    if (!more)
        SHELL_DISABLE_MORE;

    for(i=0; (i<count) && rc; i++)
    {	
        rc = mips16e ?
	         disassemble16( line, &address ) :
                 disassemble(   line, &address );

	if (SHELL_PUTS(line)) break;
    }

    SHELL_PUTC( '\n' );
}

/************************************************************************
 *                          disassemble
 ************************************************************************/
static bool
disassemble(
    char    *dest,                  /* destination line     */
    UINT32  *addr )                 /* virtual address      */
{
    UINT32  inst;
    UINT32  op, funct, tf, rt, rs, sc, funct_bshfl, funct_op,  /* Instruction fields   */
	    ac, wmask,
	    fr, ft, fs, fd,
	    rd, r2, sa, sa5, code, base, 
	    immediate, imm8, imm10, offset, index, 
            target, func,
            hint, hintx, sel, cc1, cc2,
	    lsb, msb,
	    u, h,
            baddr, jaddr, hint10, rs21, bit6;
    t_opc   *opc = &rsvd;

    if( (*addr) & 0x3 )
    {
        sprintf( dest,
                 "You can only disassemble from 32 bit boundaries.\n");
        return FALSE;
    };

    inst = REG32(*addr);
 
    dest    += sprintf(dest, "%08X:  %08X  ", *addr, inst);
    (*addr) += 4;
        
    /* Isolate all instruction fields */
    op          = (inst >> 26) & 0x3f;
    funct       = (inst >>  0) & 0x3f;
    tf          = (inst >> 16) & 0x01;
    rt          = (inst >> 16) & 0x1f;
    rs          = (inst >> 21) & 0x1f;
    rd          = (inst >> 11) & 0x1f;
    ac          = (inst >> 11) & 0x03;
    rs21	= rs           & 0x01;
    funct_bshfl = (inst >>  6) & 0x1f;
    funct_op    = (inst >>  6) & 0x1f;
    sc		= (inst >>  5) & 0x01;
    hint10	= (inst >> 10) & 0x01;
    bit6	= (inst >>  6) & 0x01;

    switch( op )
    {
      case 0 : /* SPECIAL */ 
        switch( funct )
	{
	  case 0  : 
	    switch(inst)
	    {
	      case 0        : opc = &opc_nop;   break;
	      case 0x1 << 6 : opc = &opc_ssnop; break;
	      case 0x3 << 6 : opc = &opc_ehb;   break;
	      default       : opc = &opc_sll;   break;
	    }
	    break;
	  case 1 : 
	    opc = &movci[tf];
	    break;
	  case 2 :
	    opc = &srl[rs21];
	    break;
	  case 6 :
	    opc = &srlv[bit6];
	    break;
	  case 8 :
	    opc = &jr[hint10];
	    break;
	  case 9 :
	    opc = &jalr[hint10];
	    break;
	  default : 
	    opc = &special[funct];
	    break;
	}
        break;
      case 1 : /* REGIMM */
        opc = &regimm[rt];
        break;
      case 16 : /* COP0 */
	if( rs == 11 ) {
	    if (rd == 0)
		opc = &mfmc0_0[sc];
	    else if (rd == 1)
		opc = &mfmc0_1[sc];
	    else if (rd == 12)
		opc = &mfmc0_12[sc];
	}
	else
            opc = ( rs < 16 ) ? &cop0[rs] : &cop0co[funct];
        break;
      case 17 : /* COP1 */
        switch( rs )
        {
          case 8 :
            opc = &bc1[rt & 0x3];
            break;
          case 9 :
            opc = &bc1any2[tf];
            break;
          case 10 :
            opc = &bc1any4[tf];
            break;
          case 16 :
            opc = (funct == 17) ? &movcfs[tf] : &cop1s[funct];
            break;
          case 17 :
            opc = (funct == 17) ? &movcfd[tf] : &cop1d[funct];
            break;
          case 20 :
            opc = &cop1w[funct];
            break;
          case 21 :
            opc = &cop1l[funct];
            break;
          case 22 :
            opc = (funct == 17) ? &movcfps[tf] : &cop1ps[funct];
            break;
          default :
            opc = &cop1[rs];
            break;
        }
        break;
      case 18 : /* COP2 */
        opc = (rs == 8) ? &bc2[rt & 0x3] : &cop2[rs];
	break;
      case 19 : /* COP1X */
        opc = &cop1x[funct];
        break;
      case 28 : /* SPECIAL2 */
        opc = &special2[funct];
        break;
      case 31 : /* SPECIAL3 (release2) */
	switch (funct) {
	case 10:
	    opc = &lx[funct_op];
	    break;
	case 16:
	    opc = &adduqb[funct_op];
	    break;
	case 17:
	    opc = &cmpueqqb[funct_op];
	    break;
	case 18:
	    opc = &absqph[funct_op];
	    break;
	case 19:
	    opc = &sllqb[funct_op];
	    break;
	case 32:
	    opc = &bshfl[funct_bshfl];
	    break;
	case 48:
	    opc = &dpaqwph[funct_op];
	    break;
	case 52:
	    opc = &extrw[funct_op];
	    break;
	default:
	    opc = &special3[funct];
	}
	break;
      default :
        opc = &opcode[op];
        break;
    }

    dest += sprintf(dest, "%-12s", (opc->type == 0) ? "rsvd" : opc->name);

    fr        = (inst >> 21) & 0x1f;
    ft        = (inst >> 16) & 0x1f;
    fs        = (inst >> 11) & 0x1f;
    fd        = (inst >> 6)  & 0x1f;
    rd        = (inst >> 11) & 0x1f;
    r2        = (inst >> 16) & 0x1f;
    sa        = (inst >> 6)  & 0x1f;
    sa5       = (inst >> 21) & 0x1f;
    code      = (inst >> 6)  & 0xfffff;
    base      = (inst >> 21) & 0x1f;
    immediate = (inst >> 0)  & 0xffff;
    imm8      = (inst >> 16) & 0xff;
    imm10     = SIGN( (inst >> 16) & 0x3ff, 10) & 0xffff;
    offset    = SIGN( (inst >> 0)  & 0xffff, 16);
    index     = (inst >> 16) & 0x1f;
    target    = (inst >> 0)  & 0x3ffffff;
    func      = (inst >> 0)  & 0x1ffffff;
    op        = (inst >> 16) & 0x1f;
    hint      = (inst >> 16) & 0x1f;
    hintx     = (inst >> 11) & 0x1f;
    sel       = (inst >> 0)  & 0x7;
    cc1       = (inst >> 18) & 0x7;
    cc2       = (inst >> 8)  & 0x7;
    lsb	      = (inst >> 6)  & 0x1f;
    msb       = (inst >> 11) & 0x1f;
    wmask     = (inst >> 11) & 0x1f;
    u	      = (inst >> 5)  & 0x1;
    h	      = (inst >> 4)  & 0x1;
    baddr     = (INT32)(*addr) + 4 * SIGN(immediate,16);
    jaddr     = (target<<2) | ((*addr) & 0xf0000000);

    switch (opc->type) 
    {
      case 0  : /* rsvd */
        break;
      case 1  : /* rd, rs, rt */
        sprintf(dest, "%s,%s,%s", regs[rd], regs[rs], regs[rt] );
        break;
      case 2  : /* fs, index(base) */
        sprintf(dest, "%s,%s(%s)", fpregs[fs], regs[index], regs[base]);
        break;
      case 3  : /* rt, rs, immediate (signed) */
        sprintf(dest, "%s,%s,%d", regs[rt], regs[rs], (INT16)immediate);
        break;
      case 4  : /* rs, immediate (signed) */
        sprintf(dest, "%s,%d", regs[rs], (INT16)immediate );
        break;
      case 5  : /* fd, fs, ft, rs */
        sprintf(dest, "%s,%s,%s,%s", fpregs[fd], fpregs[fs], fpregs[ft], regs[rs] );
        break;
      case 6  : /* cc1, baddr */
        sprintf(dest, "%d,0x%08x", cc1, baddr);
        break;
      case 7  : /* rs, rt, baddr */
        sprintf(dest, "%s,%s,0x%08x", regs[rs], regs[rt], baddr);
        break;
      case 8  : /* rs, baddr */
        sprintf(dest, "%s,0x%08x", regs[rs], baddr);
        break;
      case 9  : /* code */
        sprintf(dest, "0x%x", code );
        break;
      case 10 : /* cc2, fs, ft */
        sprintf(dest, "%d,%s,%s", cc2, fpregs[fs], fpregs[ft] );
        break;
      case 11 : /* 0xrt, offset(base) */
        sprintf(dest, "0x%x,%d(%s)", rt, offset, regs[base] );
        break;
      case 12 : /* rt, fs */
        sprintf(dest, "%s,%s", regs[rt], fpregs[fs] );
        break;
      case 13 : /* rd, rs */
        sprintf(dest, "%s,%s", regs[rd], regs[rs] );
        break;
      case 14 : /* func */
        sprintf(dest,"0x%x", func );
        break;
      case 15  : /* rt, rs, immediate (unsigned) */
        sprintf(dest, "%s,%s,0x%x", regs[rt], regs[rs], immediate);
        break;
      case 16 : /* fd, fs */
        sprintf(dest, "%s,%s", fpregs[fd], fpregs[fs] );
        break;
      case 17 : /* rs, rt */
        sprintf(dest, "%s,%s", regs[rs], regs[rt] );
        break;
      case 18 : /* Nothing further */
        break;
      case 19 : /* fd, fs, ft */
        sprintf(dest, "%s,%s,%s", fpregs[fd], fpregs[fs], fpregs[ft] );
        break;
      case 20 : /* rt, cp0(rd,sel) */
        sprintf(dest, "%s,%s", regs[rt], cp0regs[sel][rd] );
        break;
      case 21 : /* rd, rt, sa */
        sprintf(dest, "%s,%s,0x%x", regs[rd], regs[rt], sa );
        break;
      case 22 : /* rd, rt, rs */
        sprintf(dest, "%s,%s,%s", regs[rd], regs[rt], regs[rs] );
        break;
      case 23 : /* target */
        sprintf(dest, "0x%08x", jaddr );
        break;
      case 24 : /* rs or rs,ac */
	if (op == 0 /* special */ &&
	    (funct == 17 /* mthi */ ||
	     funct == 19 /* mtlo */) &&
	    ac != 0)
	    sprintf (dest, "%s,%s", regs[rs],acregs[ac]);
	else
        sprintf(dest, "%s", regs[rs] );
        break;
      case 25 : /* rt, offset(base) */
        sprintf(dest, "%s,%d(%s)", regs[rt], offset, regs[base] );
        break;
      case 26 : /* ft, offset(base) */
        sprintf(dest, "%s,%d(%s)", fpregs[ft], offset, regs[base] );
        break;
      case 27 : /* fd, index(base) */
        sprintf(dest, "%s,%s(%s)", fpregs[fd], regs[index], regs[base] );
        break;
      case 28 : /* rt, immediate */
        sprintf(dest, "%s,0x%x", regs[rt], immediate );
        break;
      case 29 : /* cop2reg, offset(base) */
        sprintf(dest, "$%d,%d(%s)", r2, offset, regs[base] );
        break;
      case 30 : /* fd, fr, fs, ft */
        sprintf(dest, "%s,%s,%s,%s", fpregs[fd], fpregs[fr], fpregs[fs], fpregs[ft] );
        break;
      case 31 : /* hintx, index(base) */
        sprintf(dest, "%d,%s(%s)", hintx, regs[index], regs[base] );
        break;
      case 32 : /* rd or rd, ac */
	if (op == 0 /* special */ &&
	    (funct == 16 /* mfhi */ ||
	     funct == 18 /* mflo */) &&
	    ac != 0)
	    sprintf (dest, "%s,%s", regs[rd],acregs[ac]);
	else
	    sprintf(dest, "%s", regs[rd] );
        break;
      case 33 : /* rd, rs, cc1 */
        sprintf(dest, "%s,%s,%d", regs[rd], regs[rs], cc1 );
        break;
      case 34 : /* fd, fs, cc1 */
        sprintf(dest, "%s,%s,%d", fpregs[fd], fpregs[fs], cc1 );
        break;
      case 35 : /* fd, fs, rt */
        sprintf(dest, "%s,%s,%s", fpregs[fd], fpregs[fs], regs[rt] );
        break;
      case 36 : /* hint, offset(base) */
        sprintf(dest, "%d,%d(%s)", hint, offset, regs[base] );
        break;
      case 37 : /* rt, fp_control_reg */
        sprintf(dest, "%s,$%d", regs[rt], fs);
        break;
      case 38 : /* rt, rs, pos, size */ /* pos=lsb, size=msb+1 */
        sprintf(dest, "%s,%s,%d,%d", regs[rt], regs[rs], lsb, msb+1);
	break;
      case 39 : /* rt, rs, pos, size */ /* pos=lsb, size=msb-lsb+1 (may be negative !) */
        sprintf(dest, "%s,%s,%d,%d", regs[rt], regs[rs], lsb, msb-lsb+1);
	break;
      case 40 : /* rt, rd */
        sprintf(dest, "%s,%s", regs[rt], regs[rd]);
        break;
      case 41 : /* rd, rt */
        sprintf(dest, "%s,%s", regs[rd], regs[rt]);
        break;
      case 42 : /* offset(base) */
        sprintf(dest, "%d(%s)", offset, regs[base] );
        break;
      case 43 : /* rt (unless zero) */
        if(rt)
	    sprintf(dest, "%s", regs[rt]);
        break;
      case 44 : /* rt, 0xrd, sel (rd,sel is actually impl. specific) */
        sprintf(dest, "%s,0x%x,%d", regs[rt], rd, sel );
        break;
      case 45 : /* rt, 0xrd (rd,sel is actually impl. specific) */
        sprintf(dest, "%s,0x%x", regs[rt], rd );
	break;
      case 46 : /* rd,0xrs or 0xrs */
	if (rd == 0)
	    sprintf(dest, "0x%x", rs);
	else
	    sprintf(dest, "%s,0x%x", regs[rd], rs);
	break;
      case 47 : /* rt,rd,u,sel,h */
	sprintf(dest, "%s,%s,%d,%d,%d", regs[rt], regs[rd], u, sel, h);
	break;
      case 48 : /* baddr */
	sprintf(dest, "0x%08x", baddr);
	break;
      case 49 : /* rd, imm8 */
        sprintf(dest, "%s,0x%x", regs[rd], imm8 );
        break;
      case 50 : /* rd, imm10 */
        sprintf(dest, "%s,0x%x", regs[rd], imm10 );
        break;
      case 51 : /* rd, rt, sa5 */
        sprintf(dest, "%s,%s,0x%x", regs[rd], regs[rt], sa5 );
        break;
      case 52 : /* rd, index(base) */
        sprintf(dest, "%s,%s(%s)", regs[rd], regs[index], regs[base] );
        break;
      case 53 : /* rt, rs */
	sprintf(dest, "%s,%s", regs[rt], regs[rs] );
        break;
      case 54 : /* ac, rs, rt */
	  sprintf(dest, "%s,%s,%s", acregs[ac], regs[rs], regs[rt] );
        break;
      case 55 : /* rt, ac, sa5 */
	  sprintf(dest, "%s,%s,0x%x", regs[rt], acregs[ac], sa5 );
        break;
      case 56 : /* rt, ac, rs */
	  sprintf(dest, "%s,%s,%s", regs[rt], acregs[ac], regs[rs] );
        break;
      case 57 : /* rs, wmask */
	  sprintf(dest, "%s,0x%x", regs[rs], wmask );
        break;
      case 58 : /* ac, sa5 */
	  sprintf(dest, "%s,0x%x", acregs[ac], sa5 );
        break;
      case 59 : /* ac, rs */
	  sprintf(dest, "%s,%s", acregs[ac], regs[rs] );
        break;
      case 60 : /* rs, ac */
	  sprintf(dest, "%s,%s", regs[rs], acregs[ac] );
        break;
      case 61 : /* rd,rt,u,sel,h */
	sprintf(dest, "%s,%s,%d,%d,%d", regs[rd], regs[rt], u, sel, h);
	break;
      default : /* Never happens */
        break;
    }

    strcat( dest, "\n" );

    return TRUE;
}

/************************************************************************
 *                          disassemble16
 ************************************************************************/
static bool
disassemble16(
    char    *dest,				  /* destination line   */
    UINT32  *addr )				  /* virtual address    */
{
    UINT32  inst;
    UINT32  op;
    bool    extend;
    t_opc   *opc = NULL;
    UINT32  x, f, f4, s, funct8, funct, ry;   /* Instruction fields	*/
    UINT32  count;
    
    if( (*addr) & 0x1 ) 
    {
        sprintf( dest,
                 "You can only disassemble from 16 bit boundaries.\n");
        return FALSE;
    };

    inst   = (UINT32)REG16(*addr);
    op     = (inst >> 11) & 0x1f;
    extend = ( op == 30 );

    if( extend || (op == 3) ) /* extend or jal(x) */
    {
	/* Setup the extended instrution */
	inst = (inst << 16) | (UINT32)REG16(*addr + 2);

        dest += sprintf(dest, "%08X:  %08X  ", *addr, inst);

        (*addr) += 4;

        if( extend )
            op = (inst >> 11) & 0x1f;
    }
    else
    {
        dest    += sprintf(dest, "%08X:  %04X      ", *addr, inst);
        (*addr) += 2;
    }

    /* Isolate all instruction fields */
    x      = (inst >> 26) & 0x01;
    f      = (inst >> 0 ) & 0x03;
    f4     = (inst >> 4)  & 0x01;
    funct8 = (inst >> 8)  & 0x07;
    s      = (inst >> 7)  & 0x01;
    funct  = (inst >> 0)  & 0x1f;
    ry     = (inst >> 5)  & 0x7;

    switch( op )
    {
      case 3 :  /* JALX */ 
        opc = &opc16_jalx[x]; 
        break;
      case 6 :  /* SHIFT */
        opc = &opc16_shift[f];
        break;
      case 8 :  /* RRI-A */
        opc = &opc16_rria[f4];
        break;
      case 12 : /* I8 */
        opc = (funct8 == 4) ? &opc16_svrs[s] : &opc16_i8[funct8];
        break;
      case 28 : /* RRR */
	opc = extend ? &opc16_asmacro : &opc16_rrr[f];
        break;
      case 29 : /* RR */
        switch( funct )
        {
          case 0  : opc = &opc16_jalrc[ry]; break;
          case 17 : opc = &opc16_cnvt[ry];  break;
          default : opc = &opc16_rr[funct]; break;
        }
        break;
      case 31 : /* I64 */
        opc = &opc16_i64[funct8];
        break;
      default :
        opc = &opc16[op];
        break;
    }

    /* Assembler alias */
    if( inst == MIPS16E_OPCODE_NOP ) 
        opc = &opc16_nop;

    dest += sprintf( dest, "%-12s", (opc->type == 0xff) ? "rsvd" : opc->name );

    switch (opc->type) 
    {
      case 0 : /* No parameters */
        break;
      case 1 :
        if( extend )
            sprintf(dest, "%s,sp,%d", 
			  regs16[IMM(inst,10,8)], 
			  SIGN(
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
            sprintf(dest, "%s,sp,%d", 
			  regs16[IMM(inst,10,8)], 
			  IMM(inst,7,0) << 2 );
	break;
      case 2 :
        if( extend )
            sprintf(dest, "%s,pc,%d", 
			  regs16[IMM(inst,10,8)], 
			  SIGN(
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
            sprintf(dest, "%s,pc,%d", 
			  regs16[IMM(inst,10,8)], 
			  IMM(inst,7,0) << 2 );
	break;
      case 3 :
        if( extend )
	    sprintf(dest, "0x%08x",
			  (INT32)(*addr) +
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ) << 1,
				 17 ) );
	else
	    sprintf(dest, "0x%08x",
			  (INT32)(*addr) +
			  SIGN( 
			      IMM(inst,10,0) << 1, 
			      12 ) );
	break;
      case 4 :
        if( extend )
	    sprintf(dest, "%s,0x%08x",
			  regs16[IMM(inst,10,8)], 
			  (INT32)(*addr) +
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ) << 1,
				 17 ) );
	else
	    sprintf(dest, "%s,0x%08x",
			  regs16[IMM(inst,10,8)], 
			  (INT32)(*addr) +
			  SIGN( 
			      IMM(inst,7,0) << 1, 
			      9 ) );
	break;
      case 5 :
        if( extend )
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ),
			  regs16[IMM(inst,10,8)] );
	else
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0) << 3, 
			  regs16[IMM(inst,10,8)] );
	break;
      case 6 :
        if( extend )
	    sprintf(dest, "%s,%d",
			  regs16[IMM(inst,10,8)], 
			  SIGN( 
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
	    sprintf(dest, "%s,%d",
			  regs16[IMM(inst,10,8)], 
			  SIGN( 
			      IMM(inst,7,0),
			      8 ) );
	break;
      case 7 :
        if( extend )
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,10,8)], 
			  SIGN( 
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,10,8)], 
			  IMM(inst,7,0) );

        break;
      case 8 :
        if( extend )
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,10,8)], 
			  (IMM(inst,20,16) << 11) |
			  (IMM(inst,26,21) << 5 ) |
		          (IMM(inst, 4, 0) << 0 ) );
	
	else
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,10,8)], 
			  IMM(inst,7,0) );

        break;
      case 9 :
        if( extend )
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ),
			  regs16[IMM(inst,10,8)] );
	else
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0),
			  regs16[IMM(inst,10,8)] );
	break;
      case 10 :
        if( extend )
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ),
			  regs16[IMM(inst,10,8)] );
	else
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0) << 1, 
			  regs16[IMM(inst,10,8)] );
	break;
      case 11 :
        if( extend )
            sprintf(dest, "%s,%d(sp)", 
			  regs16[IMM(inst,10,8)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "%s,%d(sp)", 
			  regs16[IMM(inst,10,8)],
			  IMM(inst,7,0) << 2 );
	break;
      case 12 :
        if( extend )
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ),
			  regs16[IMM(inst,10,8)] );
	else
            sprintf(dest, "%s,%d(%s)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0) << 2, 
			  regs16[IMM(inst,10,8)] );
	break;
      case 13 :
        if( extend )
            sprintf(dest, "%s,%d(pc)", 
			  regs16[IMM(inst,10,8)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "%s,%d(pc)", 
			  regs16[IMM(inst,10,8)],
			  IMM(inst,7,0) << 2 );
	break;
      case 14 :
	sprintf(dest, "0x%08x",
		      (*addr & 0xf0000000) +
		      (( (IMM(inst,20,16) << 21) |
		         (IMM(inst,25,21) << 16 ) |
		         (IMM(inst,15, 0) << 0 ) ) << 2) );
        break;
      case 15 :
        if( extend )
            sprintf(dest, "%s,%s,%d", 
			  regs16[IMM(inst,10,8)],
			  regs16[IMM(inst,7,5)],
		          IMM(inst,26,22) );
	else
            sprintf(dest, "%s,%s,%d",
			  regs16[IMM(inst,10,8)],
			  regs16[IMM(inst,7,5)],
		          ZERO2EIGHT(IMM(inst,4,2)) );
	break;
      case 16 :
        if( extend )
            sprintf(dest, "%s,%s,%d", 
			  regs16[IMM(inst,7,5)], 
			  regs16[IMM(inst,10,8)], 
			  SIGN(
			       (IMM(inst,19,16) << 11) |
			       (IMM(inst,26,20) << 4 ) |
			       (IMM(inst, 3, 0) << 0 ),
			       15 ) );
	else
            sprintf(dest, "%s,%s,%d", 
			  regs16[IMM(inst,7,5)], 
			  regs16[IMM(inst,10,8)],
			  SIGN(
			      IMM(inst,3,0),
			      4 ) );
	break;
      case 17 :
        if( extend )
	    sprintf(dest, "0x%08x",
			  (INT32)(*addr) +
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ) << 1,
				 17 ) );
	else
	    sprintf(dest, "0x%08x",
			  (INT32)(*addr) +
			  SIGN( 
			      IMM(inst,7,0) << 1, 
			      9 ) );
	break;
      case 18 :
        if( extend )
            sprintf(dest, "ra,%d(sp)", 
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "ra,%d(sp)", 
			  IMM(inst,7,0) << 2 );
	break;
      case 19 :
        if( extend )
	    sprintf(dest, "sp,%d",
			  SIGN( 
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
	    sprintf(dest, "sp,%d",
			  SIGN( 
			      (IMM(inst,7,0) << 3),
			      11 ) );
	break;
      case 20 :
        sprintf(dest, "%s,%s", 
		      regs[ (IMM(inst,4,3) << 3) |
			    (IMM(inst,7,5) << 0 ) ],
		      regs16[IMM(inst,2,0)] );
	break;
      case 21 :
        sprintf(dest, "%s,%s", 
		      regs16[IMM(inst,7,5)],
		      regs[  IMM(inst,4,0)] );
	break;
      case 22 :
        sprintf(dest, "%s,%s,%s", 
		      regs16[IMM(inst,4,2)],
		      regs16[IMM(inst,10,8)],
		      regs16[IMM(inst,7,5)] );
	break;
      case 23 :
        sprintf(dest, "0x%02x",
		      IMM(inst,10,5) );
	break;
      case 24 :
        sprintf(dest, "%s,%s", 
		      regs16[IMM(inst,10,8)],
		      regs16[IMM(inst,7,5)] );
	break;
      case 25 :
        sprintf(dest, "%s,%s", 
		      regs16[IMM(inst,7,5)],
		      regs16[IMM(inst,10,8)] );
	break;
      case 26 :
        if( extend )
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,7,5)],
		          (IMM(inst,21,21) << 5) |
		          (IMM(inst,26,22) << 0) );
	else
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,7,5)],
		          ZERO2EIGHT(IMM(inst,10,8)) );
	break;
      case 27 :
        sprintf(dest, "%s", 
		      regs16[IMM(inst,10,8)] );
	break;
      case 28 :
        if( extend )
            sprintf(dest, "%s,%d(sp)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "%s,%d(sp)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0) << 3 );
	break;
      case 29 :
        if( extend )
            sprintf(dest, "ra,%d(sp)", 
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "ra,%d(sp)", 
			  IMM(inst,7,0) << 3 );
	break;
      case 30 :
        if( extend )
            sprintf(dest, "%s,%d(pc)", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "%s,%d(pc)", 
			  regs16[IMM(inst,7,5)],
			  IMM(inst,4,0) << 3 );
	break;
      case 31 :
        if( extend )
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,7,5)],
			  SIGN( 
			       ( (IMM(inst,20,16) << 11) |
			         (IMM(inst,26,21) << 5 ) |
			         (IMM(inst, 4, 0) << 0 ) ),
				 16 ) );
	else
            sprintf(dest, "%s,%d", 
			  regs16[IMM(inst,7,5)],
			  SIGN(
			      IMM(inst,4,0),
			      5 ) );
	break;
      case 32 :
        if( extend )
            sprintf(dest, "%s,pc,%d", 
			  regs16[IMM(inst,7,5)], 
			  SIGN(
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
            sprintf(dest, "%s,pc,%d", 
			  regs16[IMM(inst,7,5)], 
			  IMM(inst,4,0) << 2 );
	break;
      case 33 :
        if( extend )
            sprintf(dest, "%s,sp,%d", 
			  regs16[IMM(inst,7,5)], 
			  SIGN(
			       (IMM(inst,20,16) << 11) |
			       (IMM(inst,26,21) << 5 ) |
			       (IMM(inst, 4, 0) << 0 ),
			       16 ) );
	else
            sprintf(dest, "%s,sp,%d", 
			  regs16[IMM(inst,7,5)], 
			  IMM(inst,4,0) << 2 );
	break;
      case 34 :
        if( extend )
	{
            if( IMM(inst,6,6) ) dest += sprintf( dest, "ra,");
            if( IMM(inst,5,5) ) dest += sprintf( dest, "s0,");
            if( IMM(inst,4,4) ) dest += sprintf( dest, "s1,");

	    count = IMM(inst,26,24);

	    /* s2..s8 */
	    if( count == 1 )
	        dest += sprintf( dest, "s2," );
	    if( count > 1 )
	        dest += sprintf( dest, "s2..s%d,", count+1 );
	    
	    /* a0..a3 */
	    switch( IMM(inst,19,16) )
	    {
	      case 0  : break;
	      case 1  : dest += sprintf( dest, "a3," ); break;
	      case 2  : dest += sprintf( dest, "a2..a3," ); break;
	      case 3  : dest += sprintf( dest, "a1..a3," ); break;
	      case 11 : dest += sprintf( dest, "a0..a3," ); break;
	      case 4  : break;
	      case 5  : dest += sprintf( dest, "a3," ); break;
	      case 6  : dest += sprintf( dest, "a2..a3," ); break;
	      case 7  : dest += sprintf( dest, "a1..a3," ); break;
	      case 8  : break;
	      case 9  : dest += sprintf( dest, "a3," ); break;
	      case 10 : dest += sprintf( dest, "a2..a3," ); break;
	      case 12 : break;
	      case 13 : dest += sprintf( dest, "a3," ); break;
	      case 14 : break;
	      case 15 : break;
	    }

	    /* Framesize * 8 */
	    sprintf( dest, "0x%02x", 
			   ((IMM(inst,23,20) << 4) |
			   (IMM(inst,3,0)   << 0)) << 3 );
	}
	else
	{
            if( IMM(inst,6,6) ) dest += sprintf( dest, "ra,");
            if( IMM(inst,5,5) ) dest += sprintf( dest, "s0,");
            if( IMM(inst,4,4) ) dest += sprintf( dest, "s1,");

	    sprintf( dest, "0x%02x", ZERO2_128(IMM(inst,3,0) << 3) );
	}
        break;
      case 35 :
        strcat(dest, "ra" );
	break;
      case 36 :
        sprintf(dest, "ra,%s", 
		      regs16[IMM(inst,10,8)] );
	break;
      default : /* rsvd */
        break;
    }

    strcat( dest, "\n" );

    return TRUE;
}


/* Command definition for dis */
static t_cmd cmd_def =
{
    "dis",
    dis,
    "dis [-m] [-16] <address> [<count>]",

    "Disassemble code starting at <address>.\n"
    "\n"
    "Disassembles MIPS64/MIPS32 instructions.\n"
    "Includes MIPS-3D(TM) and MIPS16e(TM) ASEs.\n"
    "Includes MIPS64/MIPS32 Release 2 instructions.\n"
    "\n"
    "<count> (default 16) specifies the number of instructions to disassemble.\n"
    "\n"
    "The continuation command '.' works together with 'dis'.",

    options,
    OPTION_COUNT,
    FALSE
};


/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/

/************************************************************************
 *
 *                          shell_dis_init
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
shell_dis_init( void )
{
    return &cmd_def;
}

