/*
	cmd.c
*/

#include "qwfwd.h"

static void CL_SendConnectPacket(
#ifdef PROTOCOL_VERSION_FTE
						peer_t *p, unsigned int ftepext
#else
						peer_t *p
#endif
								) 
{
	char data[2048];
	char biguserinfo[MAX_INFO_STRING + 32];

	if (p->ps != ps_challenge)
		return;

	#ifdef PROTOCOL_VERSION_FTE
	cls.fteprotocolextensions  = (ftepext & CL_SupportedFTEExtensions());
	#endif // PROTOCOL_VERSION_FTE

	// Let the server know what extensions we support.
	strlcpy (biguserinfo, p->userinfo, sizeof (biguserinfo));

	snprintf(data, sizeof(data), "\xff\xff\xff\xff" "connect %i %i %i \"%s\"\n", PROTOCOL_VERSION, p->qport, p->challenge, biguserinfo);

	#ifdef PROTOCOL_VERSION_FTE
	if (cls.fteprotocolextensions) 
	{
		char tmp[128];
		snprintf(tmp, sizeof(tmp), "0x%x 0x%x\n", PROTOCOL_VERSION_FTE, cls.fteprotocolextensions);
		strlcat(data, tmp, sizeof(data));
	}
	#endif // PROTOCOL_VERSION_FTE 

	NET_SendPacket(net_from_socket, strlen(data), data, &net_from);
}

// Responses to broadcasts, etc
void CL_ConnectionlessPacket (peer_t *p) 
{
	int c;
	
	#ifdef PROTOCOL_VERSION_FTE
	unsigned int pext = 0;
	#endif

    MSG_BeginReading();
    MSG_ReadLong();	// Skip the -1

	c = MSG_ReadByte();

	if (MSG_BadRead())
		return;	// Runt packet

	switch(c) 
	{
		case S2C_CHALLENGE:
		{
			Sys_Printf("%s: challenge\n", inet_ntoa(net_from.sin_addr));
			p->challenge = atoi(MSG_ReadString());

			#ifdef PROTOCOL_VERSION_FTE
			for(;;)
			{
				c = MSG_ReadLong();
				if (MSG_BadRead())
					break;
				if (c == PROTOCOL_VERSION_FTE)
					pext = MSG_ReadLong();
				else
					MSG_ReadLong();
			}

			CL_SendConnectPacket(p, pext);
			#else
			CL_SendConnectPacket(p);
			#endif // PROTOCOL_VERSION_FTE
			
			break;
		}
		case S2C_CONNECTION:
		{
			Sys_Printf("%s: connection\n", inet_ntoa(net_from.sin_addr));

			if (p->ps >= ps_connected) 
			{
				Sys_Printf("Dup connect received. Ignored.\n");
				break;
			}

			p->ps = ps_connected;

			break;
		}
		case A2C_CLIENT_COMMAND: 
		{
			// Remote command from gui front end
			Sys_Printf("%s: client command\n", inet_ntoa(net_from.sin_addr));

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

			Sys_Printf("%s: print\n", inet_ntoa(net_from.sin_addr));
			Sys_Printf("%s", MSG_ReadString());
			break;
		}
		case svc_disconnect:
		{
			Sys_Printf("%s: svc_disconnect\n", inet_ntoa(net_from.sin_addr));
			break;
		}
		default:
		{
			Sys_Printf("CL CL_ConnectionlessPacket %s:\n%s\n", inet_ntoa(net_from.sin_addr), MSG_ReadString());
			break;
		}
	}
}
