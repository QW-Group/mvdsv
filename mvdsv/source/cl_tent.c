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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"
#include "pmove.h"
#include "sound.h"

#define	MAX_BEAMS	8
typedef struct
{
	int		entity;
	struct model_s	*model;
	float	endtime;
	vec3_t	start, end;
} beam_t;

beam_t		cl_beams[MAX_BEAMS];

static vec3_t	playerbeam_end;


#define	MAX_EXPLOSIONS	8
typedef struct
{
	vec3_t	origin;
	float	start;
	model_t	*model;
} explosion_t;

explosion_t	cl_explosions[MAX_EXPLOSIONS];


sfx_t			*cl_sfx_wizhit;
sfx_t			*cl_sfx_knighthit;
sfx_t			*cl_sfx_tink1;
sfx_t			*cl_sfx_ric1;
sfx_t			*cl_sfx_ric2;
sfx_t			*cl_sfx_ric3;
sfx_t			*cl_sfx_r_exp3;

/*
=================
CL_ParseTEnts
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit = S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound ("weapons/r_exp3.wav");
}

/*
=================
CL_ClearTEnts
=================
*/
void CL_ClearTEnts (void)
{
	memset (&cl_beams, 0, sizeof(cl_beams));
	memset (&cl_explosions, 0, sizeof(cl_explosions));
}

/*
=================
CL_AllocExplosion
=================
*/
explosion_t *CL_AllocExplosion (void)
{
	int		i;
	float	time;
	int		index;
	
	for (i=0 ; i<MAX_EXPLOSIONS ; i++)
		if (!cl_explosions[i].model)
			return &cl_explosions[i];
// find the oldest explosion
	time = cl.time;
	index = 0;

	for (i=0 ; i<MAX_EXPLOSIONS ; i++)
		if (cl_explosions[i].start < time)
		{
			time = cl_explosions[i].start;
			index = i;
		}
	return &cl_explosions[index];
}

/*
=================
CL_ParseBeam
=================
*/
extern cvar_t cl_chasecam;
void CL_ParseBeam (model_t *m)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort ();
	
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

	if (ent == cl.playernum+1 || (cl.spectator && ent == Cam_TrackNum()+1 && cl_chasecam.value))
	{
		//VectorCopy (cl.frames[cl.parsecount & UPDATE_MASK].playerstate[ent - 1].origin, start);
		VectorCopy (end, playerbeam_end);	// for cl_trueLightning
	}

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}
	Con_Printf ("beam list overflow!\n");	
}

/*
=================
CL_ParseTEnt
=================
*/
void CL_ParseTEnt (void)
{
	int		type;
	vec3_t	pos;
	dlight_t	*dl;
	int		rnd;
	explosion_t	*ex;
	int		cnt;

	type = MSG_ReadByte ();
	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;
		
	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);
		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;
		
	case TE_SPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 10);

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20);

		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
		
	case TE_EXPLOSION:			// rocket explosion
	// particles

		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		if (cl_explosion.value == 6)
			R_TeleportSplash (pos);
		else if (cl_explosion.value == 7)
			R_RunParticleEffect (pos, vec3_origin, 73, 20*32);
		else if (cl_explosion.value == 8)
			R_RunParticleEffect (pos, vec3_origin, 225, 50);
		else
		{

		// Tonik: explosion modifiers...
			if (cl_explosion.value != 1 && cl_explosion.value != 3)
			{
				R_ParticleExplosion (pos);
			}
		
		// light
			if (cl_explosion.value != 2 && cl_explosion.value != 3)
			{
				dl = CL_AllocDlight (0);
				VectorCopy (pos, dl->origin);
				dl->radius = 350;
				dl->die = cl.time + 0.5;
				dl->decay = 300;
				dl->color[0] = 0.2;
				dl->color[1] = 0.1;
				dl->color[2] = 0.05;
				dl->color[3] = 0.7;
			}
		}

	// sound
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);

	// sprite
		if (cl_explosion.value != 6 && cl_explosion.value != 7 && cl_explosion.value != 8)
		{
			ex = CL_AllocExplosion ();
			VectorCopy (pos, ex->origin);
			ex->start = cl.time;
			ex->model = Mod_ForName ("progs/s_explod.spr", true);
		}
		break;
		
	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_BlobExplosion (pos);

		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt.mdl", true));
		break;
	
	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt2.mdl", true));
		break;
	
	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (Mod_ForName("progs/bolt3.mdl", true));
		break;
	
	case TE_LAVASPLASH:	
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;
	
	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		break;

	case TE_GUNSHOT:			// bullet hitting wall
		cnt = MSG_ReadByte ();
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 0, 20*cnt);
		break;
		
	case TE_BLOOD:				// bullets hitting body
		cnt = MSG_ReadByte ();
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 73, 20*cnt);
		break;

	case TE_LIGHTNINGBLOOD:		// lightning hitting body
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 225, 50);
		break;

	default:
		Host_EndGame ("CL_ParseTEnt: bad type");
	}
}


/*
=================
CL_NewTempEntity
=================
*/
entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
		return NULL;
	ent = &cl_visedicts[cl_numvisedicts];
	cl_numvisedicts++;
	ent->keynum = 0;
	
	memset (ent, 0, sizeof(*ent));

	ent->colormap = vid.colormap;
	return ent;
}


/*
=================
CL_UpdateBeams
=================
*/
void vectoangles(vec3_t vec, vec3_t ang);

void CL_UpdateBeams (void)
{
	int			i;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;
	float		forward;

// update lightning
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
			continue;

	// if coming from the player, update the start position
		if (b->entity == cl.playernum+1 || (cl.spectator && b->entity == Cam_TrackNum()+1 && cl_chasecam.value))	// entity 0 is the world
		{
			VectorCopy (cl.simorg, b->start);
			b->start[2] += cl.crouch;
			if (cl_trueLightning.value)
			{
				vec3_t	forward, right, up;
				vec3_t	v, org;
				vec3_t	ang;
				float	f, delta;
				pmtrace_t	trace;
				
				f = max(0, min(1, cl_trueLightning.value));
				
				VectorSubtract (playerbeam_end, cl.simorg, v);
				v[2] -= 22;		// adjust for view height
				vectoangles (v, ang);
				
				// lerp pitch
				ang[0] = -ang[0];
				if (ang[0] < -180)
					ang[0] += 360;
				ang[0] += (cl.viewangles[0] - ang[0])*f;
				
				// lerp yaw
				delta = cl.viewangles[1] - ang[1];
				if (delta > 180)
					delta -= 360;
				if (delta < -180)
					delta += 360;
				ang[1] += delta*f;
				ang[2] = 0;
				
				AngleVectors (ang, forward, right, up);
				VectorScale (forward, 600, forward);
				VectorCopy (cl.simorg, org);
				org[2] += 16;
				VectorAdd (org, forward, b->end);
				
				trace = PM_TraceLine (org, b->end);
				if (trace.fraction < 1)
					VectorCopy (trace.endpos, b->end);
			}
		}
		
	// calculate pitch and yaw
		VectorSubtract (b->end, b->start, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);
		while (d > 0)
		{
			ent = CL_NewTempEntity ();
			if (!ent)
				return;
			VectorCopy (org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;
			ent->angles[2] = rand()%360;

			for (i=0 ; i<3 ; i++)
				org[i] += dist[i]*30;
			d -= 30;
		}
	}
	
}

/*
=================
CL_UpdateExplosions
=================
*/
void CL_UpdateExplosions (void)
{
	int			i;
	int			f;
	explosion_t	*ex;
	entity_t	*ent;

	for (i=0, ex=cl_explosions ; i< MAX_EXPLOSIONS ; i++, ex++)
	{
		if (!ex->model)
			continue;
		f = 10*(cl.time - ex->start);
		if (f >= ex->model->numframes)
		{
			ex->model = NULL;
			continue;
		}

		ent = CL_NewTempEntity ();
		if (!ent)
			return;
		VectorCopy (ex->origin, ent->origin);
		ent->model = ex->model;
		ent->frame = f;
	}
}

/*
=================
CL_UpdateTEnts
=================
*/
void CL_UpdateTEnts (void)
{
	CL_UpdateBeams ();
	CL_UpdateExplosions ();
}
