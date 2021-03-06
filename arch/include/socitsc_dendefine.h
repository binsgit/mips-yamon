/******************************************************************
 * COPYRIGHT (c) 2000 Denali Software, Inc.  All rights reserved. *
 * -------------------------------------------------------------- *
 * This code is proprietary and confidential information of       *
 * Denali Software. It may not be reproduced, used or transmitted *
 * in any form whatsoever without the express and written         *
 * permission of Denali Software.                                 *
 ******************************************************************
 * WARNING:  This file was automatically generated.  Manual
 * editing may result in undetermined behavior, and may be in
 * violation of the Denali Support Agreement.
 *****************************************************************/
 /* REL: mips.cambridge-ssc-basic32_021907 */

#define SC_ACTIVE_AGING_ADDR              24                   
#define SC_ACTIVE_AGING_OFFSET            0                    
#define SC_ACTIVE_AGING_WIDTH             1                    
#define SC_ADDR_CMP_EN_ADDR               24                   
#define SC_ADDR_CMP_EN_OFFSET             8                    
#define SC_ADDR_CMP_EN_WIDTH              1                    
#define SC_ADDR_PINS_ADDR                 5                    
#define SC_ADDR_PINS_OFFSET               24                   
#define SC_ADDR_PINS_WIDTH                3                    
#define SC_ADDRESS_WIDTH                  14                   
#define SC_AGE_COUNT_ADDR                 27                   
#define SC_AGE_COUNT_OFFSET               16                   
#define SC_AGE_COUNT_WIDTH                4                    
#define SC_AP_ADDR                        0                    
#define SC_AP_OFFSET                      0                    
#define SC_AP_WIDTH                       1                    
#define SC_APREBIT_ADDR                   8                    
#define SC_APREBIT_CS_WIDTH               4                    
#define SC_APREBIT_OFFSET                 8                    
#define SC_APREBIT_WIDTH                  4                    
#define SC_AREFRESH_ADDR                  0                    
#define SC_AREFRESH_OFFSET                8                    
#define SC_AREFRESH_WIDTH                 1                    
#define SC_AUTO_REFRESH_MODE_ADDR         0                    
#define SC_AUTO_REFRESH_MODE_OFFSET       16                   
#define SC_AUTO_REFRESH_MODE_WIDTH        1                    
#define SC_BANK_SPLIT_EN_ADDR             24                   
#define SC_BANK_SPLIT_EN_OFFSET           16                   
#define SC_BANK_SPLIT_EN_WIDTH            1                    
#define SC_BANK_WIDTH                     2                    
#define SC_BSTLEN_ADDR                    27                   
#define SC_BSTLEN_OFFSET                  0                    
#define SC_BSTLEN_WIDTH                   3                    
#define SC_BYTE_WIDTH                     (#MAX_BYTE_WORD+1)   
#define SC_CASLAT_ADDR                    6                    
#define SC_CASLAT_LIN_ADDR                10                   
#define SC_CASLAT_LIN_GATE_ADDR           10                   
#define SC_CASLAT_LIN_GATE_OFFSET         8                    
#define SC_CASLAT_LIN_GATE_WIDTH          4                    
#define SC_CASLAT_LIN_OFFSET              0                    
#define SC_CASLAT_LIN_WIDTH               4                    
#define SC_CASLAT_OFFSET                  0                    
#define SC_CASLAT_WIDTH                   3                    
#define SC_CKE_DELAY_ADDR                 27                   
#define SC_CKE_DELAY_OFFSET               8                    
#define SC_CKE_DELAY_WIDTH                3                    
#define SC_CKE_SHIFT_WIDTH                8                    
#define SC_COLUMN_SIZE_ADDR               6                    
#define SC_COLUMN_SIZE_OFFSET             8                    
#define SC_COLUMN_SIZE_WIDTH              3                    
#define SC_COMMAND_AGE_COUNT_ADDR         27                   
#define SC_COMMAND_AGE_COUNT_OFFSET       24                   
#define SC_COMMAND_AGE_COUNT_WIDTH        4                    
#define SC_CONCURRENTAP_ADDR              0                    
#define SC_CONCURRENTAP_OFFSET            24                   
#define SC_CONCURRENTAP_WIDTH             1                    
#define SC_CONTROL_WORD_WIDTH             13                   
#define SC_CS_BANK_WIDTH                  3                    
#define SC_CS_DIFF_WIDTH                  1                    
#define SC_CS_MAP_ADDR                    4                    
#define SC_CS_MAP_OFFSET                  24                   
#define SC_CS_MAP_WIDTH                   2                    
#define SC_CS_WIDTH                       1                    
#define SC_DATA_BYTE_WIDTH                3                    
#define SC_DATA_WIDTH                     32                   
#define SC_DLL_BYPASS_MODE_ADDR           1                    
#define SC_DLL_BYPASS_MODE_OFFSET         8                    
#define SC_DLL_BYPASS_MODE_WIDTH          1                    
#define SC_DLL_DQS_DELAY_0_ADDR           12                   
#define SC_DLL_DQS_DELAY_0_OFFSET         16                   
#define SC_DLL_DQS_DELAY_0_WIDTH          7                    
#define SC_DLL_DQS_DELAY_1_ADDR           12                   
#define SC_DLL_DQS_DELAY_1_OFFSET         24                   
#define SC_DLL_DQS_DELAY_1_WIDTH          7                    
#define SC_DLL_DQS_DELAY_2_ADDR           13                   
#define SC_DLL_DQS_DELAY_2_OFFSET         0                    
#define SC_DLL_DQS_DELAY_2_WIDTH          7                    
#define SC_DLL_DQS_DELAY_3_ADDR           13                   
#define SC_DLL_DQS_DELAY_3_OFFSET         8                    
#define SC_DLL_DQS_DELAY_3_WIDTH          7                    
#define SC_DLL_DQS_DELAY_BYPASS_0_ADDR    14                   
#define SC_DLL_DQS_DELAY_BYPASS_0_OFFSET  16                   
#define SC_DLL_DQS_DELAY_BYPASS_0_WIDTH   8                    
#define SC_DLL_DQS_DELAY_BYPASS_1_ADDR    14                   
#define SC_DLL_DQS_DELAY_BYPASS_1_OFFSET  24                   
#define SC_DLL_DQS_DELAY_BYPASS_1_WIDTH   8                    
#define SC_DLL_DQS_DELAY_BYPASS_2_ADDR    15                   
#define SC_DLL_DQS_DELAY_BYPASS_2_OFFSET  0                    
#define SC_DLL_DQS_DELAY_BYPASS_2_WIDTH   8                    
#define SC_DLL_DQS_DELAY_BYPASS_3_ADDR    15                   
#define SC_DLL_DQS_DELAY_BYPASS_3_OFFSET  8                    
#define SC_DLL_DQS_DELAY_BYPASS_3_WIDTH   8                    
#define SC_DLL_DQS_DELAY_BYPASS_WIDTH     8                    
#define SC_DLL_DQS_DELAY_WIDTH            7                    
#define SC_DLL_INCREMENT_ADDR             11                   
#define SC_DLL_INCREMENT_OFFSET           16                   
#define SC_DLL_INCREMENT_WIDTH            8                    
#define SC_DLL_LOCK_ADDR                  11                   
#define SC_DLL_LOCK_OFFSET                24                   
#define SC_DLL_LOCK_WIDTH                 8                    
#define SC_DLL_NUM_DELAYS_WIDTH           8                    
#define SC_DLL_START_POINT_ADDR           12                   
#define SC_DLL_START_POINT_OFFSET         0                    
#define SC_DLL_START_POINT_WIDTH          8                    
#define SC_DLLLOCKREG_ADDR                1                    
#define SC_DLLLOCKREG_OFFSET              0                    
#define SC_DLLLOCKREG_WIDTH               1                    
#define SC_DM_WIDTH                       4                    
#define SC_DQS_OUT_SHIFT_ADDR             16                   
#define SC_DQS_OUT_SHIFT_BYPASS_ADDR      16                   
#define SC_DQS_OUT_SHIFT_BYPASS_OFFSET    24                   
#define SC_DQS_OUT_SHIFT_BYPASS_WIDTH     8                    
#define SC_DQS_OUT_SHIFT_OFFSET           16                   
#define SC_DQS_OUT_SHIFT_WIDTH            7                    
#define SC_DQS_WIDTH                      4                    
#define SC_DRIVE_DQ_DQS_ADDR              24                   
#define SC_DRIVE_DQ_DQS_OFFSET            24                   
#define SC_DRIVE_DQ_DQS_WIDTH             1                    
#define SC_EFFECT_USER_DATA_WIDTH         64                   
#define SC_EMRS1_DATA_ADDR                28                   
#define SC_EMRS1_DATA_OFFSET              16                   
#define SC_EMRS1_DATA_WIDTH               14                   
#define SC_EMRS_DATA_WIDTH                14                   
#define SC_ENABLE_QUICK_SREFRESH_ADDR     25                   
#define SC_ENABLE_QUICK_SREFRESH_OFFSET   0                    
#define SC_ENABLE_QUICK_SREFRESH_WIDTH    1                    
#define SC_FAST_WRITE_ADDR                5                    
#define SC_FAST_WRITE_OFFSET              0                    
#define SC_FAST_WRITE_WIDTH               1                    
#define SC_INITAREF_ADDR                  8                    
#define SC_INITAREF_OFFSET                16                   
#define SC_INITAREF_WIDTH                 4                    
#define SC_INT_ACK_ADDR                   8                    
#define SC_INT_ACK_OFFSET                 24                   
#define SC_INT_ACK_WIDTH                  4                    
#define SC_INT_MASK_ADDR                  10                   
#define SC_INT_MASK_OFFSET                16                   
#define SC_INT_MASK_WIDTH                 5                    
#define SC_INT_STATUS_ADDR                10                   
#define SC_INT_STATUS_OFFSET              24                   
#define SC_INT_STATUS_WIDTH               5                    
#define SC_INTRPTAPBURST_ADDR             1                    
#define SC_INTRPTAPBURST_OFFSET           16                   
#define SC_INTRPTAPBURST_WIDTH            1                    
#define SC_INTRPTREADA_ADDR               1                    
#define SC_INTRPTREADA_OFFSET             24                   
#define SC_INTRPTREADA_WIDTH              1                    
#define SC_INTRPTWRITEA_ADDR              2                    
#define SC_INTRPTWRITEA_OFFSET            0                    
#define SC_INTRPTWRITEA_WIDTH             1                    
#define SC_MAX_ADDR_WIDTH                 32                   
#define SC_MAX_CMD_REG_WIDTH              72                   
#define SC_MAX_CMD_WIDTH                  3                    
#define SC_MAX_COL_REG_ADDR               9                    
#define SC_MAX_COL_REG_OFFSET             0                    
#define SC_MAX_COL_REG_WIDTH              4                    
#define SC_MAX_CS_REG_ADDR                5                    
#define SC_MAX_CS_REG_OFFSET              8                    
#define SC_MAX_CS_REG_WIDTH               2                    
#define SC_MAX_LEN_WIDTH                  7                    
#define SC_MAX_LEN_WORD_WIDTH             5                    
#define SC_MAX_REG_ADDR                   32                   
#define SC_MAX_ROW_REG_ADDR               9                    
#define SC_MAX_ROW_REG_OFFSET             8                    
#define SC_MAX_ROW_REG_WIDTH              4                    
#define SC_MAX_USER_ADDR_WIDTH            32                   
#define SC_NO_CMD_INIT_ADDR               2                    
#define SC_NO_CMD_INIT_OFFSET             8                    
#define SC_NO_CMD_INIT_WIDTH              1                    
#define SC_OUT_OF_RANGE_ADDR_ADDR         23                   
#define SC_OUT_OF_RANGE_ADDR_OFFSET       0                    
#define SC_OUT_OF_RANGE_ADDR_WIDTH        32                   
#define SC_OUT_OF_RANGE_LENGTH_ADDR       12                   
#define SC_OUT_OF_RANGE_LENGTH_OFFSET     8                    
#define SC_OUT_OF_RANGE_LENGTH_WIDTH      7                    
#define SC_OUT_OF_RANGE_SOURCE_ID_ADDR    25                   
#define SC_OUT_OF_RANGE_SOURCE_ID_OFFSET  8                    
#define SC_OUT_OF_RANGE_SOURCE_ID_WIDTH   1                    
#define SC_OUT_OF_RANGE_TYPE_ADDR         6                    
#define SC_OUT_OF_RANGE_TYPE_OFFSET       16                   
#define SC_OUT_OF_RANGE_TYPE_WIDTH        3                    
#define SC_PLACEMENT_EN_ADDR              25                   
#define SC_PLACEMENT_EN_OFFSET            16                   
#define SC_PLACEMENT_EN_WIDTH             1                    
#define SC_POWER_DOWN_ADDR                2                    
#define SC_POWER_DOWN_OFFSET              16                   
#define SC_POWER_DOWN_WIDTH               1                    
#define SC_PRIORITY_EN_ADDR               25                   
#define SC_PRIORITY_EN_OFFSET             24                   
#define SC_PRIORITY_EN_WIDTH              1                    
#define SC_PWRUP_SREFRESH_EXIT_ADDR       26                   
#define SC_PWRUP_SREFRESH_EXIT_OFFSET     0                    
#define SC_PWRUP_SREFRESH_EXIT_WIDTH      1                    
#define SC_Q_FULLNESS_ADDR                6                    
#define SC_Q_FULLNESS_OFFSET              24                   
#define SC_Q_FULLNESS_WIDTH               3                    
#define SC_RDLAT_WIDTH                    3                    
#define SC_READ_FIFO_DEPTH_WIDTH          4                    
#define SC_READFIFO_DATA_WIDTH            64                   
#define SC_REG_ADDR_WIDTH                 5                    
#define SC_REG_DATA_WIDTH                 32                   
#define SC_REG_DIMM_ENABLE_ADDR           2                    
#define SC_REG_DIMM_ENABLE_OFFSET         24                   
#define SC_REG_DIMM_ENABLE_WIDTH          1                    
#define SC_REG_MASK_WIDTH                 4                    
#define SC_RW_SAME_EN_ADDR                26                   
#define SC_RW_SAME_EN_OFFSET              8                    
#define SC_RW_SAME_EN_WIDTH               1                    
#define SC_SDR_MODE_ADDR                  3                    
#define SC_SDR_MODE_OFFSET                0                    
#define SC_SDR_MODE_WIDTH                 1                    
#define SC_SPLITCOL_WIDTH                 14                   
#define SC_SREFRESH_ADDR                  3                    
#define SC_SREFRESH_OFFSET                8                    
#define SC_SREFRESH_WIDTH                 1                    
#define SC_START_ADDR                     3                    
#define SC_START_OFFSET                   16                   
#define SC_START_WIDTH                    1                    
#define SC_STAT_WIDTH                     2                    
#define SC_SWAP_EN_ADDR                   26                   
#define SC_SWAP_EN_OFFSET                 16                   
#define SC_SWAP_EN_WIDTH                  1                    
#define SC_TCKE_ADDR                      7                    
#define SC_TCKE_OFFSET                    0                    
#define SC_TCKE_WIDTH                     3                    
#define SC_TDAL_ADDR                      9                    
#define SC_TDAL_OFFSET                    16                   
#define SC_TDAL_WIDTH                     4                    
#define SC_TDLL_ADDR                      19                   
#define SC_TDLL_OFFSET                    16                   
#define SC_TDLL_WIDTH                     16                   
#define SC_TEMRS_ADDR                     5                    
#define SC_TEMRS_OFFSET                   16                   
#define SC_TEMRS_WIDTH                    2                    
#define SC_TINIT_ADDR                     22                   
#define SC_TINIT_OFFSET                   0                    
#define SC_TINIT_WIDTH                    24                   
#define SC_TMRD_ADDR                      11                   
#define SC_TMRD_OFFSET                    0                    
#define SC_TMRD_WIDTH                     5                    
#define SC_TPDEX_ADDR                     31                   
#define SC_TPDEX_OFFSET                   16                   
#define SC_TPDEX_WIDTH                    16                   
#define SC_TRAS_LOCKOUT_ADDR              3                    
#define SC_TRAS_LOCKOUT_OFFSET            24                   
#define SC_TRAS_LOCKOUT_WIDTH             1                    
#define SC_TRAS_MAX_ADDR                  20                   
#define SC_TRAS_MAX_OFFSET                0                    
#define SC_TRAS_MAX_WIDTH                 16                   
#define SC_TRAS_MIN_ADDR                  17                   
#define SC_TRAS_MIN_OFFSET                16                   
#define SC_TRAS_MIN_WIDTH                 8                    
#define SC_TRC_ADDR                       11                   
#define SC_TRC_OFFSET                     8                    
#define SC_TRC_WIDTH                      5                    
#define SC_TRCD_INT_ADDR                  17                   
#define SC_TRCD_INT_OFFSET                24                   
#define SC_TRCD_INT_WIDTH                 8                    
#define SC_TRCD_WIDTH                     8                    
#define SC_TREF_ADDR                      18                   
#define SC_TREF_ENABLE_ADDR               4                    
#define SC_TREF_ENABLE_OFFSET             0                    
#define SC_TREF_ENABLE_WIDTH              1                    
#define SC_TREF_OFFSET                    8                    
#define SC_TREF_WIDTH                     12                   
#define SC_TRFC_ADDR                      18                   
#define SC_TRFC_OFFSET                    0                    
#define SC_TRFC_WIDTH                     8                    
#define SC_TRP_ADDR                       9                    
#define SC_TRP_OFFSET                     24                   
#define SC_TRP_WIDTH                      4                    
#define SC_TRRD_ADDR                      7                    
#define SC_TRRD_OFFSET                    16                   
#define SC_TRRD_WIDTH                     3                    
#define SC_TWR_INT_ADDR                   7                    
#define SC_TWR_INT_OFFSET                 24                   
#define SC_TWR_INT_WIDTH                  3                    
#define SC_TWR_WIDTH                      3                    
#define SC_TWTR_ADDR                      8                    
#define SC_TWTR_OFFSET                    0                    
#define SC_TWTR_WIDTH                     3                    
#define SC_TXSNR_ADDR                     20                   
#define SC_TXSNR_OFFSET                   16                   
#define SC_TXSNR_WIDTH                    16                   
#define SC_TXSR_ADDR                      21                   
#define SC_TXSR_OFFSET                    0                    
#define SC_TXSR_WIDTH                     16                   
#define SC_TYPE_WIDTH                     3                    
#define SC_USER_DATA_WIDTH                64                   
#define SC_USER_MASK_WIDTH                8                    
#define SC_VERSION_ADDR                   21                   
#define SC_VERSION_OFFSET                 16                   
#define SC_VERSION_WIDTH                  16                   
#define SC_WR_DQS_SHIFT_ADDR              17                   
#define SC_WR_DQS_SHIFT_BYPASS_ADDR       17                   
#define SC_WR_DQS_SHIFT_BYPASS_OFFSET     8                    
#define SC_WR_DQS_SHIFT_BYPASS_WIDTH      8                    
#define SC_WR_DQS_SHIFT_OFFSET            0                    
#define SC_WR_DQS_SHIFT_WIDTH             7                    
#define SC_WRCMD_LAT_WIDTH                2                    
#define SC_WRDATA_LAT_WIDTH               2                    
#define SC_WRITE_CNT_WIDTH                3                    
#define SC_WRITE_FIFO_DEPTH_WIDTH         3                    
#define SC_WRITE_FIFO_SRAM_DEPTH_WIDTH    4                    
#define SC_WRITE_FIFO_STATUS_WIDTH        4                    
#define SC_WRITE_MODEREG_ADDR             4                    
#define SC_WRITE_MODEREG_OFFSET           16                   
#define SC_WRITE_MODEREG_WIDTH            1                    
#define SC_WRITEINTERP_ADDR               4                    
#define SC_WRITEINTERP_OFFSET             8                    
#define SC_WRITEINTERP_WIDTH              1                    
