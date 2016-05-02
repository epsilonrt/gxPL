###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

# Target Name (without extension).
TARGET = gxpl-test-core-avr

# Enable logging messages (if OFF no display is performed)
LOG = ON

# Relative path from the root directory of the user's project
PROJECT_TOPDIR = ../../..

# Default path where the library has been installed
PREFIX ?= $(PROJECT_TOPDIR)/local/lib/avr

# User Project
# USER_PROJECT = GXPL

# Usually the MCU is defined in board.mk. It is possible to change it in 
# the case where the model is not implanted (For boards with IC socket).
# MCU = atmega32

# Normally F_CPU defined in board.mk ... It is possible to modify it if the 
# frequency is not fixed by a crystal (internal clock ...)
# F_CPU = 8000000

#----------------------------------------------------------------------------
#                               ~~~~AVRIO~~~~
# Relative path of the AVRIO root directory 
AVRIO_TOPDIR = $(AVRIO_ROOT)

# Nom de la carte cible (voir le fichier avrio/boards.txt pour la liste)
# Si BOARD est vide, il faut un fichier board.mk pour définir les propriétés de
# la carte dans le répertoire où se trouve ce Makefile !
BOARD = DVK90CAN1_XBEE

# Configuration de AvrIO
# -------------------------------
AVRIO_CONFIG  =

ifeq ($(LOG),ON)
AVRIO_CONFIG += __ASSERT_USE_STDERR
else
AVRIO_CONFIG += NLOG
endif

# Valide le noyau multitâche temps réel AVRX (sous forme de bibliothèque)
AVRX = OFF

# Valide l'intégration d'Arduino (sous forme de bibliothèque: ON/OFF)
AVRIO_ARDUINO = OFF

# Utilisation de la chaîne de compilation interne à AvrIO
USE_INTERNAL_TOOLCHAIN = OFF

#                               ~~~~AVRIO~~~~
#----------------------------------------------------------------------------

# Generates a file to retrieve information on the GIT Version
GIT_VERSION = ON

# Optimization level, can be [0, 1, 2, 3, s]. 0 turns off optimization.
# (Note: 3 is not always the best optimization level)
OPT = s

# Debugging information format
DEBUG_FORMAT = dwarf-2

# Optimization level for debug, can be [0, 1, 2, 3, s]. 0 turns off optimization.
# (Note: 3 is not always the best optimization level)
DEBUG_OPT = s

# Enabling Debug information (ON / OFF)
#DEBUG = ON

# Displays the GCC compile line or not (ON / OFF)
#VIEW_GCC_LINE = ON

# Disable the deletion of variables and functions "unnecessary"
# The linker checks of a function or variable is called, if it is not the case, 
# it removes the variable or function. This can be problematic in some cases (bootloarder!)
DISABLE_DELETE_UNUSED_SECTIONS = OFF

# List C source files here. (C dependencies are automatically generated.)
SRC  = test/core/gxpl-test-core.c

# List C++ source files here. (C++ dependencies are automatically generated.)
CPPSRC =

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# Place -D or -U options here for C sources
CDEFS +=

# Place -D or -U options here for ASM sources
ADEFS +=

# Place -D or -U options here for C++ sources
CPPDEFS +=

# Enable gcc warning (without -W)
WARNINGS = all strict-prototypes no-unused-but-set-variable

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRA_INCDIRS = $(PROJECT_TOPDIR)/lib/avr

#---------------- Library Options ----------------
# Enable linking with gxPL library (ON/OFF)
#USE_GXPL_LIB = ON

# List any extra directories to look for libraries here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRA_LIBDIRS =

# List any extra libraries here.
#     Each library must be seperated by a space.
EXTRA_LIBS =

# STANDARD, MIN or FLOAT
PRINTF_VERSION = FLOAT

# STANDARD, MIN or FLOAT
SCANF_VERSION = STANDARD

# ON/OFF
MATH_LIB_ENABLE = ON

#---------------- Additional Compiler/Assembler Options ----------------
CFLAGS   += -fno-strict-aliasing
CPPFLAGS += 
ASFLAGS  += 
#---------------- Additional Linker Options ----------------
LDFLAGS  +=

#---------------- Programming Options (avrdude) ----------------
# Programming hardware
# Type: avrdude -c ?
# to get a full listing.
#
# STK200 progcd rammer on parallel port
#AVRDUDE_PROGRAMMER = stk200
#AVRDUDE_PORT = lpt1

# JTAG ICE MkII
#AVRDUDE_PROGRAMMER  = jtag2
#AVRDUDE_PORT = usb

# AVR Dragon
#AVRDUDE_PROGRAMMER = dragon_isp
AVRDUDE_PROGRAMMER = dragon_jtag
#AVRDUDE_PROGRAMMER = dragon_dw
AVRDUDE_PORT = usb

# Arduino OnBoard Programmer
#AVRDUDE_PROGRAMMER = arduino
#AVRDUDE_PORT = /dev/ttyACM0
#AVRDUDE_PORT = COM4

# ON/OFF
AVRDUDE_WRITE_EEPROM_ENABLE = OFF

#---------------- Debugging Options ----------------

# Set the DEBUG_UI to either gdb, cgdb or insight.
#DEBUG_UI = gdb
DEBUG_UI = cgdb
#DEBUG_UI = insight

# Set the debugging back-end to either avarice, simulavr.
DEBUG_BACKEND = avarice
#DEBUG_BACKEND = simulavr

# When using avarice settings for the JTAG
JTAG_DEV = usb

AVARICE_OPT = --dragon 
#AVARICE_OPT = --mkII
AVARICE_BITRATE = --jtag-bitrate 2MHz

#------------------------------------------------------------------------------
ifeq ($(GXPL_ROOT),)
GXPL_ROOT := $(PROJECT_TOPDIR)
endif

include $(GXPL_ROOT)/gxpl.mk
include $(GXPL_ROOT)/avrio.mk
