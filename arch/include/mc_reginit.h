#define TRAS_MAX_DEFAULT 100000
#define TWR_DEFAULT 15
#include <mc_reginit_ddr.h>
#include <mc_reginit_sdr.h>

#define DENALI_REG_0_RO 0x0
#define DENALI_REG_1_RO 0x1 // DLL lock
#define DENALI_REG_2_RO 0x0
#define DENALI_REG_3_RO 0x0
#define DENALI_REG_4_RO 0x0
#define DENALI_REG_5_RO 0x200 // Max CS - read only
#define DENALI_REG_6_RO 0x0
#define DENALI_REG_7_RO 0x0
#define DENALI_REG_8_RO 0x0
#define DENALI_REG_9_RO 0x0e0c // Max rowaddr & coladdr pins
#define DENALI_REG_10_RO 0x0
#define DENALI_REG_11_RO 0x0
#define DENALI_REG_12_RO 0x0
#define DENALI_REG_13_RO 0x0
#define DENALI_REG_14_RO 0x0
#define DENALI_REG_15_RO 0x0
#define DENALI_REG_16_RO 0x0
#define DENALI_REG_17_RO 0x0
#define DENALI_REG_18_RO 0x0
#define DENALI_REG_19_RO 0x0
#define DENALI_REG_20_RO 0x0
#define DENALI_REG_21_RO 0x20400000 // Version
#define DENALI_REG_22_RO 0x0
#define DENALI_REG_23_RO 0x0
#define DENALI_REG_24_RO 0x0
#define DENALI_REG_25_RO 0x0
#define DENALI_REG_26_RO 0x0
#define DENALI_REG_27_RO 0x0
#define DENALI_REG_28_RO 0x0
#define DENALI_REG_29_RO 0x0
#define DENALI_REG_30_RO 0x0
#define DENALI_REG_31_RO 0x0

#define MC_GET_REG(addr)  (REG32(KSEG1(MSC01_REGADDR_BASE + 4 * addr)))
#define MC_SET_REG(addr,val)  ((REG32(KSEG1(MSC01_REGADDR_BASE + 4 * addr))) = val)

#define MC_SET_REG_FIELD(reg,val,width,offset) (reg & ~(((1 << width) - 1) << offset) | (val & ((1 << width) - 1)) << offset)
#define MC_GET_REG_FIELD(reg,width,offset) ((reg >> offset) & ((1 << width) - 1))

