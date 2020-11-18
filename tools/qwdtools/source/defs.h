/*
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

    $Id$
*/

#ifndef __DEFS_H__
#define __DEFS_H__

#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <malloc.h>
#include <float.h>
#include <process.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 4996)
#endif
#else
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h> // basename
#define _O_TEXT 0
#define _O_BINARY 1
#endif

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <fcntl.h>

#define INI_FILE "qwdtools.ini"

typedef float vec_t;
typedef vec_t vec3_t[3];

#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}

#define DF_ORIGIN		1
#define DF_ANGLES		(1<<3)
#define DF_EFFECTS		(1<<6)
#define DF_SKINNUM		(1<<7)
#define DF_DEAD			(1<<8)
#define DF_GIB			(1<<9)
#define DF_WEAPONFRAME	(1<<10)
#define DF_MODEL		(1<<11)

// QWDTools options

#define O_CONVERT		1
#define O_MARGE			2
#define O_ANALYSE		4
#define O_LOG			8
#define O_DEBUG			16
#define O_FS			32 // filter_spectalk
#define O_FQ			64 // filter_qizmotalk
#define O_FC			128 // filter_qizmotalk
#define O_FT			256
#define O_WAITFORKBHIT	512 // "press any key" when finished
#define O_SHUTDOWN		1024
#define O_STDIN			2048
#define O_STDOUT		4096
#define O_QWDSYNC		8192

#define JOB_TODO (O_MARGE | O_CONVERT | O_ANALYSE | O_LOG | O_DEBUG)

#define QWDTOOLS
#include "../../../src/bothdefs.h"
#include "../../../src/protocol.h"
#include "../../../src/version.h"
#include "tools.h"
#include "world.h"

extern	sizebuf_t net_message;

#define MAX_UDP_PACKET (MAX_MSGLEN*2) // one more than msg + header
#define MAX_SOURCES 50

typedef struct
{
	// bandwidth estimator
	double	cleartime;					// if realtime > nc->cleartime, free to go
	double	rate;						// seconds / byte

	// sequencing variables
	int	incoming_sequence;
	int	incoming_acknowledged;
	int	incoming_reliable_acknowledged;	// single bit

	int	incoming_reliable_sequence;		// single bit, maintained local

	int	outgoing_sequence;
	int	reliable_sequence;				// single bit
	int	last_reliable_sequence;			// sequence number of last send

	int	reliable_length;

} netchan_t;


typedef enum {qwd, mvd} format_t;

typedef struct
{
	sizebuf_t	datagram;
	byte		datagram_data[MAX_DATAGRAM];
	int			lastto;
	int			lasttype;
	double		time, pingtime;
	int			stats[MAX_CLIENTS][MAX_CL_STATS]; // ouch!
	byte		buffer[15*MAX_MSGLEN];
	dbuffer_t	dbuffer;
} demo_t;

typedef struct
{
	float		sync;
	float		time;
	float		worldtime;
	format_t	format;
	byte		type;
	int			to;
	int			parsecountmod, parsecount, oldparse;
	frame_t		frames[UPDATE_BACKUP];
	player_info_t	players[MAX_CLIENTS];
	netchan_t	netchan;
	int			running;
	float		lastframe;
	int			servercount;
	int			playernum;
	int			prevnum[MAX_CLIENTS];
	int			validsequence;
	qbool		spectator;
	int			framecount;
	float		latency;
	int			spec_track;
	qbool		qwz;
	long		prevtime;
	float		basetime;
	float		lasttime;
	byte		prevnewtime;
	byte		buffer[15*MAX_MSGLEN];
	sizebuf_t	buf[UPDATE_BACKUP];
	sizebuf_t	*msgbuf;
	sizebuf_t	signon_stats;
	byte		signon_stats_buf[2*MAX_MSGLEN];
	dbuffer_t	dbuffer;
	qbool		signonloaded;
	float		ratio;
} source_t;

typedef enum
{
    TYPE_S = 1,
    TYPE_I = 2,
    TYPE_O = 4,
} type_t;

typedef struct
{

	char	*name, *shname;
	type_t	type;
	union opt {
		char	*str;
		int		*Int;
		int		Opt;
	} opt1;
	int		str_len;
	int		opt2;
} param_t;

//
// main.c
//

void Sys_fclose (FILE **hndl);
void Sys_Exit (int i);
void Sys_mkdir (const char *path);
void Sys_Error (const char *error, ...);
void Sys_Printf (char *fmt, ...);
void Dem_Stop(source_t *s);

extern sizebuf_t	net_message;
extern byte			net_message_buffer[MAX_UDP_PACKET];
extern sizebuf_t	stats_msg;
extern byte			stats_buf[MAX_MSGLEN];
extern char			currentDir[1024]; // MAX_OSPATH
#ifdef _WIN32
extern HANDLE		ConsoleInHndl, ConsoleOutHndl;
#endif
extern char			sourceName[MAX_SOURCES][MAX_OSPATH];

extern demo_t		demo;
extern source_t		*from;
extern source_t		*sources;
extern qbool 		filter_spectalk;
extern qbool 		filter_qizmotalk;

//
// dem_parse.c
//

#define NET_TIMINGS 256
#define NET_TIMINGSMASK 255

void Dem_ParseDemoMessage (void);
int TranslateFlags (int src, int to);

//
// dem_send.c
//

void WritePackets (int num);

//
// init.c
//

void CtrlH_Init (void);
void World_Init (void);
void ParseArgv (void);
void Load_ini (void);
int Files_Init (int options);

//
// ini.c
//

void ReadIni (char *buf);

//
// qwz.c
//
qbool OpenQWZ (char *files);
void StopQWZ (source_t *s);

#endif /* !__DEFS_H__ */
