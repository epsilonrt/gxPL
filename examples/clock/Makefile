###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = unix 
CLEANER_SUBDIRS = 

# Architecture du système cible
#BOARD = BOARD_RASPBERRYPI
#BOARD = BOARD_NANOPI

# Enabling Debug information (ON / OFF)
#DEBUG = ON

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS) $(CLEANER_SUBDIRS)
distclean: $(SUBDIRS) $(CLEANER_SUBDIRS) 
install: $(SUBDIRS)
uninstall: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) DEBUG=$(DEBUG)

$(CLEANER_SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)


.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS) $(CLEANER_SUBDIRS)

