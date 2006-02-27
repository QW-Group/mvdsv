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

	$Id: net_wins.c,v 1.11 2006/02/27 16:32:37 disconn3ct Exp $
*/
// net_wins.c

#include "qwsvdef.h"
#include "winquake.h"

netadr_t	net_local_adr;

netadr_t	net_from;
sizebuf_t	net_message;
int		net_socket;
int		net_telnetsocket;
int		sv_port;
int		telnetport;
int		telnet_iosock;
int		telnet_connected;

byte		net_message_buffer[MSG_BUF_SIZE];

#ifdef _WIN32
WSADATA		winsockdata;
#endif
//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr_qstorage *s)
{
/*	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
*/
	memset (s, 0, sizeof(struct sockaddr_in));
	((struct sockaddr_in*)s)->sin_family = AF_INET;

	*(int *)&((struct sockaddr_in*)s)->sin_addr = *(int *)&a->ip.ip;
	((struct sockaddr_in*)s)->sin_port = a->port;
}

void SockadrToNetadr (struct sockaddr_qstorage *s, netadr_t *a)
{
	a->type = NA_IP;
	*(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
	a->port = ((struct sockaddr_in *)s)->sin_port;
	return;
}

qboolean NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.ip.ip[0] == b.ip.ip[0] && a.ip.ip[1] == b.ip.ip[1] && a.ip.ip[2] == b.ip.ip[2] && a.ip.ip[3] == b.ip.ip[3])
		return true;
	return false;
}

qboolean NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip.ip[0] == b.ip.ip[0] && a.ip.ip[1] == b.ip.ip[1] && a.ip.ip[2] == b.ip.ip[2] && a.ip.ip[3] == b.ip.ip[3] && a.port == b.port)
		return true;
	return false;
}

char *NET_AdrToString (netadr_t a)
{
	static	char	s[64];

	snprintf (s, sizeof(s), "%i.%i.%i.%i:%i", a.ip.ip[0], a.ip.ip[1], a.ip.ip[2], a.ip.ip[3], ntohs(a.port));

	return s;
}

char *NET_BaseAdrToString (netadr_t a)
{
	static	char	s[64];

	snprintf (s, sizeof(s), "%i.%i.%i.%i", a.ip.ip[0], a.ip.ip[1], a.ip.ip[2], a.ip.ip[3]);

	return s;
}

/*
=============
NET_StringToAdr

idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
qboolean NET_StringToSockaddr (char *s, struct sockaddr_qstorage *sadr)
{
	struct hostent	*h;
	char	*colon;
	char	copy[128];

	memset (sadr, 0, sizeof(*sadr));

	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;

		((struct sockaddr_in *)sadr)->sin_port = 0;

		strlcpy (copy, s, sizeof(copy));
		// strip off a trailing :port if present
		for (colon = copy ; *colon ; colon++)
			if (*colon == ':')
			{
				*colon = 0;
				((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));
			}

		//this is the wrong way to test. a server name may start with a number.
		if (copy[0] >= '0' && copy[0] <= '9')
		{
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(copy);
		}
		else
		{
			if (! (h = gethostbyname(copy)) )
				return 0;
			if (h->h_addrtype != AF_INET)
				return 0;
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
		}
	}

	return true;
}

qboolean NET_StringToAdr (char *s, netadr_t *a)
{
	struct sockaddr_qstorage sadr;

	if (!strcmp (s, "internalserver"))
	{
		memset (a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
		return true;
	}

	if (!NET_StringToSockaddr (s, &sadr))
		return false;

	SockadrToNetadr (&sadr, a);

	return true;
}


//=============================================================================

qboolean NET_GetPacket (void)
{
	int ret;
	struct sockaddr_qstorage from;
	int fromlen;


	fromlen = sizeof(from);
	ret = recvfrom (net_socket, (char *)net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	SockadrToNetadr (&from, &net_from);
	if (ret == -1)
	{
		if (qerrno == EWOULDBLOCK)
			return false;
		if (qerrno == EMSGSIZE)
		{
			Con_Printf ("Warning:  Oversize packet from %s\n", NET_AdrToString (net_from));
			return false;
		}
		if (qerrno == 10054)
		{
			Con_DPrintf ("NET_GetPacket: Error 10054 from %s\n", NET_AdrToString (net_from));
			return false;
		}

		Sys_Error ("NET_GetPacket: recvfrom: (%i): %s\n", qerrno, strerror(qerrno));
	}

	net_message.cursize = ret;
	if (ret == sizeof(net_message_buffer) )
	{
		Con_Printf ("Warning:  Oversize packet from %s\n", NET_AdrToString (net_from));
		return false;
	}

	return ret;
}

//=============================================================================

void NET_SendPacket (int length, void *data, netadr_t to)
{
	struct sockaddr_qstorage addr;

	NetadrToSockadr (&to, &addr);

	if (-1 == sendto (net_socket, data, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
	{
		if (qerrno == EWOULDBLOCK)
			return;
		if (qerrno == ECONNREFUSED)
			return;
		Sys_Printf ("NET_SendPacket: sendto: (%i): %s\n", qerrno, strerror(qerrno));
	}
}

//=============================================================================

int UDP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Sys_Error ("UDP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));
		return INVALID_SOCKET;
	}
#if 0
// disconnect: we need it?
	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(_true)))
		Sys_Error ("UDP_OpenSocket: setsockopt SO_REUSEADDR: (%i): %s\n", qerrno, strerror(qerrno));
#endif
	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO: (%i): %s\n", qerrno, strerror(qerrno));

	address.sin_family = AF_INET;
	//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ip")) != 0 && i < com_argc)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		Con_Printf("Binding to IP Interface Address of %s\n",
		           inet_ntoa(address.sin_addr));
	}
	else
		address.sin_addr.s_addr = INADDR_ANY;

	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if (COM_CheckParm("-port"))
	{
		if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
		{
			Sys_Error ("UDP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
			closesocket(newsocket);
			return INVALID_SOCKET;
		}
	}
	else
	{
		// try any port
		int i;

		for (i = 0; i < 100; i++, (port)++)
		{
			address.sin_port = htons((short)port);
			if( bind (newsocket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100) {
			Sys_Error ("UDP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
			closesocket(newsocket);
			return INVALID_SOCKET;
		}
	}

	return newsocket;
}

int TCP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		Sys_Error ("TCP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));
		return INVALID_SOCKET;
	}

	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEADDR, (char*)&_true, sizeof(_true)))
		Sys_Error ("TCP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("TCP_OpenSocket: ioctl FIONBIO: (%i): %s\n", qerrno, strerror(qerrno));

	address.sin_family = AF_INET;
	//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ipt")) && i + 1 < com_argc)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i + 1]);
		Con_Printf("Binding telnet service to IP Interface Address of %s\n",
		           inet_ntoa(address.sin_addr));
	}
	else
		address.sin_addr.s_addr = INADDR_ANY;

	address.sin_port = htons((short)port);

	if (COM_CheckParm("-telnetport"))
	{
		if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
		{
			Sys_Error ("TCP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
			closesocket(newsocket);
			return INVALID_SOCKET;
		}
	}
	else // disconnect: is it safe for telnet port?
	{
		// try any port
		int i;

		for (i = 0; i < 100; i++, (port)++)
		{
			address.sin_port = htons((short)port);
			if( bind (newsocket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100) {
			Sys_Error ("TCP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
			closesocket(newsocket);
			return INVALID_SOCKET;
		}
	}

	if (listen(newsocket, 1) == INVALID_SOCKET) {
		Sys_Error ("TCP_OpenSocket: listen: (%i): %s\n", qerrno, strerror(qerrno));
		closesocket(newsocket);
		return INVALID_SOCKET;
	}

	return newsocket;
}

void NET_GetLocalAddress (netadr_t *out)
{
	struct sockaddr_qstorage address;
	qboolean notvalid = false;
	netadr_t adr = {0};
	char buff[512];
	int namelen;

	strcpy(buff, "localhost");
	gethostname(buff, 512);
	buff[512-1] = 0;

	if (!NET_StringToAdr (buff, &adr)) //urm
		NET_StringToAdr ("127.0.0.1", &adr);


	namelen = sizeof(address);
	if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == -1)
	{
		notvalid = true;
		NET_StringToSockaddr("0.0.0.0", (struct sockaddr_qstorage *)&address);
		Sys_Error ("NET_Init: getsockname: (%i): %s\n", qerrno, strerror(qerrno));
	}
	
	SockadrToNetadr(&address, out);
	if (!*(int*)out->ip.ip) //socket was set to auto
	{
		//change it to what the machine says it is, rather than the socket.
		*(int *)out->ip.ip = *(int *)adr.ip.ip;
	}

	if (notvalid)
		Con_Printf("Couldn't detect local ip\n");
	else
		Con_Printf("IP address %s\n", NET_AdrToString(*out));
}

/*
====================
NET_Init
====================
*/
int NET_Init (int serverport, int telnetport)
{
	// open the single socket to be used for all communications
	if (telnetport)
	{
		net_telnetsocket = TCP_OpenSocket (telnetport);
		Con_Printf("TCP Initialized\n");
		return telnetport;
	}
	
#ifdef _WIN32
	// Why we need version 2.0?
	if (WSAStartup (MAKEWORD(2, 0), &winsockdata))
		Sys_Error ("Winsock initialization failed.");
#endif

	net_socket = UDP_OpenSocket (serverport);

#if 0//ndef SERVERONLY

	{
		static DWORD id;
		CreateThread( NULL, 0, NET_SendTo_, NULL, 0, &id);
	}
#endif
	// init the message buffer
	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	// determine my name & address
	NET_GetLocalAddress (&net_local_adr);

	Con_Printf("UDP Initialized\n");

	return serverport;
}

/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown (void)
{
	if (net_socket)
		closesocket (net_socket);
	if (telnetport)
	{
		if (telnet_connected)
			closesocket(telnet_iosock);
		closesocket (net_telnetsocket);
	}

#ifdef _WIN32
	WSACleanup ();
#endif
}

