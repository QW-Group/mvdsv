#
# MVD Makefile for Linux 2.0
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

ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
RPMARCH=alpha
else
ARCH=i386
RPMARCH=i386
endif

MAINDIR=.

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)
CLIENT_DIR=$(MAINDIR)/source
SERVER_DIR=$(MAINDIR)/source

MESA_DIR=/usr/local/src/Mesa-3.0

CC=gcc
BASE_CFLAGS=-Wall -Dstricmp=strcasecmp -D_snprintf=snprintf -I$(CLIENT_DIR) -I$(SERVER_DIR)
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
ifeq ($(ARCH),axp)
RELEASE_CFLAGS=$(BASE_CFLAGS) -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations
else
RELEASE_CFLAGS=$(BASE_CFLAGS) -m486 -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
	-malign-jumps=2 -malign-functions=2
endif

LDFLAGS=-lm
XLDFLAGS=-L/usr/X11R6/lib -lX11 -lXext

DO_CC=$(CC) -Did386 $(CFLAGS) -o $@ -c $<
DO_O_CC=$(CC) -O $(CFLAGS) -o $@ -c $<
DO_SERVER_CC=$(CC) -Did386 -DSERVERONLY $(CFLAGS) -o $@ -c $<

DO_AS=$(CC) -Did386 $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

ifeq ($(ARCH),axp)
TARGETS=$(BUILDDIR)/mvdsv 
else
TARGETS=$(BUILDDIR)/mvdsv
endif

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/server 
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/server
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

#############################################################################
# SERVER
#############################################################################

QWSV_OBJS = \
	$(BUILDDIR)/server/sv_demo.o \
	 $(BUILDDIR)/server/pr_cmds.o \
	 $(BUILDDIR)/server/pr_edict.o \
	 $(BUILDDIR)/server/pr_exec.o \
	 $(BUILDDIR)/server/sv_init.o \
	 $(BUILDDIR)/server/sv_main.o \
	 $(BUILDDIR)/server/sv_nchan.o \
	 $(BUILDDIR)/server/sv_ents.o \
	 $(BUILDDIR)/server/sv_send.o \
	 $(BUILDDIR)/server/sv_move.o \
	 $(BUILDDIR)/server/sv_phys.o \
	 $(BUILDDIR)/server/sv_user.o \
	 $(BUILDDIR)/server/sv_ccmds.o \
	 $(BUILDDIR)/server/world.o \
	 $(BUILDDIR)/server/sv_sys_unix.o \
	 $(BUILDDIR)/server/sv_model.o \
	 $(BUILDDIR)/server/cmd.o \
	 $(BUILDDIR)/server/common.o \
	 $(BUILDDIR)/server/crc.o \
	 $(BUILDDIR)/server/cvar.o \
	 $(BUILDDIR)/server/mathlib.o \
	 $(BUILDDIR)/server/mdfour.o \
	 $(BUILDDIR)/server/zone.o \
	 $(BUILDDIR)/server/pmove.o \
	 $(BUILDDIR)/server/pmovetst.o \
	 $(BUILDDIR)/server/net_chan.o \
	 $(BUILDDIR)/server/net_udp.o \
	 $(BUILDDIR)/server/version.o \
	 $(BUILDDIR)/server/worlda.o \
	 $(BUILDDIR)/server/math.o \
	 $(BUILDDIR)/server/sys_x86.o 


$(BUILDDIR)/mvdsv : $(QWSV_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QWSV_OBJS) $(LDFLAGS)

$(BUILDDIR)/server/sv_demo.o :   $(SERVER_DIR)/sv_demo.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/server/pr_cmds.o :   $(SERVER_DIR)/pr_cmds.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/server/pr_edict.o :  $(SERVER_DIR)/pr_edict.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/pr_exec.o :   $(SERVER_DIR)/pr_exec.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_init.o :   $(SERVER_DIR)/sv_init.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_main.o :   $(SERVER_DIR)/sv_main.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_nchan.o :  $(SERVER_DIR)/sv_nchan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_ents.o :   $(SERVER_DIR)/sv_ents.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_send.o :   $(SERVER_DIR)/sv_send.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_move.o :   $(SERVER_DIR)/sv_move.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_phys.o :   $(SERVER_DIR)/sv_phys.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_user.o :   $(SERVER_DIR)/sv_user.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_ccmds.o :  $(SERVER_DIR)/sv_ccmds.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/world.o :     $(SERVER_DIR)/world.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_sys_unix.o :  $(SERVER_DIR)/sv_sys_unix.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/sv_model.o :     $(SERVER_DIR)/sv_model.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/cmd.o :       $(CLIENT_DIR)/cmd.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/common.o :    $(CLIENT_DIR)/common.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/crc.o :       $(CLIENT_DIR)/crc.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/cvar.o :      $(CLIENT_DIR)/cvar.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/mathlib.o :   $(CLIENT_DIR)/mathlib.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/mdfour.o :       $(CLIENT_DIR)/mdfour.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/zone.o :      $(CLIENT_DIR)/zone.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/pmove.o :     $(CLIENT_DIR)/pmove.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/pmovetst.o :  $(CLIENT_DIR)/pmovetst.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/net_chan.o :  $(CLIENT_DIR)/net_chan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/net_udp.o :   $(CLIENT_DIR)/net_udp.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/version.o :   $(CLIENT_DIR)/version.c
	$(DO_SERVER_CC)

$(BUILDDIR)/server/worlda.o :   $(CLIENT_DIR)/worlda.s
	$(DO_AS)

$(BUILDDIR)/server/math.o :   $(CLIENT_DIR)/math.s
	$(DO_AS)

$(BUILDDIR)/server/sys_x86.o :   $(CLIENT_DIR)/sys_x86.s
	$(DO_AS)


#############################################################################
# RPM
#############################################################################

# Make RPMs.  You need to be root to make this work
RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -bb
INSTALLDIR = /usr/local/games/quake

rpm: rpm-qwsv
tar: tar-qwsv

QWCL_RPMDIR=/var/tmp/qwcl-$(VERSION)
TDFXGL_DIR=/home/zoid/3dfxgl

QWPROGS = \
	$(MAINDIR)/progs/buttons.qc \
	$(MAINDIR)/progs/client.qc \
	$(MAINDIR)/progs/combat.qc \
	$(MAINDIR)/progs/defs.qc \
	$(MAINDIR)/progs/doors.qc \
	$(MAINDIR)/progs/items.qc \
	$(MAINDIR)/progs/misc.qc \
	$(MAINDIR)/progs/models.qc \
	$(MAINDIR)/progs/plats.qc \
	$(MAINDIR)/progs/player.qc \
	$(MAINDIR)/progs/progdefs.h \
	$(MAINDIR)/progs/progs.src \
	$(MAINDIR)/progs/qwprogs.dat \
	$(MAINDIR)/progs/server.qc \
	$(MAINDIR)/progs/spectate.qc \
	$(MAINDIR)/progs/sprites.qc \
	$(MAINDIR)/progs/subs.qc \
	$(MAINDIR)/progs/triggers.qc \
	$(MAINDIR)/progs/weapons.qc \
	$(MAINDIR)/progs/world.qc

QWSV_RPMDIR=/var/tmp/qwsv-$(VERSION)

rpm-qwsv: qwsv.spec $(BUILD_RELEASE_DIR)/mvdsv $(QWPROGS)
	touch $(RPMROOT)/SOURCES/qwsv-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles-qwsv DESTDIR=$(QWSV_RPMDIR)/$(INSTALLDIR)
	cp qwsv.spec $(RPMROOT)/SPECS/qwsv.spec
	cp $(MAINDIR)/quake.gif $(RPMROOT)/SOURCES/quake.gif
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) qwsv.spec
	cp $(RPMROOT)/RPMS/$(RPMARCH)/qwsv-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm archives/.
	rm -rf $(QWSV_RPMDIR)

tar-qwsv: $(BUILD_RELEASE_DIR)/mvdsv $(QWPROGS)
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles-qwsv DESTDIR=$(QWSV_RPMDIR)/$(INSTALLDIR)
	cd $(QWSV_RPMDIR)/$(INSTALLDIR); tar czvf qwsv-$(VERSION)-$(RPMARCH)-unknown-linux2.0.tar.gz *
	mv $(QWSV_RPMDIR)/$(INSTALLDIR)/*.tar.gz archives/.
	rm -rf $(QWSV_RPMDIR)

copyfiles-qwsv:
	-mkdirhier $(DESTDIR)
	-mkdirhier $(DESTDIR)/qw
	-mkdirhier $(DESTDIR)/qw/skins
	cp $(BUILD_RELEASE_DIR)/mvdsv $(DESTDIR)/.
	strip $(DESTDIR)/mvdsv
	chmod 755 $(DESTDIR)/mvdsv
	cp $(MAINDIR)/docs/README.qwsv $(DESTDIR)/.
	chmod 644 $(DESTDIR)/README.qwsv
	cp $(QWPROGS) $(DESTDIR)/qw/.
	cd $(DESTDIR)/qw; chmod 644 *
	chmod 755 $(DESTDIR)/qw/skins
	cp $(MAINDIR)/fixskins.sh $(DESTDIR)/qw/skins/.
	chmod 755 $(DESTDIR)/qw/skins/fixskins.sh

qwsv.spec : $(MAINDIR)/qwsv.spec.sh $(BUILD_RELEASE_DIR)/mvdsv
	sh $< $(VERSION) $(RPM_RELEASE) $(INSTALLDIR) > $@

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(QWSV_OBJS)
