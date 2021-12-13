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

// cvar.h
#ifndef __CVAR_H__
#define __CVAR_H__

/*

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console or prog code as well as accessed directly
in C code.

it is sufficient to initialize a cvar_t with just the first two fields, or
you can add a ,true flag for variables that you want saved to the configuration
file when the game is quit:

cvar_t	r_draworder = {"r_draworder","1"};
cvar_t	scr_screensize = {"screensize","1",CVAR_ARCHIVE};

Cvars must be registered before use, or they will have a 0 value instead of the float interpretation of the string.  Generally, all cvar_t declarations should be registered in the apropriate init function before any console commands are executed:
Cvar_Register (&host_framerate);


C code usually just references a cvar in place:
if ( r_draworder.value )

It could optionally ask for the value to be looked up for a string name:
if (Cvar_Value ("r_draworder"))

Interpreted prog code can access cvars with the cvar(name) or
cvar_set (name, value) internal functions:
teamplay = cvar("teamplay");
cvar_set ("registered", "1");

The user can access cvars from the console in two ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.
*/

// cvar flags
#define CVAR_NONE			(0)
#define CVAR_SERVERINFO		(1<<0)	// mirrored to serverinfo
#define CVAR_ROM			(1<<1)	// read only
#define	CVAR_USER_CREATED	(1<<2)	// created by a set command

typedef struct cvar_s
{
	char			*name;
	char			*string;
	int				flags;
	void			(*OnChange) (struct cvar_s *var, char *value, qbool *cancel);
	float			value;
	struct cvar_s	*hash_next;
	struct cvar_s	*next;
} cvar_t;

#define Cvar_SetCurrentGroup(...)		// ezquake compatibility
#define Cvar_ResetCurrentGroup(...)		// ezquake compatibility

void  Cvar_Register (cvar_t *variable);
// registers a cvar that already has the name, string, and optionally the
// archive elements set.

void Cvar_Set (cvar_t *var, char *value);
// equivalent to "<name> <variable>" typed at the console

void Cvar_SetROM (cvar_t *var, char *value);
// force a set even if the cvar is read only

void Cvar_SetByName (const char *var_name, char *value);
// equivalent to "<name> <variable>" typed at the console

void Cvar_SetValue (cvar_t *var, const float value);
// expands value to a string and calls Cvar_Set

void Cvar_SetValueByName (const char *var_name, const float value);
// expands value to a string and calls Cvar_Set

float Cvar_Value (const char *var_name);
// returns 0 if not defined or non numeric

char *Cvar_String (const char *var_name);
// returns an empty string if not defined

qbool Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

// Use this to walk through all vars
cvar_t *Cvar_Next (cvar_t *var);

int Cvar_GetFlags (cvar_t *var);

cvar_t *Cvar_Find (const char *var_name);
qbool Cvar_Delete (const char *name);

cvar_t *Cvar_Create (const char *name, char *string, int cvarflags);

void Cvar_Init (void);

char* Cvar_ServerInfoValue(char* key, char* value);

#endif /* !__CVAR_H__ */
