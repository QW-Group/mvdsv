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

	$Id: net.c,v 1.11 2006/04/28 17:12:44 vvd0 Exp $
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
qbool		telnet_connected;

byte		net_message_buffer[MSG_BUF_SIZE];

#ifdef _WIN32
WSADATA		winsockdata;
#endif
//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr_qstorage *s)
{
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

qbool NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.ip.ip[0] == b.ip.ip[0] && a.ip.ip[1] == b.ip.ip[1] && a.ip.ip[2] == b.ip.ip[2] && a.ip.ip[3] == b.ip.ip[3])
		return true;
	return false;
}

qbool NET_CompareAdr (netadr_t a, netadr_t b)
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
qbool NET_StringToSockaddr (char *s, struct sockaddr_qstorage *sadr)
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
				((struct sockaddr_in *)sadr)->sin_port = htons((short)Q_atoi(colon+1));
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

qbool NET_StringToAdr (char *s, netadr_t *a)
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

qbool NET_GetPacket (void)
{
	int ret;
	struct sockaddr_qstorage from;
	socklen_t fromlen = sizeof(from);

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
	int	i;
	struct sockaddr_in address;
	unsigned long _true = true;

	if ((net_socket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
		Sys_Error ("UDP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));

#ifndef _WIN32
	if (setsockopt (net_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&_true, sizeof(_true)))
		Sys_Error ("UDP_OpenSocket: setsockopt SO_REUSEADDR: (%i): %s\n", qerrno, strerror(qerrno));
#endif

	if (ioctlsocket (net_socket, FIONBIO, &_true) == -1)
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
		port = address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if (COM_CheckParm("-port"))
	{
		if( bind (net_socket, (void *)&address, sizeof(address)) == -1)
		{
			closesocket(net_socket);
			Sys_Error ("UDP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
		}
	}
	else
	{
		// try any port
		for (i = 0; i < 100; i++, port++)
		{
			address.sin_port = htons((short)port);
			if (bind (net_socket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100) {
			closesocket(net_socket);
			Sys_Error ("UDP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
		}
	}

	return ntohs(address.sin_port);
}

int TCP_OpenSocket (int port, int udp_port)
{
	int	i;
	struct sockaddr_in address;
	unsigned long _true = true;

	if ((net_telnetsocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		Sys_Error ("TCP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));

#ifndef _WIN32
	if (setsockopt (net_telnetsocket, SOL_SOCKET, SO_REUSEADDR, (void*)&_true, sizeof(_true)))
		Sys_Error ("TCP_OpenSocket: socket: (%i): %s\n", qerrno, strerror(qerrno));
#endif

	if (ioctlsocket (net_telnetsocket, FIONBIO, &_true) == -1)
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

	if (port == PORT_ANY)
		port = address.sin_port = 0;
	else
		address.sin_port = htons((short)port);

	if (COM_CheckParm("-telnetport"))
	{
		if( bind (net_telnetsocket, (void *)&address, sizeof(address)) == -1)
		{
			closesocket(net_telnetsocket);
			Sys_Error ("TCP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
		}
	}
#ifdef ENABLE_TELNET_BY_DEFAULT
	else // disconnect: is it safe for telnet port?
	{
		// try any port
		for (port = udp_port, i = 0; i < 100; i++, port++)
		{
			address.sin_port = htons((short)port);
			if (bind (net_telnetsocket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100) {
			closesocket(net_telnetsocket);
			Sys_Error ("TCP_OpenSocket: bind: (%i): %s\n", qerrno, strerror(qerrno));
		}
	}
#endif //ENABLE_TELNET_BY_DEFAULT

	if (listen(net_telnetsocket, 1)) {
		closesocket(net_telnetsocket);
		Sys_Error ("TCP_OpenSocket: listen: (%i): %s\n", qerrno, strerror(qerrno));
	}

	return ntohs(address.sin_port);
}

void NET_GetLocalAddress (netadr_t *out)
{
	struct sockaddr_qstorage address;
	qbool notvalid = false;
	netadr_t adr = {0};
	char buff[512];
	socklen_t namelen = sizeof(address);

	strlcpy(buff, "localhost", sizeof(buff));
	gethostname(buff, 512);
	buff[512-1] = 0;

	if (!NET_StringToAdr (buff, &adr)) //urm
		NET_StringToAdr ("127.0.0.1", &adr);

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
void NET_Init (int *serverport, int *telnetport)
{
	// open the single socket to be used for all communications
	
#ifdef _WIN32
	// Why we need version 2.0? Trying 1.1...  Work with 1.0 too.
	if (WSAStartup (MAKEWORD(1, 0), &winsockdata))
		Sys_Error ("WinSock initialization failed.");

	Sys_Printf("WinSock version is: %d.%d\n",
				LOBYTE(winsockdata.wVersion), HIBYTE(winsockdata.wVersion));
#endif

	*serverport = UDP_OpenSocket (*serverport);

	if (*telnetport)
	{
		*telnetport = TCP_OpenSocket (*telnetport, *serverport);
		Con_Printf("TCP Initialized\n");
	}

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
