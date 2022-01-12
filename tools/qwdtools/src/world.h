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

#ifndef __WORLD_H__
#define __WORLD_H__

// player_state_t is the information needed by a player entity
// to do move prediction and to generate a drawable entity
typedef struct
{
	int			messagenum;		// all player's won't be updated each frame

	usercmd_t	command;		// last command for prediction

	vec3_t		origin;
	vec3_t		viewangles;		// only for demos, not from server
	vec3_t		velocity;
	int			weaponframe;

	int			modelindex;
	int			frame;
	int			skinnum;
	int			effects;

	int			flags;			// dead, gib, etc
	int			msec;
} player_state_t;


#define	MAX_SCOREBOARDNAME	16
typedef struct player_info_s
{
	char	userinfo[MAX_EXT_INFO_STRING];
	char	name[MAX_SCOREBOARDNAME];
	int		stats[MAX_CL_STATS];	// health, etc
	qbool	spectator;
	int		lastsource;
} player_info_t;

typedef struct
{
	vec3_t	origin;
	vec3_t	angles;
	int		weaponframe;
	int		skinnum;
	int		model;
	int		effects;
	int		parsecount;
} demoinfo_t;

typedef struct
{
	int		modelindex;
	vec3_t	origin;
	vec3_t	angles;
	int		num;
} projectile_t;

#define	MAX_PROJECTILES	32

typedef struct
{
	// generated on client side
	usercmd_t	cmd; // cmd that generated the frame
	double		senttime; // time cmd was sent off
	int			delta_sequence; // sequence number to delta from, -1 = full update

	// received from server
	double		receivedtime; // time message was received, or -1
	player_state_t	playerstate[MAX_CLIENTS]; // message received that reflects performing
	qbool		fixangle[MAX_CLIENTS];
	// the usercmd
	packet_entities_t packet_entities;
	qbool		invalid; // true if the packet_entities delta was invalid
	sizebuf_t	buf;
	projectile_t	projectiles[MAX_PROJECTILES];
	int			num_projectiles;
	int			parsecount;
	float		latency;
	float		time;
} frame_t;


typedef struct
{
	int		length;
	char	map[MAX_STYLESTRING];
} lightstyle_t;

#define	MAX_EFRAGS	512

#define	MAX_DEMOS	8
#define	MAX_DEMONAME	16

typedef struct
{
	qbool	interpolate;
	vec3_t	origin;
	vec3_t	angles;
	int		oldindex;
} interpolate_t;

typedef struct
{
	FILE	*file;
	char	path[MAX_OSPATH];
	char	name[MAX_OSPATH];
	long	filesize;
} file_t;

typedef struct
{
	FILE	*file;
	char	name[MAX_OSPATH];

	int		frags[MAX_CLIENTS];
	int		total_clients;
	int		total_spectators;
	int		teamfrags[MAX_CLIENTS];
	int		deathmach;
	int		teamplay;
	char	*povs[MAX_CLIENTS];
	int		timelimit;
	int		fraglimit;
	float	demotime;
	int		demofps;
} analyse_t;

typedef struct
{
	int		servercount;	// server identification for prespawns
	char	mapname[64];	// full map name
	char	serverinfo[MAX_SERVERINFO_STRING];
	int		parsecount;		// server message counter
	int		delta_sequence;
	int		validsequence;	// this is the sequence number of the last good
	// packetentity_t we got.  If this is 0, we can't
	// render a frame yet
	int		lastwritten;
	frame_t	frames[UPDATE_BACKUP];

	player_info_t	players[MAX_CLIENTS];
	qbool	signonloaded;

	demoinfo_t	demoinfo[MAX_CLIENTS];
	float	time;
	long	oldftell;
	int		percentage;
	long	demossize;
	int		running;
	sizebuf_t	messages;
	byte	buffer[45*MAX_MSGLEN];
	int		lastmarged;
	qbool	signonstats;
} world_state_t;

typedef struct
{
	int			options; // QWDTools options
	int			fps;
	int			msglevel;
	file_t		debug;
	file_t		log;
	file_t		from[MAX_CLIENTS];
	file_t		demo;
	analyse_t	analyse;
	int			sources;
	int			count;
	int			fromcount;
	flist_t		filelist[50];
	int			range;
} static_world_state_t;

extern char		qizmoDir[MAX_OSPATH];
extern char		outputDir[MAX_OSPATH];
extern world_state_t	world;
extern static_world_state_t	sworld;
extern lightstyle_t	lightstyle[MAX_LIGHTSTYLES];
extern entity_state_t	baselines[MAX_EDICTS];

#endif /* !__WORLD_H__ */
