#
# mvd makefile for Linux
#
# GNU Make required
#
# ELF only
#

# RPM release number
RPM_RELEASE=1

VERSION=2.30$(GLIBC)

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6* ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif

USE_ASM=-Did386

ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
RPMARCH=alpha
else
ARCH=i386
RPMARCH=i386
ASM=$(USE_ASM)
endif

MAINDIR=.

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)
SERVER_DIR=$(MAINDIR)/source

CC=gcc
BASE_CFLAGS=-Wall -I$(SERVER_DIR) -pipe
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
RELEASE_CFLAGS=$(BASE_CFLAGS) -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations

LDFLAGS=-lm

DO_SERVER_CC=$(CC) -DSERVERONLY $(CFLAGS) -o $@ -c $<

DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

GCC2=-malign-loops=2 -malign-jumps=2 -malign-functions=2
GCC3=-falign-loops=2 -falign-jumps=2 -falign-functions=2

#############################################################################
# SETUP AND BUILD
#############################################################################

nothing-specified:
	@echo ""
	@echo "Use ./build script to build mvdsv"
	@echo ""

linux-gcc2:
	$(MAKE) linux GCC="$(GCC2)"

linux-gcc3:
	$(MAKE) linux GCC="$(GCC3)"
	
linux:
	$(MAKE) build_release OPTIMIZATION_FLAGS="$(ASM) $(RELEASE_CFLAGS) -O6 $(GCC)"

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR)
	$(MAKE) build_sv BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS) $(ASM)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR)
	$(MAKE) build_sv BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(OPTIMIZATION_FLAGS)"

build_sv: mvdsv

all: build_debug build_release

#############################################################################
# SERVER
#############################################################################

QWSV_OBJS = \
	$(BUILDDIR)/sv_demo.o \
	$(BUILDDIR)/sv_login.o \
	 $(BUILDDIR)/pr_cmds.o \
	 $(BUILDDIR)/pr_edict.o \
	 $(BUILDDIR)/pr_exec.o \
	 $(BUILDDIR)/sv_init.o \
	 $(BUILDDIR)/sv_main.o \
	 $(BUILDDIR)/sv_nchan.o \
	 $(BUILDDIR)/sv_ents.o \
	 $(BUILDDIR)/sv_send.o \
	 $(BUILDDIR)/sv_move.o \
	 $(BUILDDIR)/sv_phys.o \
	 $(BUILDDIR)/sv_user.o \
	 $(BUILDDIR)/sv_ccmds.o \
	 $(BUILDDIR)/world.o \
	 $(BUILDDIR)/sv_sys_unix.o \
	 $(BUILDDIR)/sv_model.o \
	 $(BUILDDIR)/cmd.o \
	 $(BUILDDIR)/common.o \
	 $(BUILDDIR)/crc.o \
	 $(BUILDDIR)/cvar.o \
	 $(BUILDDIR)/mathlib.o \
	 $(BUILDDIR)/mdfour.o \
	 $(BUILDDIR)/zone.o \
	 $(BUILDDIR)/pmove.o \
	 $(BUILDDIR)/pmovetst.o \
	 $(BUILDDIR)/net_chan.o \
	 $(BUILDDIR)/net_udp.o \
	 $(BUILDDIR)/version.o \
	 $(BUILDDIR)/sha1.o

ifeq ($(USE_ASM),$(ASM))
QWSV_ASM_OBJS = \
	 $(BUILDDIR)/worlda.o \
	 $(BUILDDIR)/math.o \
	 $(BUILDDIR)/sys_x86.o 
endif

mvdsv : $(QWSV_OBJS) $(QWSV_ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QWSV_OBJS) $(QWSV_ASM_OBJS) $(LDFLAGS)

$(BUILDDIR)/sv_demo.o :   $(SERVER_DIR)/sv_demo.c 
	$(DO_SERVER_CC)
$(BUILDDIR)/sv_login.o :   $(SERVER_DIR)/sv_login.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_cmds.o :   $(SERVER_DIR)/pr_cmds.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_edict.o :  $(SERVER_DIR)/pr_edict.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_exec.o :   $(SERVER_DIR)/pr_exec.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_init.o :   $(SERVER_DIR)/sv_init.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_main.o :   $(SERVER_DIR)/sv_main.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_nchan.o :  $(SERVER_DIR)/sv_nchan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_ents.o :   $(SERVER_DIR)/sv_ents.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_send.o :   $(SERVER_DIR)/sv_send.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_move.o :   $(SERVER_DIR)/sv_move.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_phys.o :   $(SERVER_DIR)/sv_phys.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_user.o :   $(SERVER_DIR)/sv_user.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_ccmds.o :  $(SERVER_DIR)/sv_ccmds.c
	$(DO_SERVER_CC)

$(BUILDDIR)/world.o :     $(SERVER_DIR)/world.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_sys_unix.o :  $(SERVER_DIR)/sv_sys_unix.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_model.o :     $(SERVER_DIR)/sv_model.c
	$(DO_SERVER_CC)

$(BUILDDIR)/cmd.o :       $(SERVER_DIR)/cmd.c
	$(DO_SERVER_CC)

$(BUILDDIR)/common.o :    $(SERVER_DIR)/common.c
	$(DO_SERVER_CC)

$(BUILDDIR)/crc.o :       $(SERVER_DIR)/crc.c
	$(DO_SERVER_CC)

$(BUILDDIR)/cvar.o :      $(SERVER_DIR)/cvar.c
	$(DO_SERVER_CC)

$(BUILDDIR)/mathlib.o :   $(SERVER_DIR)/mathlib.c
	$(DO_SERVER_CC)

$(BUILDDIR)/mdfour.o :       $(SERVER_DIR)/mdfour.c
	$(DO_SERVER_CC)

$(BUILDDIR)/zone.o :      $(SERVER_DIR)/zone.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pmove.o :     $(SERVER_DIR)/pmove.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pmovetst.o :  $(SERVER_DIR)/pmovetst.c
	$(DO_SERVER_CC)

$(BUILDDIR)/net_chan.o :  $(SERVER_DIR)/net_chan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/net_udp.o :   $(SERVER_DIR)/net_udp.c
	$(DO_SERVER_CC)

$(BUILDDIR)/version.o :   $(SERVER_DIR)/version.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sha1.o :      $(SERVER_DIR)/sha1.c
	$(DO_SERVER_CC)

$(BUILDDIR)/worlda.o :   $(SERVER_DIR)/worlda.s
	$(DO_AS)

$(BUILDDIR)/math.o :   $(SERVER_DIR)/math.s
	$(DO_AS)

$(BUILDDIR)/sys_x86.o :   $(SERVER_DIR)/sys_x86.s
	$(DO_AS)
