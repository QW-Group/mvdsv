#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <limits.h>
#include <direct.h>
#include <ctype.h>

#pragma warning( disable : 4244)

typedef enum {false, true} qboolean;

typedef unsigned char byte;
#define _DEF_BYTE_

typedef float vec_t;
typedef vec_t vec3_t[3];

#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}

#define DF_ORIGIN	1
#define DF_ANGLES	(1<<3)
#define DF_EFFECTS	(1<<6)
#define DF_SKINNUM	(1<<7)
#define DF_DEAD		(1<<8)
#define DF_GIB		(1<<9)
#define DF_WEAPONFRAME (1<<10)
#define DF_MODEL	(1<<11)

#include "protocol.h"
#include "bothdefs.h"
#include "tools.h"
#include "world.h"

extern	sizebuf_t	net_message;

#define	MAX_LATENT	32
#define HISTORY (UPDATE_BACKUP - 10)

typedef struct
{
// bandwidth estimator
	double		cleartime;			// if realtime > nc->cleartime, free to go
	double		rate;				// seconds / byte

// sequencing variables
	int			incoming_sequence;
	int			incoming_acknowledged;
	int			incoming_reliable_acknowledged;	// single bit

	int			incoming_reliable_sequence;		// single bit, maintained local

	int			outgoing_sequence;
	int			reliable_sequence;			// single bit
	int			last_reliable_sequence;		// sequence number of last send

// reliable staging and holding areas
	sizebuf_t	message;		// writing buffer to send to server
	byte		message_buf[MAX_MSGLEN];

	int			reliable_length;
	byte		reliable_buf[MAX_MSGLEN];	// unacked reliable message

// time and size data to calculate bandwidth
	int			outgoing_size[MAX_LATENT];
	double		outgoing_time[MAX_LATENT];
} netchan_t;


typedef enum {
	qwd,
	mvd
} format_t;

typedef struct
{
	FILE		*file;
	char		name[MAX_OSPATH];
	sizebuf_t	buf;
	byte		buf_data[10*MAX_MSGLEN]; // heh?
	sizebuf_t	datagram;
	byte		datagram_data[MAX_DATAGRAM];
	int			lastto;
	int			lasttype;
	double		time, pingtime;
	int			stats[MAX_CLIENTS][MAX_CL_STATS]; // ouch!
	//double		frametime[UPDATE_BACKUP];
} demo_t;

typedef struct
{
	FILE		*file;
	char		name[MAX_OSPATH];
	float		time;
	format_t	format;
	byte		type;
	int			to;
	int			parsecountmod, parsecount;
	frame_t		frames[UPDATE_BACKUP];
	player_info_t	players[MAX_CLIENTS];
	netchan_t	netchan;
	qboolean	running;
	float		lastframe;
	int			servercount;
	int			playernum;
	int			prevnum[MAX_CLIENTS];
	int			validsequence;
	qboolean	spectator;
	int			framecount;
	float		latency;
	int			spec_track;
} source_t;

//
// main.c
//

void Sys_Error (char *error, ...);
void Sys_Printf (char *fmt, ...);
void Dem_Stop(void);

extern demo_t	demo;
extern source_t	from;
extern qboolean debug;
extern qboolean filter_spectalk;
extern qboolean	filter_qizmotalk;

//
// dem_parse.c
//

#define NET_TIMINGS 256
#define NET_TIMINGSMASK 255
extern int	packet_latency[NET_TIMINGS];
void Dem_ParseDemoMessage (void);
int TranslateFlags(int src, int to);

//
// dem_send.c
//

//void WriteEntitiesToClient (sizebuf_t *msg, frame_t *frame);
void WritePackets(int num);

