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
#include <time.h>

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))
#define	RETURN_STRING(s) (((int *)pr_globals)[OFS_RETURN] = PR_SetString(s))

/*
===============================================================================

						BUILT-IN FUNCTIONS

===============================================================================
*/

char *PF_VarString (int	first)
{
	int		i;
	static char out[2048];
	
	out[0] = 0;
	for (i=first ; i<pr_argc ; i++)
	{
		strlcat (out, G_STRING((OFS_PARM0+i*3)), sizeof(out));
	}
	return out;
}


/*
=================
PF_errror

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
void PF_error (void)
{
	char	*s;
	edict_t	*ed;
	
	s = PF_VarString(0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n", PR_GetString(pr_xfunction->s_name) ,s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);

	SV_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
void PF_objerror (void)
{
	char	*s;
	edict_t	*ed;
	
	s = PF_VarString(0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n", PR_GetString(pr_xfunction->s_name),s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);
	ED_Free (ed);
	
	SV_Error ("Program error");
}



/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
void PF_makevectors (void)
{
	AngleVectors (G_VECTOR(OFS_PARM0), pr_global_struct->v_forward, pr_global_struct->v_right, pr_global_struct->v_up);
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics of the world (setting velocity and waiting).  Directly changing origin will not set internal links correctly, so clipping would be messed up.  This should be called when an object is spawned, and then only if it is teleported.

setorigin (entity, origin)
=================
*/
void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;
	
	e = G_EDICT(OFS_PARM0);
	org = G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
void PF_setsize (void)
{
	edict_t	*e;
	float	*min, *max;
	
	e = G_EDICT(OFS_PARM0);
	min = G_VECTOR(OFS_PARM1);
	max = G_VECTOR(OFS_PARM2);
	VectorCopy (min, e->v.mins);
	VectorCopy (max, e->v.maxs);
	VectorSubtract (max, min, e->v.size);
	SV_LinkEdict (e, false);
}


/*
=================
PF_setmodel

setmodel(entity, model)
Also sets size, mins, and maxs for inline bmodels
=================
*/
void PF_setmodel (void)
{
	edict_t	*e;
	char	*m, **check;
	int		i;
	model_t	*mod;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

// check to see if model was properly precached
	for (i=0, check = sv.model_precache ; *check ; i++, check++)
		if (!strcmp(*check, m))
			break;

	if (!*check)
		PR_RunError ("no precache: %s\n", m);
		
	e->v.model = PR_SetString(m);
	e->v.modelindex = i;

// if it is an inline model, get the size information for it
	if (m[0] == '*')
	{
		mod = Mod_ForName (m, true);
		VectorCopy (mod->mins, e->v.mins);
		VectorCopy (mod->maxs, e->v.maxs);
		VectorSubtract (mod->maxs, mod->mins, e->v.size);
		SV_LinkEdict (e, false);
	}

}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
void PF_bprint (void)
{
	char		*s;
	int			level;
	
	level = G_FLOAT(OFS_PARM0);

	s = PF_VarString(1);
	SV_BroadcastPrintf (level, "%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/

void PF_sprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;
	int			level;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	level = G_FLOAT(OFS_PARM1);

	s = PF_VarString(2);
	
	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	client = &svs.clients[entnum-1];
	
	SV_ClientPrintf (client, level, "%s", s);
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
void PF_centerprint (void)
{
	char		*s;
	int			entnum;
	client_t	*cl;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);
	
	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	cl = &svs.clients[entnum-1];

	ClientReliableWrite_Begin (cl, svc_centerprint, 2 + strlen(s));
	ClientReliableWrite_String (cl, s);

	if (sv.demorecording) {
		DemoWrite_Begin (dem_single, entnum - 1, 2 + strlen(s));
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_centerprint);
		MSG_WriteString ((sizebuf_t*)demo.dbuf, s);
	}
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	new;
	
	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);
	
	if (new == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		new = 1/new;
		newvalue[0] = value1[0] * new;
		newvalue[1] = value1[1] * new;
		newvalue[2] = value1[2] * new;
	}
	
	VectorCopy (newvalue, G_VECTOR(OFS_RETURN));	
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
void PF_vlen (void)
{
	float	*value1;
	float	new;
	
	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);
	
	G_FLOAT(OFS_RETURN) = new;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;
	
	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
void PF_vectoangles (void)
{
	float	*value1;
	float	forward;
	float	yaw, pitch;
	
	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	G_FLOAT(OFS_RETURN+0) = pitch;
	G_FLOAT(OFS_RETURN+1) = yaw;
	G_FLOAT(OFS_RETURN+2) = 0;
}

/*
=================
PF_Random

Returns a number from 0<= num < 1

random()
=================
*/
void PF_random (void)
{
	float		num;
		
	num = (rand ()&0x7fff) / ((float)0x7fff);
	
	G_FLOAT(OFS_RETURN) = num;
}


/*
=================
PF_ambientsound

=================
*/
void PF_ambientsound (void)
{
	char		**check;
	char		*samp;
	float		*pos;
	float 		vol, attenuation;
	int			i, soundnum;

	pos = G_VECTOR (OFS_PARM0);			
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);
	
// check to see if samp was properly precached
	for (soundnum=0, check = sv.sound_precache ; *check ; check++, soundnum++)
		if (!strcmp(*check,samp))
			break;
			
	if (!*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet

	MSG_WriteByte (&sv.signon,svc_spawnstaticsound);
	for (i=0 ; i<3 ; i++)
		MSG_WriteCoord(&sv.signon, pos[i]);

	MSG_WriteByte (&sv.signon, soundnum);

	MSG_WriteByte (&sv.signon, vol*255);
	MSG_WriteByte (&sv.signon, attenuation*64);

}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
void PF_sound (void)
{
	char		*sample;
	int			channel;
	edict_t		*entity;
	int 		volume;
	float attenuation;
		
	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);
	
	SV_StartSound (entity, channel, sample, volume, attenuation);
}

/*
=================
PF_break

break()
=================
*/
void PF_break (void)
{
Con_Printf ("break statement\n");
*(int *)-4 = 0;	// dump to debugger
//	PR_RunError ("break statement");
}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entityes, and also slide box entities
if the tryents flag is set.

traceline (vector1, vector2, tryents)
=================
*/
void PF_traceline (void)
{
	float	*v1, *v2;
	trace_t	trace;
	int		nomonsters;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	nomonsters = G_FLOAT(OFS_PARM2);
	ent = G_EDICT(OFS_PARM3);

	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist =  trace.plane.dist;	
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}

/*
=================
PF_checkpos

Returns true if the given entity can move to the given position from it's
current position by walking or rolling.
FIXME: make work...
scalar checkpos (entity, vector)
=================
*/
void PF_checkpos (void)
{
}

//============================================================================

byte	checkpvs[MAX_MAP_LEAFS/8];

int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > MAX_CLIENTS)
		check = MAX_CLIENTS;

	if (check == MAX_CLIENTS)
		i = 1;
	else
		i = check + 1;

	for ( ;  ; i++)
	{
		if (i == MAX_CLIENTS+1)
			i = 1;

		ent = EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if (ent->free)
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel, false);
	memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
int c_invis, c_notvis;
void PF_checkclient (void)
{
	edict_t	*ent, *self;
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;
	
// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 0.1)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible	
	ent = EDICT_NUM(sv.lastcheck);
	if (ent->free || ent->v.health <= 0)
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	self = PROG_TO_EDICT(pr_global_struct->self);
	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ( (l<0) || !(checkpvs[l>>3] & (1<<(l&7)) ) )
	{
c_notvis++;
		RETURN_EDICT(sv.edicts);
		return;
	}

// might be able to see it
c_invis++;
	RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
void PF_stuffcmd (void)
{
	int		entnum;
	char	*str;
	client_t	*cl;
	char	*buf;
	int		i;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > MAX_CLIENTS)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);	
	
	cl = &svs.clients[entnum-1];

	buf = cl->stufftext_buf;
	if (strlen(buf) + strlen(str) >= MAX_STUFFTEXT)
		PR_RunError ("stufftext buffer overflow");
	strlcat (buf, str, MAX_STUFFTEXT);

	for (i = strlen(buf); i >= 0; i--)
	{
		if (buf[i] == '\n')
		{
			if (!strncmp(buf, "disconnect\n", MAX_STUFFTEXT))
			{
				// so long and thanks for all the fish
				cl->drop = true;
				buf[0] = 0;
				return;
			}
			ClientReliableWrite_Begin (cl, svc_stufftext, 2+strlen(buf));
			ClientReliableWrite_String (cl, buf);
			if (sv.demorecording) {
				DemoWrite_Begin ( dem_single, cl - svs.clients, 2+strlen(buf));
				MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_stufftext);
				MSG_WriteString((sizebuf_t*)demo.dbuf, buf);
			}
			buf[0] = 0;
		}
	}
}

/*
=================
PF_localcmd

Sends text over to the client's execution buffer

localcmd (string)
=================
*/
void PF_localcmd (void)
{
	char	*str;
	
	str = G_STRING(OFS_PARM0);	
	Cbuf_AddText (str);
}

void PF_executecmd (void)
{
	int old_other, old_self; // mod_consolecmd will be executed, so we need to store this

	old_self = pr_global_struct->self;
	old_other = pr_global_struct->other;

	Cbuf_Execute();

	pr_global_struct->self = old_self;
	pr_global_struct->other = old_other;
}

#define MAX_PR_STRING_SIZE 2048

int		pr_string_index = 0;
char	pr_string_buf[8][MAX_PR_STRING_SIZE];
char	*pr_string_temp = pr_string_buf[0];

void PF_SetTempString(void)
{
	pr_string_temp = pr_string_buf[pr_string_index++&7];
}


/*
=================
PF_tokanize

tokanize string

void tokanize(string)
=================
*/

void PF_tokanize (void)
{
	char *str;

	str = G_STRING(OFS_PARM0);
	Cmd_TokenizeString(str);
}

/*
=================
PF_argc

returns number of tokens (must be executed after PF_Tokanize!)

float argc(void)
=================
*/

void PF_argc (void)
{
	G_FLOAT(OFS_RETURN) = (float) Cmd_Argc();
}

/*
=================
PF_argv

returns token requested by user (must be executed after PF_Tokanize!)

string argc(float)
=================
*/

void PF_argv (void)
{
	int num;

	num = (int) G_FLOAT(OFS_PARM0);

	if (num < 0 ) num = 0;
	if (num > Cmd_Argc()-1) num = Cmd_Argc()-1;

	snprintf (pr_string_temp, MAX_PR_STRING_SIZE, "%s", Cmd_Argv(num));
	G_INT(OFS_RETURN) = PR_SetString(pr_string_temp);
	PF_SetTempString();
}

/*
=================
PF_teamfield

string teamfield(.string field)
=================
*/

void PF_teamfield (void)
{
	pr_teamfield = G_INT(OFS_PARM0);
}

/*
=================
PF_substr

string substr(string str, float start, float len)
=================
*/

void PF_substr (void)
{
	char *s;
	int start, len, l;

	s = G_STRING(OFS_PARM0);
	start = (int) G_FLOAT(OFS_PARM1);
	len = (int) G_FLOAT(OFS_PARM2);
	l = strlen(s);

	if (start >= l || !len || !*s) {
		G_INT(OFS_RETURN) = PR_SetTmpString("");
		return;
	}

	s += start;
	l -= start;

	if (len > l + 1)
		len = l + 1;

	strlcpy(pr_string_temp, s, len + 1);

	G_INT(OFS_RETURN) = PR_SetString(pr_string_temp);

	PF_SetTempString();
}

/*
=================
PF_strcat

string strcat(string str1, string str2)
=================
*/

void PF_strcat (void)
{
	/* FIXME */
	strcpy(pr_string_temp, PF_VarString(0)/*, MAX_PR_STRING_SIZE*/);
	G_INT(OFS_RETURN) = PR_SetString(pr_string_temp);

	PF_SetTempString();
}

/*
=================
PF_strlen

float strlen(string str)
=================
*/

void PF_strlen (void)
{
	G_FLOAT(OFS_RETURN) = (float) strlen(G_STRING(OFS_PARM0));
}

/*
=================
PF_str2byte

float str2byte (string str)
=================
*/

void PF_str2byte (void)
{
	G_FLOAT(OFS_RETURN) = (float) *G_STRING(OFS_PARM0);
}

/*
=================
PF_str2short

float str2short (string str)
=================
*/

void PF_str2short (void)
{
	G_FLOAT(OFS_RETURN) = (float) LittleShort(*(short*)G_STRING(OFS_PARM0));
}

/*
=================
PF_newstr

string newstr (string str [, float size])
=================
*/

void PF_newstr (void)
{
	char *s;
	int i, size;

	s = G_STRING(OFS_PARM0);

	for (i = 0; i < MAX_PRSTR; i++)
	{
		if (pr_newstrtbl[i] == NULL)
			break;
	}

	if (i == MAX_PRSTR)
		PR_RunError("PF_newstr: MAX_PRSTR");

	size = strlen(s) + 1;
	if (pr_argc == 2 && (int) G_FLOAT(OFS_PARM1) > size) 
		size = (int) G_FLOAT(OFS_PARM1);

	pr_newstrtbl[i] = (char*) Z_Malloc(size);
	strlcpy(pr_newstrtbl[i], s, size);

	G_INT(OFS_RETURN) = -(i+MAX_PRSTR);
}

/*
=================
PF_frestr

void freestr (string str)
=================
*/

void PF_freestr (void)
{
	int num;

	num = G_INT(OFS_PARM0);
	if (num > - MAX_PRSTR)
		PR_RunError("freestr: Bad pointer");

	num = - (num + MAX_PRSTR);
	Z_Free(pr_newstrtbl[num]);
	pr_newstrtbl[num] = NULL;
}

void PF_clear_strtbl(void)
{
	int i;

	for (i = 0; i < MAX_PRSTR; i++)
	{
		if (pr_newstrtbl[i] != NULL) {
			Z_Free(pr_newstrtbl[i]);
			pr_newstrtbl[i] = NULL;
		}
	}
}

/*
=================
PF_readcmd

string readmcmd (string str)
=================
*/

void PF_readcmd (void)
{
	char *s;
	static char output[8000];
	extern char outputbuf[];
	extern redirect_t sv_redirected;
	redirect_t old;

	s = G_STRING(OFS_PARM0);

	Cbuf_Execute();
	Cbuf_AddText (s);

	old = sv_redirected;
	if (old != RD_NONE)
		SV_EndRedirect();

	SV_BeginRedirect(RD_MOD);
	Cbuf_Execute();
	strlcpy(output, outputbuf, sizeof(output));
	SV_EndRedirect();

	if (old != RD_NONE)
		SV_BeginRedirect(old);


	G_INT(OFS_RETURN) = PR_SetString(output);
}

/*
=================
PF_redirectcmd

void redirectcmd (entity to, string str)
=================
*/

void PF_redirectcmd (void)
{
	char *s;
	int entnum;
	extern redirect_t sv_redirected;

	if (sv_redirected)
		return;

	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > MAX_CLIENTS)
		PR_RunError ("Parm 0 not a client");

	s = G_STRING(OFS_PARM1);

	Cbuf_AddText (s);

	SV_BeginRedirect(RD_MOD + entnum);
	Cbuf_Execute();
	SV_EndRedirect();
}
dfunction_t *ED_FindFunction (char *name);
void SV_TimeOfDay(date_t *date);
void PF_calltimeofday (void)
{
	date_t date;
	dfunction_t *f;

	if ((f = ED_FindFunction ("timeofday")) != NULL) {

		SV_TimeOfDay(&date);

		G_FLOAT(OFS_PARM0) = (float)date.sec;
		G_FLOAT(OFS_PARM1) = (float)date.min;
		G_FLOAT(OFS_PARM2) = (float)date.hour;
		G_FLOAT(OFS_PARM3) = (float)date.day;
		G_FLOAT(OFS_PARM4) = (float)date.mon;
		G_FLOAT(OFS_PARM5) = (float)date.year;
		G_INT(OFS_PARM6) = PR_SetTmpString(date.str);

		PR_ExecuteProgram((func_t)(f - pr_functions));
	}

}

/*
=================
PF_forcedemoframe

void PF_forcedemoframe(float now)
Forces demo frame
if argument 'now' is set, frame is written instantly
=================
*/

void PF_forcedemoframe (void)
{
	demo.forceFrame = 1;
	if (G_FLOAT(OFS_PARM0) == 1)
        SV_SendDemoMessage();
}


/*
=================
PF_strcpy

void strcpy(string dst, string src)
FIXME: check for null pointers first?
=================
*/

void PF_strcpy (void)
{
	strcpy(G_STRING(OFS_PARM0), G_STRING(OFS_PARM1));
}

/*
=================
PF_strncpy

void strcpy(string dst, string src, float count)
FIXME: check for null pointers first?
=================
*/

void PF_strncpy (void)
{
	strncpy(G_STRING(OFS_PARM0), G_STRING(OFS_PARM1), (int) G_FLOAT(OFS_PARM2));
}


/*
=================
PF_strstr

string strstr(string str, string sub)
=================
*/

void PF_strstr (void)
{
	char *str, *sub, *p;

	str = G_STRING(OFS_PARM0);
	sub = G_STRING(OFS_PARM1);

	if ((p = strstr(str, sub)) == NULL)
	{
		G_INT(OFS_RETURN) = 0;
		return;
	}
	
	RETURN_STRING(p);
}

/*
====================
SV_CleanName_Init

sets chararcter table to translate quake texts to more friendly texts
====================
*/

char chartbl2[256];

void PR_CleanLogText_Init ()
{
	int i;

	for (i = 0; i < 256; i++)
		chartbl2[i] = (i&127) < 32 ? ' ' : i&127;

	chartbl2[13] = 13;
	chartbl2[10] = 10;
	// special cases

	// numbers
	for (i = 18; i < 28; i++)
		chartbl2[i] = chartbl2[i + 128] = i + 30;

	// brackets
	chartbl2[29] = chartbl2[29 + 128] = chartbl2[128] = '(';
	chartbl2[31] = chartbl2[31 + 128] = chartbl2[130] = ')';
	chartbl2[16] = chartbl2[16 + 128]= '[';
	chartbl2[17] = chartbl2[17 + 128] = ']';

	// hash
	for (i = 1; i < 5; i++)
		chartbl2[i] = chartbl2[i + 128] = '#';
	for (i = 6; i < 10; i++)
		chartbl2[i] = chartbl2[i + 128] = '#';

	chartbl2[11] = chartbl2[11 + 128] = '#';

	// dot
	chartbl2[5] = chartbl2[14] = chartbl2[15] = chartbl2[28] = chartbl2[46] = '.';
	chartbl2[5 + 128] = chartbl2[14 + 128] = chartbl2[15 + 128] = chartbl2[28 + 128] = chartbl2[46 + 128] = '.';

	// left arrow
	chartbl2[127] = '>';

	// right arrow
	chartbl2[141] = '<';

	// '='
	chartbl2[30] = chartbl2[129] = chartbl2[30 + 128] = '=';

	// whitespaces
	chartbl2[12] = chartbl2[12 + 128] = chartbl2[138] = ' ';

	chartbl2[33] = chartbl2[33 + 128]= '!';
}

void PR_CleanText(unsigned char *text)
{
	for ( ; *text; text++)
		*text = chartbl2[*text];
}

/*
================
PF_log

void log(string name, float console, string text)
=================
*/

void PF_log(void)
{
	char name[MAX_OSPATH], *text;
	FILE *file;

	snprintf(name, MAX_OSPATH, "%s/%s.log", com_gamedir, G_STRING(OFS_PARM0));
	text = PF_VarString(2);
	PR_CleanText(text);

	if ((file = fopen(name, "a")) == NULL)
	{
		Sys_Printf("coldn't open log file %s\n", name);
	} else {
		fprintf (file, text);
		fflush (file);
		fclose(file);
	}

	if (G_FLOAT(OFS_PARM1))
		Sys_Printf("%s", text);
	
}

/*
=================
PF_cvar

float cvar (string)
=================
*/
void PF_cvar (void)
{
	char	*str;
	
	str = G_STRING(OFS_PARM0);
	
	G_FLOAT(OFS_RETURN) = Cvar_VariableValue (str);
}

/*
=================
PF_cvar_set

float cvar (string)
=================
*/
void PF_cvar_set (void)
{
	char	*var_name, *val;
	cvar_t	*var;

	var_name = G_STRING(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

	var = Cvar_FindVar(var_name);
	if (!var)
	{
		Con_Printf ("PF_cvar_set: variable %s not found\n", var_name);
		return;
	}

	Cvar_Set (var, val);
}

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
void PF_findradius (void)
{
	edict_t	*ent, *chain;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int		i, j;

	chain = (edict_t *)sv.edicts;
	
	org = G_VECTOR(OFS_PARM0);
	rad = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j])*0.5);			
		if (Length(eorg) > rad)
			continue;
			
		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
void PF_dprint (void)
{
	Con_Printf ("%s",PF_VarString(0));
}

/*
=========
PF_conprint
=========
*/
void PF_conprint (void)
{
	Sys_Printf ("%s",PF_VarString(0));
}

//char	pr_string_temp[128];

void PF_ftos (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	
	if (v == (int)v)
		snprintf (pr_string_temp, MAX_PR_STRING_SIZE, "%d",(int)v);
	else
		snprintf (pr_string_temp, MAX_PR_STRING_SIZE, "%5.1f",v);
	G_INT(OFS_RETURN) = PR_SetString(pr_string_temp);
	PF_SetTempString();
}

void PF_fabs (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = fabs(v);
}

void PF_vtos (void)
{
	snprintf (pr_string_temp, MAX_PR_STRING_SIZE, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM0)[0], G_VECTOR(OFS_PARM0)[1], G_VECTOR(OFS_PARM0)[2]);
	G_INT(OFS_RETURN) = PR_SetString(pr_string_temp);

	PF_SetTempString();
}

void PF_Spawn (void)
{
	edict_t	*ed;
	ed = ED_Alloc();
	RETURN_EDICT(ed);
}

void PF_Remove (void)
{
	edict_t	*ed;
	
	ed = G_EDICT(OFS_PARM0);
	ED_Free (ed);
}


// entity (entity start, .string field, string match) find = #5;
void PF_Find (void)
{
	int		e;	
	int		f;
	char	*s, *t;
	edict_t	*ed;
	
	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");
		
	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			RETURN_EDICT(ed);
			return;
		}
	}
	
	RETURN_EDICT(sv.edicts);
}

void PR_CheckEmptyString (char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
}

void PF_precache_sound (void)
{
	char	*s;
	int		i;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);
	
	for (i=0 ; i<MAX_SOUNDS ; i++)
	{
		if (!sv.sound_precache[i])
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (!strcmp(sv.sound_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_sound: overflow");
}

void PF_precache_model (void)
{
	char	*s;
	int		i;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
			return;
		}
		if (!strcmp(sv.model_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_model: overflow");
}


void PF_coredump (void)
{
	ED_PrintEdicts ();
}

void PF_traceon (void)
{
	pr_trace = true;
}

void PF_traceoff (void)
{
	pr_trace = false;
}

void PF_eprint (void)
{
	ED_PrintNum (G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
void PF_walkmove (void)
{
	edict_t	*ent;
	float	yaw, dist;
	vec3_t	move;
	dfunction_t	*oldf;
	int 	oldself;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);
	yaw = G_FLOAT(OFS_PARM0);
	dist = G_FLOAT(OFS_PARM1);
	
	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = pr_global_struct->self;
	
	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true);
	
	
// restore program state
	pr_xfunction = oldf;
	pr_global_struct->self = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
void PF_droptofloor (void)
{
	edict_t		*ent;
	vec3_t		end;
	trace_t		trace;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;
	
	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = EDICT_TO_PROG(trace.ent);
		G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
void PF_lightstyle (void)
{
	int		style;
	char	*val;
	client_t	*client;
	int			j;
	
	style = G_FLOAT(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

// change the string in sv
	sv.lightstyles[style] = val;
	
// send message to all clients on this server
	if (sv.state != ss_active)
		return;
	
	for (j=0, client = svs.clients ; j<MAX_CLIENTS ; j++, client++)
		if ( client->state == cs_spawned )
		{
			ClientReliableWrite_Begin (client, svc_lightstyle, strlen(val)+3);
			ClientReliableWrite_Char (client, style);
			ClientReliableWrite_String (client, val);
		}
	if (sv.demorecording)
	{
		DemoWrite_Begin( dem_all, 0, strlen(val)+3);
		MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_lightstyle);
		MSG_WriteChar((sizebuf_t*)demo.dbuf, style);
		MSG_WriteString((sizebuf_t*)demo.dbuf, val);
	}
}

void PF_rint (void)
{
	float	f;
	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.5);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.5);
}
void PF_floor (void)
{
	G_FLOAT(OFS_RETURN) = floor(G_FLOAT(OFS_PARM0));
}
void PF_ceil (void)
{
	G_FLOAT(OFS_RETURN) = ceil(G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
void PF_checkbottom (void)
{
	edict_t	*ent;
	
	ent = G_EDICT(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
void PF_pointcontents (void)
{
	float	*v;
	
	v = G_VECTOR(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_PointContents (v);	
}

/*
=============
PF_nextent

entity nextent(entity)
=============
*/
void PF_nextent (void)
{
	int		i;
	edict_t	*ent;
	
	i = G_EDICTNUM(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == sv.num_edicts)
		{
			RETURN_EDICT(sv.edicts);
			return;
		}
		ent = EDICT_NUM(i);
		if (!ent->free)
		{
			RETURN_EDICT(ent);
			return;
		}
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
//cvar_t	sv_aim = {"sv_aim", "0.93"};
cvar_t	sv_aim = {"sv_aim", "2"};
void PF_aim (void)
{
	edict_t	*ent, *check, *bestent;
	vec3_t	start, dir, end, bestdir;
	int		i, j;
	trace_t	tr;
	float	dist, bestdist;
	float	speed;
	char	*noaim;

	ent = G_EDICT(OFS_PARM0);
	speed = G_FLOAT(OFS_PARM1);

	VectorCopy (ent->v.origin, start);
	start[2] += 20;

// noaim option
	i = NUM_FOR_EDICT(ent);
	if (i>0 && i<MAX_CLIENTS)
	{
		noaim = Info_ValueForKey (svs.clients[i-1].userinfo, "noaim");
		if (atoi(noaim) > 0)
		{
			VectorCopy (pr_global_struct->v_forward, G_VECTOR(OFS_RETURN));
			return;
		}
	}

// try sending a trace straight
	VectorCopy (pr_global_struct->v_forward, dir);
	VectorMA (start, 2048, dir, end);
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
	if (tr.ent && tr.ent->v.takedamage == DAMAGE_AIM
	&& (!teamplay.value || ent->v.team <=0 || ent->v.team != tr.ent->v.team) )
	{
		VectorCopy (pr_global_struct->v_forward, G_VECTOR(OFS_RETURN));
		return;
	}


// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;
	
	check = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, check = NEXT_EDICT(check) )
	{
		if (check->v.takedamage != DAMAGE_AIM)
			continue;
		if (check == ent)
			continue;
		if (teamplay.value && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		for (j=0 ; j<3 ; j++)
			end[j] = check->v.origin[j]
			+ 0.5*(check->v.mins[j] + check->v.maxs[j]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		if (dist < bestdist)
			continue;	// to far to turn
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}
	
	if (bestent)
	{
		VectorSubtract (bestent->v.origin, ent->v.origin, dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		VectorScale (pr_global_struct->v_forward, dist, end);
		end[2] = dir[2];
		VectorNormalize (end);
		VectorCopy (end, G_VECTOR(OFS_RETURN));	
	}
	else
	{
		VectorCopy (bestdir, G_VECTOR(OFS_RETURN));
	}
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);
	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;
	
	if (current == ideal)
		return;
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
	
	ent->v.angles[1] = anglemod (current + move);
}

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE			1		// reliable to one (msg_entity)
#define	MSG_ALL			2		// reliable to all
#define	MSG_INIT		3		// write to the init string
#define	MSG_MULTICAST	4		// for multicast()

sizebuf_t *WriteDest (void)
{
	int		dest;
//	int		entnum;
//	edict_t	*ent;

	dest = G_FLOAT(OFS_PARM0);
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;
	
	case MSG_ONE:
		SV_Error("Shouldn't be at MSG_ONE");
#if 0
		ent = PROG_TO_EDICT(pr_global_struct->msg_entity);
		entnum = NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > MAX_CLIENTS)
			PR_RunError ("WriteDest: not a client");
		return &svs.clients[entnum-1].netchan.message;
#endif
		
	case MSG_ALL:
		return &sv.reliable_datagram;
	
	case MSG_INIT:
		if (sv.state != ss_loading)
			PR_RunError ("PF_Write_*: MSG_INIT can only be written in spawn functions");
		return &sv.signon;

	case MSG_MULTICAST:
		return &sv.multicast;

	default:
		PR_RunError ("WriteDest: bad destination");
		break;
	}
	
	return NULL;
}

static client_t *Write_GetClient(void)
{
	int		entnum;
	edict_t	*ent;

	ent = PROG_TO_EDICT(pr_global_struct->msg_entity);
	entnum = NUM_FOR_EDICT(ent);
	if (entnum < 1 || entnum > MAX_CLIENTS)
		PR_RunError ("WriteDest: not a client");
	return &svs.clients[entnum-1];
}


void PF_WriteByte (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 1);
		ClientReliableWrite_Byte(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 1);
			MSG_WriteByte((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteByte (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteChar (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 1);
		ClientReliableWrite_Char(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 1);
			MSG_WriteByte((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteChar (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteShort (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 2);
		ClientReliableWrite_Short(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 2);
			MSG_WriteShort((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteShort (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteLong (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 4);
		ClientReliableWrite_Long(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 4);
			MSG_WriteLong((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteLong (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteAngle (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 1);
		ClientReliableWrite_Angle(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 1);
			MSG_WriteByte((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteAngle (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteCoord (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 2);
		ClientReliableWrite_Coord(cl, G_FLOAT(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 2);
			MSG_WriteCoord((sizebuf_t*)demo.dbuf, G_FLOAT(OFS_PARM1));
		}
	} else
		MSG_WriteCoord (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteString (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 1+strlen(G_STRING(OFS_PARM1)));
		ClientReliableWrite_String(cl, G_STRING(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 1 + strlen(G_STRING(OFS_PARM1)));
			MSG_WriteString((sizebuf_t*)demo.dbuf, G_STRING(OFS_PARM1));
		}
	} else
		MSG_WriteString (WriteDest(), G_STRING(OFS_PARM1));
}


void PF_WriteEntity (void)
{
	if (G_FLOAT(OFS_PARM0) == MSG_ONE) {
		client_t *cl = Write_GetClient();
		ClientReliableCheckBlock(cl, 2);
		ClientReliableWrite_Short(cl, G_EDICTNUM(OFS_PARM1));
		if (sv.demorecording)
		{
			DemoWrite_Begin(dem_single, cl - svs.clients, 2);
			MSG_WriteShort((sizebuf_t*)demo.dbuf, G_EDICTNUM(OFS_PARM1));
		}
	} else
		MSG_WriteShort (WriteDest(), G_EDICTNUM(OFS_PARM1));
}

//=============================================================================

int SV_ModelIndex (char *name);

void PF_makestatic (void)
{
	edict_t	*ent;
	int		i;
	
	ent = G_EDICT(OFS_PARM0);

	MSG_WriteByte (&sv.signon,svc_spawnstatic);

	MSG_WriteByte (&sv.signon, SV_ModelIndex(PR_GetString(ent->v.model)));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}

// throw the entity away now
	ED_Free (ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
void PF_setspawnparms (void)
{
	edict_t	*ent;
	int		i;
	client_t	*client;

	ent = G_EDICT(OFS_PARM0);
	i = NUM_FOR_EDICT(ent);
	if (i < 1 || i > MAX_CLIENTS)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);

	for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
		(&pr_global_struct->parm1)[i] = client->spawn_parms[i];
}

/*
==============
PF_changelevel
==============
*/
void PF_changelevel (void)
{
	char	*s;
	static	int	last_spawncount;

// make sure we don't issue two changelevels
	if (svs.spawncount == last_spawncount)
		return;
	last_spawncount = svs.spawncount;
	
	s = G_STRING(OFS_PARM0);
	Cbuf_AddText (va("map %s\n",s));
}


/*
==============
PF_logfrag

logfrag (killer, killee)
==============
*/
void PF_logfrag (void)
{
	edict_t	*ent1, *ent2;
	int		e1, e2;
	char	*s;
	// -> scream
	time_t		t;
	struct tm	*tblock;
	// <-


	ent1 = G_EDICT(OFS_PARM0);
	ent2 = G_EDICT(OFS_PARM1);

	e1 = NUM_FOR_EDICT(ent1);
	e2 = NUM_FOR_EDICT(ent2);
	
	if (e1 < 1 || e1 > MAX_CLIENTS
	|| e2 < 1 || e2 > MAX_CLIENTS)
		return;
	// -> scream
	t = time (NULL);
	tblock = localtime (&t);

	if (Cvar_VariableValue ("frag_log_type")) // need for old-style frag log file
		s = va("\\frag\\%s\\%s\\%s\\%s\\%d-%d-%d %d:%d:%d\\\n",svs.clients[e1-1].name, svs.clients[e2-1].name,svs.clients[e1-1].team, svs.clients[e2-1].team,
				tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday,
				tblock->tm_hour, tblock->tm_min, tblock->tm_sec);
	else
		s = va("\\%s\\%s\\\n",svs.clients[e1-1].name, svs.clients[e2-1].name);
	// <-
	SZ_Print (&svs.log[svs.logsequence&1], s);
	SV_Write_Log(FRAG_LOG, 1, s);
}


/*
==============
PF_infokey

string(entity e, string key) infokey
==============
*/
void PF_infokey (void)
{
	edict_t	*e;
	int		e1;
	char	*value;
	char	*key;
	static	char ov[256];
	client_t *cl;

	e = G_EDICT(OFS_PARM0);
	e1 = NUM_FOR_EDICT(e);
	key = G_STRING(OFS_PARM1);
	cl = &svs.clients[e1-1];

	if (e1 == 0) {
		if ((value = Info_ValueForKey (svs.info, key)) == NULL || !*value)
			value = Info_ValueForKey(localinfo, key);
	} else if (e1 <= MAX_CLIENTS) {
		value = ov;
		if (!strncmp(key, "ip", 3))
			strlcpy(ov, NET_BaseAdrToString (cl->netchan.remote_address), sizeof(ov));
		else if (!strncmp(key, "realip", 7))
			strlcpy(ov, NET_BaseAdrToString (cl->realip), sizeof(ov));
		else if (!strncmp(key, "download", 9))
			snprintf(ov, sizeof(ov), "%d", cl->download != NULL ? (int)(100*cl->downloadcount/cl->downloadsize) : -1);
		else if (!strncmp(key, "ping", 5))
			snprintf(ov, sizeof(ov), "%d", SV_CalcPing (cl));
		else if (!strncmp(key, "login", 6))
			value = cl->login;
		else
			value = Info_ValueForKey (cl->userinfo, key);
	} else
		value = "";

	strlcpy(pr_string_temp, value, MAX_PR_STRING_SIZE);
	RETURN_STRING(pr_string_temp);
	PF_SetTempString();
}

/*
==============
PF_stof

float(string s) stof
==============
*/
void PF_stof (void)
{
	char	*s;

	s = G_STRING(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = atof(s);
}


/*
==============
PF_multicast

void(vector where, float set) multicast
==============
*/
void PF_multicast (void)
{
	float	*o;
	int		to;

	o = G_VECTOR(OFS_PARM0);
	to = G_FLOAT(OFS_PARM1);

	SV_Multicast (o, to);
}


void PF_Fixme (void)
{
	PR_RunError ("unimplemented bulitin");
}



builtin_t pr_builtin[] =
{
	PF_Fixme,
PF_makevectors,	// void(entity e)	makevectors 		= #1;
PF_setorigin,	// void(entity e, vector o) setorigin	= #2;
PF_setmodel,	// void(entity e, string m) setmodel	= #3;
PF_setsize,	// void(entity e, vector min, vector max) setsize = #4;
PF_Fixme,	// void(entity e, vector min, vector max) setabssize = #5;
PF_break,	// void() break						= #6;
PF_random,	// float() random						= #7;
PF_sound,	// void(entity e, float chan, string samp) sound = #8;
PF_normalize,	// vector(vector v) normalize			= #9;
PF_error,	// void(string e) error				= #10;
PF_objerror,	// void(string e) objerror				= #11;
PF_vlen,	// float(vector v) vlen				= #12;
PF_vectoyaw,	// float(vector v) vectoyaw		= #13;
PF_Spawn,	// entity() spawn						= #14;
PF_Remove,	// void(entity e) remove				= #15;
PF_traceline,	// float(vector v1, vector v2, float tryents) traceline = #16;
PF_checkclient,	// entity() clientlist					= #17;
PF_Find,	// entity(entity start, .string fld, string match) find = #18;
PF_precache_sound,	// void(string s) precache_sound		= #19;
PF_precache_model,	// void(string s) precache_model		= #20;
PF_stuffcmd,	// void(entity client, string s)stuffcmd = #21;
PF_findradius,	// entity(vector org, float rad) findradius = #22;
PF_bprint,	// void(string s) bprint				= #23;
PF_sprint,	// void(entity client, string s) sprint = #24;
PF_dprint,	// void(string s) dprint				= #25;
PF_ftos,	// void(string s) ftos				= #26;
PF_vtos,	// void(string s) vtos				= #27;
PF_coredump,
PF_traceon,
PF_traceoff,
PF_eprint,	// void(entity e) debug print an entire entity
PF_walkmove, // float(float yaw, float dist) walkmove
PF_Fixme, // float(float yaw, float dist) walkmove
PF_droptofloor,
PF_lightstyle,
PF_rint,
PF_floor,
PF_ceil,
PF_Fixme,
PF_checkbottom,
PF_pointcontents,
PF_Fixme,
PF_fabs,
PF_aim,
PF_cvar,
PF_localcmd,
PF_nextent,
PF_Fixme,
PF_changeyaw,
PF_Fixme,
PF_vectoangles,

PF_WriteByte,
PF_WriteChar,
PF_WriteShort,
PF_WriteLong,
PF_WriteCoord,
PF_WriteAngle,
PF_WriteString,
PF_WriteEntity,

PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,
PF_Fixme,

SV_MoveToGoal,
PF_precache_file,
PF_makestatic,

PF_changelevel,
PF_Fixme,

PF_cvar_set,
PF_centerprint,

PF_ambientsound,

PF_precache_model,
PF_precache_sound,		// precache_sound2 is different only for qcc
PF_precache_file,

PF_setspawnparms,

PF_logfrag,

PF_infokey,
PF_stof,
PF_multicast,
PF_executecmd,	// #83
PF_tokanize,
PF_argc,
PF_argv,
PF_teamfield,
PF_substr,
PF_strcat,
PF_strlen,
PF_str2byte,
PF_str2short,
PF_newstr,
PF_freestr,
PF_conprint,
PF_readcmd,
PF_strcpy,
PF_strstr,
PF_strncpy,
PF_log,
PF_redirectcmd,
PF_calltimeofday,
PF_forcedemoframe	//103
};

builtin_t *pr_builtins = pr_builtin;
int pr_numbuiltins = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

