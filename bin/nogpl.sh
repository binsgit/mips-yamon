#!/bin/bash
#
# mips_start_of_legal_notice
# 
# Copyright (c) 2008 MIPS Technologies, Inc. All rights reserved.
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
# This code shall not be exported or transferred for the purpose of
# reexporting in violation of any U.S. or non-U.S. regulation, treaty,
# Executive Order, law, statute, amendment or supplement thereto.
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
# mips_end_of_legal_notice
# 
#

if [ -d ../fpuemul/math ];then
    echo " "
    echo "Removing the GPL FPU emulator"
    echo "-----------------------------"
    echo "NB: This operation is 100% reversible, but you *must* be aware"
    echo "of the side effects of non-IEEE FP support as described in"
    echo "Chapter 7 of 'See MIPS Run Linux' 2nd Edition"
    echo "(particularly Section 7.4 on page 158)"
    echo " "
    echo "Would you like to remove the GPL FPU emulator code from this"
    echo "YAMON build system ?  (y/n)"
    read CHOICE
    if [ -z "${CHOICE}" ]; then
        echo "No changes made"
        exit
    fi
    case "${CHOICE}" in
        "y")
            MOVE=1
            ;;
        "Y")
            MOVE=1
            ;;
        "yes")
            MOVE=1
            ;;
        "Yes")
            MOVE=1
            ;;
        *)
            MOVE=0
            ;;
    esac
    if [ 1 -eq ${MOVE} ]; then
        mv ../fpuemul/math ../fpuemul/no-math
        echo "GPL FPU code removed"
        echo "Please rebuild YAMON"
    else
        echo "No changes made"
    fi
else
    if [ -d ../fpuemul/no-math ]; then
        echo " "
        echo "Adding the GPL FPU emulator"
        echo "---------------------------"
        echo "NB: This operation is 100% reversible, but you *must* be aware"
        echo "of the impact the GPL may have on your requirement to release any"
        echo "changes made to this source code."
        echo " "
        echo "Would you like to add the GPL FPU emulator code to this"
        echo "YAMON build system ?  (y/n)"
        read CHOICE
        if [ -z "${CHOICE}" ]; then
            echo "No changes made"
            exit
        fi
        case "${CHOICE}" in
            "y")
                MOVE=1
                ;;
            "Y")
                MOVE=1
                ;;
            "yes")
                MOVE=1
                ;;
            "Yes")
                MOVE=1
                ;;
            *)
                MOVE=0
                ;;
        esac
        if [ 1 -eq ${MOVE} ]; then
            mv ../fpuemul/no-math ../fpuemul/math
            echo "GPL FPU code added"
            echo "Please rebuild YAMON"
        else
            echo "No changes made"
        fi
    else
        echo "Failed to locate the FPU emulator code"
        echo "Please ensure you run this script from the bin directory"
    fi
fi
