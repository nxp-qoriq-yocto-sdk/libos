#
# Copyright (C) 2010 Freescale Semiconductor, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
#  NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

export src = $(realpath ../../)/
export KCONFIG_DIR := $(src)kconfig
export LIBOS := $(realpath ../)
export LIBOS_DIR := $(LIBOS)/lib
MKDIR=mkdir -p

export VERSION=0
export SUBVERSION=1
export PROJECTVERSION=$(VERSION).$(SUBVERSION)$(EXTRAVERSION)$(LOCALVERSION)

.PHONY: all
all:

# auto.conf.cmd makes sure the config is rebuilt if Kconfig is changed
include/config/auto.conf.cmd: ;
-include include/config/auto.conf.cmd

CONFIGS1 := config xconfig gconfig menuconfig oldconfig silentoldconfig randconfig defconfig allyesconfig allnoconfig
CONFIGS2 := %_defconfig
CONFIGS   = $(CONFIGS1) $(CONFIGS2)

include/config/auto.conf: myconfig
	cp -f myconfig .config
	$(MAKE) -f $(src)kconfig/Makefile Kconfig=$(PWD)/Kconfig silentoldconfig

.PHONY: FORCE
help $(CONFIGS1): FORCE
	$(MKDIR) bin/
	$(MAKE) -C $(PWD) Kconfig=$(PWD)/Kconfig -f $(KCONFIG_DIR)/Makefile $@
$(CONFIGS2): FORCE
	$(MKDIR) bin/
	$(MAKE) -C $(PWD) Kconfig=$(PWD)/Kconfig -f $(KCONFIG_DIR)/Makefile $@

non-config := $(filter-out $(CONFIGS) clean, $(MAKECMDGOALS))
ifeq ($(MAKECMDGOALS),)
	non-config := all
endif

$(non-config): include/config/auto.conf FORCE
	$(MKDIR) bin/
	$(MAKE) -f Makefile.build $@

clean:
	rm -rf bin/ include/ .config
