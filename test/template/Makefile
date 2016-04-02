###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
ifeq ($(GXPL_ROOT),)
$(error You must set the environment variable GXPL_ROOT)
endif

#---------------- Install to gxpl-prj template directory ----------------
INSTALL_AVR_DIR=$(GXPL_ROOT)/util/gxpl-prj/template/avr

all:

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

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/button_test.c $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/button_test.project $(INSTALL_AVR_DIR)/files/test/button
	-install -m 0644 avr/test/button/Makefile $(INSTALL_AVR_DIR)/files/test/button

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230_test.project $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230.scc $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/Makefile $(INSTALL_AVR_DIR)/files/test/tsl230
	-install -m 0644 avr/test/tsl230/tsl230_test.c $(INSTALL_AVR_DIR)/files/test/tsl230

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/led_test.c $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/Makefile $(INSTALL_AVR_DIR)/files/test/led
	-install -m 0644 avr/test/led/led_test.project $(INSTALL_AVR_DIR)/files/test/led

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam_test.project $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/gifam_test.c $(INSTALL_AVR_DIR)/files/test/gifam
	-install -m 0644 avr/test/gifam/Makefile $(INSTALL_AVR_DIR)/files/test/gifam

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/xbee_test.project $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/Makefile $(INSTALL_AVR_DIR)/files/test/xbee
	-install -m 0644 avr/test/xbee/xbee_test.c $(INSTALL_AVR_DIR)/files/test/xbee

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/rc_servo_test.c $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/rc_servo_test.project $(INSTALL_AVR_DIR)/files/test/rc_servo
	-install -m 0644 avr/test/rc_servo/Makefile $(INSTALL_AVR_DIR)/files/test/rc_servo

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/Makefile $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/terminal_test.project $(INSTALL_AVR_DIR)/files/test/terminal
	-install -m 0644 avr/test/terminal/terminal_test.c $(INSTALL_AVR_DIR)/files/test/terminal

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

	-install -d -m 0755 $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/Makefile $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/lcd_test.project $(INSTALL_AVR_DIR)/files/test/lcd
	-install -m 0644 avr/test/lcd/lcd_test.c $(INSTALL_AVR_DIR)/files/test/lcd

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

.PHONY: install-avr
