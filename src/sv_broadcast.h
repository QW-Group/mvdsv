#ifndef __SV_BROADCAST_H__
#define __SV_BROADCAST_H__

#include "qwsvdef.h"

// The message prefix used by clients to trigger a broadcast.
#define BROADCAST_PREFIX ".qw "
#define BROADCAST_PREFIX_LEN (sizeof(BROADCAST_PREFIX) - 1)

// Prefix used in log messages related to broadcasts.
#define BROADCAST_LOG_PREFIX "BROADCAST:"

// When this comment was added, there were approximately 800 servers registered
// with the most popular master servers, so 4096 should be sufficient for the
// foreseeable future.
#define BROADCAST_MAX_SERVERS 4096

// Defines the number of milliseconds to wait before giving up on acquiring the
// server list lock.
#define BROADCAST_SERVER_LIST_LOCK_TIMEOUT 5000

// Wait up to 5 seconds before giving up on the queried master server.
#define BROADCAST_MASTER_TIMEOUT 5

// Updates the server list every 10 minutes.
#define BROADCAST_SERVER_LIST_UPDATE_INTERVAL 600.0

// Permits broadcasts every 30 seconds.
#define BROADCAST_MESSAGE_INTERVAL 30.0

// Maximum number of retries for each message.
#define BROADCAST_MAX_RETRIES 3

// Limits the number of entries each IP address can occupy in the broadcast
// server list. This is a safeguard against master server poisoning. Currently,
// there is no server that uses more than 12 ports.
#define BROADCAST_SERVER_MAX_ENTRIES_PER_IP 25

qbool SV_Broadcast(char *message);
void SVC_Broadcast(void);
void SV_BroadcastInit(void);
void SV_BroadcastUpdateServerList(qbool force_update);
void SV_BroadcastUpdateServerList_f(void);

#endif /* !__SV_BROADCAST_H__ */
