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

extern cvar_t	sv_demoUseCache;
extern cvar_t	sv_demoCacheSize;
extern cvar_t	sv_demoMaxDirSize;
static int		demo_max_size;
static int		demo_size;

void SV_WriteDemoMessage (sizebuf_t *msg, int type, int to, float time);
size_t (*dwrite) ( const void *buffer, size_t size, size_t count, void *stream);


void SV_DemoPings (void)
{
	client_t *client;
	int		j;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		DemoReliableWrite_Begin (dem_all, 0, 7);
		MSG_WriteByte(demo.buf, svc_updateping);
		MSG_WriteByte(demo.buf,  j);
		MSG_WriteShort(demo.buf,  SV_CalcPing(client));
		MSG_WriteByte(demo.buf, svc_updatepl);
		MSG_WriteByte (demo.buf, j);
		MSG_WriteByte (demo.buf, client->lossage);
	}
}
/*
==============
SV_DemoWriteToDisk

Writes to disk a message ment for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

#define POS_TO 1
#define POS_SIZE 5
#define POS_DATA 9

#define BUF_FULL (1<<31)

void SV_DemoWriteToDisk(int type, int to, float time)
{
	int pos = 0;
	byte *p;
	int	btype, bto, bsize;
	sizebuf_t msg;

	//Con_Printf("to disk:%d:%d, demo.bufsize:%d, cur:%d:%d\n", type, to, demo.bufsize, demo.curtype, demo.curto);
	p = demo.dbuf->data;
	while (pos < demo.dbuf->size)
	{

		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE) & (~BUF_FULL);
		pos += POS_DATA + bsize; //pos now points to next message

		//Con_Printf("type %d:%d, size :%d\n", btype, bto, bsize);

		// no type means we are writing to disk everything
		if (!type || (btype == type && bto == to))
		{
			if (bsize) {
				msg.data = p+POS_DATA;
				msg.cursize = bsize;

				SV_WriteDemoMessage(&msg, btype, bto, time);
			}

			// data is written so it need to be cleard from demobuf
			//Con_Printf("memmove size:%d\n", demo.bufsize - pos);
			memmove(p, p+POS_DATA+bsize, demo.dbuf->size - pos);
			demo.dbuf->size -= bsize+POS_DATA;
			pos -= bsize + POS_DATA;
			if (demo.dbuf->cursize > pos)
				demo.dbuf->cursize -= bsize + POS_DATA;

			if (btype == demo.dbuf->curtype && bto == demo.dbuf->curto)
			{
				//Con_Printf("remove\n");
				demo.dbuf->curtype = 0;
				demo.dbuf->curto = 0;
				demo.dbuf->msgsize = NULL;
			}
		} else {
			// move along
			p += POS_DATA + bsize;
		}
	}
}

/*
==============
DemoSetBuf

Sets position in the buf for writing to specific client
==============
*/
void DemoSetBuf(int type, int to)
{
	byte *p;
	int pos = 0;
	int	btype, bto, bsize;

	p = demo.dbuf->data;
	while (pos < demo.dbuf->size)
	{
		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE);
		pos += POS_DATA + (bsize & (~BUF_FULL));

		//Con_Printf("type:%d, to:%d, size:%d\n", btype, bto, bsize);

		if (type == btype && to == bto && !(bsize&BUF_FULL))
		{
			demo.dbuf->cursize = pos;
			demo.dbuf->msgsize = (int*)(p + POS_SIZE);
			demo.dbuf->curtype = type;
			demo.dbuf->curto = to;
			return;
		}
		p += POS_DATA + (bsize & (~BUF_FULL));
	}
	// type&&to not exist in the buf, so add it

	*p = (byte)type;
	*(int*)(p + POS_TO) = to;
	*(int*)(p + POS_SIZE) = 0;
	demo.bufsize += POS_DATA;
	demo.dbuf->size += POS_DATA;
	demo.dbuf->msgsize = (int*)(p + POS_SIZE);
	demo.dbuf->curtype = type;
	demo.dbuf->curto = to;
	demo.dbuf->cursize = demo.dbuf->size;
}

void DemoReliableWrite_Begin(int type, int to, int size)
{
	byte *p;

	if (!sv.demorecording)
		return;

	// will it fit?
	while (demo.dbuf->size + size + POS_DATA > demo.dbuf->maxsize) {
		SV_DemoWritePackets(1); // does it work?
	}

	if (demo.dbuf->curtype != type || demo.dbuf->curto != to)
		DemoSetBuf(type, to);

	if (*demo.dbuf->msgsize + size > MAX_MSGLEN)
	{
		*demo.dbuf->msgsize |= BUF_FULL;
		DemoSetBuf(type, to);
	}

	// we have to make room for new data
	if (demo.dbuf->cursize != demo.dbuf->size) {
		p = demo.dbuf->data + demo.dbuf->cursize;
		memmove(p+size, p, demo.dbuf->size - demo.dbuf->cursize);
	}

	demo.dbuf->size += size;
	*demo.dbuf->msgsize += size;
	demo.bufsize += size;
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
	prevtime = time;
	if (msec > 255) msec = 255;

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
		(byte*) demo.dest -= 0x80000;
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
	demo_client_t	*cl, *nextcl;
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

	// now move the buffer back to make place for new data

	// size of mem that has been written to disk
	j = demo.frames[demo.lastwritten&DEMO_FRAMES_MASK].buf.data - demo.buffer;
	demo.bufsize -= j;

	memmove(demo.buffer, demo.buffer + j, demo.bufsize);
	for (i = demo.lastwritten; i < demo.parsecount; i++)
	{
		demo.frames[i&DEMO_FRAMES_MASK].buf.data -= j;
		demo.frames[i&DEMO_FRAMES_MASK].buf.maxsize += j; // is it necesery?
		(byte*) demo.frames[i&DEMO_FRAMES_MASK].buf.msgsize -= j;
	}

	demo.dbuf = &demo.frames[demo.parsecount&DEMO_FRAMES_MASK].buf;
}

size_t memwrite ( const void *buffer, size_t size, size_t count, byte *mem)
{
	int i,c = count;
	const byte *buf = buffer;
	byte *m = mem;

	for (;count; count--)
		for (i = size, buf = buffer; i; i--)
			*mem++ = *buf++;

	(byte*) demo.dest += mem - m;
	return c;
}

void Demo_Init (void)
{
	int p, size = MIN_DEMO_MEMORY;

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
		dwrite = &fwrite;
		demo.dest = demo.file;
		demo.disk = true;
	} else 
	{
		dwrite = &memwrite;
		demo.dest = svs.demomem;
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
void SV_Stop (int reason)
{
	if (!sv.demorecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}
	
// write a disconnect message to the demo file

	// clearup to be sure message will fit
	demo.dbuf->cursize = 0;
	demo.dbuf->curtype = 0;
	demo.dbuf->size = 0;
	DemoReliableWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte (demo.buf, svc_disconnect);
	MSG_WriteString (demo.buf, "EndOfDemo");
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

static void SV_Record (void)
{
	sizebuf_t	buf;
	char	buf_data[MAX_MSGLEN];
	int n, i;
	char *s, info[MAX_INFO_STRING];
	
	client_t *player;
	char *gamedir;
	int seq = 1;

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

		strcpy (info, player->userinfo);
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
SV_CleanName

Cleans the demo name, removes restricted chars, makes name lowercase
====================
*/

char *SV_CleanName (unsigned char *name)
{
	int	i;
	unsigned char c, *out;
	static char text[MAX_OSPATH];

	out = text;

	for ( ;*name; name++, out++)
	{
		if (*name >= 18 && *name <= 27) // liczby
		{
			*out = *name + 30;
		} else if (*name >= 146 && *name <= 155) // liczby
		{
			*out = *name - 98;
		} else if (*name == 29 || *name == 128 || *name == 157)
		{
			*out = '(';
		} else if (*name == 31 || *name == 130 || *name == 159)
		{
			*out = ')';
		} else if (*name == 16 || *name == 144)
		{
			*out = '[';
		} else if (*name == 17 || *name == 145)
		{
			*out = ']';
		} else if (*name > 128)
		{
			*out = *name - 128;
		} else 
			*out = *name;

		if (*out >= 'A' && *out <= 'Z')
			*out += 'a' - 'A';

		*out &= 0x7F;		// strip high bit
		c = *out;
		if (c<=' ' || c=='?' || c=='*' || c=='\\' || c=='/' || c==':'
			|| c=='<' || c=='>' || c=='"')
			*out = '_';
	}

	*out = 0;
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
	int		c, i;
	char	name[MAX_OSPATH], *s;
	dir_t	dir;
	demo_frame_t *frame;

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

	if (sv.demorecording)
		SV_Stop_f();

	dir = Sys_listdir(va("%s/demos", com_gamedir), ".*");
	if (sv_demoMaxDirSize.value && dir.size > sv_demoMaxDirSize.value*1024)
	{
		Con_Printf("insufficient directory space, increase sv_demoMaxDirSize\n");
		return;
	}
  
	sprintf (name, "%s/demos/%s", com_gamedir, SV_CleanName(Cmd_Argv(1)));
	Sys_mkdir(va("%s/demos", com_gamedir));

//
// open the demo file
//
	COM_ForceExtension (name, ".mvd");

	memset(&demo, 0, sizeof(demo));

	demo.dbuf = &demo.frames[0].buf;
	demo.dbuf->data = demo.buffer;
	demo.dbuf->maxsize = sizeof(demo.buffer);
	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	demo.file = fopen (name, "wb");
	if (!demo.file)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	SV_InitRecord();

	s = name + strlen(name);
	while (*s != '/') s--;
	strcpy(demo.name, s+1);

	SV_BroadcastPrintf (PRINT_CHAT, "Server starts recording (%s):\n%s\n", demo.disk ? "disk" : "memory", demo.name);
	Cvar_SetROM(&serverdemo, demo.name);
	SV_Record ();
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

void SV_EasyRecord_f (void)
{
	int		c;
	dir_t	dir;
	char	name[1024], *s;
	char	name2[MAX_OSPATH*2];
	int		i;
	unsigned char	*p;
	FILE	*f;
	demo_frame_t *frame;

	c = Cmd_Argc();
	if (c > 2)
	{
		Con_Printf ("easyrecord [demoname]\n");
		return;
	}

	if (sv.demorecording)
		SV_Stop_f();

	dir = Sys_listdir(va("%s/demos", com_gamedir), ".*");
	if (sv_demoMaxDirSize.value && dir.size > sv_demoMaxDirSize.value*1024)
	{
		Con_Printf("insufficient directory space, increase sv_demoMaxDirSize\n");
		return;
	}

	if (c == 2)
		sprintf (name, "%s", Cmd_Argv(1));
	else {
		// guess game type and write demo name
		i = Dem_CountPlayers();
		if (teamplay.value && i > 2)
		{
			// Teamplay
			sprintf (name, "team_%s_vs_%s_%s",
				Dem_Team(1),
				Dem_Team(2),
				sv.name);
		} else {
			if (i == 2) {
				// Duel
				sprintf (name, "duel_%s_vs_%s_%s",
					Dem_PlayerName(1),
					Dem_PlayerName(2),
					sv.name);
			} else {
				// FFA
				sprintf (name, "ffa_%s(%d)",
					sv.name,
					i);
			}
		}
	}

// Make sure the filename doesn't contain illegal characters
	s = SV_CleanName(name);

	sprintf (name, va("%s/demos/%s", com_gamedir, s));
	Sys_mkdir(va("%s/demos", com_gamedir));

// find a filename that doesn't exist yet
	strcpy (name2, name);
	COM_ForceExtension (name2, ".mvd");
	f = fopen (name2, "rb");
	if (f) {
		i = 0;
		do {
			fclose (f);
			strcpy (name2, va("%s_%02i", name, i));
			COM_ForceExtension (name2, ".mvd");
			f = fopen (name2, "rb");
			i++;
		} while (f);
	}

//
// open the demo file
//

	memset(&demo, 0, sizeof(demo));

	demo.dbuf = &demo.frames[0].buf;
	demo.dbuf->data = demo.buffer;
	demo.dbuf->maxsize = sizeof(demo.buffer);
	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	demo.file = fopen (name2, "wb");
	if (!demo.file)
	{
		Con_Printf ("ERROR: couldn't open %s\n", name2);
		return;
	}

	SV_InitRecord();

	s = name2 + strlen(name2);
	while (*s != '/') s--;
	strcpy(demo.name, s+1);

	SV_BroadcastPrintf (PRINT_CHAT, "Server starts recording (%s):\n%s\n", demo.disk ? "disk" : "memory", demo.name);
	Cvar_SetROM(&serverdemo, demo.name);
	SV_Record ();
}

void SV_DemoList_f (void)
{
	dir_t	dir;
	file_t	*list;
	float	f;
	int		i;
	char	*key;

	Con_Printf("content of %s/demos/*.mvd\n", com_gamedir);
	dir = Sys_listdir(va("%s/demos", com_gamedir), ".mvd");
	list = dir.files;
	if (!list->name[0])
	{
		Con_Printf("no demos\n");
	}

	if (Cmd_Argc() == 2)
		key = Cmd_Argv(1);
	else
		key = "";

	for (i = 0; list->name[0]; i++, list++)
	{
		if (strstr(list->name, key) != NULL) {
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

		dir = Sys_listdir(va("%s/demos", com_gamedir), ".mvd");
		list = dir.files;
		for (i = 0;list->name[0]; list++)
		{
			if (strstr(list->name, ptr))
			{
				if (sv.demorecording && !strcmp(list->name, demo.name))
					SV_Stop_f();

				// stop recording first;
				sprintf(path, "%s/demos/%s", com_gamedir, list->name);
				if (!Sys_remove(path)) {
					Con_Printf("removing %s...\n", list->name);
					i++;
				}
			}
		}

		if (i) {
			Con_Printf("%d demos removed\n", i);
		} else {
			Con_Printf("no matching found\n");
		}

		return;
	}

	Q_strncpyz(name ,Cmd_Argv(1), MAX_DEMO_NAME);
	COM_DefaultExtension(name, ".mvd");

	sprintf(path, "%s/demos/%s", com_gamedir, name);

	if (sv.demorecording && !strcmp(name, demo.name))
		SV_Stop_f();

	if (!Sys_remove(path))
		Con_Printf("demo %s succesfully removed\n", name);
	else 
		Con_Printf("unable to remove demo %s\n", name);
}

void SV_DemoRemoveNum_f (void)
{
	file_t	*list;
	dir_t	dir;
	int		num;
	char	*val;
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

	dir = Sys_listdir(va("%s/demos", com_gamedir), ".mvd");
	list = dir.files;
	while (list->name[0] && num) {list++; num--;};

	if (list->name[0]) {
		if (sv.demorecording && !strcmp(list->name, demo.name))
			SV_Stop_f();

		sprintf(path, "%s/demos/%s", com_gamedir, list->name);
		if (!Sys_remove(path))
			Con_Printf("demo %s succesfully removed\n", list->name);
		else 
			Con_Printf("unable to remove demo %s\n", list->name);
	} else
		Con_Printf("invalid demo num\n");
}