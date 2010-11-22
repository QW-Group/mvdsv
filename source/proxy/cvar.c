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

#include "qwfwd.h"

static cvar_t *Cvar_Create(const char *var_name, const char *value, int cvarflags);
static cvar_t *Cvar_Set2(const char *var_name, const char *value, qbool force);

static cvar_t	*cvar_hash[32];
static cvar_t	*cvar_vars;

/*
==========
Key
==========
Returns hash key for a string
*/
static int Key (const char *name)
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
Cvar_Find
============
*/
cvar_t *Cvar_Find (const char *var_name)
{
	cvar_t	*var;
	int		key;

	key = Key (var_name);

	for (var=cvar_hash[key] ; var ; var=var->hash_next)
		if (!stricmp (var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_Value
============
*/
float Cvar_Value (const char *var_name)
{
	cvar_t	*var = Cvar_Find (var_name);
	return ( var ) ? var->value : 0;
}

/*
============
Cvar_String
============
*/
char *Cvar_String (const char *var_name)
{
	cvar_t *var = Cvar_Find (var_name);
	return ( var ) ? var->string : "";
}

/*
============
Cvar_Get
// Creates the variable if it doesn't exist.
// If the variable already exists, the value will not be set (unless ROM)
// The flags will be or'ed and default value overwritten in if the variable exists.
============
*/
cvar_t *Cvar_Get(const char *var_name, const char *value, int flags)
{
	cvar_t *var;

	if( !var_name || !var_name[0] )
	{
		Sys_Error("Cvar_Get: zero cvar name\n");
		return NULL;
	}

	if ( !value )
		value = "";

	var = Cvar_Find(var_name);

	if (var)
	{
		// var already exist.
		// in case ot READ ONLY we set it for sanity, so it can't be set to wrong previously by user etc.

		if (flags & CVAR_READONLY)
		{
			// read only var, reset flags and force set value
			var->flags = flags;
			Cvar_ForceSet(var_name, value);
		}
		else
		{
			// normal var, just "or" flags
			var->flags |= flags;		
		}
	}
	else
	{
		// create new one
		var = Cvar_Create (var_name, value, flags);
	}

	// remove possible user created flag
	if (!(flags & CVAR_USER_CREATED))
		var->flags &= ~CVAR_USER_CREATED;

	return var;
}

//
// FIXME
//
static void SV_SendServerInfoChange(char *key, char *value)
{
//	Sys_DPrintf("SV_SendServerInfoChange FIXME\n");
}

/*
============
Cvar_Set2
============
*/
static cvar_t *Cvar_Set2(const char *var_name, const char *value, qbool force)
{
	char *tmp;
	cvar_t	*var = Cvar_Find (var_name);

	if (!var)
	{
		return Cvar_Create(var_name, value, 0);
	}

	// not set if read only and not forced
	if (!force)
	{
		if ((var->flags & CVAR_READONLY) || ((var->flags & CVAR_NOSET) && ps.initialized))
		{
			Sys_Printf("%s is write protected.\n", var_name);
			return var;
		}
	}

	tmp = Sys_strdup (value);						// allocate new value, we do it before mem free so
													// Cvar_Set(var, var->string) works without problems
	Sys_free (var->string);							// NOW SAFE to free the old buffer
	var->string = tmp;								// assign new value
	var->value = atof (var->string);				// set right value
	var->integer = atoi (var->string);
	var->modified = true;

	if (var->flags & CVAR_SERVERINFO)
	{
		char buf[MAX_INFO_KEY];
		if (strcmp(var->string, Info_ValueForKey (ps.info, var->name, buf, sizeof(buf))))
		{
			Info_SetValueForStarKey (ps.info, var->name, var->string, sizeof(ps.info));
			SV_SendServerInfoChange(var->name, var->string);
		}
	}

	return var;
}

/*
============
Cvar_Set
============
*/
cvar_t *Cvar_Set(const char *var_name, const char *value)
{
	return Cvar_Set2(var_name, value, false);
}

/*
============
Cvar_ForceSet
============
*/
cvar_t *Cvar_ForceSet(const char *var_name, const char *value)
{
	return Cvar_Set2(var_name, value, true);
}

/*
============
Cvar_FullSet
============
 */
cvar_t *Cvar_FullSet(const char *var_name, const char *value, int flags)
{
	cvar_t *var;

	var = Cvar_Find(var_name);
	if( !var )
		return Cvar_Get(var_name, value, flags);

	var->flags = flags;
	return Cvar_ForceSet(var_name, value);
}

/*
============
Cvar_SetValue
============
*/
cvar_t *Cvar_SetValue(const char *var_name, float value)
{
	char	val[32];

	snprintf (val, sizeof (val), "%.8g", value);

	return Cvar_Set(var_name, val);
}

/*
===========
Cvar_Create
===========
*/
static cvar_t *Cvar_Create(const char *var_name, const char *value, int cvarflags)
{
	cvar_t		*v;
	int			key;

	v = Cvar_Find(var_name);
	if (v)
	{
		Sys_DPrintf("Cvar_Create: cvar %s already exist, unexpected\n", var_name);
		return v; // actually it should not happend
	}

	// Cvar doesn't exist, so we create it
	v = (cvar_t *) Sys_malloc (sizeof(cvar_t));

	// link it in
	v->next = cvar_vars;
	cvar_vars = v;
	key = Key (var_name);
	v->hash_next = cvar_hash[key];
	cvar_hash[key] = v;

	// set basic fields
	v->name = Sys_strdup(var_name);
	v->flags = cvarflags;

	// now set it for real
	Cvar_ForceSet(var_name, value);

	return v;
}

/*
===========
Cvar_Delete
===========
returns true if the cvar was found (and deleted)
*/

static void Cvar_Free(cvar_t *var)
{
	Sys_free (var->string);
	Sys_free (var->name);
	Sys_free (var);
}

qbool Cvar_Delete (const char *var_name)
{
	cvar_t	*var, *prev;
	int		key;

	key = Key (var_name);

	prev = NULL;
	for (var = cvar_hash[key] ; var ; var=var->hash_next)
	{
		if (!stricmp(var->name, var_name))
		{
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
		if (!stricmp(var->name, var_name))
		{
			// unlink from cvar list
			if (prev)
				prev->next = var->next;
			else
				cvar_vars = var->next;

			// free
			Cvar_Free(var);
			return true;
		}
		prev = var;
	}

	Sys_Error ("Cvar list broken");
	return false;	// shut up compiler
}

//===============================================================
// Cvars commands
//===============================================================

/*
============
Cvar_Command
 
Handles variable inspection and changing from the console
============
*/
qbool Cvar_Command (void)
{
	int			c;
	cvar_t		*v;
	char		string[1024];

	// check variables
	v = Cvar_Find (Cmd_Argv(0));
	if (!v)
		return false;

	// perform a variable print or set
	c = Cmd_Argc();

	if (c < 1)
		return false; // should not happend

	if (c == 1)
	{
		Sys_Printf("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	Cvar_Set (v->name, Cmd_Args_Range(1, c - 1, string, sizeof(string)));
	return true;
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
		Sys_Printf("toggle <cvar> : toggle a cvar on/off\n");
		return;
	}

	var = Cvar_Find (Cmd_Argv(1));
	if (!var)
	{
		Sys_Printf("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	Cvar_Set (var->name, var->value ? "0" : "1");
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
		Sys_Printf("%c%c %s\n",
		           var->flags & CVAR_ARCHIVE    ? '*' : ' ',
		           var->flags & CVAR_SERVERINFO ? 's' : ' ',
		           var->name);

	Sys_Printf("------------\n%d variables\n", i);
}

//DP_CON_SET
void Cvar_Set_f (void)
{
	cvar_t *var;
	char *var_name;
	char string[1024];

	if (Cmd_Argc() < 3)
	{
		Sys_Printf("usage: set <cvar> <value>\n");
		return;
	}

	var_name = Cmd_Argv (1);
	var = Cvar_Find (var_name);
	Cmd_Args_Range(2, Cmd_Argc() - 1, string, sizeof(string));

	if (var)
	{
		Cvar_Set (var_name, string);
	}
	else
	{
		var = Cvar_Create (var_name, string, CVAR_USER_CREATED);
	}
}

void Cvar_Inc_f (void)
{
	int		c;
	cvar_t	*var;
	float	delta;

	c = Cmd_Argc();
	if (c != 2 && c != 3)
	{
		Sys_Printf("inc <cvar> [value]\n");
		return;
	}

	var = Cvar_Find (Cmd_Argv(1));
	if (!var)
	{
		Sys_Printf("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	if (c == 3)
		delta = atof (Cmd_Argv(2));
	else
		delta = 1;

	Cvar_SetValue (var->name, var->value + delta);
}

//#define CVAR_DEBUG
#ifdef CVAR_DEBUG
static void Cvar_Hash_Print_f (void)
{
	int		i, count;
	cvar_t	*cvar;

	Sys_Printf("Cvar hash:\n");
	for (i = 0; i<32; i++)
	{
		count = 0;
		for (cvar = cvar_hash[i]; cvar; cvar=cvar->hash_next, count++);
			Sys_Printf("%i: %i\n", i, count);
	}

}
#endif

/*
===============
Cvar_DeInit
===============
Remove all cvars
*/
void Cvar_DeInit (void)
{
	cvar_t	*var, *next;

	for (var = cvar_vars; var; var = next)
	{
		next = var->next;
		Cvar_Free(var);
	}

	cvar_vars = NULL;
}

void Cvar_Init (void)
{
	Cmd_AddCommand ("cvarlist", Cvar_CvarList_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("set", Cvar_Set_f); //DP_CON_SET
	Cmd_AddCommand ("inc", Cvar_Inc_f);

#ifdef CVAR_DEBUG
	Cmd_AddCommand ("cvar_hash_print", Cvar_Hash_Print_f);
#endif
}

