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

    $Id: qwsvdef.h,v 1.19 2006/06/16 17:35:08 vvd0 Exp $
*/
// qwsvdef.h -- primary header for server

#ifndef __QWSVDEF_H__
#define __QWSVDEF_H__

//define PARANOID // speed sapping error checking

#ifdef _WIN32
#pragma warning( disable : 4244 4127 4201 4214 4514 4305 4115 4018)
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

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__sun__) || defined(__GNUC__) || defined(__APPLE__)
#if (defined(__FreeBSD__) || defined(__DragonFly__)) && defined(KQUEUE)
#include <sys/types.h>
#include <sys/event.h>
#endif
#include <sys/time.h>
#endif

#ifndef _WIN32
#include <dirent.h>
#endif

#include "bothdefs.h"
#include "common.h"
#include "bspfile.h"
#include "sys.h"
#include "zone.h"
#include "fs.h"
#include "mathlib.h"

#include "cvar.h"
#include "net.h"
#include "protocol.h"
#include "cmd.h"

#include "model.h"

#include "crc.h"
#include "sha1.h"
#include "mdfour.h"

#include "server.h"
#include "world.h"
#include "pmove.h"
#include "log.h"

#include "version.h"

#include "pcre/pcre.h"

#ifdef USE_PR2
// Angel -->
#include "pr2_vm.h"
#include "pr2.h"
// <-- Angel
#endif

//=============================================================================

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use

typedef struct
{
	int	argc;
	char	**argv;
	void	*membase;
	int	memsize;
} quakeparms_t;


//=============================================================================

typedef struct
{
	int sec;
	int min;
	int hour;
	int day;
	int mon;
	int year;
	char str[128];
} date_t;
void SV_TimeOfDay(date_t *date);

extern	quakeparms_t host_parms;

extern	cvar_t	sys_nostdout;
extern	cvar_t	developer;

extern	qbool host_initialized; // true if into command execution
extern	double	realtime; // not bounded in any way, changed at start of every frame, never reset

void SV_Error (char *error, ...);
void SV_Init (quakeparms_t *parms);

void Con_Printf (char *fmt, ...);
void Con_DPrintf (char *fmt, ...);

#endif /* !__QWSVDEF_H__ */
