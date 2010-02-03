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

Some general info.

cvar_t variables are used to hold scalar or string variables that can be changed or displayed at the console
as well as accessed directly in C code.

It is sufficient to initialize a cvar_t with just the first two fields.

cvar_t	r_draworder    = {"r_draworder", "1"};
cvar_t	scr_screensize = {"screensize",  "1", CVAR_READONLY};

Cvars must be registered before use, or they will have a 0 value instead of the float interpretation of the string.
Generally, all cvar_t declarations should be registered in the apropriate init function before any console commands
are executed:
Cvar_Register (&host_framerate);

C code usually just references a cvar in place:
if ( r_draworder.value )

It could optionally ask for the value to be looked up for a string name:
if (Cvar_Value ("r_draworder"))

The user can access cvars from the console in two ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

*/

// cvar flags
#define CVAR_ARCHIVE		(1<<0)
#define CVAR_SERVERINFO		(1<<1)	// mirrored to serverinfo
#define CVAR_NOSET			(1<<2)	// don't allow change from console at all,
									// but can be set from the command line (or during init time)
#define CVAR_READONLY		(1<<3)	// don't allow changing by user, ever
#define	CVAR_USER_CREATED	(1<<4)	// created by a set command

typedef struct cvar_s
{
	char	*name;
	char	*string;
	int		flags;
	float	value;
	int		integer;
	qbool	modified;
	struct	cvar_s *hash_next;
	struct	cvar_s *next;
} cvar_t;

cvar_t *Cvar_Get (const char *var_name, const char *value, int flags);
// Creates the variable if it doesn't exist.
// If the variable already exists, the value will not be set (unless flags are CVAR_READONLY)
// The flags will be or'ed and default value overwritten in if the variable exists.

cvar_t *Cvar_Set (const char *var_name, const char *value);
// equivalent to "<name> <variable>" typed at the console

cvar_t *Cvar_ForceSet (const char *var_name, const char *value);
// force a set even if the cvar is read only

cvar_t *Cvar_FullSet (const char *var_name, const char *value, int flags);
// create or overwrite variable, must be used for some cricial variables

cvar_t *Cvar_SetValue (const char *var_name, float value);
// expands value to a string and calls Cvar_Set

float Cvar_Value (const char *var_name);
// returns 0 if not defined or non numeric

char *Cvar_String (const char *var_name);
// returns an empty string if not defined

qbool Cvar_Command (void);
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

cvar_t *Cvar_Find (const char *var_name);
qbool Cvar_Delete (const char *var_name);

void Cvar_DeInit (void);
void Cvar_Init (void);

#endif /* !__CVAR_H__ */
