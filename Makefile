###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################
SUBDIRS = lib

# Chemin relatif du répertoire racine de xPL4Linux
PROJECT_ROOT = .

# Choix de l'architecture matérielle du système
#ARCH = ARCH_GENERIC_LINUX
ARCH = ARCH_ARM_RASPBERRYPI

# Enabling Debug information (ON / OFF)
#DEBUG = ON

#---------------- Install Options ----------------
prefix=/usr/local

INSTALL_BINDIR=$(prefix)/bin
INSTALL_DATDIR=$(prefix)/share
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all: $(SUBDIRS)
rebuild: $(SUBDIRS)
clean: $(SUBDIRS)
distclean: $(SUBDIRS)
install: install_utils $(SUBDIRS)
uninstall: uninstall_utils $(SUBDIRS)

install_utils: uninstall_utils
	@echo "$(MSG_INSTALL) $(TARGET) utils and templates in $(prefix)"
	@-install -d -m 0755 $(INSTALL_DATDIR)/xpl
	@-install -d -m 0755 $(INSTALL_DATDIR)/xpl/template
	@-install -d -m 0755 $(INSTALL_DATDIR)/xpl/template/cpp
	@-install -m 0644 $(PROJECT_ROOT)/util/template/Makefile $(INSTALL_DATDIR)/xpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/template.c $(INSTALL_DATDIR)/xpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/template.project $(INSTALL_DATDIR)/xpl/template
	@-install -m 0644 $(PROJECT_ROOT)/util/template/cpp/* $(INSTALL_DATDIR)/xpl/template/cpp
	@-install -m 0755 $(PROJECT_ROOT)/util/bin/xpl-prj $(INSTALL_BINDIR)
	@-install -m 0755 $(PROJECT_ROOT)/util/bin/git-version $(INSTALL_BINDIR)
	@sed -i -e "s#INSTALLED_TEMPLATE_DIR#$(INSTALL_DATDIR)/xpl/template#g" $(INSTALL_BINDIR)/xpl-prj

uninstall_utils:
	@echo "$(MSG_UNINSTALL) $(TARGET) utils and templates from $(prefix)"
	@-rm -fr $(INSTALL_DATDIR)/xpl
	@-rm -fr $(INSTALL_BINDIR)/xpl-prj
	@-rm -fr $(INSTALL_BINDIR)/git-version

$(SUBDIRS):
	$(MAKE) -w -C $@ $(MAKECMDGOALS) prefix=$(prefix) ARCH=$(ARCH) DEBUG=$(DEBUG)

.PHONY: all rebuild clean distclean install uninstall $(SUBDIRS)
