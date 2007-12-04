#
# Copyright © 2007 Freescale Semiconductor, Inc
# Copyright © 1999 Paul D. Smith
#
#
# The env var $CROSS_COMPILE should be set to powerpc-unknown-linux-gnu-
#

CROSS_COMPILE=powerpc-e500mc-linux-gnu-

CC=$(CROSS_COMPILE)gcc
CC_OPTS=-m32 -Wa,-me500 -I. -g -std=gnu99
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
CC_OPTS_ASM=-D_ASM -Ibin
LD=$(CROSS_COMPILE)ld
LD_OPTS=-Wl,-m -Wl,elf32ppc -Wl,-Bstatic -nostdlib
GENASSYM=./genassym.sh
MKDIR=mkdir -p

SRCS_S := head.S exceptions.S hcall.S 
SRCS_C := sprintf.c string.c trap.c tlb.c uart.c console.c hcall_wrapper.c

#SRCS_C :=  src/uart.c \
#       src/console.c src/string.c src/sprintf.c src/emulate.c src/timers.c \
#       src/paging.c src/alloc.c src/hcalls.c

OBJS := $(SRCS_S:.S=.o) $(SRCS_C:.c=.o)
OBJS := $(OBJS:%=bin/%)

all: $(OBJS)

# compile and gen dependecy file
bin/%.o : %.c
	@$(MKDIR) `dirname $@`
	$(CC) -MD $(CC_OPTS) $(CC_OPTS_C) -c -o $@ $<

bin/%.o : %.S bin/assym.s
	@$(MKDIR) `dirname $@`
	$(CC) -MD $(CC_OPTS) $(CC_OPTS_ASM) -c -o $@ $<

bin/libos_assym.o: libos_assym.c
	@$(MKDIR) `dirname $@`
	$(CC) -MD $(CC_OPTS) -c -o $@ $<

bin/assym.s: bin/libos_assym.o
	@$(MKDIR) `dirname $@`
	$(GENASSYM) -o $@ $<

# include the dependecy files
-include bin/genassym.d
-include $(OBJS:.o=.d)

clean:
	rm -rf bin
