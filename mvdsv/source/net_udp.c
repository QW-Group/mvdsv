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
// net_main.c

#include "quakedef.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <errno.h>

#if defined(sun)
#include <unistd.h>
#endif

#ifdef sun
#include <sys/filio.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

netadr_t	net_local_adr;

netadr_t	net_from;
sizebuf_t	net_message;
int		net_send_socket;	// blocking, for sends
int		net_serversocket;	// non blocking, for receives
int		net_telnetsocket;
int		sv_port;
int		telnetport;
int		telnet_iosock;
int		telnet_connected;

byte		net_message_buffer[MSG_BUF_SIZE];

int gethostname (char *, int);
int close (int);

//=============================================================================

void NetadrToSockadr (netadr_t *a, struct sockaddr_in *s)
{
	memset (s, 0, sizeof(*s));
	s->sin_family = AF_INET;

	*(int *)&s->sin_addr = *(int *)&a->ip;
	s->sin_port = a->port;
}

void SockadrToNetadr (struct sockaddr_in *s, netadr_t *a)
{
	*(int *)&a->ip = *(int *)&s->sin_addr;
	a->port = s->sin_port;
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
qboolean	NET_StringToAdr (char *s, netadr_t *a)
{
	struct hostent	*h;
	struct sockaddr_in sadr;
	char	*colon;
	char	copy[128];
	
	
	memset (&sadr, 0, sizeof(sadr));
	sadr.sin_family = AF_INET;
	
	sadr.sin_port = 0;

	strlcpy (copy, s, sizeof(copy));
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
		if (*colon == ':')
		{
			*colon = 0;
			sadr.sin_port = htons(atoi(colon+1));	
		}
	
	if (copy[0] >= '0' && copy[0] <= '9')
	{
		*(int *)&sadr.sin_addr = inet_addr(copy);
	}
	else
	{
		if (! (h = gethostbyname(copy)) )
			return 0;
		*(int *)&sadr.sin_addr = *(int *)h->h_addr_list[0];
	}
	
	SockadrToNetadr (&sadr, a);

	return true;
}


//=============================================================================

qboolean NET_GetPacket (int dummy)
{
	static int		ret;
	struct sockaddr_in	from;
	static int		fromlen;

	fromlen = sizeof(from);
	ret = recvfrom (net_serversocket, net_message_buffer, sizeof(net_message_buffer), 0, (struct sockaddr *)&from, &fromlen);
	if (ret == -1) {
		if (errno == EWOULDBLOCK)
			return false;
		if (errno == ECONNREFUSED)
			return false;
		Sys_Printf ("NET_GetPacket: %s\n", strerror(errno));
		return false;
	}

	net_message.cursize = ret;
	SockadrToNetadr (&from, &net_from);

	return ret;
}

//=============================================================================

void NET_SendPacket (int dummy, int length, void *data, netadr_t to)
{
	static struct sockaddr_in	addr;

	NetadrToSockadr (&to, &addr);

	if (-1 == sendto (net_serversocket, data, length, 0, (struct sockaddr *)&addr, sizeof(addr))) {
		if (errno == EWOULDBLOCK)
			return;
		if (errno == ECONNREFUSED)
			return;
		Sys_Printf ("NET_SendPacket: %s\n", strerror(errno));
	}
}

//=============================================================================

int UDP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		Sys_Error ("UDP_OpenSocket: socket:", strerror(errno));
	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(_true)))
		Sys_Error ("UDP_OpenSocket: setsockopt SO_REUSEADDR: %s", strerror(errno));
	if (ioctl (newsocket, FIONBIO, (char *)&_true) == -1)
		Sys_Error ("UDP_OpenSocket: ioctl FIONBIO: %s", strerror(errno));
	address.sin_family = AF_INET;
//ZOID -- check for interface binding option
	i = COM_CheckParm("-ip");
	if (i && i + 1 < com_argc)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i+1]);
		Con_Printf("Binding to IP Interface Address of %s\n",
				inet_ntoa(address.sin_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;
	if (port == PORT_ANY)
		address.sin_port = 0;
	else
		address.sin_port = htons((short)port);
	if (bind (newsocket, (void *)&address, sizeof(address)) == -1)
		Sys_Error ("UDP_OpenSocket: bind: %s", strerror(errno));

	return newsocket;
}

int TCP_OpenSocket (int port)
{
	int newsocket;
	struct sockaddr_in address;
	qboolean _true = true;
	int i;

	if ((newsocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		Sys_Error ("TCP_OpenSocket: socket:", strerror(errno));
	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEADDR, &_true, sizeof(_true)))
		Sys_Error ("TCP_OpenSocket: setsockopt SO_REUSEADDR: %s", strerror(errno));
#ifdef __FreeBSD__
	if (setsockopt (newsocket, SOL_SOCKET, SO_REUSEPORT, &_true, sizeof(_true)))
		Sys_Error ("TCP_OpenSocket: setsockopt: %s", strerror(errno));
	address.sin_len = sizeof(address);
#endif
	if (ioctl (newsocket, FIONBIO, (char *)&_true) == -1)
		Sys_Error ("TCP_OpenSocket: ioctl FIONBIO: %s", strerror(errno));

	address.sin_family = AF_INET;
//ZOID -- check for interface binding option
	i = COM_CheckParm("-ipt");
	if (i && i + 1 < com_argc)
	{
		address.sin_addr.s_addr = inet_addr(com_argv[i + 1]);
		Con_Printf("Binding telnet service to IP Interface Address of %s\n",
				inet_ntoa(address.sin_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;

	address.sin_port = htons((short)port);
	if (bind (newsocket, (void *)&address, sizeof(address)) == -1)
		Sys_Error ("TCP_OpenSocket: bind: %s", strerror(errno));
	if (listen (newsocket, 1))
		Sys_Error ("TCP_OpenSocket: listen: %s", strerror(errno));

	return newsocket;
}


void NET_GetLocalAddress (void)
{
	char	buff[MAXHOSTNAMELEN];
	struct sockaddr_in	address;
	int		namelen;

	gethostname(buff, MAXHOSTNAMELEN);
	buff[MAXHOSTNAMELEN-1] = 0;

	NET_StringToAdr (buff, &net_local_adr);

	namelen = sizeof(address);
	if (getsockname (net_serversocket, (struct sockaddr *)&address, &namelen) == -1)
		Sys_Error ("NET_Init: getsockname:", strerror(errno));
	net_local_adr.port = address.sin_port;

	Con_Printf("IP address %s\n", NET_AdrToString (net_local_adr) );
}

/*
====================
NET_Init
====================
*/
int NET_Init (int port, int dummy, int telnetport)
{
	if (dummy)
		port = dummy;

	//
	// open the single socket to be used for all communications
	//
	if (port)
	{
		net_serversocket = UDP_OpenSocket (port);
		//
		// init the message buffer
		//
		net_message.maxsize = sizeof(net_message_buffer);
		net_message.data = net_message_buffer;

		//
		// determine my name & address
		//
		NET_GetLocalAddress ();

		Con_Printf("UDP Initialized\n");
		return port;
	}
	else
	{
		net_telnetsocket = TCP_OpenSocket (telnetport);
		Con_Printf("TCP Initialized\n");
		return telnetport;
	}
}

/*
====================
NET_Shutdown
====================
*/
void	NET_Shutdown (void)
{
	close (net_serversocket);
	if (telnetport)
	{
		if (telnet_connected)
			close (telnet_iosock);
		close (net_telnetsocket);
	}
}

