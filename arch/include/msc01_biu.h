/* ##########################################################################
 *
 *  BIU Header File
 *
 * ##########################################################################
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
 * ###########################################################################
 */

#ifndef MSC01_BIU_H
#define MSC01_BIU_H

/* System Controller ID in MSC01_SC_ID */
#define MSC01_ID_SC_EC32                0x01
#define MSC01_ID_SC_EC64                0x02
#define MSC01_ID_SC_MGB                 0x03
#define MSC01_ID_SC_MGBIIA36D64C0IO     0x06
#define MSC01_ID_SC_OCP			0x08

/*****************************************************************************
 * Register offset addresses
 ****************************************************************************/

#define MSC01_SC_ID_OFS		        0x0000
#define MSC01_SC_SYSID_OFS		0x0008
#define MSC01_SC_ERR_OFS		0x0108
#define MSC01_SC_CFG_OFS		0x0110
#define MSC01_BIU_IP1BAS1H_OFS		0x0200
#define MSC01_BIU_IP1BAS1L_OFS		0x0208
#define MSC01_BIU_IP1MSK1H_OFS		0x0210
#define MSC01_BIU_IP1MSK1L_OFS		0x0218
#define MSC01_BIU_IP1BAS2H_OFS		0x0240
#define MSC01_BIU_IP1BAS2L_OFS		0x0248
#define MSC01_BIU_IP1MSK2H_OFS		0x0250
#define MSC01_BIU_IP1MSK2L_OFS		0x0258
#define MSC01_BIU_IP2BAS1H_OFS		0x0280
#define MSC01_BIU_IP2BAS1L_OFS		0x0288
#define MSC01_BIU_IP2MSK1H_OFS		0x0290
#define MSC01_BIU_IP2MSK1L_OFS		0x0298
#define MSC01_BIU_IP2BAS2H_OFS		0x02c0
#define MSC01_BIU_IP2BAS2L_OFS		0x02c8
#define MSC01_BIU_IP2MSK2H_OFS		0x02d0
#define MSC01_BIU_IP2MSK2L_OFS		0x02d8
#define MSC01_BIU_IP3BAS1H_OFS		0x0300
#define MSC01_BIU_IP3BAS1L_OFS		0x0308
#define MSC01_BIU_IP3MSK1H_OFS		0x0310
#define MSC01_BIU_IP3MSK1L_OFS		0x0318
#define MSC01_BIU_IP3BAS2H_OFS		0x0340
#define MSC01_BIU_IP3BAS2L_OFS		0x0348
#define MSC01_BIU_IP3MSK2H_OFS		0x0350
#define MSC01_BIU_IP3MSK2L_OFS		0x0358
#define MSC01_BIU_MCBAS1H_OFS		0x0380
#define MSC01_BIU_MCBAS1L_OFS		0x0388
#define MSC01_BIU_MCMSK1H_OFS		0x0390
#define MSC01_BIU_MCMSK1L_OFS		0x0398
#define MSC01_BIU_MCBAS2H_OFS		0x03c0
#define MSC01_BIU_MCBAS2L_OFS		0x03c8
#define MSC01_BIU_MCMSK2H_OFS		0x03d0
#define MSC01_BIU_MCMSK2L_OFS		0x03d8
#define MSC01_BIU_RBBASH_OFS		0x07f0
#define MSC01_BIU_RBBASL_OFS		0x07f8
#define MSC01_BIU_IP1CTRL_OFS		0x0800
#define MSC01_BIU_IP2CTRL_OFS		0x0808
#define MSC01_BIU_IP3CTRL_OFS		0x0810
#define MSC01_BIU_MCCTRL_OFS		0x0818

/*****************************************************************************
 * Register encodings
 ****************************************************************************/

#define MSC01_SC_ID_ID_SHF		16
#define MSC01_SC_ID_ID_MSK		0x00ff0000
#define MSC01_SC_ID_MAR_SHF		8
#define MSC01_SC_ID_MAR_MSK		0x0000ff00
#define MSC01_SC_ID_MIR_SHF		0
#define MSC01_SC_ID_MIR_MSK		0x000000ff

#define MSC01_SC_SYSID_ID_SHF		0
#define MSC01_SC_SYSID_ID_MSK		0xffffffff

#define MSC01_SC_ERR_RSPERR_SHF		31
#define MSC01_SC_ERR_RSPERR_MSK		0x80000000
#define MSC01_SC_ERR_RSPERR_BIT		0x80000000
#define MSC01_SC_ERR_RSPID_SHF		8
#define MSC01_SC_ERR_RSPID_MSK		0x0000ff00
#define MSC01_SC_ERR_RSPTYP_SHF	        0
#define MSC01_SC_ERR_RSPTYP_MSK	        0x00000001
#define MSC01_SC_ERR_RSPTYP_BIT	        0x00000001

#define MSC01_SC_CFG_WC_SHF		16
#define MSC01_SC_CFG_WC_MSK		0x00010000
#define MSC01_SC_CFG_WC_BIT		MSC01_SC_CFG_WC_MSK
#define MSC01_SC_CFG_EBIG_SHF		8
#define MSC01_SC_CFG_EBIG_MSK		0x00000100
#define MSC01_SC_CFG_EBIG_BIT		MSC01_SC_CFG_EBIG_MSK
#define MSC01_SC_CFG_BSEL_SHF		0
#define MSC01_SC_CFG_BSEL_MSK		0x00000003

#define MSC01_BIU_IP1BAS1H_BASH_SHF	0
#define MSC01_BIU_IP1BAS1H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP1BAS1L_BASL_SHF	22
#define MSC01_BIU_IP1BAS1L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP1MSK1H_MSKH_SHF	0
#define MSC01_BIU_IP1MSK1H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP1MSK1L_MSKL_SHF	22
#define MSC01_BIU_IP1MSK1L_MSKL_MSK	0xffc00000
#define MSC01_BIU_IP1BAS2H_BASH_SHF	0
#define MSC01_BIU_IP1BAS2H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP1BAS2L_BASL_SHF	22
#define MSC01_BIU_IP1BAS2L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP1MSK2H_MSKH_SHF	0
#define MSC01_BIU_IP1MSK2H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP1MSK2L_MSKL_SHF	22
#define MSC01_BIU_IP1MSK2L_MSKL_MSK	0xffc00000

#define MSC01_BIU_IP2BAS1H_BASH_SHF	0
#define MSC01_BIU_IP2BAS1H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP2BAS1L_BASL_SHF	22
#define MSC01_BIU_IP2BAS1L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP2MSK1H_MSKH_SHF	0
#define MSC01_BIU_IP2MSK1H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP2MSK1L_MSKL_SHF	22
#define MSC01_BIU_IP2MSK1L_MSKL_MSK	0xffc00000
#define MSC01_BIU_IP2BAS2H_BASH_SHF	0
#define MSC01_BIU_IP2BAS2H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP2BAS2L_BASL_SHF	22
#define MSC01_BIU_IP2BAS2L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP2MSK2H_MSKH_SHF	0
#define MSC01_BIU_IP2MSK2H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP2MSK2L_MSKL_SHF	22
#define MSC01_BIU_IP2MSK2L_MSKL_MSK	0xffc00000

#define MSC01_BIU_IP3BAS1H_BASH_SHF	0
#define MSC01_BIU_IP3BAS1H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP3BAS1L_BASL_SHF	22
#define MSC01_BIU_IP3BAS1L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP3MSK1H_MSKH_SHF	0
#define MSC01_BIU_IP3MSK1H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP3MSK1L_MSKL_SHF	22
#define MSC01_BIU_IP3MSK1L_MSKL_MSK	0xffc00000
#define MSC01_BIU_IP3BAS2H_BASH_SHF	0
#define MSC01_BIU_IP3BAS2H_BASH_MSK	0x0000000f
#define MSC01_BIU_IP3BAS2L_BASL_SHF	22
#define MSC01_BIU_IP3BAS2L_BASL_MSK	0xffc00000
#define MSC01_BIU_IP3MSK2H_MSKH_SHF	0
#define MSC01_BIU_IP3MSK2H_MSKH_MSK	0x0000000f
#define MSC01_BIU_IP3MSK2L_MSKL_SHF	22
#define MSC01_BIU_IP3MSK2L_MSKL_MSK	0xffc00000

#define MSC01_BIU_MCBAS1H_BASH_SHF	0
#define MSC01_BIU_MCBAS1H_BASH_MSK	0x0000000f
#define MSC01_BIU_MCBAS1L_BASL_SHF	22
#define MSC01_BIU_MCBAS1L_BASL_MSK	0xffc00000
#define MSC01_BIU_MCMSK1H_MSKH_SHF	0
#define MSC01_BIU_MCMSK1H_MSKH_MSK	0x0000000f
#define MSC01_BIU_MCMSK1L_MSKL_SHF	22
#define MSC01_BIU_MCMSK1L_MSKL_MSK	0xffc00000
#define MSC01_BIU_MCBAS2H_BASH_SHF	0
#define MSC01_BIU_MCBAS2H_BASH_MSK	0x0000000f
#define MSC01_BIU_MCBAS2L_BASL_SHF	22
#define MSC01_BIU_MCBAS2L_BASL_MSK	0xffc00000
#define MSC01_BIU_MCMSK2H_MSKH_SHF	0
#define MSC01_BIU_MCMSK2H_MSKH_MSK	0x0000000f
#define MSC01_BIU_MCMSK2L_MSKL_SHF	22
#define MSC01_BIU_MCMSK2L_MSKL_MSK	0xffc00000

#define MSC01_BIU_RBBAS_BASH_SHF	0
#define MSC01_BIU_RBBAS_BASH_MSK	0x0000000f
#define MSC01_BIU_RBMSK_BASL_SHF	22
#define MSC01_BIU_RBMSK_BASL_MSK	0xffc00000

#define MSC01_BIU_IP1CTRL_L_SHF	        0
#define MSC01_BIU_IP1CTRL_L_MSK	        0x00000001
#define MSC01_BIU_IP1CTRL_L_BIT	        MSC01_BIU_IP1CTRL_L_MSK

#define MSC01_BIU_IP2CTRL_L_SHF	        0
#define MSC01_BIU_IP2CTRL_L_MSK	        0x00000001
#define MSC01_BIU_IP2CTRL_L_BIT	        MSC01_BIU_IP2CTRL_L_MSK

#define MSC01_BIU_IP3CTRL_L_SHF	        0
#define MSC01_BIU_IP3CTRL_L_MSK	        0x00000001
#define MSC01_BIU_IP3CTRL_L_BIT	        MSC01_BIU_IP3CTRL_L_MSK

#define MSC01_BIU_MCCTRL_L_SHF	        0
#define MSC01_BIU_MCCTRL_L_MSK	        0x00000001
#define MSC01_BIU_MCCTRL_L_BIT	        MSC01_BIU_MCCTRL_L_MSK


/* BIU
   Registers absolute addresses: */

#define MSC01_SC_ID             (MSC01_BIU_REG_BASE + MSC01_SC_ID_OFS)
#define MSC01_SC_ERR            (MSC01_BIU_REG_BASE + MSC01_SC_ERR_OFS)
#define MSC01_SC_CFG            (MSC01_BIU_REG_BASE + MSC01_SC_CFG_OFS)

#define MSC01_BIU_IP1BAS1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1BAS1H_OFS)
#define MSC01_BIU_IP1BAS1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1BAS1L_OFS)
#define MSC01_BIU_IP1MSK1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1MSK1H_OFS)
#define MSC01_BIU_IP1MSK1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1MSK1L_OFS)
#define MSC01_BIU_IP1BAS2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1BAS2H_OFS)
#define MSC01_BIU_IP1BAS2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1BAS2L_OFS)
#define MSC01_BIU_IP1MSK2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1MSK2H_OFS)
#define MSC01_BIU_IP1MSK2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP1MSK2L_OFS)
#define MSC01_BIU_IP2BAS1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2BAS1H_OFS)
#define MSC01_BIU_IP2BAS1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2BAS1L_OFS)
#define MSC01_BIU_IP2MSK1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2MSK1H_OFS)
#define MSC01_BIU_IP2MSK1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2MSK1L_OFS)
#define MSC01_BIU_IP2BAS2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2BAS2H_OFS)
#define MSC01_BIU_IP2BAS2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2BAS2L_OFS)
#define MSC01_BIU_IP2MSK2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2MSK2H_OFS)
#define MSC01_BIU_IP2MSK2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP2MSK2L_OFS)
#define MSC01_BIU_IP3BAS1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3BAS1H_OFS)
#define MSC01_BIU_IP3BAS1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3BAS1L_OFS)
#define MSC01_BIU_IP3MSK1H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3MSK1H_OFS)
#define MSC01_BIU_IP3MSK1L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3MSK1L_OFS)
#define MSC01_BIU_IP3BAS2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3BAS2H_OFS)
#define MSC01_BIU_IP3BAS2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3BAS2L_OFS)
#define MSC01_BIU_IP3MSK2H      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3MSK2H_OFS)
#define MSC01_BIU_IP3MSK2L      (MSC01_BIU_REG_BASE + MSC01_BIU_IP3MSK2L_OFS)
#define MSC01_BIU_MCBAS1H       (MSC01_BIU_REG_BASE + MSC01_BIU_MCBAS1H_OFS)
#define MSC01_BIU_MCBAS1L       (MSC01_BIU_REG_BASE + MSC01_BIU_MCBAS1L_OFS)
#define MSC01_BIU_MCMSK1H       (MSC01_BIU_REG_BASE + MSC01_BIU_MCMSK1H_OFS)
#define MSC01_BIU_MCMSK1L       (MSC01_BIU_REG_BASE + MSC01_BIU_MCMSK1L_OFS)
#define MSC01_BIU_MCBAS2H       (MSC01_BIU_REG_BASE + MSC01_BIU_MCBAS2H_OFS)
#define MSC01_BIU_MCBAS2L       (MSC01_BIU_REG_BASE + MSC01_BIU_MCBAS2L_OFS)
#define MSC01_BIU_MCMSK2H       (MSC01_BIU_REG_BASE + MSC01_BIU_MCMSK2H_OFS)
#define MSC01_BIU_MCMSK2L       (MSC01_BIU_REG_BASE + MSC01_BIU_MCMSK2L_OFS)
#define MSC01_BIU_RBBASH        (MSC01_BIU_REG_BASE + MSC01_BIU_RBBASH_OFS)
#define MSC01_BIU_RBBASL        (MSC01_BIU_REG_BASE + MSC01_BIU_RBBASL_OFS)
#define MSC01_BIU_IP1CTRL       (MSC01_BIU_REG_BASE + MSC01_BIU_IP1CTRL_OFS)
#define MSC01_BIU_IP2CTRL       (MSC01_BIU_REG_BASE + MSC01_BIU_IP2CTRL_OFS)
#define MSC01_BIU_IP3CTRL       (MSC01_BIU_REG_BASE + MSC01_BIU_IP3CTRL_OFS)
#define MSC01_BIU_MCCTRL        (MSC01_BIU_REG_BASE + MSC01_BIU_MCCTRL_OFS)


#endif
