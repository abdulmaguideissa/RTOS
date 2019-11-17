# ***************************************************************************
# Filename:    Makefile.
# Description: Makefile for project build chain.
# Author:      Abdulmaguid Eissa
# Date:        Oct 10, 2019
# Copyright (c) MPL2.
# Ref: https://www.coursera.org/learn/introduction-embedded-systems/
# Ref: GNUmake
# Ref: TivaWare for ek-tm4c123gxl board.
# Ref: FreeRTOS Demos
# NOTES:
#	1. Compiler is dependent on the architecture of the processor.
#	2. Linker is depndent on the entire platform used in development. 
#   3. Change the target name to match the main entry file
# ***************************************************************************


# **********************************************************
#       Makefile script to build, clean, flash and debug
# **********************************************************

#
# Defines the part type that this project uses.
#
PART = TM4C123GH6PM

#
# The target file
#
TARGET = main

#
# Root directory for TivaWare 
#
ROOT = ../../TivaWare_C_Series-2.1.4.178

#
# Include the common make definitions.
#
include $(ROOT)/makedefs

#
# Where to find source files that do not live in this directory.
#
# Current directory
CDIR=FreeRTOS/Source
# User application dir
CPATH=.
# Tivaware dir
TPATH=${ROOT}/driverlib
# Commands dir
CMDPATH=${ROOT}/utils
# FreeRTOS dir
FPATH=${CDIR}/portable/GCC/ARM_CM4F
# Memory management dir
MPATH=${CDIR}/portable/MemMang

#
# The complete c source file paths
#
VPATH=$(CPATH):$(TPATH):$(CMDPATH):$(FPATH):$(MPATH):$(CDIR)

#
# Where to find header files that do not live in the source directory.
#
IPATH=${CDIR}/include $(FPATH) $(ROOT) $(CPATH) $(TPATH)

#
# Object files dependencies
#
OBJ= $(COMPILER)/$(TARGET).o    \
	 $(COMPILER)/usertasks.o    \
	 $(COMPILER)/port.o         \
	 $(COMPILER)/tasks.o        \
	 $(COMPILER)/queue.o        \
	 $(COMPILER)/list.o         \
	 $(COMPILER)/heap_1.o       \
	 $(COMPILER)/croutine.o     \
	 $(COMPILER)/timers.o       \
	 $(COMPILER)/event_groups.o \
	 $(COMPILER)/gpio.o         \
	 $(COMPILER)/cpu.o          \
	 $(COMPILER)/sysctl.o

INIT_OBJ= $(COMPILER)/startup_$(COMPILER).o
LIBS= ${ROOT}/driverlib/${COMPILER}/libdriver.a

#
# User commands for make
#
.PHONY: all flash clean debug help $(COMPILER)

#
# The default rule, which causes the Project Zero Example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/${TARGET}.axf

#
# 
help:
	@echo "src: $(SRC)"
	@echo "obj: $(OBJ)"
#
# The rule to flash the program of the chip
#
flash:
	lm4flash ${COMPILER}/${TARGET}.bin

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}


#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the Project Zero Example.
#
${COMPILER}/${TARGET}.axf: $(INIT_OBJ) $(OBJ) $(LIBS)
${COMPILER}/${TARGET}.axf: tm4c123xx.ld
SCATTERgcc_main=tm4c123xx.ld
ENTRY_main=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C123_RB1


#
# Rule for debugging project
#
debug: clean
debug: CFLAGS+=-g -D DEBUG 
debug: ${COMPILER}
debug: ${COMPILER}/${TARGET}.axf
debug:
	sudo ./debug_nemiver.sh

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
