###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

TARGET = gxpl-prj

#---------------- Install Options ----------------
prefix=/usr/local

INSTALL_BINDIR=$(prefix)/bin
INSTALL_DATDIR=$(prefix)/share
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all:
	@echo "shell script project, only targets install and uninstall are available !"

install: uninstall
	@echo "$(MSG_INSTALL) $(TARGET) in $(prefix)"
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template/c
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template/cpp
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template/avr
	@-install -m 0644 $(CURDIR)/template/c/* $(INSTALL_DATDIR)/gxpl/template/c
	@-install -m 0644 $(CURDIR)/template/cpp/* $(INSTALL_DATDIR)/gxpl/template/cpp
	@-cp -a  $(CURDIR)/template/avr/* $(INSTALL_DATDIR)/gxpl/template/avr
	@-install -m 0755 $(CURDIR)/gxpl-prj $(INSTALL_BINDIR)
	@sed -i -e "s#INSTALLED_TEMPLATE_DIR#$(INSTALL_DATDIR)/gxpl/template#g" $(INSTALL_BINDIR)/gxpl-prj

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) from $(prefix)"
	@-rm -fr $(INSTALL_DATDIR)/gxpl
	@-rm -f  $(INSTALL_BINDIR)/gxpl-prj

.PHONY: all install uninstall
