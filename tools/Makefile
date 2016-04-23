###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = logger sender hub bridge

#---------------- Install Options ----------------
prefix=/usr/local

INSTALL_BINDIR=$(prefix)/bin
INSTALL_DATDIR=$(prefix)/share
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
rebuild: $(SUBDIRS)
lib: $(SUBDIRS)
cleanlib: $(SUBDIRS)
install: $(SUBDIRS) install_script
uninstall: uninstall_script $(SUBDIRS)

install_script: 
	$(MAKE) -w -C script $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH)

uninstall_script:
	$(MAKE) -w -C script $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH)

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS)

.PHONY: all clean distclean rebuild lib cleanlib install_script uninstall_script $(SUBDIRS)


