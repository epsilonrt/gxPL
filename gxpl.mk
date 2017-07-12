###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifeq ($(USE_GXPL_LIB),ON)
EXTRA_LIBDIRS += $(PREFIX)/lib
EXTRA_INCDIRS +=  $(PREFIX)/include
EXTRA_LIBS += gxPL

else
VPATH+=:$(GXPL_ROOT)
EXTRA_INCDIRS += $(GXPL_ROOT) $(GXPL_ROOT)/include $(GXPL_ROOT)/src

SRC += $(addprefix src/, $(notdir $(wildcard $(GXPL_ROOT)/src/*.c)))
SRC += $(addprefix src/sys/unix/, $(notdir $(wildcard $(GXPL_ROOT)/src/sys/unix/*.c)))
SRC += $(addprefix src/sys/avr/, $(notdir $(wildcard $(GXPL_ROOT)/src/sys/avr/*.c)))
SRC += $(addprefix src/sys/win32/, $(notdir $(wildcard $(GXPL_ROOT)/src/sys/win32/*.c)))

ifeq ($(BOARD),BOARD_RASPBERRYPI)
CDEFS += -DARCH_ARM
#CFLAGS += -munaligned-access
endif
endif
