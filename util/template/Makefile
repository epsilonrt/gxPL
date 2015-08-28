###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

# Chemin relatif du répertoire racine de xPL4Linux
PROJECT_ROOT = ../..

#---------------- Install Options ----------------
prefix=/usr/local

INSTALL_BINDIR=$(prefix)/bin
INSTALL_DATDIR=$(prefix)/share
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all:

install: uninstall
	@echo "$(MSG_INSTALL) $(TARGET) utils and templates in $(prefix)"
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template
	@-install -d -m 0755 $(INSTALL_DATDIR)/gxpl/template/cpp
	@-install -m 0644 $(PROJECT_ROOT)/util/template/Makefile $(INSTALL_DATDIR)/gxpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/template.c $(INSTALL_DATDIR)/gxpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/template.project $(INSTALL_DATDIR)/gxpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/cpp/* $(INSTALL_DATDIR)/gxpl/template/cpp
	@-install -m 0755 $(PROJECT_ROOT)/util/bin/gxpl-prj $(INSTALL_BINDIR)
	@-install -m 0755 $(PROJECT_ROOT)/util/bin/git-version $(INSTALL_BINDIR)
	@sed -i -e "s#INSTALLED_TEMPLATE_DIR#$(INSTALL_DATDIR)/gxpl/template#g" $(INSTALL_BINDIR)/gxpl-prj

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) utils and templates from $(prefix)"
	@-rm -fr $(INSTALL_DATDIR)/gxpl
	@-rm -fr $(INSTALL_BINDIR)/gxpl-prj
	@-rm -fr $(INSTALL_BINDIR)/git-version

.PHONY: all install uninstall
