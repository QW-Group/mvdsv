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


static cvar_t	*cvar_hash[32];
static cvar_t	*cvar_vars;
static char	*cvar_null_string = "";


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
		if (!stricmp (var_name, var->name))
			return var;

	return NULL;
}

/*
============
Cvar_Value
============
*/
float Cvar_Value (char *var_name)
{
	cvar_t	*var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return 0;
	return atof (var->string);
}


/*
============
Cvar_String
============
*/
char *Cvar_String (char *var_name)
{
	cvar_t *var;

	var = Cvar_FindVar (var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}

void SV_SendServerInfoChange(char *key, char *value)
{
	Sys_Printf("SV_SendServerInfoChange FIXME\n");
}


/*
============
Cvar_Set
============
*/
void Cvar_Set (cvar_t *var, char *value)
{
	char *tmp;
//	static qbool changing = false;

	if (!var)
		return;

	if (var->flags & CVAR_ROM)
		return;

/* evil, do not use plz
	if (var->OnChange && !changing)
	{
		changing = true;
		if (var->OnChange(var, value))
		{
			changing = false;
			return;
		}
		changing = false;
	}
*/

	tmp = (char *) Sys_malloc (strlen(value) + 1);	// alloc new buffer
	strlcpy (tmp, value, strlen(value) + 1);		// copy to new buffer
	Sys_free (var->string);							// NOW SAFE to free the old buffer
	var->string = tmp;								// assign new buffer
	var->value = atof (var->string);				// set right value
	var->integer = atoi (var->string);
	var->modified = true;

	if (var->flags & CVAR_SERVERINFO)
	{
		char buf[MAX_INFO_KEY];
		if (strcmp(var->string, Info_ValueForKey (ps.info, var->name, buf, sizeof(buf))))
		{
			Info_SetValueForStarKey (ps.info, var->name, var->string, sizeof(ps.info));
//			SV_SendServerInfoChange(var->name, var->string);
		}
	}
}

/*
============
Cvar_SetROM
============
*/
void Cvar_SetROM (cvar_t *var, char *value)
{
	int saved_flags;

	if (!var)
		return;

	saved_flags = var->flags;
	var->flags &= ~CVAR_ROM;
	Cvar_Set (var, value);
	var->flags = saved_flags;
}

/*
============
Cvar_SetByName
============
*/
void Cvar_SetByName (char *var_name, char *value)
{
	cvar_t	*var;

	var = Cvar_FindVar (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Sys_Printf("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	Cvar_Set (var, value);
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

	snprintf (val, sizeof(val), "%f", value);
	for (i=strlen(val)-1 ; i>0 && val[i]=='0' ; i--)
		val[i] = 0;
	if (val[i] == '.')
		val[i] = 0;
	Cvar_Set (var, val);
}

/*
============
Cvar_SetValueByName
============
*/
void Cvar_SetValueByName (char *var_name, float value)
{
	char	val[32];

	snprintf (val, sizeof(val), "%.8g",value);
	Cvar_SetByName (var_name, val);
}


/*
============
Cvar_Register
 
Adds a freestanding variable to the variable list.
============
*/
void Cvar_Register (cvar_t *variable)
{
	int		key;
	char	*tmp;

	// first check to see if it has already been defined
	if (Cvar_FindVar (variable->name))
	{
		Sys_Printf("Can't register variable %s, already defined\n", variable->name);
		return;
	}

	// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Sys_Printf("Cvar_Register: %s is a command\n", variable->name);
		return;
	}

	// link the variable in
	key = Key (variable->name);
	variable->hash_next = cvar_hash[key];
	cvar_hash[key] = variable;
	variable->next = cvar_vars;
	cvar_vars = variable;

	// well, variable->string is some static data there, so duplicate it with malloc, because future sets will Sys_free it
	tmp = Sys_malloc(strlen(variable->string) + 1);
	strlcpy(tmp, variable->string, strlen(variable->string) + 1);
	variable->string = tmp;
	variable->modified = true; // this optional, Cvar_SetROM() will set it anyway

	// set it through the function to be consistent
	Cvar_SetROM (variable, variable->string);
}


/*
============
Cvar_Command
 
Handles variable inspection and changing from the console
============
*/
qbool Cvar_Command (void)
{
	int		i, c;
	cvar_t		*v;
	char		string[1024];

	// check variables
	v = Cvar_FindVar (Cmd_Argv(0));
	if (!v)
		return false;

	// perform a variable print or set
	c = Cmd_Argc();
	if (c == 1)
	{
		Sys_Printf("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}

	string[0] = 0;
	for (i=1 ; i < c ; i++)
	{
		if (i > 1)
			strlcat (string, " ", sizeof(string));
		strlcat (string, Cmd_Argv(i), sizeof(string));
	}

	Cvar_Set (v, string);
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

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var)
	{
		Sys_Printf("Unknown variable \"%s\"\n", Cmd_Argv(1));
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
		Sys_Printf("%c%c %s\n",
		           var->flags & CVAR_ARCHIVE    ? '*' : ' ',
		           var->flags & CVAR_SERVERINFO ? 's' : ' ',
		           var->name);

	Sys_Printf("------------\n%d variables\n", i);
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
	v = (cvar_t *) Sys_malloc (sizeof(cvar_t));
	// Cvar doesn't exist, so we create it
	v->next = cvar_vars;
	cvar_vars = v;

	key = Key (name);
	v->hash_next = cvar_hash[key];
	cvar_hash[key] = v;

	v->name = (char *) Sys_malloc (strlen(name)+1);
	strlcpy (v->name, name, strlen(name) + 1);
	v->string = (char *) Sys_malloc (strlen(string)+1);
	strlcpy (v->string, string, strlen(string) + 1);
	v->flags = cvarflags;
	v->value = atof (v->string);
	v->integer = atoi (v->string);
	v->modified = true;
//	v->OnChange = NULL;

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
	if (var->flags & CVAR_USER_CREATED)
	{
		Sys_free (var->name);
		Sys_free (var);
	}
}

qbool Cvar_Delete (char *name)
{
	cvar_t	*var, *prev;
	int		key;

	key = Key (name);

	prev = NULL;
	for (var = cvar_hash[key] ; var ; var=var->hash_next)
	{
		if (!stricmp(var->name, name))
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
		if (!stricmp(var->name, name))
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

//DP_CON_SET
void Cvar_Set_f (void)
{
	cvar_t *var;
	char *var_name;

	if (Cmd_Argc() != 3)
	{
		Sys_Printf("usage: set <cvar> <value>\n");
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
			Sys_Printf("\"%s\" is a command\n", var_name);
			return;
		}

#if 0
		// delete alias with the same name if it exists
		Cmd_DeleteAlias (var_name);
#endif

		var = Cvar_Create (var_name, Cmd_Argv(2), CVAR_USER_CREATED);
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

	var = Cvar_FindVar (Cmd_Argv(1));
	if (!var)
	{
		Sys_Printf("Unknown variable \"%s\"\n", Cmd_Argv(1));
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
