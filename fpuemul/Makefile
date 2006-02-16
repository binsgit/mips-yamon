# ********************************************************************
#
# Makefile used for building FPU emulator.
# 
# Note, that this makefile uses forward slashes '/' for directory
# references.
#
# ********************************************************************
#
# mips_start_of_legal_notice
# 
# Copyright (c) 2006 MIPS Technologies, Inc. All rights reserved.
#
#
# Unpublished rights (if any) reserved under the copyright laws of the
# United States of America and other countries.
#
# This code is proprietary to MIPS Technologies, Inc. ("MIPS Technologies").
# Any copying, reproducing, modifying or use of this code (in whole or in
# part) that is not expressly permitted in writing by MIPS Technologies or
# an authorized third party is strictly prohibited. At a minimum, this code
# is protected under unfair competition and copyright laws. Violations
# thereof may result in criminal penalties and fines.
#
# MIPS Technologies reserves the right to change this code to improve
# function, design or otherwise. MIPS Technologies does not assume any
# liability arising out of the application or use of this code, or of any
# error or omission in such code. Any warranties, whether express,
# statutory, implied or otherwise, including but not limited to the implied
# warranties of merchantability or fitness for a particular purpose, are
# excluded. Except as expressly provided in any written license agreement
# from MIPS Technologies or an authorized third party, the furnishing of
# this code does not give recipient any license to any intellectual property
# rights, including any patent rights, that cover this code.
#
# This code shall not be exported, reexported, transferred, or released,
# directly or indirectly, in violation of the law of any country or
# international law, regulation, treaty, Executive Order, statute,
# amendments or supplements thereto. Should a conflict arise regarding the
# export, reexport, transfer, or release of this code, the laws of the
# United States of America shall be the governing law.
#
# This code constitutes one or more of the following: commercial computer
# software, commercial computer software documentation or other commercial
# items. If the user of this code, or any related documentation of any kind,
# including related technical data or manuals, is an agency, department, or
# other entity of the United States government ("Government"), the use,
# duplication, reproduction, release, modification, disclosure, or transfer
# of this code, or any related documentation of any kind, is restricted in
# accordance with Federal Acquisition Regulation 12.212 for civilian
# agencies and Defense Federal Acquisition Regulation Supplement 227.7202
# for military agencies. The use of this code by the Government is further
# restricted in accordance with the terms of the license agreement(s) and/or
# applicable contract terms and conditions covering this code from MIPS
# Technologies or an authorized third party.
#
#
#
# 
# mips_end_of_legal_notice
# 
#
# ********************************************************************


# ********************************************************************
# Environment specifics.
#
# These (down to and including BSS_OLD, see below) are the only 
# things, you have to adapt to your specific environment.
# ********************************************************************

# Shell commands
RM        = rm
CD	  = cd
MKDIR     = mkdir
ECHO	  = echo
CAT       = cat

# ********************************************************************
# Definitions common to start, little- and big-endian images.
# ********************************************************************

ifeq ($(TOOLCHAIN),sde)
 ifneq (,$(strip $(filter 2,$(GCC_VERSION))))
  AS_OPTS   = -no-traditional-cpp
 endif
endif

# ********************************************************************
# Different assemblers have different requirements for how to
# indicate that the next section is bss :
#
# Some use :   .bss
# Others use : .section bss
#
# We select which to use based on whether symbol BSS_OLD is 0 or not.
#  
# BSS_OLD = 0  : .bss 
# BSS_OLD != 0 : .section bss
# ********************************************************************
BSS_OLD = 0

# ********************************************************************
# Target prefix.
# ********************************************************************
IMAGENAME = fpuemul

# ********************************************************************
# Target filenames for little and big endian code.
# ********************************************************************
ELF_EL  = $(IMAGENAME)_el.elf

ELF_EB  = $(IMAGENAME)_eb.elf

# ********************************************************************
# Make calls itself recursively in order to build the little- and 
# big-endian images.
# If ENDIAN is defined, this is such a recursive call.
# ********************************************************************

ifdef ENDIAN

# Setup the definitions required to build the little- or
# big-endian image (from directory bin/EL or bin/EB).

# Root directory of YAMON source code.
ROOT      = ..

# Directory where make is supposed to be invoked.
MAKEDIR   = ..

# Various filenames.
IMAGE_ELF    = ./$(ELF_$(ENDIAN))
IMAGE_MAP = $(IMAGENAME).map


else  # ifdef ENDIAN

# If ENDIAN is not defined, this makefile was called from the command prompt
# (i.e. "make"). 
# We will first make the startup code that determines endianness, then
# make the little- and big-endian images, then concatenate them.

# Setup the definitions required to make the startup code that 
# determines endianness (from bin directory).

# Root directory of YAMON source code.
ROOT      = .

# Directory where make is supposed to be invoked.
MAKEDIR   = .

# The startup code is arbitrarily compiled for big endian.
ENDIAN       = EB

# Only one directory holds the source files for the startup code.
SRCDIR       = $(ROOT)/init/reset

endif # ifdef ENDIAN

# ********************************************************************
# Definitions common to start, little- and big-endian images.
# ********************************************************************

# Directories holding the source files.
SRCDIR    = $(ROOT)/. $(ROOT)/math

# Paths for making the little- and big-endian images.
EL_PATH      = ./EL
EB_PATH      = ./EB

# Path to include directory.
INCLUDE   = -I$(ROOT)/include

W_OPTS    = -Wimplicit -Wformat

DEFINES   = -D_32_ -D__KERNEL__ -DSTANDALONE_EMULATOR
CC_OPTS   = -mips2 -g $(CPUTUNE) $(DEFINES) -O2 -G0 $(W_OPTS) $(INCLUDE)

ifeq ($(TOOLCHAIN),linux)
CC_OPTS   += -mno-abicalls -fno-pic
endif

ifeq ($(BSS_OLD),0)
CC_OPTS_A = $(CC_OPTS) $(AS_OPTS) -D_ASSEMBLER_
else
CC_OPTS_A = $(CC_OPTS) $(AS_OPTS) -D_ASSEMBLER_ -D_BSS_OLD_
endif

# This makefile.
MAKEFILE  = $(MAKEDIR)/Makefile

# Options to linker (currently none).
LD_OPTS   =

# Linker output format.
ifeq ($(ENDIAN),EB)
  OFORMAT   = $(BFDPREF)bigmips
else
  OFORMAT   = $(BFDPREF)littlemips
endif

# Search path for source files.
VPATH  = $(SRCDIR)

# Source files (C and Assembler).
SRC_C  = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_S  = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.S))
SRC    = $(SRC_C) $(SRC_S)

# Object files.
OBJ_C  = $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_S  = $(notdir $(patsubst %.S, %.o, $(SRC_S)))
OBJ    = $(OBJ_C) $(OBJ_S)

# ********************************************************************
# Targets
# ********************************************************************

.PHONY : all install emul_el emul_eb release clean depend dep 

all : emul_el emul_eb

install :
	-$(MKDIR) -p EL
	-$(MKDIR) -p EB

release :
	$(MAKE) clean        
	$(MAKE) all

$(IMAGE_ELF) : $(OBJ)
	$(LD) $(LD_OPTS) -r -o $(IMAGE_ELF) --oformat $(OFORMAT) $(OBJ)

$(OBJ_C) : %.o : %.c 
	$(CC) -c $(CC_OPTS) -$(ENDIAN) -D$(ENDIAN) -o $@ $<

$(OBJ_S) : %.o : %.S
	$(CC) -c $(CC_OPTS_A) -$(ENDIAN) -D$(ENDIAN) -o $@ $<

$(OBJ) : $(MAKEFILE)

emul_el :
	$(CD) $(EL_PATH); $(MAKE) -f ../$(MAKEFILE) ENDIAN=EL $(ELF_EL)

emul_eb :
	$(CD) $(EB_PATH); $(MAKE) -f ../$(MAKEFILE) ENDIAN=EB $(ELF_EB)

CLEAN_FILES = ./*.o ./*.elf ./depend.mk

clean :
	-$(RM) -f $(CLEAN_FILES)
	-$(CD) $(EL_PATH); $(RM) -f $(CLEAN_FILES)
	-$(CD) $(EB_PATH); $(RM) -f $(CLEAN_FILES)

depend: dep
	$(CD) $(EL_PATH); $(MAKE) -f ../$(MAKEFILE) ENDIAN=EL dep
	$(CD) $(EB_PATH); $(MAKE) -f ../$(MAKEFILE) ENDIAN=EB dep

dep:
	$(CC) $(DEFINES) $(INCLUDE) -M $(SRC) > ./depend.mk

-include ./depend.mk
