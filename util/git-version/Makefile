###############################################################################
# (c) Copyright 2015 epsilonRT                                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

TARGET = git-version

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
	@-install -m 0755 $(CURDIR)/git-version $(INSTALL_BINDIR)

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) from $(prefix)"
	@-rm -f $(INSTALL_BINDIR)/git-version

.PHONY: all install uninstall
