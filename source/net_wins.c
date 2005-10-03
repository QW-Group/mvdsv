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

	$Id: net_wins.c,v 1.5 2005/10/03 21:23:29 disconn3ct Exp $
*/
// net_wins.c

#include "quakedef.h"
#include "winquake.h"

netadr_t	net_local_adr;

netadr_t	net_from;
sizebuf_t	net_message;
int			net_clientsocket;
int			net_serversocket;
int			net_telnetsocket;
int			sv_port;
int			telnetport;
int			telnet_iosock;
int			telnet_connected;

byte		net_message_buffer[MSG_BUF_SIZE];

WSADATA		winsockdata;

// Tonik -->
#define	 PORT_LOOPBACK 65535
int			loop_c2s_messageLength;
char		loop_c2s_message[MSG_BUF_SIZE];
int			loop_s2c_messageLength;
char		loop_s2c_message[MSG_BUF_SIZE];
// <-- Tonik

//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

void SockadrToNetadr (struct sockaddr_qstorage *s, netadr_t *a)
{
	a->type = NA_IP;
	*(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
	a->port = ((struct sockaddr_in *)s)->sin_port;
	return;
}

qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3])
		return true;
	return false;
}

qboolean	NET_CompareAdr (netadr_t a, netadr_t b)
{
	if (a.ip[0] == b.ip[0] && a.ip[1] == b.ip[1] && a.ip[2] == b.ip[2] && a.ip[3] == b.ip[3] && a.port == b.port)
		return true;
	return false;
}

char	*NET_AdrToString (netadr_t a)
{
	static	char	s[64];


	snprintf (s, sizeof(s), "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], ntohs(a.port));

	return s;
}

char	*NET_BaseAdrToString (netadr_t a)
{
	static	char	s[64];
	
	snprintf (s, sizeof(s), "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3]);

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
qboolean	NET_StringToSockaddr (char *s, struct sockaddr_qstorage *sadr)
{
	struct hostent	*h;
	char	*colon;
	char	copy[128];

	memset (sadr, 0, sizeof(*sadr));

	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;

		((struct sockaddr_in *)sadr)->sin_port = 0;

		strcpy (copy, s);
		// strip off a trailing :port if present
		for (colon = copy ; *colon ; colon++)
			if (*colon == ':')
			{
				*colon = 0;
				((struct sockaddr_in *)sadr)->sin_port = htons((short)atoi(colon+1));
			}

		if (copy[0] >= '0' && copy[0] <= '9')	//this is the wrong way to test. a server name may start with a number.
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

qboolean	NET_StringToAdr (char *s, netadr_t *a)
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

qboolean NET_GetPacket (int net_socket)
{
	static int	ret;
	struct sockaddr_qstorage from;
	static int	fromlen;
//	unsigned long _true = 1;

// Tonik -->
	if (loop_c2s_messageLength > 0)
	{
/*		switch (net_socket)
		{
		case net_serversocket:
//			Con_DPrintf ("NET_GetPacket: c2s\n");
		case net_clientsocket:
		}
*/
		memcpy (net_message_buffer, loop_c2s_message, loop_c2s_messageLength);
		net_message.cursize = loop_c2s_messageLength;
		loop_c2s_messageLength = 0;
		memset (&from, 0, sizeof(from));
		((struct sockaddr_in *)&from)->sin_port = PORT_LOOPBACK;
		SockadrToNetadr (&from, &net_from);
		return net_message.cursize;
	}
// <-- Tonik

	fromlen = sizeof(from);
	ret = recvfrom (net_socket, (char *)net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	SockadrToNetadr (&from, &net_from);

	if (ret == -1)
	{
		int errno = WSAGetLastError();

		if (errno == WSAEWOULDBLOCK)
			return false;
		if (errno == WSAEMSGSIZE) {
			Con_Printf ("Warning:  Oversize packet from %s\n",
				NET_AdrToString (net_from));
			return false;
		}
		if (errno == 10054) {
			Con_DPrintf ("NET_GetPacket: Error 10054 from %s\n", NET_AdrToString (net_from));
			return false;
		}

		Sys_Error ("NET_GetPacket: %s", strerror(errno));
	}

	net_message.cursize = ret;
	if (ret == sizeof(net_message_buffer) )
	{
		Con_Printf ("Oversize packet from %s\n", NET_AdrToString (net_from));
		return false;
	}

	return ret;
}

//=============================================================================

void NET_SendPacket (int net_socket, int length, void *data, netadr_t to)
{
	static struct sockaddr_in	addr;

// Tonik -->
	if (*(int *)&to.ip == 0 && to.port == 65535)	// Loopback
	{
		if (net_socket == net_clientsocket)
		{
//			if (loop_c2s_messageLength)
//				Con_Printf ("Warning: NET_SendPacket: loop_c2s: NET_SendPacket without NET_GetPacket\n");
			memcpy (loop_c2s_message, data, length);
			loop_c2s_messageLength = length;
			return;
		}
		else if (net_socket == net_serversocket)
		{
//			if (loop_s2c_messageLength)
//				Con_Printf ("Warning: NET_SendPacket: loop_s2c: NET_SendPacket without NET_GetPacket\n");
			memcpy (loop_s2c_message, data, length);
			loop_s2c_messageLength = length;
			return;
		}
		Sys_Error("NET_SendPacket: loopback: unknown socket");
		return;
	}
// <-- Tonik
	NetadrToSockadr (&to, &addr);

//#ifdef SERVERONLY
	if (-1 == sendto (net_socket, data, length, 0, (struct sockaddr *)&addr, sizeof(addr)))
	{
		int err = WSAGetLastError();

// wouldblock is silent
        if (err == WSAEWOULDBLOCK)
	        return;

#ifndef SERVERONLY
		if (err == WSAEADDRNOTAVAIL)
			Con_DPrintf("NET_SendPacket Warning: %i\n", err);
		else
#endif
			Con_Printf ("NET_SendPacket ERROR: %i\n", errno);
	}
}

//=============================================================================

int UDP_OpenSocket (int *port/*, qboolean crash*/)
{
	int newsocket;
	struct sockaddr_in address;
	unsigned long _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		Sys_Error ("UDP_OpenSocket: socket: %s", strerror(errno));

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO: %s", strerror(errno));

	address.sin_family = AF_INET;
//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ip")) != 0 && i < com_argc) {
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		Con_Printf("Binding to IP Interface Address of %s\n",
				inet_ntoa(address.sin_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;

	if (*port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)*port);

	if (COM_CheckParm("-port")) {
		if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
		{
			Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));
		}
	} else {
		// try any port
		int i;

		for (i = 0; i < 100; i++, (*port)++) {
			address.sin_port = htons((short)*port);
			if( bind (newsocket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100)
			Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));
	}

	return newsocket;
}

int TCP_OpenSocket (int *port)
{
	int newsocket;
	struct sockaddr_in address;
	unsigned long _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		Sys_Error ("TCP_OpenSocket: socket: %s", strerror(errno));

	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEADDR, (char*)&_true, sizeof(_true)))
		Sys_Error ("TCP_OpenSocket: socket: %s", strerror(errno));

	if (ioctlsocket (newsocket, FIONBIO, &_true) == -1)
		Sys_Error ("TCP_OpenSocket: ioctl FIONBIO: %s", strerror(errno));

	address.sin_family = AF_INET;
//ZOID -- check for interface binding option
	if ((i = COM_CheckParm("-ipt")) && i + 1 < com_argc) {
		address.sin_addr.s_addr = inet_addr(com_argv[i + 1]);
		Con_Printf("Binding telnet service to IP Interface Address of %s\n",
				inet_ntoa(address.sin_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;

	address.sin_port = htons((short)*port);

	if (COM_CheckParm("-telnetport")) {
		if( bind (newsocket, (void *)&address, sizeof(address)) == -1)
		{
			Sys_Error ("TCP_OpenSocket: bind: %s", strerror(errno));
		}
	} else {
		// try any port
		int i;

		for (i = 0; i < 100; i++, (*port)++) {
			address.sin_port = htons((short)*port);
			if( bind (newsocket, (void *)&address, sizeof(address)) != -1)
				break;
		}
		if (i == 100)
			Sys_Error ("TCP_OpenSocket: bind: %s", strerror(errno));
	}

	if (listen (newsocket, 1))
		Sys_Error ("TCP_OpenSocket: listen: %s", strerror(errno));

	return newsocket;
}

void NET_GetLocalAddress (int net_socket)	// FIXME
{
	char	buff[512];
	struct sockaddr_in	address;
	int		namelen;

	gethostname(buff, 512);
	buff[512-1] = 0;

	NET_StringToAdr (buff, &net_local_adr);

	namelen = sizeof(address);
	if (getsockname (net_socket, (struct sockaddr *)&address, &namelen) == -1)
		Sys_Error ("NET_Init: getsockname:", strerror(errno));
	net_local_adr.port = address.sin_port;

	Con_Printf("IP address %s\n", NET_AdrToString (net_local_adr) );
}

/*
====================
NET_Init
====================
*/
int __serverport;	// so we can open it later
int NET_Init (int clientport, int serverport, int telnetport)
{
	// 
	// open the single socket to be used for all communications
	//
	if (telnetport)
	{
		net_telnetsocket = TCP_OpenSocket (&telnetport);
		Con_Printf("TCP Initialized\n");
		return telnetport;
	}
	// Why we need version 2.0?
	if (WSAStartup (MAKEWORD(2, 0), &winsockdata))
		Sys_Error ("Winsock initialization failed.");
	if (clientport)
		net_clientsocket = UDP_OpenSocket (&clientport/*, true*/);
	else
		net_serversocket = UDP_OpenSocket (&serverport/*, false*/);

#if 0//ndef SERVERONLY
	{
		static DWORD id;
		CreateThread( NULL, 0, NET_SendTo_, NULL, 0, &id);
	}
#endif
	//
	// init the message buffer
	//
	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	//
	// determine my name & address
	//
	if (clientport)
		NET_GetLocalAddress (net_clientsocket);
	else
		NET_GetLocalAddress (net_serversocket);

	Con_Printf("UDP Initialized\n");

	if (clientport)
		return clientport;
	return serverport;
}

/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
	if (net_clientsocket)
		closesocket (net_clientsocket);
	if (net_serversocket)
		closesocket (net_serversocket);
	if (telnetport)
	{
		if (telnet_connected)
			closesocket(telnet_iosock);
		closesocket (net_telnetsocket);
	}
	WSACleanup ();
}

