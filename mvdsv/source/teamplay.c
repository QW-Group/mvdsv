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

	$Id: teamplay.c,v 1.1.1.2 2004/09/28 18:58:02 vvd0 Exp $
*/

#include "quakedef.h"
#include "version.h"

cvar_t	_cmd_macros = {"_cmd_macros", "0"};
cvar_t	cl_parsesay = {"cl_parsesay", "0"};
cvar_t	cl_triggers = {"cl_triggers", "0"};
cvar_t	cl_nofake = {"cl_nofake", "0"};
cvar_t	cl_loadlocs = {"cl_loadlocs", "0"};
cvar_t	cl_mapname = {"mapname", "", CVAR_ROM};

cvar_t	cl_rocket2grenade = {"cl_r2g", "0"};

cvar_t	cl_teamskin = {"teamskin", ""};
cvar_t	cl_enemyskin = {"enemyskin", ""};


//===========================================================================
//								TRIGGERS
//===========================================================================

void *Cmd_FindAlias(s);	 // hmm, it's NOT void in fact

// dest must point to a 1024-byte buffer
void Cmd_ExpandString (char *data, char *dest);

char *Macro_Location_f (void);	// defined later

#define MAX_LOC_NAME 32

// this structure is cleared after entering a new map
typedef struct tvars_s {
	int		health;
	int		items;
	float	respawntrigger_time;
	float	deathtrigger_time;
	float	f_version_reply_time;
	char	lastdeathloc[MAX_LOC_NAME];
	char	tookitem[32];
	char	last_tooktrigger[32];
} tvars_t;

tvars_t vars;

void TP_ExecTrigger (char *s)
{
	if (!cl_triggers.value || cls.demoplayback)
		return;

	if (Cmd_FindAlias(s)) {
		Cbuf_AddText (s);
		Cbuf_AddText ("\n");
	}
}

#define	IT_WEAPONS (2|4|8|16|32|64)
void TP_StatChanged (int stat, int value)
{
	int		i;

	if (stat == STAT_HEALTH)
	{
		if (value > 0) {
			if (vars.health <= 0 /*&& last_health != -999*/
				/* && Q_strcasecmp(Info_ValueForKey(cl.serverinfo, "status"),
				"standby") */)	// detect Kombat Teams status
			{
				extern cshift_t	cshift_empty;
				vars.respawntrigger_time = realtime;
				//if (cl.teamfortress)
					memset (&cshift_empty, 0, sizeof(cshift_empty));
				if (!cl.spectator)
					TP_ExecTrigger ("f_respawn");
			}
			vars.health = value;
			return;
		}
		if (vars.health > 0) {		// We just died
			vars.deathtrigger_time = realtime;
			strcpy (vars.lastdeathloc, Macro_Location_f());
			if (!cl.spectator)
				TP_ExecTrigger ("f_death");
		}
		vars.health = value;
	}
	else if (stat == STAT_ITEMS)
	{
		i = value &~ vars.items;
		if (i & IT_WEAPONS && (i & IT_WEAPONS != IT_WEAPONS)
		|| i & (IT_ARMOR1|IT_ARMOR2|IT_ARMOR3|IT_SUPERHEALTH)
		|| i & (IT_INVISIBILITY|IT_INVULNERABILITY|IT_SUIT|IT_QUAD))
		{
			// ...
			//TP_ExecTrigger ("f_took");
		}

		if (i & (IT_KEY1|IT_KEY2)) {
			if (cl.teamfortress)
				strcpy (vars.tookitem, "flag");
			else
				strcpy (vars.tookitem, "key");

			// TODO: only if tooktriggers are enabled
			strcpy (vars.last_tooktrigger, vars.tookitem);
			if (!cl.spectator)
				TP_ExecTrigger ("f_took");
		}

		vars.items = value;
	}
}


/*
==========================================================================
						        MACRO FUNCTIONS
==========================================================================
*/

#define MAX_MACRO_VALUE	256
static char	macro_buf[MAX_MACRO_VALUE];


char *Macro_Health_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_HEALTH]);
	return macro_buf;
}

char *Macro_Armor_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_ARMOR]);
	return macro_buf;
}

char *Macro_Shells_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_SHELLS]);
	return macro_buf;
}

char *Macro_Nails_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_NAILS]);
	return macro_buf;
}

char *Macro_Rockets_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_ROCKETS]);
	return macro_buf;
}

char *Macro_Cells_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_CELLS]);
	return macro_buf;
}

char *Macro_Ammo_f (void)
{
	sprintf(macro_buf, "%i", cl.stats[STAT_AMMO]);
	return macro_buf;
}

char *Macro_Weapon_f (void)
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

int	_Macro_BestWeapon (void)
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

char *Macro_BestWeapon_f (void)
{
	switch (_Macro_BestWeapon())
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

char *Macro_BestAmmo_f (void)
{
	switch (_Macro_BestWeapon())
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

// needed for %b parsing
char *Macro_BestWeaponAndAmmo_f (void)
{
	char buf[MAX_MACRO_VALUE];
	sprintf (buf, "%s:%s", Macro_BestWeapon_f(), Macro_BestAmmo_f());
	strcpy (macro_buf, buf);
	return macro_buf;
}

char *Macro_ArmorType_f (void)
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

char *Macro_Powerups_f (void)
{
	int effects;

	macro_buf[0] = 0;

	if (cl.stats[STAT_ITEMS] & IT_QUAD)
		strcpy(macro_buf, "quad");

	if (cl.stats[STAT_ITEMS] & IT_INVULNERABILITY) {
		if (macro_buf[0])
			strcat(macro_buf, "/");
		strcat(macro_buf, "pent");
	}

	if (cl.stats[STAT_ITEMS] & IT_INVISIBILITY) {
		if (macro_buf[0])
			strcat(macro_buf, "/");
		strcat(macro_buf, "ring");
	}

	effects = cl.frames[cl.parsecount&UPDATE_MASK].playerstate[cl.playernum].effects;
	if ( (effects & (EF_FLAG1|EF_FLAG2)) ||		// CTF
		(cl.teamfortress && cl.stats[STAT_ITEMS] & (IT_KEY1|IT_KEY2) // TF
		/*&& (effects & EF_DIMLIGHT))*/) )
	{
		if (macro_buf[0])
			strcat(macro_buf, "/");
		strcat(macro_buf, "flag");
	}

	return macro_buf;
}

char *Macro_Location2_f (void)
{
	if (vars.deathtrigger_time && realtime - vars.deathtrigger_time <= 5)
		return vars.lastdeathloc;
	return Macro_Location_f();
}

char *Macro_LastDeath_f (void)
{
	if (vars.deathtrigger_time)
		return vars.lastdeathloc;
	else
		return "someplace";
}

typedef struct
{
	char	*name;
	char	*(*func) (void);
} macro_command_t;

// Note: longer macro names like "armortype" must be defined
// _before_ the shorter ones like "armor" to be parsed properly
macro_command_t macro_commands[] =
{
	{"health", Macro_Health_f},
	{"armortype", Macro_ArmorType_f},
	{"armor", Macro_Armor_f},
	{"shells", Macro_Shells_f},
	{"nails", Macro_Nails_f},
	{"rockets", Macro_Rockets_f},
	{"cells", Macro_Cells_f},
	{"weapon", Macro_Weapon_f},
	{"ammo", Macro_Ammo_f},
	{"bestweapon", Macro_BestWeapon_f},
	{"bestammo", Macro_BestAmmo_f},
	{"powerups", Macro_Powerups_f},
	{"location", Macro_Location_f},
	{NULL, NULL}
};

#define MAX_MACRO_STRING 1024

char *TP_ParseMacroString (char *string)
{
	static char	buf[MAX_MACRO_STRING];
	char	*s;
	int		i;
	macro_command_t	*macro;
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
			// check %[P], etc
			if (*s == '%' && s[1]=='[' && s[2] && s[3]==']')
			{
				static char mbuf[MAX_MACRO_VALUE];
				switch (s[2]) {
				case 'a':
					macro_string = Macro_ArmorType_f();
					if (!macro_string[0])
						macro_string = "a";
					if (cl.stats[STAT_ARMOR] < 30)
						sprintf (mbuf, "\x10%s:%i\x11", macro_string, cl.stats[STAT_ARMOR]);
					else
						sprintf (mbuf, "%s:%i", macro_string, cl.stats[STAT_ARMOR]);
					macro_string = mbuf;
					break;

				case 'h':
					if (cl.stats[STAT_HEALTH] >= 50)
						sprintf (macro_buf, "%i", cl.stats[STAT_HEALTH]);
					else
						sprintf (macro_buf, "\x10%i\x11", cl.stats[STAT_HEALTH]);
					macro_string = macro_buf;
					break;

				case 'P':
					macro_string = Macro_Powerups_f();
					if (macro_string[0])
						sprintf (mbuf, "\x10%s\x11", macro_string);
					else
						mbuf[0] = 0;
					macro_string = mbuf;
					break;

				// todo: %[w], %[h], %[b]

				default:
					buf[i++] = *s++;
					continue;
				}
				if (i + strlen(macro_string) >= MAX_MACRO_STRING-1)
					Sys_Error("TP_ParseMacroString: macro string length > MAX_MACRO_STRING)");
				strcpy (&buf[i], macro_string);
				i += strlen(macro_string);
				s += 4;	// skip %[<char>]
				continue;
			}
			
			// check %a, etc
			if (*s == '%')
			{
				switch (s[1])
				{
				case 'a': macro_string = Macro_Armor_f(); break;
				case 'A': macro_string = Macro_ArmorType_f(); break;
				case 'b': macro_string = Macro_BestWeaponAndAmmo_f(); break;
				case 'c': macro_string = Macro_Cells_f(); break;
				case 'd': macro_string = Macro_LastDeath_f(); break;
				case 'h': macro_string = Macro_Health_f(); break;
				case 'i': macro_string = vars.tookitem; break;
				case 'I': macro_string = vars.last_tooktrigger; break;
				case 'l': macro_string = Macro_Location_f(); break;
				case 'L': macro_string = Macro_Location2_f(); break;
				case 'P':
				case 'p': macro_string = Macro_Powerups_f(); break;
				case 'r': macro_string = Macro_Rockets_f(); break;
				case 'w': macro_string = Macro_Weapon_f(); break;
				case 'W': macro_string = Macro_Ammo_f(); break;
				default: 
					buf[i++] = *s++;
					continue;
				}
				if (i + strlen(macro_string) >= MAX_MACRO_STRING-1)
					Sys_Error("TP_ParseMacroString: macro string length > MAX_MACRO_STRING)");
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
				case '\\': ch = 0x0D; break;
				case ':': ch = 0x0A; break;
				case '[': ch = 0x10; break;
				case ']': ch = 0x11; break;
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

			if (*s == '$') {
				buf[i++] = '$';
				s++;
				continue;
			}

			// macro search
			if (_cmd_macros.value) {
				macro = macro_commands;
				while (macro->name) {
					if (!strncmp(s, macro->name, strlen(macro->name)))
					{
						macro_string = macro->func();
						if (i + strlen(macro_string) >= MAX_MACRO_STRING-1) // !!! is this right?
							Sys_Error("TP_ParseMacroString: macro string length > MAX_MACRO_STRING)");
						strcpy (&buf[i], macro_string);
						i += strlen(macro_string);
						s += strlen(macro->name);
						goto _continue;
					}
					macro++;
				}
			}

			buf[i++] = '$';
		}

		buf[i++] = *s++;

		_continue: ;
	}
	buf[i] = 0;

	return	buf;
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

void TP_LoadLocFile (char *path, qboolean quiet)
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
		Con_Printf ("TP_LoadLocFile: path name > MAX_OSPATH\n");
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

	if (quiet)
		Con_Printf ("Loaded %s\n", locname);
	else
		Con_Printf ("Loaded %s (%i locations)\n", locname, loc_numentries);
}

void TP_LoadLocFile_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Con_Printf ("loadloc <filename> : load a loc file\n");
		return;
	}

	TP_LoadLocFile (Cmd_Argv(1), false);
}

extern int parsecountmod;
char *Macro_Location_f (void)
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


void TP_MsgTrigger_f (void)
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
	"stopsound",
	"set",
	"echo",
	"say",
	"say_team",
	"alias",
	"unalias",
	"msg_trigger",
	"inc",
	"bind",
	"unbind",
	"record",
	"easyrecord",
	"stop"
};

#define NUM_TRIGGER_COMMANDS (sizeof(trigger_commands)/sizeof(trigger_commands[0]))

void TP_ExecuteTriggerString (char *text)
{
	static char	buf[1024];
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


void TP_ExecuteTriggerBuf (char *text)
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
		TP_ExecuteTriggerString (line);
		if (!text[i])
			break;
		text += i + 1;
	}
}

void TP_SearchForMsgTriggers (char *s, int level)
{
	msg_trigger_t	*t;
	char *string;
	extern char *Cmd_AliasString (char *);

	if (cls.demoplayback)
		return;

	if (level != PRINT_HIGH)	// FIXME
		return;

	for (t=msg_triggers; t; t=t->next)
		if (t->string[0] && strstr(s, t->string)) {
			string = Cmd_AliasString (t->name);
			if (string)
				TP_ExecuteTriggerBuf (string);
			else
				Con_Printf ("trigger \"%s\" has no matching alias\n", t->name);
		}
}


void TP_CheckVersionRequest (char *s)
{
	char buf[11];
	int	i;

	if (cl.spectator)
		return;

	if (vars.f_version_reply_time
		&& realtime - vars.f_version_reply_time < 20)
		return;	// don't reply again if 20 seconds haven't passed

	while (1)
	{
		switch (*s++)
		{
		case 0:
		case '\n':
			return;
		case ':':
		case (char)':'|128:
			goto ok;
		}
	}
	return;

ok:
	for (i = 0; i < 11 && s[i]; i++)
		buf[i] = s[i] &~ 128;			// strip high bit

	if (!strncmp(buf, " f_version\n", 11) || !strncmp(buf, " z_version\n", 11))
	{
		Cbuf_AddText (va("say ZQuake version %s (Build %04d)\n", Z_VERSION, build_number()));
		vars.f_version_reply_time = realtime;
	}
}


int	TP_CountPlayers ()
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++) {
		if (cl.players[i].name[0] && !cl.players[i].spectator)
			count++;
	}

	return count;
}

char *TP_EnemyTeam ()
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

char *TP_PlayerName ()
{
	static char	myname[MAX_INFO_STRING];

	strcpy (myname, Info_ValueForKey(cl.players[cl.playernum].userinfo, "name"));
	return myname;
}

char *TP_PlayerTeam ()
{
	static char	myteam[MAX_INFO_STRING];

	strcpy (myteam, Info_ValueForKey(cl.players[cl.playernum].userinfo, "team"));
	return myteam;
}

char *TP_EnemyName ()
{
	int			i;
	char		*myname;
	static char	enemyname[MAX_INFO_STRING];

	myname = TP_PlayerName ();

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

char *TP_MapName ()
{
	return cl_mapname.string;
}

/*
=============================================================================
						TEAMCOLOR & ENEMYCOLOR
=============================================================================
*/

int		cl_teamtopcolor = -1;
int		cl_teambottomcolor;
int		cl_enemytopcolor = -1;
int		cl_enemybottomcolor;

void TP_TeamColor_f (void)
{
	int	top, bottom;
	int	i;

	if (Cmd_Argc() == 1)
	{
		if (cl_teamtopcolor < 0)
			Con_Printf ("\"teamcolor\" is \"off\"\n");
		else
			Con_Printf ("\"teamcolor\" is \"%i %i\"\n", 
				cl_teamtopcolor,
				cl_teambottomcolor);
		return;
	}

	if (!strcmp(Cmd_Argv(1), "off"))
	{
		cl_teamtopcolor = -1;
		for (i = 0; i < MAX_CLIENTS; i++)
			CL_NewTranslation(i);
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else {
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}
	
	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;
	
//	if (top != cl_teamtopcolor || bottom != cl_teambottomcolor)
	{
		cl_teamtopcolor = top;
		cl_teambottomcolor = bottom;

		for (i = 0; i < MAX_CLIENTS; i++)
			CL_NewTranslation(i);
	}
}

void TP_EnemyColor_f (void)
{
	int	top, bottom;
	int	i;

	if (Cmd_Argc() == 1)
	{
		if (cl_enemytopcolor < 0)
			Con_Printf ("\"enemycolor\" is \"off\"\n");
		else
			Con_Printf ("\"enemycolor\" is \"%i %i\"\n", 
				cl_enemytopcolor,
				cl_enemybottomcolor);
		return;
	}

	if (!strcmp(Cmd_Argv(1), "off"))
	{
		cl_enemytopcolor = -1;
		for (i = 0; i < MAX_CLIENTS; i++)
			CL_NewTranslation(i);
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else {
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}
	
	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

//	if (top != cl_enemytopcolor || bottom != cl_enemybottomcolor)
	{
		cl_enemytopcolor = top;
		cl_enemybottomcolor = bottom;

		for (i = 0; i < MAX_CLIENTS; i++)
			CL_NewTranslation(i);
	}
}

//===================================================================

void TP_NewMap ()
{
	static char last_map[MAX_QPATH] = {'\0'};
	char mapname[MAX_QPATH];

	memset (&vars, 0, sizeof(vars));

	COM_StripExtension (COM_SkipPath (cl.worldmodel->name), mapname);
	if (strcmp(mapname, last_map))
	{	// map name has changed
		loc_numentries = 0;	// clear loc file
		if (cl_loadlocs.value && !cls.demoplayback) {
			char locname[MAX_OSPATH];
			_snprintf (locname, MAX_OSPATH, "%s.loc", mapname);
			TP_LoadLocFile (locname, true);
		}
		strcpy (last_map, mapname);
		Cvar_SetROM (&cl_mapname, mapname);
	}
}

/*
======================
TP_CategorizeMessage

returns a combination of these values:
0 -- unknown (probably generated by the server)
1 -- normal
2 -- team message
4 -- spectator
Note that sometimes we can't be sure who really sent the message,
e.g. when there's a player "unnamed" in your team and "(unnamed)"
in the enemy team. The result will be 3 (1+2)

Never returns 2 if we are a spectator.
======================
*/
int TP_CategorizeMessage (char *s)
{
	int		i, msglen, len;
	int		flags;
	player_info_t	*player;

	flags = 0;
	msglen = strlen(s);
	if (!msglen)
		return 0;

	for (i=0, player=cl.players ; i < MAX_CLIENTS ; i++, player++)
	{
		len = strlen(player->name);
		if (!len)
			continue;
		// check messagemode1
		if (len+2 <= msglen && s[len] == ':' && s[len+1] == ' '	&&
			!strncmp(player->name, s, len))
		{
			if (player->spectator)
				flags |= 4;
			else
				flags |= 1;
		}
		// check messagemode2
		else if (s[0] == '(' && !cl.spectator && len+4 <= msglen &&
			!strncmp(s+len+1, "): ", 3) &&
			!strncmp(player->name, s+1, len))
		{
			// no team messages in teamplay 0, except for our own
			if (i == cl.playernum || ( atoi(Info_ValueForKey(cl.serverinfo, "teamplay"))
				&& !strcmp(cl.players[cl.playernum].team, player->team)) )
				flags |= 2;
		}
	}

	return flags;
}

void TP_Init ()
{
	Cvar_RegisterVariable (&_cmd_macros);
	Cvar_RegisterVariable (&cl_parsesay);
	Cvar_RegisterVariable (&cl_triggers);
	Cvar_RegisterVariable (&cl_nofake);
	Cvar_RegisterVariable (&cl_loadlocs);
	Cvar_RegisterVariable (&cl_rocket2grenade);
	Cvar_RegisterVariable (&cl_mapname);
	Cvar_RegisterVariable (&cl_teamskin);
	Cvar_RegisterVariable (&cl_enemyskin);

	Cmd_AddCommand ("loadloc", TP_LoadLocFile_f);
	Cmd_AddCommand ("msg_trigger", TP_MsgTrigger_f);
	Cmd_AddCommand ("teamcolor", TP_TeamColor_f);
	Cmd_AddCommand ("enemycolor", TP_EnemyColor_f);
}
