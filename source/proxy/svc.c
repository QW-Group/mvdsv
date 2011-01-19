/*
	svc.c
*/

#include "qwfwd.h"

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

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

typedef struct
{
	struct sockaddr_in		adr;
	int						challenge;
	time_t					time;
	protocol_t				proto;
} challenge_t;

static challenge_t		challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting

// find challenge for address if any
static unsigned int FindChallengeForAddr(struct sockaddr_in *addr)
{
	unsigned int i;

	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		if (NET_CompareAddress(addr, &challenges[i].adr))
		{
			break; // found
		}
	}

	return i;
}

// see if the challenge is valid
static qbool CheckChallenge( int challenge )
{
	unsigned int i = FindChallengeForAddr(&net_from);

	if (i >= MAX_CHALLENGES)
	{
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c\nNo challenge for address.\n", A2C_PRINT);
		return false;
	}

	if (challenge != challenges[i].challenge)
	{
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c\nBad challenge.\n", A2C_PRINT);
		return false;
	}

	return true; // good!
}

static void SVC_GetChallenge ( protocol_t proto )
{
	time_t	oldestTime;
	int		i, oldest;
	char	buf[256];

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for (i = 0 ; i < MAX_CHALLENGES; i++)
	{
		if (NET_CompareAddress(&net_from, &challenges[i].adr))
			break;

		if (challenges[i].time < oldestTime)
		{
			oldestTime = challenges[i].time;
			oldest = i;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		i = oldest;
		challenges[i].challenge = (rand() << 16) ^ rand();
		challenges[i].adr		= net_from;
		challenges[i].time		= time(NULL);
	}

	// overwrite proto in any case!
	challenges[i].proto	= proto;

	if ( challenges[i].proto == pr_q3 )
	{
		peer_t *p = FWD_peer_by_addr( &net_from );
		if ( p && p->ps == ps_connected )
		{
			Sys_DPrintf("challenge q3 overwrite trick!\n");
			challenges[i].challenge = p->challenge; // use challenge from q3 server since q3 packets encrypted with it...
		}
	}

	if ( developer->integer )
	{
		Sys_DPrintf("challenge %s: %s %d\n", challenges[i].proto == pr_qw ? "qw" : "q3", NET_AdrToString(&net_from, buf, sizeof(buf)), challenges[i].challenge);
	}

	// send it back
	if ( challenges[i].proto == pr_qw )
	{
		char *over;

		snprintf(buf, sizeof(buf), "%c%i", S2C_CHALLENGE, challenges[i].challenge);

		over = buf + strlen(buf) + 1;

		Netchan_OutOfBand(net_from_socket, &net_from, over-buf, (byte*) buf);
	}
	else
	{
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "challengeResponse %i", challenges[i].challenge);	
	}
}

/*
==================
SVC_DirectConnect

A connection request that did not come from the master
==================
*/

static qbool CheckProtocol( int ver, protocol_t proto )
{
	if ( proto == pr_qw )
	{
		if (ver != QW_PROTOCOL_VERSION)
		{
			Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nServer is version " QW_VERSION ".\n", A2C_PRINT);
			Sys_DPrintf ("* rejected connect from version %i\n", ver);
			return false;
		}
	}
	else
	{
#if 0 // who care which version it is?
		if (ver != 68)
		{
			Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nServer is version 68.\n", A2C_PRINT);
			Sys_DPrintf ("* rejected connect from version %i\n", ver);
			return false;
		}
#endif
	}

	return true;
}

static qbool CheckUserinfo( char *userinfobuf, unsigned int bufsize, char *userinfo )
{
	strlcpy (userinfobuf, userinfo, bufsize);

	// and now validate userinfo
	if ( !ValidateUserInfo( userinfobuf ) )
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nInvalid userinfo. Restart your qwcl\n", A2C_PRINT);
		return false;
	}

	return true;
}

static void SVC_DirectConnect (void)
{
	unsigned int i = FindChallengeForAddr(&net_from);

	char userinfo[MAX_INFO_STRING], prx[MAX_INFO_KEY], *at;
	peer_t *p = NULL;
	int qport, port, challenge;
	protocol_t proto;

	if ( i >= MAX_CHALLENGES )
	{
		// here protocol is unknown
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c\nNo challenge for address.\n", A2C_PRINT);
		return;
	}

	proto = challenges[i].proto;
	challenge = challenges[i].challenge;

	// different for qw and q3
	if ( proto == pr_qw )
	{
		// check version/protocol
		if ( !CheckProtocol( atoi( Cmd_Argv( 1 ) ), proto ) )
			return; // wrong protocol number

		// get qport
		qport = atoi( Cmd_Argv( 2 ) );

		// see if the challenge is valid
		if ( !CheckChallenge( atoi( Cmd_Argv( 3 ) ) ) )
			return; // wrong challenge

		// and now validate userinfo
		if ( !CheckUserinfo( userinfo, sizeof( userinfo ), Cmd_Argv( 4 ) ) )
			return; // wrong userinfo
	}
	else
	{
		// and now validate userinfo
		if ( !CheckUserinfo( userinfo, sizeof( userinfo ), Cmd_Argv( 1 ) ) )
			return; // wrong userinfo

		// check version/protocol
		if ( !CheckProtocol( atoi( Info_ValueForKey(userinfo, "protocol", prx, sizeof(prx)) ), proto ) )
			return; // wrong protocol number

		// get qport
		qport = atoi( Info_ValueForKey(userinfo, "qport", prx, sizeof(prx)) );

		// see if the challenge is valid
		if ( !CheckChallenge( atoi( Info_ValueForKey(userinfo, "challenge", prx, sizeof(prx)) ) ) )
			return; // wrong challenge
	}

	// check proxy is full
	if (FWD_peers_count() >= maxclients->integer)
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\n" "proxy@%s is full\n\n", A2C_PRINT, hostname->string);
		return; // no more free slots
	}

	// check prx setinfo key
	Info_ValueForKey(userinfo, QWFWD_PRX_KEY, prx, sizeof(prx));
	if (!prx[0])
	{
		if ( proto == pr_qw )
		{
			Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\n" QWFWD_PRX_KEY " userinfo key is not set\n", A2C_PRINT);
		}
		else
		{
			Netchan_OutOfBandPrint (net_from_socket, &net_from, "print\n" QWFWD_PRX_KEY " userinfo key is not set\n");
		}
		return; // no proxy set
	}

	// check chaining
	if ((at = strchr(prx, '@')) && at[1])
	{
		Info_SetValueForStarKey(userinfo, QWFWD_PRX_KEY, at+1, sizeof(userinfo));
		at[0] = 0; // truncate proxy chains
	}
	else
	{
		Info_RemoveKey(userinfo, QWFWD_PRX_KEY);
	}

	// guess port
	if ((at = strchr(prx, ':')))
	{
		at[0] = 0; // truncate port from proxy name
		port = atoi(at + 1);
	}
	else
	{
		port = ( proto == pr_qw ) ? 27500 : 27960;
	}

	if (port < 1)
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nport number in " QWFWD_PRX_KEY " userinfo key is invalid\n", A2C_PRINT);
		return; // something wrong with port
	}

	// put some identifier in userinfo so server/proxy can detect that client use qwfwd.
	Info_SetValueForStarKey(userinfo, "*qwfwd", QWFWD_VERSION_SHORT, sizeof(userinfo));

	// build a new connection

	// this was new peer, lets register it then
	if ((p = FWD_peer_new(prx, port, &net_from, userinfo, qport, proto, true)))
	{
		Sys_DPrintf("peer %s:%d added or reused\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));
	}
	else
	{
		Sys_DPrintf("peer %s:%d was not added\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));
		return;
	}

	if ( proto == pr_qw )
	{
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c", S2C_CONNECTION);
	}
	else
	{
		if ( p->ps == ps_connected )
		{
			if ( p->challenge == challenge )
			{
				// ok, we are really really ready to transfer data
				Netchan_OutOfBandPrint(net_from_socket, &net_from, "connectResponse");
			}
			else
			{
				// tell client we are ready. just type /reconnect on the console!
				Netchan_OutOfBandPrint(net_from_socket, &net_from, "print\n" "/reconnect ASAP!\n");
			}
		}
	}
}

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
//#define STATUS_SPECTATORS_AS_PLAYERS	8 //for ASE - change only frags: show as "S"
//#define STATUS_SHOWTEAMS				16

static void SVC_Status (void)
{
	sizebuf_t buf;

	static byte buf_data[MSG_BUF_SIZE]; // static  - so it not allocated each time
	static char tmp[1024]; // static too

	int top, bottom, ping, opt, connect_t;
	char *name, *frags, *skin;
	peer_t *cl;

	SZ_InitEx(&buf, buf_data, sizeof(buf_data), true);

	MSG_WriteLong(&buf, -1);	// -1 sequence means out of band
	MSG_WriteChar(&buf, A2C_PRINT);

	opt = (Cmd_Argc() > 1) ? atoi(Cmd_Argv(1)) : 0;

	if (opt == STATUS_OLDSTYLE || (opt & STATUS_SERVERINFO))
	{
		snprintf(tmp, sizeof(tmp), "%s\n", ps.info);
		SZ_Print(&buf, tmp);
	}

	if (opt == STATUS_OLDSTYLE || (opt & (STATUS_PLAYERS | STATUS_SPECTATORS)))
	{
		for (cl = peers; cl; cl = cl->next)
		{
			top    = 0;//Q_atoi(Info_Get (&cl->_userinfo_ctx_, "topcolor"));
			bottom = 0;//Q_atoi(Info_Get (&cl->_userinfo_ctx_, "bottomcolor"));
			top    = (top    < 0) ? 0 : ((top    > 13) ? 13 : top);
			bottom = (bottom < 0) ? 0 : ((bottom > 13) ? 13 : bottom);
			ping   = 666; //SV_CalcPing (cl);
			name   = cl->name;
			skin   = "";
			frags  = "0";
			connect_t = (int)(time(NULL) - cl->connect)/60; // not like it proper...

			snprintf(tmp, sizeof(tmp), "%i %s %i %i \"%s\" \"%s\" %i %i\n", cl->userid, frags, connect_t, ping, name, skin, top, bottom);

			SZ_Print(&buf, tmp);
		}
	}

	if (buf.overflowed)
		return; // overflowed

	// send the datagram
	NET_SendPacket(net_from_socket, buf.cursize, buf.data, &net_from);
}

/*
================
SVC_Ping

Just responds with an acknowledgement
================
*/
static void SVC_Ping (void)
{
	char data = A2A_ACK;

	NET_SendPacket(net_from_socket, 1, &data, &net_from);
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

qbool SV_ConnectionlessPacket(void)
{
	qbool need_forward = false;
	char	*s;
	char	*c;

	MSG_BeginReading ();

	if (QRY_IsMasterReply())
	{
		SVC_QRY_ParseMasterReply();
	}
	else
	{
		MSG_ReadLong ();		// skip the -1 marker

		s = MSG_ReadString ();//MSG_ReadStringLine ();

		// check for possibile huffmen compression for q3, zzz...
		if ( s[0] == 'c' && !strncmp(s, "connect ", sizeof("connect ")-1) )
		{
			unsigned int i = FindChallengeForAddr(&net_from);

			if ( i < MAX_CHALLENGES && challenges[i].proto == pr_q3 )
			{
				Huff_DecryptPacket(&net_message, 12);
				MSG_BeginReading();
				MSG_ReadLong();
				s = MSG_ReadString ();//MSG_ReadStringLine ();
			}
		}

//		Sys_Printf ("SV connectionless packet from %s:\n%s\n", inet_ntoa(net_from.sin_addr), s);

		Cmd_TokenizeString (s);

		c = Cmd_Argv(0);

		if (!strcmp(c, "ping") || ( c[0] == A2A_PING && (c[1] == 0 || c[1] == '\n') ) )
			SVC_Ping();
		else if (!strcmp(c, "pingstatus"))
			SVC_QRY_PingStatus();
		else if (!strcmp(c,"connect"))
			SVC_DirectConnect();
		else if (!strcmp(c,"getchallenge"))
			SVC_GetChallenge( !strcmp(s,"getchallenge\n") ? pr_qw : pr_q3 );
		else if (!strcmp(c,"status"))
			SVC_Status();
		else if (!strcmp(c,"rcon"))
			need_forward = true; // we do not have own rcon command, we forward it to the server...
	}

	return need_forward;
}

