###############################################################################
#     Licensed under the Apache License, Version 2.0 (the "License")          #
###############################################################################

# Target Name (without extension).
TARGET = template

# Enable logging messages (if OFF no display is performed)
#LOG = ON

# Relative path from the root directory of the user's project
PROJECT_TOPDIR = .

# User Project
USER_PROJECT = TEMPLATE

#----------------------------------------------------------------------------
#                               ~~~~AVRIO~~~~
# Relative path of the AVRIO root directory 
AVRIO_TOPDIR = $(AVRIO_ROOT)

# Nom de la carte cible (voir le fichier avrio/boards.txt pour la liste)
# Si BOARD est vide, il faut un fichier board.mk pour définir les propriétés de
# la carte dans le répertoire où se trouve ce Makefile !
BOARD = TEMPLATE

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
# !!TODO!! Choose the desired version of device, with or without configuration.
SRC  = $(TARGET).c
#SRC  = $(TARGET)-config.c

SRC += avrio/adc.c
# all .c files in src/ directory added
SRC += $(addprefix src/, $(notdir $(wildcard $(PROJECT_TOPDIR)/src/*.c)))

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

# ON/OFF
AVRDUDE_WRITE_EEPROM_ENABLE = OFF

#------------------------------------------------------------------------------
ifeq ($(AVRIO_ROOT),)
$(error You must set the environment variable AVRIO_ROOT)
endif
ifeq ($(GXPL_ROOT),)
$(error You must set the environment variable GXPL_ROOT)
endif

include $(GXPL_ROOT)/gxpl.mk
include $(GXPL_ROOT)/avrio.mk
