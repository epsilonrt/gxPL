###############################################################################
# Copyright © 2015 Pascal JEAN aka epsilonRT                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifneq ($(AVRIO_TOPDIR),)
AVRIO_CONFIG += AVRIO_TIME_UINT32
AVRIO_CONFIG += AVRIO_TASK_ENABLE
AVRIO_CONFIG += AVRIO_TC_ENABLE
AVRIO_CONFIG += AVRIO_TC_FLAVOUR=TC_FLAVOUR_IRQ

SRC  += avrio/xbee.c avrio/xbee1.c avrio/xbee2.c  
SRC  += avrio/file.c avrio/dpin.c avrio/tc.c avrio/tc_irq.c avrio/queue.c
SRC  += avrio/dlist.c avrio/vector.c avrio/eeprom.c avrio/eefile.c avrio/crc.c
SRC  += avrio/task.c avrio/assert.c
ASRC += avrio/mutex.S avrio/util_swapbytes.S
include $(AVRIO_TOPDIR)/script/common.mk
endif

