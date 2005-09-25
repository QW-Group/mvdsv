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

	$Id: sv_main.c,v 1.21 2005/09/25 21:32:17 disconn3ct Exp $
*/

#include "version.h"
#include "qwsvdef.h"

quakeparms_t host_parms;

qboolean	host_initialized;		// true if into command execution (compatability)

float		sv_frametime;


double		realtime;				// without any filtering or bounding

int			host_hunklevel;

int			current_skill;			// for entity spawnflags checking

netadr_t	master_adr[MAX_MASTERS];	// address of group servers

client_t	*host_client;			// current client

cvar_t	sv_cpserver = {"sv_cpserver", "0"};	// some cp servers couse lags on map changes

#ifdef NEWWAY
cvar_t	sv_ticrate = {"sv_ticrate","0.014"};	// bound the size of the
#endif //NEWWAY
cvar_t	sv_mintic = {"sv_mintic","0.013"};	// bound the size of the

cvar_t	sv_maxtic = {"sv_maxtic","0.1"};	//

cvar_t	sys_select_timeout = {"sys_select_timeout", "10000"};
// MUST be set to ~ (sv_mintic / 1.3) * 1 000 000 = 10 000
// (else can occur packets lost if sv_minping > 0)
// if set too low then occur higher CPU usage
cvar_t	sys_restart_on_error = {"sys_restart_on_error", "0"};

cvar_t	developer = {"developer","0"};		// show extra messages

cvar_t	timeout = {"timeout","65"};		// seconds without any message
cvar_t	zombietime = {"zombietime", "2"};	// seconds to sink messages
											// after disconnect

cvar_t	rcon_password = {"rcon_password", ""};	// password for remote server commands
cvar_t	master_rcon_password = {"master_rcon_password", ""};	//bliP: password for remote server commands
cvar_t	password = {"password", ""};	// password for entering the game
cvar_t	sv_hashpasswords = {"sv_hashpasswords", "1"}; // 0 - plain passwords; 1 - hashed passwords
cvar_t	telnet_password = {"telnet_password", ""}; // password for login via telnet
cvar_t	not_auth_timeout = {"not_auth_timeout", "20"};
// if no password is sent (telnet_password) in "n" seconds the server refuses connection
// If set to 0, no timeout will occur
cvar_t	auth_timeout = {"auth_timeout", "3600"};
// the server will close the connection "n" seconds after the authentication is completed
// If set to 0, no timeout will occur
cvar_t	sv_crypt_rcon = {"sv_crypt_rcon", "1"}; // use SHA1 for encryption of rcon_password and using timestamps
// Time in seconds during which in rcon command this encryption is valid (change only with master_rcon_password).
cvar_t	sv_timestamplen = {"sv_timestamplen", "60"};
cvar_t	sv_rconlim = {"sv_rconlim", "10"};	// rcon bandwith limit: requests per second

//bliP: telnet log level
//cvar_t	telnet_log_level = {"telnet_log_level", "0"}; // logging level telnet console
qboolean OnChange_telnetloglevel_var (cvar_t *var, char *string);
cvar_t  telnet_log_level = {"telnet_log_level", "0", 0, OnChange_telnetloglevel_var};
//<-

cvar_t	frag_log_type = {"frag_log_type", "0"};
//	frag log type:
//		0 - old style (  qwsv - v0.165)
//		1 - new style (v0.168 - v0.172)

qboolean OnChange_qconsolelogsay_var (cvar_t *var, char *string);
cvar_t	qconsole_log_say = {"qconsole_log_say", "0", 0, OnChange_qconsolelogsay_var};
// logging "say" and "say_team" messages to the qconsole_PORT.log file

cvar_t	sys_command_line = {"sys_command_line", NULL, CVAR_ROM};

cvar_t	sv_use_dns = {"sv_use_dns", "0"}; // 1 - use DNS lookup in status command, 0 - don't use
cvar_t	spectator_password = {"spectator_password", ""};	// password for entering as a sepctator
cvar_t	vip_password = {"vip_password", ""};	// password for entering as a VIP sepctator

cvar_t	allow_download = {"allow_download", "1"};
cvar_t	allow_download_skins = {"allow_download_skins", "1"};
cvar_t	allow_download_models = {"allow_download_models", "1"};
cvar_t	allow_download_sounds = {"allow_download_sounds", "1"};
cvar_t	allow_download_maps = {"allow_download_maps", "1"};
cvar_t	allow_download_demos = {"allow_download_demos", "1"};
//bliP: init ->
cvar_t	allow_download_pakmaps = {"allow_download_pakmaps", "0"};
cvar_t	download_map_url = {"download_map_url", ""};

cvar_t	sv_specprint = {"sv_specprint", "0"};
cvar_t	sv_reconnectlimit = {"sv_reconnectlimit", "0"};

qboolean OnChange_admininfo_var (cvar_t *var, char *string);
cvar_t  sv_admininfo = {"sv_admininfo", "", 0, OnChange_admininfo_var};

cvar_t	sv_unfake = {"sv_unfake", "0"}; //bliP: 24/9 kickfake to unfake
cvar_t	sv_kicktop = {"sv_kicktop", "0"};

cvar_t	sv_maxlogsize = {"sv_maxlogsize", "0"};
//bliP: 24/9 ->
qboolean OnChange_logdir_var (cvar_t *var, char *string);
cvar_t  sv_logdir = {"sv_logdir", ".", 0, OnChange_logdir_var};
  
cvar_t  sv_speedcheck = {"sv_speedcheck", "0"};
//<-
//<-

cvar_t	sv_highchars = {"sv_highchars", "1"};

cvar_t	sv_phs = {"sv_phs", "1"};

cvar_t	pausable = {"pausable", "1"};

cvar_t	sv_maxrate = {"sv_maxrate", "0"};

cvar_t	sv_demofps = {"sv_demofps", "20"};
cvar_t	sv_demoPings = {"sv_demopings", "3"};
cvar_t	sv_demoNoVis = {"sv_demonovis", "1"};
cvar_t	sv_demoUseCache = {"sv_demoUseCache", "0"};
cvar_t	sv_demoCacheSize = {"sv_demoCacheSize", "0", CVAR_ROM};
cvar_t	sv_demoMaxSize  = {"sv_demoMaxSize", "20480"};
cvar_t	sv_demoMaxDirSize = {"sv_demoMaxDirSize", "102400"};
cvar_t	sv_demoClearOld = {"sv_demoClearOld", "0"}; //bliP: 24/9 clear old demos
cvar_t	sv_demoExtraNames = {"sv_demoExtraNames", "0"};

qboolean sv_demoDir_OnChange(cvar_t *cvar, char *value);
cvar_t	sv_demoDir = {"sv_demoDir", "demos", 0, sv_demoDir_OnChange};

cvar_t	sv_getrealip = {"sv_getrealip", "1"};
cvar_t	sv_minping = {"sv_minping", "0"};
cvar_t	sv_enable_cmd_minping = {"sv_enable_cmd_minping", "0"};
cvar_t	sv_serverip = {"sv_serverip", ""};
cvar_t	sv_maxdownloadrate = {"sv_maxdownloadrate", "0"};

cvar_t  sv_loadentfiles = {"sv_loadentfiles", "1"}; //loads .ent files by default if there
cvar_t	sv_default_name = {"sv_default_name", "unnamed"};

qboolean sv_mod_msg_file_OnChange(cvar_t *cvar, char *value);
cvar_t	sv_mod_msg_file = {"sv_mod_msg_file", "", 0, sv_mod_msg_file_OnChange};

//
// game rules mirrored in svs.info
//
cvar_t	fraglimit = {"fraglimit","0",CVAR_SERVERINFO};
cvar_t	timelimit = {"timelimit","0",CVAR_SERVERINFO};
cvar_t	teamplay = {"teamplay","0",CVAR_SERVERINFO};
cvar_t	samelevel = {"samelevel","0",CVAR_SERVERINFO};
cvar_t	maxclients = {"maxclients","8",CVAR_SERVERINFO};
cvar_t	maxspectators = {"maxspectators","8",CVAR_SERVERINFO};
cvar_t	maxvip_spectators = {"maxvip_spectators","0"/*,CVAR_SERVERINFO*/};
cvar_t	deathmatch = {"deathmatch","1",CVAR_SERVERINFO};			// 0, 1, or 2
cvar_t	spawn = {"spawn","0",CVAR_SERVERINFO};
cvar_t	watervis = {"watervis","0",CVAR_SERVERINFO};
cvar_t	serverdemo = {"serverdemo","",CVAR_SERVERINFO | CVAR_ROM};
// not mirrored
cvar_t	skill = {"skill", "1"};
cvar_t	coop = {"coop", "0"};

cvar_t	version = {"version", full_version, CVAR_ROM};

cvar_t	hostname = {"hostname","unnamed",CVAR_SERVERINFO};

cvar_t sv_forcenick = {"sv_forcenick", "0"}; //0 - don't force; 1 - as login;
cvar_t sv_registrationinfo = {"sv_registrationinfo", ""}; // text shown before "enter login" 

log_t	logs[MAX_LOG];

qboolean sv_error = 0;

void SV_AcceptClient (netadr_t adr, int userid, char *userinfo);
void Master_Shutdown (void);

//============================================================================

qboolean GameStarted(void)
{
	return sv.demorecording || strncasecmp(Info_ValueForKey(svs.info, "status"), "Standby", 8);
}
/*
================
SV_Shutdown

Quake calls this before calling Sys_Quit or Sys_Error
================
*/
void SV_Shutdown (void)
{
	int		i;
	Master_Shutdown ();
	if (telnetport)
		SV_Write_Log(TELNET_LOG, 1, "Server shutdown.\n");
	for (i = MIN_LOG; i < MAX_LOG; ++i)
	{
		if (logs[i].sv_logfile)
		{
			fclose (logs[i].sv_logfile);
			logs[i].sv_logfile = NULL;
		}
	}
	if (sv.demorecording)
		SV_Stop_f();

	NET_Shutdown ();
}

/*
================
SV_Error

Sends a datagram to all the clients informing them of the server crash,
then exits
================
*/
void SV_Error (char *error, ...)
{
	va_list		argptr;
	static	char		string[1024];
	static	qboolean inerror = false;

	sv_error = true;

	if (inerror)
		Sys_Error ("SV_Error: recursively entered (%s)", string);

	inerror = true;

	va_start (argptr,error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	Con_Printf ("SV_Error: %s\n",string);
	
	SV_FinalMessage (va("server crashed: %s\n", string));

	SV_Shutdown ();

	Sys_Error ("SV_Error: %s\n",string);
}

/*
==================
SV_FinalMessage

Used by SV_Error and SV_Quit_f to send a final message to all connected
clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage (char *message)
{
	int			i;
	client_t	*cl;
	
	SZ_Clear (&net_message);
	MSG_WriteByte (&net_message, svc_print);
	MSG_WriteByte (&net_message, PRINT_HIGH);
	MSG_WriteString (&net_message, message);
	MSG_WriteByte (&net_message, svc_disconnect);

	for (i=0, cl = svs.clients ; i<MAX_CLIENTS ; i++, cl++)
		if (cl->state >= cs_spawned)
			Netchan_Transmit (&cl->netchan, net_message.cursize
			, net_message.data);
}



/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing.
=====================
*/
#ifdef USE_PR2
void RemoveBot(client_t *cl);
#endif
void SV_DropClient (client_t *drop)
{
//bliP: cuff, mute ->
	SV_SavePenaltyFilter (drop, ft_mute, drop->lockedtill);
	SV_SavePenaltyFilter (drop, ft_cuff, drop->cuff_time);
//<-

//bliP: player logging
	if (drop->name[0])
		SV_LogPlayer(drop, "disconnect", 1);
//<-

	// add the disconnect
#ifdef USE_PR2
	if( drop->isBot )
	{
		RemoveBot(drop);
		return;  
	}
#endif
	MSG_WriteByte (&drop->netchan.message, svc_disconnect);

	if (drop->state == cs_spawned) {
		if (!drop->spectator)
		{
			// call the prog function for removing a client
			// this will set the body to a dead frame, among other things
			pr_global_struct->self = EDICT_TO_PROG(drop->edict);
#ifdef USE_PR2
			if ( sv_vm )
				PR2_GameClientDisconnect(0);
			else
#endif
				PR_ExecuteProgram (pr_global_struct->ClientDisconnect);
		}
		else if (SpectatorDisconnect)
		{
			// call the prog function for removing a client
			// this will set the body to a dead frame, among other things
			pr_global_struct->self = EDICT_TO_PROG(drop->edict);
#ifdef USE_PR2
			if ( sv_vm )
				PR2_GameClientDisconnect(1);
			else
#endif
				PR_ExecuteProgram (SpectatorDisconnect);
		}
	}

	if (drop->spectator)
		Con_Printf ("Spectator %s removed\n",drop->name);
	else
		Con_Printf ("Client %s removed\n",drop->name);

	if (drop->download)
	{
		fclose (drop->download);
		drop->download = NULL;
	}
	if (drop->upload)
	{
		fclose (drop->upload);
		drop->upload = NULL;
	}
	*drop->uploadfn = 0;

	SV_Logout(drop);

	drop->state = cs_zombie;		// become free in a few seconds
	drop->connection_started = realtime;	// for zombie timeout

	drop->old_frags = 0;
	drop->edict->v.frags = 0;
	drop->name[0] = 0;
	memset (drop->userinfo, 0, sizeof(drop->userinfo));
	memset (drop->userinfoshort, 0, sizeof(drop->userinfoshort));

// send notification to all remaining clients
	SV_FullClientUpdate (drop, &sv.reliable_datagram);
}


//====================================================================

/*
===================
SV_CalcPing

===================
*/
int SV_CalcPing (client_t *cl)
{
	float		ping;
	int			i;
	int			count;
	register	client_frame_t *frame;

  //bliP: 999 ping for connecting players
  if (cl->state != cs_spawned)
    return 999;
  //<-

	ping = 0;
	count = 0;
#ifdef USE_PR2
	if( cl->isBot )
		return sv_mintic.value * 1000;
#endif
	for (frame = cl->frames, i=0 ; i<UPDATE_BACKUP ; i++, frame++)
	{
		if (frame->ping_time > 0)
		{
			ping += frame->ping_time;
			count++;
		}
	}
	if (!count)
		return 9999;
	ping /= count;

	return ping*1000;
}

/*
===================
SV_FullClientUpdate

Writes all update values to a sizebuf
===================
*/
void SV_FullClientUpdate (client_t *client, sizebuf_t *buf)
{
	int		i;
	char	info[MAX_INFO_STRING];

	i = client - svs.clients;

//Sys_Printf("SV_FullClientUpdate:  Updated frags for client %d\n", i);

	MSG_WriteByte (buf, svc_updatefrags);
	MSG_WriteByte (buf, i);
	MSG_WriteShort (buf, client->old_frags);
	
	MSG_WriteByte (buf, svc_updateping);
	MSG_WriteByte (buf, i);
	MSG_WriteShort (buf, SV_CalcPing (client));
	
	MSG_WriteByte (buf, svc_updatepl);
	MSG_WriteByte (buf, i);
	MSG_WriteByte (buf, client->lossage);
	
	MSG_WriteByte (buf, svc_updateentertime);
	MSG_WriteByte (buf, i);
	MSG_WriteFloat (buf, realtime - client->connection_started);

	strlcpy (info, client->userinfoshort, MAX_INFO_STRING);
	Info_RemovePrefixedKeys (info, '_');	// server passwords, etc

	MSG_WriteByte (buf, svc_updateuserinfo);
	MSG_WriteByte (buf, i);
	MSG_WriteLong (buf, client->userid);
	MSG_WriteString (buf, info);
}

/*
===================
SV_FullClientUpdateToClient

Writes all update values to a client's reliable stream
===================
*/
void SV_FullClientUpdateToClient (client_t *client, client_t *cl)
{
	ClientReliableCheckBlock(cl, 24 + strlen(client->userinfo));
	if (cl->num_backbuf) {
		SV_FullClientUpdate (client, &cl->backbuf);
		ClientReliable_FinishWrite(cl);
	} else
		SV_FullClientUpdate (client, &cl->netchan.message);
}


/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see
This message can be up to around 5k with worst case string lengths.
================
*/
#define STATUS_OLDSTYLE					0
#define	STATUS_SERVERINFO				1
#define	STATUS_PLAYERS					2
#define	STATUS_SPECTATORS				4
#define	STATUS_SPECTATORS_AS_PLAYERS	8 //for ASE - change only frags: show as "S"

void SVC_Status (void)
{
	int		i, opt = 0;
	client_t	*cl;
	int		top, bottom, ping;
	char		*name, *frags;

	if (Cmd_Argc() > 1)
		opt = Q_atoi(Cmd_Argv(1));

	SV_BeginRedirect (RD_PACKET);
	if (opt == STATUS_OLDSTYLE || (opt & STATUS_SERVERINFO))
		Con_Printf ("%s\n", svs.info);
	if (opt == STATUS_OLDSTYLE || (opt & (STATUS_PLAYERS | STATUS_SPECTATORS)))
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			cl = &svs.clients[i];
			if ( (cl->state >= cs_preconnected/* || cl->state == cs_spawned */) &&
			     ( (!cl->spectator && ((opt & STATUS_PLAYERS) || opt == STATUS_OLDSTYLE)) ||
			       ( cl->spectator && ( opt & STATUS_SPECTATORS)) ) )
			{
				top    = atoi(Info_ValueForKey (cl->userinfo, "topcolor"));
				bottom = atoi(Info_ValueForKey (cl->userinfo, "bottomcolor"));
				top    = (top    < 0) ? 0 : ((top    > 13) ? 13 : top);
				bottom = (bottom < 0) ? 0 : ((bottom > 13) ? 13 : bottom);
				ping   = SV_CalcPing (cl);
				name   = cl->name;
				if (cl->spectator)
				{
					if (opt & STATUS_SPECTATORS_AS_PLAYERS)
						frags = "S";
					else
					{
						ping  = -ping;
						frags = "-9999";
						name  = va("\\s\\%s", name);
					}
				}
				else
					frags = va("%i", cl->old_frags);

				Con_Printf ("%i %s %i %i \"%s\" \"%s\" %i %i\n", cl->userid, frags,
					(int)(realtime - cl->connection_started)/60, ping, name,
					Info_ValueForKey (cl->userinfo, "skin"), top, bottom);
			}
		}
	SV_EndRedirect ();
}

/*
===================
SVC_LastScores

===================
*/
void SV_LastScores_f (void);
void SVC_LastScores (void)
{
	SV_BeginRedirect (RD_PACKET);
	SV_LastScores_f ();
	SV_EndRedirect ();
}

/*
===================
SVC_DemoList
SVC_DemoListRegex
===================
*/
void SV_DemoList_f (void);
void SVC_DemoList (void)
{
	SV_BeginRedirect (RD_PACKET);
	SV_DemoList_f ();
	SV_EndRedirect ();
}
void SV_DemoListRegex_f (void);
void SVC_DemoListRegex (void)
{
	SV_BeginRedirect (RD_PACKET);
	SV_DemoListRegex_f ();
	SV_EndRedirect ();
}

/*
===================
SV_CheckLog

===================
*/
#define	LOG_HIGHWATER	(MAX_DATAGRAM - 128)
#define	LOG_FLUSH		10*60
void SV_CheckLog (void)
{
	sizebuf_t	*sz;

	sz = &svs.log[svs.logsequence&1];

	// bump sequence if allmost full, or ten minutes have passed and
	// there is something still sitting there
	if (sz->cursize > LOG_HIGHWATER
	|| (realtime - svs.logtime > LOG_FLUSH && sz->cursize) )
	{
		// swap buffers and bump sequence
		svs.logtime = realtime;
		svs.logsequence++;
		sz = &svs.log[svs.logsequence&1];
		sz->cursize = 0;
		Con_DPrintf ("beginning fraglog sequence %i\n", svs.logsequence);
	}

}

/*
================
SVC_Log

Responds with all the logged frags for ranking programs.
If a sequence number is passed as a parameter and it is
the same as the current sequence, an A2A_NACK will be returned
instead of the data.
================
*/
void SVC_Log (void)
{
	int		seq;
	char	data[MAX_DATAGRAM+64];

	if (Cmd_Argc() == 2)
		seq = atoi(Cmd_Argv(1));
	else
		seq = -1;

	if (seq == svs.logsequence-1 || !logs[FRAG_LOG].sv_logfile)
	{	// they already have this data, or we aren't logging frags
		data[0] = A2A_NACK;
		NET_SendPacket (net_serversocket, 1, data, net_from);
		return;
	}

	Con_DPrintf ("sending log %i to %s\n", svs.logsequence-1, NET_AdrToString(net_from));

	snprintf (data, MAX_DATAGRAM + 64, "stdlog %i\n", svs.logsequence-1);
	strlcat (data, (char *)svs.log_buf[((svs.logsequence-1)&1)], MAX_DATAGRAM + 64);

	NET_SendPacket (net_serversocket, strlen(data)+1, data, net_from);
}

/*
================
SVC_Ping

Just responds with an acknowledgement
================
*/
void SVC_Ping (void)
{
	char	data;

	data = A2A_ACK;

	NET_SendPacket (net_serversocket, 1, &data, net_from);
}

/*
=================
SVC_GetChallenge

Returns a challenge number that can be used
in a subsequent client_connect command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.
=================
*/
void SVC_GetChallenge (void)
{
	int		i;
	int		oldest;
	int		oldestTime;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for (i = 0 ; i < MAX_CHALLENGES ; i++)
	{
		if (NET_CompareBaseAdr (net_from, svs.challenges[i].adr))
			break;
		if (svs.challenges[i].time < oldestTime)
		{
			oldestTime = svs.challenges[i].time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		svs.challenges[oldest].challenge = (rand() << 16) ^ rand();
		svs.challenges[oldest].adr = net_from;
		svs.challenges[oldest].time = realtime;
		i = oldest;
	}

	// send it back
	Netchan_OutOfBandPrint (net_serversocket, net_from, "%c%i", S2C_CHALLENGE, 
			svs.challenges[i].challenge);
}

qboolean ValidateUserInfo(char *userinfo)
{
	while (*userinfo)
	{
		if (*userinfo == '\\')
			userinfo++;

		if (*userinfo++ == '\\')
			return false;
		while (*userinfo && *userinfo != '\\')
			userinfo++;
	}
	return true;
}

/*
==================
SVC_DirectConnect

A connection request that did not come from the master
==================
*/
int SV_VIPbyIP(netadr_t adr);
int SV_VIPbyPass (char *pass);

#ifdef USE_PR2
extern char clientnames[MAX_CLIENTS][CLIENT_NAME_LEN];
int	userid;
#endif
void SVC_DirectConnect (void)
{
	char		userinfo[1024];
#ifndef USE_PR2
	static		int	userid;
#endif
	netadr_t	adr;
	int			i;
	client_t	*cl, *newcl;
	client_t	temp;
	edict_t		*ent;
	int			edictnum;
	char		*s, *key;
	int			clients, spectators, vips;
	qboolean	spectator;
	int			qport;
	int			version;
	int			challenge;
	qboolean	gotnewcl; //bliP: reuse connection
	qboolean	spass = false, vip;
	extern char *shortinfotbl[];

	version = atoi(Cmd_Argv(1));
	if (version != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nServer is version %4.2f.\n", A2C_PRINT, QW_VERSION);
		Con_Printf ("* rejected connect from version %i\n", version);
		return;
	}

	qport = atoi(Cmd_Argv(2));

	challenge = atoi(Cmd_Argv(3));

	// note an extra byte is needed to replace spectator key
	strlcpy (userinfo, Cmd_Argv(4), sizeof(userinfo));
	if (!ValidateUserInfo(userinfo)) {
		Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nInvalid userinfo. Restart your qwcl\n", A2C_PRINT);
		return;
	}

	// see if the challenge is valid
	for (i=0 ; i<MAX_CHALLENGES ; i++)
	{
		if (NET_CompareBaseAdr (net_from, svs.challenges[i].adr))
		{
			if (challenge == svs.challenges[i].challenge)
				break;		// good
			Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nBad challenge.\n", A2C_PRINT);
			return;
		}
	}
	if (i == MAX_CHALLENGES)
	{
		Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nNo challenge for address.\n", A2C_PRINT);
		return;
	}

	// check for password or spectator_password
	s = Info_ValueForKey (userinfo, "spectator");

	vip = false;
	if (s[0] && strcmp(s, "0"))
	{
		spass = true;

		if ((vip = SV_VIPbyPass(s)) == 0 &&
			(vip = SV_VIPbyPass(Info_ValueForKey (userinfo, "password"))) == 0) // first the pass, then ip
			vip = SV_VIPbyIP(net_from);
		
		if (spectator_password.string[0] && 
			strcasecmp(spectator_password.string, "none") &&
			strcmp(spectator_password.string, s) )
		{	// failed
			spass = false;
		}

		if (!vip && !spass)
		{
			Con_Printf ("%s:spectator password failed\n", NET_AdrToString (net_from));
			Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nrequires a spectator password\n\n", A2C_PRINT);
			return;
		}

		Info_RemoveKey (userinfo, "spectator"); // remove passwd
		Info_SetValueForStarKey (userinfo, "*spectator", "1", MAX_INFO_STRING);
		if ((spectator = atoi(s)) == 0)
			spectator = true;
	}
	else
	{
		s = Info_ValueForKey (userinfo, "password");
		if ((vip = SV_VIPbyPass(s)) == 0) // first the pass, then ip
			vip = SV_VIPbyIP(net_from);

		if (!vip && password.string[0] && 
			strcasecmp(password.string, "none") &&
			strcmp(password.string, s) )
		{
			Con_Printf ("%s:password failed\n", NET_AdrToString (net_from));
			Netchan_OutOfBandPrint (net_serversocket, net_from, "%c\nserver requires a password\n\n", A2C_PRINT);
			return;
		}
		spectator = false;
	}

	Info_RemoveKey (userinfo, "password"); // remove passwd

	adr = net_from;
	userid++;	// so every client gets a unique id

	newcl = &temp;
	memset (newcl, 0, sizeof(client_t));

	newcl->userid = userid;

	gotnewcl = false; //bliP: reuse connection

	// works properly
	if (!sv_highchars.value) {
		byte *p, *q;

		for (p = (byte *)newcl->userinfo, q = (byte *)userinfo; 
			*q && p < (byte *)newcl->userinfo + sizeof(newcl->userinfo)-1; q++)
			if (*q > 31 && *q <= 127)
				*p++ = *q;
	} else
		strlcpy (newcl->userinfo, userinfo, sizeof(newcl->userinfo));

	// if there is already a slot for this ip, reuse (changed from drop) it
	for (i=0,cl=svs.clients ; i<MAX_CLIENTS ; i++,cl++)
	{
		if (cl->state == cs_free)
			continue;
		if (NET_CompareBaseAdr (adr, cl->netchan.remote_address)
			&& ( cl->netchan.qport == qport 
			|| adr.port == cl->netchan.remote_address.port ))
		{
//bliP: reconnect limit
			if ((realtime - cl->lastconnect) < ((int)sv_reconnectlimit.value * 1000))
			{
				Con_Printf ("%s:reconnect rejected: too soon\n", NET_AdrToString (adr));
				userid--;
				return;
			}
//<-

			if (cl->state == cs_connected || cl->state == cs_preconnected) {
				Con_Printf("%s:dup connect\n", NET_AdrToString (adr));
				userid--;
				return;
			}

			Con_Printf ("%s:reconnect\n", NET_AdrToString (adr));
//bliP: reuse connect ->
			//SV_DropClient (cl);
			*cl = *newcl;
			newcl = cl;

			cl->download = NULL;
			cl->file_percent = 0;
// demo download list for internal cmd dl function
//Added by VVD {
			cl->demonum[0] = 0;
			cl->demolist = false;
// } Added by VVD

			gotnewcl = true;
			break;
//<-
		}
	}

	// count up the clients and spectators
	clients = 0;
	spectators = 0;
	vips = 0;
	for (i=0,cl=svs.clients ; i<MAX_CLIENTS ; i++,cl++)
	{
		if (cl->state == cs_free)
			continue;
//bliP: reuse connect ->
//don't count their existing connection so they can connect
		if (gotnewcl && cl->userid == newcl->userid)
			continue;
//<-

		if (cl->vip)
			vips++;
		if (cl->spectator) {
			if (!cl->vip)
				spectators++;
		} else
			clients++;
	}

	// if at server limits, refuse connection
	if (maxclients.value > MAX_CLIENTS)
		Cvar_SetValue (&maxclients, MAX_CLIENTS);
	if (maxspectators.value > MAX_CLIENTS)
		Cvar_SetValue (&maxspectators, MAX_CLIENTS);
	if (maxvip_spectators.value > MAX_CLIENTS)
		Cvar_SetValue (&maxvip_spectators, MAX_CLIENTS);

	if (maxspectators.value + maxclients.value > MAX_CLIENTS)
		Cvar_SetValue (&maxspectators, MAX_CLIENTS - maxclients.value);
	if (maxspectators.value + maxclients.value + maxvip_spectators.value > MAX_CLIENTS)
		Cvar_SetValue (&maxvip_spectators, MAX_CLIENTS - maxclients.value - maxspectators.value);

	if ( (vip && spectator && vips >= (int)maxvip_spectators.value &&
	      (spectators >= (int)maxspectators.value || !spass))
		|| (!vip && spectator && (spectators >= (int)maxspectators.value || !spass))
		|| (!spectator && clients >= (int)maxclients.value))
	{
		if (spectator == 2 && maxvip_spectators.value > vips && !vip)
		{
			Sys_Printf ("%s:full connect\n", NET_AdrToString (adr));
			newcl->rip_vip = 1; // yet can be connected if realip is on vip list
			newcl->vip = 1; // :)
		} else {
			Sys_Printf ("%s:full connect\n", NET_AdrToString (adr));
			Netchan_OutOfBandPrint (net_serversocket, adr, "%c\nserver is full\n\n", A2C_PRINT);
			userid--; //bliP: count users properly
			newcl->rip_vip = 3; // added drop client after "server is full" message
			return;
		}
	}

	// find a client slot
//bliP: reuse connection ->
	if (!gotnewcl) {
		newcl = NULL;
		for (i=0,cl=svs.clients ; i<MAX_CLIENTS ; i++,cl++)
		{
			if (cl->state == cs_free)
			{
				newcl = cl;
				break;
			}
		}
		if (!newcl)
		{
			Con_Printf ("WARNING: miscounted available clients\n");
			return;
		}
		// build a new connection
		// accept the new client
		// this is the only place a client_t is ever initialized
		*newcl = temp;
	}
//<-

	for (i = 0; i < UPDATE_BACKUP; i++)
		newcl->frames[i].entities.entities = cl_entities[newcl-svs.clients][i];

	Netchan_OutOfBandPrint (net_serversocket, adr, "%c", S2C_CONNECTION );

	edictnum = (newcl-svs.clients)+1;
	
	Netchan_Setup (&newcl->netchan, adr, qport, net_serversocket);

	newcl->state = cs_preconnected;

	newcl->datagram.allowoverflow = true;
	newcl->datagram.data = newcl->datagram_buf;
	newcl->datagram.maxsize = sizeof(newcl->datagram_buf);

	// spectator mode can ONLY be set at join time
	newcl->spectator = spectator;
	newcl->vip = vip;

	ent = EDICT_NUM(edictnum);
	newcl->edict = ent;
#ifdef USE_PR2
//restore pointer to client name
//for -progtype 0 (VM_NONE) names stored in clientnames array
//for -progtype 1 (VM_NATIVE) and -progtype 2 (VM_BYTECODE)  stored in mod memory
	if(sv_vm)
		newcl->name = PR2_GetString(ent->v.netname);
	else
		newcl->name = clientnames[edictnum - 1];
	memset(newcl->name, 0, CLIENT_NAME_LEN);
#endif

	if (vip) s = va("%d", vip);
	else s = "";

	Info_SetValueForStarKey (newcl->userinfo, "*VIP", s, MAX_INFO_STRING);
	// copy the most important userinfo into userinfoshort

	// parse some info from the info strings
	SV_ExtractFromUserinfo (newcl, true);

	for (i = 0; shortinfotbl[i] != NULL; i++)
	{
		s = Info_ValueForKey(newcl->userinfo, shortinfotbl[i]);
		Info_SetValueForStarKey (newcl->userinfoshort, shortinfotbl[i], s, MAX_INFO_STRING);
	}

	// move star keys to infoshort
	for (i= 1; (key = Info_KeyNameForKeyNum(newcl->userinfo, i)) != NULL; i++)
	{
		if (key[0] != '*')
			continue;

		s = Info_ValueForKey(newcl->userinfo, key);
		Info_SetValueForStarKey (newcl->userinfoshort, key, s, MAX_INFO_STRING);
	}

	// JACK: Init the floodprot stuff.
	for (i=0; i<10; i++)
		newcl->whensaid[i] = 0.0;
	newcl->whensaidhead = 0;
	newcl->lockedtill = 0;

	newcl->realip_num = rand();

//bliP: init
	newcl->spec_print = sv_specprint.value;
	newcl->logincount = 0;
//<-

	// call the progs to get default spawn parms for the new client
#ifdef USE_PR2
	if ( sv_vm )
		PR2_GameSetNewParms();
	else
#endif
		PR_ExecuteProgram (pr_global_struct->SetNewParms);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		newcl->spawn_parms[i] = (&pr_global_struct->parm1)[i];

	/*
	if (newcl->vip && newcl->spectator)
		Con_Printf ("VIP spectator %s connected\n", newcl->name);
	else if (newcl->spectator)
		Con_Printf ("Spectator %s connected\n", newcl->name);
	else
		Con_DPrintf ("Client %s connected\n", newcl->name);
	*/
	newcl->sendinfo = true;
}

int char2int(int c)
{
	if (c <= '9' && c >= '0')
		return c - '0';
	else if (c <= 'f' && c >= 'a')
		return c - 'a' + 10;
	else if (c <= 'F' && c >= 'A')
		return c - 'A' + 10;
	return 0;
}
/*
 * rcon_bandlim() - check for rcon requests bandwidth limit
 *
 *      From kernel of the FreeBSD 4.10 release:
 *      sys/netinet/ip_icmp.c(846): int badport_bandlim(int which);
 *
 *	Return false if it is ok to check rcon_password, true if we have
 *	hit our bandwidth limit and it is not ok.  
 *
 *	If sv_rconlim.value is <= 0, the feature is disabled and false is returned.
 *
 *	Note that the printing of the error message is delayed so we can
 *	properly print the rcon limit error rate that the system was trying to do
 *	(i.e. 22000/100 rcon pps, etc...).  This can cause long delays in printing
 *	the 'final' error, but it doesn't make sense to solve the printing 
 *	delay with more complex code.
 */
qboolean rcon_bandlim()
{
	static double	lticks = 0;
	static int	lpackets = 0;

	/*
	 * Return ok status if feature disabled or argument out of
	 * ranage.
	 */

	if (sv_rconlim.value <= 0)
		return false;

	/*
	 * reset stats when cumulative dt exceeds one second.
	 */

	if (realtime - lticks > 1.0)
	{
		if (lpackets > sv_rconlim.value)
			Sys_Printf("WARNING: Limiting rcon response from %d to %4f rcon pequests per second from %s\n",
					lpackets, sv_rconlim.value, NET_AdrToString(net_from));
		lticks = realtime;
		lpackets = 0;
	}

	/*
	 * bump packet count
	 */

	if (++lpackets > sv_rconlim.value)
		return true;

	return false;
}
//bliP: master rcon/logging ->
int Rcon_Validate (char *client_string, char *password)
{
//	char	*sha1;
	int		i;
	time_t	server_time, client_time = 0;
	double	difftime_server_client;

	if (rcon_bandlim())
		return 0;

	if (!strlen (password))
		return 0;

	if (sv_crypt_rcon.value)
	{
		time(&server_time);
		for (i = 0; i < sizeof(client_time) * 2; i += 2)
		{
//			Sys_Printf("1) %c%c, %d\n", (Cmd_Argv(1) + DIGEST_SIZE * 2)[i], (Cmd_Argv(1) + DIGEST_SIZE * 2)[i + 1], client_time);

			client_time +=  (char2int((unsigned char)(Cmd_Argv(1) + DIGEST_SIZE * 2)[i]) << (4 + i * 4)) +
					(char2int((unsigned char)(Cmd_Argv(1) + DIGEST_SIZE * 2)[i + 1]) << (i * 4));
//			Sys_Printf("2) %d, %d, %d\n", c1 << (4 + i * 4), c2 << (i * 4), client_time);
		}
		difftime_server_client = difftime(server_time, client_time);
//		Sys_Printf("3) %f, %d, %d\n", difftime_server_client, client_time, server_time);

		if (!sv_timestamplen.value)
			if (difftime_server_client >  sv_timestamplen.value ||
			    difftime_server_client < -sv_timestamplen.value)
			return 0;
		SHA1_Init();
		SHA1_Update(Cmd_Argv(0));
		SHA1_Update(" ");
		SHA1_Update(password);
		SHA1_Update(Cmd_Argv(1) + DIGEST_SIZE * 2);
		SHA1_Update(" ");
//		SHA1_Update(va("%s %s%s ", Cmd_Argv(0), password, Cmd_Argv(1) + DIGEST_SIZE * 2));
		for (i = 2; i < Cmd_Argc(); i++)
		{
//			SHA1_Update(va("%s ", Cmd_Argv(i)));
			SHA1_Update(Cmd_Argv(i));
			SHA1_Update(" ");
		}
//		sha1 = SHA1_Final();
//Con_Printf("client_string = %s\nserver_string = %s\nsha1 = %s\n", client_string, server_string, sha1);
//Con_Printf("server_string_len = %d, strlen(server_string) = %d\n", server_string_len, strlen(server_string));
		if (strncmp (Cmd_Argv(1), SHA1_Final(), DIGEST_SIZE * 2))
			return 0;
	}
	else
		if (strcmp (Cmd_Argv(1), password))
			return 0;
	return 1;
}

int Master_Rcon_Validate (void)
{
	char	*client_string;
	int	i, client_string_len = Cmd_Argc() + 1;

	for (i = 0; i < Cmd_Argc(); ++i)
		client_string_len += strlen(Cmd_Argv(i));
	client_string = Q_Malloc(client_string_len);
	*client_string = 0;
	for (i = 0; i < Cmd_Argc(); ++i)
	{
		strlcat(client_string, Cmd_Argv(i), client_string_len);
		strlcat(client_string, " ", client_string_len);
	}
//	Sys_Printf("client_string = %s\nclient_string_len = %d, strlen(client_string) = %d\n",
//		client_string, client_string_len, strlen(client_string));
	i = Rcon_Validate (client_string, master_rcon_password.string);
	Q_Free(client_string);
	return i;
}

/*
===============
SVC_RemoteCommand

A client issued an rcon command.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand (char *client_string)
{
	int		i;
	char		str[1024];
	char		plain[32];
	char		*hide, *p;
	client_t	*cl;
	qboolean	admin_cmd = false;
	qboolean	do_cmd = false;
	qboolean	bad_cmd = false;
	qboolean	banned = false;


	if (Rcon_Validate (client_string, master_rcon_password.string))
		do_cmd = true;
	else if (Rcon_Validate (client_string, rcon_password.string))
	{
		admin_cmd = true;
		if (SV_FilterPacket()) //banned players can't use rcon, but we log it
		{
			bad_cmd = true;
			banned = true;
		}
		else
		{
			strlcpy (str, Cmd_Argv(2), sizeof(str));
			if (	str[0] && //normal rcon can't use these commands
				(	!strcmp(str, "master_rcon_password") ||
					!strcmp(str, "rm") ||
					!strcmp(str, "rmdir") ||
					!strcmp(str, "ls") ||
					!strcmp(str, "chmod") ||
					!strcmp(str, "sv_admininfo") ||
					!strcmp(str, "if") ||
					!strcmp(str, "localcommand") ||
					!strcmp(str, "sv_crypt_rcon") ||
					!strcmp(str, "sv_timestamplen") ||
					!strncmp(str, "log", 3) ||
					!strcmp(str, "sys_command_line")
				)
			)
				bad_cmd = true;
		}
		do_cmd = !bad_cmd;
	}

//find player name if rcon came from someone on server
	plain[0] = '\0';
	for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
	{
		if (cl->state == cs_free)
			continue;
#ifdef USE_PR2
		if (cl->isBot)
			continue;
#endif
		if (!NET_CompareBaseAdr(net_from, cl->netchan.remote_address))
			continue;
		if (cl->netchan.remote_address.port != net_from.port)
			continue;
		strlcpy(plain, cl->name, sizeof(plain));
		Q_normalizetext(plain);
	}

	if (do_cmd)
	{
		if (!sv_crypt_rcon.value)
		{
			hide = net_message.data + 9;
			p = admin_cmd ? rcon_password.string : master_rcon_password.string;
			while (*p)
			{
				p++;
				*hide++ = '*';
			}
		}

    		if (plain[0])
			SV_Write_Log(RCON_LOG, 1, va("Rcon from %s (%s): %s\n", NET_AdrToString(net_from), plain, net_message.data + 4));
		else
			SV_Write_Log(RCON_LOG, 1, va("Rcon from %s: %s\n", NET_AdrToString(net_from), net_message.data + 4));
      
		Con_Printf("Rcon from %s:\n%s\n", NET_AdrToString(net_from), net_message.data + 4);

		SV_BeginRedirect(RD_PACKET);

		str[0] = '\0';
		for (i = 2; i < Cmd_Argc(); i++)
		{
			strlcat(str, Cmd_Argv(i), sizeof(str));
			strlcat(str, " ", sizeof(str));
		}

		Cmd_ExecuteString(str);
	}
	else
	{
		if (admin_cmd && !sv_crypt_rcon.value)
		{
			hide = net_message.data + 9;
			p = admin_cmd ? rcon_password.string : master_rcon_password.string;
			while (*p)
			{
				p++;
				*hide++ = '*';
			}
		}

		Con_Printf ("Bad rcon from %s: %s\n", NET_AdrToString(net_from), net_message.data + 4);

		if (!banned)
		{
			if (plain[0])
				SV_Write_Log(RCON_LOG, 1, va("Bad rcon from %s (%s):\n%s\n", NET_AdrToString(net_from), plain, net_message.data + 4));
			else
				SV_Write_Log(RCON_LOG, 1, va("Bad rcon from %s:\n%s\n",	NET_AdrToString (net_from), net_message.data + 4));
		}
		else
		{
			SV_Write_Log(RCON_LOG, 1, va("Rcon from banned IP: %s: %s\n", NET_AdrToString(net_from), net_message.data + 4));	
			SV_SendBan();
			return;
		}

		SV_BeginRedirect (RD_PACKET);
		if (admin_cmd)
			Con_Printf ("Command not valid.\n");
		else
			Con_Printf ("Bad rcon_password.\n");
	}
	SV_EndRedirect ();
}
//<-

void SVC_IP(void)
{
	int num;
	client_t *client;

	if (Cmd_Argc() < 3)
		return;

	num = atoi(Cmd_Argv(1));

	if (num < 0 || num >= MAX_CLIENTS)
		return;

	client = &svs.clients[num];
	if (client->state != cs_preconnected)
		return;

	// prevent cheating
	if (client->realip_num != atoi(Cmd_Argv(2)))
		return;

	// don't override previously set ip
	if (client->realip.ip[0])
		return;

	client->realip = net_from;

	// if banned drop
	if (SV_FilterPacket()/* && !client->vip*/)
		SV_DropClient(client);

}


/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/

void SV_ConnectionlessPacket (void)
{
	char	*s;
	char	*c;

	MSG_BeginReading ();
	MSG_ReadLong ();		// skip the -1 marker

	s = MSG_ReadStringLine ();

	Cmd_TokenizeString (s);

	c = Cmd_Argv(0);

	if (!strcmp(c, "ping") || ( c[0] == A2A_PING && (c[1] == 0 || c[1] == '\n')) )
		SVC_Ping ();
	else if (c[0] == A2A_ACK && (c[1] == 0 || c[1] == '\n') )
		Con_Printf ("A2A_ACK from %s\n", NET_AdrToString (net_from));
	else if (!strcmp(c,"status"))
		SVC_Status ();
	else if (!strcmp(c,"log"))
		SVC_Log ();
	else if (!strcmp(c, "rcon"))
		SVC_RemoteCommand (s);
	else if (!strcmp(c, "ip"))
		SVC_IP();
	else if (!strcmp(c,"connect"))
		SVC_DirectConnect ();
	else if (!strcmp(c,"getchallenge"))
		SVC_GetChallenge ();
	else if (!strcmp(c,"lastscores"))
		SVC_LastScores ();
	else if (!strcmp(c,"dlist"))
		SVC_DemoList ();
	else if (!strcmp(c,"dlistr"))
		SVC_DemoListRegex ();
	else if (!strcmp(c,"dlistregex"))
		SVC_DemoListRegex ();
	else if (!strcmp(c,"demolist"))
		SVC_DemoList ();
	else if (!strcmp(c,"demolistr"))
		SVC_DemoListRegex ();
	else if (!strcmp(c,"demolistregex"))
		SVC_DemoListRegex ();
	else
		Con_Printf ("bad connectionless packet from %s:\n%s\n"
		, NET_AdrToString (net_from), s);
}

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/


/*typedef struct
{
	unsigned	mask;
	unsigned	compare;
	int			level;
} ipfilter_t;
*/

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[MAX_IPFILTERS];
int			numipfilters;

ipfilter_t	ipvip[MAX_IPFILTERS];
int			numipvips;

//bliP: cuff, mute ->
penfilter_t	penfilters[MAX_PENFILTERS];
int			numpenfilters;
//<-

cvar_t	filterban = {"filterban", "1"};

/*
=================
StringToFilter
=================
*/
qboolean StringToFilter (char *s, ipfilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			//Con_Printf ("Bad filter address: %s\n", s);
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
SV_AddIPVIP_f
=================
*/
void SV_AddIPVIP_f (void)
{
	int		i, l;
	ipfilter_t f;

	if (!StringToFilter (Cmd_Argv(1), &f)) {
		Con_Printf ("Bad filter address: %s\n", Cmd_Argv(1));
		return;
	}

	l = atoi(Cmd_Argv(2));

	if (l < 1) l = 1;

	for (i=0 ; i<numipvips ; i++)
		if (ipvip[i].compare == 0xffffffff || (ipvip[i].mask == f.mask
		&& ipvip[i].compare == f.compare))
			break;		// free spot
	if (i == numipvips)
	{
		if (numipvips == MAX_IPFILTERS)
		{
			Con_Printf ("VIP spectator IP list is full\n");
			return;
		}
		numipvips++;
	}
	
	ipvip[i] = f;
	ipvip[i].level = l;
}

/*
=================
SV_RemoveIPVIP_f
=================
*/
void SV_RemoveIPVIP_f (void)
{
	ipfilter_t	f;
	int		i, j;

	if (!StringToFilter (Cmd_Argv(1), &f)) {
		Con_Printf ("Bad filter address: %s\n", Cmd_Argv(1));
		return;
	}
	for (i=0 ; i<numipvips ; i++)
		if (ipvip[i].mask == f.mask
		&& ipvip[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipvips ; j++)
				ipvip[j-1] = ipvip[j];
			numipvips--;
			Con_Printf ("Removed.\n");
			return;
		}
	Con_Printf ("Didn't find %s.\n", Cmd_Argv(1));
}

/*
=================
SV_ListIP_f
=================
*/
void SV_ListIPVIP_f (void)
{
	int		i;
	byte	b[4];

	Con_Printf ("VIP list:\n");
	for (i=0 ; i<numipvips ; i++)
	{
		*(unsigned *)b = ipvip[i].compare;
		Con_Printf ("%3i.%3i.%3i.%3i   level %d\n", b[0], b[1], b[2], b[3], ipvip[i].level);
	}
}

/*
=================
SV_WriteIPVIP_f
=================
*/
void SV_WriteIPVIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;

	snprintf (name, MAX_OSPATH, "%s/vip_ip.cfg", com_gamedir);

	Con_Printf ("Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		Con_Printf ("Couldn't open %s\n", name);
		return;
	}
	
	for (i=0 ; i<numipvips ; i++)
	{
		*(unsigned *)b = ipvip[i].compare;
		fprintf (f, "vip_addip %i.%i.%i.%i %d\n", b[0], b[1], b[2], b[3], ipvip[i].level);
	}
	
	fclose (f);
}


/*
=================
SV_AddIP_f
=================
*/
void SV_AddIP_f (void)
{
	int		i;
	ipfilter_t f;

	if (!StringToFilter (Cmd_Argv(1), &f)) {
		Con_Printf ("Bad filter address: %s\n", Cmd_Argv(1));
		return;
	}
	
	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].compare == 0xffffffff || (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare))
			break;		// free spot
	if (i == numipfilters)
	{
		if (numipfilters == MAX_IPFILTERS)
		{
			Con_Printf ("IP filter list is full\n");
			return;
		}
		numipfilters++;
	}
	
	ipfilters[i] = f;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SV_RemoveIP_f (void)
{
	ipfilter_t	f;
	int			i, j;

	if (!StringToFilter (Cmd_Argv(1), &f))
	{
		Con_Printf ("Bad filter address: %s\n", Cmd_Argv(1));
		return;
	}

	for (i=0 ; i<numipfilters ; i++)
		if (ipfilters[i].mask == f.mask
		&& ipfilters[i].compare == f.compare)
		{
			for (j=i+1 ; j<numipfilters ; j++)
				ipfilters[j-1] = ipfilters[j];
			numipfilters--;
			Con_Printf ("Removed.\n");
			return;
		}
	Con_Printf ("Didn't find %s.\n", Cmd_Argv(1));
}

/*
=================
SV_ListIP_f
=================
*/
void SV_ListIP_f (void)
{
	int		i;
	byte	b[4];

	Con_Printf ("Filter list:\n");
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		Con_Printf ("%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SV_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	byte	b[4];
	int		i;

	snprintf (name, MAX_OSPATH, "%s/listip.cfg", com_gamedir);

	Con_Printf ("Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f)
	{
		Con_Printf ("Couldn't open %s\n", name);
		return;
	}
	
	for (i=0 ; i<numipfilters ; i++)
	{
		*(unsigned *)b = ipfilters[i].compare;
		fprintf (f, "addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
	}
	
	fclose (f);
}

/*
=================
SV_SendBan
=================
*/
void SV_SendBan (void)
{
	char		data[128];

	data[0] = data[1] = data[2] = data[3] = 0xff;
	data[4] = A2C_PRINT;
	data[5] = 0;
	strlcat (data, "\nbanned.\n", sizeof(data));
	
	NET_SendPacket (net_serversocket, strlen(data), data, net_from);
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket (void)
{
	int		i;
	unsigned	in;
	
	in = *(unsigned *)net_from.ip;

	for (i=0 ; i<numipfilters ; i++)
		if ( (in & ipfilters[i].mask) == ipfilters[i].compare)
			return filterban.value;

	return !filterban.value;
}

/*
=================
SV_VIPbyIP
=================
*/
int SV_VIPbyIP (netadr_t adr)
{
	int		i;
	unsigned	in;
	
	in = *(unsigned *)adr.ip;

	for (i=0 ; i<numipvips ; i++)
		if ( (in & ipvip[i].mask) == ipvip[i].compare)
			return ipvip[i].level;

	return 0;
}

/*
=================
SV_VIPbyPass
=================
*/
int SV_VIPbyPass (char *pass)
{
	int i;

	if (!vip_password.string[0] || !strcasecmp(vip_password.string, "none"))
		return 0;

	Cmd_TokenizeString(vip_password.string);

	for (i = 0; i < Cmd_Argc(); i++)
		if (!strcmp(Cmd_Argv(i), pass) && strcasecmp(Cmd_Argv(i), "none"))
			return i+1;

	return 0;
}

char *DecodeArgs(char *args)
{
	static char string[1024];
	char *p, key[32], *s, *value, ch;
	extern char chartbl2[256];// defined in pr_cmds.c

	string[0] = 0;
	p = string;

	while (*args) {
		// skip whitespaces
		while (*args && *args <= 32)
			*p++ = *args++;

		if (*args == '\"') {
			do *p++ = *args++; while (*args && *args != '\"'); 
			*p++ = '\"';
			if (*args)
				args++;
		} else if (*args == '@' || *args == '$')
		{
			// get the key and read value from localinfo
			ch = *args;
			s = key;
			args++;
			while (*args > 32)
				*s++ = *args++;
			*s = 0;

			if ((value = Info_ValueForKey (svs.info, key)) == NULL || !*value)
				value = Info_ValueForKey(localinfo, key);

			*p++ = '\"';
			if (ch == '$') {
				if (value) while (*value)
					*p++ = chartbl2[(byte)*value++];
			} else {
				if (value) while (*value)
					*p++ = *value++;
			}
			*p++ = '\"';
		} else while (*args > 32)
			*p++ = *args++;
	}

	*p = 0;

	return string;
}

void SV_Script_f (void)
{
	char *path, *p;
	extern redirect_t sv_redirected;

	if (Cmd_Argc() < 2) {
		Con_Printf("usage: script <path> [<args>]\n");
		return;
	}

	path = Cmd_Argv(1);

//bliP: 24/9 need subdirs here ->
	if (!strncmp(path, "../", 3) || !strncmp(path, "..\\", 3))
		path += 3;

	if (strstr(path, ".."))
	{
			Con_Printf("Invalid path.\n");
			return;
	}
//<-

	path = Cmd_Argv(1);
	
	p = Cmd_Args();
	while (*p > 32)
		p++;
	while (*p && *p <= 32)
		p++;

	p = DecodeArgs(p);

	if (sv_redirected != RD_MOD)
		Sys_Printf("Running %s.qws\n", path);

	Sys_Script(path, va("%d %s", sv_redirected, p));

}

//============================================================================

//bliP: cuff, mute ->
void SV_RemoveIPFilter (int i)
{
	for (; i + 1 < numpenfilters; i++)
		penfilters[i] = penfilters[i + 1];

	numpenfilters--;
}

void SV_CleanIPList (void)
{
	int     i;

	for (i = 0; i < numpenfilters;) {
		if (penfilters[i].time && (penfilters[i].time <= realtime)) {
			SV_RemoveIPFilter (i);
		} else
			i++;
	}
}

static qboolean SV_IPCompare (byte *a, byte *b)
{
	int i;

	for (i = 0; i < 1; i++)
		if (((unsigned int *)a)[i] != ((unsigned int *)b)[i])
			return false;

	return true;
}

static void SV_IPCopy (byte *dest, byte *src)
{
	int i;

	for (i = 0; i < 1; i++)
		((unsigned int *)dest)[i] = ((unsigned int *)src)[i];
}

void SV_SavePenaltyFilter (client_t *cl, filtertype_t type, double pentime)
{
	int i;

	if (pentime < realtime)   // no point
		return;

	for (i = 0; i < numpenfilters; i++)
		if (SV_IPCompare (penfilters[i].ip, cl->realip.ip)	&& penfilters[i].type == type) {
			return;
		}

	if (numpenfilters == MAX_IPFILTERS) {
		return;
	}

	SV_IPCopy (penfilters[numpenfilters].ip, cl->realip.ip);
	penfilters[numpenfilters].time = pentime;
	penfilters[numpenfilters].type = type;
	numpenfilters++;
}

double SV_RestorePenaltyFilter (client_t *cl, filtertype_t type)
{
	int     i;
	double  time;

	time = 0.0;

	// search for existing penalty filter of same type
	for (i = 0; i < numpenfilters; i++) {
		if (type == penfilters[i].type && SV_IPCompare (cl->realip.ip, penfilters[i].ip)) {
			time = penfilters[i].time;
			SV_RemoveIPFilter (i);
			return time;
		}
	}
	return time;
}
//<-

//============================================================================

/*
=================
SV_ReadPackets
=================
*/
void SV_ReadPackets (void)
{
	int			i;
	unsigned	seq;
	client_t	*cl;
	packet_t	*pack;
	qboolean	good;
	int			qport;

	good = false;
	while (NET_GetPacket(net_serversocket))
	{
		// check for connectionless packet (0xffffffff) first
		if (*(int *)net_message.data == -1)
		{
			SV_ConnectionlessPacket ();
			continue;
		}

		if (SV_FilterPacket ())
		{
			SV_SendBan ();	// tell them we aren't listening...
			continue;
		}
		
		// read the qport out of the message so we can fix up
		// stupid address translating routers
		MSG_BeginReading ();
		MSG_ReadLong ();		// sequence number
		seq = MSG_ReadLong () & ~(1<<31);		// sequence number
		qport = MSG_ReadShort () & 0xffff;

		// check for packets from connected clients
		for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
		{
			if (cl->state == cs_free)
				continue;
			if (!NET_CompareBaseAdr (net_from, cl->netchan.remote_address))
				continue;
			if (cl->netchan.qport != qport)
				continue;
			if (cl->netchan.remote_address.port != net_from.port)
			{
				Con_DPrintf ("SV_ReadPackets: fixing up a translated port\n");
				cl->netchan.remote_address.port = net_from.port;
			}

			if (svs.num_packets == MAX_DELAYED_PACKETS) // packet has to be dropped..
			{
				Sys_Printf("svs.num_packets == MAX_DELAYED_PACKETS\n");
				break;
			}

			pack = &svs.packets[svs.num_packets++];

			pack->time = realtime;
			pack->num = i;
			SZ_Clear(&pack->sb);
			SZ_Write(&pack->sb, net_message.data, net_message.cursize);
			break;

/*			if (Netchan_Process(&cl->netchan))
			{	// this is a valid, sequenced packet, so process it
				svs.stats.packets++;
				good = true;
				cl->send_message = true;	// reply at end of frame
				if (cl->state != cs_zombie)
					SV_ExecuteClientMessage (cl);
			}
			break;
*/
		}
		
//		if (i != MAX_CLIENTS)
//			continue;
	
		// packet is not from a known client
		//	Con_Printf ("%s:sequenced packet without connection\n"
		// ,NET_AdrToString(net_from));
	}
}

/*
=================
SV_ReadDelayedPackets
=================
*/
void SV_ReadDelayedPackets (void)
{
	int			i, j/*, num = 0*/;
	client_t	*cl;
	packet_t	*pack;

	for (i = 0, pack = svs.packets; i < svs.num_packets; )
	{
		cl = &svs.clients[pack->num];
		if (realtime < pack->time + cl->delay && !sv.paused) {
			i++;
			pack++;
			continue;
		}

		//num++;

		SZ_Clear(&net_message);
		SZ_Write(&net_message, pack->sb.data, pack->sb.cursize);
		net_from = cl->netchan.remote_address;

		if (Netchan_Process(&cl->netchan))
		{	// this is a valid, sequenced packet, so process it
			svs.stats.packets++;
			cl->send_message = true;	// reply at end of frame
			if (cl->state != cs_zombie)
				SV_ExecuteClientMessage (cl);
		}

		for (j = i + 1; j < svs.num_packets; j++) {
			SZ_Clear(&svs.packets[j - 1].sb);
			SZ_Write(&svs.packets[j - 1].sb, svs.packets[j].sb.data, svs.packets[j].sb.cursize);
			svs.packets[j - 1].num = svs.packets[j].num;
			svs.packets[j - 1].time = svs.packets[j].time;
		}

		svs.num_packets--;
		//i = 0;
		//pack = svs.packets;
	}

	//svs.num_packets -= num;
}

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client in timeout.value
seconds, drop the conneciton.

When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts (void)
{
	int		i;
	client_t	*cl;
	float	droptime;
	int	nclients;
	
	droptime = realtime - timeout.value;
	nclients = 0;

	for (i=0,cl=svs.clients ; i<MAX_CLIENTS ; i++,cl++)
	{
#ifdef USE_PR2
		if( cl->isBot ) 
			continue;
#endif
		if (cl->state >= cs_preconnected /*|| cl->state == cs_spawned*/) {
			if (!cl->spectator)
				nclients++;
			if (cl->netchan.last_received < droptime) {
				SV_BroadcastPrintf (PRINT_HIGH, "%s timed out\n", cl->name);
				SV_DropClient (cl); 
				cl->state = cs_free;	// don't bother with zombie state
			}
			if (!cl->logged)
				SV_LoginCheckTimeOut(cl);
		}
		if (cl->state == cs_zombie && 
			realtime - cl->connection_started > zombietime.value)
		{
			cl->state = cs_free;	// can now be reused
		}
	}
	if (sv.paused && !nclients) {
		// nobody left, unpause the server
		SV_TogglePause("Pause released since no players are left.\n");
	}
}

/*
===================
SV_GetConsoleCommands

Add them exactly as if they had been typed at the console
===================
*/
void SV_GetConsoleCommands (void)
{
	char	*cmd;

	while (1)
	{
		cmd = Sys_ConsoleInput ();
		if (!cmd)
			break;
		Cbuf_AddText (cmd);
		Cbuf_AddText ("\n");
	}
}


/*
===================
SV_BoundRate
===================
*/
int SV_BoundRate (qboolean dl, int rate)
{
	if (!rate)
		rate = 2500;
	if (dl)
	{
		if (!sv_maxdownloadrate.value && sv_maxrate.value && rate > sv_maxrate.value)
			rate = sv_maxrate.value;

		if (sv_maxdownloadrate.value && rate > sv_maxdownloadrate.value)
			rate = sv_maxdownloadrate.value;
	} else
		if (sv_maxrate.value && rate > sv_maxrate.value)
			rate = sv_maxrate.value;

	if (rate < 500)
		rate = 500;
	if (rate > 100000)
		rate = 100000;

	return rate;
}


/*
===================
SV_CheckVars

===================
*/

void SV_CheckVars (void)
{
	static char pw[MAX_INFO_STRING]="", spw[MAX_INFO_STRING]="", vspw[MAX_INFO_STRING]="";
	static float old_maxrate = 0, old_maxdlrate = 0;
	int			v;

// check password and spectator_password
	if (strcmp(password.string, pw) ||
		strcmp(spectator_password.string, spw) || strcmp(vip_password.string, vspw))
	{
		strlcpy (pw, password.string, MAX_INFO_STRING);
		strlcpy (spw, spectator_password.string, MAX_INFO_STRING);
		strlcpy (vspw, vip_password.string, MAX_INFO_STRING);
		Cvar_Set (&password, pw);
		Cvar_Set (&spectator_password, spw);
		Cvar_Set (&vip_password, vspw);
		
		v = 0;
		if (pw && pw[0] && strcmp(pw, "none"))
			v |= 1;
		if (spw && spw[0] && strcmp(spw, "none"))
			v |= 2;
		if (vspw && vspw[0] && strcmp(vspw, "none"))
			v |= 4;
		
		Con_DPrintf ("Updated needpass.\n");
		if (!v)
			Info_SetValueForKey (svs.info, "needpass", "", MAX_SERVERINFO_STRING);
		else
			Info_SetValueForKey (svs.info, "needpass", va("%i",v), MAX_SERVERINFO_STRING);
	}

// check sv_maxrate
	if (sv_maxrate.value != old_maxrate || sv_maxdownloadrate.value != old_maxdlrate ) {
		client_t	*cl;
		int			i;
		char		*val;

		old_maxrate = sv_maxrate.value;
		old_maxdlrate = sv_maxdownloadrate.value;

		for (i=0, cl = svs.clients ; i<MAX_CLIENTS ; i++, cl++)
		{
			if (cl->state < cs_preconnected)
				continue;

			if (cl->download) {
				val = Info_ValueForKey (cl->userinfo, "drate");
				if (!*val)
					val = Info_ValueForKey (cl->userinfo, "rate");
			} else
				val = Info_ValueForKey (cl->userinfo, "rate");

			cl->netchan.rate = 1.0 / SV_BoundRate (cl->download != NULL, atoi(val));
		}
	}
}

/*
==================
SV_Frame

==================
*/
void SV_Map (qboolean now);
void SV_Frame (double time)
{
	static double	start, end;
	double	demo_start, demo_end;

#if 0	// disabled for now

	if (sv.state != ss_active || cls.state != ca_active || (int)maxclients.value > 1 || key_dest == key_game)
	{
		sv.paused &= ~2;
		cl.paused &= ~4;
	}
	else
	{
		sv.paused |= 2;
		cl.paused |= 4;
	}
#endif

	start = Sys_DoubleTime ();
	svs.stats.idle += start - end;
	
// keep the random time dependent
	rand ();

// decide the simulation time
	if (!sv.paused)
	{

		realtime += time;
#ifndef NEWWAY
		sv.time += time;
#endif
		sv.gametime += time;
	}

// check timeouts
	SV_CheckTimeouts ();

  //bliP: cuff, mute ->
// clean out expired cuffs/mutes
	SV_CleanIPList ();
  //<-

// toggle the log buffer if full
	SV_CheckLog ();

// check for commands typed to the host
	SV_GetConsoleCommands ();
	
// process console commands

	Cbuf_Execute ();
// check for map change;
	SV_Map(true);

	SV_CheckVars ();

#ifdef NEWWAY
	if (sv.gametime < sv.time)
	{
		// never let the time get too far off
		if (sv.time - sv.gametime > 0.1)
		{
			//if (sv_showclamp->value)
				Con_Printf ("sv lowclamp\n");
			sv.gametime = sv.time - 0.1;
		}

		end = Sys_DoubleTime ();
		svs.stats.active += end-start;

		if (NET_Sleep(sv.time - sv.gametime) <= 0)
			return;

		start = Sys_DoubleTime ();
		svs.stats.idle += start - end;
		//return;
	} else if (!sv.paused)
		SV_Physics ();

// get packets
	SV_ReadPackets ();
#else

// get packets
	SV_ReadPackets ();

// check delayed packets
	SV_ReadDelayedPackets ();

// move autonomous things around if enough time has passed
	if (!sv.paused)
		SV_Physics ();
#endif

// send messages back to the clients that had packets read this frame
	SV_SendClientMessages ();

// check delayed messages back to the clients that had packets read this frame
//	SV_SendDelayedClientMessages ();

	demo_start = Sys_DoubleTime ();
	SV_SendDemoMessage();
	demo_end = Sys_DoubleTime ();
	svs.stats.demo += demo_end - demo_start;

// send a heartbeat to the master if needed
	Master_Heartbeat ();

// collect timing statistics
	end = Sys_DoubleTime ();
	svs.stats.active += end-start;
	if (++svs.stats.count == STATFRAMES)
	{
		svs.stats.latched_active = svs.stats.active;
		svs.stats.latched_idle = svs.stats.idle;
		svs.stats.latched_packets = svs.stats.packets;
		svs.stats.latched_demo = svs.stats.demo;
		svs.stats.active = 0;
		svs.stats.idle = 0;
		svs.stats.packets = 0;
		svs.stats.count = 0;
		svs.stats.demo = 0;
	}
}

/*
===============
SV_InitLocal
===============
*/
void SV_Record_f (void);
void SV_EasyRecord_f (void);
//void SV_DemoList_f (void);
//void SV_DemoListRegex_f (void);
void SV_DemoRemove_f (void);
void SV_DemoRemoveNum_f (void);
void SV_Cancel_f (void);
void SV_DemoInfoAdd_f (void);
void SV_DemoInfoRemove_f (void);
void SV_DemoInfo_f (void);

void SV_InitLocal (void)
{
	int		i, len;
	extern	cvar_t	sv_maxvelocity;
	extern	cvar_t	sv_gravity;
	extern	cvar_t	sv_stopspeed;
	extern	cvar_t	sv_spectatormaxspeed;
	extern	cvar_t	sv_accelerate;
	extern	cvar_t	sv_airaccelerate;
	extern	cvar_t	sv_wateraccelerate;
	extern	cvar_t	sv_friction;
	extern	cvar_t	sv_waterfriction;
	extern	cvar_t	sv_bunnyspeedcap;
	extern	cvar_t	sv_nailhack;
	extern	cvar_t	mvd_streamport;


	Cvar_Init ();

	SV_InitOperatorCommands	();
	SV_UserInit ();

	Cvar_RegisterVariable (&sv_getrealip);
	Cvar_RegisterVariable (&sv_maxdownloadrate);
	Cvar_RegisterVariable (&sv_minping);
	Cvar_RegisterVariable (&sv_enable_cmd_minping);
	Cvar_RegisterVariable (&sv_serverip);
	Cvar_RegisterVariable (&sv_cpserver);
	Cvar_RegisterVariable (&rcon_password);
	Cvar_RegisterVariable (&master_rcon_password); //bliP: init
	Cvar_RegisterVariable (&password);
	Cvar_RegisterVariable (&sv_hashpasswords);
//Added by VVD {
	Cvar_RegisterVariable (&sv_crypt_rcon);
	Cvar_RegisterVariable (&sv_timestamplen);
	Cvar_RegisterVariable (&sv_rconlim);

	Cvar_RegisterVariable (&telnet_password);
	Cvar_RegisterVariable (&telnet_log_level);
	Cvar_RegisterVariable (&not_auth_timeout);
	Cvar_RegisterVariable (&auth_timeout);

	Cvar_RegisterVariable (&frag_log_type);
	Cvar_RegisterVariable (&qconsole_log_say);
	Cvar_RegisterVariable (&sv_use_dns);

	for (i = 0, len = 1; i < com_argc; i++)
		len += strlen(com_argv[i]) + 1;
	sys_command_line.string = (char *)Q_Malloc(len);
	sys_command_line.string[0] = 0;
	for (i = 0; i < com_argc; i++)
	{
		strlcat(sys_command_line.string, com_argv[i], len);
		strlcat(sys_command_line.string, " ", len);
	}
	Cvar_RegisterVariable (&sys_command_line);

	snprintf(full_version, SIZEOF_FULL_VERSION, FULL_VERSION "\n" BUILD_DATE "\n", build_number());
	Cvar_RegisterVariable (&version);
//Added by VVD }
	Cvar_RegisterVariable (&spectator_password);
	Cvar_RegisterVariable (&vip_password);

	Cvar_RegisterVariable (&sv_nailhack);

#ifdef NEWWAY
	Cvar_RegisterVariable (&sv_ticrate);
#endif //NEWWAY
	Cvar_RegisterVariable (&sv_mintic);
	Cvar_RegisterVariable (&sv_maxtic);
	Cvar_RegisterVariable (&sys_select_timeout);
	Cvar_RegisterVariable (&sys_restart_on_error);

	Cvar_RegisterVariable (&skill);
	Cvar_RegisterVariable (&coop);

	Cvar_RegisterVariable (&fraglimit);
	Cvar_RegisterVariable (&timelimit);
	Cvar_RegisterVariable (&teamplay);
	Cvar_RegisterVariable (&samelevel);
	Cvar_RegisterVariable (&maxclients);
	Cvar_RegisterVariable (&maxspectators);
	Cvar_RegisterVariable (&maxvip_spectators);
	Cvar_RegisterVariable (&hostname);
	Cvar_RegisterVariable (&deathmatch);
	Cvar_RegisterVariable (&spawn);
	Cvar_RegisterVariable (&watervis);
	Cvar_RegisterVariable (&serverdemo);

	Cvar_RegisterVariable (&developer);

	Cvar_RegisterVariable (&timeout);
	Cvar_RegisterVariable (&zombietime);

	Cvar_RegisterVariable (&sv_maxvelocity);
	Cvar_RegisterVariable (&sv_gravity);
	Cvar_RegisterVariable (&sv_stopspeed);
	Cvar_RegisterVariable (&sv_maxspeed);
	Cvar_RegisterVariable (&sv_spectatormaxspeed);
	Cvar_RegisterVariable (&sv_accelerate);
	Cvar_RegisterVariable (&sv_airaccelerate);
	Cvar_RegisterVariable (&sv_wateraccelerate);
	Cvar_RegisterVariable (&sv_friction);
	Cvar_RegisterVariable (&sv_bunnyspeedcap);
	Cvar_RegisterVariable (&sv_waterfriction);

	Cvar_RegisterVariable (&filterban);
	
	Cvar_RegisterVariable (&allow_download);
	Cvar_RegisterVariable (&allow_download_skins);
	Cvar_RegisterVariable (&allow_download_models);
	Cvar_RegisterVariable (&allow_download_sounds);
	Cvar_RegisterVariable (&allow_download_maps);
	Cvar_RegisterVariable (&allow_download_demos);
//bliP: init ->
	Cvar_RegisterVariable (&allow_download_pakmaps);
	Cvar_RegisterVariable (&download_map_url);

	Cvar_RegisterVariable (&sv_specprint);
	Cvar_RegisterVariable (&sv_admininfo);
	Cvar_RegisterVariable (&sv_reconnectlimit);
	Cvar_RegisterVariable (&sv_maxlogsize);
//bliP: 24/9 ->
	Cvar_RegisterVariable (&sv_logdir);
	Cvar_RegisterVariable (&sv_speedcheck);
	Cvar_RegisterVariable (&sv_unfake); // kickfake to unfake
//<-
	Cvar_RegisterVariable (&sv_kicktop);
//<-

	Cvar_RegisterVariable (&sv_highchars);

	Cvar_RegisterVariable (&sv_phs);

	Cvar_RegisterVariable (&pausable);

	Cvar_RegisterVariable (&sv_maxrate);

	Cvar_RegisterVariable (&sv_loadentfiles);
	Cvar_RegisterVariable (&sv_default_name);
	Cvar_RegisterVariable (&sv_mod_msg_file);
	Cvar_RegisterVariable (&sv_forcenick); 
	Cvar_RegisterVariable (&sv_registrationinfo);
	Cvar_RegisterVariable (&mvd_streamport);

	Cmd_AddCommand ("addip", SV_AddIP_f);
	Cmd_AddCommand ("removeip", SV_RemoveIP_f);
	Cmd_AddCommand ("listip", SV_ListIP_f);
	Cmd_AddCommand ("writeip", SV_WriteIP_f);
	Cmd_AddCommand ("vip_addip", SV_AddIPVIP_f);
	Cmd_AddCommand ("vip_removeip", SV_RemoveIPVIP_f);
	Cmd_AddCommand ("vip_listip", SV_ListIPVIP_f);
	Cmd_AddCommand ("vip_writeip", SV_WriteIPVIP_f);
	Cmd_AddCommand ("record", SV_Record_f);
	Cmd_AddCommand ("easyrecord", SV_EasyRecord_f);
	Cmd_AddCommand ("stop", SV_Stop_f);
	Cmd_AddCommand ("cancel", SV_Cancel_f);
	Cmd_AddCommand ("lastscores", SV_LastScores_f);
	Cmd_AddCommand ("dlist", SV_DemoList_f);
	Cmd_AddCommand ("dlistr", SV_DemoListRegex_f);
	Cmd_AddCommand ("dlistregex", SV_DemoListRegex_f);
	Cmd_AddCommand ("demolist", SV_DemoList_f);
	Cmd_AddCommand ("demolistr", SV_DemoListRegex_f);
	Cmd_AddCommand ("demolistregex", SV_DemoListRegex_f);
	Cmd_AddCommand ("rmdemo", SV_DemoRemove_f);
	Cmd_AddCommand ("rmdemonum", SV_DemoRemoveNum_f);
	Cmd_AddCommand ("script", SV_Script_f);
	Cmd_AddCommand ("demoInfoAdd", SV_DemoInfoAdd_f);
	Cmd_AddCommand ("demoInfoRemove", SV_DemoInfoRemove_f);
	Cmd_AddCommand ("demoInfo", SV_DemoInfo_f);


	for (i=0 ; i<MAX_MODELS ; i++)
		snprintf (localmodels[i], MODEL_NAME_LEN, "*%i", i);

	Info_SetValueForStarKey (svs.info, "*qwe_version", QWE_VERSION, MAX_SERVERINFO_STRING);
	Info_SetValueForStarKey (svs.info, "*version", va("%4.2f", QW_VERSION), MAX_SERVERINFO_STRING);
	
	// init fraglog stuff
	svs.logsequence = 1;
	svs.logtime = realtime;
	svs.log[0].data = svs.log_buf[0];
	svs.log[0].maxsize = sizeof(svs.log_buf[0]);
	svs.log[0].cursize = 0;
	svs.log[0].allowoverflow = true;
	svs.log[1].data = svs.log_buf[1];
	svs.log[1].maxsize = sizeof(svs.log_buf[1]);
	svs.log[1].cursize = 0;
	svs.log[1].allowoverflow = true;

	for (i = 0; i < MAX_DELAYED_PACKETS; i++) {
		svs.packets[i].sb.data = svs.packets[i].buf;
		svs.packets[i].sb.maxsize = sizeof(svs.packets[i].buf);
//		svs.packets_out[i].sb.data = svs.packets_out[i].buf;
//		svs.packets_out[i].sb.maxsize = sizeof(svs.packets_out[i].buf);
	}

	svs.num_packets = 0;
//	svs.num_packets_out = 0;
}


//============================================================================

/*
================
Master_Heartbeat

Send a message to the master every few minutes to
let it know we are alive, and log information
================
*/
#define	HEARTBEAT_SECONDS	300
void Master_Heartbeat (void)
{
	char		string[2048];
	int			active;
	int			i;

	if (realtime - svs.last_heartbeat < HEARTBEAT_SECONDS)
		return;		// not time to send yet

	svs.last_heartbeat = realtime;

	//
	// count active users
	//
	active = 0;
	for (i=0 ; i<MAX_CLIENTS ; i++)
		if (svs.clients[i].state == cs_connected ||
		svs.clients[i].state == cs_spawned )
			active++;

	svs.heartbeat_sequence++;
	snprintf (string, sizeof(string), "%c\n%i\n%i\n", S2M_HEARTBEAT,
		svs.heartbeat_sequence, active);


	// send to group master
	for (i=0 ; i<MAX_MASTERS ; i++)
		if (master_adr[i].port)
		{
			Con_Printf ("Sending heartbeat to %s\n", NET_AdrToString (master_adr[i]));
			NET_SendPacket (net_serversocket, strlen(string), string, master_adr[i]);
		}
}

/*
=================
Master_Shutdown

Informs all masters that this server is going down
=================
*/
void Master_Shutdown (void)
{
	char		string[2048];
	int			i;

	snprintf (string, sizeof(string), "%c\n", S2M_SHUTDOWN);

	// send to group master
	for (i=0 ; i<MAX_MASTERS ; i++)
		if (master_adr[i].port)
		{
			Con_Printf ("Sending heartbeat to %s\n", NET_AdrToString (master_adr[i]));
			NET_SendPacket (net_serversocket, strlen(string), string, master_adr[i]);
		}
}

/*
=================
SV_ExtractFromUserinfo

Pull specific info from a newly changed userinfo string
into a more C freindly form.
=================
*/
// Added by VVD {
// ktpro crash if absolute value of userinfo keys "ls" or/and "lw" is to large
void SV_SetUserInfoKeyLimit (char *key, int limit, client_t *cl, qboolean warning_msg)
{
	if (warning_msg)
		SV_ClientPrintf (cl, PRINT_HIGH, "WARNING: You can't set setinfo %s %s %i.\n",
						key, limit > 0 ? ">" : "<", limit);

	Info_SetValueForKey (cl->userinfo, key, va("%i", limit), MAX_INFO_STRING);

	MSG_WriteByte (&cl->netchan.message, svc_stufftext);
	MSG_WriteString (&cl->netchan.message, va("setinfo \"%s\" \"%i\"\n", key, limit));
}

void SV_CheckUserInfoKeyLimit (char *key, int limit, client_t *cl)
{
	char *value_c = Info_ValueForKey (cl->userinfo, key);
	int value = Q_atoi(value_c);

	if (value > limit)
		SV_SetUserInfoKeyLimit (key, limit, cl, true);
	else if (value < -limit)
		SV_SetUserInfoKeyLimit (key, -limit, cl, true);
	else if (strcmp(value_c, va("%i", value)) && *value_c)
		SV_SetUserInfoKeyLimit (key, value, cl, false);
}
// } Added by VVD

extern func_t UserInfo_Changed;

void SV_ExtractFromUserinfo (client_t *cl, qboolean namechanged)
{
	char	*val, *p, *q;
	int		i, limit;
	client_t	*client;
	int		dupc = 1;
	char	newname[80];

	if (namechanged)
	{
		// name for C code
		val = Info_ValueForKey (cl->userinfo, "name");

		// trim user name
		strlcpy (newname, val, sizeof(newname));

		for (i = 0, p = newname; *p; p++)
			if ((*p & 127) == '\\')
			{
				i = 1;
				break;
			}

		if (!i)
			for (p = newname; *p && ((*p & 127) == ' ' || *p == '\r' || *p == '\n'); p++);

		if ((p != newname && !*p) || i) {
			//white space only
			strlcpy(newname, sv_default_name.string, sizeof(newname));
			p = newname;
		}
		else
		{
			if (p != newname && *p)
			{
				for (q = newname; *p; *q++ = *p++);
				*q = 0;
			}
			for (p = newname + strlen(newname) - 1;
			     p != newname && ((*p & 127) == ' ' || *p == '\r' || *p == '\n');
			     p--);
			p[1] = 0;
		}

		if (strcmp(val, newname)) {
			Info_SetValueForKey (cl->userinfo, "name", newname, MAX_INFO_STRING);
			val = Info_ValueForKey (cl->userinfo, "name");
		}

		if (!val[0] || !strcasecmp(val, "console")) {
			Info_SetValueForKey (cl->userinfo, "name", sv_default_name.string, MAX_INFO_STRING);
			val = Info_ValueForKey (cl->userinfo, "name");
		}

		// check to see if another user by the same name exists
		while (1) {
			for (i=0, client = svs.clients ; i<MAX_CLIENTS ; i++, client++) {
				if (client->state != cs_spawned || client == cl)
					continue;
				if (!strcasecmp(client->name, val))
					break;
			}
			if (i != MAX_CLIENTS) { // dup name
				if (strlen(val) > CLIENT_NAME_LEN - 1)
					val[CLIENT_NAME_LEN - 4] = 0;
				p = val;

				if (val[0] == '(') {
					if (val[2] == ')')
						p = val + 3;
					else if (val[3] == ')')
						p = val + 4;
				}

				snprintf(newname, sizeof(newname), "(%d)%-.40s", dupc++, p);
				Info_SetValueForKey (cl->userinfo, "name", newname, MAX_INFO_STRING);
				val = Info_ValueForKey (cl->userinfo, "name");
			} else
				break;
		}
	
		if (strncmp(val, cl->name, strlen(cl->name) + 1)) {
			if (!sv.paused) {
				if (!cl->lastnametime || realtime - cl->lastnametime > 5) {
					cl->lastnamecount = 0;
					cl->lastnametime = realtime;
				} else if (cl->lastnamecount++ > 4) {
					SV_BroadcastPrintf (PRINT_HIGH, "%s was kicked for name spamming\n", cl->name);
					SV_ClientPrintf (cl, PRINT_HIGH, "You were kicked from the game for name spamming\n");
					SV_LogPlayer(cl, "name spam", 1); //bliP: player logging
					SV_DropClient (cl); 
					return;
				}
			}
				
			if (cl->state >= cs_spawned && !cl->spectator)
				SV_BroadcastPrintf (PRINT_HIGH, "%s changed name to %s\n", cl->name, val);
		}

		strlcpy (cl->name, val, CLIENT_NAME_LEN);

		if (cl->state >= cs_spawned) //bliP: player logging
			SV_LogPlayer(cl, "name change", 1);
	}

	// team
	strlcpy (cl->team, Info_ValueForKey (cl->userinfo, "team"), sizeof(cl->team));

	// rate
	if (cl->download) {
		val = Info_ValueForKey (cl->userinfo, "drate");
		if (!Q_atoi(val))
			val = Info_ValueForKey (cl->userinfo, "rate");
	} else
		val = Info_ValueForKey (cl->userinfo, "rate");
	cl->netchan.rate = 1.0 / SV_BoundRate (cl->download != NULL, Q_atoi(val));

	// message level
	val = Info_ValueForKey (cl->userinfo, "msg");
	if (strlen(val))
		cl->messagelevel = Q_atoi(val);

//bliP: spectator print ->
	val = Info_ValueForKey(cl->userinfo, "sp");
	if (strlen(val))
		cl->spec_print = Q_atoi(val);
//<-
// Added by VVD {
// ktpro crash if absolute value of userinfo keys "ls" or/and "lw" is to large
	limit = 63;
    SV_CheckUserInfoKeyLimit("lw", limit, cl);
    SV_CheckUserInfoKeyLimit("ls", limit, cl);
// } Added by VVD
}


//============================================================================

//bliP: 24/9 logdir ->
qboolean OnChange_logdir_var (cvar_t *var, char *value)
{
	if (strstr(value, ".."))
		return true;
	if (value[0])
		Sys_mkdir (value);
	return false;	
}
//<-

//bliP: admininfo ->
qboolean OnChange_admininfo_var (cvar_t *var, char *value)
{
	if (value[0])
		Info_SetValueForStarKey (svs.info, "*admin", value, MAX_SERVERINFO_STRING);
	else
		Info_RemoveKey (svs.info, "*admin");
	return false;	
}
//<-

//bliP: telnet log level ->
qboolean OnChange_telnetloglevel_var (cvar_t *var, char *value)
{
	logs[TELNET_LOG].log_level = atoi(value);
	return false;	
}
//<-
qboolean OnChange_qconsolelogsay_var (cvar_t *var, char *value)
{
	logs[CONSOLE_LOG].log_level = atoi(value);
	return false;	
}

/*
====================
SV_InitNet
====================
*/
void SetWindowText_(char*);
void SV_InitNet (void)
{
	int	p;

	sv_port = PORT_SERVER;
	telnetport = 0;

	p = COM_CheckParm ("-port");
	if (p && p + 1 < com_argc)
	{
		sv_port = atoi(com_argv[p + 1]);
		Con_Printf ("Port: %i\n", sv_port);
	}
	sv_port = NET_Init (0, sv_port, 0);

	p = COM_CheckParm ("-telnetport");
	if (p && p + 1 < com_argc)
	{
		p = atoi(com_argv[p + 1]);
		Con_Printf ("Telnet port: %i\n", p);
	}
	else
		p = 0/*sv_port*/;
	if (p)
		telnetport = NET_Init (0, 0, p);

	Netchan_Init ();
	// heartbeats will always be sent to the id master
	svs.last_heartbeat = -99999;		// send immediately
//	NET_StringToAdr ("192.246.40.70:27000", &idmaster_adr);

#if defined (_WIN32) && !defined(_CONSOLE) && defined(SERVERONLY)
	SetWindowText_(va("mvdsv:%d - QuakeWorld server", sv_port));
#endif

}


/*
====================
SV_Init
====================
*/
void SV_Init (quakeparms_t *parms)
{
	COM_InitArgv (parms->argc, parms->argv);

	if (COM_CheckParm ("-minmemory"))
		parms->memsize = MINIMUM_MEMORY;

	host_parms = *parms;

	if (parms->memsize < MINIMUM_MEMORY)
		SV_Error ("Only %4.1f megs of memory reported, can't execute game", parms->memsize / (float)0x100000);

	Memory_Init (parms->membase, parms->memsize);
	Cbuf_Init ();
	Cmd_Init ();	

	COM_Init ();
	
#ifdef USE_PR2
	PR2_Init();
#else
	PR_Init ();
#endif
	Mod_Init ();

	SV_InitNet ();

	SV_InitLocal ();

	Sys_Init ();
	Pmove_Init ();

	Demo_Init ();
	Login_Init ();

	Hunk_AllocName (0, "-HOST_HUNKLEVEL-");
	host_hunklevel = Hunk_LowMark ();

	Cbuf_InsertText ("exec server.cfg\n");

	host_initialized = true;
	
	Con_Printf ("%4.1f megabyte heap\n",parms->memsize/ (1024*1024.0));	

	Version_f();

	Con_Printf ("======== QuakeWorld Initialized ========\n");

// process command line arguments
	Cmd_StuffCmds_f ();
	Cbuf_Execute ();

	if (telnetport)
	{
		SV_Write_Log(TELNET_LOG, 1, "============================================\n");
		SV_Write_Log(TELNET_LOG, 1, va("mvdsv %s started\n", QWE_VERSION));
	}

	SV_Map(true);

// if a map wasn't specified on the command line, spawn start map
	if (sv.state == ss_dead) {
		Cmd_ExecuteString ("map start");
		SV_Map(true);
	}

	if (sv.state == ss_dead)
		SV_Error ("Couldn't spawn a server");
}


/*
============
SV_TimeOfDay
============
*/
void SV_TimeOfDay(date_t *date)
{
	struct tm *newtime;
	time_t long_time;
	
	time(&long_time);
	newtime = localtime(&long_time);

//bliP: date check ->
	if (!newtime) {
		date->day = 0;
		date->mon = 0;
		date->year = 0;
		date->hour = 0;
		date->min = 0;
		date->sec = 0;
		strlcpy(date->str, "#bad date#", sizeof(date->str));
		return;
	}
//<-

	date->day = newtime->tm_mday;
	date->mon = newtime->tm_mon;
	date->year = newtime->tm_year + 1900;
	date->hour = newtime->tm_hour;
	date->min = newtime->tm_min;
	date->sec = newtime->tm_sec;
	strftime(date->str, sizeof(date->str)-1, "%a %b %d, %H:%M:%S %Y", newtime);
}

//bliP: player logging ->
/*
============
SV_LogPlayer
============
*/
void SV_LogPlayer(client_t *cl, char *msg, int level)
{
	SV_Write_Log(PLAYER_LOG, level,
		va("%s\\%s\\%i\\%s\\%s\\%i%s\n",
			msg,
			cl->name,
			cl->userid,
			NET_BaseAdrToString(cl->netchan.remote_address),
			NET_BaseAdrToString(cl->realip),
			cl->netchan.remote_address.port,
			cl->userinfo
		)
	);
}

/*
============
SV_Write_Log
============
*/
void SV_Write_Log(int sv_log, int level, char *msg)
{
	static date_t date;
	char *log_msg, *error_msg;

	if (!(logs[sv_log].sv_logfile && *msg))
		return;

//bliP: moved telnet bit to on cvar change ->
	//if (sv_log == TELNET_LOG)
	//	logs[sv_log].log_level = Cvar_VariableValue("telnet_log_level");
//<-

	if (logs[sv_log].log_level < level)
		return;

	SV_TimeOfDay(&date);

	switch (sv_log)
	{
	case FRAG_LOG:
	case MOD_FRAG_LOG:
		log_msg = msg; // these logs aren't in com_gamedir
		error_msg = va("Can't write in %s log file: "/*%s/ */"%sN.log.\n",
				/*com_gamedir,*/ logs[sv_log].message_on,
				logs[sv_log].file_name);
		break;
	default:
		log_msg = va("[%s].[%d] %s", date.str, level, msg);
		error_msg = va("Can't write in %s log file: "/*%s/ */"%s%i.log.\n",
				/*com_gamedir,*/ logs[sv_log].message_on,
				logs[sv_log].file_name, sv_port);
	}

	if (fprintf(logs[sv_log].sv_logfile, "%s", log_msg) < 0)
	{
//bliP: Sys_Error to Con_DPrintf ->
//VVD: Con_DPrintf to Sys_Printf ->
		Sys_Printf("%s", error_msg);
//<-
		SV_Logfile(sv_log, false);
	}
	else
	{
		fflush(logs[sv_log].sv_logfile);
		if (sv_maxlogsize.value &&
		    (COM_FileLength(logs[sv_log].sv_logfile) > sv_maxlogsize.value))
		{
			SV_Logfile(sv_log, true);
		}
	}
}

/*
============
Sys_compare_by functions for sort files in list
============
*/
int Sys_compare_by_date(const void *a, const void *b)
{
	return (int)(((file_t *)a)->time - ((file_t *)b)->time);
}

int Sys_compare_by_name(const void *a, const void *b)
{
	return strncmp(((file_t *)a)->name, ((file_t *)b)->name, MAX_DEMO_NAME);
}

//bliP: plain player names ->
/*char qfont_table[256] = {
	'\0', '#', '#', '#', '#', '.', '#', '#',
	'#', 9, 10, '#', ' ', 13, '.', '.',
	'[', ']', '0', '1', '2', '3', '4', '5',
	'6', '7', '8', '9', '.', '<', '=', '>',
	' ', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
	'x', 'y', 'z', '{', '|', '}', '~', '<',

	'<', '=', '>', '#', '#', '.', '#', '#',
	'#', '#', ' ', '#', ' ', '>', '.', '.',
	'[', ']', '0', '1', '2', '3', '4', '5',
	'6', '7', '8', '9', '.', '<', '=', '>',
	' ', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
	'x', 'y', 'z', '{', '|', '}', '~', '<'
};*/

/*
==================
Q_normalizetext
returns readable extended quake names
==================
*/
char *Q_normalizetext(unsigned char *str)
{
	extern char chartbl2[];
	unsigned char	*i;

	for (i = str; *i; i++) 
		*i = chartbl2[*i];
	return str;
}

/*
==================
Q_redtext
returns extended quake names
==================
*/
char *Q_redtext(unsigned char *str)
{
	unsigned char	*i;
	for (i = str; *i; i++)
		if (*i > 32 && *i < 128)
			*i |= 128;
	return str;
}
//<-

/*
==================
Q_yelltext
returns extended quake names (yellow numbers)
==================
*/
char *Q_yelltext(unsigned char *str)
{
	unsigned char	*i;
	for (i = str; *i; i++)
	{
		if (*i >= '0' && *i <= '9')
			*i += 18 - '0';
		else if (*i > 32 && *i < 128)
			*i |= 128;
		else if (*i == 13)
			*i = ' ';
	}
	return str;
}
