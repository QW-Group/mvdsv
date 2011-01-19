/*
	cmd.c
*/

#include "qwfwd.h"

static void CL_SendConnectPacket_QW(peer_t *p) 
{
	char data[2048];
	char biguserinfo[MAX_INFO_STRING + 32];

	if (p->ps != ps_challenge)
		return;

	// Let the server know what extensions we support.
	strlcpy (biguserinfo, p->userinfo, sizeof (biguserinfo));
	snprintf(data, sizeof(data), "\xff\xff\xff\xff" "connect %i %i %i \"%s\"\n", QW_PROTOCOL_VERSION, p->qport, p->challenge, biguserinfo);

	NET_SendPacket(net_from_socket, strlen(data), data, &net_from);
}

// Responses to broadcasts, etc
static qbool CL_ConnectionlessPacket_QW (peer_t *p) 
{
	qbool need_forward = false;
	int c;
	
    MSG_BeginReading();
    MSG_ReadLong();	// Skip the -1

	c = MSG_ReadByte();

	if (MSG_BadRead())
		return need_forward;	// Runt packet

	switch(c) 
	{
		case S2C_CHALLENGE:
		{
			Sys_DPrintf("%s: challenge\n", inet_ntoa(net_from.sin_addr));
			p->challenge = atoi(MSG_ReadString());

			CL_SendConnectPacket_QW(p);

			break;
		}
		case S2C_CONNECTION:
		{
			Sys_DPrintf("%s: connection\n", inet_ntoa(net_from.sin_addr));

			if (p->ps >= ps_connected) 
			{
				Sys_DPrintf("Dup connect received. Ignored.\n");
				break;
			}

			p->ps = ps_connected;

			break;
		}
		case A2C_CLIENT_COMMAND: 
		{
			// Remote command from gui front end
			Sys_DPrintf("%s: client command\n", inet_ntoa(net_from.sin_addr));

			break;
		}
		case A2C_PRINT:		
		{
			// Print command from somewhere.
			
			#ifdef FTE_PEXT_CHUNKEDDOWNLOADS
			if (net_message.cursize > 100 && !strncmp((char *)net_message.data + 5, "\\chunk", sizeof("\\chunk")-1)) 
			{
				CL_Parse_OOB_ChunkedDownload();
				return;
			}
			#endif // FTE_PEXT_CHUNKEDDOWNLOADS

//			Sys_Printf("%s: print\n", inet_ntoa(net_from.sin_addr));
//			Sys_Printf("%s", MSG_ReadString());

			need_forward = true; // so client have chance to see what server trying to say

			break;
		}
		case svc_disconnect:
		{
			Sys_DPrintf("%s: svc_disconnect\n", inet_ntoa(net_from.sin_addr));
			break;
		}
		default:
		{
			Sys_DPrintf("CL CL_ConnectionlessPacket %s:\n%c%s\n", inet_ntoa(net_from.sin_addr), c, MSG_ReadString());
			break;
		}
	}

	return need_forward;
}

static void CL_SendConnectPacket_Q3(peer_t *p) 
{
	char tmp[128];
	char data[2048];
	byte msg_data[2048];
	char biguserinfo[MAX_INFO_STRING + 100];
	sizebuf_t msg;

	if (p->ps != ps_challenge)
		return;

	// add challenge to the temporary userinfo
	strlcpy (biguserinfo, p->userinfo, sizeof (biguserinfo));
	snprintf(tmp, sizeof(tmp), "%d", p->challenge);
	Info_SetValueForKey(biguserinfo, "challenge", tmp, sizeof(biguserinfo));
	// make string
	snprintf(data, sizeof(data), "\xff\xff\xff\xff" "connect \"%s\"", biguserinfo);	
	// init msg
	SZ_InitEx(&msg, msg_data, sizeof(msg_data), true);
	SZ_Print(&msg, data);
	// god damn compress it
	Huff_EncryptPacket(&msg, 12);

	// ok, send it!
	NET_SendPacket(net_from_socket, msg.cursize, msg.data, &net_from);
}

static qbool CL_ConnectionlessPacket_Q3 (peer_t *p) 
{
	char	*s, buf[] = "xxx.xxx.xxx.xxx:xxxxx";
	char	*c;
	qbool need_forward = false;
	
    MSG_BeginReading();
    MSG_ReadLong();	// Skip the -1

	s = MSG_ReadStringLine();
	Cmd_TokenizeString( s );
	c = Cmd_Argv(0);

	if ( developer->integer )
	{
		Sys_DPrintf ("CL packet %s: %s\n", NET_AdrToString(&net_from, buf, sizeof(buf)), s);
	}

	// challenge from the server we are connecting to
	if ( !stricmp(c, "challengeResponse") )
	{
		if ( p->ps != ps_challenge )
		{
			Sys_DPrintf( "Unwanted challenge response received.  Ignored.\n" );
		}
		else
		{
			// start sending connect requests instead of challenge request packets
			p->challenge = atoi(Cmd_Argv(1));

			// take this address as the new server address.  This allows
			// a server proxy to hand off connections to multiple servers
//			clc.serverAddress = from;

			Sys_DPrintf ("challengeResponse: %d\n", p->challenge);

			CL_SendConnectPacket_Q3( p );
		}

		return need_forward;
	}

	// server connection
	if ( !stricmp(c, "connectResponse") )
	{
		if ( p->ps >= ps_connected )
		{
			Sys_DPrintf ("Dup connect received.  Ignored.\n");
			return need_forward;
		}

		if ( p->ps != ps_challenge )
		{
			Sys_DPrintf ("connectResponse packet while not connecting.  Ignored.\n");
			return need_forward;
		}

		Sys_DPrintf ("connectResponse\n");

		// we are connected now
		p->ps = ps_connected;

// possibile to lost this message, so moved to the other place where it sended time to time
//		Netchan_OutOfBandPrint(net_socket, &p->from, "print\n" "/reconnect ASAP!\n");

		return need_forward;
	}

	// a disconnect message from the server, which will happen if the server
	// dropped the connection but it is still getting packets from us
	if ( !stricmp(c, "disconnect") )
	{
//		CL_DisconnectPacket( from );
		p->ps = ps_drop; // drop this peer
		return need_forward = true; // so client have chance to see what server trying to say
	}

	// echo request from server
	if ( !stricmp(c, "print") )
	{
		Sys_DPrintf( "%s", MSG_ReadString() );
		return need_forward = true; // so client have chance to see what server trying to say
	}

	return need_forward;
}


qbool CL_ConnectionlessPacket (peer_t *p) 
{
	if ( p->proto == pr_qw )
	{
		return CL_ConnectionlessPacket_QW( p );
	}
	else
	{
		return CL_ConnectionlessPacket_Q3( p );
	}
}
