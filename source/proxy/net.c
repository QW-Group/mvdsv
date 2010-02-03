// net.c

#include "qwfwd.h"

cvar_t				*net_ip;
cvar_t				*net_port;

int					net_socket;
struct sockaddr_in	net_from;
int					net_from_socket;
sizebuf_t			net_message;
static byte			net_message_buffer[MSG_BUF_SIZE];

//=============================================================================

int NET_GetPacket(int s, sizebuf_t *msg)
{
	int ret;
	socklen_t fromlen = sizeof (net_from);

	SZ_Clear(msg);

	net_from_socket = s;

	ret = recvfrom(s, (char *)msg->data, msg->maxsize, 0, (struct sockaddr *) &net_from, &fromlen);

	if (ret == SOCKET_ERROR)
	{
		if (qerrno == EWOULDBLOCK)
			return false;

		if (qerrno == EMSGSIZE)
		{
			Sys_Printf ("NET_GetPacket: Oversize packet from %s\n", inet_ntoa(net_from.sin_addr));
			return false;
		}

		if (qerrno == ECONNRESET)
		{
			Sys_DPrintf ("NET_GetPacket: Connection was forcibly closed by %s\n", inet_ntoa(net_from.sin_addr));
			return false;
		}

		Sys_Error ("NET_GetPacket: recvfrom: (%i): %s", qerrno, strerror (qerrno));
	}

	if (ret >= msg->maxsize)
	{
		Sys_Printf ("NET_GetPacket: Oversize packet from %s\n", inet_ntoa(net_from.sin_addr));
		return false;
	}

	msg->cursize = ret;
	msg->data[ret] = 0;

	return ret;
}

//=============================================================================

void NET_SendPacket(int s, int length, const void *data, struct sockaddr_in *to)
{
	socklen_t addrlen = sizeof(*to);

	if (sendto(s, (const char *) data, length, 0, (struct sockaddr *)to, addrlen) == SOCKET_ERROR)
	{
		if (qerrno == EWOULDBLOCK)
			return;

		if (qerrno == ECONNREFUSED)
			return;

		Sys_Printf ("NET_SendPacket: sendto: (%i): %s\n", qerrno, strerror (qerrno));
	}
}

//=============================================================================

int NET_UDP_OpenSocket(const char *ip, int port, qbool do_bind)
{
	int s;
	unsigned long _true = 1;

	if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Sys_Printf("NET_UDP_OpenSocket: socket: (%i): %s\n", qerrno, strerror (qerrno));
		return INVALID_SOCKET;
	}

#ifndef _WIN32
	if (do_bind)
	{
// qqshka: funny, but on linux it allow me bind to the same port multiple times...
// qqshka: it was: Linux xxx 2.6.21-2950.fc8xen #1 SMP Tue Oct 23 12:23:33 EDT 2007 x86_64 x86_64 x86_64 GNU/Linux
//		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&_true, sizeof(_true)))
//		{
//			Sys_Printf("NET_UDP_OpenSocket: setsockopt SO_REUSEADDR: (%i): %s", qerrno, strerror (qerrno));
//			closesocket(s);
//			return INVALID_SOCKET;
//		}
	}
#endif

	if (ioctlsocket(s, FIONBIO, &_true) == SOCKET_ERROR)
	{
		Sys_Printf("NET_UDP_OpenSocket: ioctl FIONBIO: (%i): %s\n", qerrno, strerror (qerrno));
		closesocket(s);
		return INVALID_SOCKET;
	}

	if (do_bind)
	{
		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = (ip && *ip) ? inet_addr(ip) : INADDR_ANY;
		address.sin_port = htons ((short) port);

		if (bind(s, (struct sockaddr *)&address, sizeof(address)))
		{
			Sys_Printf("NET_UDP_OpenSocket: bind: (%i): %s\n", qerrno, strerror (qerrno));
			closesocket(s);
			return INVALID_SOCKET;
		}
	}

	return s;
}

//=============================================================================

qbool NET_GetSockAddrIn_ByHostAndPort(struct sockaddr_in *address, const char *host, int port)
{
	struct hostent *phe;
	struct sockaddr_in sin;

	memset(address, 0, sizeof(*address));

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

//	if((sin.sin_port = htons((u_short)port)) == 0)
//	{
//		Sys_Printf("NET_GetSockAddrIn_ByHostAndPort: wrong port: %d\n", port);
//		return false;
//	}
	sin.sin_port = htons((u_short)port);

	/* Map host name to IP address, allowing for dotted decimal */
	if((phe = gethostbyname(host)))
	{
		memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
	}
	else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		Sys_Printf("NET_GetSockAddrIn_ByHostAndPort: wrong host: %s\n", host);
		return false;
	}

	*address = sin;
	return true;
}

//=============================================================================
// return true if adresses equal
qbool NET_CompareAddress(struct sockaddr_in *a, struct sockaddr_in *b)
{
	if (!a || !b)
		return false;

	return (
			!memcmp(&a->sin_addr, &b->sin_addr, sizeof(a->sin_addr))
			&&
		    !memcmp(&a->sin_port, &b->sin_port, sizeof(a->sin_port))
		    );
}

char *NET_BaseAdrToString (struct sockaddr_in *a, char *buf, size_t bufsize)
{
	unsigned char ip[4];

	*(unsigned int*)ip = a->sin_addr.s_addr;

	snprintf(buf, bufsize, "%i.%i.%i.%i", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);

	return buf;
}

char *NET_AdrToString (struct sockaddr_in *a, char *buf, size_t bufsize)
{
	unsigned char ip[4];

	*(unsigned int*)ip = a->sin_addr.s_addr;

	snprintf(buf, bufsize, "%i.%i.%i.%i:%i", (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3], (int)ntohs(a->sin_port));

	return buf;
}

//=============================================================================

#define	PACKET_HEADER 8 /* FIXME: WTF IS THAT? */

/*
===============
Netchan_OutOfBand

Sends an out-of-band datagram
================
*/
void Netchan_OutOfBand(int s, struct sockaddr_in *adr, int length, byte *data)
{
	sizebuf_t send1;
	byte send_buf[MAX_MSGLEN + PACKET_HEADER];

	SZ_InitEx(&send1, send_buf, sizeof(send_buf), true);

	// write the packet header
	MSG_WriteLong(&send1, -1);	// -1 sequence means out of band
	// write data
	SZ_Write(&send1, data, length);

	if (send1.overflowed)
	{
		Sys_Printf("Netchan_OutOfBand: overflowed\n");
		return; // ah, should not happens
	}

	// send the datagram
	NET_SendPacket(s, send1.cursize, send1.data, adr);
}

/*
===============
Netchan_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void Netchan_OutOfBandPrint(int s, struct sockaddr_in *adr, const char *format, ...)
{
	char string[MAX_MSGLEN + PACKET_HEADER]; // string size should be somehow linked with Netchan_OutOfBand()
	va_list argptr;

	va_start(argptr, format);
	vsnprintf(string, sizeof (string), format, argptr);
	va_end(argptr);

	Netchan_OutOfBand(s, adr, strlen (string), (byte *) string);
}

//=============================================================================

/*
====================
NET_Init
====================
*/
void NET_Init (void)
{
	char *ip = (*ps.params.ip) ? ps.params.ip : "0.0.0.0";
	char port[64] = {0};

	snprintf(port, sizeof(port), "%d", 	ps.params.port ? ps.params.port : QWFWD_DEFAULT_PORT);

	if (*ps.params.ip) // if cmd line - force it, so we have priority over cfg
		net_ip	 = Cvar_FullSet("net_ip", ip, CVAR_NOSET);
	else
		net_ip	 = Cvar_Get("net_ip",	  ip, CVAR_NOSET);

	if (ps.params.port) // if cmd line - force it, so we have priority over cfg
		net_port = Cvar_FullSet("net_port",	port, CVAR_NOSET);
	else
		net_port = Cvar_Get("net_port",		port, CVAR_NOSET);

#ifdef _WIN32
	{
		WSADATA		winsockdata;

		if (WSAStartup(MAKEWORD (2, 1), &winsockdata))
			Sys_Error("WinSock initialization failed");
	}
#endif

	if ((net_socket = NET_UDP_OpenSocket(net_ip->string, net_port->integer, true)) == INVALID_SOCKET)
		Sys_Error("NET_Init: failed to initialize socket");

	// init the message buffer
	SZ_InitEx(&net_message, net_message_buffer, sizeof(net_message_buffer), false);

	Sys_DPrintf("UDP Initialized\n");
}

