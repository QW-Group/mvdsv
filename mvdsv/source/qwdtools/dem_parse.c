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
// cl_parse.c  -- parse a message received from the server

#include "defs.h"

char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value
	
	"svc_serverdata",		// [long] version ...
	"svc_lightstyle",		// [byte] [string]
	"svc_updatename",		// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",			// [byte] impact [byte] blood [vec3] from
	
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	
	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",

	"svc_cdtrack",
	"svc_sellscreen",

	"svc_smallkick",
	"svc_bigkick",

	"svc_updateping",
	"svc_updateentertime",

	"svc_updatestatlong",
	"svc_muzzleflash",
	"svc_updateuserinfo",
	"svc_download",
	"svc_playerinfo",
	"svc_nails",
	"svc_choke",
	"svc_modellist",
	"svc_soundlist",
	"svc_packetentities",
 	"svc_deltapacketentities",
	"svc_maxspeed",
	"svc_entgravity",

	"svc_setinfo",
	"svc_serverinfo",
	"svc_updatepl",
	"svc_nails2",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL",
	"NEW PROTOCOL"
};

#define	svc_qizmomsg	83		// qizmo voice message

int	parsecountmod;
double	parsecounttime;

int		cl_spikeindex, cl_playerindex, cl_flagindex;
int		cl_telemin = 9999, cl_telemax = 0;

projectile_t	cl_projectiles[MAX_PROJECTILES];
int				cl_num_projectiles;

//=============================================================================

int packet_latency[NET_TIMINGS];
int	msg_startcount;
//double parsecounttime;

int To(void)
{
	if (!from.spectator)
		return from.to;

	if (from.spec_track == -1)
		return from.to;

	return from.spec_track;
}


/*
=====================
Dem_ParseDownload

A download message has been received from the server
=====================
*/

void Dem_ParseDownload (void)
{
	int		size, percent;

	msg_startcount = msg_readcount;
	// read the data
	size = MSG_ReadShort ();
	percent = MSG_ReadByte ();

	if (size > 0)
		msg_readcount += size;

	DemoWrite_Begin(from.type, from.to, msg_readcount - msg_startcount + 1);
	MSG_WriteByte(msgbuf,svc_download);
	MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
}

/*
=====================================================================

  SERVER CONNECTING MESSAGES

=====================================================================
*/

/*
==================
Dem_ParseServerData
==================
*/

void Dem_ParseServerData (void)
{
	char	str[MAX_INFO_STRING], str2[MAX_INFO_STRING];
	int protover, count;

	count = 0;

// parse protocol version number
// allow 2.2 and 2.29 demos to play
	protover = MSG_ReadLong ();
	count += 4;
	if (protover != PROTOCOL_VERSION && 
		!(protover == 26 || protover == 27 || protover == 28)) {
		Sys_Printf ("Incompatible demo version: %i\n", protover);
		Dem_Stop();
		return;
	}

	from.servercount = MSG_ReadLong ();
	count += 4;

	// game directory
	strcpy(str,MSG_ReadString ());
	count += strlen(str)+1;

	if (from.format == mvd){
		world.time = world.frames[0].time = from.time = realtime = demo.time = from.lastframe = MSG_ReadFloat();
		from.playernum = 31;
	} else {
		// parse player slot, high bit means spectator
		from.playernum = MSG_ReadByte ();
		from.spectator = false;
		if (from.playernum & 128)
		{
			from.playernum &= ~128;
			from.spectator = true;
		}
		from.type = dem_single;
		from.to = from.playernum;
		world.time = world.frames[0].time = demo.time = from.lastframe;
	}
	count += 4;

	// get the full level name
	strcpy(str2,MSG_ReadString ());
	count += strlen(str2)+1;

	DemoWrite_Begin(dem_all, 0, count + 40+1);
	MSG_WriteByte(msgbuf, svc_serverdata);
	MSG_WriteLong(msgbuf, protover);
	MSG_WriteLong(msgbuf, world.servercount);
	MSG_WriteString(msgbuf, str);
	MSG_WriteFloat(msgbuf, world.time);
	MSG_WriteString(msgbuf, str2);
	MSG_Forward(msgbuf, msg_readcount, 40);
}

/*
==================
Dem_Parselist
==================
*/

void Dem_Parselist (byte type)
{
	char	*str;
	int n;

	msg_startcount = msg_readcount;
	n = MSG_ReadByte();

	for (;;) {
		str = MSG_ReadString ();
		if (!str[0])
			break;
		n++;
		if (!strcmp(str,"progs/spike.mdl"))
			cl_spikeindex = n;
		if (!strcmp(str,"progs/player.mdl"))
			cl_playerindex = n;
		if (!strcmp(str,"progs/flag.mdl"))
			cl_flagindex = n;
		if (!strncmp(str,"misc/r_tele",11)) {
			if (n < cl_telemin)
				cl_telemin = n;
			if (n > cl_telemax)
				cl_telemax = n;
		}
	}

	MSG_ReadByte();

	DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount+1);
	MSG_WriteByte(msgbuf, type);
	MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
}


/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
Dem_ParseStartSoundPacket
==================
*/
void Dem_ParseStartSoundPacket(void)
{
    int		channel, sound_num, i, j, k;
	vec3_t	pos, dist;
	qboolean found = false;

	msg_startcount = msg_readcount;
	channel = MSG_ReadShort(); 

	if (channel & SND_VOLUME)
		MSG_ReadByte();
	if (channel & SND_ATTENUATION)
		MSG_ReadByte();

	sound_num = MSG_ReadByte();

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();

	if (cl_telemax && sound_num >= cl_telemin && sound_num <= cl_telemax)
	{
		j = from.parsecount;
		k = world.parsecount;
		do {
			if (from.frames[j&UPDATE_MASK].time < world.frames[k&UPDATE_MASK].time)
				k = k == 0 ? 0 : k - 1;

		for (i = 0; i < MAX_CLIENTS; i++)
		{
			if (!from.players[i].name[0])
				continue;
			if (from.players[i].spectator)
				continue;

			//Sys_Printf("%d sound:%d\n", i, sound_num);
			VectorSubtract(pos, from.frames[j&UPDATE_MASK].playerstate[i].origin, dist);
			//Sys_Printf("%d:dist:%f\n", i, dist);
			if (Length(dist) < 30.0 && world.frames[k&UPDATE_MASK].fixangle[i] == false) {
				found = true;
				world.frames[k&UPDATE_MASK].fixangle[i] = true;
				if (sworld.options & O_DEBUG)
					fprintf(sworld.debug.file, "fixangle:%d, %d\n", i, k);
			}
		}
		j--;
		} while (!found && j >= from.parsecount && j >= 0);
	}

	DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
	MSG_WriteByte(msgbuf, svc_sound);
	MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
}       


/*
==================
Dem_ParseClientdata

Server information pertaining to this client only, sent every frame
==================
*/

void Dem_ParseClientdata (void)
{
	int			i;
	frame_t		*frame;
	float		latency;

// calculate simulated time of message

	i = from.netchan.incoming_acknowledged;
	from.parsecount = i;
	i &= UPDATE_MASK;
	from.parsecountmod = i;
	frame = &from.frames[i];
	if (from.format == qwd)
		parsecounttime = from.frames[i].senttime;
	else {
		from.frames[i].senttime = realtime;
		parsecounttime = realtime;
	}


	frame->time = from.time;
	frame->receivedtime = realtime;

// calculate latency
	latency = frame->receivedtime - frame->senttime;

	if (latency < 0 || latency > 1.0)
	{
//		Con_Printf ("Odd latency: %5.2f\n", latency);
	}
	else
	{
	// drift the average latency towards the observed latency
		if (latency < from.latency)
			from.latency = latency;
		else 
			from.latency += 0.001f;
	}

	world.time = frame->senttime + from.latency;
}


qboolean findPlayer(char *name)
{
	int i;

	if (!*name)
		return false;
	
	for (i=0; i < MAX_CLIENTS; i++)
		if (!strcmp(name, world.players[i].name))
			return true;
}


/*
==============
Dem_ParsePrint
==============
*/

void Dem_ParsePrint (void)
{
	char *s, str[128];
	byte level;
	msg_startcount = msg_readcount;

	level = MSG_ReadByte ();
	s = MSG_ReadString ();

	// filters

	if (level < sworld.msglevel)
		return;

	if (sworld.options & O_FC && level == PRINT_CHAT)
		return;
	if (sworld.options & O_FS && level == PRINT_CHAT && !strncmp(s, "[SPEC] ", 7))
		return;
	if (sworld.options & O_FQ && level == PRINT_CHAT && s[0] == '[' && strstr(s, "]: ") != NULL)
		return;
	if (sworld.options & O_FT && level == PRINT_CHAT && s[0] == '(' && strstr(s, "): ") != NULL)
	{
		strncpy(str, s + 1, sizeof(str));
		*(str + ((strstr(s, "): ") - str))) = 0;
		if (findPlayer(str))
			return;
	}

	if (sworld.options & O_LOG)
		fprintf(sworld.log.file, "%s", s);

	DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
	MSG_WriteByte(msgbuf, svc_print);
	MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
}

/*
==============
Dem_UpdateUserinfo
==============
*/

void Dem_UpdateUserinfo (void)
{
	int slot;
	player_info_t *player;

	msg_startcount = msg_readcount;

	slot = MSG_ReadByte ();
	if (slot >= MAX_CLIENTS) {
		Sys_Printf ("ERROR: Dem_ParseDemoMessage: svc_updateuserinfo > MAX_CLIENTS\n");
		Dem_Stop();
		return;
	}

	player = &from.players[slot];
	/*player->userid = */MSG_ReadLong ();

	strncpy (player->userinfo, MSG_ReadString(), sizeof(player->userinfo));
	strncpy (player->name, Info_ValueForKey (player->userinfo, "name"), sizeof(player->name));

	if (Info_ValueForKey (player->userinfo, "*spectator")[0])
		player->spectator = true;
	else
		player->spectator = false;

	DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
	MSG_WriteByte(msgbuf, svc_updateuserinfo);
	MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
}

/*
==================
Dem_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
int	bitcounts[32];	/// just for protocol profiling
void Dem_ParseDelta (entity_state_t *efrom, entity_state_t *eto, int bits)
{
	int			i;

	// set everything to the state we are delta'ing from
	*eto = *efrom;

	eto->number = bits & 511;
	bits &= ~511;

	if (bits & U_MOREBITS)
	{	// read in the low order bits
		i = MSG_ReadByte ();
		bits |= i;
	}

	// count the bits for net profiling
	for (i=0 ; i<16 ; i++)
		if (bits&(1<<i))
			bitcounts[i]++;

	eto->flags = bits;
	
	if (bits & U_MODEL)
		eto->modelindex = MSG_ReadByte ();
		
	if (bits & U_FRAME)
		eto->frame = MSG_ReadByte ();

	if (bits & U_COLORMAP)
		eto->colormap = MSG_ReadByte();

	if (bits & U_SKIN)
		eto->skinnum = MSG_ReadByte();

	if (bits & U_EFFECTS)
		eto->effects = MSG_ReadByte();

	if (bits & U_ORIGIN1)
		eto->origin[0] = MSG_ReadCoord ();
		
	if (bits & U_ANGLE1)
		eto->angles[0] = MSG_ReadAngle();

	if (bits & U_ORIGIN2)
		eto->origin[1] = MSG_ReadCoord ();
		
	if (bits & U_ANGLE2)
		eto->angles[1] = MSG_ReadAngle();

	if (bits & U_ORIGIN3)
		eto->origin[2] = MSG_ReadCoord ();
		
	if (bits & U_ANGLE3)
		eto->angles[2] = MSG_ReadAngle();

	if (bits & U_SOLID)
	{
		// FIXME
	}
}

/*
=================
FlushEntityPacket
=================
*/

void FlushEntityPacket (void)
{
	int			word;
	entity_state_t	olde, newe;

	memset (&olde, 0, sizeof(olde));

	//world.validsequence = 0;		// can't render a frame
	from.frames[from.netchan.incoming_sequence&UPDATE_MASK].invalid = true;

	// read it all, but ignore it
	while (1)
	{
		word = (unsigned short)MSG_ReadShort ();
		if (msg_badread)
		{	// something didn't parse right...
			Sys_Printf ("ERROR:msg_badread in packetentities\n");
			Dem_Stop();
			return;
		}

		if (!word)
			break;	// done

		Dem_ParseDelta (&olde, &newe, word);
	}
}


/*
==================
Dem_ParsePacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/

void Dem_ParsePacketEntities (qboolean delta)
{
	int			oldpacket, newpacket;
	packet_entities_t	*oldp, *newp, dummy;
	int			oldindex, newindex;
	int			word, newnum, oldnum;
	qboolean	full;
	byte		deltafrom;

	newpacket = from.netchan.incoming_sequence&UPDATE_MASK;
	newp = &from.frames[newpacket].packet_entities;
	from.frames[newpacket].invalid = false;

	if (delta)
	{
		deltafrom = MSG_ReadByte ();

		oldpacket = from.frames[newpacket].delta_sequence;
		if (from.format == mvd) {
			oldpacket = (from.netchan.incoming_sequence-1);
		} else {
			if (from.netchan.outgoing_sequence - from.netchan.incoming_sequence >= UPDATE_BACKUP-1)
			{	// there are no valid frames left, so drop it
				FlushEntityPacket ();
				return;
			}

			if (from.netchan.outgoing_sequence - oldpacket >= UPDATE_BACKUP-1)
			{	// we can't use this, it is too old
				FlushEntityPacket ();
				return;
			}
		}

		from.validsequence = from.netchan.incoming_sequence;
		oldp = &from.frames[oldpacket&UPDATE_MASK].packet_entities;
		full = false;
	}
	else
	{	// this is a full update that we can start delta compressing from now
		oldp = &dummy;
		dummy.num_entities = 0;
		from.validsequence = from.netchan.incoming_sequence;
		full = true;
	}

	oldindex = 0;
	newindex = 0;
	newp->num_entities = 0;

	world.validsequence = 1;

	while (1)
	{
		word = (unsigned short)MSG_ReadShort ();
		if (msg_badread)
		{	// something didn't parse right...
			Sys_Printf ("ERROR:msg_badread in packetentities\n");
			Dem_Stop();
			return;
		}

		if (!word)
		{
			while (oldindex < oldp->num_entities)
			{	// copy all the rest of the entities from the old packet
				if (newindex >= MAX_PACKET_ENTITIES)
				{
					Sys_Printf ("Dem_ParsePacketEntities: newindex == MAX_PACKET_ENTITIES\n");
					Dem_Stop();
					return;
				}
				newp->entities[newindex] = oldp->entities[oldindex];
				newindex++;
				oldindex++;
			}
			break;
		}
		newnum = word&511;
		oldnum = oldindex >= oldp->num_entities ? 9999 : oldp->entities[oldindex].number;

		while (newnum > oldnum)
		{
			if (full)
			{
				//Con_Printf ("WARNING: oldcopy on full update");
				FlushEntityPacket ();
				return;
			}

			// copy one of the old entities over to the new packet unchanged
			if (newindex >= MAX_PACKET_ENTITIES) {
				Sys_Printf ("ERROR:Dem_ParsePacketEntities: newindex == MAX_PACKET_ENTITIES\n");
				Dem_Stop();
				return;
			}
			newp->entities[newindex] = oldp->entities[oldindex];
			newindex++;
			oldindex++;
			oldnum = oldindex >= oldp->num_entities ? 9999 : oldp->entities[oldindex].number;
		}

		if (newnum < oldnum)
		{	// new from baseline
			if (word & U_REMOVE)
			{
				if (full)
				{
					//world.validsequence = 0;
					//Con_Printf ("WARNING: U_REMOVE on full update\n");
					FlushEntityPacket ();
					return;
				}
				continue;
			}
			if (newindex >= MAX_PACKET_ENTITIES) {
				Sys_Printf ("ERROR:Dem_ParsePacketEntities: newindex == MAX_PACKET_ENTITIES\n");
				Dem_Stop();
				return;
			}
			Dem_ParseDelta (&baselines[newnum], &newp->entities[newindex], word);
			newindex++;
			continue;
		}

		if (newnum == oldnum)
		{	// delta from previous
			if (full)
			{
				//world.validsequence = 0;
				//Con_Printf ("WARNING: delta on full update");
			}
			if (word & U_REMOVE)
			{
				oldindex++;
				continue;
			}

			Dem_ParseDelta (&oldp->entities[oldindex], &newp->entities[newindex], word);
			newindex++;
			oldindex++;
		}

	}

	newp->num_entities = newindex;
	if (sworld.options & O_DEBUG)
		fprintf(sworld.debug.file, "num_entities:%d\n", newp->num_entities);
}

int TranslateFlags(int src, int to)
{
	int dst = 0;

	if (to == qwd)
	{
		if (src & DF_EFFECTS)
			dst |= PF_EFFECTS;
		if (src & DF_SKINNUM)
			dst |= PF_SKINNUM;
		if (src & DF_DEAD)
			dst |= PF_DEAD;
		if (src & DF_GIB)
			dst |= PF_GIB;
		if (src & DF_WEAPONFRAME)
			dst |= PF_WEAPONFRAME;
		if (src & DF_MODEL)
			dst |= PF_MODEL;

		return dst;
	}

	if (src & PF_EFFECTS)
		dst |= DF_EFFECTS;
	if (src & PF_SKINNUM)
		dst |= DF_SKINNUM;
	if (src & PF_DEAD)
		dst |= DF_DEAD;
	if (src & PF_GIB)
		dst |= DF_GIB;
	if (src & PF_WEAPONFRAME)
		dst |= DF_WEAPONFRAME;
	if (src & PF_MODEL)
		dst |= DF_MODEL;

	return dst;
}

void Dem_ParsePlayerinfo (void)
{
	int			msec = 0;
	int			flags;
	player_info_t	*info;
	player_state_t	*state, *prevstate, dummy;
	int			num;
	int			i;
	//static int model = 0;

	num = MSG_ReadByte ();
	//Sys_Printf("%d:parse:%d\n",from.parsecount, num);
	if (num >= MAX_CLIENTS) {
		Sys_Printf ("ERROR:Dem_ParsePlayerinfo: bad num\n");
		Dem_Stop();
		return;
	}

	info = &from.players[num];

	memset(&dummy, 0, sizeof(dummy));

	state = &from.frames[from.parsecountmod].playerstate[num];
	if (from.prevnum[num] > from.parsecount) {
		from.prevnum[num] = 0;
		prevstate = &dummy;
	} else {
		if (from.prevnum[num] - from.parsecount >= UPDATE_BACKUP-1)
			prevstate = &dummy;
		else 
			prevstate = &from.frames[from.prevnum[num]&UPDATE_MASK].playerstate[num];
	}

	from.prevnum[num] = from.parsecount;

	if (from.format == mvd)
	{
		memcpy(state, prevstate, sizeof(player_state_t));
		flags = MSG_ReadShort ();
		state->flags = 0;

		state->flags = TranslateFlags(flags, qwd);

		//Con_Printf("flags:%i\n", flags);
		state->messagenum = from.parsecount;
		state->command.msec = 0;

		state->frame = MSG_ReadByte ();
		state->command.msec = 0;

		for (i=0; i <3; i++)
			if (flags & (DF_ORIGIN << i)) {
				state->origin[i] = MSG_ReadCoord ();
			}

		for (i=0; i <3; i++)
			if (flags & (DF_ANGLES << i)) {
				state->command.angles[i] = MSG_ReadAngle16 ();
			}

		if (flags & DF_MODEL) {
			state->modelindex = MSG_ReadByte ();
		}

		if (flags & DF_SKINNUM) {
			state->skinnum = MSG_ReadByte ();
		}

		if (flags & DF_EFFECTS) {
			state->effects = MSG_ReadByte ();
		}

		if (flags & DF_WEAPONFRAME) {
			state->weaponframe = MSG_ReadByte ();
		}

		if (from.spectator && num == from.playernum)
			state->messagenum = -1;
		return;
	}

	flags = state->flags = MSG_ReadShort ();

	state->messagenum = from.parsecount;
	state->origin[0] = MSG_ReadCoord ();
	state->origin[1] = MSG_ReadCoord ();
	state->origin[2] = MSG_ReadCoord ();

	state->frame = MSG_ReadByte ();

	// the other player's last move was likely some time
	// before the packet was sent out, so accurately track
	// the exact time it was valid at
	if (flags & PF_MSEC)
	{
		msec = MSG_ReadByte ();
	}

	if (flags & PF_COMMAND)
		MSG_ReadDeltaUsercmd (&nullcmd, &state->command);

	state->command.msec = msec;

	for (i=0 ; i<3 ; i++)
	{
		if (flags & (PF_VELOCITY1<<i) ) {
			state->velocity[i] = MSG_ReadShort();
		} else
			state->velocity[i] = 0;
	}
	if (flags & PF_MODEL) {
		state->modelindex = MSG_ReadByte ();
	} else
		state->modelindex = cl_playerindex;

	if (flags & PF_SKINNUM) {
		state->skinnum = MSG_ReadByte ();
	}
	else
		state->skinnum = 0;

	if (flags & PF_EFFECTS) {
		state->effects = MSG_ReadByte ();
	} else
		state->effects = 0;

	if (flags & PF_WEAPONFRAME) {
		state->weaponframe = MSG_ReadByte ();
	} else
		state->weaponframe = 0;

	if (from.spectator && num == from.playernum)
		state->messagenum = -1;
}

/*
=====================
Dem_ParseProjectiles

Nails are passed as efficient temporary entities
=====================
*/
void Dem_ParseProjectiles (qboolean nails2)
{
	int		i, c, j, num;
	byte	bits[6];
	projectile_t	*pr;
	frame_t	*frame;

	frame = &from.frames[from.parsecountmod];

	c = MSG_ReadByte ();
	for (i=0 ; i<c ; i++)
	{
		if (nails2)
			num = MSG_ReadByte();
		else
			num = 0;

		for (j=0 ; j<6 ; j++)
			bits[j] = MSG_ReadByte ();

		if (frame->num_projectiles == MAX_PROJECTILES)
			continue;

		pr = &frame->projectiles[frame->num_projectiles];
		frame->num_projectiles++;

		pr->modelindex = cl_spikeindex;
		pr->origin[0] = ( ( bits[0] + ((bits[1]&15)<<8) ) <<1) - 4096;
		pr->origin[1] = ( ( (bits[1]>>4) + (bits[2]<<4) ) <<1) - 4096;
		pr->origin[2] = ( ( bits[3] + ((bits[4]&15)<<8) ) <<1) - 4096;
		pr->angles[0] = 360*(bits[4]>>4)/16;
		pr->angles[1] = 360*bits[5]/256;
		pr->num = num;
	}
}

/*
=================
CL_ParseTEnt
=================
*/
void Dem_ParseTEnt (void)
{
	int		type;

	type = MSG_ReadByte ();

	switch (type)
	{
	case TE_LIGHTNING1:
	case TE_LIGHTNING2:
	case TE_LIGHTNING3:
		DemoWrite_Begin(dem_all, 0, 16);
		MSG_WriteByte(msgbuf, svc_temp_entity);
		MSG_WriteByte(msgbuf, type);
		MSG_Forward(msgbuf, msg_readcount, 14);
		break;
	case TE_GUNSHOT:
	case TE_BLOOD:
		DemoWrite_Begin(dem_all, 0, 9);
		MSG_WriteByte(msgbuf, svc_temp_entity);
		MSG_WriteByte(msgbuf, type);
		MSG_Forward(msgbuf, msg_readcount, 7);
		break;
	default:
		DemoWrite_Begin(dem_all, 0, 8);
		MSG_WriteByte(msgbuf, svc_temp_entity);
		MSG_WriteByte(msgbuf, type);
		MSG_Forward(msgbuf, msg_readcount, 6);
		break;
	}
}

/*
==================
Dem_ParseBaseline
==================
*/

void Dem_ParseBaseline (entity_state_t *es)
{
	int			i;
	
	es->modelindex = MSG_ReadByte ();
	es->frame = MSG_ReadByte ();
	es->colormap = MSG_ReadByte();
	es->skinnum = MSG_ReadByte();
	for (i=0 ; i<3 ; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}
}

#define SHOWNET(x) if (sworld.options & O_DEBUG && cmd < 60) fprintf(sworld.debug.file, "%3i:%s\n", msg_readcount-1, x);
/*
=====================
Dem_ParseServerMessage
=====================
*/
int	received_framecount;
void Dem_ParseDemoMessage (void)
{
	int			cmd, oldcmd;
	int			i;
	char		*s;
	extern sizebuf_t stats_msg;

//
// if recording demos, copy the message out
//

	Dem_ParseClientdata ();
	from.frames[from.parsecountmod].num_projectiles = 0;

//
// parse the message
//
	while (1)
	{
		if (!from.running)
			return;

		if (msg_badread)
		{
			Sys_Printf ("ERROR:Dem_ParseDemoMessage: Bad demo message\n");
			Dem_Stop();
			return;
		}

		oldcmd = cmd;

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}

		SHOWNET(svc_strings[cmd]);
	
	// other commands
		switch (cmd)
		{
		default:
			Sys_Printf ("ERROR:Dem_ParseDemoMessage: Illegible demo message %d(prev=%d)\n", cmd, oldcmd);
			Dem_Stop();
			return;
			
		case svc_nop:
			DemoWrite_Begin(dem_all, 0, 1);
			MSG_WriteByte(msgbuf, cmd);
			break;
			
		case svc_disconnect:
			Dem_Stop();
			return;

		case svc_print:
			Dem_ParsePrint ();
			break;
			
		case svc_centerprint:
			s = MSG_ReadString();

			DemoWrite_Begin(dem_all, 0, strlen(s)+2);
			MSG_WriteByte(msgbuf, cmd);
			MSG_WriteString(msgbuf, s);
			break;
			
		case svc_stufftext:
			msg_startcount = msg_readcount;

			s = MSG_ReadString ();

			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "text:%s\n", s);

			if (!strncmp(s, "reconnect", 9)) {
				Dem_Stop();
				return;
			}

			if (!strncmp(s, "changing", 8)) {
				break;
			}

			if (msgmax != MAX_MSGLEN)
				DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
			else
				DemoWrite_Begin(from.type, from.to, msg_readcount - msg_startcount + 1);
			MSG_WriteByte(msgbuf, svc_stufftext);
			MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
			break;
			
		case svc_damage:
			DemoWrite_Begin(from.type, To(), 9);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 8);
			break;

		case svc_serverdata:
			Dem_ParseServerData ();
			break;
			
		case svc_setangle:
			DemoWrite_Begin(dem_all, 0, 5);
			MSG_WriteByte(msgbuf, cmd);
			if (from.format == qwd) {
				MSG_WriteByte(msgbuf, To());
				for (i=0 ; i<3 ; i++)
					from.frames[from.parsecountmod].playerstate[from.to].command.angles[i] = MSG_ReadAngle ();

				//world.frames[world.parsecount&UPDATE_MASK].fixangle[from.to] = true;
				
				for (i=0 ; i<3 ; i++)
					MSG_WriteAngle(msgbuf, from.frames[from.parsecountmod].playerstate[from.to].command.angles[i]);
				
				//MSG_Forward(msgbuf, msg_readcount, 3);
			} else {
				MSG_Forward(msgbuf, msg_readcount, 4);
			}
			world.frames[world.parsecount&UPDATE_MASK].fixangle[from.to] = -1;

			//world.players[from.to].fixangle = false;
			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "setangle:%d\n", from.to);
			break;
			
		case svc_lightstyle:
			msg_startcount = msg_readcount;
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES) {
				Sys_Printf ("svc_lightstyle > MAX_LIGHTSTYLES\n");
				Dem_Stop();
				return;
			}
			strcpy (lightstyle[i].map,  MSG_ReadString());
			lightstyle[i].length = strlen(lightstyle[i].map);

			DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
			break;
			
		case svc_sound:
			Dem_ParseStartSoundPacket();
			break;
			
		case svc_stopsound:
			DemoWrite_Begin(dem_all, 0, 3);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 2);
			break;
		
		case svc_updatefrags:
			DemoWrite_Begin(dem_all, 0, 4);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 3);
			break;			

		case svc_updateping:
			DemoWrite_Begin(dem_all, 0, 4);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 3);
			break;
			
		case svc_updatepl:
			DemoWrite_Begin(dem_all, 0, 3);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 2);
			break;
			
		case svc_updateentertime:
			DemoWrite_Begin(dem_all, 0, 6);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 5);
			break;
			
		case svc_spawnbaseline:
			msg_startcount = msg_readcount;
			i = MSG_ReadShort ();
			Dem_ParseBaseline (&baselines[i]);

			DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
			break;
		case svc_spawnstatic:
			DemoWrite_Begin(dem_all, 0, 14);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 13);
			break;			
		case svc_temp_entity:
			Dem_ParseTEnt ();
			break;

		case svc_killedmonster:
			DemoWrite_Begin(from.type, To(), 1);
			MSG_WriteByte(msgbuf, cmd);
			break;

		case svc_foundsecret:
			DemoWrite_Begin(from.type, To(), 1);
			MSG_WriteByte(msgbuf, cmd);
			break;

		case svc_updatestat:
			if (from.spectator) {
				MSG_WriteByte(&stats_msg, cmd);
				MSG_Forward(&stats_msg, msg_readcount, 2);
				break;
			}

			DemoWrite_Begin(dem_stats, To(), 3);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 2);
			break;
		case svc_updatestatlong:
			if (from.spectator) {
				MSG_WriteByte(&stats_msg, cmd);
				MSG_Forward(&stats_msg, msg_readcount, 5);
				break;
			}
			DemoWrite_Begin(dem_stats, To(), 6);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 5);
			break;
			
		case svc_spawnstaticsound:
			DemoWrite_Begin(dem_all, 0, 10);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 9);
			break;

		case svc_cdtrack:
			DemoWrite_Begin(dem_all, 0, 2);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 1);
			break;

		case svc_intermission:
			DemoWrite_Begin(dem_all, 0, 10);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 9);
			break;

		case svc_finale:
			s = MSG_ReadString();

			DemoWrite_Begin(dem_all, 0, strlen(s)+2);
			MSG_WriteByte(msgbuf, cmd);
			MSG_WriteString(msgbuf, s);
			break;
			
		case svc_sellscreen:
			DemoWrite_Begin(dem_all, 0, 1);
			MSG_WriteByte(msgbuf, cmd);
			break;

		case svc_smallkick:
			DemoWrite_Begin(from.type, To(), 1);
			MSG_WriteByte(msgbuf, cmd);
			break;
		case svc_bigkick:
			DemoWrite_Begin(from.type, To(), 1);
			MSG_WriteByte(msgbuf, cmd);
			break;

		case svc_muzzleflash:
			DemoWrite_Begin(dem_all, 0, 3);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 2);
			break;

		case svc_updateuserinfo:
			Dem_UpdateUserinfo();
			break;

		case svc_setinfo:
			msg_startcount = msg_readcount;

			MSG_ReadByte ();
			MSG_ReadString();
			MSG_ReadString();

			DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
			break;

		case svc_serverinfo:
			msg_startcount = msg_readcount;
	
			MSG_ReadString();
			MSG_ReadString();

			DemoWrite_Begin(dem_all, 0, msg_readcount - msg_startcount + 1);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_startcount, msg_readcount - msg_startcount);
			break;

		case svc_download:
			Dem_ParseDownload ();
			break;

		case svc_playerinfo:
			Dem_ParsePlayerinfo ();
			break;

		case svc_nails:
			Dem_ParseProjectiles (false);
			break;
		case svc_nails2:
			Dem_ParseProjectiles (true);
			break;

		case svc_chokecount:		// some preceding packets were choked
			DemoWrite_Begin(from.type, from.to, 2);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 1);
			break;

		case svc_modellist:
			Dem_Parselist ((byte)cmd);
			break;

		case svc_soundlist:
			Dem_Parselist ((byte)cmd);
			break;

		case svc_packetentities:
			Dem_ParsePacketEntities (false);
			break;

		case svc_deltapacketentities:
			Dem_ParsePacketEntities (true);
			break;

		case svc_maxspeed :
			DemoWrite_Begin(dem_all, 0, 5);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 4);
			break;

		case svc_entgravity :
			DemoWrite_Begin(dem_all, 0, 5);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 4);
			break;

		case svc_setpause:
			DemoWrite_Begin(dem_all, 0, 2);
			MSG_WriteByte(msgbuf, cmd);
			MSG_Forward(msgbuf, msg_readcount, 1);
			break;
		case svc_qizmomsg:
			// ignore it
			return;
		}
	}
}
