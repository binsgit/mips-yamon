/*
 * SOCit SC/SCP register definitions
 */

/* High level SOCit SC memory map */
#define SOCITSC_DRAM_MEMLO_BASE		0x00000000	/* 256MB */
#define SOCITSC_OCP0_MEMLO_BASE		0x10000000	/* 128MB */
#define SOCITSC_OCP1_MEMLO_BASE		0x18000000	/* 64MB */
#define SOCITSC_OCP2_MEMLO_BASE		0x1c000000	/* 60MB */
#define SOCITSC_OCP0_BOOT_BASE		0x1fc00000	/* 1MB */ 
#define SOCITSC_OCP1_BOOT_BASE		0x1fd00000	/* 1MB */ 
#define SOCITSC_OCP2_BOOT_BASE		0x1fe00000	/* 1MB */ 
#define SOCITSC_OCP0_DEV_BASE		0x1ff00000	/* 256KB */ 
#define SOCITSC_OCP1_DEV_BASE		0x1ff40000	/* 256KB */ 
#define SOCITSC_OCP2_DEV_BASE		0x1ff80000	/* 256KB */ 
#define SOCITSC_SMX_REG_BASE		0x1ffc0000	/* 128KB */ 
#define SOCITSC_DRAM_REG_BASE		0x1ffe0000	/* 128KB */ 
#define SOCITSC_OCP0_MEMHI_BASE		0x20000000	/* 512MB */ 
#define SOCITSC_OCP1_MEMHI_BASE		0x40000000	/* 512MB */ 
#define SOCITSC_OCP2_MEMHI_BASE		0x60000000	/* 512MB */ 
#define SOCITSC_DRAM_MEMHI_BASE		0x80000000	/* 2GB */ 

/*
 * If an AHB bridge is attached to an OCP port the bridge control registers
 * are overlayed at the top of the associciated device region
 */
#define SOCITSC_AHB0_BC_BASE		(SOCITSC_OCP0_DEV_BASE+0x3f000)
#define SOCITSC_AHB1_BC_BASE		(SOCITSC_OCP1_DEV_BASE+0x3f000)
#define SOCITSC_AHB2_BC_BASE		(SOCITSC_OCP2_DEV_BASE+0x3f000)

/* AHB Bridge Control Registers */
#define AHBM_BASE_OFS			0x00		/* Master Decode Base */
#define AHBM_MASK_OFS			0x08		/* Master Decode Mask */
#define AHBM_TRANS_OFS			0x10		/* Master Decode Translation */
#define AHBM_CTRL_OFS			0x18		/* Master Decode Region Control */
#define  AHBM_CTRL_ENABLE_MSK		0x00000001	/*  Enable */
#define  AHBM_CTRL_ENABLE_SHF		0
#define  AHBM_CTRL_LOCKED_MSK		0x00000002	/*  Locked */
#define  AHBM_CTRL_LOCKED_SHF		1
#define  AHBM_CTRL_PROT_MSK		0x000000f0	/*  Protection */
#define  AHBM_CTRL_PROT_SHF		4
#define AHBS_BASE_OFS			0x80		/* Slave Decode Base */
#define AHBS_MASK_OFS			0x88		/* Slave Decode Mask */
#define AHBS_TRANS_OFS			0x90		/* Slave Decode Translation */
#define AHBS_CTRL_OFS			0x98		/* Slave Decode Region Control */
#define  AHBS_CTRL_ENABLE_MSK		0x00000001	/*  Enable */
#define  AHBS_CTRL_ENABLE_SHF		0
#define  AHBS_CTRL_PROT_MSK		0x000ffff0	/*  Protection */
#define  AHBS_CTRL_PROT_SHF		4
#define AHBM_BOOT_BASE_OFS		0x1000		/* Boot Region Translation */
#define AHBM_BOOT_MASK_OFS		0x1008		/* Boot Region Mask */
#define AHBM_BOOT_TRANS_OFS		0x1010		/* Boot Region Translation */
#define AHBM_BOOT_CTRL_OFS		0x1018		/* Boot Region Control */
#define  AHBM_BOOT_CTRL_ENABLE_MSK	0x00000001	/*  Enable */
#define  AHBS_BOOT_CTRL_ENABLE_SHF	0
#define  AHBM_BOOT_CTRL_LOCKED_MSK	0x00000002	/*  Locked */
#define  AHBM_BOOT_CTRL_LOCKED_SHF	1
#define  AHBM_BOOT_CTRL_PROT_MSK	0x000000f0	/*  Protection */
#define  AHBM_BOOT_CTRL_PROT_SHF	4
#define AHBS_ENABLE_OFS			0x1028
#define AHB_STAT_OFS			0x1030
#define AHBS_COHER_OFS			0x1038

/* Access to AHB Master/Slave Decode Region registers */
#ifdef _ASSEMBLER_
#define AHBMREGOFFS(n,offs)	(((n)   )*0x20+AHBM_##offs##_OFS)
#define AHBSREGOFFS(n,offs)	(((n)+16)*0x20+AHBS_##offs##_OFS)
#else
#define AHBMREG(ahb,n,offs)	REG((ahb)+((n)   )*0x20,AHBM_##offs##_OFS)
#define AHBSREG(ahb,n,offs)	REG((ahb)+((n)+16)*0x20,AHBS_##offs##_OFS)
#endif


/* Configuration specific information */
/* FIXME: move these */

/*
 * MIPS PCI bridge
 * Accessed via AHB bridge on OCP0
 */
#define SOCITSC_MIPS_PCI_BASE		SOCITSC_OCP0_DEV_BASE
#define SOCITSC_MIPS_PCI_SIZE		0x00100000
#define SOCITSC_MIPS_PCI_REG_BASE	(SOCITSC_MIPS_PCI_BASE+0x10000)

/*
 * MIPS IP blocks
 * Accessed via AHB bridge on OCP2
 */
#define SOCITSC_MIPS_IP_BASE		SOCITSC_OCP2_DEV_BASE
#define SOCITSC_MIPS_IP_SIZE		0x00100000
#define SOCITSC_MIPS_PBC_REG_BASE	(SOCITSC_MIPS_IP_BASE+0x10000)
#define SOCITSC_MIPS_ICU_REG_BASE	(SOCITSC_MIPS_IP_BASE+0x20000)
#define SOCITSC_MIPS_TMR_REG_BASE	(SOCITSC_MIPS_IP_BASE+0x30000)

/*
 * MIPS Malta CBUS
 * Accessed via AHB bridge/PBC on OCP2
 */
#define SOCITSC_MIPS_CBUS_BASE	SOCITSC_OCP2_MEMLO_BASE
#define SOCITSC_MIPS_CBUS_SIZE	0x08000000


