
/************************************************************************
 *
 *      syscon_api.h
 *
 *      The 'syscon' module implements a generic api, which is available
 *      on all MIPS designed boards and which establishes an adaptation
 *      to the specific system resources per board. The 'syscon' api 
 *      must be used in drivers, protocols, applications etc. to access
 *      any required system data to allow modules being portable across
 *      all MIPS designed boards.
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


#ifndef SYSCON_API_H
#define SYSCON_API_H


/************************************************************************
 *    Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>
#include <syserror.h>

/************************************************************************
 *  Macro Definitions
*************************************************************************/


/************************************************************************
 *   SYSCON, ERROR completion codes
*************************************************************************/

#define ERROR_SYSCON              0x00008000 /* for compile time check       */
#define ERROR_SYSCON_OUT_OF_MEMORY    0x8000 /* No more dynamic memory       */
#define ERROR_SYSCON_UNKNOWN_PARAM    0x8001 /* Parameter is not supported   */
#define ERROR_SYSCON_IP_UNKNOWN       0x8002 /* IP address unknown           */
#define ERROR_SYSCON_SUBNET_UNKNOWN   0x8003 /* Subnetmask unknown           */
#define ERROR_SYSCON_GATEWAY_UNKNOWN  0x8004 /* Default gateway unknown      */
#define ERROR_SYSCON_READ_ONLY_ID     0x8005 /* This parameter is read-only  */
#define ERROR_SYSCON_WRITE_ONLY_ID    0x8006 /* This parameter is write-only */
#define ERROR_SYSCON_ENV_VAR_TOO_BIG  0x8007 /* environment var size too big */
#define ERROR_SYSCON_INVALID_CHAR_POS 0x8008 /* Alphanumeric display does not*/
                                             /* support this position id     */
#define ERROR_SYSCON_SIZE_MISMATCH    0x8009 /* parameter size check failed  */


/************************************************************************
 *  System Definitions
*************************************************************************/


/* Memory allocation */
#define SYS_MALLOC_DYNAMIC_MEMORY_SIZE  (4+90+1)*2048+4096
typedef struct sys_malloc
{
    UINT32 size;        /* size (bytes) of memory block to be allocated */
    UINT32 boundary;    /* address boundary alignment unit              */
    void   **memory;    /* pointer to allocated memory block            */
} t_sys_malloc;


/* String or one-character output for Alphanumeric display. */
typedef struct sys_alphanumeric
{
    UINT32 posid;     /* position to start the string or char display; 
                         position '0' is leftmost                       */
    UINT8  *string;   /* nul-terminated string or pointer ref for char  */
} t_sys_alphanumeric;


/* System error to string conversion. */
typedef struct sys_error_string
{
    UINT32 syserror;   /* system error code to be converted             */
    UINT32 count;      /* number of string-pointers returned            */
    UINT8  **strings;  /* pointer to array of string-pointers           */
} t_sys_error_string;
#define SYSCON_ERRORMSG_IDX 0 /* String index for error message         */
#define SYSCON_DIAGMSG_IDX  1 /* String index for diagnose message      */
#define SYSCON_HINTMSG_IDX  2 /* String index for hint message          */

/* Prototype of system error lookup function: */
typedef INT32 (*t_sys_error_lookup)(
          t_sys_error_string *p_param );  /* INOUT: code to string(s)   */


/* System error lookup registration */
typedef struct sys_error_lookup_registration
{
    UINT32             prefix;  /* syserror prefix, 
                                   allocated in syserror.h              */
    t_sys_error_lookup lookup;  /* lookup function                      */
} t_sys_error_lookup_registration;

/* System batch file descriptor:
     Used to read or write a file via TFTP.
     The file content is conveyed in one
     operation between the 'buffer' and
     the host.                                */
typedef struct sys_batch_file_descriptor
{
    UINT32 mode ;         /* the IP-address of the TFTP server host     */
    UINT32 host_ip ;      /* the IP-address of the TFTP server host     */
    UINT8  *file_name ;   /* nul-terminated ASCII string                */
    UINT8  *buffer ;      /* Pointer for caller allocated buffer        */
    UINT32 buffer_size ;  /* Before the file operation (read or write),
                               this parameter contains the size of
                               the caller supplied buffer,
                             After the file operation has been completed,
                               this parameter contains the actual
                               number of bytes, which have been read or
                               written.                                 */
} t_sys_batch_file_descriptor ;
#define SYSCON_FILE_READ_MODE  0 /* File buffer is to be 'read'         */
#define SYSCON_FILE_WRITE_MODE 1 /* File buffer is to be 'written'      */

/* EEPROM definition */
typedef struct
{
    UINT32 minor;
    char   *name;
    char   *descr;
}
t_syscon_eeprom_def;

/* Register information */
typedef struct
{
    UINT32 id;
    UINT32 reg;
    UINT32 sel;
}
t_syscon_reginfo_def;

/************************************************************************
 *  SYSCON enumeration definitions
 *
 *  Note that in order to maintain backward compatibility with
 *  earlier YAMON revisions, new IDs must be added at the end of this
 *  enumerated type.
 ************************************************************************/
typedef enum syscon_ids
{
    /* REVISION register */
    SYSCON_BOARD_REVISION_ID = 0,                    /* UINT32        */
    SYSCON_BOARD_PRODUCTID_ID = 1,                   /* UINT32        */
    SYSCON_BOARD_PRODUCTNAME_ID = 2,                 /* char *        */
    SYSCON_BOARD_PRODUCTREV_ID = 3,                  /* UINT32        */
    SYSCON_BOARD_CORECARDID_ID = 4,                  /* UINT32        */
    SYSCON_BOARD_CORECARD_NAME_ID = 5,               /* char *        */
    SYSCON_BOARD_CORECARDREV_ID = 6,                 /* UINT32        */
    SYSCON_BOARD_FPGAREV_ID = 7,                     /* UINT32        */
    SYSCON_BOARD_RTL_ID = 8,                         /* UINT32        */
    SYSCON_BOARD_RTLREV_MAJOR_ID = 9,                /* UINT32        */
    SYSCON_BOARD_RTLREV_MINOR_ID = 10,               /* UINT32        */
    SYSCON_BOARD_RTL_NAME_ID = 11,                   /* char *        */

    /* Serial number */
    SYSCON_BOARD_SN_ID = 12,                         /* t_sn_ascii    */

    /* NMI */
    SYSCON_BOARD_NMI_STATUS_ID = 13,                 /* bool          */
    SYSCON_BOARD_NMI_WERR_ID = 14,                   /* UINT32        */

    /* Default configuration switch (if available) */
    SYSCON_BOARD_USE_DEFAULT_ID = 15,                /* UINT32        */
    SYSCON_BOARD_INIT_BASED_ON_ENV_ID = 16,          /* bool          */

    /* Memory */
    SYSCON_BOARD_STACK_SIZE_ID = 17,                     /* UINT32    */
    SYSCON_BOARD_APPL_STACK_SIZE_ID = 18,                /* UINT32    */
    SYSCON_BOARD_FREE_MEM_ID = 19,                       /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_ACTUAL_SIZE_ID = 20,          /* UINT32    */

    SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_SPD_ID = 21,       /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_REFRESH_NS_CFG_ID = 22,       /* UINT32    */
 
    SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_NS_SPD_ID = 23,     /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_SRASPRCHG_CYCLES_CFG_ID = 24, /* UINT32    */
 
    SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_NS_SPD_ID = 25,     /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_SRAS2SCAS_CYCLES_CFG_ID = 26, /* UINT32    */
 
    SYSCON_BOARD_SYSTEMRAM_CASLAT_CYCLES_CFG_ID = 27,    /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_READ_BURSTLEN_CFG_ID = 28,    /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_WRITE_BURSTLEN_CFG_ID = 29,   /* UINT32    */

    /* Malloc */
    SYSCON_BOARD_MALLOC_ID = 30,                     /* t_sys_malloc  */

    /* Reset/power off */
    SYSCON_BOARD_SOFTRES_ID = 31,                    /* void          */
    SYSCON_BOARD_BRKRES_ID = 32,                     /* UINT32        */
    SYSCON_BOARD_PSUSTBY_ID = 33,                    /* UINT32        */

    /* Frequencies */
    SYSCON_BOARD_CPU_CLOCK_FREQ_ID = 34,             /* UINT32        */
    SYSCON_BOARD_BUS_CLOCK_FREQ_ID = 35,             /* UINT32        */
    SYSCON_BOARD_PCI_FREQ_KHZ_ID = 36,               /* UINT32        */

    /* Base addresses and sizes */
    SYSCON_BOARD_SYSTEMFLASH_BASE_ID = 37,           /* void *        */
    SYSCON_BOARD_SYSTEMFLASH_SIZE_ID = 38,           /* UINT32        */
    SYSCON_BOARD_MONITORFLASH_BASE_ID = 39,          /* void *        */
    SYSCON_BOARD_MONITORFLASH_SIZE_ID = 40,          /* UINT32        */
    SYSCON_BOARD_FILEFLASH_BASE_ID = 41,             /* void *        */
    SYSCON_BOARD_FILEFLASH_SIZE_ID = 42,             /* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_BASE_ID = 43,             /* void *        */
    SYSCON_BOARD_SYSTEMRAM_SIZE_ID = 44,             /* UINT32        */
    SYSCON_BOARD_SRAM_BASE_ID = 45,                  /* void *        */
    SYSCON_BOARD_SRAM_SIZE_ID = 46,                  /* UINT32        */
    SYSCON_BOARD_GT64120_BASE_ID = 47,               /* void *        */
    SYSCON_BOARD_SAA9730_BASE_ID = 48,               /* void *        */
    SYSCON_BOARD_AM79C973_BASE_ID = 49,              /* void *        */
    SYSCON_BOARD_SYM53C810A_BASE_ID = 50,            /* void *        */
    SYSCON_BOARD_INTEL21150_BASE_ID = 51,            /* void *        */
    SYSCON_BOARD_TI16C550C_BASE0_ID = 52,            /* void *        */
    SYSCON_BOARD_TI16C550C_BASE1_ID = 53,            /* void *        */
    SYSCON_BOARD_ICTA_BASE_ID = 54,                  /* void *        */
    SYSCON_BOARD_TMRA_BASE_ID = 55,                  /* void *        */
    SYSCON_BOARD_PIIX4_SMB_BASE_ID = 56,             /* void *        */
    SYSCON_BOARD_PIIX4_POWER_BASE_ID = 57,           /* void *        */

    /* LEDs and display */
    SYSCON_BOARD_LEDGREEN_ID = 58,                   /* UINT32        */
    SYSCON_BOARD_LEDBAR_ID = 59,                     /* UINT32        */
    SYSCON_BOARD_ASCIIWORD_ID = 60,                  /* UINT32        */
    SYSCON_BOARD_ASCIICHAR_ID = 61,             /* t_sys_alphanumeric */
    SYSCON_BOARD_ASCIISTRING_ID = 62,           /* t_sys_alphanumeric */

    /* Time count */
    SYSCON_BOARD_GET_MILLISEC_ID = 63,               /* UINT32        */

    /* Flash parameters */
    SYSCON_BOARD_SYSTEMFLASH_SECTORSIZE_ID = 64,       /* UINT32      */
    SYSCON_BOARD_MONITORFLASH_SECTORSIZE_ID = 65,      /* UINT32      */
    SYSCON_BOARD_FILEFLASH_SECTORSIZE_ID = 66,         /* UINT32      */
    SYSCON_BOARD_SYSTEMFLASH_BANKCOUNT_ID = 67,        /* UINT32      */
    SYSCON_BOARD_SYSTEMFLASH_BLOCKCOUNT_ID = 68,       /* UINT32      */
    SYSCON_BOARD_SYSTEMFLASH_WRITE_ENABLE_ID = 69,     /* UINT32      */
    SYSCON_BOARD_SYSTEMFLASH_WRITE_DISABLE_ID = 70,    /* UINT32      */
    SYSCON_BOARD_SYSTEMFLASH_WRITE_PROTECTED_ID = 71,  /* UINT32      */
    SYSCON_BOARD_MONITORFLASH_WRITE_PROTECTED_ID = 72, /* UINT32      */
    SYSCON_BOARD_FILEFLASH_WRITE_PROTECTED_ID = 73,    /* UINT32      */

    /* RTC */
    SYSCON_BOARD_RTC_ADDR_ID = 74,                   /* UINT32        */
    SYSCON_BOARD_RTC_DATA_ID = 75,                   /* UINT32        */
    SYSCON_BOARD_RTC_REG_SIZE_ID = 76,               /* UINT8         */

    /* EEPROM addresses */
    SYSCON_BOARD_EEPROM_ADDR = 77,                   /* UINT8         */
    SYSCON_BOARD_EEPROM_SPD_ADDR = 78,               /* UINT8         */

    /* Cache */
    SYSCON_CPU_ICACHE_SIZE_ID = 79,                  /* UINT32        */
    SYSCON_CPU_ICACHE_LINESIZE_ID = 80,              /* UINT32        */
    SYSCON_CPU_ICACHE_ASSOC_ID = 81,                 /* UINT32        */

    SYSCON_CPU_ICACHE_SIZE_CURRENT_ID = 82,          /* UINT32        */
    SYSCON_CPU_ICACHE_LINESIZE_CURRENT_ID = 83,      /* UINT32        */
    SYSCON_CPU_ICACHE_ASSOC_CURRENT_ID = 84,         /* UINT32        */

    SYSCON_CPU_ICACHE_AVAIL_BPW_ID = 85,             /* t_sys_array   */
    SYSCON_CPU_ICACHE_AVAIL_ASSOC_ID = 86,           /* t_sys_array   */

    SYSCON_CPU_DCACHE_SIZE_ID = 87,                  /* UINT32        */
    SYSCON_CPU_DCACHE_LINESIZE_ID = 88,              /* UINT32        */
    SYSCON_CPU_DCACHE_ASSOC_ID = 89,                 /* UINT32        */

    SYSCON_CPU_DCACHE_SIZE_CURRENT_ID = 90,          /* UINT32        */
    SYSCON_CPU_DCACHE_LINESIZE_CURRENT_ID = 91,      /* UINT32        */
    SYSCON_CPU_DCACHE_ASSOC_CURRENT_ID = 92,         /* UINT32        */

    SYSCON_CPU_DCACHE_AVAIL_BPW_ID = 93,             /* t_sys_array   */
    SYSCON_CPU_DCACHE_AVAIL_ASSOC_ID = 94,           /* t_sys_array   */

    SYSCON_CPU_CACHE_CONFIGURABLE_ID = 95,           /* bool          */

    SYSCON_CPU_TLB_COUNT_ID = 96,                    /* UINT8         */
    SYSCON_CPU_TLB_COUNT_RESET_ID = 97,              /* UINT8         */
    SYSCON_CPU_TLB_AVAIL_ID = 98,                    /* bool          */
    SYSCON_CPU_MMU_CONFIGURABLE_ID = 99,             /* bool          */

    /* Endianness */
    SYSCON_CPU_ENDIAN_BIG_ID = 100,                  /* bool          */

    /* CP0 registers */
    SYSCON_CPU_CP0_INDEX_ID = 101,                   /* UINT32        */
    SYSCON_CPU_CP0_RANDOM_ID = 102,                  /* UINT32        */
    SYSCON_CPU_CP0_ENTRYLO0_ID = 103,                /* UINT32/UINT64 */
    SYSCON_CPU_CP0_ENTRYLO1_ID = 104,                /* UINT32/UINT64 */
    SYSCON_CPU_CP0_CONTEXT_ID = 105,                 /* UINT32/UINT64 */
    SYSCON_CPU_CP0_PAGEMASK_ID = 106,                /* UINT32        */
    SYSCON_CPU_CP0_WIRED_ID = 107,                   /* UINT32        */
    SYSCON_CPU_CP0_BADVADDR_ID = 108,                /* UINT32/UINT64 */
    SYSCON_CPU_CP0_COUNT_ID = 109,                   /* UINT32        */
    SYSCON_CPU_CP0_ENTRYHI_ID = 110,                 /* UINT32/UINT64 */
    SYSCON_CPU_CP0_COMPARE_ID = 111,                 /* UINT32        */
    SYSCON_CPU_CP0_STATUS_ID = 112,                  /* UINT32        */
    SYSCON_CPU_CP0_CAUSE_ID = 113,                   /* UINT32        */
    SYSCON_CPU_CP0_EPC_ID = 114,                     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_PRID_ID = 115,                    /* UINT32        */
    SYSCON_CPU_CP0_CONFIG_ID = 116,                  /* UINT32        */
    SYSCON_CPU_CP0_CONFIG1_ID = 117,                 /* UINT32        */
    SYSCON_CPU_CP0_LLADDR_ID = 118,                  /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHLO_ID = 119,                 /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHHI_ID = 120,                 /* UINT32/UINT64 */
    SYSCON_CPU_CP0_DEBUG_ID = 121,                   /* UINT32        */
    SYSCON_CPU_CP0_DEPC_ID = 122,                    /* UINT32/UINT64 */
    SYSCON_CPU_CP0_PERFCOUNT_ID = 123,               /* UINT32        */
    SYSCON_CPU_CP0_ERRCTL_ID = 124,                  /* UINT32        */
    SYSCON_CPU_CP0_CACHEERR_ID = 125,                /* UINT32        */
    SYSCON_CPU_CP0_TAGLO_ID = 126,                   /* UINT32        */
    SYSCON_CPU_CP0_TAGHI_ID = 127,                   /* UINT32        */
    SYSCON_CPU_CP0_DATALO_ID = 128,                  /* UINT32        */
    SYSCON_CPU_CP0_DATAHI_ID = 129,                  /* UINT32        */
    SYSCON_CPU_CP0_ERROREPC_ID = 130,                /* UINT32/UINT64 */
    SYSCON_CPU_CP0_DESAVE_ID = 131,                  /* UINT32/UINT64 */
    SYSCON_CPU_CYCLE_PER_COUNT_ID = 132,             /* UINT32        */
    SYSCON_CPU_CP0_CONFIG_RESET_ID = 133,            /* UINT32        */
    SYSCON_CPU_CP0_CONFIG1_RESET_ID = 134,           /* UINT32        */

    /* Networking */
    SYSCON_COM_EN0_MAJOR_DEVICE_ID = 135,            /* UINT32        */
    SYSCON_COM_EN0_IC_ID = 136,                      /* bool          */
    SYSCON_COM_EN0_INTLINE_ID = 137,                 /* UINT32        */
    SYSCON_COM_EN0_MAC_ADDR_ID = 138,                /* t_mac_addr    */
    SYSCON_COM_EN0_IP_ADDR_ID = 139,                 /* UINT32        */
    SYSCON_COM_EN0_IP_SUBNETMASK_ID = 140,           /* UINT32        */
    SYSCON_COM_EN0_IP_GATEWAYADDR_ID = 141,          /* UINT32        */
    SYSCON_COM_EN0_ENABLE_ID = 142,                  /* void          */
    SYSCON_COM_EN0_DISABLE_ID = 143,                 /* void          */


    /* TTY0 */
    SYSCON_COM_TTY0_MAJOR = 144,                     /* UINT32        */
    SYSCON_COM_TTY0_MINOR = 145,                     /* UINT32        */

    SYSCON_COM_TTY0_REG_SIZE_ID = 146,               /* UINT8         */
    SYSCON_COM_TTY0_REG_SPACING_ID = 147,            /* UINT8         */
    SYSCON_COM_TTY0_IC_IN_USE_ID = 148,              /* bool          */
    SYSCON_COM_TTY0_ADJUST_BREAK_ID = 149,           /* bool          */
    SYSCON_COM_TTY0_INTLINE_ID = 150,                /* UINT32        */
    SYSCON_COM_TTY0_BIGEND_ID = 151,                 /* bool          */

    SYSCON_COM_TTY0_BAUDRATE_ID = 152,               /* UINT8         */
    SYSCON_COM_TTY0_DATABITS_ID = 153,               /* UINT8         */
    SYSCON_COM_TTY0_PARITY_ID = 154,                 /* UINT8         */
    SYSCON_COM_TTY0_STOPBITS_ID = 155,               /* UINT8         */
    SYSCON_COM_TTY0_FLOWCTRL_ID = 156,               /* UINT8         */
    SYSCON_COM_TTY0_BAUDRATE_DIV_ID = 157, /* t_SERIAL_baudrate_div * */

    SYSCON_COM_TTY0_BAUDRATE_VERIFY_ID = 158,        /* UINT8         */
    SYSCON_COM_TTY1_MAJOR = 159,                     /* UINT32        */

    /* TTY1 */
    SYSCON_COM_TTY1_MINOR = 160,                     /* UINT32        */
    SYSCON_COM_TTY1_REG_SIZE_ID = 161,               /* UINT8         */

    SYSCON_COM_TTY1_REG_SPACING_ID = 162,            /* UINT8         */
    SYSCON_COM_TTY1_IC_IN_USE_ID = 163,              /* bool          */
    SYSCON_COM_TTY1_ADJUST_BREAK_ID = 164,           /* bool          */
    SYSCON_COM_TTY1_INTLINE_ID = 165,                /* UINT32        */
    SYSCON_COM_TTY1_BIGEND_ID = 166,                 /* bool          */

    SYSCON_COM_TTY1_BAUDRATE_ID = 167,               /* UINT8         */
    SYSCON_COM_TTY1_DATABITS_ID = 168,               /* UINT8         */
    SYSCON_COM_TTY1_PARITY_ID = 169,                 /* UINT8         */
    SYSCON_COM_TTY1_STOPBITS_ID = 170,               /* UINT8         */
    SYSCON_COM_TTY1_FLOWCTRL_ID = 171,               /* UINT8         */

    SYSCON_COM_TTY1_BAUDRATE_DIV_ID = 172, /* t_SERIAL_baudrate_div * */
    SYSCON_COM_TTY1_BAUDRATE_VERIFY_ID = 173,        /* UINT8         */

    SYSCON_COM_TTY1_ON_TEXAS = 174,                  /* bool          */

    /* IIC */
    SYSCON_COM_IIC_BAUDRATE_ID = 175,                /* UINT8         */
    SYSCON_COM_PIIX4_IIC_BIGEND_ID = 176,            /* bool          */

    /* Environment variables */
    SYSCON_DISK_ENVIRONMENT_ID = 177,       /* t_user_environment_var */

    /* System error handling services */
    SYSCON_ERROR_REGISTER_LOOKUP_ID = 178,     /* t_sys_error_lookup_registration */
    SYSCON_ERROR_LOOKUP_ID = 179,              /* t_sys_error_lookup  */

    /* File transfer */
    SYSCON_FILE_BATCH_ACCESS_ID = 180, /* t_sys_batch_file_descriptor */
    SYSCON_FILE_BOOTSERVER_ID = 181,                 /* UINT32        */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.01 and later revisions
     **********************************************************************/

    /* Secondary cache */
    SYSCON_CPU_SCACHE_SIZE_CURRENT_ID = 182,         /* UINT32        */
    SYSCON_CPU_SCACHE_LINESIZE_CURRENT_ID = 183,     /* UINT32        */
    SYSCON_CPU_SCACHE_ASSOC_CURRENT_ID = 184,        /* UINT32        */

    /* PCI memory mapping */
    SYSCON_CORE_PCI_MEM_START = 185,                 /* UINT32        */
    SYSCON_CORE_PCI_MEM_SIZE = 186,                  /* UINT32        */
    SYSCON_CORE_PCI_MEM_OFFSET = 187,                /* UINT32        */
    SYSCON_CORE_PCI_IO_START = 188,                  /* UINT32        */
    SYSCON_CORE_PCI_IO_SIZE = 189,                   /* UINT32        */
    SYSCON_CORE_PCI_IO_OFFSET = 190,                 /* UINT32        */

    /* EEPROM definitions */
    SYSCON_BOARD_EEPROM_COUNT_ID = 191,              /* UINT8         */
    SYSCON_BOARD_EEPROM_DEF_ID = 192,         /* t_syscon_eeprom_def* */

    /* Number of UARTs on board */
    SYSCON_BOARD_UART_COUNT_ID = 193,                /* UINT8         */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.03 and later revisions
     **********************************************************************/

    SYSCON_SYSCTRL_NAME_ID = 194,                    /* char *        */
    SYSCON_SYSCTRL_REV_ID = 195,                     /* char *        */
    SYSCON_SYSCTRL_SYSID_ID = 196,                   /* UINT32        */
    SYSCON_SYSCTRL_PBCREV_MAJOR_ID = 197,            /* UINT32        */
    SYSCON_SYSCTRL_PBCREV_MINOR_ID = 198,            /* UINT32        */
    SYSCON_SYSCTRL_PCIREV_MAJOR_ID = 199,            /* UINT32        */
    SYSCON_SYSCTRL_PCIREV_MINOR_ID = 200,            /* UINT32        */
    SYSCON_SYSCTRL_WC_CFG_ID = 201,                  /* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_DDR_CFG_ID = 202,         /* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_CSLAT_CYCLES_CFG_ID = 203,/* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_WRLAT_CYCLES_CFG_ID = 204,/* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_RDDEL_CYCLES_CFG_ID = 205,/* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_FW_CFG_ID = 206,          /* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_CLKRAT_CFG_ID = 207,      /* char *        */
    SYSCON_BOARD_SYSTEMRAM_PARITY_CFG_ID = 208,      /* UINT32        */
    SYSCON_SYSCTRL_REGADDR_BASE_ID = 209,            /* UINT32        */
    SYSCON_BOARD_NMI_ACK_ID = 210,                   /* bool          */
    SYSCON_BOARD_PCI_ALLOC_ERR_ID = 211,             /* bool          */

    /* More CP0 registers */
    SYSCON_CPU_CP0_PAGEGRAIN_ID = 212,		     /* UINT32	      */
    SYSCON_CPU_CP0_HWRENA_ID = 213,		     /* UINT32	      */
    SYSCON_CPU_CP0_EBASE_ID = 214,		     /* UINT32	      */
    SYSCON_CPU_CP0_CONFIG2_ID = 215,		     /* UINT32	      */
    SYSCON_CPU_CP0_CONFIG3_ID = 216,		     /* UINT32	      */
    SYSCON_CPU_CP0_XCONTEXT_ID = 217,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_TRACECONTROL_ID = 218,	     /* UINT32	      */
    SYSCON_CPU_CP0_TRACECONTROL2_ID = 219,	     /* UINT32	      */
    SYSCON_CPU_CP0_USERTRACEDATA_ID = 220,	     /* UINT32	      */
    SYSCON_CPU_CP0_TRACEBPC_ID = 221,		     /* UINT32	      */
    SYSCON_CPU_CP0_PERFCOUNT_COUNT0_ID = 222,	     /* UINT32	      */
    SYSCON_CPU_CP0_PERFCOUNT_CTRL1_ID = 223,	     /* UINT32	      */
    SYSCON_CPU_CP0_PERFCOUNT_COUNT1_ID = 224,	     /* UINT32	      */
    SYSCON_CPU_CP0_PERFCOUNT_CTRL2_ID = 225,	     /* UINT32	      */
    SYSCON_CPU_CP0_PERFCOUNT_COUNT2_ID = 226,	     /* UINT32	      */
    SYSCON_CPU_CP0_DERRCTL_ID = 227,		     /* UINT32	      */
    SYSCON_CPU_CP0_IERRCTL_ID = 228,		     /* UINT32	      */
    SYSCON_CPU_CP0_ITAGLO_ID = 229,		     /* UINT32	      */
    SYSCON_CPU_CP0_IDATALO_ID = 230,		     /* UINT32	      */
    SYSCON_CPU_CP0_DTAGLO_ID = 231,		     /* UINT32	      */
    SYSCON_CPU_CP0_DDATALO_ID = 232,		     /* UINT32	      */
    SYSCON_CPU_CP0_ITAGHI_ID = 233,		     /* UINT32	      */
    SYSCON_CPU_CP0_IDATAHI_ID = 234,		     /* UINT32	      */
    SYSCON_CPU_CP0_DTAGHI_ID = 235,		     /* UINT32	      */
    SYSCON_CPU_CP0_DDATAHI_ID = 236,		     /* UINT32	      */
    SYSCON_CPU_CP0_INTCTL_ID = 237,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCTL_ID = 238,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSMAP_ID = 239,		     /* UINT32	      */
    SYSCON_CPU_CP0_L23TAGLO_ID = 240,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_L23DATALO_ID = 241,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_L23TAGHI_ID = 242,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_L23DATAHI_ID = 243,		     /* UINT32/UINT64 */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.05 and later revisions
     **********************************************************************/
    SYSCON_CPU_CP0_WATCHLO0_ID = 244,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHHI0_ID = 245,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHLO1_ID = 246,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHHI1_ID = 247,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHLO2_ID = 248,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHHI2_ID = 249,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHLO3_ID = 250,               /* UINT32/UINT64 */
    SYSCON_CPU_CP0_WATCHHI3_ID = 251,               /* UINT32/UINT64 */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.07 and later revisions
     **********************************************************************/

    SYSCON_CPU_EICMODE_ID = 252,	             /* bool          */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.08 and later revisions
     **********************************************************************/

    SYSCON_CPU_CP0_MVPCONTROL_ID = 253,		     /* UINT32	      */
    SYSCON_CPU_CP0_MVPCONF0_ID = 254,		     /* UINT32	      */
    SYSCON_CPU_CP0_MVPCONF1_ID = 255,		     /* UINT32	      */
    SYSCON_CPU_CP0_VPECONTROL_ID = 256,		     /* UINT32	      */
    SYSCON_CPU_CP0_VPECONF0_ID = 257,		     /* UINT32	      */
    SYSCON_CPU_CP0_VPECONF1_ID = 258,		     /* UINT32	      */
    SYSCON_CPU_CP0_YQMASK_ID = 259,		     /* UINT32	      */
    SYSCON_CPU_CP0_VPESCHEDULE_ID = 260,	     /* UINT32	      */
    SYSCON_CPU_CP0_VPESCHEFBACK_ID = 261,	     /* UINT32	      */
    SYSCON_CPU_CP0_TCSTATUS_ID = 262,		     /* UINT32	      */
    SYSCON_CPU_CP0_TCBIND_ID = 263,		     /* UINT32	      */
    SYSCON_CPU_CP0_TCRESTART_ID = 264,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_TCHALT_ID = 265,		     /* UINT32	      */
    SYSCON_CPU_CP0_TCCONTEXT_ID = 266,		     /* UINT32/UINT64 */
    SYSCON_CPU_CP0_TCSCHEDULE_ID = 267,		     /* UINT32	      */
    SYSCON_CPU_CP0_TCSCHEFBACK_ID = 268,	     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCONF0_ID = 269,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCONF1_ID = 270,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCONF2_ID = 271,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCONF3_ID = 272,		     /* UINT32	      */
    SYSCON_CPU_CP0_SRSCONF4_ID = 273,		     /* UINT32	      */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.09 and later revisions
     **********************************************************************/

    SYSCON_CPU_REGISTER_SETS_ID = 274,		     /* UINT8	      */
    SYSCON_CPU_REGINFO_ID = 275,		     /* t_syscon_reginfo */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.12 and later revisions
     **********************************************************************/

    SYSCON_CPU_CP0_CONFIG4_ID = 276,		     /* UINT32	      */
    SYSCON_CPU_CP0_CONFIG5_ID = 277,		     /* UINT32	      */
    SYSCON_CPU_CP0_CONFIG6_ID = 278,		     /* UINT32	      */
    SYSCON_CPU_CP0_CONFIG7_ID = 279,		     /* UINT32	      */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.13 and later revisions
     **********************************************************************/

    /* Secondary cache */
    SYSCON_CPU_SCACHE_SIZE_ID = 280,		     /* UINT32        */
    SYSCON_CPU_SCACHE_LINESIZE_ID = 281,	     /* UINT32        */
    SYSCON_CPU_SCACHE_ASSOC_ID = 282,		     /* UINT32        */

    SYSCON_BOARD_SYSTEMRAM_RASMIN_CYCLES_CFG_ID = 283,   /* UINT32    */
    SYSCON_BOARD_SYSTEMRAM_RAS2RAS_CYCLES_CFG_ID = 284,  /* UINT32    */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.14 and later revisions
     **********************************************************************/

    /* Soft Endian support */
    SYSCON_BOARD_SOFTEND_VALID_ID = 285,	     /* UINT32        */
    SYSCON_BOARD_SOFTEND_DONE_ID = 286,		     /* UINT32        */
    SYSCON_BOARD_SOFTEND_RESETSYS_ID = 287,	     /* UINT32        */
    SYSCON_BOARD_SYSTEMRAM_RASMAX_CYCLES_CFG_ID = 288,   /* UINT32    */

    /**********************************************************************
     * Below IDs are available in YAMON revision 02.16 and later revisions
     **********************************************************************/
    SYSCON_CPU_CP0_TRACEIBPC_ID = 289,		     /* UINT32	      */
    SYSCON_CPU_CP0_TRACEDBPC_ID = 290,		     /* UINT32	      */
    SYSCON_CPU_CP0_USERLOCAL_ID = 291,		     /* UINT32	      */
    SYSCON_CPU_CP0_VPEOPT_ID = 292,		     /* UINT32	      */

    /**********************************************************************
     * Insert new IDs below. Remember to update SYSCON_OBJ_COUNT !!!
     **********************************************************************/

    SYSCON_OBJ_COUNT = 293
}
t_syscon_ids;


/************************************************************************
 *    Public variables
 ************************************************************************/

/************************************************************************
 *   Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          SYSCON_init
 *  Description :
 *  -------------
 *
 *  Initializes the 'syscon' module.
 *  
 *  Parameters :
 *  ------------
 *
 *  None
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0)
 *
 ************************************************************************/
INT32 
SYSCON_init( void );


/************************************************************************
 *
 *                          SYSCON_read
 *  Description :
 *  -------------
 *
 *  Read the value of system configuration object given by 'id'.
 *
 *  Parameters :
 *  ------------
 *
 *  'id',           IN,    Object id.
 *  'param',        INOUT, Buffer for object value.
 *  'param_size',   IN,    Size of buffer (must match size of object).
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), returned parameter value and size are valid.
 *  'ERROR_SYSCON_UNKNOWN_PARAM': parameter not available on this board
 *  'ERROR_SYSCON_SIZE_MISMATCH': parameter size check failed
 *
 ************************************************************************/
INT32 
SYSCON_read(
    t_syscon_ids  id,		/* IN: object ID			*/
    void          *param,       /* IN: Buffer for object value		*/
    UINT32	  param_size);  /* IN: Buffer size (bytes)		*/


/************************************************************************
 *
 *                          SYSCON_write
 *  Description :
 *  -------------
 *
 *  Write the system configuration object given by 'id'.
 *
 *  Parameters :
 *  ------------
 *
 *  'id',         IN,    Object id.
 *  'param',      IN,    Buffer containing value to write.
 *  'param_size', IN,    Size of buffer.
 *
 *  Return values :
 *  ---------------
 *
 *  'OK'(=0), parameter value has been set
 *  'ERROR_SYSCON_UNKNOWN_PARAM': parameter not available on this board
 *  'ERROR_SYSCON_SIZE_MISMATCH': parameter size check failed
 *
 ************************************************************************/
INT32 
SYSCON_write(
    t_syscon_ids  id,		/* IN: object ID			*/
    void          *param,       /* IN: Buffer with value		*/
    UINT32	  param_size);  /* IN: Size of buffer (bytes)		*/


#endif /* #ifndef SYSCON_API_H */
