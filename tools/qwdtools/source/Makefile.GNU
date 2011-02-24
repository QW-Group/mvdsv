#
# QWDTools Makefile for Linux and SunOS
#
# GNU Make required
#
# ELF only
#
#	$Id$
#

MAINDIR = ../../..
SV_DIR = $(MAINDIR)/source
QWDTOOLS_DIR = $(MAINDIR)/tools/qwdtools/source

# to compile qwdtools as 32bit on 64bit target platform use next:
# for gcc its like: make qwdtools FORCE32BITFLAGS=-m32
# configure script add FORCE32BITFLAGS=-m32

BASE_CFLAGS=-Wall -pipe -pthread -funsigned-char -DUSE_PR2 -D$(BYTE_ORDER)Q__ $(FORCE32BITFLAGS)
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
STRIP_FLAGS=--strip-unneeded --remove-section=.comment qwdtools
else
ifeq ($(UNAME),SunOS)
LDFLAGS+= -lsocket -lnsl
CFLAGS+= -DBSD_COMP
endif
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

all : qwdtools
	$(STRIP) $(STRIP_FLAGS)

qwdtools : $(QWDTOOLS_OBJS) $(QWDTOOLS_ASM_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o qwdtools $^

clean : 
	-rm -f $(QWDTOOLS_DIR)/*.o qwdtools
