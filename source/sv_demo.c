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

void SV_WriteDemoMessage (sizebuf_t *msg, qboolean change);


void SV_DemoPings (void)
{
	client_t *client;
	int		j;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		DemoReliableWrite_Begin (dem_all, 0, 7);
		MSG_WriteByte(&demo.buf, svc_updateping);
		MSG_WriteByte(&demo.buf,  j);
		MSG_WriteShort(&demo.buf,  SV_CalcPing(client));
		MSG_WriteByte(&demo.buf, svc_updatepl);
		MSG_WriteByte (&demo.buf, j);
		MSG_WriteByte (&demo.buf, client->lossage);
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

void SV_DemoWriteToDisk(int type, int to)
{
	int pos = 0;
	byte *p;
	int	btype, bto, bsize;
	sizebuf_t msg;
	qboolean change;

	//Con_Printf("to disk:%d:%d, demo.bufsize:%d, cur:%d:%d\n", type, to, demo.bufsize, demo.curtype, demo.curto);
	p = demo.buf.data;
	while (pos < demo.bufsize)
	{

		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE);
		pos += POS_DATA + bsize; //pos now points to next message

		//Con_Printf("type %d:%d, size :%d\n", btype, bto, bsize);

		// no type means we are writing to disk everything
		if (!type || (btype == type && bto == to))
		{
			if (bsize) {
				change = false;
				if (!demo.lasttype || demo.lasttype != btype || demo.lastto != bto)
				{
					//Con_Printf("change:%d\n", demo.lasttype);
					// changed message direction
					demo.lasttype = btype;
					demo.lastto = bto;
					change = true;

					//SV_WriteDemoMessage(NULL, (byte)btype);
				}

				//Con_Printf("bsize:%d\n", bsize);

				msg.data = p+POS_DATA;
				msg.cursize = bsize;

				SV_WriteDemoMessage(&msg, change);
			}

			// data is written so it need to be cleard from demobuf
			//Con_Printf("memmove size:%d\n", demo.bufsize - pos);
			memmove(p, p+POS_DATA+bsize, demo.bufsize - pos);
			demo.bufsize -= bsize+POS_DATA;
			pos -= bsize + POS_DATA;
			if (demo.buf.cursize > pos)
				demo.buf.cursize -= bsize + POS_DATA;

			if (btype == demo.curtype && bto == demo.curto)
			{
				//Con_Printf("remove\n");
				demo.curtype = 0;
				demo.curto = 0;
				demo.cursize = NULL;
			}

			// did we find what we were looking for ?
			if (type)
				return;
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

	p = demo.buf.data;
	while (pos < demo.bufsize)
	{
		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE);
		pos += POS_DATA + bsize;

		//Con_Printf("type:%d, to:%d, size:%d\n", btype, bto, bsize);

		if (type == btype && to == bto)
		{
			demo.buf.cursize = pos;
			demo.cursize = (int*)(p + POS_SIZE);
			demo.curtype = type;
			demo.curto = to;
			return;
		}
		p += POS_DATA + bsize;
	}
	// type&&to not exist in the buf, so add it

	*p = (byte)type;
	*(int*)(p + POS_TO) = to;
	*(int*)(p + POS_SIZE) = 0;
	demo.bufsize += POS_DATA;
	demo.cursize = (int*)(p + POS_SIZE);
	demo.curtype = type;
	demo.curto = to;
	demo.buf.cursize = demo.bufsize;
}

void DemoReliableWrite_Begin(int type, int to, int size)
{
	byte *p;

	//Con_Printf("here %d:%d, size:%d, %d\n", type, to, size, demo.bufsize);
	// will it fit?
	if (demo.bufsize + size + POS_DATA > demo.buf.maxsize) {
		SV_DemoWriteToDisk(0,0); // flush everything on disk
	}

	if (demo.curtype != type || demo.curto != to)
		DemoSetBuf(type, to);

	if (*demo.cursize + size > MAX_MSGLEN)
	{
		SV_DemoWriteToDisk(type, to);
		DemoSetBuf (type, to);
	}

	// we have to make room for new data
	if (demo.buf.cursize != demo.bufsize) {
		p = demo.buf.data + demo.buf.cursize;
		memmove(p+size, p, demo.bufsize - demo.buf.cursize);
	}

	demo.bufsize += size;
	*demo.cursize += size;
}

/*
====================
SV_WriteDemoMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
void SV_WriteDemoMessage (sizebuf_t *msg, qboolean change)
{
	int		len, to, msec;
	byte	c, type;
	static double prevtime;

//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!sv.demorecording)
		return;

	msec = (demo.time - prevtime)*1000;
	prevtime = demo.time;
	//Con_Printf("msec:%d\n", msec);
	if (msec > 255) msec = 255;

	//fl = LittleFloat((float)demo.time);
	//fwrite (&fl, sizeof(fl), 1, demo.file);
	c = msec;
	fwrite(&c, sizeof(c), 1, demo.file);

	//fwrite (&type, sizeof(type), 1, demo.file);

	if (change)
	{
		switch (demo.lasttype)
		{
		case dem_all:
			c = dem_all;
			fwrite (&c, sizeof(c), 1, demo.file);
			break;
		case dem_multiple:
			c = dem_multiple;
			fwrite (&c, sizeof(c), 1, demo.file);

			to = LittleLong(demo.lastto);
			fwrite (&to, sizeof(to), 1, demo.file);
			break;
		case dem_single:
		case dem_stats:
			c = demo.lasttype + (demo.lastto << 3);
			//c = demo.lastto;
			fwrite (&c, sizeof(c), 1, demo.file);
			break;
		default:
			SV_Stop_f ();
			Con_Printf("bad demo message type:%d", type);
			return;
		}
	} else {
		c = dem_read;
		fwrite (&c, sizeof(c), 1, demo.file);
	}


	len = LittleLong (msg->cursize);
	fwrite (&len, 4, 1, demo.file);
	fwrite (msg->data, msg->cursize, 1, demo.file);

	fflush (demo.file);
}


/*
====================
SV_Stop_f

stop recording a demo
====================
*/
void SV_Stop_f (void)
{
	if (!sv.demorecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}

	
// write a disconnect message to the demo file
	DemoReliableWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte (&demo.buf, svc_disconnect);
	MSG_WriteString (&demo.buf, "EndOfDemo");
	SV_DemoWriteToDisk(0,0);

// finish up
	fclose (demo.file);
	demo.file = NULL;
	sv.demorecording = false;
	SV_BroadcastPrintf (PRINT_CHAT, "Completed demo\n");
	Cvar_SetROM(&serverdemo, "");
	//Con_Printf ("Completed demo\n");
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

//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!sv.demorecording)
		return;

	c = 0;
	fwrite (&c, sizeof(c), 1, demo.file);

	c = dem_read;
	fwrite (&c, sizeof(c), 1, demo.file);

	len = LittleLong (msg->cursize);
	fwrite (&len, 4, 1, demo.file);

	fwrite (msg->data, msg->cursize, 1, demo.file);

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
	fwrite (&c, sizeof(c), 1, demo.file);

	c = dem_set;
	fwrite (&c, sizeof(c), 1, demo.file);

	
	len = LittleLong(0);
	fwrite (&len, 4, 1, demo.file);
	len = LittleLong(0);
	fwrite (&len, 4, 1, demo.file);

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

	//clean up demoinfo for clients
	for (i = 0; i < MAX_CLIENTS; i++)
		memset(&svs.clients[i].demoinfo, 0, sizeof(demoinfo_t));

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
	/*
	if (cl.spectator)
		MSG_WriteByte (&buf, cl.playernum | 128);
	else
		MSG_WriteByte (&buf, cl.playernum);
	*/

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

		memset(&player->demoinfo, 0, sizeof(player->demoinfo));

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
SV_Record_f

record <demoname>
====================
*/
void SV_Record_f (void)
{
	int		c;
	char	name[MAX_OSPATH], *s;

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
  
	sprintf (name, "%s/demos/%s", com_gamedir, Cmd_Argv(1));
	Sys_mkdir(va("%s/demos", com_gamedir));

//
// open the demo file
//
	COM_ForceExtension (name, ".mvd");

	memset(&demo, 0, sizeof(demo));
	demo.buf.maxsize = sizeof(demo.buf_data);
	demo.buf.data = demo.buf_data;
	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	demo.file = fopen (name, "wb");
	if (!demo.file)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	s = name + strlen(name);
	while (*s != '/') s--;
	strcpy(demo.name, s+1);

	//Con_Printf ("recording to %s.\n", name);
	SV_BroadcastPrintf (PRINT_CHAT, "recording to %s.\n", demo.name);
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
	static char lastteam[2][32];
	qboolean first = true;
	client_t *client;
	static int index = 0;

	index = 1 - index;

	for (i = 0, client = svs.clients; num && i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (first || strcmp(lastteam[index], Info_ValueForKey(client->userinfo, "team")))
		{
			first = false;
			num--;
			strcpy(lastteam[index], Info_ValueForKey(client->userinfo, "team"));
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
	char	name[1024], *s;
	char	name2[MAX_OSPATH*2];
	int		i;
	unsigned char	*p;
	FILE	*f;

	c = Cmd_Argc();
	if (c > 2)
	{
		Con_Printf ("easyrecord [demoname]\n");
		return;
	}

	if (sv.demorecording)
		SV_Stop_f();


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
	for (p=name ; *p ; p++)	{
		char c;
		*p &= 0x7F;		// strip high bit
		c = *p;
		if (c<=' ' || c=='?' || c=='*' || c=='\\' || c=='/' || c==':'
			|| c=='<' || c=='>' || c=='"')
			*p = '_';
	}

	sprintf (name, va("%s/demos/%s", com_gamedir, name));
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
	demo.buf.maxsize = sizeof(demo.buf_data);
	demo.buf.data = demo.buf_data;
	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	demo.file = fopen (name2, "wb");
	if (!demo.file)
	{
		Con_Printf ("ERROR: couldn't open %s\n", name2);
		return;
	}
	s = name2 + strlen(name2);
	while (*s != '/') s--;
	strcpy(demo.name, s+1);

	//Con_Printf ("recording to %s.\n", name2);

	SV_BroadcastPrintf (PRINT_CHAT, "recording to %s.\n", demo.name);
	Cvar_SetROM(&serverdemo, demo.name);
	SV_Record ();
}

void SV_DemoList_f (void)
{
	dir_t	*list;
	int		i = 0;
	char	*key;

	Con_Printf("demolist:\n");
	list = Sys_listdir(va("%s/demos", com_gamedir), ".mvd");
	if (!list->name[0])
	{
		Con_Printf("no demos\n");
		return;
	}

	if (Cmd_Argc() == 2)
		key = Cmd_Argv(1);
	else
		key = "";

	while (list->name[0])
	{
		if (strstr(list->name, key) != NULL) {
			if (sv.demorecording && !strcmp(list->name, demo.name))
				Con_Printf("*%d: %s\n", i, list->name);
			else
				Con_Printf("%d: %s\n", i, list->name);//, (int)list->size/1024);
		}
		list++;
		i++;
	}
}

void SV_DemoRemove_f (void)
{
	char name[MAX_DEMO_NAME];
	char path[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf("rmdemo <demoname>\n");
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
	dir_t	*list;
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

	list = Sys_listdir(va("%s/demos", com_gamedir), ".mvd");
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