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
// view.c -- player eye positioning

#include "quakedef.h"
#include "r_local.h"
#include "pmove.h"

/*

The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/

cvar_t	cl_rollspeed = {"cl_rollspeed", "200"};
cvar_t	cl_rollangle = {"cl_rollangle", "2.0"};

cvar_t	cl_bob = {"cl_bob","0.02"};
cvar_t	cl_bobcycle = {"cl_bobcycle","0.6"};
cvar_t	cl_bobup = {"cl_bobup","0.5"};

cvar_t	v_kicktime = {"v_kicktime", "0.5"};
cvar_t	v_kickroll = {"v_kickroll", "0.6"};
cvar_t	v_kickpitch = {"v_kickpitch", "0.6"};

qboolean V_OnIdleChange (cvar_t *var, char *value);
cvar_t	v_iyaw_cycle = {"v_iyaw_cycle", "2", 0, V_OnIdleChange};
cvar_t	v_iroll_cycle = {"v_iroll_cycle", "0.5", 0, V_OnIdleChange};
cvar_t	v_ipitch_cycle = {"v_ipitch_cycle", "1", 0, V_OnIdleChange};
cvar_t	v_iyaw_level = {"v_iyaw_level", "0.3", 0, V_OnIdleChange};
cvar_t	v_iroll_level = {"v_iroll_level", "0.1", 0, V_OnIdleChange};
cvar_t	v_ipitch_level = {"v_ipitch_level", "0.3", 0, V_OnIdleChange};

cvar_t	v_idlescale = {"v_idlescale", "0", 0, V_OnIdleChange};

cvar_t	crosshair = {"crosshair","2",CVAR_ARCHIVE};
cvar_t	crosshaircolor = {"crosshaircolor","79",CVAR_ARCHIVE};

cvar_t  cl_crossx = {"cl_crossx", "0", CVAR_ARCHIVE};
cvar_t  cl_crossy = {"cl_crossy", "0", CVAR_ARCHIVE};

cvar_t  v_contentblend = {"v_contentblend", "1"};

cvar_t	v_damagecshift = {"v_damagecshift", "1"};
cvar_t	v_quadcshift = {"v_quadcshift", "1"};
cvar_t	v_suitcshift = {"v_suitcshift", "1"};
cvar_t	v_ringcshift = {"v_ringcshift", "1"};
cvar_t	v_pentcshift = {"v_pentcshift", "1"};

cvar_t	v_bonusflash = {"cl_bonusflash", "1"};

float	v_dmg_time, v_dmg_roll, v_dmg_pitch;

extern	int			in_forward, in_forward2, in_back;

frame_t		*view_frame;
player_state_t		*view_message;


qboolean V_OnIdleChange (cvar_t *var, char *value)
{
	// Don't allow cheating in TF
	if (cl.teamfortress && cls.state >= ca_connected &&
		cbuf_current != &cbuf_svc)
		return true;
	return false;
}

/*
===============
V_CalcRoll

===============
*/
float V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	vec3_t	forward, right, up;
	float	sign;
	float	side;
	float	value;
	
	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = cl_rollangle.value;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
V_CalcBob

===============
*/
float V_CalcBob (void)
{
	static	double	bobtime;
	static float	bob;
	float	cycle;
	
	if (cl.spectator)
		return 0;

	if (onground == -1)
		return bob;		// just use old value

	bobtime += host_frametime;
	cycle = bobtime - (int)(bobtime/cl_bobcycle.value)*cl_bobcycle.value;
	cycle /= cl_bobcycle.value;
	if (cycle < cl_bobup.value)
		cycle = M_PI * cycle / cl_bobup.value;
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup.value)/(1.0 - cl_bobup.value);

// bob is proportional to simulated velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob = sqrt(cl.simvel[0]*cl.simvel[0] + cl.simvel[1]*cl.simvel[1]) * cl_bob.value;
	bob = bob*0.3 + bob*0.7*sin(cycle);
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	return bob;
	
}


//=============================================================================


cvar_t	v_centermove = {"v_centermove", "0.15"};
cvar_t	v_centerspeed = {"v_centerspeed","500"};


void V_StartPitchDrift (void)
{
#if 1
	if (cl.laststop == cl.time)
	{
		return;		// something else is keeping it from drifting
	}
#endif
	if (cl.nodrift || !cl.pitchvel)
	{
		cl.pitchvel = v_centerspeed.value;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

void V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards cl.idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.

Drifting is enabled when the center view key is hit, mlook is released and
lookspring is non 0, or when 
===============
*/
void V_DriftPitch (void)
{
	float		delta, move;

	if (view_message->onground == -1 || cls.demoplayback)
	{
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}

// don't count small mouse motion
	if (cl.nodrift)
	{
		if ( fabs(cl.frames[(cls.netchan.outgoing_sequence-1)&UPDATE_MASK].cmd.forwardmove) < 200)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;
	
		if ( cl.driftmove > v_centermove.value)
		{
			V_StartPitchDrift ();
		}
		return;
	}
	
	delta = 0 - cl.viewangles[PITCH];

	if (!delta)
	{
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed.value;
	
//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0)
	{
		if (move > delta)
		{
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}





/*
============================================================================== 
 
						PALETTE FLASHES 
 
============================================================================== 
*/ 
 
 
cshift_t	cshift_empty = { {130,80,50}, 0 };
cshift_t	cshift_water = { {130,80,50}, 128 };
cshift_t	cshift_slime = { {0,25,5}, 150 };
cshift_t	cshift_lava = { {255,80,0}, 150 };

cvar_t		v_gamma = {"gamma","0.7",CVAR_ARCHIVE};
cvar_t		v_contrast = {"contrast","1",CVAR_ARCHIVE};

byte		gammatable[256];	// palette is sent through this

byte		current_pal[768];	// Tonik: used for screenshots


#ifdef	GLQUAKE

cvar_t		gl_cshiftpercent = {"gl_cshiftpercent", "100"};
cvar_t		gl_gamma = {"gl_gamma","1",CVAR_ARCHIVE};
cvar_t		gl_contrast = {"gl_contrast","1",CVAR_ARCHIVE};
float		v_blend[4];		// rgba 0.0 - 1.0
unsigned short	ramps[3][256];

#endif	// GLQUAKE


#ifndef GLQUAKE
void BuildGammaTable (float g, float c)
{
	int		i, inf;
	
	if (g < 0.3) g = 0.3;
	if (g > 3) g = 3;
	if (c < 1) c = 1;
	if (c > 3) c = 3;

	if (g == 1 && c == 1)
	{
		for (i=0 ; i<256 ; i++)
			gammatable[i] = i;
		return;
	}
	
	for (i=0 ; i<256 ; i++)
	{
		inf = 255 * pow ((i+0.5)/255.5*c, g) + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		gammatable[i] = inf;
	}
}

/*
=================
V_CheckGamma
=================
*/
qboolean V_CheckGamma (void)
{
	static float old_gamma;
	static float old_contrast;
	
	if (v_gamma.value == old_gamma && v_contrast.value == old_contrast)
		return false;
	old_gamma = v_gamma.value;
	old_contrast = v_contrast.value;
	
	BuildGammaTable (v_gamma.value, v_contrast.value);
	vid.recalc_refdef = 1;				// force a surface cache flush
	
	return true;
}
#endif	// !GLQUAKE


/*
===============
V_ParseDamage
===============
*/
void V_ParseDamage (void)
{
	int		armor, blood;
	vec3_t	from;
	int		i;
	vec3_t	forward, right, up;
	float	side;
	float	count;
	float	fraction;
	
	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i=0 ; i<3 ; i++)
		from[i] = MSG_ReadCoord ();

	count = blood*0.5 + armor*0.5;
	if (count < 10)
		count = 10;

	cl.faceanimtime = cl.time + 0.2;		// but sbar face into pain frame

	cl.cshifts[CSHIFT_DAMAGE].percent += 3*count;
	if (cl.cshifts[CSHIFT_DAMAGE].percent < 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;
	if (cl.cshifts[CSHIFT_DAMAGE].percent > 150)
		cl.cshifts[CSHIFT_DAMAGE].percent = 150;

	fraction = v_damagecshift.value;
	if (fraction < 0) fraction = 0;
	if (fraction > 1) fraction = 1;
	cl.cshifts[CSHIFT_DAMAGE].percent *= fraction;

	if (armor > blood)		
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
	}
	else if (armor)
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
	}
	else
	{
		cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
		cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
	}

//
// calculate view angle kicks
//
	VectorSubtract (from, cl.simorg, from);
	VectorNormalize (from);
	
	AngleVectors (cl.simangles, forward, right, up);

	side = DotProduct (from, right);
	v_dmg_roll = count*side*v_kickroll.value;
	
	side = DotProduct (from, forward);
	v_dmg_pitch = count*side*v_kickpitch.value;

	v_dmg_time = v_kicktime.value;
}


/*
==================
V_cshift_f
==================
*/
void V_cshift_f (void)
{
	// don't allow cheating in TF
	if (cls.state >= ca_connected && cl.teamfortress
	&& cbuf_current != &cbuf_svc)
		return;

	cshift_empty.destcolor[0] = atoi(Cmd_Argv(1));
	cshift_empty.destcolor[1] = atoi(Cmd_Argv(2));
	cshift_empty.destcolor[2] = atoi(Cmd_Argv(3));
	cshift_empty.percent = atoi(Cmd_Argv(4));
}


/*
==================
V_BonusFlash_f

When you run over an item, the server sends this command
==================
*/
void V_BonusFlash_f (void)
{
	if (!v_bonusflash.value && cbuf_current == &cbuf_svc)
		return;

	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

/*
=============
V_SetContentsColor

Underwater, lava, etc each has a color shift
=============
*/
void V_SetContentsColor (int contents)
{
	if (!v_contentblend.value) {
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		return;
	}

	switch (contents)
	{
	case CONTENTS_EMPTY:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		break;
	case CONTENTS_LAVA:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
		break;
	case CONTENTS_SOLID:
	case CONTENTS_SLIME:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
		break;
	default:
		cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}

	if (v_contentblend.value > 0 && v_contentblend.value < 1
		&& contents != CONTENTS_EMPTY)
		cl.cshifts[CSHIFT_CONTENTS].percent *= v_contentblend.value;

#ifdef GLQUAKE
	if (!gl_polyblend.value && !cl.teamfortress)
		cl.cshifts[CSHIFT_CONTENTS].percent = 0;
	else {
		// ignore gl_cshiftpercent on custom cshifts (set with v_cshift
		// command) to avoid cheating in TF
		if (contents != CONTENTS_EMPTY) {
			if (!gl_polyblend.value)
				cl.cshifts[CSHIFT_CONTENTS].percent = 0;
			else
				cl.cshifts[CSHIFT_CONTENTS].percent *= gl_cshiftpercent.value;
		}
		else
			cl.cshifts[CSHIFT_CONTENTS].percent *= 100;
	}

#endif
}

/*
=============
V_CalcPowerupCshift
=============
*/
void V_CalcPowerupCshift (void)
{
	float fraction;

	if (cl.stats[STAT_ITEMS] & IT_QUAD)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 255;
		fraction = v_quadcshift.value;
		if (fraction < 0) fraction = 0;
		if (fraction > 1) fraction = 1;
		cl.cshifts[CSHIFT_POWERUP].percent = 30 * fraction;
	}
	else if (cl.stats[STAT_ITEMS] & IT_SUIT)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 0;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = v_suitcshift.value;
		if (fraction < 0) fraction = 0;
		if (fraction > 1) fraction = 1;
		cl.cshifts[CSHIFT_POWERUP].percent = 20 * fraction;
	}
	else if (cl.stats[STAT_ITEMS] & IT_INVISIBILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 100;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 100;
		fraction = v_ringcshift.value;
		if (fraction < 0) fraction = 0;
		if (fraction > 1) fraction = 1;
		cl.cshifts[CSHIFT_POWERUP].percent = 100 * fraction;
	}
	else if (cl.stats[STAT_ITEMS] & IT_INVULNERABILITY)
	{
		cl.cshifts[CSHIFT_POWERUP].destcolor[0] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[1] = 255;
		cl.cshifts[CSHIFT_POWERUP].destcolor[2] = 0;
		fraction = v_pentcshift.value;
		if (fraction < 0) fraction = 0;
		if (fraction > 1) fraction = 1;
		cl.cshifts[CSHIFT_POWERUP].percent = 30 * fraction;
	}
	else
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
}


/*
=============
V_CalcBlend
=============
*/
#ifdef	GLQUAKE
void V_CalcBlend (void)
{
	float	r, g, b, a, a2;
	int		j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	V_CalcPowerupCshift ();

// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	for (j=0 ; j<NUM_CSHIFTS ; j++)	
	{
		if ((!gl_cshiftpercent.value || !gl_polyblend.value)
			&& j != CSHIFT_CONTENTS)
			continue;

		if (j == CSHIFT_CONTENTS)
			a2 = cl.cshifts[j].percent / 100.0 / 255.0;
		else
			a2 = ((cl.cshifts[j].percent * gl_cshiftpercent.value) / 100.0) / 255.0;

		if (!a2)
			continue;
		a = a + a2*(1-a);
//Con_Printf ("j:%i a:%f\n", j, a);
		a2 = a2/a;
		r = r*(1-a2) + cl.cshifts[j].destcolor[0]*a2;
		g = g*(1-a2) + cl.cshifts[j].destcolor[1]*a2;
		b = b*(1-a2) + cl.cshifts[j].destcolor[2]*a2;
	}

	v_blend[0] = r/255.0;
	v_blend[1] = g/255.0;
	v_blend[2] = b/255.0;
	v_blend[3] = a;
	if (v_blend[3] > 1)
		v_blend[3] = 1;
	if (v_blend[3] < 0)
		v_blend[3] = 0;
}

void V_AddLightBlend (float r, float g, float b, float a2)
{
	float	a;

	if (!gl_polyblend.value || !gl_cshiftpercent.value)
		return;

	a2 = a2 * gl_cshiftpercent.value / 100.0;

	v_blend[3] = a = v_blend[3] + a2*(1-v_blend[3]);

	a2 = a2/a;

	v_blend[0] = v_blend[1]*(1-a2) + r*a2;
	v_blend[1] = v_blend[1]*(1-a2) + g*a2;
	v_blend[2] = v_blend[2]*(1-a2) + b*a2;
}
#endif


/*
=============
V_UpdatePalette
=============
*/
#ifdef	GLQUAKE
void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	static float	prev_blend[4];
	float	a, rgb[3];
	int		c;
	float	gamma, contrast;
	static float	old_gamma, old_contrast;
#ifdef _WIN32
	extern float	vid_gamma;
#endif

	if (cls.state != ca_active) {
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
	}
	else
		V_CalcPowerupCshift ();

	new = false;

	for (i=0 ; i<4 ; i++) {
		if (v_blend[i] != prev_blend[i]) {
			new = true;
			prev_blend[i] = v_blend[i];
		}
	}

	gamma = gl_gamma.value;
	if (gamma < 0.3)
		gamma = 0.3;
	if (gamma > 3)
		gamma = 3;
	if (gamma != old_gamma) {
		old_gamma = gamma;
		new = true;
	}

	contrast = gl_contrast.value;
	if (contrast > 3)
		contrast = 3;
	if (contrast < 1)
		contrast = 1;
	if (contrast != old_contrast) {
		old_contrast = contrast;
		new = true;
	}

	if (!new)
		return;

	a = v_blend[3];

#ifdef _WIN32
	if (!vid_hwgamma_enabled)
		a = 0;
#endif

	rgb[0] = 255*v_blend[0]*a;
	rgb[1] = 255*v_blend[1]*a;
	rgb[2] = 255*v_blend[2]*a;

	a = 1-a;
#ifdef _WIN32
	if (vid_gamma != 1.0) {
		contrast = pow (contrast, vid_gamma);
		gamma = gamma/vid_gamma;
	}
#endif

	for (i=0 ; i<256 ; i++)
	{
		for (j=0 ; j<3 ; j++) {
			// apply blend and contrast
			c = (i*a + rgb[j]) * contrast;
			if (c > 255)
				c = 255;
			// apply gamma
			c = 255 * pow((c + 0.5)/255.5, gamma) + 0.5;
			if (c < 0)
				c = 0;
			if (c > 255)
				c = 255;
			ramps[j][i] = c << 8;
		}
	}

#ifdef _WIN32
	VID_SetDeviceGammaRamp ((unsigned short *) ramps);
#endif
}
#else	// !GLQUAKE
/*
=============
V_UpdatePalette
=============
*/
void V_UpdatePalette (void)
{
	int		i, j;
	qboolean	new;
	byte	*basepal, *newpal;
//	byte	pal[768];
	int		r,g,b;
	qboolean force;
	static cshift_t	prev_cshifts[NUM_CSHIFTS];

	if (cls.state != ca_active) {
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		cl.cshifts[CSHIFT_POWERUP].percent = 0;
	}
	else
		V_CalcPowerupCshift ();
	
	new = false;
	
	for (i=0 ; i<NUM_CSHIFTS ; i++)
	{
		if (cl.cshifts[i].percent != prev_cshifts[i].percent)
		{
			new = true;
			prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		for (j=0 ; j<3 ; j++)
			if (cl.cshifts[i].destcolor[j] != prev_cshifts[i].destcolor[j])
			{
				new = true;
				prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
	}
	
// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime*150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime*100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;
			
	basepal = host_basepal;
//	newpal = pal;
	newpal = current_pal;	// Tonik: so we can use current_pal
							// for screenshots
	
	for (i=0 ; i<256 ; i++)
	{
		r = basepal[0];
		g = basepal[1];
		b = basepal[2];
		basepal += 3;
	
		for (j=0 ; j<NUM_CSHIFTS ; j++)	
		{
			r += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[0]-r))>>8;
			g += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[1]-g))>>8;
			b += (cl.cshifts[j].percent*(cl.cshifts[j].destcolor[2]-b))>>8;
		}
		
		newpal[0] = gammatable[r];
		newpal[1] = gammatable[g];
		newpal[2] = gammatable[b];
		newpal += 3;
	}

	VID_ShiftPalette (current_pal);	
}

#endif	// !GLQUAKE

/* 
============================================================================== 
 
						VIEW RENDERING 
 
============================================================================== 
*/ 

float angledelta (float a)
{
	a = anglemod(a);
	if (a > 180)
		a -= 360;
	return a;
}

/*
==================
CalcGunAngle
==================
*/
void CalcGunAngle (void)
{	
	float	yaw, pitch, move;
	static float oldyaw = 0;
	static float oldpitch = 0;
	
	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = angledelta(yaw - r_refdef.viewangles[YAW]) * 0.4;
	if (yaw > 10)
		yaw = 10;
	if (yaw < -10)
		yaw = -10;
	pitch = angledelta(-pitch - r_refdef.viewangles[PITCH]) * 0.4;
	if (pitch > 10)
		pitch = 10;
	if (pitch < -10)
		pitch = -10;
	move = host_frametime*20;
	if (yaw > oldyaw)
	{
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	}
	else
	{
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}
	
	if (pitch > oldpitch)
	{
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	}
	else
	{
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}
	
	oldyaw = yaw;
	oldpitch = pitch;

	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewent.angles[PITCH] = - (r_refdef.viewangles[PITCH] + pitch);
}

/*
==============
V_BoundOffsets
==============
*/
void V_BoundOffsets (void)
{
// absolutely bound refresh relative to entity clipping hull
// so the view can never be inside a solid wall

	if (r_refdef.vieworg[0] < cl.simorg[0] - 14)
		r_refdef.vieworg[0] = cl.simorg[0] - 14;
	else if (r_refdef.vieworg[0] > cl.simorg[0] + 14)
		r_refdef.vieworg[0] = cl.simorg[0] + 14;
	if (r_refdef.vieworg[1] < cl.simorg[1] - 14)
		r_refdef.vieworg[1] = cl.simorg[1] - 14;
	else if (r_refdef.vieworg[1] > cl.simorg[1] + 14)
		r_refdef.vieworg[1] = cl.simorg[1] + 14;
	if (r_refdef.vieworg[2] < cl.simorg[2] - 22)
		r_refdef.vieworg[2] = cl.simorg[2] - 22;
	else if (r_refdef.vieworg[2] > cl.simorg[2] + 30)
		r_refdef.vieworg[2] = cl.simorg[2] + 30;
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle (void)
{
	r_refdef.viewangles[ROLL] += v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value;
	r_refdef.viewangles[PITCH] += v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value;
	r_refdef.viewangles[YAW] += v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value;

	cl.viewent.angles[ROLL] -= v_idlescale.value * sin(cl.time*v_iroll_cycle.value) * v_iroll_level.value;
	cl.viewent.angles[PITCH] -= v_idlescale.value * sin(cl.time*v_ipitch_cycle.value) * v_ipitch_level.value;
	cl.viewent.angles[YAW] -= v_idlescale.value * sin(cl.time*v_iyaw_cycle.value) * v_iyaw_level.value;
}


/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll (void)
{
	float		side;
		
	side = V_CalcRoll (cl.simangles, cl.simvel);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0)
	{
		r_refdef.viewangles[ROLL] += v_dmg_time/v_kicktime.value*v_dmg_roll;
		r_refdef.viewangles[PITCH] += v_dmg_time/v_kicktime.value*v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}

}


/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef (void)
{
	entity_t	*view;
	float		old;

// view is the weapon model
	view = &cl.viewent;

	VectorCopy (cl.simorg, r_refdef.vieworg);
	VectorCopy (cl.simangles, r_refdef.viewangles);
	view->model = NULL;

// always idle in intermission
	old = v_idlescale.value;
	v_idlescale.value = 1;
	V_AddIdle ();
	v_idlescale.value = old;
}

/*
==================
V_CalcRefdef

==================
*/
void V_CalcRefdef (void)
{
	entity_t	*view;
	int			i;
	vec3_t		forward, right, up;
	float		bob;

	V_DriftPitch ();

// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	bob = V_CalcBob ();
	
// refresh position from simulated origin
	VectorCopy (cl.simorg, r_refdef.vieworg);

	r_refdef.vieworg[2] += bob;

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
	r_refdef.vieworg[0] += 1.0/16;
	r_refdef.vieworg[1] += 1.0/16;
	r_refdef.vieworg[2] += 1.0/16;

	VectorCopy (cl.simangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle ();

	if (view_message->flags & PF_GIB)
		r_refdef.vieworg[2] += 8;	// gib view height
	else if (view_message->flags & PF_DEAD)
		r_refdef.vieworg[2] -= 16;	// corpse view height
	else
		r_refdef.vieworg[2] += 22;	// view height

	if (view_message->flags & PF_DEAD)		// PF_GIB will also set PF_DEAD
		r_refdef.viewangles[ROLL] = 80;	// dead view angle


// offsets
	AngleVectors (cl.simangles, forward, right, up);
	
// set up gun position
	VectorCopy (cl.simangles, view->angles);
	
	CalcGunAngle ();

	VectorCopy (cl.simorg, view->origin);
	view->origin[2] += 22;

	for (i=0 ; i<3 ; i++)
	{
		view->origin[i] += forward[i]*bob*0.4;
//		view->origin[i] += right[i]*bob*0.4;
//		view->origin[i] += up[i]*bob*0.8;
	}
	view->origin[2] += bob;

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV
	if (scr_viewsize.value == 110)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 100)
		view->origin[2] += 2;
	else if (scr_viewsize.value == 90)
		view->origin[2] += 1;
	else if (scr_viewsize.value == 80)
		view->origin[2] += 0.5;

	if (view_message->flags & (PF_GIB|PF_DEAD) )
 		view->model = NULL;
 	else
		view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = view_message->weaponframe;
	view->colormap = vid.colormap;

// set up the refresh position
	r_refdef.viewangles[PITCH] += cl.punchangle;

// smooth out stair step ups
	r_refdef.vieworg[2] += cl.crouch;
	view->origin[2] += cl.crouch;
}

/*
=============
DropPunchAngle
=============
*/
void DropPunchAngle (void)
{
	cl.punchangle -= 10*host_frametime;
	if (cl.punchangle < 0)
		cl.punchangle = 0;
}


/*
==================
V_RenderView

The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
the entity origin, so any view position inside that will be valid
==================
*/
extern vrect_t scr_vrect;

void V_RenderView (void)
{
//	if (cl.simangles[ROLL])
//		Sys_Error ("cl.simangles[ROLL]");	// DEBUG
cl.simangles[ROLL] = 0;	// FIXME @@@ 

	if (cls.state != ca_active) {
#ifdef GLQUAKE
		V_CalcBlend ();
#endif
		return;
	}

	view_frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	view_message = &view_frame->playerstate[cl.playernum];

	DropPunchAngle ();
	if (cl.intermission)
	{	// intermission / finale rendering
		V_CalcIntermissionRefdef ();	
	}
	else
	{
		V_CalcRefdef ();
	}

	R_PushDlights ();
	R_RenderView ();

#ifndef GLQUAKE
	if (crosshair.value)
		Draw_Crosshair();
#endif
		
}

//============================================================================

/*
=============
V_Init
=============
*/
void V_Init (void)
{
	Cmd_AddCommand ("v_cshift", V_cshift_f);	
	Cmd_AddCommand ("bf", V_BonusFlash_f);
	Cmd_AddCommand ("centerview", V_StartPitchDrift);

	Cvar_RegisterVariable (&v_centermove);
	Cvar_RegisterVariable (&v_centerspeed);

	Cvar_RegisterVariable (&v_iyaw_cycle);
	Cvar_RegisterVariable (&v_iroll_cycle);
	Cvar_RegisterVariable (&v_ipitch_cycle);
	Cvar_RegisterVariable (&v_iyaw_level);
	Cvar_RegisterVariable (&v_iroll_level);
	Cvar_RegisterVariable (&v_ipitch_level);

	Cvar_RegisterVariable (&v_contentblend);
	Cvar_RegisterVariable (&v_damagecshift);
	Cvar_RegisterVariable (&v_quadcshift);
	Cvar_RegisterVariable (&v_suitcshift);
	Cvar_RegisterVariable (&v_ringcshift);
	Cvar_RegisterVariable (&v_pentcshift);

	Cvar_RegisterVariable (&v_bonusflash);

	Cvar_RegisterVariable (&v_idlescale);
	Cvar_RegisterVariable (&crosshaircolor);
	Cvar_RegisterVariable (&crosshair);
	Cvar_RegisterVariable (&cl_crossx);
	Cvar_RegisterVariable (&cl_crossy);
#ifdef GLQUAKE
	Cvar_RegisterVariable (&gl_cshiftpercent);
#endif

	Cvar_RegisterVariable (&cl_rollspeed);
	Cvar_RegisterVariable (&cl_rollangle);
	Cvar_RegisterVariable (&cl_bob);
	Cvar_RegisterVariable (&cl_bobcycle);
	Cvar_RegisterVariable (&cl_bobup);

	Cvar_RegisterVariable (&v_kicktime);
	Cvar_RegisterVariable (&v_kickroll);
	Cvar_RegisterVariable (&v_kickpitch);	

	Cvar_RegisterVariable (&v_gamma);
	Cvar_RegisterVariable (&v_contrast);
#ifdef GLQUAKE
	Cvar_RegisterVariable (&gl_gamma);
	Cvar_RegisterVariable (&gl_contrast);
#endif

#ifndef GLQUAKE
	BuildGammaTable (v_gamma.value, v_contrast.value);
#endif

}


