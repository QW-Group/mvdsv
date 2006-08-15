#
# QuakeWorld/MVDSV and QWDTools Makefile for Linux and SunOS
#
# GNU Make required
#
# ELF only
#
#	$Id: Makefile.GNU,v 1.6 2006/08/15 12:23:19 oldmanuk Exp $
#

MAINDIR=.
SV_DIR=$(MAINDIR)/source
QWDTOOLS_DIR=$(MAINDIR)/source/qwdtools

BASE_CFLAGS=-Wall -pipe -funsigned-char -DUSE_PR2 -D$(BYTE_ORDER)Q__
WITH_OPTIMIZED_CFLAGS=YES

USE_ASM=-Did386
ifeq ($(WITH_OPTIMIZED_CFLAGS),YES)
ifneq (,$(findstring 86,$(shell uname -m)))
ifeq (,$(findstring 64,$(shell uname -m)))
ifneq ($(UNAME),Darwin)
ASM=$(USE_ASM)
endif
endif
CFLAGS=$(ASM) $(BASE_CFLAGS) -O2 -fno-strict-aliasing -s -ffast-math -funroll-loops
else
CFLAGS=$(BASE_CFLAGS) -Wsign-compare -ggdb
endif

LDFLAGS=-lm
ifeq ($(UNAME),Linux)
LDFLAGS+=-ldl
STRIP=-strip
STRIP_FLAGS=--strip-unneeded --remove-section=.comment mvdsv qwdtools
else
ifeq ($(UNAME),SunOS)
LDFLAGS+= -lsocket -lnsl
CFLAGS+= -DBSD_COMP
endif
endif

ifeq ($(UNAME),Darwin)
ifeq ($(CC_BASEVERSION),4) # if gcc4 then build universal binary
CFLAGS+= -arch ppc -arch i386
endif
endif

#############################################################################
# SERVER
#############################################################################

SV_OBJS = \
		$(SV_DIR)/pr_cmds.o \
		$(SV_DIR)/pr_edict.o \
		$(SV_DIR)/pr_exec.o \
\
		$(SV_DIR)/pr2_cmds.o \
		$(SV_DIR)/pr2_edict.o \
		$(SV_DIR)/pr2_exec.o \
		$(SV_DIR)/pr2_vm.o \
\
		$(SV_DIR)/sv_ccmds.o \
		$(SV_DIR)/sv_demo.o \
		$(SV_DIR)/sv_ents.o \
		$(SV_DIR)/sv_init.o \
		$(SV_DIR)/sv_login.o \
		$(SV_DIR)/sv_main.o \
		$(SV_DIR)/sv_master.o \
		$(SV_DIR)/sv_mod_frags.o \
		$(SV_DIR)/sv_move.o \
		$(SV_DIR)/sv_nchan.o \
		$(SV_DIR)/sv_phys.o \
		$(SV_DIR)/sv_send.o \
		$(SV_DIR)/sv_sys_unix.o \
		$(SV_DIR)/sv_user.o \
\
		$(SV_DIR)/bothtools.o \
		$(SV_DIR)/cmd.o \
		$(SV_DIR)/common.o \
		$(SV_DIR)/cmodel.o \
		$(SV_DIR)/crc.o \
		$(SV_DIR)/cvar.o \
		$(SV_DIR)/fs.o \
		$(SV_DIR)/mathlib.o \
		$(SV_DIR)/mdfour.o \
		$(SV_DIR)/net_chan.o \
		$(SV_DIR)/net.o \
		$(SV_DIR)/pmove.o \
		$(SV_DIR)/pmovetst.o \
		$(SV_DIR)/sha1.o \
		$(SV_DIR)/version.o \
		$(SV_DIR)/world.o \
		$(SV_DIR)/zone.o \
\
		$(SV_DIR)/pcre/get.o \
		$(SV_DIR)/pcre/pcre.o

ifeq ($(USE_ASM),$(ASM))
SV_ASM_OBJS = \
		$(SV_DIR)/bothtoolsa.o \
		$(SV_DIR)/math.o
endif

#############################################################################
# QWDTOOLS
#############################################################################

QWDTOOLS_OBJS = \
		$(SV_DIR)/bothtools.o \
		$(QWDTOOLS_DIR)/dem_parse.o \
		$(QWDTOOLS_DIR)/dem_send.o \
		$(QWDTOOLS_DIR)/ini.o \
		$(QWDTOOLS_DIR)/init.o \
		$(QWDTOOLS_DIR)/main.o \
		$(QWDTOOLS_DIR)/marge.o \
		$(QWDTOOLS_DIR)/qwz.o \
		$(QWDTOOLS_DIR)/sync.o \
		$(QWDTOOLS_DIR)/tools.o

ifeq ($(USE_ASM),$(ASM))
QWDTOOLS_ASM_OBJS = \
		$(SV_DIR)/bothtoolsa.o
endif

#############################################################################
# SETUP AND BUILD
#############################################################################

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

.s.o :
	$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -c $< -o $@

all : mvdsv qwdtools
	$(STRIP) $(STRIP_FLAGS)

mvdsv : $(SV_OBJS) $(SV_ASM_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mvdsv $^

qwdtools : $(QWDTOOLS_OBJS) $(QWDTOOLS_ASM_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o qwdtools $^

clean : 
	-rm -f $(SV_DIR)/core $(SV_DIR)/*.o $(SV_DIR)/pcre/*.o $(QWDTOOLS_DIR)/*.o
