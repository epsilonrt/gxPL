###############################################################################
# Copyright © 2015 epsilonRT                                                  #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

SUBDIRS = unix

# Architecture du système cible
#BOARD = BOARD_RASPBERRYPI
#BOARD = BOARD_NANOPI

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
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) DEBUG=$(DEBUG)

.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS)
