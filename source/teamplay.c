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

	$Id: teamplay.c,v 1.1.1.1 2004/09/28 18:57:00 vvd0 Exp $
*/

#include "quakedef.h"

cvar_t	_cmd_macros = {"_cmd_macros", "0"};
cvar_t	cl_parsesay = {"cl_parsesay", "0"};
cvar_t	cl_triggers = {"cl_triggers", "0"};
cvar_t	cl_nofake = {"cl_nofake", "0"};
cvar_t	cl_loadlocs = {"cl_loadlocs", "0"};

cvar_t	cl_rocket2grenade = {"cl_r2g", "0"};

//===========================================================================
//								TRIGGERS
//===========================================================================

// Defined later...
char *Cmd_Macro_Location_f (void);

void *Cmd_FindAlias(s);	 // hmm

#define MAX_LOC_NAME 32



int	last_health = 0;
int	last_items = 0;
int	last_respawntrigger = 0;
int	last_deathtrigger = 0;

char	lastdeathloc[MAX_LOC_NAME];

void CL_ExecTrigger (char *s)
{
	if (!cl_triggers.value)
		return;

	if (Cmd_FindAlias(s))
	{
		Cbuf_AddText (s);
		Cbuf_AddText ("\n");
		// maybe call Cbuf_Execute?
	}
}

#define	IT_WEAPONS (2|4|8|16|32|64)
void CL_StatChanged (int stat, int value)
{
	int		i;

	if (stat == STAT_HEALTH)
	{
		if (value > 0) {
			if (last_health <= 0 /*&& last_health != -999*/
				/* && Q_strcasecmp(Info_ValueForKey(cl.serverinfo, "status"),
				"standby") */)	// detect Kombat Teams status
			{
				extern cshift_t	cshift_empty;
				last_respawntrigger = realtime;
				//if (cl.teamfortress)
					memset (&cshift_empty, 0, sizeof(cshift_empty));
				CL_ExecTrigger ("f_respawn");
			}
			last_health = value;
			return;
		}
		if (last_health > 0) {		// We just died
			last_deathtrigger = realtime;
			strcpy (lastdeathloc, Cmd_Macro_Location_f());
			CL_ExecTrigger ("f_death");
		}
		last_health = value;
	}
	else if (stat == STAT_ITEMS)
	{
		i = value &~ last_items;
		if (i & IT_WEAPONS && (i & IT_WEAPONS != IT_WEAPONS)
		|| i & (IT_ARMOR1|IT_ARMOR2|IT_ARMOR3|IT_SUPERHEALTH)
		|| i & (IT_INVISIBILITY|IT_INVULNERABILITY|IT_SUIT|IT_QUAD))
		{
			// ...
			//CL_ExecTrigger ("f_took");
		}
		last_items = value;
	}
}

void CL_NewMap (void)
{
	last_health = 0;

}


//===========================================================================
//								MACROS
//===========================================================================


typedef char *(*mcommand_t) (void);

typedef struct cmd_macro_s
{
	struct cmd_macro_s	*next;
	char	*name;
	mcommand_t	function;
} cmd_macro_t;

cmd_macro_t	*cmd_macros;

#define MAX_MACRO_VALUE	255


void Cmd_AddMacro (char *macro_name, mcommand_t function)
{
	cmd_macro_t	*macro;
	
	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddMacro after host_initialized");
		
// fail if the macro is a variable name
	if (Cvar_VariableString(macro_name)[0])
	{
		Con_Printf ("Cmd_AddMacro: %s already defined as a var\n", macro_name);
		return;
	}
	
// fail if the macro already exists
	for (macro=cmd_macros ; macro ; macro=macro->next)
	{
		if (!strcmp (macro_name, macro->name))
		{
			Con_Printf ("Cmd_AddMacro: %s already defined\n", macro_name);
			return;
		}
	}

	macro = Hunk_Alloc (sizeof(cmd_macro_t));	// FIXME: use zone memory?
	macro->name = macro_name;
	macro->function = function;
	macro->next = cmd_macros;
	cmd_macros = macro;
}


/*
==========================================================================
						 MACRO FUNCTIONS DEFINITION
==========================================================================
*/

static char	macro_buf[MAX_MACRO_VALUE];

char *Cmd_Macro_Health_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_HEALTH]);
	return macro_buf;
}

char *Cmd_Macro_Armor_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_ARMOR]);
	return macro_buf;
}

char *Cmd_Macro_Shells_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_SHELLS]);
	return macro_buf;
}

char *Cmd_Macro_Nails_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_NAILS]);
	return macro_buf;
}

char *Cmd_Macro_Rockets_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_ROCKETS]);
	return macro_buf;
}

char *Cmd_Macro_Cells_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_CELLS]);
	return macro_buf;
}

char *Cmd_Macro_Ammo_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_AMMO]);
	return macro_buf;
}

char *Cmd_Macro_Weapon_f (void)
{
	switch (cl.stats[STAT_ACTIVEWEAPON])
	{
	case IT_AXE: return "axe";
	case IT_SHOTGUN: return "sg";
	case IT_SUPER_SHOTGUN: return "ssg";
	case IT_NAILGUN: return "ng";
	case IT_SUPER_NAILGUN: return "sng";
	case IT_GRENADE_LAUNCHER: return "gl";
	case IT_ROCKET_LAUNCHER: return "rl";
	case IT_LIGHTNING: return "lg";
	default:
		return "";
	}
}

int	_Cmd_Macro_BestWeapon (void)
{
	int	best;

	best = 0;
	if (cl.stats[STAT_ITEMS] & IT_AXE)
		best = IT_AXE;
	if (cl.stats[STAT_ITEMS] & IT_SHOTGUN && cl.stats[STAT_SHELLS] >= 1)
		best = IT_SHOTGUN;
	if (cl.stats[STAT_ITEMS] & IT_SUPER_SHOTGUN && cl.stats[STAT_SHELLS] >= 2)
		best = IT_SUPER_SHOTGUN;
	if (cl.stats[STAT_ITEMS] & IT_NAILGUN && cl.stats[STAT_NAILS] >= 1)
		best = IT_NAILGUN;
	if (cl.stats[STAT_ITEMS] & IT_SUPER_NAILGUN && cl.stats[STAT_NAILS] >= 2)
		best = IT_SUPER_NAILGUN;
	if (cl.stats[STAT_ITEMS] & IT_GRENADE_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
		best = IT_GRENADE_LAUNCHER;
	if (cl.stats[STAT_ITEMS] & IT_LIGHTNING && cl.stats[STAT_CELLS] >= 1)
		best = IT_LIGHTNING;
	if (cl.stats[STAT_ITEMS] & IT_ROCKET_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
		best = IT_ROCKET_LAUNCHER;

	return best;
}

char *Cmd_Macro_BestWeapon_f (void)
{
	switch (_Cmd_Macro_BestWeapon())
	{
	case IT_AXE: return "axe";
	case IT_SHOTGUN: return "sg";
	case IT_SUPER_SHOTGUN: return "ssg";
	case IT_NAILGUN: return "ng";
	case IT_SUPER_NAILGUN: return "sng";
	case IT_GRENADE_LAUNCHER: return "gl";
	case IT_ROCKET_LAUNCHER: return "rl";
	case IT_LIGHTNING: return "lg";
	default:
		return "";
	}
}

char *Cmd_Macro_BestAmmo_f (void)
{
	switch (_Cmd_Macro_BestWeapon())
	{
	case IT_SHOTGUN: case IT_SUPER_SHOTGUN: 
		sprintf(macro_buf, "%i", cl.stats[STAT_SHELLS]);
		return macro_buf;

	case IT_NAILGUN: case IT_SUPER_NAILGUN:
		sprintf(macro_buf, "%i", cl.stats[STAT_NAILS]);
		return macro_buf;

	case IT_GRENADE_LAUNCHER: case IT_ROCKET_LAUNCHER:
		sprintf(macro_buf, "%i", cl.stats[STAT_ROCKETS]);
		return macro_buf;

	case IT_LIGHTNING:
		sprintf(macro_buf, "%i", cl.stats[STAT_CELLS]);
		return macro_buf;

	default:
		return "0";
	}
}

char *Cmd_Macro_ArmorType_f (void)
{
	if (cl.stats[STAT_ITEMS] & IT_ARMOR1)
		return "ga";
	else if (cl.stats[STAT_ITEMS] & IT_ARMOR2)
		return "ya";
	else if (cl.stats[STAT_ITEMS] & IT_ARMOR3)
		return "ra";
	else
		return "";	// no armor at all
}

char *Cmd_Macro_Powerups_f (void)
{
	macro_buf[0] = 0;

	if (cl.stats[STAT_ITEMS] & IT_QUAD)
		strcpy(macro_buf, "quad");

	if (cl.stats[STAT_ITEMS] & IT_INVULNERABILITY)
	{
		if (macro_buf[0])
			strcat(macro_buf, "/");
		strcat(macro_buf, "pent");
	}

	if (cl.stats[STAT_ITEMS] & IT_INVISIBILITY)
	{
		if (macro_buf[0])
			strcat(macro_buf, "/");
		strcat(macro_buf, "ring");
	}

	return macro_buf;
}

char *Cmd_Macro_Location2_f (void)
{
	if (last_deathtrigger && realtime - last_deathtrigger <= 5)
		return lastdeathloc;
	return Cmd_Macro_Location_f();
}

char *Cmd_Macro_LastDeath_f (void)
{
	if (last_deathtrigger)
		return lastdeathloc;
	else
		return "someplace";
}


#define MAX_MACRO_STRING 1024

// TODO: rewrite this!
char *CL_ParseMacroString (char *string)
{
	static char	buf[MAX_MACRO_STRING];
	char	*s;
	int		i;
	cmd_macro_t	*macro;
	char	*macro_string;
	char	ch;

	if (!_cmd_macros.value && !cl_parsesay.value)
		return string;

	s = string;
	i = 0;

	while (*s && i < MAX_MACRO_STRING-1)
	{
		if (cl_parsesay.value == 1)
		{
			if (*s == '%')
			{
				macro_string = NULL;
				switch (s[1])
				{
				case 'a': macro_string = Cmd_Macro_Armor_f(); break;
				case 'A': macro_string = Cmd_Macro_ArmorType_f(); break;
				case 'c': macro_string = Cmd_Macro_Cells_f(); break;
				case 'd': macro_string = Cmd_Macro_LastDeath_f(); break;
				case 'h': macro_string = Cmd_Macro_Health_f(); break;
				case 'l': macro_string = Cmd_Macro_Location_f(); break;
				case 'L': macro_string = Cmd_Macro_Location2_f(); break;
				case 'P': macro_string = Cmd_Macro_Powerups_f(); break;
				case 'r': macro_string = Cmd_Macro_Rockets_f(); break;
				case 'w': macro_string = Cmd_Macro_Weapon_f(); break;
				case 'W': macro_string = Cmd_Macro_Ammo_f(); break;
				default: 
					buf[i++] = *s++;
					continue;
				}
					if (i + strlen(macro_string) >= MAX_MACRO_STRING-1) // !!! is this right?
						Sys_Error("CL_ParseMacroString: macro string length > MAX_MACRO_STRING)");
					strcpy (&buf[i], macro_string);
					i += strlen(macro_string);
					s += 2;	// skip % and letter
					continue;
			}

			if (*s == '$')
			{
				ch = 0;
				switch (s[1])
				{
				case '\\': ch = 13; break;
				case '[': ch = 0x90; break;
				case ']': ch = 0x91; break;
				case 'G': ch = 0x86; break;
				case 'R': ch = 0x87; break;
				case 'Y': ch = 0x88; break;
				case 'B': ch = 0x89; break;
				}

				if (ch) 
				{
					buf[i++] = ch;
					s += 2;
					continue;
				}

			}
		}

		if (_cmd_macros.value)
		if (*s == '$')
		{	
			s++;

			if (*s == '$')
			{
				buf[i++] = '$';
				s++;
				continue;
			}

		// macro search
			if (_cmd_macros.value)
			{
				macro = cmd_macros;
				while (macro)
				{
					if (!strncmp(s, macro->name, strlen(macro->name)))
					{
						macro_string = macro->function();
						if (i + strlen(macro_string) >= MAX_MACRO_STRING-1) // !!! is this right?
							Sys_Error("CL_ParseMacroString: macro string length > MAX_MACRO_STRING)");
						strcpy (&buf[i], macro_string);
						i += strlen(macro_string);
						s += strlen(macro->name);
						goto _continue;
					}
					macro = macro->next;
				}
			}

		// skip unknown macro name
			while (*s && ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z')))
				s++;	
			continue;
		}

		buf[i++] = *s++;

		_continue: ;
	}
	buf[i] = 0;

	return	buf;
}



void Cmd_Macro_Init (void)
{
	Cmd_AddMacro("health", Cmd_Macro_Health_f);
	Cmd_AddMacro("armortype", Cmd_Macro_ArmorType_f);
	Cmd_AddMacro("armor", Cmd_Macro_Armor_f);
	Cmd_AddMacro("shells", Cmd_Macro_Shells_f);
	Cmd_AddMacro("nails", Cmd_Macro_Nails_f);
	Cmd_AddMacro("rockets", Cmd_Macro_Rockets_f);
	Cmd_AddMacro("cells", Cmd_Macro_Cells_f);
	Cmd_AddMacro("weapon", Cmd_Macro_Weapon_f);
	Cmd_AddMacro("ammo", Cmd_Macro_Ammo_f);
	Cmd_AddMacro("bestweapon", Cmd_Macro_BestWeapon_f);
	Cmd_AddMacro("bestammo", Cmd_Macro_BestAmmo_f);
	Cmd_AddMacro("powerups", Cmd_Macro_Powerups_f);
	Cmd_AddMacro("location", Cmd_Macro_Location_f);
}


/*
=============================================================================

							PROXY .LOC FILES

=============================================================================
*/

typedef struct locdata_s {
	vec3_t coord;
	char name[MAX_LOC_NAME];
} locdata_t;

#define MAX_LOC_ENTRIES 1024

locdata_t locdata[MAX_LOC_ENTRIES];	// FIXME: allocate dynamically?
int	loc_numentries;

#define SKIPBLANKS(ptr) while (*ptr == ' ' || *ptr == 9 || *ptr == 13) ptr++
#define SKIPTOEOL(ptr) while (*ptr != 10 && *ptr == 0) ptr++

void CL_LoadLocFile (char *path, qboolean quiet)
{
	char	*buf, *p;
	int		i, n, sign;
	int		line;
	int		nameindex;
	int		mark;
	char	locname[MAX_OSPATH];

	if (!*path)
		return;

	strcpy (locname, "locs/");
	if (strlen(path) + strlen(locname) + 2+4 > MAX_OSPATH)
	{
		Con_Printf ("CL_LoadLocFile: path name > MAX_OSPATH\n");
		return;
	}
	strcat (locname, path);
	if (!strstr(locname, "."))	
		strcat (locname, ".loc");	// Add default extension

	mark = Hunk_LowMark ();
	buf = (char *) COM_LoadHunkFile (locname);

	if (!buf)
	{
		if (!quiet)
			Con_Printf ("Could not load %s\n", locname);
		return;
	}

// Parse the whole file now

	loc_numentries = 0;

	p = buf;
	line = 1;

	while (1)
	{
//		while (*buf == ' ' || *buf == 9)
//			buf++;
		SKIPBLANKS(p);

		if (*p == 0)
			goto _endoffile;

		if (*p == 10 || (*p == '/' && p[1] == '/'))
		{
			p++;
			goto _endofline;
		}

		for (i = 0; i < 3; i++)
		{
			n = 0;
			sign = 1;
			while (1)
			{
				switch (*p++)
				{
				case ' ': case 9:
					goto _next;

				case '-':
					if (n)
					{
						Con_Printf ("Error in loc file on line #%i\n", line);
						SKIPTOEOL(p);		
						goto _endofline;
					}
					sign = -1;
					break;

				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					n = n*10 + (p[-1] - '0');
					break;

				default:	// including eol or eof
					Con_Printf ("Error in loc file on line #%i\n", line);
					SKIPTOEOL(p);		
					goto _endofline;
				}
			}
_next:
			n *= sign;
			locdata[loc_numentries].coord[i] = n;

			SKIPBLANKS(p);
		}


// Save the location's name
//
		nameindex = 0;

		while (1)
		{
			switch (*p)
			{
			case 13:
				p++;
				break;

			case 10: case 0:
				locdata[loc_numentries].name[nameindex] = 0;
				loc_numentries++;

				if (loc_numentries >= MAX_LOC_ENTRIES)
					goto _endoffile;

				// leave the 0 or 10 in buffer, so it is parsed properly
				goto _endofline;

			default:
				if (nameindex < MAX_LOC_NAME-1)
					locdata[loc_numentries].name[nameindex++] = *p;
				p++;
			}
		}
_endofline:
		line++;
	}
_endoffile:

	Hunk_FreeToLowMark (mark);

	Con_Printf ("Loaded %s (%i locations)\n", locname, loc_numentries);
}

void CL_LoadLocFile_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("loadloc <filename> : load a loc file\n");
		return;
	}

	CL_LoadLocFile (Cmd_Argv(1), false);
}

extern int parsecountmod;
char *Cmd_Macro_Location_f (void)
{
	int		i;
	int		min_num;
	vec_t	min_dist;
	vec3_t	vec;
	vec3_t	org;
	
	if (!loc_numentries || (cls.state != ca_active))
		return "someplace";

	VectorCopy (cl.frames[parsecountmod].playerstate[cl.playernum].origin, org);
	for (i = 0; i < 3; i++)
		org[i] *= 8;

	min_num = 0;
	min_dist = 9999999;

	for (i = 0; i < loc_numentries; i++)
	{
//    	Con_DPrintf ("%f %f %f: %s\n", locdata[i].coord[0], locdata[i].coord[1], locdata[i].coord[2], locdata[i].name);
		VectorSubtract (org, locdata[i].coord, vec);
		if (Length(vec) < min_dist)
		{
			min_num = i;
			min_dist = Length(vec);
		}
	}

	return locdata[min_num].name;
}

/*
=============================================================================

							MESSAGE TRIGGERS

=============================================================================
*/

typedef struct msg_trigger_s {
	char name[32];
	char string[64];
	struct msg_trigger_s *next;
} msg_trigger_t;

static msg_trigger_t *msg_triggers;

msg_trigger_t *CL_FindTrigger (char *name)
{
	msg_trigger_t *t;

	for (t=msg_triggers; t; t=t->next)
		if (!strcmp(t->name, name))
			return t;

	return NULL;
}


void CL_MsgTrigger_f (void)
{
	int		c;
	char	*name;
	msg_trigger_t	*trig;

	c = Cmd_Argc();

	if (c > 3) {
		Con_Printf ("msg_trigger <trigger name> \"string\"\n");
		return;
	}

	if (c == 1) {
		if (!msg_triggers)
			Con_Printf ("no triggers defined\n");
		else
		for (trig=msg_triggers; trig; trig=trig->next)
			Con_Printf ("%s : \"%s\"\n", trig->name, trig->string);
		return;
	}

	name = Cmd_Argv(1);
	if (strlen(name) > 31) {
		Con_Printf ("trigger name too long\n");
		return;
	}

	if (c == 2) {
		trig = CL_FindTrigger (name);
		if (trig)
			Con_Printf ("%s: \"%s\"\n", trig->name, trig->string);
		else
			Con_Printf ("trigger \"%s\" not found\n", name);
		return;
	}

	if (c == 3) {
		if (strlen(Cmd_Argv(2)) > 63) {
			Con_Printf ("trigger string too long\n");
			return;
		}
		
		trig = CL_FindTrigger (name);

		if (!trig) {
			// allocate new trigger
			trig = Z_Malloc (sizeof(msg_trigger_t));
			trig->next = msg_triggers;
			msg_triggers = trig;
			strcpy (trig->name, name);
		}

		strcpy (trig->string, Cmd_Argv(2));
	}
}

char *trigger_commands[] = {
	"play",
	"playvol",
	"set",
	"echo",
	"say",
	"say_team",
	"alias",
	"msg_trigger",
	"inc"
};

#define NUM_TRIGGER_COMMANDS (sizeof(trigger_commands)/sizeof(trigger_commands[0]))

void CL_ExecuteTriggerString (char *text)
{
	char	buf[1024];
	char	*arg0;
	int		i;
	cmd_function_t	*cmd;

	Cmd_ExpandString (text, buf);
	Cmd_TokenizeString (buf);
			
	if (!Cmd_Argc())
		return;		// no tokens

// check cvars
	if (Cvar_Command())
		return;

// check commands
	arg0 = Cmd_Argv(0);

	for (i=0; i < NUM_TRIGGER_COMMANDS ; i++)
		if (!Q_strcasecmp(arg0, trigger_commands[i]))
		{
			cmd = Cmd_FindCommand (arg0);
			if (cmd) {
				if (!cmd->function)
					Cmd_ForwardToServer ();
				else
					cmd->function ();
				return;
			}
		}

	if (cl_warncmd.value || developer.value)
		Con_Printf ("Invalid trigger command: \"%s\"\n", arg0);
}


void CL_ExecuteTriggerBuf (char *text)
{
	char	line[1024];
	int		i, quotes;

	while (*text)
	{
		quotes = 0;
		for (i=0 ; text[i] ; i++)
		{
			if (text[i] == '"')
				quotes++;
			if ( !(quotes&1) &&  text[i] == ';' )
				break;	// don't break if inside a quoted string
			if (text[i] == '\n')
				break;
		}
		memcpy (line, text, i);
		line[i] = 0;
		CL_ExecuteTriggerString (line);
		if (!text[i])
			break;
		text += i + 1;
	}
}

void CL_SearchForMsgTriggers (char *s)
{
	msg_trigger_t	*t;
	char *string;
	extern char *Cmd_AliasString (char *);

	for (t=msg_triggers; t; t=t->next)
		if (t->string[0] && strstr(s, t->string)) {
			string = Cmd_AliasString (t->name);
			if (string)
				CL_ExecuteTriggerBuf (string);
			else
				Con_Printf ("trigger \"%s\" has no matching alias\n", t->name);
		}
}


int	CL_CountPlayers ()
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (cl.players[i].name[0] && !cl.players[i].spectator)
			count++;
	}

	return count;
}

char *CL_EnemyTeam ()
{
	int			i;
	char		myteam[MAX_INFO_STRING];
	static char	enemyteam[MAX_INFO_STRING];

	strcpy (myteam, Info_ValueForKey(cls.userinfo, "team"));

	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (cl.players[i].name[0] && !cl.players[i].spectator)
		{
			strcpy (enemyteam, Info_ValueForKey(cl.players[i].userinfo, "team"));
			if (strcmp(myteam, enemyteam) != 0)
				return enemyteam;
		}
	}
	return "";
}

char *CL_PlayerName ()
{
	static char	myname[MAX_INFO_STRING];

	strcpy (myname, Info_ValueForKey(cl.players[cl.playernum].userinfo, "name"));
	return myname;
}

char *CL_PlayerTeam ()
{
	static char	myteam[MAX_INFO_STRING];

	strcpy (myteam, Info_ValueForKey(cl.players[cl.playernum].userinfo, "team"));
	return myteam;
}

char *CL_EnemyName ()
{
	int			i;
	char		*myname;
	static char	enemyname[MAX_INFO_STRING];

	myname = CL_PlayerName ();

	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (cl.players[i].name[0] && !cl.players[i].spectator)
		{
			strcpy (enemyname, Info_ValueForKey(cl.players[i].userinfo, "name"));
			if (strcmp(enemyname, myname) != 0)
				return enemyname;
		}
	}
	return "";
}


char *CL_MapName ()
{
	static char mapname[MAX_INFO_STRING];

	// FIXME: take map name from modellist?
	strcpy (mapname, Info_ValueForKey (cl.serverinfo, "map"));

	return mapname;
}


void CL_InitTeamplay()
{
	Cvar_RegisterVariable (&_cmd_macros);
	Cvar_RegisterVariable (&cl_parsesay);
	Cvar_RegisterVariable (&cl_triggers);
	Cvar_RegisterVariable (&cl_nofake);
	Cvar_RegisterVariable (&cl_loadlocs);
	Cvar_RegisterVariable (&cl_rocket2grenade);

	Cmd_Macro_Init();
	Cmd_AddCommand ("loadloc", CL_LoadLocFile_f);
	Cmd_AddCommand ("msg_trigger", CL_MsgTrigger_f);
}
