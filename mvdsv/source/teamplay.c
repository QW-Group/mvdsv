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

#include "quakedef.h"
#include "version.h"

cvar_t	cl_parsesay = {"cl_parsesay", "0"};
cvar_t	cl_triggers = {"cl_triggers", "0"};
cvar_t	cl_nofake = {"cl_nofake", "0"};
cvar_t	cl_loadlocs = {"cl_loadlocs", "0"};
cvar_t	cl_mapname = {"mapname", "", CVAR_ROM};

cvar_t	cl_rocket2grenade = {"cl_r2g", "0"};

cvar_t	cl_teamskin = {"teamskin", ""};
cvar_t	cl_enemyskin = {"enemyskin", ""};

cvar_t	tp_name_axe = {"tp_name_axe", "axe"};
cvar_t	tp_name_sg = {"tp_name_sg", "sg"};
cvar_t	tp_name_ssg = {"tp_name_ssg", "ssg"};
cvar_t	tp_name_ng = {"tp_name_ng", "ng"};
cvar_t	tp_name_sng = {"tp_name_sng", "sng"};
cvar_t	tp_name_gl = {"tp_name_gl", "gl"};
cvar_t	tp_name_rl = {"tp_name_rl", "rl"};
cvar_t	tp_name_lg = {"tp_name_lg", "lg"};
cvar_t	tp_name_ra = {"tp_name_ra", "ra"};
cvar_t	tp_name_ya = {"tp_name_ya", "ya"};
cvar_t	tp_name_ga = {"tp_name_ga", "ga"};
cvar_t	tp_name_quad = {"tp_name_quad", "quad"};
cvar_t	tp_name_pent = {"tp_name_pent", "pent"};
cvar_t	tp_name_ring = {"tp_name_ring", "ring"};
cvar_t	tp_name_suit = {"tp_name_suit", "suit"};
cvar_t	tp_name_shells = {"tp_name_shells", "shells"};
cvar_t	tp_name_nails = {"tp_name_nails", "nails"};
cvar_t	tp_name_rockets = {"tp_name_rockets", "rockets"};
cvar_t	tp_name_cells = {"tp_name_cells", "cells"};
cvar_t	tp_name_mh = {"tp_name_mh", "mh"};
cvar_t	tp_name_health = {"tp_name_health", "health"};
cvar_t	tp_name_backpack = {"tp_name_backpack", "pack"};
cvar_t	tp_name_flag = {"tp_name_flag", "flag"};


//===========================================================================
//								TRIGGERS
//===========================================================================

char *Macro_Location_f (void);
void TP_FindModelNumbers (void);

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

	if (Cmd_FindAlias(s))
	{
		char *astr, *p;
		qboolean quote = false;

		astr = Cmd_AliasString (s);
		for (p=astr ; *p ; p++)
		{
			if (*p == '"')
				quote = !quote;
			if (!quote && *p == ';')
			{
				// more than one command, add it to the command buffer
				Cbuf_AddText (astr);
				Cbuf_AddText ("\n");
				return;
			}
		}
		// a single line, so execute it right away
		Cmd_ExecuteString (astr);
		return;
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

char *Macro_WeaponNum_f (void)
{
	switch (cl.stats[STAT_ACTIVEWEAPON])
	{
	case IT_AXE: return "1";
	case IT_SHOTGUN: return "2";
	case IT_SUPER_SHOTGUN: return "3";
	case IT_NAILGUN: return "4";
	case IT_SUPER_NAILGUN: return "5";
	case IT_GRENADE_LAUNCHER: return "6";
	case IT_ROCKET_LAUNCHER: return "7";
	case IT_LIGHTNING: return "8";
	default:
		return "0";
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

char *Macro_Time_f (void)
{
	time_t		t;
	struct tm	*ptm;

	macro_buf[0] = 0;
	time(&t);
	ptm = localtime(&t);
	strftime(macro_buf, sizeof(macro_buf)-1, "%H:%M", ptm);
	return macro_buf;
}

char *Macro_Date_f (void)
{
	time_t		t;
	struct tm	*ptm;

	macro_buf[0] = 0;
	time(&t);
	ptm = localtime(&t);
	strftime(macro_buf, sizeof(macro_buf)-1, "%d.%m.%y", ptm);
	return macro_buf;
}

// returns the last item picked up
char *Macro_Item_f (void)
{
	strcpy (macro_buf, vars.tookitem);
	return macro_buf;
}

// returns the last item that triggered f_took
char *Macro_Took_f (void)
{
	strcpy (macro_buf, vars.last_tooktrigger);
	return macro_buf;
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
	{"weaponnum", Macro_WeaponNum_f},
	{"weapon", Macro_Weapon_f},
	{"ammo", Macro_Ammo_f},
	{"bestweapon", Macro_BestWeapon_f},
	{"bestammo", Macro_BestAmmo_f},
	{"powerups", Macro_Powerups_f},
	{"location", Macro_Location_f},
	{"time", Macro_Time_f},
	{"date", Macro_Date_f},
	{"item", Macro_Item_f},
	{"took", Macro_Took_f},
	{NULL, NULL}
};

#define MAX_MACRO_STRING 1024

/*
==============
TP_MacroString

returns NULL if no matching macro was found
==============
*/
int macro_length;	// length of macro name

char *TP_MacroString (char *s)
{
	static char	buf[MAX_MACRO_STRING];
	macro_command_t	*macro;

	macro = macro_commands;
	while (macro->name) {
		if (!Q_strncasecmp(s, macro->name, strlen(macro->name)))
		{
			macro_length = strlen(macro->name);
			return macro->func();
		}
		macro++;
	}

	macro_length = 0;
	return NULL;
}

/*
=============
TP_ParseChatString

Parses %a-like expressions
=============
*/
char *TP_ParseMacroString (char *string)
{
	static char	buf[MAX_MACRO_STRING];
	char	*s;
	int		i;
	char	*macro_string;
	char	ch;

	if (!cl_parsesay.value)
		return string;

	s = string;
	i = 0;

	while (*s && i < MAX_MACRO_STRING-1)
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
				
				// todo: %[w], %[b]
				
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
		
		// "fun chars"
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
			
			if (ch) {
				buf[i++] = ch;
				s += 2;
				continue;
			}
		}

		buf[i++] = *s++;
	}
	buf[i] = 0;

	return	buf;
}

/*
==============
TP_MacroList
==============
*/
void TP_MacroList_f (void)
{
	macro_command_t	*macro;
	int	i;

	for (macro=macro_commands,i=0 ; macro->name ; macro++,i++)
		Con_Printf ("%s\n", macro->name);

	Con_Printf ("------------\n%d macros\n", i);
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
	char	name[32];
	char	string[64];
	int		level;
	struct msg_trigger_s *next;
} msg_trigger_t;

static msg_trigger_t *msg_triggers;

msg_trigger_t *TP_FindTrigger (char *name)
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

	if (c > 5) {
		Con_Printf ("msg_trigger <trigger name> \"string\" [-l <level>]\n");
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
		trig = TP_FindTrigger (name);
		if (trig)
			Con_Printf ("%s: \"%s\"\n", trig->name, trig->string);
		else
			Con_Printf ("trigger \"%s\" not found\n", name);
		return;
	}

	if (c >= 3) {
		if (strlen(Cmd_Argv(2)) > 63) {
			Con_Printf ("trigger string too long\n");
			return;
		}
		
		trig = TP_FindTrigger (name);

		if (!trig) {
			// allocate new trigger
			trig = Z_Malloc (sizeof(msg_trigger_t));
			trig->next = msg_triggers;
			msg_triggers = trig;
			strcpy (trig->name, name);
			trig->level = PRINT_HIGH;
		}

		strcpy (trig->string, Cmd_Argv(2));
		if (c == 5 && !Q_strcasecmp (Cmd_Argv(3), "-l")) {
			trig->level = Q_atoi (Cmd_Argv(4));
			if ((unsigned)trig->level > PRINT_CHAT)
				trig->level = PRINT_HIGH;
		}
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
	"if"
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

	if (cls.demoplayback)
		return;

	for (t=msg_triggers; t; t=t->next)
		if (t->level == level && t->string[0] && strstr(s, t->string))
		{
			if (level == PRINT_CHAT && (
				strstr (s, "f_version") || strstr (s, "f_system") ||
				strstr (s, "f_speed") || strstr (s, "f_modified")))
				continue; 	// don't let llamas fake proxy replies

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
#ifdef RELEASE_VERSION
		Cbuf_AddText (va("say QWExtended Client version %s "
			QW_PLATFORM ":" QW_RENDERER "\n", QWE_VERSION));
#else
		Cbuf_AddText (va("say QWExtended Client version %s (Build %04d) "
			QW_PLATFORM ":" QW_RENDERER "\n", QWE_VERSION, build_number()));
#endif
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

void TP_FixTeamSets(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		Skin_Find(&cl.players[i]);
		CL_NewTranslation(i);
	}
}

void TP_NewMap ()
{
	static char last_map[MAX_QPATH] = {'\0'};
	char mapname[MAX_QPATH];

	memset (&vars, 0, sizeof(vars));
	TP_FindModelNumbers ();

	COM_StripExtension (COM_SkipPath (cl.worldmodel->name), mapname);
	if (strcmp(mapname, last_map))
	{	// map name has changed
		loc_numentries = 0;	// clear loc file
		if (cl_loadlocs.value && !cls.demoplayback ) {
			char locname[MAX_OSPATH];
			_snprintf (locname, MAX_OSPATH, "%s.loc", mapname);
			TP_LoadLocFile (locname, true);
		}
		strcpy (last_map, mapname);
		Cvar_SetROM (&cl_mapname, mapname);
	}

	TP_ExecTrigger ("f_newmap");
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

//===================================================================
// Pickup triggers
//

// symbolic names used in tp_took command
char *pknames[] = {"quad", "pent", "ring", "suit", "ra", "ya",	"ga",
"mh", "health", "lg", "rl", "gl", "sng", "ng", "ssg", "pack",
"cells", "rockets", "nails", "shells", "flag"};

enum {pk_quad, pk_pent, pk_ring, pk_suit, pk_ra, pk_ya, pk_ga,
pk_mh, pk_health, pk_lg, pk_rl, pk_gl, pk_sng, pk_ng, pk_ssg, pk_pack,
pk_cells, pk_rockets, pk_nails, pk_shells, pk_flag, MAX_PKFLAGS};

#define default_pkflags ((1<<pk_quad)|(1<<pk_pent)|(1<<pk_ring)| \
		(1<<pk_ra)|(1<<pk_ya)|(1<<pk_lg)|(1<<pk_rl)|(1<<pk_mh)|(1<<pk_flag))

int pkflags = default_pkflags;


void TP_TookTrigger_f (void)
{
	int		i, j, c;
	char	*p;
	char	str[255] = "";
	qboolean	removeflag = false;
	int		flag;
	
	c = Cmd_Argc ();
	if (c == 1)
	{
		if (!pkflags)
			strcpy (str, "nothing");
		for (i=0 ; i<MAX_PKFLAGS ; i++)
			if (pkflags & (1<<i))
			{
				if (*str)
					strcat (str, " ");
				strcat (str, pknames[i]);
			}
		Con_Printf ("%s\n", str);
		return;
	}

	if (*Cmd_Argv(1) != '+' && *Cmd_Argv(1) != '-')
		pkflags = 0;

	for (i=1 ; i<c ; i++)
	{
		p = Cmd_Argv (i);
		if (*p == '+') {
			removeflag = false;
			p++;
		} else if (*p == '-') {
			removeflag = true;
			p++;
		}

		flag = 0;
		for (j=0 ; j<MAX_PKFLAGS ; j++) {
			if (!Q_strncasecmp (p, pknames[j], 3)) {
				flag = 1<<j;
				break;
			}
		}

		if (!flag) {
			if (!Q_strcasecmp (p, "armor"))
				flag = (1<<pk_ra)|(1<<pk_ya)|(1<<pk_ga);
			else if (!Q_strcasecmp (p, "weapons"))
				flag = (1<<pk_lg)|(1<<pk_rl)|(1<<pk_gl)|(1<<pk_sng)|
						(1<<pk_ng)|(1<<pk_ssg);
			else if (!Q_strcasecmp (p, "powerups"))
				flag = (1<<pk_quad)|(1<<pk_pent)|(1<<pk_ring);
			else if (!Q_strcasecmp (p, "ammo"))
				flag = (1<<pk_cells)|(1<<pk_rockets)|(1<<pk_nails)|(1<<pk_shells);
			else if (!Q_strcasecmp (p, "default"))
				flag = default_pkflags;
			else if (!Q_strcasecmp (p, "all"))
				flag = (1<<MAX_PKFLAGS)-1;
		}

		if (removeflag)
			pkflags &= ~flag;
		else
			pkflags |= flag;
	}
}


/*
// FIXME: maybe use sound indexes so we don't have to make strcmp's
// every time?

#define S_LOCK4		1	// weapons/lock4.wav
#define S_PKUP		2	// weapons/pkup.wav
#define S_HEALTH25	3	// items/health1.wav
#define S_HEALTH15	4	// items/r_item1.wav
#define S_MHEALTH	5	// items/r_item2.wav
#define S_DAMAGE	6	// items/damage.wav
#define S_EYES		7	// items/inv1.wav
#define S_PENT		8	// items/protect.wav
#define S_ARMOR		9	// items/armor1.wav

static char *tp_soundnames[] =
{
	"weapons/lock4.wav",
	"weapons/pkup.wav",
	"items/health1.wav",
	"items/r_item1.wav",
	"items/r_item2.wav",
	"items/damage.wav",
	"items/inv1.wav",
	"items/protect.wav"
	"items/armor1.wav"
};

#define TP_NUMSOUNDS (sizeof(tp_soundnames)/sizeof(tp_soundnames[0]))

int	sound_numbers[MAX_SOUNDS];

void TP_FindSoundNumbers (void)
{
	int		i, j;
	char	*s;
	for (i=0 ; i<MAX_SOUNDS ; i++)
	{
		s = &cl.sound_name[i];
		for (j=0 ; j<TP_NUMSOUNDS ; j++)
			...
	}
}
*/

// model numbers
int tp_armorindex;	// armor
int tp_packindex;	// backpack
int tp_ssgindex, tp_ngindex, tp_sngindex, tp_glindex,
	tp_rlindex, tp_lgindex;		// weapons
int tp_shells1index, tp_shells2index,
	tp_nails1index, tp_nails2index,
	tp_rockets1index, tp_rockets2index,
	tp_cells1index, tp_cells2index;	// ammo

void TP_FindModelNumbers (void)
{
	int		i;
	char	*s;
	
	tp_ssgindex = tp_ngindex = tp_sngindex =
	tp_glindex = tp_rlindex = tp_lgindex = tp_packindex =
	tp_armorindex = tp_shells1index = tp_shells2index =
	tp_nails1index = tp_nails2index = tp_rockets1index =
	tp_rockets2index = tp_cells1index = tp_cells2index = -1;

	// model 0 is world
	for (i=1 ; i<MAX_MODELS ; i++)
	{
		s = cl.model_name[i];
		if (!strcmp(s, "progs/g_shot.mdl"))
			tp_ssgindex = i;
		else if (!strcmp(s, "progs/g_nail.mdl"))
			tp_ngindex = i;
		else if (!strcmp(s, "progs/g_nail2.mdl"))
			tp_sngindex = i;
		else if (!strcmp(s, "progs/g_rock.mdl"))
			tp_glindex = i;
		else if (!strcmp(s, "progs/g_rock2.mdl"))
			tp_rlindex = i;
		else if (!strcmp(s, "progs/g_light.mdl"))
			tp_lgindex = i;
		else if (!strcmp(s, "progs/armor.mdl"))
			tp_armorindex = i;
		else if (!strcmp(s, "progs/backpack.mdl"))
			tp_packindex = i;
		else if (!strcmp(s, "maps/b_shell0.bsp"))
			tp_shells1index = i;
		else if (!strcmp(s, "maps/b_shell1.bsp"))
			tp_shells2index = i;
		else if (!strcmp(s, "maps/b_nail0.bsp"))
			tp_nails1index = i;
		else if (!strcmp(s, "maps/b_nail1.bsp"))
			tp_nails2index = i;
		else if (!strcmp(s, "maps/b_rock0.bsp"))
			tp_rockets1index = i;
		else if (!strcmp(s, "maps/b_rock1.bsp"))
			tp_rockets2index = i;
		else if (!strcmp(s, "maps/b_batt0.bsp"))
			tp_cells1index = i;
		else if (!strcmp(s, "maps/b_batt1.bsp"))
			tp_cells2index = i;
	}
}


static int FindNearestItem (int type)
{
	frame_t		*frame;
	packet_entities_t	*pak;
	entity_state_t		*ent;
	int	i, bestidx, bestdist, bestskin;
	vec3_t	org, v;
	extern	int oldparsecountmod;

	VectorCopy (cl.frames[(cls.netchan.incoming_sequence)&UPDATE_MASK]
		.playerstate[cl.playernum].origin, org);

	// look in previous frame 
	frame = &cl.frames[oldparsecountmod&UPDATE_MASK];
	pak = &frame->packet_entities;
	bestdist = 250;
	bestidx = 0;
	for (i=0,ent=pak->entities ; i<pak->num_entities ; i++,ent++)
	{
		int j, dist;
		
		j = ent->modelindex;
		switch (type)
		{
		case 0:	// weapon
			if (j != tp_ssgindex && j != tp_ngindex && j != tp_sngindex
				&& j != tp_glindex && j != tp_rlindex && j != tp_lgindex)
				continue;
			break;
		case 1:	// armor
			if (j != tp_armorindex)
				continue;
			break;
		case 2:	// ammo or backpack
			if (j != tp_packindex && j != tp_shells1index && j != tp_shells2index
				&& j != tp_nails1index && j != tp_nails2index 
				&& j != tp_rockets1index && j != tp_rockets2index
				&& j != tp_cells1index && j != tp_cells2index)
				continue;
		}
		VectorSubtract (ent->origin, org, v);
		if ((dist = Length(v)) > bestdist)
			continue;
		bestdist = dist;
		bestidx = j;
		bestskin = ent->skinnum;
	}

	if (type == 1 && bestidx)	// armor
		return -(bestskin + 1);	// -1=green, -2=yellow, -3=red

	return bestidx;
}

static int CountTeammates ()
{
	int	i, count;
	player_info_t	*player;
	char	*myteam;

	count = 0;
	myteam = cl.players[cl.playernum].team;
	for (i=0, player=cl.players; i < MAX_CLIENTS ; i++, player++) {
		if (player->name[0] && !player->spectator && (i != cl.playernum)
									&& !strcmp(player->team, myteam))
			count++;
	}

	return count;
}

static void ExecTookTrigger (char *s, int flag)
{
	strcpy (vars.tookitem, s);
	if (pkflags & (1<<flag))
	{
		if (CountTeammates () > 0)
		{
			strcpy (vars.last_tooktrigger, s);
			TP_ExecTrigger ("f_took");
		}
	}
}

void TP_CheckPickupSound (char *s)
{
	int		idx;

	if (cl.spectator || !atoi(Info_ValueForKey(cl.serverinfo, "teamplay")))
		return;

	if (!strcmp(s, "items/damage.wav"))
		ExecTookTrigger (tp_name_quad.string, pk_quad);
	else if (!strcmp(s, "items/protect.wav"))
		ExecTookTrigger (tp_name_pent.string, pk_pent);
	else if (!strcmp(s, "items/inv1.wav"))
		ExecTookTrigger (tp_name_ring.string, pk_ring);
	else if (!strcmp(s, "items/suit.wav"))
		ExecTookTrigger (tp_name_suit.string, pk_suit);
	else if (!strcmp(s, "items/health1.wav") ||
			 !strcmp(s, "items/r_item1.wav"))
		ExecTookTrigger (tp_name_health.string, pk_health);
	else if (!strcmp(s, "items/r_item2.wav"))
		ExecTookTrigger (tp_name_mh.string, pk_mh);
	else
		goto more;
	return;

more:
	if (!cl.validsequence)
		return;

	// weapons
	if (!strcmp(s, "weapons/pkup.wav"))
	{
		int	deathmatch;

		deathmatch = atoi(Info_ValueForKey(cl.serverinfo, "deathmatch"));
		if (deathmatch == 2 || deathmatch == 3)
			return;
		idx = FindNearestItem(0);
		if (idx == tp_ssgindex)
			ExecTookTrigger (tp_name_ssg.string, pk_ssg);
		else if (idx == tp_ngindex)
			ExecTookTrigger (tp_name_ng.string, pk_ng);
		else if (idx == tp_sngindex)
			ExecTookTrigger (tp_name_sng.string, pk_sng);
		else if (idx == tp_glindex)
			ExecTookTrigger (tp_name_gl.string, pk_gl);
		else if (idx == tp_rlindex)
			ExecTookTrigger (tp_name_rl.string, pk_rl);
		else if (idx == tp_lgindex)
			ExecTookTrigger (tp_name_lg.string, pk_lg);
		return;
	}

	// armor
	if (!strcmp(s, "items/armor1.wav"))
	{
		idx = FindNearestItem (1);

		switch (idx) {
			case -1: ExecTookTrigger (tp_name_ga.string, pk_ga); break;
			case -2: ExecTookTrigger (tp_name_ya.string, pk_ya); break;
			case -3: ExecTookTrigger (tp_name_ra.string, pk_ra); break;
		}
		return;
	}

	// backpack or ammo
	if (!strcmp (s, "weapons/lock4.wav"))
	{
		idx = FindNearestItem (2);
		if (idx == tp_packindex)
			ExecTookTrigger (tp_name_backpack.string, pk_pack);
		else if (idx == tp_shells1index || idx == tp_shells2index)
			ExecTookTrigger (tp_name_shells.string, pk_shells);
		else if (idx == tp_nails1index || idx == tp_nails2index)
			ExecTookTrigger (tp_name_nails.string, pk_nails);
		else if (idx == tp_rockets1index || idx == tp_rockets2index)
			ExecTookTrigger (tp_name_rockets.string, pk_rockets);
		else if (idx == tp_cells1index || idx == tp_cells2index)
			ExecTookTrigger (tp_name_cells.string, pk_cells);
		return;
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

		if (i & (IT_KEY1|IT_KEY2)) {
			if (cl.teamfortress && !cl.spectator)
				ExecTookTrigger (tp_name_flag.string, pk_flag);
		}

		vars.items = value;
	}
}


void TP_Init ()
{
	Cvar_RegisterVariable (&cl_parsesay);
	Cvar_RegisterVariable (&cl_triggers);
	Cvar_RegisterVariable (&cl_nofake);
	Cvar_RegisterVariable (&cl_loadlocs);
	Cvar_RegisterVariable (&cl_rocket2grenade);
	Cvar_RegisterVariable (&cl_mapname);
	Cvar_RegisterVariable (&cl_teamskin);
	Cvar_RegisterVariable (&cl_enemyskin);
	Cvar_RegisterVariable (&tp_name_axe);
	Cvar_RegisterVariable (&tp_name_sg);
	Cvar_RegisterVariable (&tp_name_ssg);
	Cvar_RegisterVariable (&tp_name_ng);
	Cvar_RegisterVariable (&tp_name_sng);
	Cvar_RegisterVariable (&tp_name_gl);
	Cvar_RegisterVariable (&tp_name_rl);
	Cvar_RegisterVariable (&tp_name_lg);
	Cvar_RegisterVariable (&tp_name_ra);
	Cvar_RegisterVariable (&tp_name_ya);
	Cvar_RegisterVariable (&tp_name_ga);
	Cvar_RegisterVariable (&tp_name_quad);
	Cvar_RegisterVariable (&tp_name_pent);
	Cvar_RegisterVariable (&tp_name_ring);
	Cvar_RegisterVariable (&tp_name_suit);
	Cvar_RegisterVariable (&tp_name_shells);
	Cvar_RegisterVariable (&tp_name_nails);
	Cvar_RegisterVariable (&tp_name_rockets);
	Cvar_RegisterVariable (&tp_name_cells);
	Cvar_RegisterVariable (&tp_name_mh);
	Cvar_RegisterVariable (&tp_name_health);
	Cvar_RegisterVariable (&tp_name_backpack);
	Cvar_RegisterVariable (&tp_name_flag);

	Cmd_AddCommand ("macrolist", TP_MacroList_f);
	Cmd_AddCommand ("loadloc", TP_LoadLocFile_f);
	Cmd_AddCommand ("msg_trigger", TP_MsgTrigger_f);
	Cmd_AddCommand ("teamcolor", TP_TeamColor_f);
	Cmd_AddCommand ("enemycolor", TP_EnemyColor_f);
	Cmd_AddCommand ("tp_took", TP_TookTrigger_f);
}
