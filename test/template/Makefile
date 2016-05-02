###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifeq ($(GXPL_ROOT),)
$(error You must set the environment variable GXPL_ROOT)
endif

#---------------- Install to gxpl-prj template directory ----------------
INSTALL_AVR_DIR=$(GXPL_ROOT)/util/gxpl-prj/template/avr
INSTALL_UNIX_DIR=$(GXPL_ROOT)/util/gxpl-prj/template/unix

all:

install: install-unix install-avr

install-avr:
	-install -d -m 0755 $(INSTALL_AVR_DIR)/

	-install -m 0644 avr/template.workspace $(INSTALL_AVR_DIR)/
	-install -m 0644 avr/template.project $(INSTALL_AVR_DIR)/
	-install -m 0644 avr/template.c $(INSTALL_AVR_DIR)/
	-install -m 0644 avr/template-config.c $(INSTALL_AVR_DIR)/
	-install -m 0644 avr/src/template.h $(INSTALL_AVR_DIR)

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/config.h $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/README $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/board.mk $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/AUTHORS $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/Makefile $(INSTALL_AVR_DIR)/files/
	-install -m 0644 avr/NOTICE $(INSTALL_AVR_DIR)/files/

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/src
	-install -m 0644 avr/src/sensor.c $(INSTALL_AVR_DIR)/files/src
	-install -m 0644 avr/src/ui.c $(INSTALL_AVR_DIR)/files/src

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test
	-install -m 0644 avr/test/Makefile $(INSTALL_AVR_DIR)/files/test

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/spi
	-install -m 0644 avr/test/spi/spi_test.project $(INSTALL_AVR_DIR)/files/test/spi
	-install -m 0644 avr/test/spi/Makefile $(INSTALL_AVR_DIR)/files/test/spi
	-install -m 0644 avr/test/spi/spi_test.c $(INSTALL_AVR_DIR)/files/test/spi

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/kernel
	-install -m 0644 avr/test/kernel/kernel_test.c $(INSTALL_AVR_DIR)/files/test/kernel
	-install -m 0644 avr/test/kernel/kernel_test.project $(INSTALL_AVR_DIR)/files/test/kernel
	-install -m 0644 avr/test/kernel/Makefile $(INSTALL_AVR_DIR)/files/test/kernel
	-install -m 0644 avr/test/kernel/kernel_test.aps $(INSTALL_AVR_DIR)/files/test/kernel

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/button_test.c $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/button_test.project $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/Makefile $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/button_test.aps $(INSTALL_AVR_DIR)/files/test/button

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230_test.project $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230.scc $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/Makefile $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230_test.c $(INSTALL_AVR_DIR)/files/test/tsl230

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/led_test.c $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/Makefile $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/led_test.project $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/led_test.aps $(INSTALL_AVR_DIR)/files/test/led

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam_test.project $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam_test.c $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/Makefile $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam_test.aps $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam.* $(INSTALL_AVR_DIR)/files/test/gifam

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/xbee_test.project $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/Makefile $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/xbee_test.c $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/xbee_test.aps $(INSTALL_AVR_DIR)/files/test/xbee

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/rc_servo_test.c $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/rc_servo_test.project $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/Makefile $(INSTALL_AVR_DIR)/files/test/rc_servo

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/Makefile $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/terminal_test.project $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/terminal_test.c $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/terminal_test.aps $(INSTALL_AVR_DIR)/files/test/terminal

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/hsc_spi
	-install -m 0644 avr/test/hsc_spi/hsc_spi_test.c $(INSTALL_AVR_DIR)/files/test/hsc_spi
	-install -m 0644 avr/test/hsc_spi/hsc_spi_test.project $(INSTALL_AVR_DIR)/files/test/hsc_spi
	-install -m 0644 avr/test/hsc_spi/Makefile $(INSTALL_AVR_DIR)/files/test/hsc_spi
	-install -m 0644 avr/test/hsc_spi/hsc.scc $(INSTALL_AVR_DIR)/files/test/hsc_spi

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/adc
	-install -m 0644 avr/test/adc/adc_test.c $(INSTALL_AVR_DIR)/files/test/adc
	-install -m 0644 avr/test/adc/adc_test.project $(INSTALL_AVR_DIR)/files/test/adc
	-install -m 0644 avr/test/adc/Makefile $(INSTALL_AVR_DIR)/files/test/adc

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/hih6130
	-install -m 0644 avr/test/hih6130/hih6130.scc $(INSTALL_AVR_DIR)/files/test/hih6130
	-install -m 0644 avr/test/hih6130/hih6130_test.project $(INSTALL_AVR_DIR)/files/test/hih6130
	-install -m 0644 avr/test/hih6130/Makefile $(INSTALL_AVR_DIR)/files/test/hih6130
	-install -m 0644 avr/test/hih6130/hih6130_test.c $(INSTALL_AVR_DIR)/files/test/hih6130
	-install -m 0644 avr/test/hih6130/hih6130_test.aps $(INSTALL_AVR_DIR)/files/test/hih6130

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/Makefile $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/lcd_test.project $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/lcd_test.c $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/lcd_test.aps $(INSTALL_AVR_DIR)/files/test/lcd

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-tsl230.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-servo.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-lcd.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-button.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-led.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-spi.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-tc.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-config.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-adc.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-kernel.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-lcdbacklight.h $(INSTALL_AVR_DIR)/files/board
	-install -m 0644 avr/board/avrio-board-twi.h $(INSTALL_AVR_DIR)/files/board

install-unix:
	-install -d -m 0755 $(INSTALL_UNIX_DIR)/

	-install -m 0644 unix/template.workspace $(INSTALL_UNIX_DIR)/
	-install -m 0644 unix/template.project $(INSTALL_UNIX_DIR)/
	-install -m 0644 unix/template.c $(INSTALL_UNIX_DIR)/
	-install -m 0644 unix/template-daemon.c $(INSTALL_UNIX_DIR)/
	-install -m 0644 unix/src/template.h $(INSTALL_UNIX_DIR)

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/
	-install -m 0644 unix/config.h $(INSTALL_UNIX_DIR)/files/
	-install -m 0644 unix/README $(INSTALL_UNIX_DIR)/files/
	-install -m 0644 unix/AUTHORS $(INSTALL_UNIX_DIR)/files/
	-install -m 0644 unix/Makefile $(INSTALL_UNIX_DIR)/files/
	-install -m 0644 unix/NOTICE $(INSTALL_UNIX_DIR)/files/

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/src
	-install -m 0644 unix/src/main.c $(INSTALL_UNIX_DIR)/files/src
	-install -m 0644 unix/src/device.c $(INSTALL_UNIX_DIR)/files/src
	-install -m 0644 unix/src/device-config.c $(INSTALL_UNIX_DIR)/files/src
	-install -m 0644 unix/src/sensor.c $(INSTALL_UNIX_DIR)/files/src
	-install -m 0644 unix/src/ui.c $(INSTALL_UNIX_DIR)/files/src

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test
	-install -m 0644 unix/test/Makefile $(INSTALL_UNIX_DIR)/files/test

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/switch
	-install -m 0644 unix/test/switch/switch_test.project $(INSTALL_UNIX_DIR)/files/test/switch
	-install -m 0644 unix/test/switch/Makefile $(INSTALL_UNIX_DIR)/files/test/switch
	-install -m 0644 unix/test/switch/switch_test.c $(INSTALL_UNIX_DIR)/files/test/switch

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/led
	-install -m 0644 unix/test/led/led_test.c $(INSTALL_UNIX_DIR)/files/test/led
	-install -m 0644 unix/test/led/Makefile $(INSTALL_UNIX_DIR)/files/test/led
	-install -m 0644 unix/test/led/led_test.project $(INSTALL_UNIX_DIR)/files/test/led

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/button
	-install -m 0644 unix/test/button/button_test.c $(INSTALL_UNIX_DIR)/files/test/button
	-install -m 0644 unix/test/button/button_test.project $(INSTALL_UNIX_DIR)/files/test/button
	-install -m 0644 unix/test/button/Makefile $(INSTALL_UNIX_DIR)/files/test/button

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/iaq
	-install -m 0644 unix/test/iaq/iaq_test.c $(INSTALL_UNIX_DIR)/files/test/iaq
	-install -m 0644 unix/test/iaq/iaq_test.project $(INSTALL_UNIX_DIR)/files/test/iaq
	-install -m 0644 unix/test/iaq/Makefile $(INSTALL_UNIX_DIR)/files/test/iaq
	
	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/eeprom
	-install -m 0644 unix/test/eeprom/eeprom_test.c $(INSTALL_UNIX_DIR)/files/test/eeprom
	-install -m 0644 unix/test/eeprom/eeprom_test.project $(INSTALL_UNIX_DIR)/files/test/eeprom
	-install -m 0644 unix/test/eeprom/Makefile $(INSTALL_UNIX_DIR)/files/test/eeprom

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/xbee
	-install -m 0644 unix/test/xbee/Makefile $(INSTALL_UNIX_DIR)/files/test/xbee
  
	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/xbee/node
	-install -m 0644 unix/test/xbee/node/xbee_node_test.project $(INSTALL_UNIX_DIR)/files/test/xbee/node
	-install -m 0644 unix/test/xbee/node/Makefile $(INSTALL_UNIX_DIR)/files/test/xbee/node
	-install -m 0644 unix/test/xbee/node/xbee_node_test.c $(INSTALL_UNIX_DIR)/files/test/xbee/node

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/xbee/coordinator
	-install -m 0644 unix/test/xbee/coordinator/xbee_coordinator_test.project $(INSTALL_UNIX_DIR)/files/test/xbee/coordinator
	-install -m 0644 unix/test/xbee/coordinator/Makefile $(INSTALL_UNIX_DIR)/files/test/xbee/coordinator
	-install -m 0644 unix/test/xbee/coordinator/xbee_coordinator_test.c $(INSTALL_UNIX_DIR)/files/test/xbee/coordinator

	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/rs485
	-install -m 0644 unix/test/rs485/rs485_test.c $(INSTALL_UNIX_DIR)/files/test/rs485
	-install -m 0644 unix/test/rs485/rs485_test.project $(INSTALL_UNIX_DIR)/files/test/rs485
	-install -m 0644 unix/test/rs485/Makefile $(INSTALL_UNIX_DIR)/files/test/rs485
  
	-install -d -m 0755 $(INSTALL_UNIX_DIR)/files/test/serial
	-install -m 0644 unix/test/serial/serial_test.c $(INSTALL_UNIX_DIR)/files/test/serial
	-install -m 0644 unix/test/serial/serial_test.project $(INSTALL_UNIX_DIR)/files/test/serial
	-install -m 0644 unix/test/serial/Makefile $(INSTALL_UNIX_DIR)/files/test/serial

.PHONY: install-avr install-unix install
