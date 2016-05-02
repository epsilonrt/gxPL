###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = unix 
CLEANER_SUBDIRS = 

# Choix de l'architecture matérielle du système
ARCH = ARCH_GENERIC_LINUX
#ARCH = ARCH_ARM_RASPBERRYPI

# Enabling Debug information (ON / OFF)
#DEBUG = ON

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS) $(CLEANER_SUBDIRS)
distclean: $(SUBDIRS) $(CLEANER_SUBDIRS) 

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH) DEBUG=$(DEBUG)

$(CLEANER_SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)


.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS) $(CLEANER_SUBDIRS)

