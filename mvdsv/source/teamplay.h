/*
	teamplay.c

	Teamplay enhancements ("proxy features")

	Copyright (C) 2000-2001       Anton Gavrilov

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA
*/

extern cvar_t cl_parsesay;
extern cvar_t cl_triggers;
extern cvar_t cl_nofake;
extern cvar_t cl_loadlocs;
extern cvar_t cl_rocket2grenade;
extern cvar_t cl_teamskin;
extern cvar_t cl_enemyskin;

// triggers
void TP_ExecTrigger (char *s);
void TP_StatChanged (int stat, int value);
void TP_CheckPickupSound (char *s);

// message triggers
void TP_SearchForMsgTriggers (char *s, int level);

// used by easyrecord command
int TP_CountPlayers();
char *TP_MapName();
char *TP_PlayerName();
char *TP_PlayerTeam();
char *TP_EnemyName();
char *TP_EnemyTeam();

// teamcolor&enemycolor
extern int cl_teamtopcolor;
extern int cl_teambottomcolor;
extern int cl_enemytopcolor;
extern int cl_enemybottomcolor;

void TP_LoadLocFile (char *path, qboolean quiet);
char *TP_ParseMacroString(char *string);
void TP_NewMap ();
void TP_CheckVersionRequest(char *s);
int TP_CategorizeMessage (char *s);
void TP_FixTeamSets();

void TP_Init ();


//#define FPD_NO_TEAM_MACROS	1
#define FPD_NO_FORCE_SKIN	256
#define FPD_NO_FORCE_COLOR	512
