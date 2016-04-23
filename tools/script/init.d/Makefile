###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = hub xbee-bridge

all: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
rebuild: $(SUBDIRS)
lib: $(SUBDIRS)
cleanlib: $(SUBDIRS)
install: $(SUBDIRS)
uninstall: $(SUBDIRS)

.PHONY: all clean distclean rebuild lib cleanlib  $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

