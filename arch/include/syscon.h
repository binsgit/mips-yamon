
/************************************************************************
 *
 *  syscon.h
 *
 *  Private SYSCON definitions.
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


#ifndef SYSCON_H
#define SYSCON_H


/************************************************************************
 *  Include files
 ************************************************************************/

#include <sysdefs.h>
#include <sys_api.h>

/************************************************************************
 *  Definitions
 ************************************************************************/

/* SYSCON read/write function */
typedef  UINT32 (*t_syscon_func)(void *param, void *data, UINT32 size);

/* SYSCON object */
typedef struct
{
    t_syscon_func   read;        /* Read function  */
    void	    *read_data;  /* Read data	   */
    t_syscon_func   write;	 /* Write function */
    void	    *write_data; /* Write data	   */
}
t_syscon_obj;


/* Macro for conversion between bus cycles and time interval in ns */
#define CYCLES2NS( cycles ) (((UINT32)1e9 / sys_busfreq_hz) * (cycles))


/*  Macro for conversion between time interval in ns and number of
 *  SysAD bus cycles.
 *  Round result down.
 */
#define NS2COUNT_ROUND_DOWN(ns,count)	\
    count =  sys_busfreq_hz / 1000000;  \
    count *= ns;			\
    count /= 1000;


/************************************************************************
 *  Public variables
 ************************************************************************/

/************************************************************************
 *  Public functions
 ************************************************************************/


/************************************************************************
 *
 *                          syscon_get_eeprom_data
 *  Description :
 *  -------------
 *
 *  Read data store in EEPROM.
 *  Data is stored in the format defined in file eeprom_nm24c09_layout.h 
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_get_eeprom_data( 
    t_mac_addr **mac_addr,  /* Set to mac addr (NULL if invalid)	*/
    t_sn_bcd   **sn );	    /* Set to S/N (NULL if invalid)		*/


/************************************************************************
 *
 *                          syscon_generic_tty_init
 *  Description :
 *  -------------
 *
 *  Initialize generic TTY parts of SYSCON module.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_generic_tty_init( void );



/************************************************************************
 *
 *                          syscon_arch_tty_init
 *  Description :
 *  -------------
 *
 *  Initialize platform specific part of SYSCON TTY objects.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void
syscon_arch_tty_init( void );


/************************************************************************
 *
 *                          syscon_arch_board_init
 *  Description :
 *  -------------
 *
 *  Initialize platform specific part of SYSCON (except TTY related parts)
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_board_init( 
    t_syscon_obj *objects );


/************************************************************************
 *
 *                          syscon_arch_core_init
 *  Description :
 *  -------------
 *
 *  Initialize core card specific part of SYSCON
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_core_init( 
    t_syscon_obj *objects,		/* Array of SYSCON objects	*/
    UINT32	 nb_ram_size,		/* Size of MAX RAM range	*/
    UINT32	 nb_pci_mem_start,      /* PCI memory range start	*/
    UINT32	 nb_pci_mem_size,	/* PCI memory range size 	*/
    UINT32	 nb_pci_mem_offset,	/* PCI memory range offset	*/
    UINT32	 nb_pci_io_start,	/* PCI I/O range start		*/
    UINT32	 nb_pci_io_size,	/* PCI I/O range size 		*/
    UINT32	 nb_pci_io_offset );	/* PCI I/O range offset 	*/


/************************************************************************
 *
 *                          syscon_arch_cpu_init
 *  Description :
 *  -------------
 *
 *  Initialize cpu specific part of SYSCON
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_arch_cpu_init( 
    t_syscon_obj *objects );


/************************************************************************
 *
 *                          syscon_register_id_board
 *  Description :
 *  -------------
 *
 *  Function used to register platform specific SYSCON object functions.
 *
 *  A read and/or write function may be registered for each platform.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed for the particular platform.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.

 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_id_board(
    t_syscon_ids   id,			/* OBJECT ID from syscon_api.h	*/
    /* Atlas */
    t_syscon_func  read_atlas,		/* Atlas read function		*/
    void	   *read_data_atlas,	/* Registered data		*/
    t_syscon_func  write_atlas,		/* Atlas write function		*/
    void	   *write_data_atlas,	/* Registered data		*/
    /* SEAD */
    t_syscon_func  read_sead,		/* SEAD read function		*/
    void	   *read_data_sead,	/* Registered data		*/
    t_syscon_func  write_sead,		/* SEAD write function		*/
    void	   *write_data_sead,	/* Registered data		*/
    /* Malta */
    t_syscon_func  read_malta,		/* Malta read function		*/
    void	   *read_data_malta,	/* Registered data		*/
    t_syscon_func  write_malta,		/* Malta write function		*/
    void	   *write_data_malta ); /* Registered data		*/


/************************************************************************
 *
 *                          syscon_register_id_core
 *  Description :
 *  -------------
 *
 *  Function used to register core card specific SYSCON object functions.
 *
 *  A read and/or write function may be registered for each core card.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed for the particular platform.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.

 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_id_core(
    t_syscon_ids   id,			  /* OBJECT ID from syscon_api.h */

    /* Galileo based */
    t_syscon_func  read_galileo,	  /* Core Galileo read function  */
    void	   *read_data_galileo,	  /* Registered data		 */
    t_syscon_func  write_galileo,	  /* Core Galileo write function */
    void	   *write_data_galileo,   /* Registered data		 */

    /* SysCtrl based */
    t_syscon_func  read_sysctl,	          /* Core SysCtrl read function	 */
    void	   *read_data_sysctl,     /* Registered data		 */
    t_syscon_func  write_sysctl,	  /* Core SysCtrl write function */
    void	   *write_data_sysctl,    /* Registered data		 */

    /* Bonito64 based */
    t_syscon_func  read_bonito64,	  /* Core Bonito64 read          */
    void	   *read_data_bonito64,	  /* Registered data		 */
    t_syscon_func  write_bonito64,	  /* Core Bonito64 write	 */
    void	   *write_data_bonito64 ); /* Registered data		 */


/************************************************************************
 *
 *                          syscon_register_id_mips32
 *  Description :
 *  -------------
 *
 *  Function used to register SYSCON object functions for objects 
 *  that depend on whether CPU is MIPS32/64 or not.
 *
 *  A read and/or write function may be registered.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.

 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_id_mips32(
    t_syscon_ids   id,			 /* OBJECT ID from syscon_api.h */
    /* MIPS32/MIPS64 processor */
    t_syscon_func  read_mips32,		 /* MIPS32/64 read function	*/
    void	   *read_data_mips32,	 /* Registered data		*/
    t_syscon_func  write_mips32,	 /* MIPS32/64 write function	*/
    void	   *write_data_mips32,   /* Registered data		*/
    /* Other processor */
    t_syscon_func  read_other,		 /* "Other CPU" read function	*/
    void	   *read_data_other,	 /* Registered data		*/
    t_syscon_func  write_other,	         /* "Other CPU" write function	*/
    void	   *write_data_other );  /* Registered data		*/


/************************************************************************
 *
 *                          syscon_register_generic
 *  Description :
 *  -------------
 *
 *  Function used to register generic SYSCON object functions.
 *
 *  A read and/or write function may be registered.
 *  A NULL function pointer indicates that the operation (read or
 *  write) is not allowed.
 *
 *  read_data and write_data pointers are passed to the read and write
 *  function.
 *
 *  Return values :
 *  ---------------
 *
 *  None
 *
 ************************************************************************/
void 
syscon_register_generic(
    t_syscon_ids   id,			/* OBJECT ID from syscon_api.h	*/
    t_syscon_func  read_generic,	/* Generic read function	*/
    void	   *read_data,		/* Registered data		*/
    t_syscon_func  write_generic,	/* Generic write function	*/
    void	   *write_data );	/* Registered data		*/



/*  Functions of the format required for registered SYSCON functions.	*/

UINT32
syscon_true_read(
    void   *param,	/* bool : Set to TRUE				*/
    void   *data,
    UINT32 size );

UINT32
syscon_false_read(
    void   *param,	/* bool : Set to FALSE				*/
    void   *data,
    UINT32 size );

UINT32
syscon_endian_big_read(
    void   *param,	/* bool : Set to TRUE if big endian		*/
    void   *data,
    UINT32 size );

UINT32
syscon_string_read(
    void   *param,
    void   *data,
    UINT32 size );	/* (char *) : Set to (char *)data		*/

UINT32
syscon_uint8_read(
    void   *param,	/* UINT8 : Set to *(UINT8 *)data		*/
    void   *data,
    UINT32 size );

UINT32
syscon_uint32_read(
    void   *param,	/* UINT32 : Set to *(UINT32 *)data		*/
    void   *data,
    UINT32 size );

UINT32
syscon_bool_read(
    void   *param,	/* bool : Set to *(bool *)data			*/
    void   *data,
    UINT32 size );

UINT32
syscon_bool_write(
    void   *param,	/* *(bool *)data set to *(bool *)param		*/
    void   *data,
    UINT32 size );


#endif /* #ifndef SYSCON_H */

