/*
	query.c - query master/normal qw servers
*/

#include "qwfwd.h"

#define QW_SERVER_RATE (0.1) // seconds, accept fraction, how frequently sent ONE packet to some server, so 0.1 means one packet per 1/10 of second
#define QW_SERVER_PING_QUERY "\xff\xff\xff\xffk\n"
#define QW_SERVER_MIN_PING_REQUEST_TIME 60 // seconds, minimal time interval allowed to sent ping, so we do not spam server too fast
#define QW_SERVER_DEAD_TIME (60 * 60) // seconds, if we do not get reply from server in this time, guess server is DEAD


#define QW_MASTER_QUERY "c\n"
#define QW_MASTER_QUERY_TIME (60 * 30) // seconds, how frequently we query masters
#define QW_MASTER_QUERY_TIME_SHORT 60 // seconds, how frequently we query master if we do not get reply from it yet
#define QW_MASTERS_FORCE_RE_INIT (60 * 60 * 24) // seconds, force re-init masters time to time, so we add proper masters if there was some ip/dns changes
#define QW_MASTER_HEARTBEAT_SECONDS (60 * 5) // seconds, frequency of heartbeat

#define QW_DEFAULT_MASTER_SERVERS "master.quakeservers.net asgaard.morphos-team.net master.quakeworld.nu"
#define QW_DEFAULT_MASTER_SERVER_PORT 27000

#define MAX_MASTERS 8 // size for masters fixed size array, I am lazy

#define MAX_SERVERS 512 // we will not add more than that servers to our list, just for some sanity

static cvar_t *masters_query;
static cvar_t *masters_heartbeat;
static cvar_t *masters_list;

// master state enum
typedef enum
{
	ms_unknown,		// unknown state
	ms_used			// this slot used in masters_t struct
} master_state_t;

// single master struct
typedef struct master
{
	master_state_t			state;		// master state
	time_t					next_query;	// next time when query master server
	struct sockaddr_in		addr;		// master addr
} master_t;

// all masters in one struct
typedef struct masters
{
	time_t					init_time;				// this is used to periodical re-init initiation

	time_t					last_heartbeat;			// when we send heartbeat last time
	int						heartbeat_sequence;		// heartbeat sequence number

	master_t				master[MAX_MASTERS];	// masters fixed size array, I am lazy
} masters_t;

// single server struct
typedef struct server
{
	struct sockaddr_in		addr;			// addr

	qbool					reply;			// true if we get reply after ping packet was sent,
											// reset to false each time we sent packet
	double					ping_sent_at;	// last time when we send ping request, so we can calculate ping time
	double					ping_reply_at;	// last time when we receive ping reply from server,
											// so we can guess is server dead etc
	int						ping;			// ping to that server in milliseconds

	struct server			*next;			// next server in linked list
} server_t;

static int sv_count;
static server_t *servers;
static masters_t masters;

static master_t	*QRY_Master_ByAddr(struct sockaddr_in *addr)
{
	int						i;
	master_t				*m;

	for (i = 0, m = masters.master; i < MAX_MASTERS; i++, m++)
	{
		if (m->state != ms_used)
			continue; // master slot unused

		if (NET_CompareAddress(addr, &m->addr))
			return m;
	}

	return NULL;
}


static qbool QRY_AddMaster(const char *master)
{
	int						i, port;
	master_t				*m;
	struct sockaddr_in		addr;
	char					host[1024], *column;

	// decide host:port, port is optional and DEFAULT_MASTER_SERVER_PORT is used if ommited
	port = 0;
	strlcpy(host, master, sizeof(host));
	if ((column = strchr(host, ':')))
	{
		column[0] = 0; // truncate host name
		port = atoi(column + 1); // get port for real
	}
	port = (port > 0 && port < 65535) ? port : QW_DEFAULT_MASTER_SERVER_PORT;

	if (!host[0])
	{
		Sys_Printf("failed to add master server: %s\n", master);
		return false; // empty host name, not funny
	}

	if (!NET_GetSockAddrIn_ByHostAndPort(&addr, host, port))
	{
		Sys_Printf("failed to add master server: %s\n", master);
		return false;
	}

	if (QRY_Master_ByAddr(&addr))
	{
		Sys_Printf("failed to add master server: %s - alredy added!\n", master);
		return false;
	}

	for (i = 0, m = masters.master; i < MAX_MASTERS; i++, m++)
	{
		if (m->state == ms_used)
			continue; // master slot used

		memset(m, 0, sizeof(*m)); // reset data in slot

		m->state = ms_used;
		m->addr = addr;

		Sys_Printf("master server added: %s\n", master);
		return true;
	}

	Sys_Printf("failed to add master server: %s\n", master);
	return false;
}

static void QRY_Cmd_Heartbeat_f(void)
{
	masters.last_heartbeat = time(NULL) - QW_MASTER_HEARTBEAT_SECONDS - 1; // trigger heartbeat ASAP
}

// clear masters
static void QRY_MastersInit(void)
{
	memset(&masters, 0, sizeof(masters));
	masters.init_time = time(NULL);

	QRY_Cmd_Heartbeat_f();  // trigger heartbeat ASAP
}

// check if "masters" or "masters_query" cvar changed and do appropriate action
static void QRY_CheckMastersModified(void)
{
	char *mlist;

	// for fix issues with DNS and such force masters re-init time to time
	if (time(NULL) - masters.init_time > QW_MASTERS_FORCE_RE_INIT)
	{
		Sys_DPrintf("forcing masters re-init\n");
		masters_list->modified = true;
	}

	// "masters" and "masters_query" was not modified, do nothing
	if (!masters_list->modified && !masters_query->modified)
		return;

	// clear masters
	QRY_MastersInit();

	// add all masters
	for ( mlist = masters_list->string; (mlist = COM_Parse(mlist)); )
	{
		QRY_AddMaster(com_token);
	}

	masters_list->modified = masters_query->modified = false;
}

// query master servers
static void QRY_QueryMasters(void)
{
	int			i;
	master_t	*m;
	time_t		current_time = time(NULL);
	char		buf[] = "xxx.xxx.xxx.xxx:xxxxx";

	// do we need query masters?
	if (!masters_query->integer)
		return;

	for (i = 0, m = masters.master; i < MAX_MASTERS; i++, m++)
	{
		if (m->state != ms_used)
			continue; // master slot not used

		if (current_time <= m->next_query)
			continue; // not yet

		Sys_DPrintf("query master: %s\n", NET_AdrToString(&m->addr, buf, sizeof(buf)));
		
		NET_SendPacket(net_socket, sizeof(QW_MASTER_QUERY), QW_MASTER_QUERY, &m->addr);
		m->next_query = current_time + QW_MASTER_QUERY_TIME_SHORT; // delay next query for some time
	}
}

// heartbeat master servers.
// send a message to the master every few minutes.
static void QRY_HeartbeatMasters(void)
{
	char		string[128];
	int			i, len;
	master_t	*m;
	time_t		current_time = time(NULL);
	char		buf[] = "xxx.xxx.xxx.xxx:xxxxx";

	// do we need heartbeat masters?
	if (!masters_heartbeat->integer)
		return;

	if (current_time < masters.last_heartbeat + QW_MASTER_HEARTBEAT_SECONDS)
		return; // not yet
	
	masters.last_heartbeat = current_time;
	masters.heartbeat_sequence++;
	snprintf(string, sizeof(string), "%c\n%i\n%i\n", S2M_HEARTBEAT, masters.heartbeat_sequence, FWD_peers_count());
	len = strlen(string);

	if (developer->integer > 1)
		Sys_DPrintf("heartbeat:\n%s\n", string);

	for (i = 0, m = masters.master; i < MAX_MASTERS; i++, m++)
	{
		if (m->state != ms_used)
			continue; // master slot not used
		
		Sys_DPrintf("heartbeat master: %s\n", NET_AdrToString(&m->addr, buf, sizeof(buf)));
		NET_SendPacket(net_socket, len, string, &m->addr);
	}
}

qbool QRY_IsMasterReply(void)
{
	if (net_message.cursize < 6 || memcmp(net_message.data, "\xff\xff\xff\xff\x64\x0a", 6))
		return false;

	return true;
}

static server_t *QRY_SV_Add(const char *remote_host, int remote_port); // forward reference

void SVC_QRY_ParseMasterReply(void)
{
    int				i, c;
	master_t		*m;
	int				ret = net_message.cursize;
	unsigned char	*answer = net_message.data; // not the smartest way, but why copy from one place to another...

	// no point to parse it, we do not query masters
	if (!masters_query->integer)
	{
		Sys_DPrintf("master server reply ignored\n");
		return;
	}

	Sys_DPrintf ("master server reply from %s:%d\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));

	// is it reply from registered master server or someone trying to do some evil things?
	for (i = 0, m = masters.master; i < MAX_MASTERS; i++, m++)
	{
		if (m->state != ms_used)
			continue; // master slot not used

		if (NET_CompareAddress(&net_from, &m->addr))
		{
			// OK - it is reply from registered master server
			m->next_query = time(NULL) + QW_MASTER_QUERY_TIME; // delay next query for some time
			break;
		}
	}

	if (i >= MAX_MASTERS)
	{
		Sys_Printf("Reply from not registered master server\n");
		return;
	}

	Sys_DPrintf("master server returned %d bytes\n", ret);

	for (c = 0, i = 6; i + 5 < ret; i += 6, c++)
	{
		char ip[64];
		int port = 256 * (int)answer[i+4] + (int)answer[i+5];

		snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
			(int)answer[i+0], (int)answer[i+1],
			(int)answer[i+2], (int)answer[i+3]);

		if (developer->integer > 1)
			Sys_DPrintf("SERVER: %4d %s:%d\n", c, ip, port);

		QRY_SV_Add(ip, port);
	}
}

//========================================

static int QRY_SV_Count(void)
{
	return sv_count;
}

static server_t	*QRY_SV_new(struct sockaddr_in *addr, qbool link)
{
	server_t *sv;

	if (!addr)
		return NULL; // not funny

	if (QRY_SV_Count() >= MAX_SERVERS)
		return NULL;

	sv_count++;

	sv = Sys_malloc(sizeof(*sv));
	sv->addr = *addr;
	sv->ping = 0xFFFF; // mark as unreachable

	if (link)
	{
		sv->next = servers;
		servers = sv;
	}

	return sv;
}

// free server data, perform unlink if requested
static void QRY_SV_free(server_t *sv, qbool unlink)
{
	if (!sv)
		return;

	if (unlink)
	{
		server_t *next, *prev, *current;

		prev = NULL;
		current = servers;

		for ( ; current; )
		{
			next = current->next;

			if (sv == current)
			{
				if (prev)
					prev->next = next;
				else
					servers = next;

				break;
			}

			prev = current;
			current = next;
		}
	}

	// free all data related to server
	Sys_free(sv);

	sv_count--;
}

// return server by pseudo index
static server_t	*QRY_SV_ByIndex(int idx)
{
	server_t	*sv;

	if (idx < 0)
		return NULL;

	for (sv = servers; sv; sv = sv->next, idx--)
		if (!idx)
			return sv;

	return NULL;
}

static server_t	*QRY_SV_ByAddr(struct sockaddr_in *addr)
{
	server_t	*sv;

	for (sv = servers; sv; sv = sv->next)
		if (NET_CompareAddress(addr, &sv->addr))
			return sv;

	return NULL;
}

static server_t	*QRY_SV_Add(const char *remote_host, int remote_port)
{
	server_t	*sv;
	struct sockaddr_in addr;

	if (!NET_GetSockAddrIn_ByHostAndPort(&addr, remote_host, remote_port))
		return NULL; // failed to resolve host name?

	if ((sv = QRY_SV_ByAddr(&addr)))
	{
//		Sys_Printf("QRY_SV_Add: alredy have %s:%d\n", remote_host, remote_port);
		return sv; // we alredy have such server on list
	}

	if (!(sv = QRY_SV_new(&addr, true)))
		return NULL; // fail of some kind

	return sv;
}

static void QRY_SV_PingServers(void)
{
	static int		idx;
	static double	last;

	double			current = Sys_DoubleTime(); // we need double time for ping measurement
	server_t		*sv;

	// do not ping servers since we do not query masters
	if (!masters_query->integer)
		return;

	if (!servers)
		return; // nothing to do

	if (current - last < QW_SERVER_RATE)
		return; // do not ping servers too fast

	last = current;

	idx = (int)max(0, idx);
	sv = QRY_SV_ByIndex(idx++);
	if (!sv)
		sv = QRY_SV_ByIndex(idx = 0); // can't find server by index, try with index 0

	if (!sv)
		return; // hm, should not be the case...

	// check for dead server
	if (!sv->reply && sv->ping_sent_at - sv->ping_reply_at > QW_SERVER_DEAD_TIME)
	{
		Sys_DPrintf("dead -> %s:%d\n", inet_ntoa(sv->addr.sin_addr), (int)ntohs(sv->addr.sin_port));

		QRY_SV_free(sv, true); // remove damn server, however master server may add it back...
		idx--; // step back index
		return;
	}

	if (sv->ping_sent_at && current - sv->ping_sent_at < QW_SERVER_MIN_PING_REQUEST_TIME)
		return; // do not spam server

	sv->ping_sent_at = current; // remember when we sent ping
	sv->reply = false; // reset reply flag

	NET_SendPacket(net_socket, sizeof(QW_SERVER_PING_QUERY)-1, QW_SERVER_PING_QUERY, &sv->addr);
//	Sys_Printf("ping(%3d) -> %s:%d\n", idx, inet_ntoa(sv->addr.sin_addr), (int)ntohs(sv->addr.sin_port));
}

void QRY_SV_PingReply(void)
{
	server_t *sv = NULL;

	// ignore server ping reply since we do not query masters and can't keep server list up2date
	if (!masters_query->integer)
	{
		Sys_DPrintf("server reply ignored\n");
		return;
	}

	sv = QRY_SV_ByAddr(&net_from);

	if (sv)
	{
		double current = Sys_DoubleTime();
		double ping = current - sv->ping_sent_at;

		sv->ping = (int)max(0, 1000.0 * ping);
		sv->ping_reply_at = current;
		sv->reply = true;

//		Sys_Printf("ping <- %s:%d, %d\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port), sv->ping);
	}
	else
	{
//		Sys_Printf("ping <- %s:%d, not registered server\n", inet_ntoa(net_from.sin_addr), (int)ntohs(net_from.sin_port));
	}
}

void SVC_QRY_PingStatus(void)
{
	static sizebuf_t buf; // static  - so it not allocated each time
	static byte		buf_data[MSG_BUF_SIZE]; // static  - so it not allocated each time

	server_t		*sv;

	SZ_InitEx(&buf, buf_data, sizeof(buf_data), true);

	MSG_WriteLong(&buf, -1);	// -1 sequence means out of band
	MSG_WriteChar(&buf, A2C_PRINT);

	// if we does not query masters then we can't proved reliable info, so do not send servers list
	if (masters_query->integer)
	{
		for (sv = servers; sv; sv = sv->next)
		{
			MSG_WriteLong(&buf, *(int *)&sv->addr.sin_addr);
			MSG_WriteShort(&buf, (short)ntohs(sv->addr.sin_port));
			MSG_WriteShort(&buf, (short)sv->ping);
		}
	}

	if (buf.overflowed)
	{
		Sys_Printf("SVC_QRY_PingStatus: overflow\n");
		return; // overflowed
	}

	// send the datagram
	NET_SendPacket(net_from_socket, buf.cursize, buf.data, &net_from);
}
//==============================================

static void QRY_Cmd_SvList_f(void)
{
	server_t	*sv;
	int idx;
	char ipport[] = "xxx.xxx.xxx.xxx:xxxxx";

	Sys_Printf("=== server list ===\n");
	Sys_Printf("### %-*s ping\n", sizeof(ipport)-1, "address");
	Sys_Printf("--------------------------------------\n");

	for (idx = 1, sv = servers; sv; sv = sv->next, idx++)
	{
		Sys_Printf("%3d %-*s %d\n",
			idx, sizeof(ipport)-1, NET_AdrToString(&sv->addr, ipport, sizeof(ipport)), (int)sv->ping);
	}

	Sys_Printf("--------------------------------------\n");
	Sys_Printf("%d servers\n", idx-1);
}

//==============================================

void QRY_Frame(void)
{
	QRY_CheckMastersModified();		// check is "masters" variable changed
	QRY_QueryMasters();				// request time to time server list from masters
	QRY_HeartbeatMasters();			// send heartbeat to masters time to time
	QRY_SV_PingServers();			// ping time to time normal qw servers
}

//==============================================

void QRY_Init(void)
{
	masters_query		= Cvar_Get("masters_query",		"1", 0);
	masters_heartbeat	= Cvar_Get("masters_heartbeat",	"1", 0);
	masters_list		= Cvar_Get("masters",			QW_DEFAULT_MASTER_SERVERS, 0);

	Cmd_AddCommand("svlist", QRY_Cmd_SvList_f);
	Cmd_AddCommand("heartbeat", QRY_Cmd_Heartbeat_f);

	// clear masters
	QRY_MastersInit();
}
