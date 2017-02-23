#
# QuakeWorld/MVDSV Makefile for Linux and SunOS
#
# GNU Make required
#
# ELF only
#
#	$Id$
#

SV_DIR = ../../src

# to compile mvdsv as 32bit on 64bit target platform use next:
# for gcc its like: make mvdsv FORCE32BITFLAGS=-m32
# configure script add FORCE32BITFLAGS=-m32

BASE_CFLAGS=-Wall -pipe -pthread -funsigned-char -DSERVERONLY -DUSE_PR2 -D$(BYTE_ORDER)Q__ $(FORCE32BITFLAGS)
WITH_OPTIMIZED_CFLAGS=YES

USE_ASM=-Did386
ifeq ($(WITH_OPTIMIZED_CFLAGS),YES)
ifeq ($(ARCH),x86)
ifneq ($(UNAME),Darwin)
ifneq ($(UNAME),MacOSX)
ASM=$(USE_ASM)
endif
endif
endif
CFLAGS=$(ASM) $(BASE_CFLAGS) -O2 -fno-strict-aliasing -ffast-math -funroll-loops
else
CFLAGS=$(BASE_CFLAGS) -Wsign-compare -ggdb
endif

LDFLAGS=-lm
ifeq ($(UNAME),Linux)
LDFLAGS+=-ldl
STRIP=-strip
STRIP_FLAGS=--strip-unneeded --remove-section=.comment mvdsv
else
ifeq ($(UNAME),SunOS)
LDFLAGS+= -lsocket -lnsl
CFLAGS+= -DBSD_COMP
endif
endif

CURL_CFLAGS ?= $(shell pkg-config libcurl --cflags)
CURL_LIBS ?= $(shell pkg-config libcurl --libs)
CFLAGS += $(CURL_CFLAGS)
LDFLAGS += $(CURL_LIBS)

ifeq ($(CC_BASEVERSION),4) # if gcc4 then build universal binary
ifeq ($(UNAME),Darwin)
CFLAGS+= -arch ppc -arch i386
endif
ifeq ($(UNAME),MacOSX)
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
		$(SV_DIR)/sv_demo_misc.o \
		$(SV_DIR)/sv_demo_qtv.o \
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
		$(SV_DIR)/sv_save.o \
\
		$(SV_DIR)/bothtools.o \
		$(SV_DIR)/cmd.o \
		$(SV_DIR)/common.o \
		$(SV_DIR)/cmodel.o \
		$(SV_DIR)/crc.o \
		$(SV_DIR)/cvar.o \
		$(SV_DIR)/hash.o \
		$(SV_DIR)/fs.o \
		$(SV_DIR)/vfs_os.o \
		$(SV_DIR)/vfs_pak.o \
		$(SV_DIR)/mathlib.o \
		$(SV_DIR)/md4.o \
		$(SV_DIR)/net_chan.o \
		$(SV_DIR)/net.o \
		$(SV_DIR)/pmove.o \
		$(SV_DIR)/pmovetst.o \
		$(SV_DIR)/sha1.o \
		$(SV_DIR)/build.o \
		$(SV_DIR)/world.o \
		$(SV_DIR)/zone.o \
\
		$(SV_DIR)/pcre/get.o \
		$(SV_DIR)/pcre/pcre.o \
\
		$(SV_DIR)/central.o

ifeq ($(USE_ASM),$(ASM))
SV_ASM_OBJS = \
		$(SV_DIR)/bothtoolsa.o \
		$(SV_DIR)/math.o
endif

#############################################################################
# SETUP AND BUILD
#############################################################################

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

.s.o :
	$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -c $< -o $@

all : mvdsv
	$(STRIP) $(STRIP_FLAGS)

mvdsv : $(SV_OBJS) $(SV_ASM_OBJS)
	$(CC) $(CFLAGS) -o mvdsv $^ $(LDFLAGS)

clean : 
	-rm -f $(SV_DIR)/core $(SV_DIR)/*.o $(SV_DIR)/pcre/*.o mvdsv
