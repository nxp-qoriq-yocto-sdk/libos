
#
# Copyright (C) 2008 Freescale Semiconductor, Inc.
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


CROSS_COMPILE=powerpc-e500mc-linux-gnu-
LIBFDT_DIR := ../../dtc/libfdt
LIBOS_DIR := ../lib
LIBOS_INC := ../include
LIBOS_BIN := bin/libos
LIBOS_CLIENT_H := src/libos-client.h

CC=$(CROSS_COMPILE)gcc
CC_OPTS=-m32 -Wa,-me500 -Isrc -I$(LIBFDT_DIR) -I$(LIBOS_INC) -g \
        -std=gnu99  -include $(LIBOS_CLIENT_H)

CC_OPTS_C= -Wall \
  -Wundef \
  -Wstrict-prototypes \
  -Wno-trigraphs \
  -fno-strict-aliasing \
  -fno-common \
  -O2 \
  -msoft-float \
  -pipe \
  -ffixed-r2 \
  -mmultiple \
  -mno-altivec \
  -funit-at-a-time \
  -mno-string \
  -fomit-frame-pointer \
  -Wno-unused \
  -Werror
CC_OPTS_ASM=-D_ASM -Ibin/src
LD=$(CROSS_COMPILE)ld
LD_OPTS=-Wl,-m -Wl,elf32ppc -Wl,-Bstatic -nostdlib
GENASSYM=$(LIBOS_DIR)/tools/genassym.sh
MKDIR=mkdir -p

all: bin/example.uImage bin/example.map

LIBFDT_objdir := src
include $(LIBFDT_DIR)/Makefile.libfdt
CONFIG_LIBOS_QUEUE=y
CONFIG_LIBOS_NS16550=y
include $(LIBOS_DIR)/Makefile.libos

LIBOS_SRCS := $(LIBOS_STARTUP) $(LIBOS_FSL_BOOKE_TLB) $(LIBOS_EXCEPTION) \
              $(LIBOS_LIB) $(LIBOS_CONSOLE) $(libos-src-y)
SRCS := $(LIBOS_SRCS:%=libos/%) src/init.c \

OBJS := $(basename $(SRCS))
OBJS := $(OBJS:%=%.o) $(LIBFDT_OBJS:%=libfdt/%)
OBJS := $(OBJS:%=bin/%)

bin/example.uImage: bin/example.bin
	mkimage -A ppc -O linux -T kernel -C none -a 00000000 -e 00000000 -d $< $@

bin/example.bin.gz: bin/example.bin
	gzip -f $<

bin/example.bin: bin/example
	$(CROSS_COMPILE)objcopy -O binary $< $@

bin/example: $(OBJS)
	$(CC) $(LD_OPTS) -Wl,-Texample.lds -o $@ $(OBJS) -lgcc

# compile and gen dependecy file
bin/src/%.o : src/%.c
	@$(MKDIR) `dirname $@`
	$(CC) -MD $(CC_OPTS) $(CC_OPTS_C) -c -o $@ $<

bin/libfdt/%.o : $(LIBFDT_DIR)/%.c
	@$(MKDIR) `dirname $@`
	$(CC) -MD $(CC_OPTS) $(CC_OPTS_C) \
	-include src/libfdt_env.h -c -o $@ $<

# include the dependecy files
-include bin/src/genassym.d
-include $(OBJS:.o=.d)

bin/example.map: bin/example
	nm -n bin/example > $@	
 
clean:
	rm -rf bin
