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

#include "quakedef.h"
#include "winquake.h"
#include "teamplay.h"
#include "version.h"
#include "sha1.h"

void SCR_RSShot_f (void);
void CL_ProcessServerInfo (void);
void SV_Serverinfo_f (void);
void Key_WriteBindings (FILE *f);
void S_StopAllSounds (qboolean clear);

/*
===================
Cmd_ForwardToServer

adds the current command line as a clc_stringcmd to the client message.
things like kill, say, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void Cmd_ForwardToServer (void)
{
	char	*s;

	if (cls.state == ca_disconnected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	// lowercase command
	for (s=Cmd_Argv(0) ; *s ; s++)
		*s = (char)tolower(*s);
	SZ_Print (&cls.netchan.message, Cmd_Argv(0));
	if (Cmd_Argc() > 1)
	{
		SZ_Print (&cls.netchan.message, " ");
		SZ_Print (&cls.netchan.message, Cmd_Args());
	}
}

// don't forward the first argument
void CL_ForwardToServer_f (void)
{
// Added by VVD {
	char		*client_string, *server_string, client_time_str[9];
	int		i, client_string_len, server_string_len;
	extern cvar_t	cl_crypt_rcon;
	time_t		client_time;
// Added by VVD }

	if (cls.state == ca_disconnected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	if (strcasecmp(Cmd_Argv(1), "snap") == 0) {
		SCR_RSShot_f ();
		return;
	}

//bliP ->
	if (strcasecmp(Cmd_Argv(1), "fileul") == 0) {
		CL_StartFileUpload ();
		return;
	}
//<-
	
	if (cls.demoplayback)
		return;		// not really connected

	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
// Added by VVD {
		if (cl_crypt_rcon.value && strcasecmp(Cmd_Argv(1), "techlogin") == 0)
		{
			time(&client_time);
			for (i = 0; i < sizeof(client_time); ++i)
			{
				snprintf(client_time_str + i * 2, 8 * 2 + 1 - i * 2, "%02X",
					(client_time >> (i * 8)) & 0xFF);
//				Con_Printf("client_time_str = %s\n", client_time_str);
			}
			client_string_len = Cmd_Argc() + 8;

			for (i = 1; i < Cmd_Argc(); ++i)
				client_string_len += strlen(Cmd_Argv(i));
			server_string_len = client_string_len - strlen(Cmd_Argv(2)) + DIGEST_SIZE * 2 + 8;
			client_string = Q_Malloc(client_string_len);
			server_string = Q_Malloc(server_string_len);
			*client_string = *server_string = 0;
			strlcat(client_string, Cmd_Argv(1), client_string_len);
			strlcat(client_string, " ", client_string_len);
			strlcat(client_string, Cmd_Argv(2), client_string_len);
			strlcat(client_string, client_time_str, client_string_len);
			strlcat(client_string, " ", client_string_len);
			for (i = 3; i < Cmd_Argc(); ++i)
			{
				strlcat(client_string, Cmd_Argv(i), client_string_len);
				strlcat(client_string, " ", client_string_len);
			}
			strlcpy(server_string, Cmd_Argv(1), server_string_len);
			strlcat(server_string, " ", server_string_len);
			strlcat(server_string, SHA1(client_string, client_string_len - 1), server_string_len);
			strlcat(server_string, client_time_str, server_string_len);
			strlcat(server_string, " ", server_string_len);
//		Con_Printf("client_string = %s\nserver_string = %s\n", client_string, server_string);
//		Con_Printf("server_string_len = %d, strlen(server_string) = %d\n", server_string_len, strlen(server_string));
//		Con_Printf("client_string_len = %d, strlen(client_string) = %d\n", client_string_len, strlen(client_string));
			Q_Free(client_string);
			for (i = 3; i < Cmd_Argc(); ++i)
			{
				strlcat(server_string, Cmd_Argv(i), server_string_len);
				strlcat(server_string, " ", server_string_len);
			}
			SZ_Print (&cls.netchan.message, server_string);
			Q_Free(server_string);
		}
		else
// Added by VVD }
			SZ_Print (&cls.netchan.message, Cmd_Args());
	}
}

/*
===============
CL_Say_f

Handles both say and say_team
===============
*/
void CL_Say_f (void)
{
	char	*s;

	if (cls.state == ca_disconnected)
	{
		Con_Printf ("Can't \"%s\", not connected\n", Cmd_Argv(0));
		return;
	}

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	// lowercase command
	for (s=Cmd_Argv(0) ; *s ; s++)
		*s = (char)tolower(*s);
	SZ_Print (&cls.netchan.message, Cmd_Argv(0));
	if (Cmd_Argc() > 1)
	{
		SZ_Print (&cls.netchan.message, " ");

		s = TP_ParseMacroString(Cmd_Args());
		if (*s && *s < 32 && *s != 10)
		{
			SZ_Print (&cls.netchan.message, "\"");
			SZ_Print (&cls.netchan.message, s);
			SZ_Print (&cls.netchan.message, "\"");
		}
		else
			SZ_Print (&cls.netchan.message, s);
	}
}


/*
=====================
CL_Pause_f
=====================
*/
void CL_Pause_f (void)
{
	if (cls.demoplayback)
		cl.paused ^= 2;
	else
		Cmd_ForwardToServer();
}


/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
void CL_Packet_f (void)
{
	char	send[2048];
	int		i, l;
	char	*in, *out;
	netadr_t	adr;

	if (Cmd_Argc() != 3)
	{
		Con_Printf ("packet <destination> <contents>\n");
		return;
	}

	if (!NET_StringToAdr (Cmd_Argv(1), &adr))
	{
		Con_Printf ("Bad address\n");
		return;
	}

	if (adr.port == 0)
		adr.port = BigShort (27500);

	in = Cmd_Argv(2);
	out = send+4;
	send[0] = send[1] = send[2] = send[3] = 0xff;

	l = strlen (in);
	for (i=0 ; i<l ; i++)
	{
		if (in[i] == '\\' && in[i+1] == 'n')
		{
			*out++ = '\n';
			i++;
		}
		else
			*out++ = in[i];
	}
	*out = 0;

	NET_SendPacket (net_clientsocket, out-send, send, adr);
}


/*
=====================
CL_Rcon_f

Send the rest of the command line over as
an unconnected command.
=====================
*/
void CL_Rcon_f (void)
{
	char	message[1024], *sha1, client_time_str[9];
	int		i;
	netadr_t	to;
	extern cvar_t	rcon_password, rcon_address, cl_crypt_rcon;
	time_t		client_time;

	message[0] = 255;
	message[1] = 255;
	message[2] = 255;
	message[3] = 255;
	message[4] = 0;
	strlcat (message, "rcon ", sizeof(message));
	if (rcon_password.string[0])
		strlcat (message, rcon_password.string, sizeof(message));

// Added by VVD {
	if (cl_crypt_rcon.value)
	{
		time(&client_time);
		for (i = 0; i < sizeof(client_time); ++i)
		{
			snprintf(client_time_str + i * 2, 8 * 2 + 1 - i * 2, "%02X",
				(client_time >> (i * 8)) & 0xFF);
			Con_Printf("client_time_str = %s\n", client_time_str);
		}
		strlcat (message, client_time_str, sizeof(message));
		strlcat (message, " ", sizeof(message));
		for (i = 1; i < Cmd_Argc(); i++)
		{
			strlcat (message, Cmd_Argv(i), sizeof(message));
			strlcat (message, " ", sizeof(message));
		}
		sha1 = SHA1(message + 4, strlen(message) - 4);
		message[4] = 0;
		strlcat (message, "rcon ", sizeof(message));
		strlcat (message, sha1, sizeof(message));
		strlcat (message, client_time_str, sizeof(message));
		strlcat (message, " ", sizeof(message));
		for (i = 1; i < Cmd_Argc(); i++)
		{
			strlcat (message, Cmd_Argv(i), sizeof(message));
			strlcat (message, " ", sizeof(message));
		}
	}
	else
	{
		strlcat (message, " ", sizeof(message));
		for (i=1 ; i<Cmd_Argc() ; i++)
		{
			strlcat (message, Cmd_Argv(i), sizeof(message));
			strlcat (message, " ", sizeof(message));
		}
	}
// } Added by VVD
	if (cls.state >= ca_connected)
		to = cls.netchan.remote_address;
	else
	{
		if (!strlen(rcon_address.string))
		{
			Con_Printf ("You must either be connected,\n"
						"or set the 'rcon_address' cvar\n"
						"to issue rcon commands\n");
			return;
		}
		NET_StringToAdr (rcon_address.string, &to);
		if (to.port == 0)
			to.port = BigShort (27500);
	}
	NET_SendPacket (net_clientsocket, strlen(message)+1, message, to);
}


/*
=====================
CL_Download_f
=====================
*/
void CL_Download_f (void)
{
	char *p, *q;

	if (cls.state == ca_disconnected)
	{
		Con_Printf ("Must be connected.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: download <datafile>\n");
		return;
	}

	snprintf (cls.downloadname, MAX_OSPATH, "%s/%s", com_gamedir, Cmd_Argv(1));

	p = cls.downloadname;
	for (;;) {
		if ((q = strchr(p, '/')) != NULL) {
			*q = 0;
			Sys_mkdir(cls.downloadname);
			*q = '/';
			p = q + 1;
		} else
			break;
	}

	if (cls.download)
	{
		fclose(cls.download);
		cls.download = NULL;
	}
	

	strlcpy(cls.downloadtempname, cls.downloadname, MAX_OSPATH);
	cls.download = fopen (cls.downloadname, "wb");
	cls.downloadtype = dl_single;

	MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
	SZ_Print (&cls.netchan.message, va("download %s\n",Cmd_Argv(1)));
}


/*
====================
CL_User_f

user <name or userid>

Dump userdata / masterdata for a user
====================
*/
void CL_User_f (void)
{
	int		uid;
	int		i;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("Usage: user <username / userid>\n");
		return;
	}

	uid = atoi(Cmd_Argv(1));

	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		if (!cl.players[i].name[0])
			continue;
		if (cl.players[i].userid == uid
		|| !strcmp(cl.players[i].name, Cmd_Argv(1)) )
		{
			Info_Print (cl.players[i].userinfo);
			return;
		}
	}
	Con_Printf ("User not in server.\n");
}

/*
====================
CL_Users_f

Dump userids for all current players
====================
*/
void CL_Users_f (void)
{
	int		i;
	int		c;

	c = 0;
	Con_Printf ("userid frags name\n");
	Con_Printf ("------ ----- ----\n");
	for (i=0 ; i<MAX_CLIENTS ; i++)
	{
		if (cl.players[i].name[0])
		{
			Con_Printf ("%6i %4i %s\n", cl.players[i].userid, cl.players[i].frags, cl.players[i].name);
			c++;
		}
	}

	Con_Printf ("%i total users\n", c);
}

/*
====================
CL_Color_f

Just for quake compatability
====================
*/
void CL_Color_f (void)
{
	extern cvar_t	topcolor, bottomcolor;
	int		top, bottom;
	char	num[16];

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%s %s\"\n",
			Info_ValueForKey (cls.userinfo, "topcolor"),
			Info_ValueForKey (cls.userinfo, "bottomcolor") );
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}
	
	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;
	
	snprintf (num, sizeof(num), "%i", top);
	Cvar_Set (&topcolor, num);
	snprintf (num, sizeof(num), "%i", bottom);
	Cvar_Set (&bottomcolor, num);
}


/*
==================
CL_FullInfo_f

usage:
fullinfo \name\unnamed\topcolor\0\bottomcolor\1, etc
==================
Casey was here :)
*/
void CL_FullInfo_f (void)
{
	char	key[512];
	char	value[512];
	char	*o;
	char	*s;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("fullinfo <complete info string>\n");
		return;
	}

	s = Cmd_Argv(1);
	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (!*s)
		{
			Con_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;

		if (!stricmp(key, pmodel_name) || !stricmp(key, emodel_name))
			continue;

		Info_SetValueForKey (cls.userinfo, key, value, MAX_INFO_STRING);
	}
}

/*
==================
CL_SetInfo_f

Allow clients to change userinfo
==================
*/
void CL_SetInfo_f (void)
{
	if (Cmd_Argc() == 1)
	{
		Info_Print (cls.userinfo);
		return;
	}
	if (Cmd_Argc() != 3)
	{
		Con_Printf ("usage: setinfo [ <key> <value> ]\n");
		return;
	}
	if (!stricmp(Cmd_Argv(1), pmodel_name) || !strcmp(Cmd_Argv(1), emodel_name))
		return;

	Info_SetValueForKey (cls.userinfo, Cmd_Argv(1), Cmd_Argv(2), MAX_INFO_STRING);
	if (cls.state >= ca_connected)
		Cmd_ForwardToServer ();
}


/*
==================
CL_Quit_f
==================
*/
void CL_Quit_f (void)
{
	M_Menu_Quit_f ();
	return;
#if 0
	CL_Disconnect ();
	Sys_Quit ();
#endif
}


/*
===============
CL_Windows_f
===============
*/
#ifdef _WINDOWS
void CL_Windows_f (void)
{
	SendMessage(mainwindow, WM_SYSKEYUP, VK_TAB, 1 | (0x0F << 16) | (1<<29));
}
#endif


/*
===============
CL_Serverinfo_f
===============
*/
void CL_Serverinfo_f (void)
{

// Tonik: no need to request serverinfo from server, because we
// already have it cached in cl.serverinfo
// this also lets us get serverinfo when playing a demo
	//Cmd_ForwardToServer();
	if (cls.state >= ca_onserver && cl.serverinfo)
		Info_Print (cl.serverinfo);
	else
		// so that it says we are not connected :)
		Cmd_ForwardToServer();	
}


/*
===============
CL_WriteConfig_f

Writes key bindings and archived cvars to a custom config file
===============
*/
void CL_WriteConfig_f (void)
{
	FILE	*f;
	char	name[MAX_QPATH];

	if (Cmd_Argc() != 2) {
		Con_Printf ("usage: writeconfig <filename>\n");
		return;
	}

	strlcpy (name, Cmd_Argv(1), sizeof(name));
	COM_ForceExtension (name, ".cfg");

	Con_Printf ("Writing %s\n", name);

	f = fopen (va("%s/%s", com_gamedir, name), "w");
	if (!f) {
		Con_Printf ("Couldn't write %s.\n", name);
		return;
	}
	
	Key_WriteBindings (f);
	Cvar_WriteVariables (f);

	fclose (f);
}

void CL_DemoJump_f (void)
{
	if (Cmd_Argc() != 2) {
		Sys_Printf("usage: demojump <sec>\n");
		return;
	}

	if (atoi(Cmd_Argv(1)) <= 0)
		return;

	realtime += atoi(Cmd_Argv(1));
}


void Host_Savegame_f(void);
void Host_Loadgame_f(void);

double ping_time;
void Cmd_Ping_f (void)
{
	char	data[6];
	netadr_t	adr;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("ping <destination>\n");
		return;
	}

	if (!NET_StringToAdr (Cmd_Argv(1), &adr))
	{
		Con_Printf ("Bad address\n");
		return;
	}

	if (adr.port == 0)
		adr.port = BigShort (27500);


	data[0] = 0xff;
	data[1] = 0xff;
	data[2] = 0xff;
	data[3] = 0xff;
	data[4] = A2A_PING;
	data[5] = 0;
		
	NET_SendPacket (net_clientsocket, 6, &data, adr);
	ping_time = realtime;
}

void CL_InitCommands (void)
{
// general commands
	Cmd_AddCommand ("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand ("download", CL_Download_f);
	Cmd_AddCommand ("packet", CL_Packet_f);
	Cmd_AddCommand ("pause", CL_Pause_f);
	Cmd_AddCommand ("quit", CL_Quit_f);
	Cmd_AddCommand ("rcon", CL_Rcon_f);
	Cmd_AddCommand ("say", CL_Say_f);
	Cmd_AddCommand ("say_team", CL_Say_f);
	Cmd_AddCommand ("serverinfo", CL_Serverinfo_f);
	Cmd_AddCommand ("skins", Skin_Skins_f);
	Cmd_AddCommand ("allskins", Skin_AllSkins_f);
	Cmd_AddCommand ("user", CL_User_f);
	Cmd_AddCommand ("users", CL_Users_f);
	Cmd_AddCommand ("version", Version_f);
	Cmd_AddCommand ("writeconfig", CL_WriteConfig_f);

	Cmd_AddCommand ("ping", Cmd_Ping_f);

// client info setting
	Cmd_AddCommand ("color", CL_Color_f);
	Cmd_AddCommand ("fullinfo", CL_FullInfo_f);
	Cmd_AddCommand ("setinfo", CL_SetInfo_f);

#if 0
	Cmd_AddCommand ("save", Host_Savegame_f);
	Cmd_AddCommand ("load", Host_Loadgame_f);
#endif

// demo recording & playback
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("easyrecord", CL_EasyRecord_f);
	Cmd_AddCommand ("rerecord", CL_ReRecord_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("demojump", CL_DemoJump_f);

//
// forward to server commands
//
	Cmd_AddCommand ("kill", NULL);

//
//  Windows commands
//
#ifdef _WINDOWS
	Cmd_AddCommand ("windows", CL_Windows_f);
#endif
}


/*
==============================================================================

SERVER COMMANDS

Server commands are commands stuffed by server into client's cbuf
We use a separate command buffer for them -- there are several
reasons for that:
1. So that partially stuffed commands are always executed properly
2. Not to let players cheat in TF (v_cshift etc don't work in console)
3. To hide some commands the user doesn't need to know about, like
changing, fullserverinfo, nextul, stopul
==============================================================================
*/

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
void CL_Changing_f (void)
{
	//if (cls.download)  // don't change when downloading
	//	return;

	S_StopAllSounds (true);
	cl.intermission = 0;
	cls.state = ca_connected;	// not active anymore, but not disconnected

	Con_Printf ("\nChanging map...\n");
}


/*
==================
CL_FullServerinfo_f

Sent by server when serverinfo changes
==================
*/
void CL_FullServerinfo_f (void)
{
	char *p;
	float v;

	if (Cmd_Argc() != 2)
	{
		//Con_Printf ("usage: fullserverinfo <complete info string>\n");
		return;
	}

	strlcpy (cl.serverinfo, Cmd_Argv(1), MAX_SERVERINFO_STRING);

	
	server_version = 0;

	if ((p = Info_ValueForKey(cl.serverinfo, "*qwex_version")) && *p) {
		v = Q_atof(p);
		if (v) {
				Con_Printf("QWExtended Server Version %s\n", p);
			server_version = 2.40;
		}
	}
	if ((p = Info_ValueForKey(cl.serverinfo, "*version")) && *p) {
		v = Q_atof(p);
		if (v) {
			if (!server_version)
				Con_Printf("Version %1.2f Server\n", v);
			server_version = v;
		}
	}
	

	CL_ProcessServerInfo ();
}


void CL_Fov_f (void)
{
	extern cvar_t scr_fov, default_fov;

	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"fov\" is \"%s\"\n", scr_fov.string);
		return;
	}

	if (Q_atof(Cmd_Argv(1)) == 90.0 && default_fov.value)
		Cvar_SetValue (&scr_fov, default_fov.value);
	else
		Cvar_Set (&scr_fov, Cmd_Argv(1));
}


void CL_R_DrawViewModel_f (void)
{
	extern cvar_t cl_filterdrawviewmodel;

	if (cl_filterdrawviewmodel.value)
		return;
	Cvar_Command ();
}


typedef struct {
	char	*name;
	void	(*func) (void);
} svcmd_t;

svcmd_t svcmds[] =
{
	{"changing", CL_Changing_f},
	{"fullserverinfo", CL_FullServerinfo_f},
	{"nextul", CL_NextUpload},
	{"stopul", CL_StopUpload},
	{"fov", CL_Fov_f},
	{"r_drawviewmodel", CL_R_DrawViewModel_f},
	{"fileul", CL_StartFileUpload}, //bliP
	{NULL, NULL}
};

/*
================
CL_CheckServerCommand

Called by Cmd_ExecuteString if cbuf_current==&cbuf_svc
================
*/
qboolean CL_CheckServerCommand ()
{
	svcmd_t	*cmd;
	char	*s;

	s = Cmd_Argv (0);
	for (cmd=svcmds ; cmd->name ; cmd++)
		if (!strcmp (s, cmd->name) ) {
			cmd->func ();
			return true;
		}

	return false;
}

