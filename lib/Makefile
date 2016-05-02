###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

SUBDIRS = unix

# Choix de l'architecture matérielle du système
ARCH = ARCH_GENERIC_LINUX
#ARCH = ARCH_ARM_RASPBERRYPI

# Activation des informations Debug (ON/OFF)
# Si défini sur ON, aucune information de debug ne sera générée
#DEBUG = ON

#---------------- Install Options ----------------
prefix=/usr/local

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
install: $(SUBDIRS)
uninstall: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH) DEBUG=$(DEBUG)

.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS)
