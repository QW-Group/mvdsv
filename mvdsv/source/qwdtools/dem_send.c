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

#define ISDEAD(i) ( (i) >=41 && (i) <=102 )

void Interpolate(int num, frame_t *frame, demoinfo_t *demoinfo)
{
	float	exacttime, nexttime, f;
	int		i,j;
	frame_t	*nextframe;
	qbool good;
	player_state_t *state, *nextstate, *prevstate;

	good = false;

	if (frame->fixangle[num] == true)
	{
		VectorCopy(frame->playerstate[num].origin, demoinfo->origin);
		VectorCopy(frame->playerstate[num].command.angles, demoinfo->angles);
		return;
	}

	i = world.lastwritten;
	prevstate = nextstate = state = &frame->playerstate[num];
	nexttime = exacttime = frame->time - (float)(state->msec)*0.001;

	if (exacttime > frame->time)
	{
		while (exacttime > frame->time && i > 0)
		{
			good = false;
			i--;

			nextstate = prevstate;
			nexttime = exacttime;
			nextframe = &world.frames[i&UPDATE_MASK];
			prevstate = &nextframe->playerstate[num];

			if (prevstate->messagenum > state->messagenum)
				break;

			if (nextframe->fixangle[num])
			{
				break;
			}
			if (prevstate->messagenum != nextframe->parsecount)
			{
				break;
			}
			if (!ISDEAD(prevstate->frame) && ISDEAD(state->frame))
			{
				break;
			}

			exacttime = nextframe->time - (float)(prevstate->msec)*0.001;
			good = true;
		}
	}
	else
	{
		while (nexttime < frame->time && i < world.parsecount)
		{
			good = false;
			i++;

			prevstate = nextstate;
			exacttime = nexttime;
			nextframe = &world.frames[i&UPDATE_MASK];
			nextstate = &nextframe->playerstate[num];
			if (nextframe->fixangle[num])
			{
				break;
			}
			if (nextstate->messagenum != nextframe->parsecount)
			{
				break;
			}
			if (!ISDEAD(nextstate->frame) && ISDEAD(state->frame))
			{
				break;
			}

			nexttime = nextframe->time - (float)(nextstate->msec)*0.001;
			good = true;
		}
	}

	if (good && nexttime > frame->time)
	{
		f = (frame->time - nexttime)/(nexttime - exacttime);

		for (j=0;j<3;j++)
		{
			demoinfo->angles[j] = adjustangle(prevstate->command.angles[j], nextstate->command.angles[j],1.0+f);
			demoinfo->origin[j] = nextstate->origin[j] + f*(nextstate->origin[j]-prevstate->origin[j]);
		}
	}
	else
	{
		VectorCopy(state->origin, demoinfo->origin);
		VectorCopy(state->command.angles, demoinfo->angles);
	}
}

/*
==================
WritePlayers

Writes an update of players state
==================
*/

void WritePlayers (sizebuf_t *msg, frame_t *frame)
{
	int i, j, msec, dflags;
	player_state_t	*state;
	demoinfo_t	demoinfo;
	frame_t		*nextframe;

	state = frame->playerstate;
	nextframe = &world.frames[(world.lastwritten+1)&UPDATE_MASK];

	for (i = 0; i < MAX_CLIENTS; i++, state++)
	{
		if (state->messagenum != frame->parsecount)
			continue;

		dflags = 0;

		if (world.lastwritten - world.demoinfo[i].parsecount >= UPDATE_BACKUP - 1)
		{
			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "wipe out:%d, %d\n", i, world.lastwritten - world.demoinfo[i].parsecount);
			memset(&world.demoinfo[i], 0, sizeof(demoinfo_t));
		}

		world.demoinfo[i].parsecount = world.lastwritten;

		Interpolate(i, frame, &demoinfo);
		msec = state->msec;

		demoinfo.angles[2] = 0; // no roll angle

		if (frame->fixangle[i] == true/* || (ISDEAD(state->frame) && !ISDEAD(nextframe->playerstate[i].frame))*/)
		{
			frame->fixangle[i] = false;
			MSG_WriteByte(msg, svc_setangle);
			MSG_WriteByte(msg, i);
			for (j=0 ; j < 3 ; j++)
				MSG_WriteAngle (msg, nextframe->playerstate[i].command.angles[j]);

			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "send fixangle:%d\n", i);
		}

		if (state->flags & PF_DEAD)
		{	// don't show the corpse looking around...
			demoinfo.angles[0] = 0;
			demoinfo.angles[1] = state->command.angles[1];
			demoinfo.angles[2] = 0;
		}

		for (j=0; j < 3; j++)
			if ( world.demoinfo[i].origin[j] != demoinfo.origin[j] )
				dflags |= DF_ORIGIN << j;

		for (j=0; j < 3; j++)
			if (world.demoinfo[i].angles[j] != demoinfo.angles[j])
				dflags |= DF_ANGLES << j;

		if (state->modelindex != world.demoinfo[i].model)
			dflags |= DF_MODEL;
		if (state->effects != world.demoinfo[i].effects)
			dflags |= DF_EFFECTS;
		if (state->skinnum != world.demoinfo[i].skinnum)
			dflags |= DF_SKINNUM;
		if (state->flags & PF_DEAD)
			dflags |= DF_DEAD;
		if (state->flags & PF_GIB)
			dflags |= DF_GIB;
		if (state->weaponframe != world.demoinfo[i].weaponframe)
			dflags |= DF_WEAPONFRAME;

		MSG_WriteByte (msg, svc_playerinfo);
		MSG_WriteByte (msg, i);
		MSG_WriteShort (msg, dflags);

		MSG_WriteByte (msg, state->frame);

		for (j=0 ; j<3 ; j++)
			if (dflags & (DF_ORIGIN << j))
				MSG_WriteCoord (msg, demoinfo.origin[j]);

		for (j=0 ; j<3 ; j++)
			if (dflags & (DF_ANGLES << j))
				MSG_WriteAngle16 (msg, demoinfo.angles[j]);

		if (dflags & DF_MODEL)
			MSG_WriteByte (msg, state->modelindex);

		if (dflags & DF_SKINNUM)
			MSG_WriteByte (msg, state->skinnum);

		if (dflags & DF_EFFECTS)
			MSG_WriteByte (msg, state->effects);

		if (dflags & DF_WEAPONFRAME)
			MSG_WriteByte (msg, state->weaponframe);

		VectorCopy(demoinfo.origin, world.demoinfo[i].origin);
		VectorCopy(demoinfo.angles, world.demoinfo[i].angles);
		world.demoinfo[i].skinnum = state->skinnum;
		world.demoinfo[i].effects = state->effects;
		world.demoinfo[i].weaponframe = state->weaponframe;
		world.demoinfo[i].model = state->modelindex;
	}

	for (i = 0; i < MAX_CLIENTS; i++)
		frame->fixangle[i] = false;
}

/*
==================
WriteDelta

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
void WriteDelta (entity_state_t *efrom, entity_state_t *to, sizebuf_t *msg, qbool force)
{
	int		bits;
	int		i;

	// send an update
	bits = 0;

	for (i=0 ; i<3 ; i++)
		if ( to->origin[i] != efrom->origin[i] )
			bits |= U_ORIGIN1<<i;

	if ( to->angles[0] != efrom->angles[0] )
		bits |= U_ANGLE1;

	if ( to->angles[1] != efrom->angles[1] )
		bits |= U_ANGLE2;

	if ( to->angles[2] != efrom->angles[2] )
		bits |= U_ANGLE3;

	if ( to->colormap != efrom->colormap )
		bits |= U_COLORMAP;

	if ( to->skinnum != efrom->skinnum )
		bits |= U_SKIN;

	if ( to->frame != efrom->frame )
		bits |= U_FRAME;

	if ( to->effects != efrom->effects )
		bits |= U_EFFECTS;

	if ( to->modelindex != efrom->modelindex )
		bits |= U_MODEL;

	if (bits & 511)
		bits |= U_MOREBITS;

	if (to->flags & U_SOLID)
		bits |= U_SOLID;

	//
	// write the message
	//
	if (!to->number)
	{
		Sys_Printf ("ERROR:Unset entity number\n");
		Dem_Stop(from);
		return;
	}

	if (to->number >= 512)
	{
		Sys_Printf ("ERROR:Entity number >= 512\n");
		Dem_Stop(from);
		return;
	}

	if (!bits && !force)
		return;		// nothing to send!
	i = to->number | (bits&~511);
	if (i & U_REMOVE)
	{
		Sys_Printf ("U_REMOVE");
		Dem_Stop(from);
		return;
	}
	MSG_WriteShort (msg, i);

	if (bits & U_MOREBITS)
		MSG_WriteByte (msg, bits&255);
	if (bits & U_MODEL)
		MSG_WriteByte (msg,	to->modelindex);
	if (bits & U_FRAME)
		MSG_WriteByte (msg, to->frame);
	if (bits & U_COLORMAP)
		MSG_WriteByte (msg, to->colormap);
	if (bits & U_SKIN)
		MSG_WriteByte (msg, to->skinnum);
	if (bits & U_EFFECTS)
		MSG_WriteByte (msg, to->effects);
	if (bits & U_ORIGIN1)
		MSG_WriteCoord (msg, to->origin[0]);
	if (bits & U_ANGLE1)
		MSG_WriteAngle(msg, to->angles[0]);
	if (bits & U_ORIGIN2)
		MSG_WriteCoord (msg, to->origin[1]);
	if (bits & U_ANGLE2)
		MSG_WriteAngle(msg, to->angles[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, to->origin[2]);
	if (bits & U_ANGLE3)
		MSG_WriteAngle(msg, to->angles[2]);
}


/*
=============
EmitPacketEntities

Writes a delta update of a packet_entities_t to the message.
=============
*/
static void EmitPacketEntities (sizebuf_t *msg, packet_entities_t *to)
{
	frame_t *fromframe;
	packet_entities_t *from1;
	int oldindex, newindex;
	int oldnum, newnum;
	int oldmax;

	// this is the frame that we are going to delta update from
	if (world.delta_sequence != -1)
	{
		fromframe = &world.frames[world.delta_sequence & UPDATE_MASK];
		from1 = &fromframe->packet_entities;
		oldmax = from1->num_entities;

		MSG_WriteByte (msg, svc_deltapacketentities);
		MSG_WriteByte (msg, world.delta_sequence);
	}
	else
	{
		oldmax = 0;	// no delta update
		from1 = NULL;

		MSG_WriteByte (msg, svc_packetentities);
	}

	newindex = 0;
	oldindex = 0;
	//Con_Printf ("---%i to %i ----\n", client->delta_sequence & UPDATE_MASK
	//			, client->netchan.outgoing_sequence & UPDATE_MASK);
	while (newindex < to->num_entities || oldindex < oldmax)
	{
		newnum = newindex >= to->num_entities ? 9999 : to->entities[newindex].number;
		oldnum = oldindex >= oldmax ? 9999 : from1->entities[oldindex].number;

		if (newnum == oldnum)
		{	// delta update from old position
			//Con_Printf ("delta %i\n", newnum);
			WriteDelta (&from1->entities[oldindex], &to->entities[newindex], msg, false);
			oldindex++;
			newindex++;
			continue;
		}

		if (newnum < oldnum)
		{	// this is a new entity, send it from the baseline
			//Con_Printf ("baseline %i\n", newnum);
			WriteDelta (&baselines[newnum], &to->entities[newindex], msg, true);
			newindex++;
			continue;
		}

		if (newnum > oldnum)
		{	// the old entity isn't present in the new message
			//Con_Printf ("remove %i\n", oldnum);
			MSG_WriteShort (msg, oldnum | U_REMOVE);
			oldindex++;
			continue;
		}
	}

	MSG_WriteShort (msg, 0);	// end of packetentities
}


void EmitNailUpdate (sizebuf_t *msg, frame_t *frame)
{
	byte	bits[6];	// [48 bits] xyzpy 12 12 12 4 8
	int		n, i;
	int		x, y, z, p, yaw;
	projectile_t	*pr;

	if (!frame->num_projectiles)
		return;

	if (frame->projectiles[0].num)
		MSG_WriteByte (msg, svc_nails2);
	else
		MSG_WriteByte (msg, svc_nails);

	MSG_WriteByte (msg, frame->num_projectiles);

	for (n=0 ; n<frame->num_projectiles ; n++)
	{
		pr = &frame->projectiles[n];
		x = (int)(pr->origin[0]+4096)>>1;
		y = (int)(pr->origin[1]+4096)>>1;
		z = (int)(pr->origin[2]+4096)>>1;
		p = Q_rint(16*pr->angles[0]/360)&15;
		yaw = Q_rint(256*pr->angles[1]/360)&255;

		bits[0] = x;
		bits[1] = (x>>8) | (y<<4);
		bits[2] = (y>>4);
		bits[3] = z;
		bits[4] = (z>>8) | (p<<4);
		bits[5] = yaw;

		if (pr->num)
			MSG_WriteByte(msg, pr->num);

		for (i=0 ; i<6 ; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}

void Marge (sizebuf_t *dest, int start, int end);
void WritePackets(int num)
{
	frame_t *frame;
	sizebuf_t	msg, msg2;
	byte		buf[MAX_DATAGRAM], buf2[45*MAX_MSGLEN];

	msg.data = buf;
	msg.maxsize = sizeof(buf);
	msg2.data = buf2;
	msg2.maxsize = sizeof(buf2);
	msg2.cursize = 0;
	msg2.bufsize = 0;

	while (num)
	{
		msg.cursize = 0;
		frame = &world.frames[world.lastwritten&UPDATE_MASK];

		msg2.data = buf2;
		msg2.maxsize = sizeof(buf2);

		Marge(&msg2, world.lastwritten, world.lastwritten + sworld.range < world.parsecount ? world.lastwritten + sworld.range: world.parsecount);

		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "real:%f, demo:%f\n", world.time, demo.time);
		// Add packet entities, nails, and player
		if (!frame->invalid)
		{
			if (!world.lastwritten)
				world.delta_sequence = -1;

			WritePlayers (&msg, frame);
			EmitPacketEntities (&msg, &frame->packet_entities);
			EmitNailUpdate(&msg, frame);

			if (frame->packet_entities.num_entities)
				world.delta_sequence = world.lastwritten&255;
		}

		SV_MVDWriteToDisk(&msg2, demo.lasttype,demo.lastto, frame->time); // this goes first to reduce demo size a bit
		SV_MVDWriteToDisk(&msg2, 0,0, frame->time); // now goes the rest
		if (msg.cursize)
			WriteDemoMessage(&msg, dem_all, 0, frame->time);

		num--;
		if (world.lastwritten == world.parsecount)
			break;

		world.lastwritten++;
	}

	//msgbuf = &world.frames[world.parsecount&UPDATE_MASK].buf;
	//msgbuf->maxsize = MAXSIZE + msgbuf->bufsize;
}
