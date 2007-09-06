/*
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

    $Id$
*/

// sv_demo.c - mvd demo related code

#include "qwsvdef.h"

qbool	sv_demoDir_OnChange(cvar_t *cvar, const char *value);

cvar_t	sv_demoUseCache		= {"sv_demoUseCache",	"0"};
cvar_t	sv_demoCacheSize	= {"sv_demoCacheSize",	"0", CVAR_ROM};
cvar_t	sv_demoMaxDirSize	= {"sv_demoMaxDirSize",	"102400"};
cvar_t	sv_demoClearOld		= {"sv_demoClearOld",	"0"};
cvar_t	sv_demoDir			= {"sv_demoDir",		"demos", 0, sv_demoDir_OnChange};
cvar_t	sv_demofps			= {"sv_demofps",		"30"};
cvar_t	sv_demoIdlefps		= {"sv_demoIdlefps",	"10"};
cvar_t	sv_demoPings		= {"sv_demopings",		"3"};
cvar_t	sv_demoNoVis		= {"sv_demonovis",		"1"};
cvar_t	sv_demoMaxSize		= {"sv_demoMaxSize",	"20480"};
cvar_t	sv_demoExtraNames	= {"sv_demoExtraNames", "0"};

cvar_t	sv_demoPrefix		= {"sv_demoPrefix",		""};
cvar_t	sv_demoSuffix		= {"sv_demoSuffix",		""};
cvar_t	sv_demotxt			= {"sv_demotxt",		"1"};
cvar_t	sv_onrecordfinish	= {"sv_onRecordFinish", ""};

cvar_t	sv_ondemoremove		= {"sv_onDemoRemove",	""};
cvar_t	sv_demoRegexp		= {"sv_demoRegexp",		"\\.mvd(\\.(gz|bz2|rar|zip))?$"};


static dbuffer_t	*demobuffer;
static int 			header = (char *)&((header_t*)0)->data - (char *)NULL;


// last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
char 				*lastdemosname[16];
int  				lastdemospos;

mvddest_t			*singledest;

entity_state_t		demo_entities[UPDATE_MASK+1][MAX_DEMO_PACKET_ENTITIES];
client_frame_t		demo_frames[UPDATE_MASK+1];


#define MAXSIZE (demobuffer->end < demobuffer->last ? \
				demobuffer->start - demobuffer->end : \
				demobuffer->maxsize - demobuffer->end)


mvddest_t *DestByName (char *name)
{
	mvddest_t *d;

	for (d = demo.dest; d; d = d->nextdest)
		if (!strncmp(name, d->name, sizeof(d->name)-1))
			return d;

	return NULL;
}

static void DestClose (mvddest_t *d, qbool destroyfiles)
{
	char path[MAX_OSPATH];

	if (d->cache)
		Q_free(d->cache);
	if (d->file)
		fclose(d->file);
	if (d->socket)
		closesocket(d->socket);

	if (destroyfiles)
	{
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, d->path, d->name);
		Sys_remove(path);
		strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);
		Sys_remove(path);
	}

	Q_free(d);
}

#define demo_size_padding 0x1000

void DestFlush (qbool compleate)
{
	int len;
	mvddest_t *d, *t;

	if (!demo.dest)
		return;

	while (demo.dest->error)
	{
		d = demo.dest;
		demo.dest = d->nextdest;

		DestClose(d, false);

		if (!demo.dest)
		{
			SV_MVDStop(3, false);
			return;
		}
	}

	for (d = demo.dest; d; d = d->nextdest)
	{
		switch(d->desttype)
		{
		case DEST_FILE:
			fflush (d->file);
			break;

		case DEST_BUFFEREDFILE:
			if (d->cacheused + demo_size_padding > d->maxcachesize || compleate)
			{
				len = fwrite(d->cache, 1, d->cacheused, d->file);
				if (len < d->cacheused)
					d->error = true;
				fflush(d->file);

				d->cacheused = 0;
			}
			break;

		case DEST_STREAM:
			if (d->io_time + qtv_streamtimeout.value <= Sys_DoubleTime())
			{
				// problem what send() have internal buffer, so send() success some time even peer side does't read,
				// this may take some time before internal buffer overflow and timeout trigger, depends of buffer size.
				Con_Printf("Stream timeout\n");
				d->error = true;
			}

			if (d->cacheused && !d->error)
			{
				len = send(d->socket, d->cache, d->cacheused, 0);

				if (len == 0) //client died
				{
//					d->error = true;
					// man says: The calls return the number of characters sent, or -1 if an error occurred.   
					// so 0 is legal or what?
				}
				else if (len > 0) //we put some data through
				{ //move up the buffer
					d->cacheused -= len;
					memmove(d->cache, d->cache+len, d->cacheused);

					d->io_time = Sys_DoubleTime(); // update IO activity
				}
				else
				{ //error of some kind. would block or something
					if (qerrno != EWOULDBLOCK && qerrno != EAGAIN)
						d->error = true;
				}
			}
			break;

		case DEST_NONE:
		default:
			Sys_Error("DestFlush encoundered bad dest.");
		}

		if (d->desttype != DEST_STREAM) // no max size for stream
			if ((unsigned int)sv_demoMaxSize.value && d->totalsize > ((unsigned int)sv_demoMaxSize.value * 1024))
				d->error = 2;	//abort, but don't kill it.

		while (d->nextdest && d->nextdest->error)
		{
			t = d->nextdest;
			d->nextdest = t->nextdest;

			DestClose(t, false);
		}
	}
}

// if param "mvdonly" == true then close only demos, not QTV's steams
static int DestCloseAllFlush (qbool destroyfiles, qbool mvdonly)
{
	int numclosed = 0;
	mvddest_t *d, **prev, *next;
	DestFlush(true); //make sure it's all written.

	prev = &demo.dest;
	d = demo.dest;
	while (d)
	{
		next = d->nextdest;
		if (!mvdonly || d->desttype != DEST_STREAM)
		{
			*prev = d->nextdest;
			DestClose(d, destroyfiles);
			Run_sv_demotxt_and_sv_onrecordfinish (d, destroyfiles);
			numclosed++;
		}
		else
			prev = &d->nextdest;

		d = next;
	}

	return numclosed;
}

int DemoWriteDest (void *data, int len, mvddest_t *d)
{
	if (d->error)
		return 0;
	d->totalsize += len;
	switch(d->desttype)
	{
		case DEST_FILE:
			fwrite(data, len, 1, d->file);
			break;
		case DEST_BUFFEREDFILE:	//these write to a cache, which is flushed later
		case DEST_STREAM:
			if (d->cacheused+len > d->maxcachesize)
			{
				d->error = true;
				return 0;
			}
			memcpy(d->cache+d->cacheused, data, len);
			d->cacheused += len;
			break;
		case DEST_NONE:
		default:
			Sys_Error("DemoWriteDest encoundered bad dest.");
	}
	return len;
}

int DemoWrite (void *data, int len) //broadcast to all proxies/mvds
{
	mvddest_t *d;
	for (d = demo.dest; d; d = d->nextdest)
	{
		if (singledest && singledest != d)
			continue;
		DemoWriteDest(data, len, d);
	}
	return len;
}

void MVDBuffer_Init (dbuffer_t *dbuffer, byte *buf, size_t size)
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
MVDSetMsgBuf

Sets the frame message buffer
==============
*/

void MVDSetMsgBuf (demobuf_t *prev,demobuf_t *cur)
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
MVDSetBuf

Sets position in the buf for writing to specific client
==============
*/
static void MVDSetBuf (byte type, int to)
{
	header_t *p;
	int pos = 0;

	p = (header_t *)demo.dbuf->data;

	while (pos < demo.dbuf->bufsize)
	{
		pos += header + p->size;

		if (type == p->type && to == p->to && !p->full)
		{
			demo.dbuf->cursize = pos;
			demo.dbuf->h = p;
			return;
		}

		p = (header_t *)(p->data + p->size);
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

void MVDMoveBuf (void)
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

void MVDWrite_Begin (byte type, int to, int size)
{
	byte *p;
	qbool move = false;

	// will it fit?
	while (demo.dbuf->bufsize + size + header > demo.dbuf->maxsize)
	{
		// if we reached the end of buffer move msgbuf to the begining
		if (!move && demobuffer->end > demobuffer->start)
			move = true;

		SV_MVDWritePackets(1);
		if (move && demobuffer->start > demo.dbuf->bufsize + header + size)
			MVDMoveBuf();
	}

	if (demo.dbuf->h == NULL || demo.dbuf->h->type != type || demo.dbuf->h->to != to || demo.dbuf->h->full)
	{
		MVDSetBuf(type, to);
	}

	if (demo.dbuf->h->size + size > MAX_MSGLEN)
	{
		demo.dbuf->h->full = 1;
		MVDSetBuf(type, to);
	}

	// we have to make room for new data
	if (demo.dbuf->cursize != demo.dbuf->bufsize)
	{
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
SV_WriteMVDMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
static void SV_WriteMVDMessage (sizebuf_t *msg, int type, int to, float time1)
{
	int		len, i, msec;
	byte	c;
	static double prevtime;

	if (!sv.mvdrecording)
		return;

	msec = (time1 - prevtime) * 1000;
	prevtime += msec * 0.001;

	if (msec > 255)
		msec = 255;
	if (msec < 2)
		msec = 0;

	c = msec;
	DemoWrite(&c, sizeof(c));

	if (demo.lasttype != type || demo.lastto != to)
	{
		demo.lasttype = type;
		demo.lastto = to;
		switch (demo.lasttype)
		{
		case dem_all:
			c = dem_all;
			DemoWrite (&c, sizeof(c));
			break;
		case dem_multiple:
			c = dem_multiple;
			DemoWrite (&c, sizeof(c));

			i = LittleLong(demo.lastto);
			DemoWrite (&i, sizeof(i));
			break;
		case dem_single:
		case dem_stats:
			c = demo.lasttype + (demo.lastto << 3);
			DemoWrite (&c, sizeof(c));
			break;
		default:
			SV_MVDStop_f ();
			Con_Printf("bad demo message type:%d", type);
			return;
		}
	}
	else
	{
		c = dem_read;
		DemoWrite (&c, sizeof(c));
	}

	len = LittleLong (msg->cursize);
	DemoWrite (&len, 4);
	DemoWrite (msg->data, msg->cursize);

	DestFlush(false);
}

/*
==============
SV_MVDWriteToDisk

Writes to disk a message meant for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

void SV_MVDWriteToDisk (int type, int to, float time1)
{
	int pos = 0, oldm, oldd;
	header_t *p;
	int size;
	sizebuf_t msg;

	p = (header_t *)demo.dbuf->data;
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
			if (size)
			{
				msg.data = p->data;
				msg.cursize = size;

				SV_WriteMVDMessage(&msg, p->type, p->to, time1);
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
		p = (header_t *)(p->data + size);
	}

	if (demobuffer->start == demobuffer->last)
	{
		if (demobuffer->start == demobuffer->end)
		{
			demobuffer->end = 0; // demobuffer is empty
			demo.dbuf->data = demobuffer->data;
		}

		// go back to begining of the buffer
		demobuffer->last = demobuffer->end;
		demobuffer->start = 0;
	}
}

/*
====================
SV_MVDWritePackets

Interpolates to get exact players position for current frame
and writes packets to the disk/memory
====================
*/

void SV_MVDWritePackets (int num)
{
	demo_frame_t	*frame, *nextframe;
	demo_client_t	*cl, *nextcl = NULL;
	int				i, j, flags;
	qbool			valid;
	double			time1, playertime, nexttime;
	float			f;
	vec3_t			origin, angles;
	sizebuf_t		msg;
	byte			msg_buf[MAX_MSGLEN];
	demoinfo_t		*demoinfo;

	if (!sv.mvdrecording)
		return;

	msg.data = msg_buf;
	msg.maxsize = sizeof(msg_buf);

	if (num > demo.parsecount - demo.lastwritten + 1)
		num = demo.parsecount - demo.lastwritten + 1;

	// 'num' frames to write
	for ( ; num; num--, demo.lastwritten++)
	{
		frame = &demo.frames[demo.lastwritten&DEMO_FRAMES_MASK];
		time1 = frame->time;
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

			if (!cl->parsecount && svs.clients[i].state != cs_spawned)
				continue; // not valid, occur on first frame

			nexttime = playertime = time1 - cl->sec;

			for (j = demo.lastwritten+1, valid = false; nexttime < time1 && j < demo.parsecount; j++)
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

				if (nexttime >= time1)
				{
					// good, found what we were looking for
					valid = true;
					break;
				}
			}

			if (valid)
			{
				f = (time1 - nexttime)/(nexttime - playertime);
				for (j=0;j<3;j++)
				{
					angles[j] = adjustangle(cl->info.angles[j], nextcl->info.angles[j],1.0+f);
					origin[j] = nextcl->info.origin[j] + f*(nextcl->info.origin[j]-cl->info.origin[j]);
				}
			}
			else
			{
				VectorCopy(cl->info.origin, origin);
				VectorCopy(cl->info.angles, angles);
			}

			// now write it to buf
			flags = cl->flags;

			if (cl->fixangle)
			{
				demo.fixangletime[i] = cl->cmdtime;
			}

			for (j=0; j < 3; j++)
				if (origin[j] != demoinfo->origin[j])
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

		SV_MVDWriteToDisk(demo.lasttype,demo.lastto, (float)time1); // this goes first to reduce demo size a bit
		SV_MVDWriteToDisk(0,0, (float)time1); // now goes the rest
		if (msg.cursize)
			SV_WriteMVDMessage(&msg, dem_all, 0, (float)time1);
	}

	if (demo.lastwritten > demo.parsecount)
		demo.lastwritten = demo.parsecount;

	demo.dbuf = &demo.frames[demo.parsecount&DEMO_FRAMES_MASK].buf;
	demo.dbuf->maxsize = MAXSIZE + demo.dbuf->bufsize;
}

/*
====================
SV_InitRecord
====================
*/
static mvddest_t *SV_InitRecordFile (char *name)
{
	char *s;
	mvddest_t *dst;
	FILE *file;

	char path[MAX_OSPATH];

	Con_DPrintf("SV_InitRecordFile: Demo name: \"%s\"\n", name);
	file = fopen (name, "wb");
	if (!file)
	{
		Con_Printf ("ERROR: couldn't open \"%s\"\n", name);
		return NULL;
	}

	dst = (mvddest_t*) Q_malloc (sizeof(mvddest_t));

	if (!(int)sv_demoUseCache.value)
	{
		dst->desttype = DEST_FILE;
		dst->file = file;
		dst->maxcachesize = 0;
	}
	else
	{
		dst->desttype = DEST_BUFFEREDFILE;
		dst->file = file;
		dst->maxcachesize = (int) sv_demoCacheSize.value; // 0x81000
		dst->cache = (char *) Q_malloc (dst->maxcachesize);
	}

	s = name + strlen(name);
	while (*s != '/') s--;
	strlcpy(dst->name, s+1, sizeof(dst->name));
	strlcpy(dst->path, sv_demoDir.string, sizeof(dst->path));

	SV_BroadcastPrintf (PRINT_CHAT, "Server starts recording (%s):\n%s\n",
						(dst->desttype == DEST_BUFFEREDFILE) ? "memory" : "disk", s+1);
	Cvar_SetROM(&serverdemo, dst->name);

	strlcpy(path, name, MAX_OSPATH);
	strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);

	if ((int)sv_demotxt.value)
	{
		FILE *f;
		char *text;

		if (sv_demotxt.value == 2)
		{
			if ((f = fopen (path, "a+t")))
				fclose(f); // at least made empty file
		}
		else if ((f = fopen (path, "w+t")))
		{
			text = SV_PrintTeams();
			fwrite(text, strlen(text), 1, f);
			fflush(f);
			fclose(f);
		}
	}
	else
		Sys_remove(path);

	return dst;
}

/*
====================
SV_MVDStop

stop recording a demo
====================
*/
void SV_MVDStop (int reason, qbool mvdonly)
{
	size_t name_len;
	int numclosed;

	if (!sv.mvdrecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}

	if (reason == 2 || reason == 3)
	{
		DestCloseAllFlush(true, mvdonly);
		// stop and remove

		if (!demo.dest)
			sv.mvdrecording = false;

		if (reason == 3)
			SV_BroadcastPrintf (PRINT_CHAT, "QTV disconnected\n");
		else
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording canceled, demo removed\n");

		Cvar_SetROM(&serverdemo, "");

		return;
	}
	
	// write a disconnect message to the demo file
	// clearup to be sure message will fit
	demo.dbuf->cursize = 0;
	demo.dbuf->h = NULL;
	demo.dbuf->bufsize = 0;
	MVDWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_disconnect);
	MSG_WriteString ((sizebuf_t*)demo.dbuf, "EndOfDemo");

	SV_MVDWritePackets(demo.parsecount - demo.lastwritten + 1);
	// finish up

	// last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
	name_len = strlen(demo.dest->name) + 1;
	if (++lastdemospos > 15)
		lastdemospos &= 0xF;
	if (lastdemosname[lastdemospos])
		Q_free(lastdemosname[lastdemospos]);
	lastdemosname[lastdemospos] = (char *) Q_malloc(name_len);
	strlcpy(lastdemosname[lastdemospos], demo.dest->name, name_len);
	Con_DPrintf("SV_MVDStop: Demo name for 'cmd dl .': \"%s\"\n", lastdemosname[lastdemospos]);

	numclosed = DestCloseAllFlush(false, mvdonly);

	if (!demo.dest)
		sv.mvdrecording = false;

	if (numclosed)
	{
		if (!reason)
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording completed\n");
		else
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording stoped\nMax demo size exceeded\n");
	}

	Cvar_SetROM(&serverdemo, "");
}

/*
====================
SV_MVDStop_f
====================
*/
void SV_MVDStop_f (void)
{
	SV_MVDStop(0, true);
}

/*
====================
SV_MVD_Cancel_f

Stops recording, and removes the demo
====================
*/
void SV_MVD_Cancel_f (void)
{
	SV_MVDStop(2, true);
}

/*
====================
SV_WriteMVDMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
static void SV_WriteRecordMVDMessage (sizebuf_t *msg, int seq)
{
	int len;
	byte c;

	if (!sv.mvdrecording)
		return;

	if (!msg->cursize)
		return;

	c = 0;
	DemoWrite (&c, sizeof(c));

	c = dem_read;
	DemoWrite (&c, sizeof(c));

	len = LittleLong (msg->cursize);
	DemoWrite (&len, 4);

	DemoWrite (msg->data, msg->cursize);

	DestFlush(false);
}

static void SV_WriteSetMVDMessage (void)
{
	int len;
	byte c;

	//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!sv.mvdrecording)
		return;

	c = 0;
	DemoWrite (&c, sizeof(c));

	c = dem_set;
	DemoWrite (&c, sizeof(c));


	len = LittleLong(0);
	DemoWrite (&len, 4);
	len = LittleLong(0);
	DemoWrite (&len, 4);

	DestFlush(false);
}

// mvd/qtv related stuff
// Well, here is a chance what player connect after demo recording started,
// so demo.info[edictnum - 1].model == player_model so SV_MVDWritePackets() will not wrote player model index,
// so client during playback this demo will got invisible model, because model index will be 0.
// Fixing that.
// Btw, struct demo contain different client specific structs, may be they need clearing too, not sure.
void MVD_PlayerReset(int player)
{
	if (player < 0 || player >= MAX_CLIENTS) { // protect from lamers
		Con_Printf("MVD_PlayerReset: wrong player num %d\n", player);
		return;
	}

	memset(&(demo.info[player]), 0, sizeof(demo.info[0]));
}

qbool SV_MVD_Record (mvddest_t *dest)
{
	int i;
	qbool map_change = (dest == demo.dest); // some magical guessing is this is a map change

	if (!dest)
		return false;

	DestFlush(true);

	if (map_change && !demo.dest)
		return false; // seems we close all dests with DestFlush() few line above, so in mapchange case nothing to do here

	if (!sv.mvdrecording)
	{
		// this is either mapchange and we have QTV connected
		// or we just use /record or whatever command first time and here no recording yet

    	// and here we memset() not whole demo_t struct, but part,
    	// so demo.dest and demo.pendingdest is not overwriten
		memset(&demo, 0, ((int)&(((demo_t *)0)->mem_set_point)));

		for (i = 0; i < UPDATE_BACKUP; i++)
		{
			demo.recorder.frames[i].entities.entities = demo_entities[i];
		}

		MVDBuffer_Init(&demo.dbuffer, demo.buffer, sizeof(demo.buffer));
		MVDSetMsgBuf(NULL, &demo.frames[0].buf);

		demo.datagram.maxsize = sizeof(demo.datagram_data);
		demo.datagram.data = demo.datagram_data;
	}

	if (map_change)
	{
		//
		// map change, sent initial stats to all dests
		//
		SV_MVD_SendInitialGamestate(NULL);
	}
	else
	{
		//
		// seems we initializing new dest, sent initial stats only to this dest
		//
		dest->nextdest = demo.dest;
		demo.dest = dest;

		SV_MVD_SendInitialGamestate(dest);
	}

	// done
	return true;
}

// we change map - clear whole demo struct and sent initial state to all dest if any (for QTV only I thought)
qbool SV_MVD_Re_Record(void)
{
	return SV_MVD_Record (demo.dest);
}

void SV_MVD_SendInitialGamestate(mvddest_t *dest)
{
//	qbool first_dest = !sv.mvdrecording; // if we are not recording yet, that must be first dest
	sizebuf_t	buf;
	unsigned char buf_data[MAX_MSGLEN];
	unsigned int n;
	char *s, info[MAX_INFO_STRING];

	client_t *player;
	char *gamedir;
	int seq = 1, i;

	if (!demo.dest)
		return;

	sv.mvdrecording = true; // NOTE:  afaik set to false on map change, so restore it here
	
	
	demo.pingtime = demo.time = sv.time;


	singledest = dest;

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
	MSG_WriteString (&buf,
#ifdef USE_PR2
	                 PR2_GetString
#else
					 PR_GetString
#endif
	                (sv.edicts->v.message));

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
	SV_WriteRecordMVDMessage (&buf, seq++);
	SZ_Clear (&buf);

	// soundlist
	MSG_WriteByte (&buf, svc_soundlist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.sound_precache[n+1];
	while (s)
	{
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2)
		{
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
			MSG_WriteByte (&buf, svc_soundlist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.sound_precache[n+1];
	}

	if (buf.cursize)
	{
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// modellist
	MSG_WriteByte (&buf, svc_modellist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.model_precache[n+1];
	while (s)
	{
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2)
		{
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
			MSG_WriteByte (&buf, svc_modellist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.model_precache[n+1];
	}
	if (buf.cursize)
	{
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// prespawn

	for (n = 0; n < sv.num_signon_buffers; n++)
	{
		if (buf.cursize+sv.signon_buffer_size[n] > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
		}
		SZ_Write (&buf,
		          sv.signon_buffers[n],
		          sv.signon_buffer_size[n]);
	}

	if (buf.cursize > MAX_MSGLEN/2)
	{
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("cmd spawn %i 0\n",svs.spawncount) );

	if (buf.cursize)
	{
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// send current status of all other players

	for (i = 0; i < MAX_CLIENTS; i++)
	{
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

		if (buf.cursize > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
		}
	}

	// that need only if that non first dest, demo code suppose we alredy have this, and do not send
	// this set proper model origin and angles etc for players
	for (i = 0; i < MAX_CLIENTS /* && !first_dest */ ; i++)
	{
		vec3_t origin, angles;
		edict_t *ent;
		int j, flags;

		player = svs.clients + i;
		ent = player->edict;

		if (player->state != cs_spawned)
			continue;

		flags =   (DF_ORIGIN << 0) | (DF_ORIGIN << 1) | (DF_ORIGIN << 2)
				| (DF_ANGLES << 0) | (DF_ANGLES << 1) | (DF_ANGLES << 2)
				| DF_EFFECTS | DF_SKINNUM 
				| (ent->v.health <= 0 ? DF_DEAD : 0)
				| (ent->v.mins[2] != -24 ? DF_GIB : 0)
				| DF_WEAPONFRAME | DF_MODEL;

		VectorCopy(ent->v.origin, origin);
		VectorCopy(ent->v.angles, angles);
		angles[0] *= -3;
#ifdef USE_PR2
		if( player->isBot )
			VectorCopy(ent->v.v_angle, angles);
#endif
		angles[2] = 0; // no roll angle

		if (ent->v.health <= 0)
		{	// don't show the corpse looking around...
			angles[0] = 0;
			angles[1] = ent->v.angles[1];
			angles[2] = 0;
		}

		MSG_WriteByte (&buf, svc_playerinfo);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, flags);

		MSG_WriteByte (&buf, ent->v.frame);

		for (j = 0 ; j < 3 ; j++)
			if (flags & (DF_ORIGIN << j))
				MSG_WriteCoord (&buf, origin[j]);

		for (j = 0 ; j < 3 ; j++)
			if (flags & (DF_ANGLES << j))
				MSG_WriteAngle16 (&buf, angles[j]);

		if (flags & DF_MODEL)
			MSG_WriteByte (&buf, ent->v.modelindex);

		if (flags & DF_SKINNUM)
			MSG_WriteByte (&buf, ent->v.skin);

		if (flags & DF_EFFECTS)
			MSG_WriteByte (&buf, ent->v.effects);

		if (flags & DF_WEAPONFRAME)
			MSG_WriteByte (&buf, ent->v.weaponframe);

		if (buf.cursize > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
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
	MSG_WriteString (&buf, "skins\n");

	SV_WriteRecordMVDMessage (&buf, seq++);

	SV_WriteSetMVDMessage();

	singledest = NULL;
}

/*
====================
SV_MVD_Record_f

record <demoname>
====================
*/
static void SV_MVD_Record_f (void)
{
	int c;
	char name[MAX_OSPATH+MAX_DEMO_NAME];
	char newname[MAX_DEMO_NAME];

	c = Cmd_Argc();
	if (c != 2)
	{
		Con_Printf ("record <demoname>\n");
		return;
	}

	if (sv.state != ss_active)
	{
		Con_Printf ("Not active yet.\n");
		return;
	}

	// clear old demos
	if (!SV_DirSizeCheck())
		return;

	strlcpy(newname, va("%s%s%s", sv_demoPrefix.string, SV_CleanName((unsigned char*)Cmd_Argv(1)),
						sv_demoSuffix.string), sizeof(newname) - 4);

	Sys_mkdir(va("%s/%s", fs_gamedir, sv_demoDir.string));

	snprintf (name, sizeof(name), "%s/%s/%s", fs_gamedir, sv_demoDir.string, newname);

	if ((c = strlen(name)) > 3)
		if (strcmp(name + c - 4, ".mvd"))
			strlcat(name, ".mvd", sizeof(name));

	//
	// open the demo file and start recording
	//
	SV_MVD_Record (SV_InitRecordFile(name));
}

/*
====================
SV_MVDEasyRecord_f

easyrecord [demoname]
====================
*/

static void SV_MVDEasyRecord_f (void)
{
	int		c;
	char	name[MAX_DEMO_NAME];
	char	name2[MAX_OSPATH*7]; // scream
	//char	name2[MAX_OSPATH*2];
	int		i;
	dir_t	dir;
	char	*name3;

	c = Cmd_Argc();
	if (c > 2)
	{
		Con_Printf ("easyrecord [demoname]\n");
		return;
	}

	if (!SV_DirSizeCheck()) // clear old demos
		return;

	if (c == 2)
		strlcpy (name, Cmd_Argv(1), sizeof(name));
	else
	{
		i = Dem_CountPlayers();
		if ((int)teamplay.value >= 1 && i > 2)
		{
			// Teamplay
			snprintf (name, sizeof(name), "%don%d_", Dem_CountTeamPlayers(Dem_Team(1)), Dem_CountTeamPlayers(Dem_Team(2)));
			if ((int)sv_demoExtraNames.value > 0)
			{
				strlcat (name, va("[%s]_%s_vs_[%s]_%s_%s",
				                  Dem_Team(1), Dem_PlayerNameTeam(Dem_Team(1)),
				                  Dem_Team(2), Dem_PlayerNameTeam(Dem_Team(2)),
				                  sv.mapname), sizeof(name));
			}
			else
				strlcat (name, va("%s_vs_%s_%s", Dem_Team(1), Dem_Team(2), sv.mapname), sizeof(name));
		}
		else
		{
			if (i == 2)
			{
				// Duel
				snprintf (name, sizeof(name), "duel_%s_vs_%s_%s",
				          Dem_PlayerName(1),
				          Dem_PlayerName(2),
				          sv.mapname);
			}
			else
			{
				// FFA
				snprintf (name, sizeof(name), "ffa_%s(%d)", sv.mapname, i);
			}
		}
	}

	// <-

	// Make sure the filename doesn't contain illegal characters
	strlcpy(name, va("%s%s%s", sv_demoPrefix.string,
			 SV_CleanName((unsigned char*)name), sv_demoSuffix.string), MAX_DEMO_NAME);
	strlcpy(name2, name, sizeof(name2));
	Sys_mkdir(va("%s/%s", fs_gamedir, sv_demoDir.string));

	// FIXME: very SLOW

	if (!(name3 = quote(name2)))
		return;
	dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string),
					  va("^%s%s", name3, sv_demoRegexp.string), SORT_NO);
	Q_free(name3);
	for (i = 1; dir.numfiles; )
	{
		snprintf(name2, sizeof(name2), "%s_%02i", name, i++);
		if (!(name3 = quote(name2)))
			return;
		dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string),
						  va("^%s%s", name3, sv_demoRegexp.string), SORT_NO);
		Q_free(name3);
	}

	snprintf(name2, sizeof(name2), va("%s/%s/%s.mvd", fs_gamedir, sv_demoDir.string, name2));

	SV_MVD_Record (SV_InitRecordFile(name2));
}

//============================================================

#define MIN_DEMO_MEMORY 0x100000

static void MVD_Init (void)
{
	int p, size = MIN_DEMO_MEMORY;

	memset(&demo, 0, sizeof(demo)); // clear whole demo struct at least once
	
	Cvar_Register (&sv_demofps);
	Cvar_Register (&sv_demoIdlefps);
	Cvar_Register (&sv_demoPings);
	Cvar_Register (&sv_demoNoVis);
	Cvar_Register (&sv_demoUseCache);
	Cvar_Register (&sv_demoCacheSize);
	Cvar_Register (&sv_demoMaxSize);
	Cvar_Register (&sv_demoMaxDirSize);
	Cvar_Register (&sv_demoClearOld); //bliP: 24/9 clear old demos
	Cvar_Register (&sv_demoDir);
	Cvar_Register (&sv_demoPrefix);
	Cvar_Register (&sv_demoSuffix);
	Cvar_Register (&sv_onrecordfinish);
	Cvar_Register (&sv_ondemoremove);
	Cvar_Register (&sv_demotxt);
	Cvar_Register (&sv_demoExtraNames);
	Cvar_Register (&sv_demoRegexp);

	p = COM_CheckParm ("-democache");
	if (p)
	{
		if (p < com_argc-1)
			size = Q_atoi (com_argv[p+1]) * 1024;
		else
			Sys_Error ("MVD_Init: you must specify a size in KB after -democache");
	}

	if (size < MIN_DEMO_MEMORY)
	{
		Con_Printf("Minimum memory size for demo cache is %dk\n", MIN_DEMO_MEMORY / 1024);
		size = MIN_DEMO_MEMORY;
	}

	Cvar_SetROM(&sv_demoCacheSize, va("%d", size/1024));
	CleanName_Init();

	// clean last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
	for (p = 0; p < 16; p++)
		lastdemosname[p] = NULL;
	lastdemospos = 0;
}

void SV_MVDInit (void)
{
	MVD_Init();

	Cmd_AddCommand ("record",			SV_MVD_Record_f);
	Cmd_AddCommand ("easyrecord",		SV_MVDEasyRecord_f);
	Cmd_AddCommand ("stop",				SV_MVDStop_f);
	Cmd_AddCommand ("cancel",			SV_MVD_Cancel_f);
	Cmd_AddCommand ("lastscores",		SV_LastScores_f);
	Cmd_AddCommand ("dlist",			SV_DemoList_f);
	Cmd_AddCommand ("dlistr",			SV_DemoListRegex_f);
	Cmd_AddCommand ("dlistregex",		SV_DemoListRegex_f);
	Cmd_AddCommand ("demolist",			SV_DemoList_f);
	Cmd_AddCommand ("demolistr",		SV_DemoListRegex_f);
	Cmd_AddCommand ("demolistregex",	SV_DemoListRegex_f);
	Cmd_AddCommand ("rmdemo",			SV_MVDRemove_f);
	Cmd_AddCommand ("rmdemonum",		SV_MVDRemoveNum_f);
	Cmd_AddCommand ("script",			SV_Script_f);
	Cmd_AddCommand ("demoInfoAdd",		SV_MVDInfoAdd_f);
	Cmd_AddCommand ("demoInfoRemove",	SV_MVDInfoRemove_f);
	Cmd_AddCommand ("demoInfo",			SV_MVDInfo_f);

	QTV_Init();
}
