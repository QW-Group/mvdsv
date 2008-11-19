#ifndef _QWFWD_H
#define _QWFWD_H

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <memory.h>
#include <string.h>

#ifdef __GNUC__
#define LittleLong(x) ({ typeof(x) _x = (x); _x = (((unsigned char *)&_x)[0]|(((unsigned char *)&_x)[1]<<8)|(((unsigned char *)&_x)[2]<<16)|(((unsigned char *)&_x)[3]<<24)); _x; })
#define LittleShort(x) ({ typeof(x) _x = (x); _x = (((unsigned char *)&_x)[0]|(((unsigned char *)&_x)[1]<<8)); _x; })
#else
#define LittleLong(x) (x)
#define LittleShort(x) (x)
#endif

#ifdef _WIN32

#include <winsock2.h>

typedef int socklen_t;

	#if defined(_DEBUG) && defined(_MSC_VER)
// uncomment/comment it if you wish/unwish see leaked memory(if any) in output window under MSVC
//		#define _CRTDBG_MAP_ALLOC
		#ifdef _CRTDBG_MAP_ALLOC
			#include <stdlib.h>
			#include <crtdbg.h>
		#endif
	#endif

#define EWOULDBLOCK		WSAEWOULDBLOCK
#define EMSGSIZE		WSAEMSGSIZE
#define ECONNRESET		WSAECONNRESET
#define ECONNABORTED	WSAECONNABORTED
#define ECONNREFUSED	WSAECONNREFUSED
#define EADDRNOTAVAIL	WSAEADDRNOTAVAIL
#define EAFNOSUPPORT	WSAEAFNOSUPPORT

#define qerrno			WSAGetLastError()

#else

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>

#define ioctlsocket		ioctl
#define closesocket		close
#define qerrno			errno

#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
#endif

typedef unsigned char byte;

#ifndef __cplusplus
typedef enum {false, true} qbool;
#else
typedef int qbool;
extern "C" {
#endif

#define	MAX_INFO_STRING		1024
#define MAX_INFO_KEY 		64

typedef enum
{
	ps_drop,		// we should drop this peer soon
	ps_challenge,	// peer getting a challenge
	ps_connected	// perr fully connected
} peer_state_t;


typedef struct peer
{
	time_t last;					// socket timeout helper
	time_t connect;					// connect helper
	int challenge;					// challenge num
	char userinfo[MAX_INFO_STRING]; // userinfo
	char name[MAX_INFO_KEY];		// name, extracted from userinfo
	int userid;						// unique per proxy userid
	int qport;						// qport
	struct sockaddr_in from;		// client addr
	struct sockaddr_in to;			// remote addr
	int s;							// socket, used for connection to remote host
	peer_state_t ps;				// peer state
	struct peer *next;				// next peer in linked list
} peer_t;

// used for passing params for thread
typedef struct fwd_params
{
	int port;
} fwd_params_t;

#define	MSG_BUF_SIZE		8192
#define MAX_MSGLEN 			1450

//=========================================

#define	QW_VERSION			"2.40"
#define	PROTOCOL_VERSION	28

// out of band message id bytes

// M = master, S = server, C = client, A = any
// the second character will always be \n if the message isn't a single
// byte long (?? not true anymore?)

#define	S2C_CHALLENGE		'c'
#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define A2A_ECHO		'e'	// for echoing
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'

// server to client
#define	svc_disconnect			2

//
// peer.c
//

peer_t		*peers;

peer_t		*FWD_peer_new(const char *remote_host, int remote_port, struct sockaddr_in *from, const char *userinfo, int qport, qbool link);
void		FWD_update_peers(void);

//
// msg.c
//

typedef struct sizebuf_s
{
	qbool allowoverflow; // if false, do a Sys_Error
	qbool overflowed; // set to true if the buffer size failed
	byte *data;
	int maxsize;
	int cursize;
} sizebuf_t;

void		SZ_Clear (sizebuf_t *buf);
void		SZ_InitEx (sizebuf_t *buf, byte *data, const int length, qbool allowoverflow);
void		SZ_Init (sizebuf_t *buf, byte *data, const int length);
void		*SZ_GetSpace (sizebuf_t *buf, const int length);
void		SZ_Write (sizebuf_t *buf, const void *data, int length);
void		SZ_Print (sizebuf_t *buf, const char *data);

//============================================================================

void		MSG_BeginReading (void);
int			MSG_GetReadCount(void);
qbool		MSG_BadRead (void);
int			MSG_ReadChar (void);
int			MSG_ReadByte (void);
int			MSG_ReadShort (void);
int			MSG_ReadLong (void);
float		MSG_ReadFloat (void);
char		*MSG_ReadString (void);
char		*MSG_ReadStringLine (void);

void		MSG_WriteChar (sizebuf_t *sb, const int c);
void		MSG_WriteByte (sizebuf_t *sb, const int c);
void		MSG_WriteShort (sizebuf_t *sb, const int c);
void		MSG_WriteLong (sizebuf_t *sb, const int c);
void		MSG_WriteFloat (sizebuf_t *sb, const float f);
void		MSG_WriteString (sizebuf_t *sb, const char *s);

//
// sys.c
//

#ifdef _WIN32

// vc++ snprintf and vsnprintf are non-standard and not compatible with C99.
int			qsnprintf(char *str, size_t n, char const *fmt, ...);
int			qvsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
#define		snprintf		qsnprintf
#define		vsnprintf		qvsnprintf

#endif

#if defined(__linux__) || defined(_WIN32) || defined(__CYGWIN__)

size_t			strlcpy (char *dst, const char *src, size_t siz);
size_t			strlcat (char *dst, char *src, size_t siz);

#endif

unsigned int	Sys_Milliseconds (void);
void			Sys_Printf (char *fmt, ...);
void			Sys_Exit (int code);
void			Sys_Error (char *error, ...);

#ifdef _CRTDBG_MAP_ALLOC
//
// otherwise we see always sys.c and this is not really helpful...
//
// but it should not be issue since we just debug it.
#define			Sys_malloc(_xxx)	calloc(1, (_xxx))

#else

void			*Sys_malloc (size_t size);

#endif

char			*Sys_strdup (const char *src);
#define			Sys_free(ptr) if(ptr) { free(ptr); ptr = NULL; }

// qqshka - hmm, seems in C these are macros, i don't like macros,
// however this functions work incorrectly on unsigned types!!!
#undef max
#undef min

#ifndef min
double			min( double a, double b );
#define KTX_MIN
#endif

#ifndef max
double			max( double a, double b );
#define KTX_MAX
#endif

double			bound( double a, double b, double c );

//
// net.c
//
int					net_socket;
struct sockaddr_in	net_from;
int					net_from_socket;
sizebuf_t			net_message;

int					NET_GetPacket(int s, sizebuf_t *msg);
void				NET_SendPacket(int s, int length, const void *data, struct sockaddr_in *to);
int					NET_UDP_OpenSocket(int port, qbool do_bind);
qbool				NET_GetSockAddrIn_ByHostAndPort(struct sockaddr_in *address, const char *host, int port);
qbool				NET_CompareAddress(struct sockaddr_in *a, struct sockaddr_in *b);

void				Netchan_OutOfBand(int s, struct sockaddr_in *adr, int length, byte *data);
void				Netchan_OutOfBandPrint(int s, struct sockaddr_in *adr, const char *format, ...);

void				NET_Init(int server_port);

//
// cmd.c
//

int					Cmd_Argc(void);
char				*Cmd_Argv(int arg);
char				*Cmd_Args(void);

void				Cmd_TokenizeString (char *text);

#define				MAX_COM_TOKEN	1024
char				com_token[MAX_COM_TOKEN];

char				*COM_Parse (char *data);		// Parse a token out of a string

//
// svc.c
//

void				SV_ConnectionlessPacket (void);

//
// clc.c
//

void				CL_ConnectionlessPacket (peer_t *p);

//
// info.c
//

qbool				ValidateUserInfo (char *userinfo);
char				*Info_ValueForKey (const char *s, const char *const key, char *const buffer, size_t buffersize);
void				Info_RemoveKey (char *s, const char *key);
void				Info_SetValueForStarKey (char *s, const char *key, const char *value, int maxsize);

#ifdef __cplusplus
}
#endif

#endif // _QWFWD_H
