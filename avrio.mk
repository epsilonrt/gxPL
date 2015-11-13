###############################################################################
# Copyright © 2015 Pascal JEAN aka epsilonRT                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifneq ($(AVRIO_TOPDIR),)
AVRIO_CONFIG += AVRIO_SERIAL_FLAVOUR=SERIAL_FLAVOUR_IRQ
AVRIO_CONFIG += AVRIO_SERIAL_RTSCTS
AVRIO_CONFIG += AVRIO_TIME_UINT32
AVRIO_CONFIG += AVRIO_TASK_ENABLE

SRC  += avrio/xbee.c avrio/xbee1.c avrio/xbee2.c  
SRC  += avrio/serial.c avrio/queue.c avrio/assert.c
SRC  += avrio/dlist.c avrio/vector.c avrio/eeprom.c avrio/eefile.c avrio/crc.c
SRC  += avrio/task.c
ASRC += avrio/mutex.S avrio/util_swapbytes.S
include $(AVRIO_TOPDIR)/script/common.mk
endif


