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

	$Id: net.h,v 1.9 2006/02/27 12:01:59 disconn3ct Exp $
*/
// net.h -- quake's interface to the networking layer
#ifndef __NET_H__
#define __NET_H__

#ifdef _WIN32
#include <Winsock2.h>

#define EWOULDBLOCK	WSAEWOULDBLOCK
#define EMSGSIZE	WSAEMSGSIZE
#define ECONNRESET	WSAECONNRESET
#define ECONNABORTED	WSAECONNABORTED
#define ECONNREFUSED	WSAECONNREFUSED
#define EADDRNOTAVAIL	WSAEADDRNOTAVAIL
#define EAFNOSUPPORT	WSAEAFNOSUPPORT

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>

#ifdef sun
#include <sys/filio.h>
#endif

#ifdef NeXT
#include <libc.h>
#endif

#define closesocket close
#define ioctlsocket ioctl
#endif

#if defined(_WIN32)
#define qerrno WSAGetLastError()
#else
#define qerrno errno
#endif


#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif


#define	PORT_ANY	-1

typedef enum {NA_INVALID, NA_LOOPBACK, NA_IP, NA_IPV6, NA_IPX, NA_BROADCAST_IP, NA_BROADCAST_IP6, NA_BROADCAST_IPX} netadrtype_t;

typedef enum {NS_CLIENT, NS_SERVER} netsrc_t;

typedef struct
{
	netadrtype_t	type;

	union {
		byte	ip[4];
		byte	ip6[16];
		byte	ipx[10];
	} ip;

	unsigned short	port;
} netadr_t;

struct sockaddr_qstorage
{
	short dontusesa_family;
	unsigned char dontusesa_pad[6];
#if defined(_MSC_VER) || defined(MINGW)
	__int64 sa_align;
#else
	int sa_align[2];
#endif
	unsigned char sa_pad2[112];
};

extern	netadr_t	net_local_adr;
extern	netadr_t	net_from;		// address of who sent the packet
extern	sizebuf_t	net_message;

extern	cvar_t	hostname;

extern	int	net_socket;
extern	int	net_telnetsocket;
extern	int	telnetport;
extern	int	sv_port;
extern	int	telnet_iosock;
extern	int	telnet_connected;
extern	cvar_t	not_auth_timeout;
extern	cvar_t	auth_timeout;

int		NET_Init (int serverport, int telnetport);
void		NET_Shutdown (void);
qboolean	NET_GetPacket (void);
void		NET_SendPacket (int length, void *data, netadr_t to);

qboolean	NET_CompareAdr (netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b);
char		*NET_AdrToString (netadr_t a);
char		*NET_BaseAdrToString (netadr_t a);
qboolean	NET_StringToAdr (char *s, netadr_t *a);

//============================================================================

#define	OLD_AVG		0.99		// total = oldtotal*OLD_AVG + new*(1-OLD_AVG)

#define	MAX_LATENT	32

typedef struct
{
	qboolean	fatal_error;

	int		dropped;		// between last packet and previous

	float		last_received;		// for timeouts

// the statistics are cleared at each client begin, because
// the server connecting process gives a bogus picture of the data
	float		frame_latency;		// rolling average
	float		frame_rate;

	int		drop_count;		// dropped packets, cleared each level
	int		good_count;		// cleared each level

	netadr_t	remote_address;
	int		qport;

// bandwidth estimator
	double		cleartime;		// if realtime > nc->cleartime, free to go
	double		rate;			// seconds / byte

// sequencing variables
	int		incoming_sequence;
	int		incoming_acknowledged;
	int		incoming_reliable_acknowledged;	// single bit

	int		incoming_reliable_sequence;		// single bit, maintained local

	int		outgoing_sequence;
	int		reliable_sequence;	// single bit
	int		last_reliable_sequence;	// sequence number of last send

// reliable staging and holding areas
	sizebuf_t	message;		// writing buffer to send to server
	byte		message_buf[MAX_MSGLEN];

	int		reliable_length;
	byte		reliable_buf[MAX_MSGLEN];	// unacked reliable message

// time and size data to calculate bandwidth
	int		outgoing_size[MAX_LATENT];
	double		outgoing_time[MAX_LATENT];
} netchan_t;

void Netchan_Init (void);
void Netchan_Transmit (netchan_t *chan, int length, byte *data);
void Netchan_OutOfBand (netadr_t adr, int length, byte *data);
void Netchan_OutOfBandPrint (netadr_t adr, char *format, ...);
qboolean Netchan_Process (netchan_t *chan);
void Netchan_Setup (netchan_t *chan, netadr_t adr, int qport);

qboolean Netchan_CanPacket (netchan_t *chan);
qboolean Netchan_CanReliable (netchan_t *chan);

void SockadrToNetadr (struct sockaddr_qstorage *s, netadr_t *a);

#endif /* __NET_H__ */
