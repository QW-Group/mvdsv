/*
	teamplay.c

	Teamplay enhancements ("proxy features")

	Copyright (C) 2000       Anton Gavrilov (tonik@quake.ru)

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

	$Id: teamplay.h,v 1.1.1.1 2004/09/28 18:57:00 vvd0 Exp $
*/

extern cvar_t cl_parsesay;
extern cvar_t cl_triggers;
extern cvar_t cl_nofake;
extern cvar_t cl_loadlocs;
extern cvar_t cl_rocket2grenade;

extern int		loc_numentries;		// Tonik

void CL_StatChanged (int stat, int value);
void CL_NewMap (void);
void CL_ExecTrigger (char *s);

void Cmd_Macro_Init (void);
void CL_LoadLocFile (char *path, qboolean quiet);
char *CL_ParseMacroString(char *string);

int CL_CountPlayers();
char *CL_MapName();
char *CL_PlayerName();
char *CL_PlayerTeam();
char *CL_EnemyName();
char *CL_EnemyTeam();

void CL_InitTeamplay ();
