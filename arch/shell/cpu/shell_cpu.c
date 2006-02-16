
/************************************************************************
 *
 *  shell_cpu.c
 *
 *  CPU specific setup of shell
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
#include <sys_api.h>
#include <mips.h>
#include <qed.h>
#include <shell_api.h>
#include <shell.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Static variables
 ************************************************************************/

static t_shell_cache_config_name_val mapping_generic[] =
{
    { "off",   
      "Uncached",
      K_CacheAttrU       
    },

    { "on",
      "Cached",
      K_CacheAttrCN    
    }
};
#define CACHE_COUNT_GENERIC \
    (sizeof(mapping_generic)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_4kc[] =
{
    { "off",   
      "Uncached",
      K_CacheAttrU       
    },

    { "on",
      "Write-through, no write allocate",
      K_CacheAttrCN    
    }
};
#define CACHE_COUNT_4KC \
    (sizeof(mapping_4kc)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_4ke5k[] =
{
    { "off",   
      "Uncached",
      K_CacheAttrU       
    },

    { "on",    
      "Write-back, write allocate",
      K_CacheAttrCN    
    },

    { "wb",
      "Write-back, write allocate",
      K_CacheAttrCN    
    },

    { "wt",
      "Write-through, no write allocate",
      K_CacheAttrCWTnWA  
    },

    { "wtall",
      "Write-through, write allocate",
      K_CacheAttrCWTWA    
    }
};
#define CACHE_COUNT_4KE_5K \
    (sizeof(mapping_4ke5k)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_20kc[] =
{
    { "off",   
      "Uncached",
      K_CacheAttrU
    },

    { "on",    
      "Write-back, noncoherent",
      K_CacheAttrCN    
    },

    { "wb",
      "Write-back, noncoherent",
      K_CacheAttrCN    
    },

    { "wbch",
      "Write-back, coherent",
      K_CacheAttrCCE
    },

    { "wt",
      "Write-through, noncoherent, no write allocate",
      K_CacheAttrCWTnWA
    },
};
#define CACHE_COUNT_20KC \
    (sizeof(mapping_20kc)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_24k[] =
{
    { "off",   
      "Uncached",
      K_CacheAttrU       
    },

    { "on",    
      "Write-back, write allocate",
      K_CacheAttrCN    
    },

    { "wb",
      "Write-back, write allocate",
      K_CacheAttrCN    
    },

    { "wt",
      "Write-through, no write allocate",
      K_CacheAttrCWTnWA  
    }
};
#define CACHE_COUNT_24K \
    (sizeof(mapping_24k)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_rm52xx[] =
{
    { "off",
      "Uncached",
      C0_CONFIG_QED_K0_UNCACHED
    },

    { "on",
      "Write-back",
      C0_CONFIG_QED_K0_NONCOHERENT
    },

    { "wb",
      "Write-back",
      C0_CONFIG_QED_K0_NONCOHERENT
    },

    { "wt",
      "Write-through, no write allocate",
      C0_CONFIG_QED_K0_WTHRU_NOALLOC
    },

    { "wtall",
      "Write-through, write allocate",
      C0_CONFIG_QED_K0_WTHRU_ALLOC
    }
};
#define CACHE_COUNT_RM52XX \
    (sizeof(mapping_rm52xx)/sizeof(t_shell_cache_config_name_val))


static t_shell_cache_config_name_val mapping_rm70xx[] =
{
    { "off",
      "Uncached",
      C0_CONFIG_QED_K0_UNCACHED
    },

    { "on",
      "Write-back",
      C0_CONFIG_QED_K0_NONCOHERENT
    },

    { "wb",
      "Write-back",
      C0_CONFIG_QED_K0_NONCOHERENT
    },

    { "wt",
      "Write-through, no write allocate",
      C0_CONFIG_QED_K0_WTHRU_NOALLOC
    },

    { "wtall",
      "Write-through, write allocate",
      C0_CONFIG_QED_K0_WTHRU_ALLOC
    },

    { "bp",
      "Bypass",
      C0_CONFIG_QED_K0_BYPASS
    },

    { "l2on",    
      "Secondary cache on",
      SHELL_CACHE_CONFIG_L2_ENABLE
    },

    { "l2off",    
      "Secondary cache off",
      SHELL_CACHE_CONFIG_L2_DISABLE
    }
};
#define CACHE_COUNT_RM70XX \
    (sizeof(mapping_rm70xx)/sizeof(t_shell_cache_config_name_val))



/************************************************************************
 *  Static function prototypes
 ************************************************************************/

/************************************************************************
 *  Implementation : Static functions
 ************************************************************************/

/************************************************************************
 *  Implementation : Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          shell_arch_cache_config
 *  Description :
 *  -------------
 *
 *  Get CPU specific cache options (used by shell cache command)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
shell_arch_cache_config(
    t_shell_cache_config_name_val **mapping,
    UINT32			  *config_count )
{

    switch( sys_processor )
    {
      case MIPS_M4K : /* CPU has no cache */
        *mapping      = NULL;
	*config_count = 0;
	break;

      case MIPS_4Kc :
      case MIPS_4Kmp :
        *mapping      = mapping_4kc;
	*config_count = CACHE_COUNT_4KC;
	break;

      case MIPS_4KEc     :
      case MIPS_4KEc_R2  :
      case MIPS_4KEmp    :
      case MIPS_4KEmp_R2 :
      case MIPS_4KSc     :
      case MIPS_4KSd     :
      case MIPS_5K       :
      case MIPS_5KE      :
        *mapping      = mapping_4ke5k;
	*config_count = CACHE_COUNT_4KE_5K;
	break;

      case MIPS_20Kc :
      case MIPS_25Kf :
        *mapping      = mapping_20kc;
	*config_count = CACHE_COUNT_20KC;
        break;

      case MIPS_24K :
      case MIPS_34K :
        *mapping      = mapping_24k;
	*config_count = CACHE_COUNT_24K;
        break;

      case QED_RM52XX :
        *mapping      = mapping_rm52xx;
	*config_count = CACHE_COUNT_RM52XX;
	break;

      case QED_RM70XX :
        *mapping      = mapping_rm70xx;
	*config_count = CACHE_COUNT_RM70XX;
	break;

      default :
        *mapping      = mapping_generic;
	*config_count = CACHE_COUNT_GENERIC;
	break;
    }
}

