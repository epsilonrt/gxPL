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
	@-install -d -m 0755 $(INSTALL_DATDIR)/app
	@-install -d -m 0755 $(INSTALL_DATDIR)/app/template
	@-install -d -m 0755 $(INSTALL_DATDIR)/app/template/c
	@-install -d -m 0755 $(INSTALL_DATDIR)/app/template/cpp
	@-install -m 0644 $(CURDIR)/template/c/* $(INSTALL_DATDIR)/app/template/c
	@-install -m 0644 $(CURDIR)/template/cpp/* $(INSTALL_DATDIR)/app/template/cpp
	@-install -m 0755 $(CURDIR)/app-prj $(INSTALL_BINDIR)
	@sed -i -e "s#INSTALLED_TEMPLATE_DIR#$(INSTALL_DATDIR)/app/template#g" $(INSTALL_BINDIR)/app-prj

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) from $(prefix)"
	@-rm -fr $(INSTALL_DATDIR)/app
	@-rm -f  $(INSTALL_BINDIR)/app-prj

.PHONY: all install uninstall
