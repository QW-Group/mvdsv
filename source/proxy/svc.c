/*
	svc.c
*/

#include "qwfwd.h"

/*
==================
SVC_DirectConnect

A connection request that did not come from the master
==================
*/

static qbool CheckChallange( int challenge );

static qbool CheckProtocol( int ver )
{
	if (ver != PROTOCOL_VERSION)
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nServer is version " QW_VERSION ".\n", A2C_PRINT);
		Sys_Printf ("* rejected connect from version %i\n", ver);
		return false;
	}

	return true;
}

qbool CheckUserinfo( char *userinfobuf, unsigned int bufsize, char *userinfo )
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
#define PRX_KEY "prx"

	char userinfo[MAX_INFO_STRING], prx[MAX_INFO_KEY], *at;
	peer_t *p = NULL;
	int qport, port;

#ifdef PROTOCOL_VERSION_FTE
	unsigned int protextsupported = 0;
#endif

	// check version/protocol
	if ( !CheckProtocol( atoi( Cmd_Argv( 1 ) ) ) )
		return; // wrong protocol number

	// get qport
	qport = atoi( Cmd_Argv( 2 ) );

	// see if the challenge is valid
	if ( !CheckChallange( atoi( Cmd_Argv( 3 ) ) ) )
		return; // wrong challange

	// and now validate userinfo
	if ( !CheckUserinfo( userinfo, sizeof( userinfo ), Cmd_Argv( 4 ) ) )
		return; // wrong userinfo

	Info_ValueForKey(userinfo, PRX_KEY, prx, sizeof(prx));
	if (!prx[0])
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\n" PRX_KEY " userinfo key is not set\n", A2C_PRINT);
		return; // no proxy set
	}

	if ((at = strchr(prx, '@')) && at[1])
	{
		Info_SetValueForStarKey(userinfo, PRX_KEY, at+1, sizeof(userinfo));
		at[0] = 0; // truncate proxy chains
	}
	else
	{
		Info_RemoveKey(userinfo, PRX_KEY);
	}

	if ((at = strchr(prx, ':')))
	{
		at[0] = 0; // truncate port from proxy name
		port = atoi(at + 1);
	}
	else
	{
		port = 27500;
	}

	if (port < 1)
	{
		Netchan_OutOfBandPrint (net_from_socket, &net_from, "%c\nport number in " PRX_KEY " userinfo key is invalid\n", A2C_PRINT);
		return; // something wrong with port
	}

#ifdef PROTOCOL_VERSION_FTE

//
// WARNING: WARNING: WARNING: using Cmd_TokenizeString() so do all Cmd_Argv() above.
//

	while( !msg_badread )
	{
		Cmd_TokenizeString( MSG_ReadStringLine() );

		switch( Q_atoi( Cmd_Argv( 0 ) ) )
		{
		case PROTOCOL_VERSION_FTE:
			protextsupported = Q_atoi( Cmd_Argv( 1 ) );
			Con_DPrintf("Client supports 0x%x fte extensions\n", protextsupported);
			break;
		}
	}

	msg_badread = false;

#endif

	// build a new connection

	// this was new peer, lets register it then
	if ((p = FWD_peer_new(prx, port, &net_from, true)))
	{
		p->qport = qport;
		strlcpy(p->userinfo, userinfo, sizeof(p->userinfo));
		Sys_Printf("peer %s:%d added\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));
	}
	else
	{
		Sys_Printf("peer %s:%d was not added\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));
		return;
	}

#ifdef PROTOCOL_VERSION_FTE
	newcl->fteprotocolextensions = protextsupported;
#endif

	Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c", S2C_CONNECTION);
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

// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

typedef struct
{
	struct sockaddr_in		adr;
	int						challenge;
	time_t					time;
} challenge_t;

static challenge_t		challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting

// see if the challenge is valid
static qbool CheckChallange( int challenge )
{
	int i;

	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		if (NET_CompareAddress(&net_from, &challenges[i].adr))
		{
			if (challenge == challenges[i].challenge)
				break;		// good

			Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c\nBad challenge.\n", A2C_PRINT);
			return false;
		}
	}

	if (i == MAX_CHALLENGES)
	{
		Netchan_OutOfBandPrint(net_from_socket, &net_from, "%c\nNo challenge for address.\n", A2C_PRINT);
		return false;
	}

	return true;
}

static void SVC_GetChallenge (void)
{
	time_t	oldestTime;
	int		i, oldest;
	char	buf[256], *over;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	for (i = 0 ; i < MAX_CHALLENGES ; i++)
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

	// send it back
	snprintf(buf, sizeof(buf), "%c%i", S2C_CHALLENGE, challenges[i].challenge);
	over = buf + strlen(buf) + 1;

// FIXME: blah...
#ifdef PROTOCOL_VERSION_FTE
	//tell the client what fte extensions we support
	if (svs.fteprotocolextensions)
	{
		int lng;

		lng = LittleLong(PROTOCOL_VERSION_FTE);
		memcpy(over, &lng, sizeof(int));
		over += 4;

		lng = LittleLong(svs.fteprotocolextensions);
		memcpy(over, &lng, sizeof(int));
		over += 4;
	}
#endif

	Netchan_OutOfBand(net_from_socket, &net_from, over-buf, (byte*) buf);
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

void SV_ConnectionlessPacket(void)
{
	char	*s;
	char	*c;

	MSG_BeginReading ();
	MSG_ReadLong ();		// skip the -1 marker

	s = MSG_ReadStringLine ();

	Cmd_TokenizeString (s);

	c = Cmd_Argv(0);

	if (!strcmp(c,"connect"))
		SVC_DirectConnect ();
	else if (!strcmp(c,"getchallenge"))
		SVC_GetChallenge();
//	else
//		Sys_Printf ("SV bad connectionless packet from %s:\n%s\n" , inet_ntoa(net_from.sin_addr), s);

	Sys_Printf ("SV connectionless packet from %s:\n%s\n", inet_ntoa(net_from.sin_addr), s);
}

