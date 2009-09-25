// net.c

#include "qwfwd.h"

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
			Sys_Printf ("NET_GetPacket: Connection was forcibly closed by %s\n", inet_ntoa(net_from.sin_addr));
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
void NET_Init (const char *ip, int server_port)
{
#ifdef _WIN32
	{
		WSADATA		winsockdata;

		if (WSAStartup(MAKEWORD (2, 1), &winsockdata))
			Sys_Error("WinSock initialization failed");
	}
#endif

	if ((net_socket = NET_UDP_OpenSocket(ip, server_port, true)) == INVALID_SOCKET)
		Sys_Error("NET_Init: failed to initialize socket");

	// init the message buffer
	SZ_InitEx(&net_message, net_message_buffer, sizeof(net_message_buffer), false);

//	Sys_Printf("UDP Initialized\n");
}

