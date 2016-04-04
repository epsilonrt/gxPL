###############################################################################
#     Licensed under the Apache License, Version 2.0 (the "License")          #
###############################################################################
SUBDIRS = led button switch xbee rs485 i2c

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

.PHONY: all clean distclean rebuild lib cleanlib extcoff program debug elf hex eep lss sym $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

