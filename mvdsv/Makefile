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
GLCFLAGS=-DGLQUAKE -I/usr/local/src/Mesa-3.0/include -I/usr/include/glide

LDFLAGS=-lm
SVGALDFLAGS=-lvga
XLDFLAGS=-L/usr/X11R6/lib -lX11 -lXext

GL_SVGA_LDFLAGS=-L/usr/X11/lib -L/usr/X11R6/lib -L/usr/local/src/Mesa-3.0/lib -lm -lGL -lglide3 -lX11 -lXext -lvga
GL_X11_LDFLAGS=-L/usr/X11R6/lib -L/usr/local/src/Mesa-3.0/lib -lm -lGL -lX11 -lXext


DO_CC=$(CC) -Did386 $(CFLAGS) -o $@ -c $<
DO_O_CC=$(CC) -O $(CFLAGS) -o $@ -c $<
DO_GL_CC=$(CC) $(CFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_SERVER_CC=$(CC) -Did386 -DSERVERONLY $(CFLAGS) -o $@ -c $<

DO_AS=$(CC) -Did386 $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_GL_AS=$(CC) $(CFLAGS) $(GLCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

ifeq ($(ARCH),axp)
TARGETS=$(BUILDDIR)/mvdsv 
else
TARGETS=$(BUILDDIR)/mvdsv $(BUILDDIR)/qwplayer $(BUILDDIR)/qwplayer.x11  $(BUILDDIR)/qwplayer-gl.x11
endif

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/client \
		$(BUILD_DEBUG_DIR)/glclient \
		$(BUILD_DEBUG_DIR)/server 
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/client \
		$(BUILD_RELEASE_DIR)/glclient \
		$(BUILD_RELEASE_DIR)/server
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

build_gl:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/glclient
	$(MAKE) tar_gl BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

build_sv:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/server
	$(MAKE) tar_sv BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)
tar_gl:  $(BUILDDIR)/qwplayer-gl.x11
tar_sv:  $(BUILDDIR)/mvdsv

#############################################################################
# SERVER
#############################################################################

QWSV_OBJS = \
	$(BUILDDIR)/server/sv_demo.o \
	$(BUILDDIR)/server/sv_login.o \
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
$(BUILDDIR)/server/sv_login.o :   $(SERVER_DIR)/sv_login.c 
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
# CLIENT
#############################################################################

QWCL_OBJS = \
	$(BUILDDIR)/client/cl_demo.o \
	$(BUILDDIR)/client/cl_ents.o \
	$(BUILDDIR)/client/cl_cmd.o \
	$(BUILDDIR)/client/cl_input.o \
	$(BUILDDIR)/client/cl_main.o \
	$(BUILDDIR)/client/cl_parse.o \
	$(BUILDDIR)/client/cl_pred.o \
	$(BUILDDIR)/client/cl_tent.o \
	$(BUILDDIR)/client/cl_cam.o \
	$(BUILDDIR)/client/cmd.o \
	$(BUILDDIR)/client/common.o \
	$(BUILDDIR)/client/console.o \
	$(BUILDDIR)/client/crc.o \
	$(BUILDDIR)/client/cvar.o \
	$(BUILDDIR)/client/d_edge.o \
	$(BUILDDIR)/client/d_fill.o \
	$(BUILDDIR)/client/d_init.o \
	$(BUILDDIR)/client/d_modech.o \
	$(BUILDDIR)/client/d_part.o \
	$(BUILDDIR)/client/d_polyse.o \
	$(BUILDDIR)/client/d_scan.o \
	$(BUILDDIR)/client/d_sky.o \
	$(BUILDDIR)/client/d_sprite.o \
	$(BUILDDIR)/client/d_surf.o \
	$(BUILDDIR)/client/d_vars.o \
	$(BUILDDIR)/client/d_zpoint.o \
	$(BUILDDIR)/client/draw.o \
	$(BUILDDIR)/client/keys.o \
	$(BUILDDIR)/client/mathlib.o \
	$(BUILDDIR)/client/mdfour.o \
	$(BUILDDIR)/client/menu.o \
	$(BUILDDIR)/client/model.o \
	$(BUILDDIR)/client/net_chan.o \
	$(BUILDDIR)/client/net_udp.o \
	$(BUILDDIR)/client/nonintel.o \
	$(BUILDDIR)/client/pmove.o \
	$(BUILDDIR)/client/pmovetst.o \
	$(BUILDDIR)/client/r_aclip.o \
	$(BUILDDIR)/client/r_alias.o \
	$(BUILDDIR)/client/r_bsp.o \
	$(BUILDDIR)/client/r_draw.o \
	$(BUILDDIR)/client/r_edge.o \
	$(BUILDDIR)/client/r_efrag.o \
	$(BUILDDIR)/client/r_light.o \
	$(BUILDDIR)/client/r_main.o \
	$(BUILDDIR)/client/r_misc.o \
	$(BUILDDIR)/client/r_part.o \
	$(BUILDDIR)/client/r_sky.o \
	$(BUILDDIR)/client/r_sprite.o \
	$(BUILDDIR)/client/r_surf.o \
	$(BUILDDIR)/client/r_vars.o \
	$(BUILDDIR)/client/sbar.o \
	$(BUILDDIR)/client/screen.o \
	$(BUILDDIR)/client/skin.o \
	$(BUILDDIR)/client/snd_dma.o \
	$(BUILDDIR)/client/snd_mem.o \
	$(BUILDDIR)/client/snd_mix.o \
	$(BUILDDIR)/client/view.o \
	$(BUILDDIR)/client/wad.o \
	$(BUILDDIR)/client/zone.o \
	$(BUILDDIR)/client/cd_linux.o \
	$(BUILDDIR)/client/sys_linux.o \
	$(BUILDDIR)/client/snd_linux.o \
	$(BUILDDIR)/client/version.o \
	$(BUILDDIR)/client/teamplay.o \
	$(BUILDDIR)/client/cl_slist.o \

ifeq ($(ARCH),i386)
	QWCL_AS_OBJS = \
	$(BUILDDIR)/client/cl_math.o \
	$(BUILDDIR)/client/d_copy.o \
	$(BUILDDIR)/client/d_draw.o \
	$(BUILDDIR)/client/d_draw16.o \
	$(BUILDDIR)/client/d_parta.o \
	$(BUILDDIR)/client/d_polysa.o \
	$(BUILDDIR)/client/d_scana.o \
	$(BUILDDIR)/client/d_spr8.o \
	$(BUILDDIR)/client/d_varsa.o \
	$(BUILDDIR)/client/math.o \
	$(BUILDDIR)/client/r_aclipa.o \
	$(BUILDDIR)/client/r_aliasa.o \
	$(BUILDDIR)/client/r_drawa.o \
	$(BUILDDIR)/client/r_edgea.o \
	$(BUILDDIR)/client/r_varsa.o \
	$(BUILDDIR)/client/snd_mixa.o \
	$(BUILDDIR)/client/surf16.o \
	$(BUILDDIR)/client/surf8.o \
	$(BUILDDIR)/client/sys_x86.o
else
	QWCL_AS_OBJS=
endif

QWCL_SVGA_OBJS = $(BUILDDIR)/client/vid_svgalib.o
QWCL_X11_OBJS = $(BUILDDIR)/client/vid_x.o

$(BUILDDIR)/qwplayer : $(QWCL_OBJS) $(QWCL_AS_OBJS) $(QWCL_SVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QWCL_OBJS) $(QWCL_AS_OBJS) $(QWCL_SVGA_OBJS) \
		$(LDFLAGS) $(SVGALDFLAGS)

$(BUILDDIR)/qwplayer.x11 : $(QWCL_OBJS) $(QWCL_AS_OBJS) $(QWCL_X11_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QWCL_OBJS) $(QWCL_AS_OBJS) $(QWCL_X11_OBJS) \
		$(LDFLAGS) $(XLDFLAGS)

$(BUILDDIR)/client/cl_demo.o :        $(CLIENT_DIR)/cl_demo.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_ents.o :        $(CLIENT_DIR)/cl_ents.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_input.o :       $(CLIENT_DIR)/cl_input.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_main.o :        $(CLIENT_DIR)/cl_main.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_parse.o :       $(CLIENT_DIR)/cl_parse.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_pred.o :        $(CLIENT_DIR)/cl_pred.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_tent.o :        $(CLIENT_DIR)/cl_tent.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_cam.o :         $(CLIENT_DIR)/cl_cam.c
	$(DO_CC)

$(BUILDDIR)/client/cl_cmd.o :        $(CLIENT_DIR)/cl_cmd.c
	$(DO_CC)

$(BUILDDIR)/client/cmd.o :            $(CLIENT_DIR)/cmd.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/common.o :         $(CLIENT_DIR)/common.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/console.o :        $(CLIENT_DIR)/console.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/crc.o :            $(CLIENT_DIR)/crc.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cvar.o :           $(CLIENT_DIR)/cvar.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_edge.o :         $(CLIENT_DIR)/d_edge.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_fill.o :         $(CLIENT_DIR)/d_fill.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_init.o :         $(CLIENT_DIR)/d_init.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_modech.o :       $(CLIENT_DIR)/d_modech.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_part.o :         $(CLIENT_DIR)/d_part.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_polyse.o :       $(CLIENT_DIR)/d_polyse.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_scan.o :         $(CLIENT_DIR)/d_scan.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_sky.o :          $(CLIENT_DIR)/d_sky.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_sprite.o :       $(CLIENT_DIR)/d_sprite.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_surf.o :         $(CLIENT_DIR)/d_surf.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_vars.o :         $(CLIENT_DIR)/d_vars.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_zpoint.o :       $(CLIENT_DIR)/d_zpoint.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/draw.o :           $(CLIENT_DIR)/draw.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/keys.o :           $(CLIENT_DIR)/keys.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/mathlib.o :        $(CLIENT_DIR)/mathlib.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/mdfour.o :            $(CLIENT_DIR)/mdfour.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/menu.o :           $(CLIENT_DIR)/menu.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/model.o :          $(CLIENT_DIR)/model.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/net_chan.o :       $(CLIENT_DIR)/net_chan.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/net_udp.o :        $(CLIENT_DIR)/net_udp.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/nonintel.o :       $(CLIENT_DIR)/nonintel.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/pmove.o :          $(CLIENT_DIR)/pmove.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/pmovetst.o :       $(CLIENT_DIR)/pmovetst.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_aclip.o :        $(CLIENT_DIR)/r_aclip.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_alias.o :        $(CLIENT_DIR)/r_alias.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_bsp.o :          $(CLIENT_DIR)/r_bsp.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_draw.o :         $(CLIENT_DIR)/r_draw.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_edge.o :         $(CLIENT_DIR)/r_edge.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_efrag.o :        $(CLIENT_DIR)/r_efrag.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_light.o :        $(CLIENT_DIR)/r_light.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_main.o :         $(CLIENT_DIR)/r_main.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_misc.o :         $(CLIENT_DIR)/r_misc.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_part.o :         $(CLIENT_DIR)/r_part.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_sky.o :          $(CLIENT_DIR)/r_sky.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_sprite.o :       $(CLIENT_DIR)/r_sprite.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_surf.o :         $(CLIENT_DIR)/r_surf.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/r_vars.o :         $(CLIENT_DIR)/r_vars.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/sbar.o :           $(CLIENT_DIR)/sbar.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/screen.o :         $(CLIENT_DIR)/screen.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/skin.o :           $(CLIENT_DIR)/skin.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/snd_dma.o :        $(CLIENT_DIR)/snd_dma.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/snd_mem.o :        $(CLIENT_DIR)/snd_mem.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/snd_mix.o :        $(CLIENT_DIR)/snd_mix.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/view.o :           $(CLIENT_DIR)/view.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/wad.o :            $(CLIENT_DIR)/wad.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/zone.o :           $(CLIENT_DIR)/zone.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cd_linux.o :       $(CLIENT_DIR)/cd_linux.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/sys_linux.o :      $(CLIENT_DIR)/sys_linux.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/snd_linux.o :      $(CLIENT_DIR)/snd_linux.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/version.o :      $(CLIENT_DIR)/version.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/teamplay.o :      $(CLIENT_DIR)/teamplay.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/cl_slist.o :      $(CLIENT_DIR)/cl_slist.c
	$(DO_CC)
                                                                      
$(BUILDDIR)/client/d_copy.o :         $(CLIENT_DIR)/d_copy.s
	$(DO_AS)

$(BUILDDIR)/client/d_draw.o :         $(CLIENT_DIR)/d_draw.s
	$(DO_AS)

$(BUILDDIR)/client/d_draw16.o :       $(CLIENT_DIR)/d_draw16.s
	$(DO_AS)

$(BUILDDIR)/client/d_parta.o :        $(CLIENT_DIR)/d_parta.s
	$(DO_AS)

$(BUILDDIR)/client/d_polysa.o :       $(CLIENT_DIR)/d_polysa.s
	$(DO_AS)

$(BUILDDIR)/client/d_scana.o :        $(CLIENT_DIR)/d_scana.s
	$(DO_AS)

$(BUILDDIR)/client/d_spr8.o :         $(CLIENT_DIR)/d_spr8.s
	$(DO_AS)

$(BUILDDIR)/client/d_varsa.o :        $(CLIENT_DIR)/d_varsa.s
	$(DO_AS)

$(BUILDDIR)/client/cl_math.o :        $(CLIENT_DIR)/cl_math.s
	$(DO_AS)

$(BUILDDIR)/client/math.o :           $(CLIENT_DIR)/math.s
	$(DO_AS)

$(BUILDDIR)/client/r_aclipa.o :       $(CLIENT_DIR)/r_aclipa.s
	$(DO_AS)

$(BUILDDIR)/client/r_aliasa.o :       $(CLIENT_DIR)/r_aliasa.s
	$(DO_AS)

$(BUILDDIR)/client/r_drawa.o :        $(CLIENT_DIR)/r_drawa.s
	$(DO_AS)

$(BUILDDIR)/client/r_edgea.o :        $(CLIENT_DIR)/r_edgea.s
	$(DO_AS)

$(BUILDDIR)/client/r_varsa.o :        $(CLIENT_DIR)/r_varsa.s
	$(DO_AS)

$(BUILDDIR)/client/snd_mixa.o :       $(CLIENT_DIR)/snd_mixa.s
	$(DO_AS)

$(BUILDDIR)/client/surf16.o :         $(CLIENT_DIR)/surf16.s
	$(DO_AS)

$(BUILDDIR)/client/surf8.o :          $(CLIENT_DIR)/surf8.s
	$(DO_AS)

$(BUILDDIR)/client/sys_x86.o :       $(CLIENT_DIR)/sys_x86.s
	$(DO_AS)

$(BUILDDIR)/client/vid_svgalib.o : $(CLIENT_DIR)/vid_svgalib.c
	$(DO_O_CC)

$(BUILDDIR)/client/vid_x.o : $(CLIENT_DIR)/vid_x.c
	$(DO_CC)

#############################################################################
# GL CLIENT
#############################################################################

GLQWCL_OBJS = \
	$(BUILDDIR)/glclient/cl_demo.o \
	$(BUILDDIR)/glclient/cl_ents.o \
	$(BUILDDIR)/glclient/cl_cmd.o \
	$(BUILDDIR)/glclient/cl_input.o \
	$(BUILDDIR)/glclient/cl_main.o \
	$(BUILDDIR)/glclient/cl_parse.o \
	$(BUILDDIR)/glclient/cl_pred.o \
	$(BUILDDIR)/glclient/cl_tent.o \
	$(BUILDDIR)/glclient/cl_cam.o \
	$(BUILDDIR)/glclient/cmd.o \
	$(BUILDDIR)/glclient/common.o \
	$(BUILDDIR)/glclient/console.o \
	$(BUILDDIR)/glclient/crc.o \
	$(BUILDDIR)/glclient/cvar.o \
	$(BUILDDIR)/glclient/keys.o \
	$(BUILDDIR)/glclient/mathlib.o \
	$(BUILDDIR)/glclient/mdfour.o \
	$(BUILDDIR)/glclient/menu.o \
	$(BUILDDIR)/glclient/net_chan.o \
	$(BUILDDIR)/glclient/net_udp.o \
	$(BUILDDIR)/glclient/nonintel.o \
	$(BUILDDIR)/glclient/pmove.o \
	$(BUILDDIR)/glclient/pmovetst.o \
	$(BUILDDIR)/glclient/r_part.o \
	$(BUILDDIR)/glclient/sbar.o \
	$(BUILDDIR)/glclient/skin.o \
	$(BUILDDIR)/glclient/snd_dma.o \
	$(BUILDDIR)/glclient/snd_mem.o \
	$(BUILDDIR)/glclient/snd_mix.o \
	$(BUILDDIR)/glclient/view.o \
	$(BUILDDIR)/glclient/wad.o \
	$(BUILDDIR)/glclient/zone.o \
	$(BUILDDIR)/glclient/cd_linux.o \
	$(BUILDDIR)/glclient/sys_linux.o \
	$(BUILDDIR)/glclient/snd_linux.o \
	$(BUILDDIR)/glclient/version.o \
	$(BUILDDIR)/glclient/cl_slist.o \
	$(BUILDDIR)/glclient/teamplay.o \
	\
	$(BUILDDIR)/glclient/gl_draw.o \
	$(BUILDDIR)/glclient/gl_mesh.o \
	$(BUILDDIR)/glclient/gl_model.o \
	$(BUILDDIR)/glclient/gl_ngraph.o \
	$(BUILDDIR)/glclient/gl_refrag.o \
	$(BUILDDIR)/glclient/gl_rlight.o \
	$(BUILDDIR)/glclient/gl_rmain.o \
	$(BUILDDIR)/glclient/gl_rmisc.o \
	$(BUILDDIR)/glclient/gl_rsurf.o \
	$(BUILDDIR)/glclient/gl_screen.o \
	$(BUILDDIR)/glclient/gl_warp.o \
	\
#	$(BUILDDIR)/glclient/cl_math.o \
#	$(BUILDDIR)/glclient/math.o \
#	$(BUILDDIR)/glclient/snd_mixa.o \
#	$(BUILDDIR)/glclient/sys_x86.o
	
GLQWCL_SVGA_OBJS = $(BUILDDIR)/glclient/gl_vidlinux.o
GLQWCL_X11_OBJS = $(BUILDDIR)/glclient/gl_vidlinuxglx.o

$(BUILDDIR)/qwplayer-gl : $(GLQWCL_OBJS) $(GLQWCL_SVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQWCL_OBJS) $(GLQWCL_SVGA_OBJS) $(LDFLAGS) $(GL_SVGA_LDFLAGS)

$(BUILDDIR)/qwplayer-gl.x11 : $(GLQWCL_OBJS) $(GLQWCL_X11_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQWCL_OBJS) $(GLQWCL_X11_OBJS) $(LDFLAGS) $(GL_X11_LDFLAGS)

$(BUILDDIR)/glclient/cl_demo.o :       $(CLIENT_DIR)/cl_demo.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_ents.o :       $(CLIENT_DIR)/cl_ents.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_input.o :      $(CLIENT_DIR)/cl_input.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_main.o :       $(CLIENT_DIR)/cl_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_parse.o :      $(CLIENT_DIR)/cl_parse.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_pred.o :       $(CLIENT_DIR)/cl_pred.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_tent.o :       $(CLIENT_DIR)/cl_tent.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_cam.o :        $(CLIENT_DIR)/cl_cam.c
	$(DO_GL_CC)
$(BUILDDIR)/glclient/cl_cmd.o :        $(CLIENT_DIR)/cl_cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cmd.o :           $(CLIENT_DIR)/cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/common.o :        $(CLIENT_DIR)/common.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/console.o :       $(CLIENT_DIR)/console.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/crc.o :           $(CLIENT_DIR)/crc.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cvar.o :          $(CLIENT_DIR)/cvar.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/keys.o :          $(CLIENT_DIR)/keys.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/mathlib.o :       $(CLIENT_DIR)/mathlib.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/mdfour.o :           $(CLIENT_DIR)/mdfour.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/menu.o :          $(CLIENT_DIR)/menu.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/net_chan.o :      $(CLIENT_DIR)/net_chan.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/net_udp.o :       $(CLIENT_DIR)/net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/nonintel.o :      $(CLIENT_DIR)/nonintel.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/pmove.o :         $(CLIENT_DIR)/pmove.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/pmovetst.o :      $(CLIENT_DIR)/pmovetst.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/r_part.o :        $(CLIENT_DIR)/r_part.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/sbar.o :          $(CLIENT_DIR)/sbar.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/screen.o :        $(CLIENT_DIR)/screen.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/skin.o :          $(CLIENT_DIR)/skin.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/snd_dma.o :       $(CLIENT_DIR)/snd_dma.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/snd_mem.o :       $(CLIENT_DIR)/snd_mem.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/snd_mix.o :       $(CLIENT_DIR)/snd_mix.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/view.o :          $(CLIENT_DIR)/view.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/wad.o :           $(CLIENT_DIR)/wad.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/zone.o :          $(CLIENT_DIR)/zone.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cd_linux.o :      $(CLIENT_DIR)/cd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/sys_linux.o :     $(CLIENT_DIR)/sys_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/snd_linux.o :     $(CLIENT_DIR)/snd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/cl_slist.o :       $(CLIENT_DIR)/cl_slist.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_draw.o :       $(CLIENT_DIR)/gl_draw.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_mesh.o :       $(CLIENT_DIR)/gl_mesh.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_model.o :      $(CLIENT_DIR)/gl_model.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_ngraph.o :     $(CLIENT_DIR)/gl_ngraph.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_refrag.o :     $(CLIENT_DIR)/gl_refrag.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_rlight.o :     $(CLIENT_DIR)/gl_rlight.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_rmain.o :      $(CLIENT_DIR)/gl_rmain.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_rmisc.o :      $(CLIENT_DIR)/gl_rmisc.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_rsurf.o :      $(CLIENT_DIR)/gl_rsurf.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_screen.o :     $(CLIENT_DIR)/gl_screen.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_vidlinux.o :   $(CLIENT_DIR)/gl_vidlinux.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_vidlinuxglx.o :   $(CLIENT_DIR)/gl_vidlinuxglx.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/gl_warp.o :       $(CLIENT_DIR)/gl_warp.c
	$(DO_GL_CC)
	
$(BUILDDIR)/glclient/version.o :      $(CLIENT_DIR)/version.c
	$(DO_GL_CC)

$(BUILDDIR)/glclient/teamplay.o :      $(CLIENT_DIR)/teamplay.c
	$(DO_GL_CC)


$(BUILDDIR)/glclient/cl_math.o :       $(CLIENT_DIR)/cl_math.s
	$(DO_GL_AS)

$(BUILDDIR)/glclient/math.o :          $(CLIENT_DIR)/math.s
	$(DO_GL_AS)

$(BUILDDIR)/glclient/snd_mixa.o :      $(CLIENT_DIR)/snd_mixa.s
	$(DO_GL_AS)

$(BUILDDIR)/glclient/sys_x86.o :      $(CLIENT_DIR)/sys_x86.s
	$(DO_GL_AS)


