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
/* ZOID
 *
 * Player camera tracking in Spectator mode
 *
 * This takes over player controls for spectator automatic camera.
 * Player moves as a spectator, but the camera tracks and enemy player
 */

#include "quakedef.h"
#include "winquake.h"
#include "pmove.h"
#include "sbar.h"

#define BUTTON_JUMP 2
#define BUTTON_ATTACK 1
#define MAX_ANGLE_TURN 10

static vec3_t desired_position; // where the camera wants to be
static qboolean locked = false;
static int oldbuttons;

// track high fragger
cvar_t cl_hightrack = {"cl_hightrack", "0" };
cvar_t cl_track_validonly = {"cl_track_validonly", "1" };
cvar_t cl_matrixcam_dist = {"cl_matrixcam_dist", "50" };
cvar_t cl_matrixcam_speed = {"cl_matrixcam_speed", "3" };

cvar_t cl_chasecam = {"cl_chasecam", "0"};

//cvar_t cl_camera_maxpitch = {"cl_camera_maxpitch", "10" };
//cvar_t cl_camera_maxyaw = {"cl_camera_maxyaw", "30" };

qboolean cam_forceview;
vec3_t cam_viewangles;
double cam_lastviewtime;

int spec_track = 0; // player# of who we are tracking
int	ideal_track = 0;
float	last_lock = 0;
int autocam = CAM_NONE;

void vectoangles(vec3_t vec, vec3_t ang)
{
	float	forward;
	float	yaw, pitch;
	
	if (vec[1] == 0 && vec[0] == 0)
	{
		yaw = 0;
		if (vec[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(vec[1], vec[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
		pitch = (int) (atan2(vec[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	ang[0] = pitch;
	ang[1] = yaw;
	ang[2] = 0;
}

static float vlen(vec3_t v)
{
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// returns true if weapon model should be drawn in camera mode
qboolean Cam_DrawViewModel(void)
{
	if (!cl.spectator)
		return true;

	if (autocam && locked && cl_chasecam.value == 1)
		return true;
	return false;
}

static qboolean tooClose = false;

// returns true if we should draw this player, we don't if we are chase camming
qboolean Cam_DrawPlayer(int playernum)
{
	if (cl.spectator && autocam && locked && spec_track == playernum) {
		if (cl_chasecam.value == 1) 
			return false;
		if (cl_chasecam.value == 3 && tooClose) 
			return false;
	}

	return true;
}

int Cam_TrackNum(void)
{
	if (!autocam)
		return -1;
	return spec_track;
}
void TP_FixTeamSets(void);

void Cam_Unlock(void)
{
	if (autocam) {
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, "ptrack");
		autocam = CAM_NONE;
		locked = false;
		Sbar_Changed();
		TP_FixTeamSets();
	}
}

void Cam_Lock(int playernum)
{
	char st[40];

	sprintf(st, "ptrack %i", playernum);
	if (cls.demoplayback2) {
		memcpy(cl.stats, cl.players[playernum].stats, sizeof(cl.stats));
	}

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	MSG_WriteString (&cls.netchan.message, st);
	spec_track = playernum;
	last_lock = realtime;
	cam_forceview = true;
	locked = false;
	Sbar_Changed();
	TP_FixTeamSets();
}

pmtrace_t Cam_DoTrace(vec3_t vec1, vec3_t vec2)
{
#if 0
	memset(&pmove, 0, sizeof(pmove));

	pmove.numphysent = 1;
	VectorCopy (vec3_origin, pmove.physents[0].origin);
	pmove.physents[0].model = cl.worldmodel;
#endif

	VectorCopy (vec1, pmove.origin);
	return PM_PlayerMove(pmove.origin, vec2);
}
	
// Returns distance or 9999 if invalid for some reason
static float Cam_TryFlyby(player_state_t *self, player_state_t *player, vec3_t vec, qboolean checkvis)
{
	vec3_t v;
	pmtrace_t trace;
	float len;

	vectoangles(vec, v);
//	v[0] = -v[0];
	VectorCopy (v, pmove.angles);
	VectorNormalize(vec);
	VectorMA(player->origin, 800, vec, v);
	// v is endpos
	// fake a player move
	trace = Cam_DoTrace(player->origin, v);
	if (/*trace.inopen ||*/ trace.inwater)
		return 9999;
	VectorCopy(trace.endpos, vec);
	VectorSubtract(trace.endpos, player->origin, v);
	len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	if (len < 32 || len > 800)
		return 9999;
	if (checkvis) {
		VectorSubtract(trace.endpos, self->origin, v);
		len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

		trace = Cam_DoTrace(self->origin, vec);
		if (trace.fraction != 1 || trace.inwater)
			return 9999;
	}
	return len;
}

// Is player visible?
static qboolean Cam_IsVisible(player_state_t *player, vec3_t vec)
{
	pmtrace_t trace;
	vec3_t v;
	float d;

	trace = Cam_DoTrace(player->origin, vec);
	if (trace.fraction != 1 || /*trace.inopen ||*/ trace.inwater)
		return false;
	// check distance, don't let the player get too far away or too close
	VectorSubtract(player->origin, vec, v);
	d = vlen(v);
	if (d < 16)
		return false;
	return true;
}

static qboolean InitFlyby(player_state_t *self, player_state_t *player, int checkvis) 
{
    float f, max;
    vec3_t vec, vec2;
	vec3_t forward, right, up;

	VectorCopy(player->viewangles, vec);
    vec[0] = 0;
	AngleVectors (vec, forward, right, up);
//	for (i = 0; i < 3; i++)
//		forward[i] *= 3;

    max = 1000;
	VectorAdd(forward, up, vec2);
	VectorAdd(vec2, right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorAdd(forward, up, vec2);
	VectorSubtract(vec2, right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorAdd(forward, right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorSubtract(forward, right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorAdd(forward, up, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorSubtract(forward, up, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorAdd(up, right, vec2);
	VectorSubtract(vec2, forward, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorSubtract(up, right, vec2);
	VectorSubtract(vec2, forward, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	// invert
	VectorSubtract(vec3_origin, forward, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorCopy(forward, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	// invert
	VectorSubtract(vec3_origin, right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }
	VectorCopy(right, vec2);
    if ((f = Cam_TryFlyby(self, player, vec2, checkvis)) < max) {
        max = f;
		VectorCopy(vec2, vec);
    }

	// ack, can't find him
    if (max >= 1000) {
//		Cam_Unlock();
		return false;
	}
	locked = true;
	VectorCopy(vec, desired_position); 
	return true;
}

static void Cam_CheckHighTarget(void)
{
	int i, j, max;
	player_info_t	*s;

	j = -1;
	for (i = 0, max = -9999; i < MAX_CLIENTS; i++) {
		s = &cl.players[i];
		if (s->name[0] && !s->spectator && s->frags > max) {
			max = s->frags;
			j = i;
		}
	}
	if (j >= 0) {
		if (!locked || cl.players[j].frags > cl.players[spec_track].frags)
		{
			Cam_Lock(j);
			ideal_track = spec_track;
		}
	} else
		Cam_Unlock();
}

void Transform(vec3_t in, double angle, vec3_t out)
{
	float si, co;

	si = sin(angle);
	co = cos(angle);

	out[0] = in[0]*co + in[1]*si;
	out[1] = -in[0]*si + in[1]*co;
	out[2] = in[2];
}

void Cam_Matrix(usercmd_t *cmd, player_state_t *self, player_state_t *player)
{
	vec3_t		ideal, dist, forward;
	pmtrace_t	trace;
	float		l, scale, len, camdist, camspeed;

	camdist = max(cl_matrixcam_dist.value, 5);
	camspeed = max(cl_matrixcam_speed.value, 0);

	cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;

	VectorCopy(self->origin, ideal);
	ideal[2] = player->origin[2];

	VectorSubtract(ideal, player->origin, dist);

	if ( (len = vlen(dist)) == 0 ) {
		len = 1;
		dist[0] = 1;
	}

	Transform(dist, camspeed*real_frametime, forward);

	l = camdist - len;
	if (camdist > len)
		scale = min(l, 500*real_frametime);
	else
		scale = max(l, -500*real_frametime);

	VectorMA(player->origin, (len + scale)/len, forward, ideal);

	trace = Cam_DoTrace(player->origin, ideal);
	//VectorCopy(trace.endpos, self->origin);
	VectorCopy(trace.endpos, desired_position);

	//VectorSubtract(player->origin, self->origin, forward);
	//vectoangles(forward, cl.viewangles);
}

void Cam_Chase(usercmd_t *cmd, player_state_t *self, player_state_t *player)
{
	vec3_t		ideal, forward, right, up;
	pmtrace_t	trace;
	float		camdist;

	camdist = max(cl_matrixcam_dist.value, 5);
	
	cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;

	AngleVectors(player->viewangles, forward, right, up);

	VectorMA(player->origin, -camdist, forward, ideal);
	ideal[2] += 10;

	trace = Cam_DoTrace(player->origin, ideal);
	if (trace.fraction != 1.0) {
		VectorMA(trace.endpos, -min(1.0 - trace.fraction, 16.0/camdist), forward, ideal);
		VectorCopy(ideal, desired_position);
	} else 
		VectorCopy(trace.endpos, desired_position);

	// if to close don't draw the player
	VectorSubtract(desired_position, player->origin, ideal);
	if (vlen(ideal) < 16)
		tooClose = true;
	else
		tooClose = false;
}
	
// ZOID
//
// Take over the user controls and track a player.
// We find a nice position to watch the player and move there
void Cam_Track(usercmd_t *cmd)
{
	player_state_t *player, *self;
	frame_t *frame;
	vec3_t vec;
	float len;

	if (!cl.spectator)
		return;
	
	if (cl_hightrack.value && !locked)
		Cam_CheckHighTarget();

	if (!autocam || cls.state != ca_active)
		return;

	if (locked && (!cl.players[spec_track].name[0] || cl.players[spec_track].spectator)) {
		locked = false;
		if (cl_hightrack.value)
			Cam_CheckHighTarget();
		else
			Cam_Unlock();
		return;
	}

	frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	if (autocam && cls.demoplayback2 && cl_track_validonly.value)
	{
		if (ideal_track != spec_track && realtime - last_lock > 1 && frame->playerstate[ideal_track].messagenum == cl.parsecount)
			Cam_Lock(ideal_track);

		if (frame->playerstate[spec_track].messagenum != cl.parsecount)
		{
			int i;

			for (i = 0; i < MAX_CLIENTS; i++)
			{
				if (frame->playerstate[i].messagenum == cl.parsecount)
					break;
			}
			if (i < MAX_CLIENTS)
				Cam_Lock(i);
		}
	}

	player = frame->playerstate + spec_track;
	self = frame->playerstate + cl.playernum;

	if (!locked || !Cam_IsVisible(player, desired_position)) {
		if (!locked || realtime - cam_lastviewtime > 0.1) {
			if (!InitFlyby(self, player, true))
				InitFlyby(self, player, false);
			cam_lastviewtime = realtime;
		}
	} else
		cam_lastviewtime = realtime;
	
	// couldn't track for some reason
	if (!locked || !autocam)
		return;

	if (cl_chasecam.value == 1) {
		cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;

		VectorCopy(player->viewangles, cl.viewangles);
		VectorCopy(player->origin, desired_position);
		if (memcmp(&desired_position, &self->origin, sizeof(desired_position)) != 0) {
			MSG_WriteByte (&cls.netchan.message, clc_tmove);
			MSG_WriteCoord (&cls.netchan.message, desired_position[0]);
			MSG_WriteCoord (&cls.netchan.message, desired_position[1]);
			MSG_WriteCoord (&cls.netchan.message, desired_position[2]);
			// move there locally immediately
			VectorCopy(desired_position, self->origin);
		}
		self->weaponframe = player->weaponframe;

	} else {
		if (cl_chasecam.value == 2)
			Cam_Matrix(cmd, self, player);
		if (cl_chasecam.value == 3)
			Cam_Chase(cmd, self, player);

		// Ok, move to our desired position and set our angles to view
		// the player
		VectorSubtract(desired_position, self->origin, vec);
		len = vlen(vec);
		cmd->forwardmove = cmd->sidemove = cmd->upmove = 0;
		if (len > 16 || cl_chasecam.value == 2) { // close enough?
			MSG_WriteByte (&cls.netchan.message, clc_tmove);
			MSG_WriteCoord (&cls.netchan.message, desired_position[0]);
			MSG_WriteCoord (&cls.netchan.message, desired_position[1]);
			MSG_WriteCoord (&cls.netchan.message, desired_position[2]);
		}

		// move there locally immediately
		VectorCopy(desired_position, self->origin);
										 
		VectorSubtract(player->origin, desired_position, vec);
		vectoangles(vec, cl.viewangles);
		cl.viewangles[0] = -cl.viewangles[0];
	}
}

#if 0
static float adjustang(float current, float ideal, float speed)
{
	float move;

	current = anglemod(current);
	ideal = anglemod(ideal);

	if (current == ideal)
		return current;

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
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

//Con_Printf("c/i: %4.2f/%4.2f move: %4.2f\n", current, ideal, move);
	return anglemod (current + move);
}
#endif

#if 0
void Cam_SetView(void)
{
	return;
	player_state_t *player, *self;
	frame_t *frame;
	vec3_t vec, vec2;

	if (cls.state != ca_active || !cl.spectator || 
		!autocam || !locked)
		return;

	frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	player = frame->playerstate + spec_track;
	self = frame->playerstate + cl.playernum;

	VectorSubtract(player->origin, cl.simorg, vec);
	if (cam_forceview) {
		cam_forceview = false;
		vectoangles(vec, cam_viewangles);
		cam_viewangles[0] = -cam_viewangles[0];
	} else {
		vectoangles(vec, vec2);
		vec2[PITCH] = -vec2[PITCH];

		cam_viewangles[PITCH] = adjustang(cam_viewangles[PITCH], vec2[PITCH], cl_camera_maxpitch.value);
		cam_viewangles[YAW] = adjustang(cam_viewangles[YAW], vec2[YAW], cl_camera_maxyaw.value);
	}
	VectorCopy(cam_viewangles, cl.viewangles);
	VectorCopy(cl.viewangles, cl.simangles);
}
#endif

void Cam_FinishMove(usercmd_t *cmd)
{
	int i;
	player_info_t	*s;
	int end;

	if (cls.state != ca_active)
		return;

	if (!cl.spectator) // only in spectator mode
		return;

#if 0
	if (autocam && locked) {
		frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
		player = frame->playerstate + spec_track;
		self = frame->playerstate + cl.playernum;

		VectorSubtract(player->origin, self->origin, vec);
		if (cam_forceview) {
			cam_forceview = false;
			vectoangles(vec, cam_viewangles);
			cam_viewangles[0] = -cam_viewangles[0];
		} else {
			vectoangles(vec, vec2);
			vec2[PITCH] = -vec2[PITCH];

			cam_viewangles[PITCH] = adjustang(cam_viewangles[PITCH], vec2[PITCH], cl_camera_maxpitch.value);
			cam_viewangles[YAW] = adjustang(cam_viewangles[YAW], vec2[YAW], cl_camera_maxyaw.value);
		}
		VectorCopy(cam_viewangles, cl.viewangles);
	}
#endif

	if (cmd->buttons & BUTTON_ATTACK) {
		if (!(oldbuttons & BUTTON_ATTACK)) {

			oldbuttons |= BUTTON_ATTACK;
			autocam++;

			if (autocam > CAM_TRACK) {
				Cam_Unlock();
				VectorCopy(cl.viewangles, cmd->angles);
				return;
			}
		} else
			return;
	} else {
		oldbuttons &= ~BUTTON_ATTACK;
		if (!autocam)
			return;
	}

	if (autocam && cl_hightrack.value) {
		Cam_CheckHighTarget();
		return;
	}

	if (locked) {
		if ((cmd->buttons & BUTTON_JUMP) && (oldbuttons & BUTTON_JUMP)) {
			return;		// don't pogo stick
		}

		if (!(cmd->buttons & BUTTON_JUMP)) {
			oldbuttons &= ~BUTTON_JUMP;
			return;
		}
		oldbuttons |= BUTTON_JUMP;	// don't jump again until released
	}

//	Con_Printf("Selecting track target...\n");

	if (locked && autocam)
		end = (ideal_track + 1) % MAX_CLIENTS;
	else
		end = ideal_track;
	i = end;
	do {
		s = &cl.players[i];
		if (s->name[0] && !s->spectator) {
			Cam_Lock(i);
			Con_Printf("tracking %s\n", s->name);
			ideal_track = i;
			return;
		}
		i = (i + 1) % MAX_CLIENTS;
	} while (i != end);
	// stay on same guy?
	i = ideal_track;
	s = &cl.players[i];
	if (s->name[0] && !s->spectator) {
		Cam_Lock(i);
		ideal_track = i;
		return;
	}
	Con_Printf("No target found ...\n");
	autocam = locked = false;
}

void Cam_Reset(void)
{
	autocam = CAM_NONE;
	spec_track = 0;
	ideal_track = 0;
}

void CL_InitCam(void)
{
	Cvar_RegisterVariable (&cl_hightrack);
	Cvar_RegisterVariable (&cl_chasecam);
	Cvar_RegisterVariable (&cl_track_validonly);
	Cvar_RegisterVariable (&cl_matrixcam_dist);
	Cvar_RegisterVariable (&cl_matrixcam_speed);
//	Cvar_RegisterVariable (&cl_camera_maxpitch);
//	Cvar_RegisterVariable (&cl_camera_maxyaw);
}


