#
# QuakeWorld/MVDSV qwdtools makefile for Linux
#
# GNU Make required
#
# ELF only
#

CC=gcc
RELEASE_FLAGS= -O6 -I.. -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
DEBUG_CFLAGS= -g -Wall -I..
LDFLAGS=-lm

GCC2=-malign-loops=2 -malign-jumps=2 -malign-functions=2
GCC3=-falign-loops=2 -falign-jumps=2 -falign-functions=2

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_LD=$(CC) $(CFLAGS) $(LDFLAGS) -o ../../qwdtools $^

QWDOBJS=dem_parse.o \
dem_send.o \
ini.o \
init.o \
main.o \
marge.o \
qwz.o \
sync.o \
tools.o

linux-gcc2:
	$(MAKE) qwdtools CFLAGS="$(RELEASE_FLAGS) $(GCC2)"

linux-gcc3:
	$(MAKE) qwdtools CFLAGS="$(RELEASE_FLAGS) $(GCC3)"

debug:
	$(MAKE) qwdtools CFLAGS="$(DEBUG_CFLAGS)"

qwdtools: $(QWDOBJS)
	$(DO_LD)

clean:
	rm -f *.o
