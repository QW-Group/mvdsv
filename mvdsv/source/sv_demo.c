/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "qwsvdef.h"
#include "winquake.h"

#define MIN_DEMO_MEMORY 0x100000
#define USACACHE (sv_demoUseCache.value && svs.demomemsize)
#define DWRITE(a,b,c,d) b*dwrite(a,b,c,d)
#define MAXSIZE (demobuffer->end < demobuffer->last ? \
				demobuffer->start - demobuffer->end : \
				demobuffer->maxsize - demobuffer->end)


extern cvar_t	sv_demoUseCache;
extern cvar_t	sv_demoCacheSize;
extern cvar_t	sv_demoMaxDirSize;
extern cvar_t	sv_demoDir;
extern cvar_t	sv_demofps;
extern cvar_t	sv_demoPings;
extern cvar_t	sv_demoNoVis;
extern cvar_t	sv_demoMaxSize;
extern cvar_t	sv_demoExtraNames;

static int		demo_max_size;
static int		demo_size;
cvar_t			sv_demoPrefix = {"sv_demoPrefix", ""};
cvar_t			sv_demoSuffix = {"sv_demoSuffix", ""};
cvar_t			sv_onrecordfinish = {"sv_onRecordFinish", ""};
cvar_t			sv_ondemoremove = {"sv_onDemoRemove", ""};
cvar_t			sv_demotxt = {"sv_demotxt", "1"};

void SV_WriteDemoMessage (sizebuf_t *msg, int type, int to, float time);
size_t (*dwrite) ( const void *buffer, size_t size, size_t count, void *stream);

static dbuffer_t	*demobuffer;
static int	header = (int)&((header_t*)0)->data;

entity_state_t demo_entities[UPDATE_MASK+1][MAX_DEMO_PACKET_ENTITIES];

// only one .. is allowed (security)
qboolean sv_demoDir_OnChange(cvar_t *cvar, char *value)
{
	if (!value[0])
		return true;

	if (value[0] == '.' && value[1] == '.')
		value += 2;
	if (strstr(value,"/.."))
		return true;

	return false;
}

void SV_DemoPings (void)
{
	client_t *client;
	int		j;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		DemoWrite_Begin (dem_all, 0, 7);
		MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updateping);
		MSG_WriteByte((sizebuf_t*)demo.dbuf,  j);
		MSG_WriteShort((sizebuf_t*)demo.dbuf,  SV_CalcPing(client));
		MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updatepl);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, j);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, client->lossage);
	}
}

void DemoBuffer_Init(dbuffer_t *dbuffer, byte *buf, size_t size)
{
	demobuffer = dbuffer;

	demobuffer->data = buf;
	demobuffer->maxsize = size;
	demobuffer->start = 0;
	demobuffer->end = 0;
	demobuffer->last = 0;
}

/*
==============
Demo_SetMsgBuf

Sets the frame message buffer
==============
*/

void DemoSetMsgBuf(demobuf_t *prev,demobuf_t *cur)
{
	// fix the maxsize of previous msg buffer,
	// we won't be able to write there anymore
	if (prev != NULL)
		prev->maxsize = prev->bufsize;

	demo.dbuf = cur;
	memset(demo.dbuf, 0, sizeof(*demo.dbuf));

	demo.dbuf->data = demobuffer->data + demobuffer->end;
	demo.dbuf->maxsize = MAXSIZE;
}

/*
==============
DemoWriteToDisk

Writes to disk a message meant for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

void SV_DemoWriteToDisk(int type, int to, float time)
{
	int pos = 0, oldm, oldd;
	header_t *p;
	int	size;
	sizebuf_t msg;

	(byte*)p = demo.dbuf->data;
	demo.dbuf->h = NULL;

	oldm = demo.dbuf->bufsize;
	oldd = demobuffer->start;
	while (pos < demo.dbuf->bufsize)
	{
		size = p->size;
		pos += header + size;

		// no type means we are writing to disk everything
		if (!type || (p->type == type && p->to == to))
		{
			if (size) {
				msg.data = p->data;
				msg.cursize = size;

				SV_WriteDemoMessage(&msg, p->type, p->to, time);
			}

			// data is written so it need to be cleard from demobuf
			if (demo.dbuf->data != (byte*)p)
				memmove(demo.dbuf->data + size + header, demo.dbuf->data, (byte*)p - demo.dbuf->data);

			demo.dbuf->bufsize -= size + header;
			demo.dbuf->data += size + header;
			pos -= size + header;
			demo.dbuf->maxsize -= size + header;
			demobuffer->start += size + header;
		}
		// move along
		(byte*)p = p->data + size;
	}

	if (demobuffer->start == demobuffer->last) {
		if (demobuffer->start == demobuffer->end) {
			demobuffer->end = 0; // demobuffer is empty
			demo.dbuf->data = demobuffer->data;
		}

		// go back to begining of the buffer
		demobuffer->last = demobuffer->end;
		demobuffer->start = 0;
	}
}

/*
==============
DemoSetBuf

Sets position in the buf for writing to specific client
==============
*/

static void DemoSetBuf(byte type, int to)
{
	header_t *p;
	int pos = 0;

	(byte*)p = demo.dbuf->data;

	while (pos < demo.dbuf->bufsize)
	{
		pos += header + p->size;

		if (type == p->type && to == p->to && !p->full)
		{
			demo.dbuf->cursize = pos;
			demo.dbuf->h = p;
			return;
		}

		(byte*)p = p->data + p->size;
	}
	// type&&to not exist in the buf, so add it

	p->type = type;
	p->to = to;
	p->size = 0;
	p->full = 0;

	demo.dbuf->bufsize += header;
	demo.dbuf->cursize = demo.dbuf->bufsize;
	demobuffer->end += header;
	demo.dbuf->h = p;
}

void DemoMoveBuf(void)
{
	// set the last message mark to the previous frame (i/e begining of this one)
	demobuffer->last = demobuffer->end - demo.dbuf->bufsize;

	// move buffer to the begining of demo buffer
	memmove(demobuffer->data, demo.dbuf->data, demo.dbuf->bufsize);
	demo.dbuf->data = demobuffer->data;
	demobuffer->end = demo.dbuf->bufsize;
	demo.dbuf->h = NULL; // it will be setup again
	demo.dbuf->maxsize = MAXSIZE + demo.dbuf->bufsize;
}

void DemoWrite_Begin(byte type, int to, int size)
{
	byte *p;
	qboolean move = false;

	// will it fit?
	while (demo.dbuf->bufsize + size + header > demo.dbuf->maxsize)
	{
		// if we reached the end of buffer move msgbuf to the begining
		if (!move && demobuffer->end > demobuffer->start)
			move = true;

		SV_DemoWritePackets(1);
		if (move && demobuffer->start > demo.dbuf->bufsize + header + size)
			DemoMoveBuf();
	}

	if (demo.dbuf->h == NULL || demo.dbuf->h->type != type || demo.dbuf->h->to != to || demo.dbuf->h->full) {
		DemoSetBuf(type, to);
	}

	if (demo.dbuf->h->size + size > MAX_MSGLEN)
	{
		demo.dbuf->h->full = 1;
		DemoSetBuf(type, to);
	}

	// we have to make room for new data
	if (demo.dbuf->cursize != demo.dbuf->bufsize) {
		p = demo.dbuf->data + demo.dbuf->cursize;
		memmove(p+size, p, demo.dbuf->bufsize - demo.dbuf->cursize);
	}

	demo.dbuf->bufsize += size;
	demo.dbuf->h->size += size;
	if ((demobuffer->end += size) > demobuffer->last)
		demobuffer->last = demobuffer->end;
}

/*
====================
SV_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void SV_WriteDemoMessage (sizebuf_t *msg, int type, int to, float time)
{
	int		len, i, msec;
	byte	c;
	static double prevtime;

	if (!sv.demorecording)
		return;

	msec = (time - prevtime)*1000;
	prevtime += msec*0.001;
	if (msec > 255) msec = 255;
	if (msec < 2) msec = 0;

	c = msec;
	demo.size += DWRITE(&c, sizeof(c), 1, demo.dest);

	if (demo.lasttype != type || demo.lastto != to)
	{
		demo.lasttype = type;
		demo.lastto = to;
		switch (demo.lasttype)
		{
		case dem_all:
			c = dem_all;
			demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);
			break;
		case dem_multiple:
			c = dem_multiple;
			demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);

			i = LittleLong(demo.lastto);
			demo.size += DWRITE (&i, sizeof(i), 1, demo.dest);
			break;
		case dem_single:
		case dem_stats:
			c = demo.lasttype + (demo.lastto << 3);
			demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);
			break;
		default:
			SV_Stop_f ();
			Con_Printf("bad demo message type:%d", type);
			return;
		}
	} else {
		c = dem_read;
		demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);
	}


	len = LittleLong (msg->cursize);
	demo.size += DWRITE (&len, 4, 1, demo.dest);
	demo.size += DWRITE (msg->data, msg->cursize, 1, demo.dest);

	if (demo.disk)
		fflush (demo.file);
	else if (demo.size - demo_size > demo_max_size)
	{
		demo_size = demo.size;
		demo.mfile -= 0x80000;
		fwrite(svs.demomem, 1, 0x80000, demo.file);
		fflush(demo.file);
		memmove(svs.demomem, svs.demomem + 0x80000, demo.size - 0x80000);
	}
}


/*
====================
SV_DemoWritePackets

Interpolates to get exact players position for current frame
and writes packets to the disk/memory
====================
*/

float adjustangle(float current, float ideal, float fraction)
{
	float move;

	move = ideal - current;
	if (ideal > current)
	{

		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	move *= fraction;

	return (current + move);
}

#define DF_ORIGIN	1
#define DF_ANGLES	(1<<3)
#define DF_EFFECTS	(1<<6)
#define DF_SKINNUM	(1<<7)
#define DF_DEAD		(1<<8)
#define DF_GIB		(1<<9)
#define DF_WEAPONFRAME (1<<10)
#define DF_MODEL	(1<<11)

void SV_DemoWritePackets (int num)
{
	demo_frame_t	*frame, *nextframe;
	demo_client_t	*cl, *nextcl = NULL;
	int				i, j, flags;
	qboolean		valid;
	double			time, playertime, nexttime;
	float			f;
	vec3_t			origin, angles;
	sizebuf_t		msg;
	byte			msg_buf[MAX_MSGLEN];
	demoinfo_t		*demoinfo;

	if (!sv.demorecording)
		return;

	msg.data = msg_buf;
	msg.maxsize = sizeof(msg_buf);

	if (num > demo.parsecount - demo.lastwritten + 1)
		num = demo.parsecount - demo.lastwritten + 1;

	// 'num' frames to write
	for ( ; num; num--, demo.lastwritten++)
	{
		frame = &demo.frames[demo.lastwritten&DEMO_FRAMES_MASK];
		time = frame->time;
		nextframe = frame;
		msg.cursize = 0;

		demo.dbuf = &frame->buf;

		// find two frames
		// one before the exact time (time - msec) and one after,
		// then we can interpolte exact position for current frame
		for (i = 0, cl = frame->clients, demoinfo = demo.info; i < MAX_CLIENTS; i++, cl++, demoinfo++)
		{
			if (cl->parsecount != demo.lastwritten)
				continue; // not valid

			nexttime = playertime = time - cl->sec;

			for (j = demo.lastwritten+1, valid = false; nexttime < time && j < demo.parsecount; j++)
			{
				nextframe = &demo.frames[j&DEMO_FRAMES_MASK];
				nextcl = &nextframe->clients[i];

				if (nextcl->parsecount != j)
					break; // disconnected?
				if (nextcl->fixangle)
					break; // respawned, or walked into teleport, do not interpolate!
				if (!(nextcl->flags & DF_DEAD) && (cl->flags & DF_DEAD))
					break; // respawned, do not interpolate

				nexttime = nextframe->time - nextcl->sec;

				if (nexttime >= time)
				{
					// good, found what we were looking for
					valid = true;
					break;
				}
			}

			if (valid)
			{
				f = (time - nexttime)/(nexttime - playertime);
				for (j=0;j<3;j++) {
					angles[j] = adjustangle(cl->info.angles[j], nextcl->info.angles[j],1.0+f);
					origin[j] = nextcl->info.origin[j] + f*(nextcl->info.origin[j]-cl->info.origin[j]);
				}
			} else {
				VectorCopy(cl->info.origin, origin);
				VectorCopy(cl->info.angles, angles);
			}

			// now write it to buf
			flags = cl->flags;

			if (cl->fixangle) {
				demo.fixangletime[i] = cl->cmdtime;
			}

			for (j=0; j < 3; j++)
				if (origin[j] != demoinfo->origin[i])
					flags |= DF_ORIGIN << j;

			if (cl->fixangle || demo.fixangletime[i] != cl->cmdtime)
			{
				for (j=0; j < 3; j++)
					if (angles[j] != demoinfo->angles[j])
						flags |= DF_ANGLES << j;
			}

			if (cl->info.model != demoinfo->model)
				flags |= DF_MODEL;
			if (cl->info.effects != demoinfo->effects)
				flags |= DF_EFFECTS;
			if (cl->info.skinnum != demoinfo->skinnum)
				flags |= DF_SKINNUM;
			if (cl->info.weaponframe != demoinfo->weaponframe)
				flags |= DF_WEAPONFRAME;

			MSG_WriteByte (&msg, svc_playerinfo);
			MSG_WriteByte (&msg, i);
			MSG_WriteShort (&msg, flags);

			MSG_WriteByte (&msg, cl->frame);

			for (j=0 ; j<3 ; j++)
				if (flags & (DF_ORIGIN << j))
					MSG_WriteCoord (&msg, origin[j]);
		
			for (j=0 ; j<3 ; j++)
				if (flags & (DF_ANGLES << j))
					MSG_WriteAngle16 (&msg, angles[j]);


			if (flags & DF_MODEL)
				MSG_WriteByte (&msg, cl->info.model);

			if (flags & DF_SKINNUM)
				MSG_WriteByte (&msg, cl->info.skinnum);

			if (flags & DF_EFFECTS)
				MSG_WriteByte (&msg, cl->info.effects);

			if (flags & DF_WEAPONFRAME)
				MSG_WriteByte (&msg, cl->info.weaponframe);

			VectorCopy(cl->info.origin, demoinfo->origin);
			VectorCopy(cl->info.angles, demoinfo->angles);
			demoinfo->skinnum = cl->info.skinnum;
			demoinfo->effects = cl->info.effects;
			demoinfo->weaponframe = cl->info.weaponframe;
			demoinfo->model = cl->info.model;
		}

		SV_DemoWriteToDisk(demo.lasttype,demo.lastto, (float)time); // this goes first to reduce demo size a bit
		SV_DemoWriteToDisk(0,0, (float)time); // now goes the rest
		if (msg.cursize)
			SV_WriteDemoMessage(&msg, dem_all, 0, (float)time);
	}

	if (demo.lastwritten > demo.parsecount)
		demo.lastwritten = demo.parsecount;

	demo.dbuf = &demo.frames[demo.parsecount&DEMO_FRAMES_MASK].buf;
	demo.dbuf->maxsize = MAXSIZE + demo.dbuf->bufsize;
}

size_t memwrite ( const void *buffer, size_t size, size_t count, byte **mem)
{
	int i,c = count;
	const byte *buf;

	for (;count; count--)
		for (i = size, buf = buffer; i; i--)
			*(*mem)++ = *buf++;

	return c;
}

static char chartbl[256];
void CleanName_Init ();

void Demo_Init (void)
{
	int p, size = MIN_DEMO_MEMORY;

	Cvar_RegisterVariable (&sv_demofps);
	Cvar_RegisterVariable (&sv_demoPings);
	Cvar_RegisterVariable (&sv_demoNoVis);
	Cvar_RegisterVariable (&sv_demoUseCache);
	Cvar_RegisterVariable (&sv_demoCacheSize);
	Cvar_RegisterVariable (&sv_demoMaxSize);
	Cvar_RegisterVariable (&sv_demoMaxDirSize);
	Cvar_RegisterVariable (&sv_demoDir);
	Cvar_RegisterVariable (&sv_demoPrefix);
	Cvar_RegisterVariable (&sv_demoSuffix);
	Cvar_RegisterVariable (&sv_onrecordfinish);
	Cvar_RegisterVariable (&sv_ondemoremove);
	Cvar_RegisterVariable (&sv_demotxt);
	Cvar_RegisterVariable (&sv_demoExtraNames);


	p = COM_CheckParm ("-democache");
	if (p)
	{
		if (p < com_argc-1)
			size = Q_atoi (com_argv[p+1]) * 1024;
		else
			Sys_Error ("Memory_Init: you must specify a size in KB after -democache");
	}

	if (size < MIN_DEMO_MEMORY)
	{
		Con_Printf("Minimum memory size for demo cache is %dk\n", MIN_DEMO_MEMORY / 1024);
		size = MIN_DEMO_MEMORY;
	}

	svs.demomem = Hunk_AllocName ( size, "demo" );
	svs.demomemsize = size;
	demo_max_size = size - 0x80000;
	Cvar_SetROM(&sv_demoCacheSize, va("%d", size/1024));
	CleanName_Init();
}

/*
====================
SV_InitRecord
====================
*/

qboolean SV_InitRecord(void)
{
	if (!USACACHE)
	{
		dwrite = (void *)&fwrite;
		demo.dest = demo.file;
		demo.disk = true;
	} else 
	{
		dwrite = (void *)&memwrite;
		demo.mfile = svs.demomem;
		demo.dest = &demo.mfile;
	}

	demo_size = 0;

	return true;
}

/*
====================
SV_Stop

stop recording a demo
====================
*/
void SV_Script_f (void);
void SV_Stop (int reason)
{
	if (!sv.demorecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}

	if (reason == 2)
	{
		char path[MAX_OSPATH];
		// stop and remove
		if (demo.disk)
			fclose(demo.file);

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, demo.path, demo.name);
		Sys_remove(path);

		strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);
		Sys_remove(path);

		demo.file = NULL;
		sv.demorecording = false;

		SV_BroadcastPrintf (PRINT_CHAT, "Server recording canceled, demo removed\n");

		Cvar_SetROM(&serverdemo, "");

		return;
	}
// write a disconnect message to the demo file

	// clearup to be sure message will fit
	demo.dbuf->cursize = 0;
	demo.dbuf->h = NULL;
	demo.dbuf->bufsize = 0;
	DemoWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_disconnect);
	MSG_WriteString ((sizebuf_t*)demo.dbuf, "EndOfDemo");

	SV_DemoWritePackets(demo.parsecount - demo.lastwritten + 1);
// finish up
	if (!demo.disk)
	{
		fwrite(svs.demomem, 1, demo.size - demo_size, demo.file);
		fflush(demo.file);
	}

	fclose (demo.file);
	
	demo.file = NULL;
	sv.demorecording = false;
	if (!reason)
		SV_BroadcastPrintf (PRINT_CHAT, "Server recording completed\n");
	else
		SV_BroadcastPrintf (PRINT_CHAT, "Server recording stoped\nMax demo size exceeded\n");

	if (sv_onrecordfinish.string[0])
	{
		extern redirect_t sv_redirected;
		int old = sv_redirected;
		char path[MAX_OSPATH];
		char *p;

		if ((p = strstr(sv_onrecordfinish.string, " ")) != NULL)
			*p = 0; // strip parameters

		strlcpy(path, demo.name, MAX_OSPATH);
		strlcpy(path + strlen(demo.name) - 3, "txt", MAX_OSPATH - strlen(demo.name) + 3);

		sv_redirected = RD_NONE; // onrecord script is called always from the console
		Cmd_TokenizeString(va("script %s \"%s\" \"%s\" \"%s\" %s", sv_onrecordfinish.string, demo.path, serverdemo.string, path, p != NULL ? p+1 : ""));
		if (p) *p = ' ';
		SV_Script_f();

		sv_redirected = old;
	}
	
	Cvar_SetROM(&serverdemo, "");
}

/*
====================
SV_Stop_f
====================
*/
void SV_Stop_f (void)
{
	SV_Stop(0);
}

/*
====================
SV_Cancel_f

Stops recording, and removes the demo
====================
*/
void SV_Cancel_f (void)
{
	SV_Stop(2);
}

/*
====================
SV_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/

void SV_WriteRecordDemoMessage (sizebuf_t *msg, int seq)
{
	int		len;
	byte	c;

	if (!sv.demorecording)
		return;

	c = 0;
	demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);

	c = dem_read;
	demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);

	len = LittleLong (msg->cursize);
	demo.size += DWRITE (&len, 4, 1, demo.dest);

	demo.size += DWRITE (msg->data, msg->cursize, 1, demo.dest);

	if (demo.disk)
		fflush (demo.file);
}

void SV_WriteSetDemoMessage (void)
{
	int		len;
	byte	c;

//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!sv.demorecording)
		return;

	c = 0;
	demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);

	c = dem_set;
	demo.size += DWRITE (&c, sizeof(c), 1, demo.dest);

	
	len = LittleLong(0);
	demo.size += DWRITE (&len, 4, 1, demo.dest);
	len = LittleLong(0);
	demo.size += DWRITE (&len, 4, 1, demo.dest);

	if (demo.disk)
		fflush (demo.file);
}

void SV_TimeOfDay(date_t *date);
static char *SV_PrintTeams(void)
{
	char		*teams[MAX_CLIENTS], *p;
	int		i, j, numcl = 0, numt = 0, scores;
	client_t	*clients[MAX_CLIENTS];
	char		buf[2048];
	static char	lastscores[2048];
	extern cvar_t	teamplay;
	extern char	chartbl2[];
	date_t		date;
	SV_TimeOfDay(&date);

	// count teams and players
	for (i=0; i < MAX_CLIENTS; i++)
	{
		if (svs.clients[i].state != cs_spawned)
			continue;
		if (svs.clients[i].spectator)
			continue;

		clients[numcl++] = &svs.clients[i];
		for (j = 0; j < numt; j++)
			if (!strcmp(svs.clients[i].team, teams[j]))
				break;
		if (j != numt)
			continue;

		teams[numt++] = svs.clients[i].team;
	}

	// create output
	lastscores[0] = 0;
	snprintf(buf, sizeof(buf),
		"date %s\nmap %s\nteamplay %d\ndeathmatch %d\ntimelimit %d\n",
		date.str, sv.name, (int)teamplay.value, (int)deathmatch.value,
		(int)timelimit.value);
	if (numcl == 2) // duel
	{
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"player1: %s (%i)\nplayer2: %s (%i)\n",
			clients[0]->name, clients[0]->old_frags,
			clients[1]->name, clients[1]->old_frags);
		snprintf(lastscores, sizeof(lastscores), "duel: %s vs %s @ %s - %i:%i\n",
			clients[0]->name, clients[1]->name, sv.name, 
			clients[0]->old_frags, clients[1]->old_frags);
	} else if (!teamplay.value) // ffa
	{ 
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "players:\n");
		snprintf(lastscores, sizeof(lastscores), "ffa:");
		for (i = 0; i < numcl; i++)
		{
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"  %s (%i)\n", clients[i]->name, clients[i]->old_frags);
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"  %s(%i)", clients[i]->name, clients[i]->old_frags);
		}
		snprintf(lastscores + strlen(lastscores),
			sizeof(lastscores) - strlen(lastscores), " @ %s\n", sv.name);
	} else { // teamplay
		snprintf(lastscores, sizeof(lastscores), "tp:");
		for (j = 0; j < numt; j++) {
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"team[%i] %s:\n", j, teams[j]);
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"%s[", teams[j]);
			scores = 0;
			for (i = 0; i < numcl; i++)
				if (!strcmp(clients[i]->team, teams[j]))
				{
					snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
						"  %s (%i)\n", clients[i]->name, clients[i]->old_frags);
					snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
						" %s(%i) ", clients[i]->name, clients[i]->old_frags);
					scores += clients[i]->old_frags;
				}
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"](%i)  ", scores);

		}
		snprintf(lastscores + strlen(lastscores),
			sizeof(lastscores) - strlen(lastscores), "@ %s\n", sv.name);
	}

	for (p = buf; *p; p++) *p = chartbl2[(byte)*p];
	for (p = lastscores; *p; p++) *p = chartbl2[(byte)*p];
	strlcat(lastscores, buf, sizeof(lastscores));
	return lastscores;
}
//tp: team1[ player1_1(scores1_1) ... playerN_1(scoresN_1) ](scores1) ...
//    teamM[ player1_M(scores1_M) ... playerK_M(scoresK_M) ](scoresM)  @ map
//duel: player1 vs player2 @ map - scores1:scores2
//ffa: player1(scores1) ... playerN(scoresN) @ map

static void SV_Record (char *name)
{
	sizebuf_t	buf;
	char buf_data[MAX_MSGLEN];
	int n, i;
	char *s, info[MAX_INFO_STRING], path[MAX_OSPATH], *text;
	
	client_t *player;
	char *gamedir;
	int seq = 1;

	memset(&demo, 0, sizeof(demo));
	for (i = 0; i < UPDATE_BACKUP; i++)
		demo.recorder.frames[i].entities.entities = demo_entities[i];
	
	DemoBuffer_Init(&demo.dbuffer, demo.buffer, sizeof(demo.buffer));
	DemoSetMsgBuf(NULL, &demo.frames[0].buf);

	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	demo.file = fopen (name, "wb");
	if (!demo.file)
	{
		Con_Printf ("ERROR: couldn't open %s\n", name);
		return;
	}

	SV_InitRecord();

	s = name + strlen(name);
	while (*s != '/') s--;
	strlcpy(demo.name, s+1, sizeof(demo.name));
	strlcpy(demo.path, sv_demoDir.string, sizeof(demo.path));
	
	if (!*demo.path)
		strlcpy(demo.path, ".", MAX_OSPATH);

	SV_BroadcastPrintf (PRINT_CHAT, "Server starts recording (%s):\n%s\n", demo.disk ? "disk" : "memory", demo.name);
	Cvar_SetROM(&serverdemo, demo.name);

	strlcpy(path, name, MAX_OSPATH);
	strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);

	if (sv_demotxt.value) {
		FILE *f;
		if (f = fopen (path, "w+t"))
		{
			text = SV_PrintTeams();
			fwrite(text, strlen(text), 1, f);
			fflush(f);
			fclose(f);
		}
	} else
		Sys_remove(path);

	sv.demorecording = true;
	demo.pingtime = demo.time = sv.time;

/*-------------------------------------------------*/

// serverdata
	// send the info about the new client to all connected clients
	memset(&buf, 0, sizeof(buf));
	buf.data = buf_data;
	buf.maxsize = sizeof(buf_data);

// send the serverdata

	gamedir = Info_ValueForKey (svs.info, "*gamedir");
	if (!gamedir[0])
		gamedir = "qw";

	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, svs.spawncount);
	MSG_WriteString (&buf, gamedir);


	MSG_WriteFloat (&buf, sv.time);

	// send full levelname
	MSG_WriteString (&buf, PR_GetString(sv.edicts->v.message));

	// send the movevars
	MSG_WriteFloat(&buf, movevars.gravity);
	MSG_WriteFloat(&buf, movevars.stopspeed);
	MSG_WriteFloat(&buf, movevars.maxspeed);
	MSG_WriteFloat(&buf, movevars.spectatormaxspeed);
	MSG_WriteFloat(&buf, movevars.accelerate);
	MSG_WriteFloat(&buf, movevars.airaccelerate);
	MSG_WriteFloat(&buf, movevars.wateraccelerate);
	MSG_WriteFloat(&buf, movevars.friction);
	MSG_WriteFloat(&buf, movevars.waterfriction);
	MSG_WriteFloat(&buf, movevars.entgravity);

	// send music
	MSG_WriteByte (&buf, svc_cdtrack);
	MSG_WriteByte (&buf, 0); // none in demos

	// send server info string
	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("fullserverinfo \"%s\"\n", svs.info) );

	// flush packet
	SV_WriteRecordDemoMessage (&buf, seq++);
	SZ_Clear (&buf);

// soundlist
	MSG_WriteByte (&buf, svc_soundlist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.sound_precache[n+1];
	while (s) {
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2) {
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordDemoMessage (&buf, seq++);
			SZ_Clear (&buf); 
			MSG_WriteByte (&buf, svc_soundlist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.sound_precache[n+1];
	}

	if (buf.cursize) {
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordDemoMessage (&buf, seq++);
		SZ_Clear (&buf); 
	}

// modellist
	MSG_WriteByte (&buf, svc_modellist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.model_precache[n+1];
	while (s) {
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2) {
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordDemoMessage (&buf, seq++);
			SZ_Clear (&buf); 
			MSG_WriteByte (&buf, svc_modellist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.model_precache[n+1];
	}
	if (buf.cursize) {
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordDemoMessage (&buf, seq++);
		SZ_Clear (&buf); 
	}

// prespawn

	for (n = 0; n < sv.num_signon_buffers; n++)
	{
		SZ_Write (&buf, 
			sv.signon_buffers[n],
			sv.signon_buffer_size[n]);

		if (buf.cursize > MAX_MSGLEN/2) {
			SV_WriteRecordDemoMessage (&buf, seq++);
			SZ_Clear (&buf); 
		}
	}

	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("cmd spawn %i 0\n",svs.spawncount) );
	
	if (buf.cursize) {
		SV_WriteRecordDemoMessage (&buf, seq++);
		SZ_Clear (&buf); 
	}

// send current status of all other players

	for (i = 0; i < MAX_CLIENTS; i++) {
		player = svs.clients + i;

		MSG_WriteByte (&buf, svc_updatefrags);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, player->old_frags);
		
		MSG_WriteByte (&buf, svc_updateping);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, SV_CalcPing(player));
		
		MSG_WriteByte (&buf, svc_updatepl);
		MSG_WriteByte (&buf, i);
		MSG_WriteByte (&buf, player->lossage);
		
		MSG_WriteByte (&buf, svc_updateentertime);
		MSG_WriteByte (&buf, i);
		MSG_WriteFloat (&buf, realtime - player->connection_started);

		strlcpy (info, player->userinfoshort, MAX_INFO_STRING);
		Info_RemovePrefixedKeys (info, '_');	// server passwords, etc

		MSG_WriteByte (&buf, svc_updateuserinfo);
		MSG_WriteByte (&buf, i);
		MSG_WriteLong (&buf, player->userid);
		MSG_WriteString (&buf, info);

		if (buf.cursize > MAX_MSGLEN/2) {
			SV_WriteRecordDemoMessage (&buf, seq++);
			SZ_Clear (&buf); 
		}
	}
	
// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteByte (&buf, svc_lightstyle);
		MSG_WriteByte (&buf, (char)i);
		MSG_WriteString (&buf, sv.lightstyles[i]);
	}

	// get the client to check and download skins
	// when that is completed, a begin command will be issued
	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("skins\n") );

	SV_WriteRecordDemoMessage (&buf, seq++);

	SV_WriteSetDemoMessage();
	// done
}

/*
====================
SV_CleanName_Init

sets chararcter table for quake text->filename translation
====================
*/

void CleanName_Init ()
{
	int i;

	for (i = 0; i < 256; i++)
		chartbl[i] = (((i&127) < 'a' || (i&127) > 'z') && ((i&127) < '0' || (i&127) > '9')) ? '_' : (i&127);

	// special cases

	// numbers
	for (i = 18; i < 29; i++)
		chartbl[i] = chartbl[i + 128] = i + 30;

	// allow lowercase only
	for (i = 'A'; i <= 'Z'; i++)
		chartbl[i] = chartbl[i+128] = i + 'a' - 'A';

	// brackets
	chartbl[29] = chartbl[29+128] = chartbl[128] = '(';
	chartbl[31] = chartbl[31+128] = chartbl[130] = ')';
	chartbl[16] = chartbl[16 + 128]= '[';
	chartbl[17] = chartbl[17 + 128] = ']';

	// dot
	chartbl[5] = chartbl[14] = chartbl[15] = chartbl[28] = chartbl[46] = '.';
	chartbl[5 + 128] = chartbl[14 + 128] = chartbl[15 + 128] = chartbl[28 + 128] = chartbl[46 + 128] = '.';

	// !
	chartbl[33] = chartbl[33 + 128] = '!';

	// #
	chartbl[35] = chartbl[35 + 128] = '#';

	// %
	chartbl[37] = chartbl[37 + 128] = '%';

	// &
	chartbl[38] = chartbl[38 + 128] = '&';

	// '
	chartbl[39] = chartbl[39 + 128] = '\'';

	// (
	chartbl[40] = chartbl[40 + 128] = '(';

	// )
	chartbl[41] = chartbl[41 + 128] = ')';

	// +
	chartbl[43] = chartbl[43 + 128] = '+';

	// -
	chartbl[45] = chartbl[45 + 128] = '-';

	// @
	chartbl[64] = chartbl[64 + 128] = '@';

	// ^
//	chartbl[94] = chartbl[94 + 128] = '^';


	chartbl[91] = chartbl[91 + 128] = '[';
	chartbl[93] = chartbl[93 + 128] = ']';
	
	chartbl[16] = chartbl[16 + 128] = '[';
	chartbl[17] = chartbl[17 + 128] = ']';

	chartbl[123] = chartbl[123 + 128] = '{';
	chartbl[125] = chartbl[125 + 128] = '}';
}

/*
====================
SV_CleanName

Cleans the demo name, removes restricted chars, makes name lowercase
====================
*/

char *SV_CleanName (unsigned char *name)
{
	static char text[1024];
	char *out = text;

	*out = chartbl[*name++];

	while (*name && out - text < sizeof(text))
		if (*out == '_' && chartbl[*name] == '_')
			name++;
		else *++out = chartbl[*name++];

	*++out = 0;
	return text;
}

/*
====================
SV_Record_f

record <demoname>
====================
*/
void SV_Record_f (void)
{
	int		c;
	char	name[MAX_OSPATH+MAX_DEMO_NAME];
	char	newname[MAX_DEMO_NAME];
	dir_t	dir;

	c = Cmd_Argc();
	if (c != 2)
	{
		Con_Printf ("record <demoname>\n");
		return;
	}

	if (sv.state != ss_active) {
		Con_Printf ("Not active yet.\n");
		return;
	}

	
	dir = Sys_listdir(va("%s/%s", com_gamedir, sv_demoDir.string), ".*", SORT_NO);
	if (sv_demoMaxDirSize.value && dir.size > sv_demoMaxDirSize.value*1024)
	{
		Con_Printf("insufficient directory space, increase sv_demoMaxDirSize\n");
		return;
	}

	strlcpy(newname, va("%s%s", sv_demoPrefix.string, SV_CleanName(Cmd_Argv(1))),
			sizeof(newname) - strlen(sv_demoSuffix.string) - 5);
	strlcat(newname, sv_demoSuffix.string, MAX_DEMO_NAME);
  
	snprintf (name, MAX_OSPATH+MAX_DEMO_NAME, "%s/%s/%s", com_gamedir, sv_demoDir.string, newname);

	if (sv.demorecording)
		SV_Stop_f();


	Sys_mkdir(va("%s/%s", com_gamedir, sv_demoDir.string));

//
// open the demo file
//
	COM_ForceExtension (name, ".mvd");

	
	SV_Record (name);
}

/*
====================
SV_EasyRecord_f

easyrecord [demoname]
====================
*/

int	Dem_CountPlayers ()
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (svs.clients[i].name[0] && !svs.clients[i].spectator)
			count++;
	}

	return count;
}

char *Dem_Team(int num)
{
	int i;
	static char *lastteam[2];
	qboolean first = true;
	client_t *client;
	static int index = 0;

	index = 1 - index;

	for (i = 0, client = svs.clients; num && i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (first || strcmp(lastteam[index], client->team))
		{
			first = false;
			num--;
			lastteam[index] = client->team;
		}
	}

	if (num)
		return "";

	return lastteam[index];
}

char *Dem_PlayerName(int num)
{
	int i;
	client_t *client;

	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (!--num)
			return client->name;
	}

	return "";
}

// -> scream
char *Dem_PlayerNameTeam(char *t)
{
	int	i;
	client_t *client;
	static char	n[1024];
	int	sep;

	n[0] = 0;

	sep = 0;

	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (strcmp(t, client->team)==0) 
		{
			if (sep >= 1)
				strlcat (n, "_", sizeof(n));
//				snprintf (n, sizeof(n), "%s_", n);
			strlcat (n, client->name, sizeof(n));
//			snprintf (n, sizeof(n),"%s%s", n, client->name);
			sep++;
		}
	}

	return n;
}

int	Dem_CountTeamPlayers (char *t)
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (svs.clients[i].name[0] && !svs.clients[i].spectator)
			if (strcmp(&svs.clients[i].team[0], t)==0)
				count++;
	}

	return count;
}

// <-

void SV_EasyRecord_f (void)
{
	int		c;
	dir_t	dir;
	char	name[1024];
	char	name2[MAX_OSPATH*7]; // scream
	//char	name2[MAX_OSPATH*2];
	int		i;
	FILE	*f;

	c = Cmd_Argc();
	if (c > 2)
	{
		Con_Printf ("easyrecord [demoname]\n");
		return;
	}

	if (sv.demorecording)
		SV_Stop_f();

	dir = Sys_listdir(va("%s/%s", com_gamedir,sv_demoDir.string), ".*", SORT_NO);
	if (sv_demoMaxDirSize.value && dir.size > sv_demoMaxDirSize.value*1024)
	{
		Con_Printf("insufficient directory space, increase sv_demoMaxDirSize\n");
		return;
	}

	// -> scream
	/*if (c == 2)
		strlcpy (name, Cmd_Argv(1), sizeof(name));
		
	else { 
		// guess game type and write demo name
		i = Dem_CountPlayers();
		if (teamplay.value && i > 2)
		{
			// Teamplay
			snprintf (name, sizeof(name), "team_%s_vs_%s_%s",
				Dem_Team(1),
				Dem_Team(2),
				sv.name);
		} else {
			if (i == 2) {
				// Duel
				snprintf (name, sizeof(name), "duel_%s_vs_%s_%s",
					Dem_PlayerName(1),
					Dem_PlayerName(2),
					sv.name);
			} else {
				// FFA
				snprintf (name, sizeof(name), "ffa_%s(%d)",
					sv.name,
					i);
			}
		}
	}

	*/
	if (c == 2)
		strlcpy (name, Cmd_Argv(1), sizeof(name));
	else
	{
		i = Dem_CountPlayers();
		if (teamplay.value >= 1 && i > 2)
		{
			// Teamplay
			snprintf (name, sizeof(name), "%don%d_", Dem_CountTeamPlayers(Dem_Team(1)), Dem_CountTeamPlayers(Dem_Team(2)));
			if (sv_demoExtraNames.value > 0)
			{
				strlcat (name, va("[%s]_%s_vs_[%s]_%s_%s", 
									Dem_Team(1), Dem_PlayerNameTeam(Dem_Team(1)), 
									Dem_Team(2), Dem_PlayerNameTeam(Dem_Team(2)),
									sv.name), sizeof(name));
			} else
				strlcat (name, va("%s_vs_%s_%s", Dem_Team(1), Dem_Team(2), sv.name), sizeof(name));
		} else {
			if (i == 2) {
				// Duel
				snprintf (name, sizeof(name), "duel_%s_vs_%s_%s",
					Dem_PlayerName(1),
					Dem_PlayerName(2),
					sv.name);
			} else {
				// FFA
				snprintf (name, sizeof(name), "ffa_%s(%d)", sv.name, i);
			}
		}
	}

	// <-

// Make sure the filename doesn't contain illegal characters
	strlcpy(name, va("%s%s", sv_demoPrefix.string, SV_CleanName(name)),
			MAX_DEMO_NAME - strlen(sv_demoSuffix.string) - 7);
	strlcat(name, sv_demoSuffix.string, sizeof(name));
	strlcpy(name, va("%s/%s/%s", com_gamedir, sv_demoDir.string, name), sizeof(name));
// find a filename that doesn't exist yet
	strlcpy(name2, name, sizeof(name2));
	Sys_mkdir(va("%s/%s", com_gamedir, sv_demoDir.string));
	COM_ForceExtension (name2, ".mvd");
	if ((f = fopen (name2, "rb")) == 0)
		f = fopen(va("%s.gz", name2), "rb");
	
	if (f) {
		i = 1;
		do {
			fclose (f);
			snprintf(name2, sizeof(name2), "%s_%02i", name, i);
			COM_ForceExtension (name2, ".mvd");
			if ((f = fopen (name2, "rb")) == 0)
				f = fopen(va("%s.gz", name2), "rb");
			i++;
		} while (f);
	}

	SV_Record (name2);
}

void SV_DemoList_f (void)
{
	dir_t	dir;
	file_t	*list;
	float	f;
	int		i,j,show;

	Con_Printf("content of %s/%s/*.mvd\n", com_gamedir,sv_demoDir.string);
	dir = Sys_listdir(va("%s/%s", com_gamedir,sv_demoDir.string), ".mvd", SORT_BY_DATE);
	list = dir.files;
	if (!list->name[0])
	{
		Con_Printf("no demos\n");
	}

	for (i = 1; list->name[0]; i++, list++)
	{
		for (j = 1; j < Cmd_Argc(); j++)
			if (strstr(list->name, Cmd_Argv(j)) == NULL)
				break;
		show = Cmd_Argc() == j;

		if (show) {
			if (sv.demorecording && !strcmp(list->name, demo.name))
				Con_Printf("*%d: %s %dk\n", i, list->name, demo.size/1024);
			else
				Con_Printf("%d: %s %dk\n", i, list->name, list->size/1024);
		}
	}

	if (sv.demorecording)
		dir.size += demo.size;

	Con_Printf("\ndirectory size: %.1fMB\n",(float)dir.size/(1024*1024));
	if (sv_demoMaxDirSize.value) {
		f = (sv_demoMaxDirSize.value*1024 - dir.size)/(1024*1024);
		if ( f < 0)
			f = 0;
		Con_Printf("space available: %.1fMB\n", f);
	}
}

char *SV_DemoNum(int num)
{
	file_t	*list;
	dir_t	dir;

	if (num <= 0)
		return NULL;

	dir = Sys_listdir(va("%s/%s", com_gamedir, sv_demoDir.string), ".mvd", SORT_BY_DATE);
	if (num > dir.numfiles)
		return NULL;

	list = dir.files;

	num--;

	while (list->name[0] && num) {list++; num--;};

	if (list->name[0]) 
		return list->name;

	return NULL;
}

char *SV_DemoName2Txt(char *name)
{
	char s[MAX_OSPATH];

	if (!name)
		return NULL;

	strlcpy(s, name, MAX_OSPATH);

	if (strstr(s, ".mvd.gz") != NULL)
		strlcpy(s + strlen(s) - 6, "txt", MAX_OSPATH - strlen(s) + 6);
	else
		strlcpy(s + strlen(s) - 3, "txt", MAX_OSPATH - strlen(s) + 3);

	return va("%s", s);
}

char *SV_DemoTxTNum(int num)
{
	return SV_DemoName2Txt(SV_DemoNum(num));
}

void SV_DemoRemove_f (void)
{
	char name[MAX_DEMO_NAME], *ptr;
	char path[MAX_OSPATH];
	int i;

	if (Cmd_Argc() != 2)
	{
		Con_Printf("rmdemo <demoname> - removes the demo\nrmdemo *<token>   - removes demo with <token> in the name\nrmdemo *          - removes all demos\n");
		return;
	}

	ptr = Cmd_Argv(1);
	if (*ptr == '*')
	{
		dir_t dir;
		file_t *list;

		// remove all demos with specified token
		ptr++;

		dir = Sys_listdir(va("%s/%s", com_gamedir, sv_demoDir.string), ".mvd", SORT_BY_DATE);
		list = dir.files;
		for (i = 0;list->name[0]; list++)
		{
			if (strstr(list->name, ptr))
			{
				if (sv.demorecording && !strcmp(list->name, demo.name))
					SV_Stop_f();

				// stop recording first;
				snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, list->name);
				if (!Sys_remove(path)) {
					Con_Printf("removing %s...\n", list->name);
					i++;
				}
		
				Sys_remove(SV_DemoName2Txt(path));
			}
		}

		if (i) {
			Con_Printf("%d demos removed\n", i);
		} else {
			Con_Printf("no matching found\n");
		}

		return;
	}

	strlcpy(name, Cmd_Argv(1), MAX_DEMO_NAME);
	COM_DefaultExtension(name, ".mvd");

	snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, name);

	if (sv.demorecording && !strcmp(name, demo.name))
		SV_Stop_f();

	if (!Sys_remove(path)) {
		Con_Printf("demo %s successfully removed\n", name);

		if (*sv_ondemoremove.string)
		{
			extern redirect_t sv_redirected;
			int old = sv_redirected;

			sv_redirected = RD_NONE; // this script is called always from the console
			Cmd_TokenizeString(va("script %s \"%s\" \"%s\"", sv_ondemoremove.string, sv_demoDir.string, name));
			SV_Script_f();

			sv_redirected = old;
		}
	} else 
		Con_Printf("unable to remove demo %s\n", name);

	Sys_remove(SV_DemoName2Txt(path));
}

void SV_DemoRemoveNum_f (void)
{
	int		num;
	char	*val, *name;
	char path[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf("rmdemonum <#>\n");
		return;
	}

	val = Cmd_Argv(1);
	if ((num = atoi(val)) == 0 && val[0] != '0')
	{
		Con_Printf("rmdemonum <#>\n");
		return;
	}

	name = SV_DemoNum(num);

	if (name != NULL) {
		if (sv.demorecording && !strcmp(name, demo.name))
			SV_Stop_f();

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, name);
		if (!Sys_remove(path)) {
			Con_Printf("demo %s succesfully removed\n", name);
			if (*sv_ondemoremove.string)
			{
				extern redirect_t sv_redirected;
				int old = sv_redirected;

				sv_redirected = RD_NONE; // this script is called always from the console
				Cmd_TokenizeString(va("script %s \"%s\" \"%s\"", sv_ondemoremove.string, sv_demoDir.string, name));
				SV_Script_f();

				sv_redirected = old;
			}
		} else 
			Con_Printf("unable to remove demo %s\n", name);

		Sys_remove(SV_DemoName2Txt(path));
	} else
		Con_Printf("invalid demo num\n");
}

void SV_DemoInfoAdd_f (void)
{
	char *name, *args, path[MAX_OSPATH];
	FILE *f;

	if (Cmd_Argc() < 3) {
		Con_Printf("usage:demoInfoAdd <demonum> <info string>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*"))
	{
		if (!sv.demorecording) {
			Con_Printf("Not recording demo!\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, demo.path, SV_DemoName2Txt(demo.name));
	} else {
		name = SV_DemoTxTNum(atoi(Cmd_Argv(1)));

		if (!name) {
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, name);
	}

	if ((f = fopen(path, "a+t")) == NULL)
	{
		Con_Printf("faild to open the file\n");
		return;
	}

	// skip demonum
	args = Cmd_Args();
	while (*args > 32) args++;
	while (*args && *args <= 32) args++;

	fwrite(args, strlen(args), 1, f);
	fwrite("\n", 1, 1, f);
	fflush(f);
	fclose(f);
}

void SV_DemoInfoRemove_f (void)
{
	char *name, path[MAX_OSPATH];

	if (Cmd_Argc() < 2) {
		Con_Printf("usage:demoInfoRemove <demonum>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*"))
	{
		if (!sv.demorecording) {
			Con_Printf("Not recording demo!\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, demo.path, SV_DemoName2Txt(demo.name));
	} else {
		name = SV_DemoTxTNum(atoi(Cmd_Argv(1)));

		if (!name) {
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, name);
	}

	if (Sys_remove(path))
		Con_Printf("failed to remove the file\n");
	else Con_Printf("file removed\n");
}

void SV_DemoInfo_f (void)
{
	char buf[512];
	FILE *f = NULL;
	char *name, path[MAX_OSPATH];

	if (Cmd_Argc() < 2) {
		Con_Printf("usage: demoinfo <demonum>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*"))
	{
		if (!sv.demorecording) {
			Con_Printf("Not recording demo!\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, demo.path, SV_DemoName2Txt(demo.name));
	} else {
		name = SV_DemoTxTNum(atoi(Cmd_Argv(1)));

		if (!name) {
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, name);
	}

	if ((f = fopen(path, "rt")) == NULL)
	{
		Con_Printf("(empty)\n");
		return;
	}

	while (!feof(f))
	{
		buf[fread (buf, 1, sizeof(buf) - 1, f)] = 0;
		for (name = buf; *name; name++)
		{
			if ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z'))
				*name |= 128;
			else if (*name >= '0' && *name <= '9')
				*name += 18 - '0';
		}
		Con_Printf("%s", buf);
	}

	fclose(f);
}

#define MAXDEMOS	10
void SV_LastScores_f (void)
{
	int	demos, i;
	char	buf[512];
	FILE	*f = NULL;
	char	*name, path[MAX_OSPATH];
	dir_t	dir;

	if (Cmd_Argc() > 2) {
		Con_Printf("usage: lastscores [<numlastdemos>]\n<numlastdemos> = '0' for all demos\n<numlastdemos> = '' for last %i demos\n", MAXDEMOS);
		return;
	}

	if (Cmd_Argc() == 1)
		demos = MAXDEMOS;
	else
		demos = atoi(Cmd_Argv(1));

	if (sv.demorecording && demos > MAXDEMOS)
		Con_Printf("<numlastdemos> was decreased to %i: demo recording in progress.\n", demos = MAXDEMOS);

	dir = Sys_listdir(va("%s/%s", com_gamedir, sv_demoDir.string), ".txt", SORT_BY_DATE);

	if (!dir.numfiles)
	{
		Con_Printf("No txt files with info.\n");
		return;
	}
	if (dir.numfiles < demos || !demos)
		demos = dir.numfiles;

	for (i = dir.numfiles - demos; i < dir.numfiles; i++)
	{
		snprintf(path, MAX_OSPATH, "%s/%s/%s", com_gamedir, sv_demoDir.string, dir.files[i].name);

		Con_Printf("%i. ", i);
		if ((f = fopen(path, "rt")) == NULL)
		{
			Con_Printf("(empty)\n");
			continue;
		}

		while (!feof(f))
		{
			buf[fread (buf, 1, sizeof(buf) - 1, f)] = 0;
			for (name = buf; *name; name++)
			{
				if (*name == '\n')
				{
					name[1] = 0;
					break;
				}
				else if ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z'))
					*name |= 128;
				else if (*name >= '0' && *name <= '9')
					*name += 18 - '0';
			}
			Con_Printf("%s", buf);
		}
		fclose(f);
	}
}
