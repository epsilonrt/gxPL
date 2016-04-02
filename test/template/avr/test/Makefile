###############################################################################
#     Licensed under the Apache License, Version 2.0 (the "License")          #
###############################################################################

SUBDIRS  = adc  button gifam hih6130 hsc_spi kernel
SUBDIRS += lcd led rc_servo spi terminal tsl230 xbee

all: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
rebuild: $(SUBDIRS)
lib: $(SUBDIRS)
cleanlib: $(SUBDIRS)
extcoff: $(SUBDIRS)
program: $(SUBDIRS)
debug: $(SUBDIRS)

elf: $(SUBDIRS)
hex: $(SUBDIRS)
eep: $(SUBDIRS)
lss: $(SUBDIRS)
sym: $(SUBDIRS)
har: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

.PHONY: all clean distclean rebuild lib cleanlib extcoff program debug elf hex eep lss sym har $(SUBDIRS)
