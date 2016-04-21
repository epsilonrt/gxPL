###############################################################################
# (c) Copyright 2015 Pascal JEAN aka epsilonRT                                #
# All rights reserved.                                                        #
# Licensed under the Apache License, Version 2.0 (the "License")              #
###############################################################################

TARGET = gxpl-xbee-bridge

#---------------- Install Options ----------------
INSTALL_BINDIR=/etc/init.d
INSTALL_DATDIR=/etc
MSG_INSTALL = [INSTALL]
MSG_UNINSTALL = [UNINSTALL]

all:
	@echo "shell script project, only targets install and uninstall are available !"

install: uninstall
	@echo "$(MSG_INSTALL) $(TARGET) in $(INSTALL_BINDIR)"
	@-install -m 0644 --backup=existing $(TARGET).conf $(INSTALL_DATDIR)
	@-install -m 0755 $(TARGET) $(INSTALL_BINDIR)

uninstall:
	@echo "$(MSG_UNINSTALL) $(TARGET) from $(INSTALL_BINDIR)"
ifeq (,$(wildcard $(INSTALL_BINDIR)/$(TARGET)))
else
	@-$(INSTALL_BINDIR)/$(TARGET) stop
endif
	@-cp -f $(INSTALL_DATDIR)/$(TARGET).conf $(INSTALL_DATDIR)/$(TARGET).conf.bak
	@-rm -f $(INSTALL_BINDIR)/$(TARGET)

.PHONY: all install uninstall
