/*
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 */

/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED
#define CONFIG_MIPS 1
#undef  CONFIG_SMP

/*
 * Code maturity level options
 */
#define CONFIG_EXPERIMENTAL 1

/*
 * Machine selection
 */
#undef  CONFIG_ACER_PICA_61
#undef  CONFIG_ALGOR_P4032
#undef  CONFIG_BAGET_MIPS
#undef  CONFIG_DECSTATION
#undef  CONFIG_DDB5074
#undef  CONFIG_MIPS_EV96100
#undef  CONFIG_MIPS_EV64120
#undef  CONFIG_MIPS_ATLAS
#define CONFIG_MIPS_MALTA 1
#undef  CONFIG_MIPS_SEAD
#undef  CONFIG_NINO
#undef  CONFIG_MIPS_MAGNUM_4000
#undef  CONFIG_MOMENCO_OCELOT
#undef  CONFIG_DDB5476
#undef  CONFIG_OLIVETTI_M700
#undef  CONFIG_SGI_IP22
#undef  CONFIG_SNI_RM200_PCI
#undef  CONFIG_MIPS_ITE8172
#undef  CONFIG_MIPS_IVR
#undef  CONFIG_MCA
#undef  CONFIG_SBUS
#define CONFIG_I8259 1
#define CONFIG_PCI 1
#define CONFIG_HAVE_STD_PC_SERIAL_PORT 1
#define CONFIG_SWAP_IO_SPACE 1
#define CONFIG_PC_KEYB 1
#undef  CONFIG_ISA
#undef  CONFIG_EISA

/*
 * Loadable module support
 */
#undef  CONFIG_MODULES

/*
 * CPU selection
 */
#undef  CONFIG_CPU_R3000
#undef  CONFIG_CPU_R6000
#undef  CONFIG_CPU_R4300
#undef  CONFIG_CPU_R4X00
#undef  CONFIG_CPU_R5000
#undef  CONFIG_CPU_R5432
#undef  CONFIG_CPU_RM7000
#undef  CONFIG_CPU_NEVADA
#undef  CONFIG_CPU_R10000
#undef  CONFIG_CPU_SB1
#define CONFIG_CPU_MIPS32 1
#undef  CONFIG_CPU_ADVANCED
#define CONFIG_CPU_HAS_LLSC 1
#undef  CONFIG_CPU_HAS_WB

/*
 * General setup
 */
#define CONFIG_CPU_LITTLE_ENDIAN 1
#define CONFIG_MIPS_FPU_EMULATOR 1
#define CONFIG_KCORE_ELF 1
#define CONFIG_ELF_KERNEL 1
#undef  CONFIG_BINFMT_AOUT
#define CONFIG_BINFMT_ELF 1
#undef  CONFIG_BINFMT_MISC
#define CONFIG_NET 1
#undef  CONFIG_PCI_NAMES
#undef  CONFIG_HOTPLUG
#undef  CONFIG_PCMCIA
#define CONFIG_SYSVIPC 1
#undef  CONFIG_BSD_PROCESS_ACCT
#define CONFIG_SYSCTL 1

/*
 * Memory Technology Devices (MTD)
 */
#undef  CONFIG_MTD

/*
 * Parallel port support
 */
#undef  CONFIG_PARPORT

/*
 * Block devices
 */
#undef  CONFIG_BLK_DEV_FD
#undef  CONFIG_BLK_DEV_XD
#undef  CONFIG_PARIDE
#undef  CONFIG_BLK_CPQ_DA
#undef  CONFIG_BLK_CPQ_CISS_DA
#undef  CONFIG_BLK_DEV_DAC960
#define CONFIG_BLK_DEV_LOOP 1
#undef  CONFIG_BLK_DEV_NBD
#define CONFIG_BLK_DEV_RAM 1
#define CONFIG_BLK_DEV_RAM_SIZE (4096)
#undef  CONFIG_BLK_DEV_INITRD

/*
 * Multi-device support (RAID and LVM)
 */
#undef  CONFIG_MD
#undef  CONFIG_BLK_DEV_MD
#undef  CONFIG_MD_LINEAR
#undef  CONFIG_MD_RAID0
#undef  CONFIG_MD_RAID1
#undef  CONFIG_MD_RAID5
#undef  CONFIG_BLK_DEV_LVM

/*
 * Networking options
 */
#undef  CONFIG_PACKET
#undef  CONFIG_NETLINK
#undef  CONFIG_NETFILTER
#undef  CONFIG_FILTER
#define CONFIG_UNIX 1
#define CONFIG_INET 1
#undef  CONFIG_IP_MULTICAST
#undef  CONFIG_IP_ADVANCED_ROUTER
#define CONFIG_IP_PNP 1
#undef  CONFIG_IP_PNP_BOOTP
#undef  CONFIG_IP_PNP_RARP
#undef  CONFIG_NET_IPIP
#undef  CONFIG_NET_IPGRE
#undef  CONFIG_INET_ECN
#undef  CONFIG_SYN_COOKIES
#undef  CONFIG_IPV6
#undef  CONFIG_KHTTPD
#undef  CONFIG_ATM

/*
 *  
 */
#undef  CONFIG_IPX
#undef  CONFIG_ATALK
#undef  CONFIG_DECNET
#undef  CONFIG_BRIDGE
#undef  CONFIG_X25
#undef  CONFIG_LAPB
#undef  CONFIG_LLC
#undef  CONFIG_NET_DIVERT
#undef  CONFIG_ECONET
#undef  CONFIG_WAN_ROUTER
#undef  CONFIG_NET_FASTROUTE
#undef  CONFIG_NET_HW_FLOWCONTROL

/*
 * QoS and/or fair queueing
 */
#undef  CONFIG_NET_SCHED

/*
 * Telephony Support
 */
#undef  CONFIG_PHONE
#undef  CONFIG_PHONE_IXJ

/*
 * ATA/IDE/MFM/RLL support
 */
#define CONFIG_IDE 1

/*
 * IDE, ATA and ATAPI Block devices
 */
#define CONFIG_BLK_DEV_IDE 1

/*
 * Please see Documentation/ide.txt for help/info on IDE drives
 */
#undef  CONFIG_BLK_DEV_HD_IDE
#undef  CONFIG_BLK_DEV_HD
#define CONFIG_BLK_DEV_IDEDISK 1
#undef  CONFIG_IDEDISK_MULTI_MODE
#undef  CONFIG_BLK_DEV_IDEDISK_VENDOR
#undef  CONFIG_BLK_DEV_IDEDISK_FUJITSU
#undef  CONFIG_BLK_DEV_IDEDISK_IBM
#undef  CONFIG_BLK_DEV_IDEDISK_MAXTOR
#undef  CONFIG_BLK_DEV_IDEDISK_QUANTUM
#undef  CONFIG_BLK_DEV_IDEDISK_SEAGATE
#undef  CONFIG_BLK_DEV_IDEDISK_WD
#undef  CONFIG_BLK_DEV_COMMERIAL
#undef  CONFIG_BLK_DEV_TIVO
#undef  CONFIG_BLK_DEV_IDECS
#define CONFIG_BLK_DEV_IDECD 1
#undef  CONFIG_BLK_DEV_IDETAPE
#undef  CONFIG_BLK_DEV_IDEFLOPPY
#undef  CONFIG_BLK_DEV_IDESCSI

/*
 * IDE chipset support/bugfixes
 */
#undef  CONFIG_BLK_DEV_CMD640
#undef  CONFIG_BLK_DEV_CMD640_ENHANCED
#undef  CONFIG_BLK_DEV_ISAPNP
#undef  CONFIG_BLK_DEV_RZ1000
#define CONFIG_BLK_DEV_IDEPCI 1
#undef  CONFIG_IDEPCI_SHARE_IRQ
#define CONFIG_BLK_DEV_IDEDMA_PCI 1
#undef  CONFIG_BLK_DEV_OFFBOARD
#define CONFIG_IDEDMA_PCI_AUTO 1
#define CONFIG_BLK_DEV_IDEDMA 1
#undef  CONFIG_IDEDMA_PCI_WIP
#undef  CONFIG_IDEDMA_NEW_DRIVE_LISTINGS
#undef  CONFIG_BLK_DEV_AEC62XX
#undef  CONFIG_AEC62XX_TUNING
#undef  CONFIG_BLK_DEV_ALI15X3
#undef  CONFIG_WDC_ALI15X3
#undef  CONFIG_BLK_DEV_AMD7409
#undef  CONFIG_AMD7409_OVERRIDE
#undef  CONFIG_BLK_DEV_CMD64X
#undef  CONFIG_BLK_DEV_CY82C693
#undef  CONFIG_BLK_DEV_CS5530
#undef  CONFIG_BLK_DEV_HPT34X
#undef  CONFIG_HPT34X_AUTODMA
#undef  CONFIG_BLK_DEV_HPT366
#undef  CONFIG_BLK_DEV_NS87415
#undef  CONFIG_BLK_DEV_OPTI621
#undef  CONFIG_BLK_DEV_PDC202XX
#undef  CONFIG_PDC202XX_BURST
#undef  CONFIG_BLK_DEV_OSB4
#undef  CONFIG_BLK_DEV_SIS5513
#undef  CONFIG_BLK_DEV_SLC90E66
#undef  CONFIG_BLK_DEV_TRM290
#undef  CONFIG_BLK_DEV_VIA82CXXX
#undef  CONFIG_IDE_CHIPSETS
#define CONFIG_IDEDMA_AUTO 1
#undef  CONFIG_IDEDMA_IVB
#undef  CONFIG_DMA_NONPCI
#undef  CONFIG_BLK_DEV_IDE_MODES

/*
 * SCSI support
 */
#undef  CONFIG_SCSI

/*
 * I2O device support
 */
#undef  CONFIG_I2O
#undef  CONFIG_I2O_PCI
#undef  CONFIG_I2O_BLOCK
#undef  CONFIG_I2O_LAN
#undef  CONFIG_I2O_SCSI
#undef  CONFIG_I2O_PROC

/*
 * Network device support
 */
#define CONFIG_NETDEVICES 1

/*
 * ARCnet devices
 */
#undef  CONFIG_ARCNET
#undef  CONFIG_DUMMY
#undef  CONFIG_BONDING
#undef  CONFIG_EQUALIZER
#undef  CONFIG_TUN
#undef  CONFIG_NET_SB1000

/*
 * Ethernet (10 or 100Mbit)
 */
#define CONFIG_NET_ETHERNET 1
#undef  CONFIG_NET_VENDOR_3COM
#undef  CONFIG_LANCE
#undef  CONFIG_NET_VENDOR_SMC
#undef  CONFIG_NET_VENDOR_RACAL
#undef  CONFIG_AT1700
#undef  CONFIG_DEPCA
#undef  CONFIG_HP100
#undef  CONFIG_NET_ISA
#define CONFIG_NET_PCI 1
#define CONFIG_PCNET32 1
#undef  CONFIG_ADAPTEC_STARFIRE
#undef  CONFIG_APRICOT
#undef  CONFIG_CS89x0
#undef  CONFIG_TULIP
#undef  CONFIG_DE4X5
#undef  CONFIG_DGRS
#undef  CONFIG_DM9102
#undef  CONFIG_EEPRO100
#undef  CONFIG_EEPRO100_PM
#undef  CONFIG_LNE390
#undef  CONFIG_NATSEMI
#undef  CONFIG_NE2K_PCI
#undef  CONFIG_NE3210
#undef  CONFIG_ES3210
#undef  CONFIG_8139TOO
#undef  CONFIG_8139TOO_PIO
#undef  CONFIG_8139TOO_TUNE_TWISTER
#undef  CONFIG_8139TOO_8129
#undef  CONFIG_SIS900
#undef  CONFIG_EPIC100
#undef  CONFIG_SUNDANCE
#undef  CONFIG_TLAN
#undef  CONFIG_VIA_RHINE
#undef  CONFIG_WINBOND_840
#undef  CONFIG_HAPPYMEAL
#undef  CONFIG_LAN_SAA9730
#undef  CONFIG_NET_POCKET

/*
 * Ethernet (1000 Mbit)
 */
#undef  CONFIG_ACENIC
#undef  CONFIG_HAMACHI
#undef  CONFIG_YELLOWFIN
#undef  CONFIG_SK98LIN
#undef  CONFIG_FDDI
#undef  CONFIG_HIPPI
#undef  CONFIG_PPP
#undef  CONFIG_SLIP

/*
 * Wireless LAN (non-hamradio)
 */
#undef  CONFIG_NET_RADIO

/*
 * Token Ring devices
 */
#undef  CONFIG_TR
#undef  CONFIG_NET_FC
#undef  CONFIG_RCPCI
#undef  CONFIG_SHAPER

/*
 * Wan interfaces
 */
#undef  CONFIG_WAN

/*
 * Amateur Radio support
 */
#undef  CONFIG_HAMRADIO

/*
 * IrDA (infrared) support
 */
#undef  CONFIG_IRDA

/*
 * ISDN subsystem
 */
#undef  CONFIG_ISDN

/*
 * Old CD-ROM drivers (not SCSI, not IDE)
 */
#undef  CONFIG_CD_NO_IDESCSI

/*
 * Character devices
 */
#undef  CONFIG_VT
#define CONFIG_SERIAL 1
#define CONFIG_SERIAL_CONSOLE 1
#undef  CONFIG_SERIAL_EXTENDED
#undef  CONFIG_SERIAL_NONSTANDARD
#undef  CONFIG_UNIX98_PTYS

/*
 * I2C support
 */
#undef  CONFIG_I2C

/*
 * Mice
 */
#undef  CONFIG_BUSMOUSE
#undef  CONFIG_MOUSE

/*
 * Joysticks
 */
#undef  CONFIG_JOYSTICK

/*
 * Input core support is needed for joysticks
 */
#undef  CONFIG_QIC02_TAPE

/*
 * Watchdog Cards
 */
#undef  CONFIG_WATCHDOG
#undef  CONFIG_INTEL_RNG
#undef  CONFIG_NVRAM
#undef  CONFIG_RTC
#undef  CONFIG_DTLK
#undef  CONFIG_R3964
#undef  CONFIG_APPLICOM

/*
 * Ftape, the floppy tape device driver
 */
#undef  CONFIG_FTAPE
#undef  CONFIG_AGP
#undef  CONFIG_DRM

/*
 * Multimedia devices
 */
#undef  CONFIG_VIDEO_DEV

/*
 * File systems
 */
#undef  CONFIG_QUOTA
#undef  CONFIG_AUTOFS_FS
#undef  CONFIG_AUTOFS4_FS
#undef  CONFIG_REISERFS_FS
#undef  CONFIG_REISERFS_CHECK
#undef  CONFIG_ADFS_FS
#undef  CONFIG_ADFS_FS_RW
#undef  CONFIG_AFFS_FS
#undef  CONFIG_HFS_FS
#undef  CONFIG_BFS_FS
#undef  CONFIG_FAT_FS
#undef  CONFIG_MSDOS_FS
#undef  CONFIG_UMSDOS_FS
#undef  CONFIG_VFAT_FS
#undef  CONFIG_EFS_FS
#undef  CONFIG_JFFS_FS
#undef  CONFIG_CRAMFS
#undef  CONFIG_RAMFS
#define CONFIG_ISO9660_FS 1
#undef  CONFIG_JOLIET
#undef  CONFIG_MINIX_FS
#undef  CONFIG_NTFS_FS
#undef  CONFIG_NTFS_RW
#undef  CONFIG_HPFS_FS
#define CONFIG_PROC_FS 1
#undef  CONFIG_DEVFS_FS
#undef  CONFIG_DEVFS_MOUNT
#undef  CONFIG_DEVFS_DEBUG
#undef  CONFIG_DEVPTS_FS
#undef  CONFIG_QNX4FS_FS
#undef  CONFIG_QNX4FS_RW
#undef  CONFIG_ROMFS_FS
#define CONFIG_EXT2_FS 1
#undef  CONFIG_SYSV_FS
#undef  CONFIG_SYSV_FS_WRITE
#undef  CONFIG_UDF_FS
#undef  CONFIG_UDF_RW
#undef  CONFIG_UFS_FS
#undef  CONFIG_UFS_FS_WRITE

/*
 * Network File Systems
 */
#undef  CONFIG_CODA_FS
#define CONFIG_NFS_FS 1
#define CONFIG_NFS_V3 1
#define CONFIG_ROOT_NFS 1
#undef  CONFIG_NFSD
#undef  CONFIG_NFSD_V3
#define CONFIG_SUNRPC 1
#define CONFIG_LOCKD 1
#define CONFIG_LOCKD_V4 1
#undef  CONFIG_SMB_FS
#undef  CONFIG_NCP_FS
#undef  CONFIG_NCPFS_PACKET_SIGNING
#undef  CONFIG_NCPFS_IOCTL_LOCKING
#undef  CONFIG_NCPFS_STRONG
#undef  CONFIG_NCPFS_NFS_NS
#undef  CONFIG_NCPFS_OS2_NS
#undef  CONFIG_NCPFS_SMALLDOS
#undef  CONFIG_NCPFS_NLS
#undef  CONFIG_NCPFS_EXTRAS

/*
 * Partition Types
 */
#define CONFIG_PARTITION_ADVANCED 1
#undef  CONFIG_ACORN_PARTITION
#undef  CONFIG_OSF_PARTITION
#undef  CONFIG_AMIGA_PARTITION
#undef  CONFIG_ATARI_PARTITION
#undef  CONFIG_MAC_PARTITION
#define CONFIG_MSDOS_PARTITION 1
#undef  CONFIG_BSD_DISKLABEL
#undef  CONFIG_MINIX_SUBPARTITION
#undef  CONFIG_SOLARIS_X86_PARTITION
#undef  CONFIG_UNIXWARE_DISKLABEL
#define CONFIG_SGI_PARTITION 1
#undef  CONFIG_ULTRIX_PARTITION
#undef  CONFIG_SUN_PARTITION
#undef  CONFIG_SMB_NLS
#undef  CONFIG_NLS

/*
 * Sound
 */
#undef  CONFIG_SOUND

/*
 * USB support
 */
#undef  CONFIG_USB

/*
 * Input core support
 */
#undef  CONFIG_INPUT

/*
 * Kernel hacking
 */
#define CONFIG_CROSSCOMPILE 1
#undef  CONFIG_REMOTE_DEBUG
#undef  CONFIG_LL_DEBUG
#undef  CONFIG_MAGIC_SYSRQ
#undef  CONFIG_MIPS_UNCACHED
