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

#include "qwsvdef.h"

#define CHAN_AUTO   0
#define CHAN_WEAPON 1
#define CHAN_VOICE  2
#define CHAN_ITEM   3
#define CHAN_BODY   4

/*
=============================================================================

Con_Printf redirection

=============================================================================
*/

#define MAX_REDIRECTMESSAGES 4
char	outputbuf[8000];

redirect_t	sv_redirected;
static int	sv_redirectbufcount;

extern cvar_t sv_phs;

/*
==================
SV_FlushRedirect
==================
*/
void SV_FlushRedirect (void)
{
	char	send[8000+6];

	if (sv_redirected == RD_PACKET)
	{
		send[0] = 0xff;
		send[1] = 0xff;
		send[2] = 0xff;
		send[3] = 0xff;
		send[4] = A2C_PRINT;
		memcpy (send+5, outputbuf, strlen(outputbuf)+1);

		NET_SendPacket (net_serversocket, strlen(send)+1, send, net_from);
	}
	else if (sv_redirected == RD_CLIENT && sv_redirectbufcount < MAX_REDIRECTMESSAGES)
	{
		ClientReliableWrite_Begin (host_client, svc_print, strlen(outputbuf)+3);
		ClientReliableWrite_Byte (host_client, PRINT_HIGH);
		ClientReliableWrite_String (host_client, outputbuf);
		sv_redirectbufcount++;
	} else if (sv_redirected == RD_MOD) {
		//return;
	} else if (sv_redirected > RD_MOD && sv_redirectbufcount < MAX_REDIRECTMESSAGES)
	{
		client_t *cl;

		cl = svs.clients + sv_redirected - RD_MOD - 1;

		if (cl->state == cs_spawned)
		{
			ClientReliableWrite_Begin (cl, svc_print, strlen(outputbuf)+3);
			ClientReliableWrite_Byte (cl, PRINT_HIGH);
			ClientReliableWrite_String (cl, outputbuf);
			sv_redirectbufcount++;
		}
	}

	// clear it
	outputbuf[0] = 0;
}


/*
==================
SV_BeginRedirect

  Send Con_Printf data to the remote client
  instead of the console
==================
*/
void SV_BeginRedirect (redirect_t rd)
{
	sv_redirected = rd;
	outputbuf[0] = 0;
	sv_redirectbufcount = 0;
	
}

void SV_EndRedirect (void)
{
	SV_FlushRedirect ();
	sv_redirected = RD_NONE;
}



/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/
#define	MAXPRINTMSG	4096
// FIXME: make a buffer size safe vs<n>printf? - FIXED.
void Con_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	
	va_start (argptr,fmt);
	vsnprintf (msg, MAXPRINTMSG, fmt, argptr);
	va_end (argptr);

	// add to redirected message
	if (sv_redirected)
	{
		if (strlen (msg) + strlen(outputbuf) > /*sizeof(outputbuf) - 1*/ MAX_MSGLEN - 10)
			SV_FlushRedirect ();
		strlcat (outputbuf, msg, sizeof(outputbuf));
		return;
	}

	Sys_Printf ("%s", msg);	// also echo to debugging console
	SV_Write_Log(CONSOLE_LOG, 1, msg);

	// dumb error message to log file if 
	if (sv_error)
		SV_Write_Log(ERROR_LOG, 1, msg);
}

/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
void Con_DPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if (!developer.value)
		return;

	va_start (argptr,fmt);
	vsnprintf (msg, MAXPRINTMSG, fmt, argptr);
	va_end (argptr);
	
	Con_Printf ("%s", msg);
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

static void SV_PrintToClient(client_t *cl, int level, char *string)
{
	ClientReliableWrite_Begin (cl, svc_print, strlen(string)+3);
	ClientReliableWrite_Byte (cl, level);
	ClientReliableWrite_String (cl, string);
}


/*
=================
SV_ClientPrintf

Sends text across to be displayed if the level passes
=================
*/
void SV_ClientPrintf (client_t *cl, int level, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	if (level < cl->messagelevel)
		return;

	va_start (argptr,fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	if (sv.demorecording) {

		DemoWrite_Begin (dem_single, cl - svs.clients, strlen(string)+3);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_print);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, level);
		MSG_WriteString ((sizebuf_t*)demo.dbuf, string);
	}

	SV_PrintToClient(cl, level, string);
}

void SV_ClientPrintf2 (client_t *cl, int level, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	if (level < cl->messagelevel)
		return;

	va_start (argptr,fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	SV_PrintToClient(cl, level, string);
}


/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (int level, char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	client_t	*cl;
	int			i;

	va_start (argptr,fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	Sys_Printf ("%s", string);	// print to the console

	for (i=0, cl = svs.clients ; i<MAX_CLIENTS ; i++, cl++)
	{
		if (level < cl->messagelevel)
			continue;
		if (!cl->state)
			continue;

		SV_PrintToClient(cl, level, string);
	}

	if (sv.demorecording) {
		DemoWrite_Begin (dem_all, 0, strlen(string)+3);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_print);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, level);
		MSG_WriteString ((sizebuf_t*)demo.dbuf, string);
	}
}

/*
=================
SV_BroadcastCommand

Sends text to all active clients
=================
*/
void SV_BroadcastCommand (char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	
	if (!sv.state)
		return;
	va_start (argptr,fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&sv.reliable_datagram, svc_stufftext);
	MSG_WriteString (&sv.reliable_datagram, string);
}


/*
=================
SV_Multicast

Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.

MULTICAST_ALL	same as broadcast
MULTICAST_PVS	send to clients potentially visible from org
MULTICAST_PHS	send to clients potentially hearable from org
=================
*/
void SV_Multicast (vec3_t origin, int to)
{
	client_t	*client;
	byte		*mask;
	mleaf_t		*leaf;
	int			leafnum;
	int			j;
	qboolean	reliable;

	leaf = Mod_PointInLeaf (origin, sv.worldmodel);
	if (!leaf)
		leafnum = 0;
	else
		leafnum = leaf - sv.worldmodel->leafs;

	reliable = false;

	switch (to)
	{
	case MULTICAST_ALL_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_ALL:
		mask = sv.pvs;		// leaf 0 is everything;
		break;

	case MULTICAST_PHS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PHS:
		mask = sv.phs + leafnum * 4*((sv.worldmodel->numleafs+31)>>5);
		break;

	case MULTICAST_PVS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PVS:
		mask = sv.pvs + leafnum * 4*((sv.worldmodel->numleafs+31)>>5);
		break;

	default:
		mask = NULL;
		SV_Error ("SV_Multicast: bad to:%i", to);
	}

	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		if (to == MULTICAST_PHS_R || to == MULTICAST_PHS) {
			vec3_t delta;
			VectorSubtract(origin, client->edict->v.origin, delta);
			if (Length(delta) <= 1024)
				goto inrange;
		}

		leaf = Mod_PointInLeaf (client->edict->v.origin, sv.worldmodel);
		if (leaf)
		{
			// -1 is because pvs rows are 1 based, not 0 based like leafs
			leafnum = leaf - sv.worldmodel->leafs - 1;
			if ( !(mask[leafnum>>3] & (1<<(leafnum&7)) ) )
			{
//				Con_Printf ("supressed multicast\n");
				continue;
			}
		}

inrange:
		if (reliable) {
			ClientReliableCheckBlock(client, sv.multicast.cursize);
			ClientReliableWrite_SZ(client, sv.multicast.data, sv.multicast.cursize);
		} else
			SZ_Write (&client->datagram, sv.multicast.data, sv.multicast.cursize);
	}

	if (sv.demorecording)
	{
		if (reliable)
		{
			//DemoWrite_Begin(dem_multiple, cls, sv.multicast.cursize);
			DemoWrite_Begin(dem_all, 0, sv.multicast.cursize);
			SZ_Write((sizebuf_t*)demo.dbuf, sv.multicast.data, sv.multicast.cursize);
		} else 
			SZ_Write(&demo.datagram, sv.multicast.data, sv.multicast.cursize);
	}


	SZ_Clear (&sv.multicast);
}


/*  
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)

==================
*/  
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,
    float attenuation)
{       
    int         sound_num;
    int			field_mask;
    int			i;
	int			ent;
	vec3_t		origin;
	qboolean	use_phs;
	qboolean	reliable = false;

	if (volume < 0 || volume > 255)
		SV_Error ("SV_StartSound: volume = %i", volume);

	if (attenuation < 0 || attenuation > 4)
		SV_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (channel < 0 || channel > 15)
		SV_Error ("SV_StartSound: channel = %i", channel);

// find precache number for sound
    for (sound_num=1 ; sound_num<MAX_SOUNDS
        && sv.sound_precache[sound_num] ; sound_num++)
        if (!strcmp(sample, sv.sound_precache[sound_num]))
            break;
    
    if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
    {
        Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
        return;
    }
    
	ent = NUM_FOR_EDICT(entity);

	if ((channel & 8) || !sv_phs.value)	// no PHS flag
	{
		if (channel & 8)
			reliable = true; // sounds that break the phs are reliable
		use_phs = false;
		channel &= 7;
	}
	else
		use_phs = true;

//	if (channel == CHAN_BODY || channel == CHAN_VOICE)
//		reliable = true;

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		channel |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		channel |= SND_ATTENUATION;

	// use the entity origin unless it is a bmodel
	if (entity->v.solid == SOLID_BSP)
	{
		for (i=0 ; i<3 ; i++)
			origin[i] = entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]);
	}
	else
	{
		VectorCopy (entity->v.origin, origin);
	}

	MSG_WriteByte (&sv.multicast, svc_sound);
	MSG_WriteShort (&sv.multicast, channel);
	if (channel & SND_VOLUME)
		MSG_WriteByte (&sv.multicast, volume);
	if (channel & SND_ATTENUATION)
		MSG_WriteByte (&sv.multicast, attenuation*64);
	MSG_WriteByte (&sv.multicast, sound_num);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord (&sv.multicast, origin[i]);

	if (use_phs)
		SV_Multicast (origin, reliable ? MULTICAST_PHS_R : MULTICAST_PHS);
	else
		SV_Multicast (origin, reliable ? MULTICAST_ALL_R : MULTICAST_ALL);
}           


/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

int		sv_nailmodel, sv_supernailmodel, sv_playermodel;

void SV_FindModelNumbers (void)
{
	int		i;

	sv_nailmodel = -1;
	sv_supernailmodel = -1;
	sv_playermodel = -1;

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
			break;
		if (!strcmp(sv.model_precache[i],"progs/spike.mdl"))
			sv_nailmodel = i;
		if (!strcmp(sv.model_precache[i],"progs/s_spike.mdl"))
			sv_supernailmodel = i;
		if (!strcmp(sv.model_precache[i],"progs/player.mdl"))
			sv_playermodel = i;
	}
}


/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (client_t *client, sizebuf_t *msg)
{
	int		i, clnum;
	edict_t	*other;
	edict_t	*ent;

	ent = client->edict;

	clnum = NUM_FOR_EDICT(ent) - 1;

	// send the chokecount for r_netgraph
	if (client->chokecount)
	{
		MSG_WriteByte (msg, svc_chokecount);
		MSG_WriteByte (msg, client->chokecount);
		client->chokecount = 0;
	}

	// send a damage message if the player got hit this frame
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, ent->v.dmg_save);
		MSG_WriteByte (msg, ent->v.dmg_take);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i]));
	
		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

	// add this to server demo
	if (sv.demorecording && msg->cursize)
	{
		DemoWrite_Begin(dem_single, clnum, msg->cursize);
		SZ_Write((sizebuf_t*)demo.dbuf, msg->data, msg->cursize);
	}

	// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle)
	{
		MSG_WriteByte (msg, svc_setangle);
		for (i=0 ; i < 3 ; i++)
			MSG_WriteAngle (msg, ent->v.angles[i] );
		VectorCopy(ent->v.angles, demo.angles[clnum]);
		ent->v.fixangle = 0;
		demo.fixangle[clnum] = true;

		if (sv.demorecording)
		{
			MSG_WriteByte (&demo.datagram, svc_setangle);
			MSG_WriteByte (&demo.datagram, clnum);
			for (i=0 ; i < 3 ; i++)
				MSG_WriteAngle (&demo.datagram, demo.angles[clnum][i] );
		}
	}
}

/*
=======================
SV_UpdateClientStats

Performs a delta update of the stats array.  This should only be performed
when a reliable message can be delivered this frame.
=======================
*/
void SV_UpdateClientStats (client_t *client)
{
	edict_t	*ent;
	int		stats[MAX_CL_STATS];
	int		i;
	
	ent = client->edict;
	memset (stats, 0, sizeof(stats));
	
	// if we are a spectator and we are tracking a player, we get his stats
	// so our status bar reflects his
	if (client->spectator && client->spec_track > 0)
		ent = svs.clients[client->spec_track - 1].edict;

	stats[STAT_HEALTH] = ent->v.health;
	stats[STAT_WEAPON] = SV_ModelIndex(PR_GetString(ent->v.weaponmodel));
	stats[STAT_AMMO] = ent->v.currentammo;
	stats[STAT_ARMOR] = ent->v.armorvalue;
	stats[STAT_SHELLS] = ent->v.ammo_shells;
	stats[STAT_NAILS] = ent->v.ammo_nails;
	stats[STAT_ROCKETS] = ent->v.ammo_rockets;
	stats[STAT_CELLS] = ent->v.ammo_cells;
	if (!client->spectator || client->spec_track > 0)
		stats[STAT_ACTIVEWEAPON] = ent->v.weapon;
	// stuff the sigil bits into the high bits of items for sbar
	stats[STAT_ITEMS] = (int)ent->v.items | ((int)pr_global_struct->serverflags << 28);

	for (i=0 ; i<MAX_CL_STATS ; i++)
		if (stats[i] != client->stats[i])
		{
			client->stats[i] = stats[i];
			if (stats[i] >=0 && stats[i] <= 255)
			{
				ClientReliableWrite_Begin(client, svc_updatestat, 3);
				ClientReliableWrite_Byte(client, i);
				ClientReliableWrite_Byte(client, stats[i]);
			}
			else
			{
				ClientReliableWrite_Begin(client, svc_updatestatlong, 6);
				ClientReliableWrite_Byte(client, i);
				ClientReliableWrite_Long(client, stats[i]);
			}
		}
}

/*
=======================
SV_SendClientDatagram
=======================
*/
qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;


	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;
	msg.allowoverflow = true;
	msg.overflowed = false;

	// for faster downloading skip half the frames
	/*if (client->download && client->netchan.outgoing_sequence & 1) 
	{
		// we're sending fake invalid delta update, so that client won't update screen
		MSG_WriteByte (&msg, svc_deltapacketentities);
		MSG_WriteByte (&msg, 0);
		MSG_WriteShort (&msg, 0);

		Netchan_Transmit (&client->netchan, msg.cursize, buf);
	}
	*/

	
	// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client, &msg);

	// send over all the objects that are in the PVS
	// this will include clients, a packetentities, and
	// possibly a nails update
	SV_WriteEntitiesToClient (client, &msg, false);

	// copy the accumulated multicast datagram
	// for this client out to the message
	if (client->datagram.overflowed)
		Con_Printf ("WARNING: datagram overflowed for %s\n", client->name);
	else
		SZ_Write (&msg, client->datagram.data, client->datagram.cursize);
	SZ_Clear (&client->datagram);

	// send deltas over reliable stream
	if (Netchan_CanReliable (&client->netchan))
		SV_UpdateClientStats (client);

	if (msg.overflowed)
	{
		Con_Printf ("WARNING: msg overflowed for %s\n", client->name);
		SZ_Clear (&msg);
	}

	// send the datagram
	Netchan_Transmit (&client->netchan, msg.cursize, buf);

	return true;
}

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
void SV_UpdateToReliableMessages (void)
{
	int			i, j, cls;
	client_t *client;
	eval_t *val;
	edict_t *ent;

// check for changes to be sent over the reliable streams to all clients
	for (i=0, host_client = svs.clients ; i<MAX_CLIENTS ; i++, host_client++)
	{
		if (host_client->state != cs_spawned)
			continue;
		cls |= 1 << i;

		if (host_client->sendinfo)
		{
			host_client->sendinfo = false;
			SV_FullClientUpdate (host_client, &sv.reliable_datagram);
		}
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			cls = 0;
			for (j=0, client = svs.clients ; j<MAX_CLIENTS ; j++, client++)
			{
				if (client->state < cs_preconnected)
					continue;
				ClientReliableWrite_Begin(client, svc_updatefrags, 4);
				ClientReliableWrite_Byte(client, i);
				ClientReliableWrite_Short(client, host_client->edict->v.frags);
			}
			
			if (sv.demorecording) {
				DemoWrite_Begin(dem_all, 0, 4);
				MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updatefrags);
				MSG_WriteByte((sizebuf_t*)demo.dbuf, i);
				MSG_WriteShort((sizebuf_t*)demo.dbuf, host_client->edict->v.frags);
			}

			host_client->old_frags = host_client->edict->v.frags;
		}

		// maxspeed/entgravity changes
		ent = host_client->edict;

		val = GetEdictFieldValue(ent, "gravity");
		if (val && host_client->entgravity != val->_float) {
			host_client->entgravity = val->_float;
			ClientReliableWrite_Begin(host_client, svc_entgravity, 5);
			ClientReliableWrite_Float(host_client, host_client->entgravity);
			if (sv.demorecording)
			{
				DemoWrite_Begin(dem_single, i, 5);
				MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_entgravity);
				MSG_WriteFloat((sizebuf_t*)demo.dbuf, host_client->entgravity);
			}
		}
		val = GetEdictFieldValue(ent, "maxspeed");
		if (val && host_client->maxspeed != val->_float) {
			host_client->maxspeed = val->_float;
			ClientReliableWrite_Begin(host_client, svc_maxspeed, 5);
			ClientReliableWrite_Float(host_client, host_client->maxspeed);
			if (sv.demorecording)
			{
				DemoWrite_Begin(dem_single, i, 5);
				MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_maxspeed);
				MSG_WriteFloat((sizebuf_t*)demo.dbuf, host_client->maxspeed);
			}
		}

	}

	if (sv.datagram.overflowed)
		SZ_Clear (&sv.datagram);

	// append the broadcast messages to each client messages
	cls = 0;
	for (j=0, client = svs.clients ; j<MAX_CLIENTS ; j++, client++)
	{
		if (client->state < cs_preconnected)
			continue;	// reliables go to all connected or spawned

		ClientReliableCheckBlock(client, sv.reliable_datagram.cursize);
		ClientReliableWrite_SZ(client, sv.reliable_datagram.data, sv.reliable_datagram.cursize);

		if (client->state != cs_spawned)
			continue;	// datagrams only go to spawned

		SZ_Write (&client->datagram
			, sv.datagram.data
			, sv.datagram.cursize);
	}

	if (sv.demorecording && sv.reliable_datagram.cursize) {
		DemoWrite_Begin(dem_all, 0, sv.reliable_datagram.cursize);
		SZ_Write((sizebuf_t*)demo.dbuf, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}
		
	if (sv.demorecording)
		SZ_Write(&demo.datagram, sv.datagram.data, sv.datagram.cursize); // FIZME: ???

	SZ_Clear (&sv.reliable_datagram);
	SZ_Clear (&sv.datagram);
}

#ifdef _WIN32
#pragma optimize( "", off )
#endif



/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i, j;
	client_t	*c;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i=0, c = svs.clients ; i<MAX_CLIENTS ; i++, c++)
	{
		if (!c->state)
			continue;

		if (c->drop) {
			SV_DropClient(c);
			c->drop = false;
			continue;
		}

		// check to see if we have a backbuf to stick in the reliable
		if (c->num_backbuf) {
			// will it fit?
			if (c->netchan.message.cursize + c->backbuf_size[0] <
				c->netchan.message.maxsize) {

				Con_DPrintf("%s: backbuf %d bytes\n",
					c->name, c->backbuf_size[0]);

				// it'll fit
				SZ_Write(&c->netchan.message, c->backbuf_data[0],
					c->backbuf_size[0]);
				
				//move along, move along
				for (j = 1; j < c->num_backbuf; j++) {
					memcpy(c->backbuf_data[j - 1], c->backbuf_data[j],
						c->backbuf_size[j]);
					c->backbuf_size[j - 1] = c->backbuf_size[j];
				}

				c->num_backbuf--;
				if (c->num_backbuf) {
					memset(&c->backbuf, 0, sizeof(c->backbuf));
					c->backbuf.data = c->backbuf_data[c->num_backbuf - 1];
					c->backbuf.cursize = c->backbuf_size[c->num_backbuf - 1];
					c->backbuf.maxsize = sizeof(c->backbuf_data[c->num_backbuf - 1]);
				}
			}
		}

		// if the reliable message overflowed,
		// drop the client
		if (c->netchan.message.overflowed)
		{
			SZ_Clear (&c->netchan.message);
			SZ_Clear (&c->datagram);
			SV_BroadcastPrintf (PRINT_HIGH, "%s overflowed\n", c->name);
			Con_Printf ("WARNING: reliable overflow for %s\n",c->name);
			SV_DropClient (c);
			c->send_message = true;
			c->netchan.cleartime = 0;	// don't choke this message
		}

		// only send messages if the client has sent one
		// and the bandwidth is not choked
		if (!c->send_message)
			continue;
		c->send_message = false;	// try putting this after choke?
		if (!sv.paused && !Netchan_CanPacket (&c->netchan))
		{
			c->chokecount++;
			continue;		// bandwidth choke
		}

		if (c->state == cs_spawned)
			SV_SendClientDatagram (c);
		else
			Netchan_Transmit (&c->netchan, 0, NULL);	// just update reliable
			
	}
}

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
void DemoSetMsgBuf(demobuf_t *prev,demobuf_t *cur);
void SV_SendDemoMessage(void)
{
	int			i, j, cls = 0;
	client_t	*c;
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;
	edict_t		*ent;
	int			stats[MAX_CL_STATS];
	float		min_fps;
	extern		cvar_t sv_demofps;
	extern		cvar_t sv_demoPings;
	extern		cvar_t	sv_demoMaxSize;

	if (!sv.demorecording)
		return;

	if (sv_demoPings.value)
	{
		if (sv.time - demo.pingtime > sv_demoPings.value)
		{
			SV_DemoPings();
			demo.pingtime = sv.time;
		}
	}


	if (!sv_demofps.value)
		min_fps = 20.0;
	else
		min_fps = sv_demofps.value;

	min_fps = max(4, min_fps);

	if (!demo.forceFrame && (sv.time - demo.time < 1.0/min_fps))
		return;

	demo.forceFrame = 0;

	for (i=0, c = svs.clients ; i<MAX_CLIENTS ; i++, c++)
	{
		if (c->state != cs_spawned)
			continue;	// datagrams only go to spawned

		cls |= 1 << i;
	}

	if (!cls) {
		SZ_Clear (&demo.datagram);
		return;
	}

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg.cursize = 0;
	msg.allowoverflow = true;
	msg.overflowed = false;
	
	for (i=0, c = svs.clients ; i<MAX_CLIENTS ; i++, c++)
	{
		if (c->state != cs_spawned)
			continue;	// datagrams only go to spawned

		if (c->spectator)
			continue;

		ent = c->edict;
		memset (stats, 0, sizeof(stats));

		stats[STAT_HEALTH] = ent->v.health;
		stats[STAT_WEAPON] = SV_ModelIndex(PR_GetString(ent->v.weaponmodel));
		stats[STAT_AMMO] = ent->v.currentammo;
		stats[STAT_ARMOR] = ent->v.armorvalue;
		stats[STAT_SHELLS] = ent->v.ammo_shells;
		stats[STAT_NAILS] = ent->v.ammo_nails;
		stats[STAT_ROCKETS] = ent->v.ammo_rockets;
		stats[STAT_CELLS] = ent->v.ammo_cells;
		stats[STAT_ACTIVEWEAPON] = ent->v.weapon;
		

		// stuff the sigil bits into the high bits of items for sbar
		stats[STAT_ITEMS] = (int)ent->v.items | ((int)pr_global_struct->serverflags << 28);

		for (j=0 ; j<MAX_CL_STATS ; j++)
			if (stats[j] != demo.stats[i][j])
			{
				demo.stats[i][j] = stats[j];
				if (stats[j] >=0 && stats[j] <= 255)
				{
					DemoWrite_Begin(dem_stats, i, 3);
					MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updatestat);
					MSG_WriteByte((sizebuf_t*)demo.dbuf, j);
					MSG_WriteByte((sizebuf_t*)demo.dbuf, stats[j]);
				}
				else
				{
					DemoWrite_Begin(dem_stats, i, 6);
					MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updatestatlong);
					MSG_WriteByte((sizebuf_t*)demo.dbuf, j);
					MSG_WriteLong((sizebuf_t*)demo.dbuf, stats[j]);
				}
			}
	}

	// send over all the objects that are in the PVS
	// this will include clients, a packetentities, and
	// possibly a nails update
	msg.cursize = 0;
	if (!demo.recorder.delta_sequence)
		demo.recorder.delta_sequence = -1;
	SV_WriteEntitiesToClient (&demo.recorder, &msg, true);
	DemoWrite_Begin(dem_all, 0, msg.cursize);
	SZ_Write ((sizebuf_t*)demo.dbuf, msg.data, msg.cursize);
	// copy the accumulated multicast datagram
	// for this client out to the message
	if (demo.datagram.cursize) {
		DemoWrite_Begin(dem_all, 0, demo.datagram.cursize);
		SZ_Write ((sizebuf_t*)demo.dbuf, demo.datagram.data, demo.datagram.cursize);
		SZ_Clear (&demo.datagram);
	}

	demo.recorder.delta_sequence = demo.recorder.netchan.incoming_sequence&255;
	demo.recorder.netchan.incoming_sequence++;
	demo.frames[demo.parsecount&DEMO_FRAMES_MASK].time = demo.time = sv.time;

	if (demo.parsecount - demo.lastwritten > 60) // that's a backup of 3sec in 20fps, should be enough
	{
		SV_DemoWritePackets(1);
	}

	demo.parsecount++;
	DemoSetMsgBuf(demo.dbuf,&demo.frames[demo.parsecount&DEMO_FRAMES_MASK].buf);

	if (sv_demoMaxSize.value && demo.size > sv_demoMaxSize.value*1024)
		SV_Stop(1);
}


#ifdef _WIN32
#pragma optimize( "", on )
#endif



/*
=======================
SV_SendMessagesToAll

FIXME: does this sequence right?
=======================
*/
void SV_SendMessagesToAll (void)
{
	int			i;
	client_t	*c;

	for (i=0, c = svs.clients ; i<MAX_CLIENTS ; i++, c++)
		if (c->state)		// FIXME: should this only send to active?
			c->send_message = true;
	
	SV_SendClientMessages ();
}

