###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

# Target Name (without extension).
TARGET = gxPL

# Chemin relatif du répertoire racine du projet de l'utilisateur
PROJECT_TOPDIR = ../..

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
#BOARD = UNO
#BOARD = XNET_NODE
BOARD = DVK90CAN1_XBEE

# Configuration de AvrIO
# -------------------------------
AVRIO_CONFIG  = 

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
SRC  = 

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
EXTRA_INCDIRS =

#---------------- Library Options ----------------
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

#---------------- Install Options ----------------
#prefix=/usr/local/lib/avr
prefix=$(PROJECT_TOPDIR)/local/lib/avr

INSTALL_LIBDIR=$(prefix)/lib
INSTALL_INCDIR=$(prefix)/include

#------------------------------------------------------------------------------
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

ifeq ($(GXPL_ROOT),)
GXPL_ROOT := $(PROJECT_TOPDIR)
endif

all: lib

include $(GXPL_ROOT)/gxpl.mk
include $(GXPL_ROOT)/avrio.mk

#---------------- Specific targets ----------------
install: uninstall lib
# static library
	@-install -d -m 0755 $(INSTALL_LIBDIR)/
	@echo "$(MSG_INSTALL) $(TARGET) lib and headers in $(prefix) $(DEBUG)"
	@-install -m 0755 $(TARGET_LIB_PATH).a $(INSTALL_LIBDIR)
ifeq ($(DEBUG),)
	@echo "          Release version, discard debug symbols from library..."
	@avr-strip --strip-unneeded $(INSTALL_LIBDIR)/lib$(TARGET).a
else
	@echo "          Debug version, keep debug symbols from library..."
endif

# header files
	@-install -d -m 0755 $(INSTALL_INCDIR)/
	@-install -d -m 0755 $(INSTALL_INCDIR)/gxPL
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL.h $(INSTALL_INCDIR)
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL/defs.h $(INSTALL_INCDIR)/gxPL
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL/message.h $(INSTALL_INCDIR)/gxPL
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL/util.h $(INSTALL_INCDIR)/gxPL
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL/io.h $(INSTALL_INCDIR)/gxPL
	@-install -m 0644 $(GXPL_ROOT)/include/gxPL/device.h $(INSTALL_INCDIR)/gxPL

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) lib and headers from $(prefix)"
	@-rm -f $(INSTALL_LIBDIR)/$(TARGET_LIB_PATH).a
	@-rm -f $(INSTALL_INCDIR)/gxPL.h
	@-rm -fr $(INSTALL_INCDIR)/gxPL
