###############################################################################
# Copyright © 2015 Pascal JEAN aka epsilonRT                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

VPATH+=:$(GXPL_ROOT)
EXTRA_INCDIRS += $(GXPL_ROOT) $(GXPL_ROOT)/include $(GXPL_ROOT)/src
SRC += $(addprefix src/, $(notdir $(wildcard $(GXPL_ROOT)/src/*.c)))
SRC += $(addprefix src/sys/unix/, $(notdir $(wildcard $(GXPL_ROOT)/src/sys/unix/*.c)))
SRC += $(addprefix src/sys/win32/, $(notdir $(wildcard $(GXPL_ROOT)/src/sys/win32/*.c)))
