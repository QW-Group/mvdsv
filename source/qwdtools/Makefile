#
# QuakeWorld/MVDSV qwdtools Makefile for FreeBSD
#

CC=gcc
MAINDIR=.

SOURCEDIR=${MAINDIR}

DO_CFLAGS =	${CFLAGS} -funsigned-char -I..
WITH_OPTIMIZED_CFLAGS = YES

.if defined(WITH_OPTIMIZED_CFLAGS)

DO_CFLAGS +=	-O9 -pipe -s -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
# Get __FreeBSD_version
.if !defined(OSVERSION)
.if exists(/sbin/sysctl)
OSVERSION!=     /sbin/sysctl -n kern.osreldate
.else
OSVERSION!=     /usr/sbin/sysctl -n kern.osreldate
.endif
.endif

.if ${OSVERSION} < 500035
#GCC 2
DO_CFLAGS +=	-malign-loops=2 -malign-jumps=2 -malign-functions=2
.else
DO_CFLAGS +=	-falign-loops=2 -falign-jumps=2 -falign-functions=2
#GCC 3
.endif

.endif

########################################################################

SV_OBJS = \
		dem_parse.o \
		dem_send.o \
		ini.o \
		init.o \
		main.o \
		marge.o \
		qwz.o \
		sync.o \
		tools.o


SV_LIBS = -lm

.c.o:
		${CC} ${DO_CFLAGS} -c $< -o $*.o

all:	${SV_OBJS} ${SV_AS_OBJS}
		${CC} ${CFLAGS} -o ../../qwdtools ${SV_OBJS} ${SV_AS_OBJS} ${SV_LIBS}

clean:
		-rm -f *.o *.core
