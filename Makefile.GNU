#
# QuakeWorld/MVDSV Makefile for Linux and SunOS
#
# GNU Make required
#
# ELF only
#
#	$Id$
#

SV_DIR = ./src

# to compile mvdsv as 32bit on 64bit target platform use next:
# for gcc its like: make mvdsv FORCE32BITFLAGS=-m32
# configure script add FORCE32BITFLAGS=-m32

BASE_CFLAGS=-Wall -pipe -funsigned-char -DSERVERONLY -DUSE_PR2 -D$(BYTE_ORDER)Q__ $(FORCE32BITFLAGS)
WITH_OPTIMIZED_CFLAGS=YES

# libcurl support
ifeq ($(shell pkg-config --exists libcurl && echo 1),1)
	CURL_CFLAGS = `pkg-config libcurl --cflags`
	CURL_LIBS = `pkg-config libcurl --libs`
endif

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
ASMFLAGS=-DELF -x assembler-with-cpp
ifeq ($(UNAME),Linux)
	LDFLAGS+=-ldl
	CFLAGS+= -pthread
	STRIP=-strip
	STRIP_FLAGS=--strip-unneeded --remove-section=.comment mvdsv
else
	ifeq ($(UNAME),SunOS)
		LDFLAGS+= -lsocket -lnsl
		CFLAGS+= -pthread -DBSD_COMP
	else
		ifeq ($(UNAME),CYGWIN_NT-10.0)
			LDFLAGS += -Lmingw32-libs/lib -static -lws2_32 -lwinmm
			CFLAGS += -D_WIN32_WINNT=0x0501 -D_USE_32BIT_TIME_T -D__USE_MINGW_ANSI_STDIO -D_CONSOLE
			STRIP=-strip
			STRIP_FLAGS=--strip-unneeded --remove-section=.comment mvdsv
			ASMFLAGS = -x assembler-with-cpp
		else
			CFLAGS+= -pthread 
		endif
	endif
endif

ifdef CURL_CFLAGS
	CFLAGS += $(CURL_CFLAGS)
	CFLAGS += -DWWW_INTEGRATION
	LDFLAGS += $(CURL_LIBS)
endif

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

SV_OBJS += \
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
		$(SV_DIR)/sha3.o \
		$(SV_DIR)/build.o \
		$(SV_DIR)/world.o \
		$(SV_DIR)/zone.o \
\
		$(SV_DIR)/pcre/get.o \
		$(SV_DIR)/pcre/pcre.o \

ifdef CURL_CFLAGS
		SV_WEB_INTEGRATION = $(SV_DIR)/central.o
endif

ifeq ($(USE_ASM),$(ASM))
SV_ASM_OBJS = \
		$(SV_DIR)/bothtoolsa.o \
		$(SV_DIR)/math.o
endif

#############################################################################
# SETUP AND BUILD
#############################################################################

# Define V=1 to show command line.
ifdef V
    Q :=
    E := @true
else
    Q := @
    E := @echo
endif

.c.o :
	$(E) [CC] $@
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

.s.o :
	$(E) [ASM] $@
	$(Q)$(CC) $(CFLAGS) $(ASMFLAGS) -c $< -o $@

all : mvdsv
	$(E) [STRIP]
	$(Q)$(STRIP) $(STRIP_FLAGS)

mvdsv : $(SV_WEB_INTEGRATION) $(SV_OBJS) $(SV_ASM_OBJS)
	$(E) [LINK]
	$(Q)$(CC) $(CFLAGS) -o mvdsv $^ $(LDFLAGS)

clean : 
	$(E) [CLEAN]
	$(Q)-rm -f $(SV_DIR)/core $(SV_DIR)/*.o $(SV_DIR)/pcre/*.o mvdsv
