/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// qwsvdef.h -- primary header for server
#ifndef __QWSVDEF_H__
#define __QWSVDEF_H__

//for msvc #pragma message lines
#define MY_STRINGIFY2(s) #s
#define MY_STRINGIFY(s) MY_STRINGIFY2(s)

#if defined(_MSC_VER)
#define MSVC_LINE   __FILE__"("MY_STRINGIFY(__LINE__)") : warning : "
#define msg(s) message(MSVC_LINE s)
#elif __GNUC__ >=4
#define msg(s) message(s)
#else
#define msg(...)
#endif

//define PARANOID // speed sapping error checking

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE	// don't bitch about strncat etc
#pragma warning( disable : 4244 4267 4127 4201 4214 4514 4305 4115 4018 4996)
#endif

#include <time.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <limits.h>

#include "quakeasm.h"
#include "bothdefs.h"

#include "mathlib.h"
#include "zone.h"
#include "cvar.h"
#include "cmd.h"
#include "hash.h"
#include "protocol.h"

#include "common.h"
#include "net.h"
#include "sys.h"
#include "fs.h"
#include "vfs.h"

#include "cmodel.h"

#include "crc.h"
#include "sha1.h"
#include "sha3.h"

#include "server.h"

#include "sv_world.h"
#include "pmove.h"
#include "log.h"
#include "sv_broadcast.h"

#include "version.h"

#ifndef PCRE_STATIC
#define PCRE_STATIC
#endif

#include "pcre/pcre.h"

//=============================================================================

extern	cvar_t	sys_nostdout;
extern	cvar_t	developer;

extern	qbool	host_initialized;
extern	qbool	host_everything_loaded;
extern	double	curtime; // not bounded or scaled, shared by local client and server

void SV_Error (char *error, ...);
void SV_Init (void);
void SV_Shutdown (char *finalmsg);
#define Host_Error SV_Error // ezquake compatibility
void Host_Init (int argc, char **argv, int default_memsize);
void Host_ClearMemory (void);

void Con_Printf (char *fmt, ...);
void Con_DPrintf (char *fmt, ...);

#endif /* !__QWSVDEF_H__ */
