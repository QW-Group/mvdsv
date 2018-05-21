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

#include "qwsvdef.h"


static cvar_t	*cvar_hash[32];
static cvar_t	*cvar_vars;
static char	*cvar_null_string = "";

/*
============
Cvar_Next

Use this to walk through all vars.
============
*/
cvar_t *Cvar_Next (cvar_t *var)
{
	if (var)
		return var->next;
	else
		return cvar_vars;
}

/*
============
Cvar_GetFlags
============
*/
int Cvar_GetFlags (cvar_t *var)
{
	return var->flags;
}

/*
============
Cvar_Find
============
*/
cvar_t *Cvar_Find (const char *var_name)
{
	cvar_t *var;
	int key;

	key = Com_HashKey (var_name);

	for (var=cvar_hash[key] ; var ; var=var->hash_next)
		if (!strcasecmp (var_name, var->name))
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
	cvar_t	*var;

	var = Cvar_Find (var_name);
	if (!var)
		return 0;
	return Q_atof (var->string);
}


/*
============
Cvar_String
============
*/
char *Cvar_String (const char *var_name)
{
	cvar_t *var;

	var = Cvar_Find (var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}

void SV_SendServerInfoChange(char *key, char *value);


/*
============
Cvar_Set
============
*/
void Cvar_Set (cvar_t *var, char *value)
{
	static qbool changing = false;
	char *tmp;

	if (!var)
		return;

	// force serverinfo "0" vars to be "".
	if ((var->flags & CVAR_SERVERINFO) && !strcmp(value, "0"))
		value = "";

	if (var->flags & CVAR_ROM)
		return;

	if (var->OnChange && !changing)
	{
		qbool cancel = false;

		changing = true;
		var->OnChange(var, value, &cancel);
		changing = false;

		if (cancel)
			return; // change rejected.
	}

	// dup string first (before free) since 'value' and 'var->string' can point at the same memory area.
	tmp = Q_strdup(value);
	// free the old value string
	Q_free (var->string);
	// assign new value.
	var->string = tmp;
	var->value = Q_atof (var->string);

	if (var->flags & CVAR_SERVERINFO)
	{
		SV_ServerinfoChanged (var->name, var->string);
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
void Cvar_SetByName (const char *var_name, char *value)
{
	cvar_t	*var;

	var = Cvar_Find (var_name);
	if (!var)
	{	// there is an error in C code if this happens
		Con_DPrintf ("Cvar_Set: variable %s not found\n", var_name);
		return;
	}

	Cvar_Set (var, value);
}

/*
============
Cvar_SetValue
============
*/
void Cvar_SetValue (cvar_t *var, const float value)
{
	char val[32];
	int	i;

	snprintf (val, sizeof (val), "%f", value);
	for (i = strlen (val) - 1; i > 0 && val[i]=='0'; i--)
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
void Cvar_SetValueByName (const char *var_name, const float value)
{
	char val[32];

	snprintf (val, sizeof (val), "%.8g", value);
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
	char	*value;
	int		key;

	// first check to see if it has already been defined
	if (Cvar_Find (variable->name))
	{
		Con_Printf ("Can't register variable %s, already defined\n", variable->name);
		return;
	}

	// check for overlap with a command
	if (Cmd_Exists (variable->name))
	{
		Con_Printf ("Cvar_Register: %s is a command\n", variable->name);
		return;
	}

	// link the variable in
	key = Com_HashKey (variable->name);
	variable->hash_next = cvar_hash[key];
	cvar_hash[key] = variable;
	variable->next = cvar_vars;
	cvar_vars = variable;

	// set it through the function to be consistent
	value = variable->string;
	variable->string = Q_strdup("");
	Cvar_SetROM (variable, value);
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
	v = Cvar_Find (Cmd_Argv(0));
	if (!v)
		return false;

	// perform a variable print or set
	c = Cmd_Argc();
	if (c == 1)
	{
		Con_Printf ("\"%s\" is \"%s\"\n", v->name, v->string);
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
		Con_Printf ("toggle <cvar> : toggle a cvar on/off\n");
		return;
	}

	var = Cvar_Find (Cmd_Argv(1));
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

static int Cvar_CvarCompare (const void *p1, const void *p2)
{
	return strcmp ((*((cvar_t **) p1))->name, (*((cvar_t **) p2))->name);
}

static void Cvar_CvarList_f(void)
{
	cvar_t **sorted_cvars = NULL;
	cvar_t *var;
	const char *pattern;
	int i = 0;
	int num_cvars = 0;
	int pattern_matched = 0;

	pattern = (Cmd_Argc() > 1) ? Cmd_Argv(1) : NULL;

	for (var = cvar_vars; var; var = var->next) {
		num_cvars++;
	}

	if (num_cvars > 0) {
		sorted_cvars = malloc(num_cvars * sizeof(cvar_t*));
		if (!sorted_cvars) {
			Sys_Error("Failed to allocate memory");
		}
	}
	else {
		Con_Printf("No cvars found\n");
		return;
	}

	for (var = cvar_vars; var; var = var->next) {
		sorted_cvars[i++] = var;
	}
	qsort(sorted_cvars, num_cvars, sizeof(cvar_t *), Cvar_CvarCompare);

	Con_Printf("List of cvars:\n");
	for (i = 0; i < num_cvars; i++) {
		var = sorted_cvars[i];
		if (pattern && !Q_glob_match(pattern, var->name)) {
			continue;
		}

		Con_Printf("%c %s %s\n", var->flags & CVAR_SERVERINFO ? 's' : ' ', var->name, var->string);
		pattern_matched++;
	}

	Con_Printf("------------\n%d/%d %svariables\n", pattern_matched, num_cvars, (pattern) ? "matching " : "");
	free(sorted_cvars);
}

/*
===========
Cvar_Create
===========
*/
cvar_t *Cvar_Create (const char *name, char *string, int cvarflags)
{
	cvar_t		*v;
	int			key;

	v = Cvar_Find(name);
	if (v)
		return v;
	v = (cvar_t *) Q_malloc (sizeof(cvar_t));
	// Cvar doesn't exist, so we create it
	v->next = cvar_vars;
	cvar_vars = v;

	key = Com_HashKey (name);
	v->hash_next = cvar_hash[key];
	cvar_hash[key] = v;

	v->name = (char *) Q_strdup (name);
	v->string = (char *) Q_strdup (string);
	v->flags = cvarflags;
	v->value = Q_atof (v->string);
	v->OnChange = NULL;

	return v;
}

/*
===========
Cvar_Delete
===========
returns true if the cvar was found (and deleted)
*/
qbool Cvar_Delete (const char *name)
{
	cvar_t	*var, *prev;
	int		key;

	key = Com_HashKey (name);

	prev = NULL;
	for (var = cvar_hash[key] ; var ; var=var->hash_next)
	{
		if (!strcasecmp(var->name, name))
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
		if (!strcasecmp(var->name, name))
		{
			// unlink from cvar list
			if (prev)
				prev->next = var->next;
			else
				cvar_vars = var->next;

			// free
			Q_free (var->string);
			Q_free (var->name);
			Q_free (var);
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
		Con_Printf ("usage: set <cvar> <value>\n");
		return;
	}

	var_name = Cmd_Argv (1);
	var = Cvar_Find (var_name);

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
		Con_Printf ("inc <cvar> [value]\n");
		return;
	}

	var = Cvar_Find (Cmd_Argv(1));
	if (!var)
	{
		Con_Printf ("Unknown variable \"%s\"\n", Cmd_Argv(1));
		return;
	}

	if (c == 3)
		delta = Q_atof (Cmd_Argv(2));
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
	Cmd_AddCommand ("cvardump", Cvar_CvarList_f);
	Cmd_AddCommand ("toggle", Cvar_Toggle_f);
	Cmd_AddCommand ("set", Cvar_Set_f); //DP_CON_SET
	Cmd_AddCommand ("inc", Cvar_Inc_f);

#ifdef CVAR_DEBUG
	Cmd_AddCommand ("cvar_hash_print", Cvar_Hash_Print_f);
#endif
}
