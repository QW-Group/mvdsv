// Portions Copyright (C) 2000 by Anton Gavrilov (tonik@quake.ru)

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
// cvar.c -- dynamic variable tracking

#ifdef SERVERONLY 
#include "qwsvdef.h"
#else
#include "quakedef.h"
#endif

#ifdef QW_BOTH
#include "progs.h"		// FIXME
#include "server.h"
#endif

static cvar_t	*cvar_hash[32];
static cvar_t	*cvar_vars;
static char		*cvar_null_string = "";


/*
==========
Key
==========
Returns hash key for a string
*/
static int Key (char *name)
{
	int	v;
	int c;

	v = 0;
	while ( (c = *name++) != 0 )
//		v += *name;
		v += c &~ 32;	// very lame, but works (case insensitivity)

	return v % 32;
}

/*
============
Cvar_FindVar
============
*/
cvar_t *Cvar_FindVar (char *var_name)
{
	cvar_t	*var;
	int		key;

	key = Key (var_name);
	
	for (var=cvar_hash[key] ; var ; var=var->hash_next)
		if (!Q_strcasecmp (var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_VariableValue
============
*/
float Cvar_VariableValue (char *var_name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return Q_atof (var->string);
}


/*
============
Cvar_VariableString
============
*/
char *Cvar_VariableString (char *var_name)
{
	cvar_t *var;
	
	var = Cvar_FindVar (var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}


/*
============
Cvar_CompleteVariable
============
*/
char *Cvar_CompleteVariable (char *partial)
{
	cvar_t		*cvar;
	int			len;
	
	len = strlen(partial);
	
	if (!len)
		return NULL;
		
	// check exact match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!Q_strcasecmp (partial,cvar->name))
			return cvar->name;

	// check partial match
	for (cvar=cvar_vars ; cvar ; cvar=cvar->next)
		if (!Q_strncasecmp (partial,cvar->name, len))
			return cvar->name;

	return NULL;
}


#ifdef SERVERONLY
void SV_SendServerInfoChange(char *key, char *value);
#endif

/*
============
Cvar_Set
============
*/
void Cvar_Set (cvar_t *var, char *value)
{
	static qboolean	changing = false;

	if (!var)
		return;

	if (var->OnChange && !changing) {
		changing = true;
		if (var->OnChange(var, value)) {
			changing = false;
			return;
		}
		changing = false;
	}

	Z_Free (var->string);	// free the old value string
	
	var->string = Z_Malloc (strlen(value)+1);
	strcpy (var->string, value);
	var->value = Q_atof (var->string);

#if defined(SERVERONLY) || defined(QW_BOTH)
	if (var->flags & CVAR_SERVERINFO)
	{
		Info_SetValueForKey (svs.info, var->name, var->string, MAX_SERVERINFO_STRING);
		SV_SendServerInfoChange(var->name, var->string);
//		SV_BroadcastCommand ("fullserverinfo \"%s\"\n", svs.info);
	}
#endif

#ifndef SERVERONLY
	if (var->flags & CVAR_USERINFO)
	{
		Info_SetValueForKey (cls.userinfo, var->name, var->string, MAX_INFO_STRING);
		if (cls.state >= ca_connected)
		{
			MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
			SZ_Print (&cls.netchan.message, va("setinfo \"%s\" \"%s\"\n", var->name, var->string));
		}
	}
#endif
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (cvar_t *var, float value)
{
	char	val[32];
	int	i;
	
	sprintf (val, "%f", value);
	for (i=strlen(val)-1 ; i>0 && val[i]=='0' ; i--)
		val[i] = 0;
	if (val[i] == '.')
		val[i] = 0;
	Cvar_Set (var, val);
}

/*
============
Cvar_RegisterVariable

Adds a freestanding variable to the variable list.
============
*/
void Cvar_RegisterVariable (cvar_t *variable)
{
	char	value[512];
	int		key;

// first check to see if it has already been defined
	if (Cvar_FindVar (variable->name))
	{
		Con_Printf ("Can't register variable %s, already defined\n", variable->name);
		return;
	}
	
// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_RegisterVariable: %s is a command\n", variable->name);
		return;
	}
		
// link the variable in
	key = Key (variable->name);
	variable->hash_next = cvar_hash[key];
	cvar_hash[key] = variable;
	variable->next = cvar_vars;
	cvar_vars = variable;

// copy the value off, because future sets will Z_Free it
	strcpy (value, variable->string);
	variable->string = Z_Malloc (1);	
	
// set it through the function to be consistent
	Cvar_Set (variable, value);
}


/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
qboolean Cvar_Command (void)
{
	cvar_t			*v;

// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;
		
// perform a variable print or set
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set (v, Cmd_Argv(1));
	return true;
}


/*
============
Cvar_WriteVariables

Writes lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void Cvar_WriteVariables (FILE *f)
{
	cvar_t	*var;
	
	for (var = cvar_vars ; var ; var = var->next)
		if (var->flags & CVAR_ARCHIVE)
			fprintf (f, "%s \"%s\"\n", var->name, var->string);
}


/*
=============
Cvar_Toggle_f
=============
*/
void Cvar_Toggle_f (void)
{
	cvar_t *var;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("toggle <cvar> : toggle a cvar on/off\n");
		return;
	}

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	Cvar_Set (var, var->value ? "0" : "1");
}

/*
===============
Cvar_CvarList_f
===============
List all cvars
TODO: allow cvar name mask as a parameter, e.g. cvarlist cl_*
*/
void Cvar_CvarList_f (void)
{
	cvar_t	*var;
	int i;

	for (var=cvar_vars, i=0 ; var ; var=var->next, i++)
		Con_Printf("%c%c%c %s\n",
			var->flags & CVAR_ARCHIVE ? '*' : ' ',
			var->flags & CVAR_USERINFO ? 'u' : ' ',
			var->flags & CVAR_SERVERINFO ? 's' : ' ',
			var->name);

	Con_Printf ("------------\n%d variables\n", i);
}

/*
===========
Cvar_Create
===========
*/
cvar_t *Cvar_Create (char *name, char *string, int cvarflags)
{
	cvar_t		*v;
	int			key;

	v = Cvar_FindVar(name);
	if (v)
		return v;
	v = (cvar_t *) Z_Malloc(sizeof(cvar_t));
	// Cvar doesn't exist, so we create it
	v->next = cvar_vars;
	cvar_vars = v;

	key = Key (name);
	v->hash_next = cvar_hash[key];
	cvar_hash[key] = v;

	v->name = Z_Malloc(strlen(name)+1);
	strcpy (v->name, name);
	v->string = Z_Malloc (strlen(string)+1);
	strcpy (v->string, string);
	v->flags = cvarflags;
	v->value = Q_atof (v->string);

	return v;
}

/*
===========
Cvar_Delete
===========
returns true if the cvar was found (and deleted)
*/
qboolean Cvar_Delete (char *name)
{
	cvar_t	*var, *prev;
	int		key;

	key = Key (name);

	prev = NULL;
	for (var = cvar_hash[key] ; var ; var=var->hash_next)
	{
		if (!Q_strcasecmp(var->name, name)) {
			// unlink from hash
			if (prev)
				prev->hash_next = var->next;
			else
				cvar_hash[key] = var->next;
			break;
		}
		prev = var;
	}

	if (!var)
		return false;

	prev = NULL;
	for (var = cvar_vars ; var ; var=var->next)
	{
		if (!Q_strcasecmp(var->name, name)) {
			// unlink from cvar list
			if (prev)
				prev->next = var->next;
			else
				cvar_vars = var->next;

			// free
			Z_Free (var->string);
			Z_Free (var->name);
			Z_Free (var);
			return true;
		}
		prev = var;
	}

	Sys_Error ("Cvar list broken");
	return false;	// shut up compiler
}


void Cvar_Set_f (void)
{
	cvar_t *var;
	char *var_name;

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("usage: set <cvar> <value>\n");
		return;
	}

	var_name = Cmd_Argv (1);
	var = Cvar_FindVar (var_name);

	if (var)
	{
		Cvar_Set (var, Cmd_Argv(2));
	}
	else
	{
		if (Cmd_Exists(var_name))
		{
			Con_Printf ("\"%s\" is a command\n", var_name);
			return;
		}

		// delete alias with the same name if it exists
		Cmd_DeleteAlias (var_name);

		var = Cvar_Create (var_name, Cmd_Argv(2), CVAR_USER_CREATED);
	}
}

void Cvar_Inc_f (void)
{
	int		c;
	cvar_t	*var;
	float	delta;

	c = Cmd_Argc();
	if (c != 2 && c != 3) {
		Con_Printf ("inc <cvar> [value]\n");
		return;
	}

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var) {
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	if (c == 3)
		delta = atof (Cmd_Argv(2));
	else
		delta = 1;

	Cvar_SetValue (var, var->value + delta);
}

//#define CVAR_DEBUG
#ifdef CVAR_DEBUG
static void Cvar_Hash_Print_f (void)
{
	int		i, count;
	cvar_t	*cvar;

	Con_Printf ("Cvar hash:\n");
	for (i = 0; i<32; i++)
	{
		count = 0;
		for (cvar = cvar_hash[i]; cvar; cvar=cvar->hash_next, count++);
		Con_Printf ("%i: %i\n", i, count);
	}

}
#endif

void Cvar_Init (void)
{
	Cmd_AddCommand ("cvarlist", Cvar_CvarList_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("set", Cvar_Set_f);
	Cmd_AddCommand ("inc", Cvar_Inc_f);

#ifdef CVAR_DEBUG
	Cmd_AddCommand ("cvar_hash_print", Cvar_Hash_Print_f);
#endif
}
