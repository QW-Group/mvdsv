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

#include "defs.h"

int memsize, membase;
int	fps;

sizebuf_t	net_message;
byte		net_message_buffer[MAX_UDP_PACKET];

demo_t		demo;
source_t	*from = NULL;
source_t	*sources = NULL;

world_state_t	world;
static_world_state_t	sworld;
lightstyle_t	lightstyle[MAX_LIGHTSTYLES];
entity_state_t	baselines[MAX_EDICTS];
sizebuf_t		stats_msg;
byte			stats_buf[MAX_MSGLEN];
char			currentDir[1024]; // MAX_OSPATH
char			qizmoDir[MAX_OSPATH];
char			outputDir[MAX_OSPATH];
char			sourceName[MAX_SOURCES][MAX_OSPATH];
#ifdef _WIN32
HANDLE			ConsoleInHndl, ConsoleOutHndl;
#endif

void QWDToolsMsg(void);

/*
================
Sys_Error
================
*/

void Sys_fclose(FILE **hndl)
{
	if (*hndl == NULL)
		return;
	if (*hndl == stdin)
		return;
	if (*hndl == stdout)
		return;
	fclose(*hndl);
	*hndl = NULL;
}

void Sys_Exit (int i)
{
#ifdef _WIN32
	DWORD r;
	struct _INPUT_RECORD in;
#endif
	int j;

	Dem_Stop(NULL);

	for (j = 0; j < sworld.fromcount; j++)
		Sys_fclose(&sworld.from[j].file);

	Sys_fclose(&sworld.demo.file);
	Sys_fclose(&sworld.debug.file);
	Sys_fclose(&sworld.log.file);
	Sys_fclose(&sworld.analyse.file);

	FreeFileList(sworld.filelist);
	Q_free(sources);

#ifdef _WIN32
	if ( sworld.options & O_WAITFORKBHIT)
	{
		if (i != 2)
			SetConsoleTitle(QWDTOOLS_NAME " Done");

		Sys_Printf("\nPress any key to continue\n");
		do ReadConsoleInput( ConsoleInHndl, &in, 1, &r); while(in.EventType != KEY_EVENT);
	}
#endif

	exit(i);
}

void Sys_mkdir (const char *path)
{
#ifdef _WIN32
	_mkdir (path);
#else
	mkdir (path, 0777);
#endif
}


void Sys_Error (const char *error, ...)
{
	va_list	argptr;
	char text[1024];

	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);


	if (sworld.from->file != NULL && sworld.from->file != stdin)
		Sys_Printf ("ERROR: %s, at:%d\n", text, ftell(sworld.from->file));
	else
		Sys_Printf ("ERROR: %s\n", text);

	Sys_Exit (1);
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		buf[1024];
#ifdef _WIN32
	DWORD		count;
#endif

	// stdout can be redirected to a file or other process
	if (
#ifdef _WIN32
	    ConsoleOutHndl != NULL &&
#endif
	    sworld.options & O_STDOUT)
	{
		va_start (argptr,fmt);
		vsnprintf(buf, sizeof(buf), fmt, argptr);
		va_end (argptr);
#ifdef _WIN32
		WriteFile(ConsoleOutHndl, buf, strlen(buf), &count, NULL);
#else
		fprintf(stderr, "%s", buf);
#endif
		return;
	}

	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

/*
=================
ConnectionlessPacket
 
Responses to broadcasts, etc
=================
*/

extern int msg_startcount;

/*
=================
Netchan_Process
 
called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/

qbool Netchan_Process (netchan_t *chan)
{
	unsigned		sequence, sequence_ack;
	unsigned		reliable_ack, reliable_message;

	// get sequence numbers
	MSG_BeginReading ();
	sequence = MSG_ReadLong ();
	sequence_ack = MSG_ReadLong ();

	// read the qport if we are a server
	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);
	sequence_ack &= ~(1<<31);

	//
	// discard stale or duplicated packets
	//
	if (sequence <= (unsigned)chan->incoming_sequence)
	{
		return false;
	}

	//
	// if the current outgoing reliable message has been acknowledged
	// clear the buffer to make way for the next
	//
	if (reliable_ack == (unsigned)chan->reliable_sequence)
		chan->reliable_length = 0;	// it has been received

	//
	// if this message contains a reliable message, bump incoming_reliable_sequence
	//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
		chan->incoming_reliable_sequence ^= 1;

	return true;
}


void ConnectionlessPacket (void)
{
	int		c;

	MSG_BeginReading ();
	MSG_ReadLong ();        // skip the -1

	c = MSG_ReadByte ();
	if (c == S2C_CONNECTION)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "connected\n");

		from->netchan.incoming_sequence = 0;
		from->netchan.incoming_acknowledged= 0;
		from->netchan.incoming_acknowledged = 0;
		from->netchan.incoming_reliable_acknowledged = 0;
		return;
	}

	// remote command from gui front end
	if (c == A2C_CLIENT_COMMAND)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "A2C\n");
		MSG_ReadString ();
		MSG_ReadString ();

		return;
	}

	// print command from somewhere
	if (c == A2C_PRINT)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "print\n");
		MSG_ReadString ();
		return;
	}

	// ping from somewhere
	if (c == A2A_PING)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "ping\n");
		return;
	}

	if (c == S2C_CHALLENGE)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "challenge\n");
		MSG_ReadString ();
		return;
	}

	if (c == svc_disconnect)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "disconnect\n");
		Dem_Stop(from);
		return;
	}
}

void Dem_Stop(source_t *s)
{
	int i;
	int run;

	if (!sources)
		return;

	if (sworld.options & O_QWDSYNC)
	{
		if (s != NULL)
			s->running = 0;
		return;
	}

	run = world.running;

	if (s == NULL)
		for (i = 0; i < sworld.fromcount; i++)
		{
			StopQWZ(sources + i);

			world.running -= sources[i].running;
			sources[i].running = 0;
		}
	else
	{
		StopQWZ(s);
		world.running -= s->running;
		s->running = 0;
	}

	if (msgbuf)
	{
		msgbuf->cursize = 0;
		msgbuf->bufsize = 0;
		msgbuf->h = NULL;
	}

	if (!run || world.running)
		return;

	// last demo closed, say bye
	QWDToolsMsg();

	MVDWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte(msgbuf, svc_disconnect);
	MSG_WriteString (msgbuf, "EndOfDemo");
}

/*
====================
GetDemoMessage
====================
*/

qbool GetDemoMessage (void)
{
	int		r, i, j, num;
	float	f, demotime;
	byte	newtime;
	byte	c;
	usercmd_t *pcmd;

	num = from - sources;

nextdemomessage:

	// read the time from the packet
	newtime = 0;
	if (!from->lasttime)
	{
		if (from->format == mvd)
		{
			r = fread(&newtime, sizeof(newtime), 1, sworld.from[num].file);
			from->prevtime += newtime;
			demotime = from->basetime + from->prevtime*0.001*from->ratio;
		}
		else
		{
			r = fread(&demotime, sizeof(demotime), 1, sworld.from[num].file);
			demotime = LittleFloat(demotime);
		}

		if (r != 1)
		{
			Dem_Stop(from);
			return 0;
		}
	}
	else
	{
		demotime = from->lasttime;
		newtime = from->prevnewtime;
		from->lasttime = 0;
	}

	// decide if it is time to grab the next message
	if (from->lastframe < 0)
		from->lastframe = demotime;
	else if (demotime > from->lastframe + 0.001)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "signonloaded %f, %f, %d\n", demotime, from->lastframe, newtime);
		from->lastframe = demotime;
		from->lasttime = demotime;
		from->prevnewtime = newtime;

		// signon datagram is for sure loaded now
		from->signonloaded = true;
		return 0; // already read this frame's message
	}

	if (demotime - from->time > 0.0001 && from->format == mvd)
	{
		from->netchan.incoming_sequence++;
		from->netchan.incoming_acknowledged++;
		from->frames[from->netchan.incoming_acknowledged&UPDATE_MASK].num_projectiles = 0;
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "sequence:%d\n", from->netchan.incoming_sequence);
	}

	from->time = demotime; // warp
	//from->prevtime = demotime;

	if (sworld.options & O_DEBUG)
		fprintf(sworld.debug.file, "msec:%d  %ld\n", newtime, ftell(sworld.from[num].file));

	// get the msg type
	fread (&c, sizeof(c), 1, sworld.from[num].file);

	switch (c&7)
	{
	case dem_cmd :
		// user sent input
		i = from->netchan.outgoing_sequence & UPDATE_MASK;
		pcmd = &from->frames[i].cmd;
		r = fread (pcmd, sizeof(*pcmd), 1, sworld.from[num].file);
		if (r != 1)
		{
			Dem_Stop(from);
			return 0;
		}

		// byte order stuff
		for (j = 0; j < 3; j++)
		{
			pcmd->angles[j] = LittleFloat(pcmd->angles[j]);
			if (pcmd->angles[j] > 180)
				pcmd->angles[j] -= 360;
		}

		pcmd->forwardmove = LittleShort(pcmd->forwardmove);
		pcmd->sidemove    = LittleShort(pcmd->sidemove);
		pcmd->upmove      = LittleShort(pcmd->upmove);
		from->frames[i].senttime = demotime;
		from->frames[i].receivedtime = -1;		// we haven't gotten a reply yet
		from->netchan.outgoing_sequence++;
		from->frames[i].playerstate[from->playernum].command = *pcmd;
		for (j=0 ; j<3 ; j++)
		{
			r = fread (&f, 4, 1, sworld.from[num].file);
			from->frames[from->netchan.incoming_sequence&UPDATE_MASK].playerstate[from->playernum].command.angles[j] = LittleFloat (f);
		}

		break;

	case dem_read:
readit:
		// get the next message
		fread (&net_message.cursize, 4, 1, sworld.from[num].file);
		net_message.cursize = LittleLong (net_message.cursize);

		if (net_message.cursize > MAX_UDP_PACKET)
		{
			Sys_Printf ("ERROR: Demo message > MAX_UDP_PACKET (%d)\n", net_message.cursize);
			Dem_Stop(from);
			return 0;
		}
		r = fread (net_message.data, net_message.cursize, 1, sworld.from[num].file);
		if (r != 1)
		{
			Dem_Stop(from);
			return 0;
		}

		break;

	case dem_set :
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "dem_set\n");
		fread (&i, 4, 1, sworld.from[num].file);
		from->netchan.outgoing_sequence = LittleLong(i);
		fread (&i, 4, 1, sworld.from[num].file);
		from->netchan.incoming_sequence = LittleLong(i);
		if (from->format == mvd)
		{
			from->netchan.incoming_acknowledged = from->netchan.incoming_sequence;
			goto nextdemomessage;
		}

		break;

	case dem_multiple:
		r = fread (&i, 4, 1, sworld.from[num].file);
		if (r != 1)
		{
			Dem_Stop(from);
			return 0;
		}

		from->to = LittleLong(i);
		from->type = dem_multiple;
		goto readit;

	case dem_single:
		from->to = c>>3;
		from->type = dem_single;
		goto readit;
	case dem_stats:
		from->to = c>>3;
		from->type = dem_stats;
		goto readit;
	case dem_all:
		from->to = 0;
		from->type = dem_all;
		goto readit;
	default :
		Dem_Stop(from);
		return 0;
	}

	return 1;
}

int To(void);
void CheckSpectator (void)
{
	player_state_t	*state, *self;
	int i,j;

	if (!from->spectator)
		return;

	self = &from->frames[from->netchan.incoming_sequence&UPDATE_MASK].playerstate[from->playernum];
	for (i = 0, state = from->frames[from->netchan.incoming_sequence&UPDATE_MASK].playerstate; i < MAX_CLIENTS; i++, state++)
	{
		if (state->messagenum != from->parsecount)
			continue;

		if (i == from->playernum)
			continue;

		for (j = 0; j < 3; j++)
			if (abs ((int)(state->command.angles[j] - self->command.angles[j])) > 0.0)
			{
				break;
			}

		if (j != 3)
			continue;
		break;
	}

	if (i == MAX_CLIENTS)
	{
		from->spec_track = -1;
	}
	else
		from->spec_track = i;
}

void ReadPackets (void)
{
	// if it's not a time to read packet from this demofile, return
	if (world.time && from->worldtime > world.time)
	{
		return;
	}

	if (!from->running)
		return;

	if (sworld.options & O_QWDSYNC)
		SZ_Clear(msgbuf);

	while (GetDemoMessage())
	{
		//
		// remote command packet
		//
		if (*(int *)net_message.data == -1)
		{
			ConnectionlessPacket ();

			if (!from->running)
			{
				break;
			}
			continue;
		}

		if (net_message.cursize < 8 && from->format != mvd)
		{
			continue;
		}


		if (from->format == qwd)
		{
			if (!Netchan_Process(&from->netchan))
				continue;		// wasn't accepted for some reason
		}
		else
		{
			MSG_BeginReading ();
		}

		Dem_ParseDemoMessage ();

		if (sworld.options & O_QWDSYNC || !(sworld.options & (O_CONVERT | O_MARGE)))
			SZ_Clear(msgbuf);


		if (!from->running)
			return;
	}

	CheckSpectator();

	if (stats_msg.cursize)
	{
		MVDWrite_Begin(dem_stats, To(), stats_msg.cursize);
		SZ_Write(msgbuf, stats_msg.data, stats_msg.cursize);
		SZ_Clear(&stats_msg);
	}

	if (sworld.options & O_QWDSYNC)
		return;

	/*
	if (world.signonloaded && from->signon_stats.cursize)
	{
		DemoWrite_Cat(&from->signon_stats);
		SZ_Clear(&from->signon_stats);
	}
	*/

	if (sworld.from->file != stdin)
	{
		int p = 0, i;
		long c;

		for (i = 0, c = 0; i < sworld.fromcount; i++)
			if (sources[i].running)
				c += ftell(sworld.from[i].file);
			else c += sworld.from[i].filesize;

		if (c - world.oldftell > 256000)
		{
#ifdef _WIN32
			if ((p = (int)100.0*c/world.demossize) != world.percentage)
				SetConsoleTitle(va("qwdtools  %d%% (%d of %d)", p, sworld.count, sworld.sources));
#endif
			world.percentage = p;
			world.oldftell = c;
		}
	}

}

__inline void SetWorldTime()
{
	static float lasttime = 0;

	if (lasttime > world.time)
		lasttime = world.time;

	while (lasttime <= world.time)
		lasttime += 1.0/sworld.fps;

	world.time = lasttime;

}

__inline void GetDemoTimes(double *mintime, double *nearest)
{
	int i;
	double t;

	*mintime = *nearest = -1;

	for (i = 0; i < sworld.fromcount; i++)
	{
		if (!sources[i].running)
			continue;

		t = sources[i].worldtime;

		if (*mintime < 0 || *mintime > t)
			*mintime = t;
		if (*nearest < 0 || fabs(t - world.time) < fabs(*nearest - world.time))
			*nearest = t;
	}
}

void AddEntities(packet_entities_t *to, frame_t *prev, frame_t *next)
{
	packet_entities_t *prevp, *nextp, newp, worldp;
	double	f;
	int		i,k,j;

	// check if interpolations is possible
	if (prev->invalid || prev->parsecount > next->parsecount)
		prev = next;

	if (next->invalid)
		return;
#if 1
	if (next == prev || next->time == world.time)
		newp = next->packet_entities;
	else
	{
		f = (prev->time - next->time)/(next->time - world.time);

		nextp = &next->packet_entities;
		prevp = &prev->packet_entities;

		newp.num_entities = nextp->num_entities;

		j = 0;

		for (i=0; i < next->packet_entities.num_entities; i++)
		{
			while (j < prevp->num_entities && i < nextp->entities[i].number)
				j++;

			newp.entities[i] = nextp->entities[i];
			if (j < prevp->num_entities && prevp->entities[j].number == nextp->entities[i].number)
			{
				for (k=0; k<3; k++)
				{
					newp.entities[i].origin[k] = prevp->entities[j].origin[k] + f*(prevp->entities[j].origin[k] - nextp->entities[i].origin[k]);
					newp.entities[i].angles[k] = AdjustAngle(nextp->entities[i].angles[k], prevp->entities[j].angles[k],1.0+f);
				}
			}
			else
			{
				VectorCopy(nextp->entities[i].origin, newp.entities[i].origin);
				VectorCopy(nextp->entities[i].angles, newp.entities[i].angles);
			}
		}
	}
#else
	newp = next->packet_entities;
#endif

	i = 0;
	j = 0;
	worldp.num_entities = 0;

	// scaling
	while (i < newp.num_entities && j < to->num_entities)
		if (newp.entities[i].number < to->entities[j].number)
			worldp.entities[worldp.num_entities++] = newp.entities[i++];
		else if (newp.entities[i].number > to->entities[j].number)
			worldp.entities[worldp.num_entities++] = to->entities[j++];
		else
		{
			worldp.entities[worldp.num_entities++] = to->entities[j++];
			i++;
		}

	while (i < newp.num_entities)
		worldp.entities[worldp.num_entities++] = newp.entities[i++];

	while (j < to->num_entities)
		worldp.entities[worldp.num_entities++] = to->entities[j++];

	*to = worldp;
}

void AddNails(frame_t *to, frame_t *frame)
{
	int num, i, j;
	vec3_t diff;

	num = to->num_projectiles;

	for (j = 0; j < frame->num_projectiles; j++)
	{
		for (i = 0; i < num; i++)
		{
			VectorSubtract(frame->projectiles[j].origin, to->projectiles[i].origin, diff);
			if (VectorLength(diff) < 64)
				break;
		}

		if (i != num)
			break;

		to->projectiles[to->num_projectiles++] = frame->projectiles[j];
	}
}

void AddPlayers(frame_t *to, frame_t *frame)
{
	int i, last;
	player_state_t *state, *wstate;
	int msec;

	wstate = to->playerstate;
	state = frame->playerstate;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (state[i].messagenum != from->parsecount)
			continue;

		if (world.players[i].lastsource)
		{
			last = world.players[i].lastsource - 1;
			if (last != from-sources && sources[last].running && sources[last].frames[sources[last].parsecount&UPDATE_MASK].playerstate[i].messagenum == sources[last].parsecount)
				continue;
			world.players[i].lastsource = from-sources + 1;
		}
		else world.players[i].lastsource = from-sources + 1;

		msec = (int)((world.time - frame->time) * 1000);
		if (msec > 100) msec = 100;
		else if (msec < -100) msec = -100;

		msec += state[i].command.msec;

		if (wstate[i].messagenum != to->parsecount)
		{
			wstate[i] = state[i];
			wstate[i].msec = msec;
			wstate[i].messagenum = to->parsecount;
		}
	}
}

void Update_SignonStats(void)
{
	int j, i, *stats, *wstats;

	for (from = sources; from - sources < sworld.fromcount; from++)
	{
		if (!from->running)
			continue;

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (from->players[i].spectator)
				continue;
			if (!from->players[i].name[0])
				continue;

			stats = from->players[i].stats;
			wstats = world.players[i].stats;

			for (j=0 ; j<MAX_CL_STATS ; j++)
				if (stats[j] != wstats[j] && stats[j])
				{
					wstats[j] = stats[j];
					if (stats[j] >=0 && stats[j] <= 255)
					{
						MVDWrite_Begin(dem_stats, i, 3);
						MSG_WriteByte(msgbuf, svc_updatestat);
						MSG_WriteByte(msgbuf, j);
						MSG_WriteByte(msgbuf, stats[j]);
					}
					else
					{
						MVDWrite_Begin(dem_stats, i, 6);
						MSG_WriteByte(msgbuf, svc_updatestatlong);
						MSG_WriteByte(msgbuf, j);
						MSG_WriteLong(msgbuf, stats[j]);
					}
				}
		}
	}
}

qbool Synchronize (void);
void MainLoop(void)
{
	frame_t	*fframe, *tframe, *pframe;
	float	acc_time = 0, acc_ctime = 0;
	int		acc_count = 0;
	int		acc_count_o = 0;
	int		num = -1;
	double	mintime, nearest;

#ifdef _WIN32
	if (!(sworld.options & O_STDIN))
		SetConsoleTitle(va("qwdtools  0%% (%d of %d)", sworld.count, sworld.sources));
#endif
	// check if demos are from the same game, and read signon datagram
	world.signonstats = sworld.options & O_MARGE;
	for (from = sources; from - sources < sworld.fromcount; from++)
	{
		DemoBuffer_Set(&from->dbuffer);
		ReadPackets();
	}
	world.signonstats = false;

	// sync demos
	world.signonloaded = true;
	if (!Synchronize())
	{
		Sys_Printf("couldn't synchronize demos\n");
		Dem_Stop(NULL);
		return;
	}
	world.signonloaded = false;

	for (from = sources; from - sources < sworld.fromcount; from++)
	{

		if (from->running && (from->worldtime < world.time || num == -1))
		{
			num = from - sources;
			demo.time = world.time = from->worldtime;
		}
	}

	// now write the signon datagram
	if (sworld.options & (O_CONVERT | O_MARGE))
	{

		if (num == -1)
		{
			Sys_Printf("No signon datagram!\n");
			Dem_Stop(NULL);
			return;
		}
		else
		{
			DemoBuffer_Set(&sources[num].dbuffer);
			if (sworld.options & O_MARGE)
				Update_SignonStats();
			SV_MVDWriteToDisk(sources[num].dbuffer.msgbuf,0,0,0);

			acc_time = acc_ctime = world.time = sources[num].worldtime;
			world.signonloaded = true;
		}
	}

	// clean signon datagrams from all of the demos, they won't be needed
	for (from = sources; from - sources < sworld.fromcount; from++)
	{
		SZ_Clear(&from->frames[0].buf);
		DemoBuffer_Clear(&from->dbuffer);
		MVDSetMsgBuf(&from->dbuffer,&from->frames[0].buf);
	}

	// set next frame time that's wished to get
	SetWorldTime();

	while (world.running)
	{
		if  (sworld.options & O_SHUTDOWN)
			Sys_Exit(2);

		for (from = sources; from - sources < sworld.fromcount; from++)
		{
			if (!from->running)
				continue;

			DemoBuffer_Set(&from->dbuffer);

			// read packet from demo
			ReadPackets();
		}

		// check if all demos passed world.time, find nearest one
		GetDemoTimes(&mintime, &nearest);

		// if not converting, stop here
		if (!(sworld.options & (O_CONVERT | O_MARGE)))
		{
			if (mintime >= world.time)
			{
				world.time = nearest;
				SetWorldTime();
			}
			continue;
		}

		if (mintime >= world.time || !world.running)
		{
			// fix world.time,
			world.time = nearest;

			//interpolate entities position for this frame
			tframe = &world.frames[world.parsecount&UPDATE_MASK];
			tframe->invalid = true;
			tframe->packet_entities.num_entities = 0;
			tframe->num_projectiles = 0;
			tframe->parsecount = world.parsecount;
			tframe->time = world.time;
			tframe->latency = 0;

			for (from = sources; from - sources < sworld.fromcount; from++)
			{
				if (!from->running)
					continue;

				if (from->oldparse == from->parsecount)
				{
					//continue;
				}

				if (from->worldtime > world.time + 1)
					continue;

				from->oldparse = from->parsecount;
				fframe = &from->frames[from->parsecount&UPDATE_MASK];
				tframe->invalid = false;

				if (from->worldtime == world.time || from->parsecount == 0)
				{
					AddEntities(&tframe->packet_entities, fframe, fframe);
					AddNails(tframe, fframe);
					AddPlayers(tframe, fframe);
				}
				else
				{
					num = from->parsecount - 1;
					pframe = &from->frames[(num)&UPDATE_MASK];
					while (pframe->time > world.time)
					{
						pframe = &from->frames[(--num)&UPDATE_MASK];

						if (pframe->parsecount > fframe->parsecount)
							break;

						if (!pframe->parsecount)
							break;
					}

					if (pframe->parsecount <= fframe->parsecount)
						AddEntities(&tframe->packet_entities, pframe, fframe);
					else
						AddEntities(&tframe->packet_entities, fframe, fframe);

					AddNails(tframe, fframe);
					AddPlayers(tframe, fframe);
				}
			}

			if (!tframe->invalid)
			{
				SetWorldTime();

				if (world.parsecount - world.lastwritten > 60)
				{
					WritePackets(30);
				}

				world.parsecount++;
				for (from = sources; from - sources < sworld.fromcount; from++)
				{
					MVDSetMsgBuf(&from->dbuffer,&from->frames[world.parsecount&UPDATE_MASK].buf);
				}
			}
		}

		if (!world.running)
			WritePackets(world.parsecount - world.lastwritten + 1);
	}

	//for (from = sources; from - sources < sworld.fromcount; from++)
	//	Sys_Printf("sync:%f\n", from->sync);

	if (sworld.options & O_CONVERT)
	{
		if (acc_count)
			Sys_Printf(" average demo fps:%.1f (originally %.1f)\n", acc_count/(acc_ctime - acc_time), acc_count_o/(acc_ctime - acc_time));
	}
}

void QWDToolsMsg(void)
{
	char str[1024];

	strlcpy(str, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n"
	       " This demo was converted\n"
	       " from QWD by " QWDTOOLS_NAME " " VERSION_NUMBER "\n"
	       "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n",
			sizeof(str));

	MVDWrite_Begin(dem_all, 0, strlen(str) + 3);
	MSG_WriteByte(msgbuf, svc_print);
	MSG_WriteByte(msgbuf, PRINT_CHAT);
	MSG_WriteString(msgbuf, str);
}

qbool ClearWorld(void)
{
	char *ext;
	extern char stdintype[32];
	int i;

	sources = (source_t*) realloc (sources, sizeof(source_t)*sworld.fromcount);
	if (!sources)
		Sys_Error ("Not enough memory free; check disk space");

	memset(sources, 0, sizeof(source_t)*sworld.fromcount);
	memset(&world, 0, sizeof(world));
	memset(&demo, 0, sizeof(demo));

	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	world.messages.data = world.buffer;
	world.messages.maxsize = sizeof(world.buffer);

	// for converting spectator demos
	stats_msg.maxsize = sizeof(stats_buf);
	stats_msg.data = stats_buf;

	MVDBuffer_Init(&demo.dbuffer, demo.buffer, sizeof(demo.buffer), &world.frames[0].buf);

	for (i = 0, from = sources; i < sworld.fromcount; i++, from++)
	{
		MVDBuffer_Init(&from->dbuffer, from->buffer, sizeof(from->buffer), &from->buf[0]);

		from->running = 1 << i;
		from->lastframe = -1;
		from->ratio = 1;
		from->signon_stats.maxsize = sizeof(from->signon_stats_buf);
		from->signon_stats.data = from->signon_stats_buf;
		world.running |= from->running;

		if (sworld.options & O_STDIN)
		{
			strlcpy(sworld.from[i].name, "stdin", MAX_OSPATH);
			ext = va(".%s", stdintype);
		}
		else
			ext = COM_FileExtension(sworld.from[i].name);

		// qwz -> qwd
		if (!strcasecmp(ext, "qwz"))
		{
			*(sworld.from[i].name + strlen(sworld.from[i].name) - 1) = 'd';
			ext[3] = 'd';
			from->qwz = true;
		}

		if (!strcasecmp(ext, "qwd"))
			from->format = qwd;
		else if (!strcasecmp(ext, "mvd"))
			from->format = mvd;
		else
		{
			Sys_Printf("Ignoring %s\n", sworld.from[i].name);
			return false;
		}
	}

	return true;
}

/*
==================
main
==================
*/

int main (int argc, char **argv)
{
	int		i, j, options, count;
	flist_t	*source;
	file_t	*fromfiles;

	Argv_Init(argc, argv);
	CtrlH_Init();
	World_Init();

	//	Sys_Printf( VERSION " (c) 2001 Bartlomiej Rychtarski\nhttp://qwex.n3.net/   mailto:highlander@gracz.net\n\n");
	Sys_Printf(QWDTOOLS_NAME " version " VERSION_NUMBER " (c) 2001-2003 Bartlomiej Rychtarski\n");
	Sys_Printf("Unix port by David (hexum) Balcom and VVD, 2004-2007\n");
	Sys_Printf("Part of the " SERVER_NAME " project: " HOMEPAGE_URL "\n\n");

	Tools_Init();

	Load_ini();
	ParseArgv();

	options = sworld.options & JOB_TODO;

	if (sworld.options & O_FC)
		Sys_Printf("   -filter_chats\n");
	else
	{
		if (sworld.options & O_FS)
			Sys_Printf("   -filter_spectalk\n");
		if (sworld.options & O_FQ)
			Sys_Printf("   -filter_qizmotalk\n");
		if (sworld.options & O_FT)
			Sys_Printf("   -filter_teamchats\n");
	}

	Sys_Printf("   -fps %d\n", sworld.fps);
	Sys_Printf("   -msglevel %d\n\n", sworld.msglevel);

	if (sworld.options & O_MARGE)
		count = MAX_CLIENTS;
	else
		count = 1;

	// serve all source files in loop
	source = sworld.filelist;
	i = 0;
	while (1)
	{
		if  (sworld.options & O_SHUTDOWN)
			Sys_Exit(2);

		fromfiles = sworld.from;
		sworld.fromcount = 0;

		// get source file names
		for (; sworld.fromcount < count && (i < source->count || ((i=0) < (++source)->count));
		        i++, fromfiles++, sworld.fromcount++)
		{
			strlcpy(fromfiles->path, source->path, sizeof(fromfiles->path));
			strlcpy(fromfiles->name, source->list[i], sizeof(fromfiles->name));
		}

		if (!sworld.fromcount)
			break;

		sworld.count += sworld.fromcount;

		if (sworld.fromcount == MAX_CLIENTS)
			Sys_Printf("Warning: too many source files\n");


		// alloc memory and cleanup the world
		if (!ClearWorld())
		{
			if (options & O_MARGE)
			{
				Sys_Printf("not all files could be opened\n");
				break;
			}
			continue;
		}

		// try to open source files
		sworld.options &= ~(JOB_TODO);
		sworld.options |=  Files_Init(options);

		// run program
		if (sworld.options & JOB_TODO)
			MainLoop();

		for (j = 0; j < sworld.fromcount; j++)
			Sys_fclose(&sworld.from[j].file);

		Sys_fclose(&sworld.demo.file);
		Sys_fclose(&sworld.debug.file);
		Sys_fclose(&sworld.log.file);
		Sys_fclose(&sworld.analyse.file);

		// if marging no loop
		if (options & O_MARGE)
			break;
	}

	Sys_Printf("\nDone.\n");

	Sys_Exit(0);
	return false; // to happy compiler
}
