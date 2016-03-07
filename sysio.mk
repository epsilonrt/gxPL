###############################################################################
# Copyright © 2015 Pascal JEAN aka epsilonRT                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifeq ($(USE_SYSIO_LIB),ON)
EXTRA_LIBS += sysio
else
ifneq ($(SYSIO_ROOT),)
include $(SYSIO_ROOT)/sysio.mk
else
VPATH+=:$(3RDPARTY_ROOT)
EXTRA_INCDIRS += $(3RDPARTY_ROOT)/sysio
SRC += $(addprefix sysio/src/, $(notdir $(wildcard $(3RDPARTY_ROOT)/sysio/src/*.c)))
SRC += $(addprefix sysio/src/posix/, $(notdir $(wildcard $(3RDPARTY_ROOT)/sysio/src/posix/*.c)))
endif
endif

