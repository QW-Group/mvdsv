/*
 * Copyright (C) 2025 Oscar Linderholm <osm@recv.se>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "qwsvdef.h"

typedef struct {
	char *message;
	char *name;
} args_t;

typedef struct {
	unsigned int ip;
	unsigned int count;
	time_t window_start;
} ratelimit_t;

static DWORD WINAPI SV_BroadcastSend(void *data);
static DWORD WINAPI SV_BroadcastQueryMasters(void *data);
static void SV_BroadcastQueryMaster(int sock, netadr_t *naddr, netadr_t *server, int *server_count);
static qbool SVC_BroadcastIsRateLimited(netadr_t *from);

static mutex_t servers_update_lock;
static double last_servers_update;

static mutex_t broadcast_lock;
static double last_broadcast;

static mutex_t server_list_lock;
static netadr_t server_list[BROADCAST_MAX_SERVERS];
static int server_list_count;

static ratelimit_t ratelimit[BROADCAST_RATELIMIT_MAX_ENTRIES];

void SV_BroadcastInit(void)
{
	Sys_MutexInit(&broadcast_lock);
	last_broadcast = -99999;

	Sys_MutexInit(&servers_update_lock);
	last_servers_update = -99999;

	Sys_MutexInit(&server_list_lock);
}

void SV_BroadcastUpdateServerList_f(void)
{
	SV_BroadcastUpdateServerList(true);
}

void SV_BroadcastEnabledOnChange(cvar_t *cvar, char *value, qbool *cancel)
{
	// We can't use the CVAR_SERVERINFO flag in this case because we want
	// the key to be named "broadcast" not "sv_broadcast_enabled".
	// Therefore, the cvar will not have this flag set, instead, we'll
	// trigger the serverinfo update manually.
	SV_ServerinfoChanged("broadcast", value);
}

void SV_BroadcastUpdateServerList(qbool force_update)
{
	extern netadr_t master_adr[MAX_MASTERS];
	extern cvar_t sv_broadcast_enabled;

	if (!sv_broadcast_enabled.value)
	{
		if (force_update)
		{
			Con_Printf("SV_BroadcastUpdateServerList: "
				"Cannot update broadcast servers, sv_broadcast_enabled is off\n");
		}
		return;
	}

	if (GameStarted())
	{
		if (force_update)
		{
			Con_Printf("SV_BroadcastUpdateServerList: "
				"Cannot update broadcast servers, a game has already started\n");
		}
		return;
	}

	if (!force_update && realtime - last_servers_update < BROADCAST_SERVER_LIST_UPDATE_INTERVAL)
	{
		return;
	}

	// If there are no master servers configured, we'll wait the entire
	// update_interval before trying again.
	if (!master_adr[0].port)
	{
		Con_Printf("SV_BroadcastUpdateServerList: No master servers configured\n");
		last_servers_update = realtime;
		return;
	}

	if (!Sys_MutexTryLock(&servers_update_lock))
	{
		if (force_update)
		{
			Con_Printf("SV_BroadcastUpdateServerList: "
				"An update of broadcast servers is already running\n");
		}
		return;
	}

	Con_Printf("Broadcast server list sync started\n");

	if (Sys_CreateThread(SV_BroadcastQueryMasters, NULL))
	{
		Con_Printf("SV_BroadcastUpdateServerList: Unable to start query masters thread\n");
		last_servers_update = realtime;
		Sys_MutexUnlock(&servers_update_lock);
	}
}

static DWORD WINAPI SV_BroadcastQueryMasters(void *data)
{
	static struct timeval timeout = {BROADCAST_MASTER_TIMEOUT, 0};
	static netadr_t servers[BROADCAST_MAX_SERVERS];
	extern netadr_t master_adr[MAX_MASTERS];
	int server_count = 0;
	int sock = 0;
	int i = 0;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		Con_Printf("SV_BroadcastQueryMasters: Unable to initialize socket\n");
		goto out;
	}

#ifdef _WIN32
	static int timeout_ms = -1;

	if (timeout_ms == -1)
	{
		timeout_ms = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms)) < 0)
#else
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
#endif
	{
		Con_Printf("SV_BroadcastQueryMasters: Unable to set timeout on socket\n");
		goto cleanup;
	}

	memset(servers, 0, sizeof(servers));

	for (i = 0; i < MAX_MASTERS; i++)
	{
		if (!master_adr[i].port)
		{
			break;
		}

		SV_BroadcastQueryMaster(sock, &master_adr[i], servers, &server_count);
	}

	if (!Sys_MutexTryLockWithTimeout(&server_list_lock, BROADCAST_SERVER_LIST_LOCK_TIMEOUT))
	{
		Con_Printf("SV_BroadcastQueryMasters: Failed to acquire server_list_lock, aborting\n");
		goto cleanup;
	}
	server_list_count = server_count;
	memcpy(server_list, servers, sizeof(netadr_t) * server_list_count);
	Sys_MutexUnlock(&server_list_lock);
	Con_Printf("Broadcast server list sync complete: %d server(s) available.\n", server_list_count);

cleanup:
	closesocket(sock);
out:
	Sys_MutexUnlock(&servers_update_lock);
	last_servers_update = realtime;

	return 0;
}

static void SV_BroadcastQueryMaster(int sock, netadr_t *naddr, netadr_t *servers, int *server_count)
{
	// Payload to send to the master server, which returns a list of all
	// registered servers.
	static const char payload[] = { 0x63 };

	// This is the header we expect to receive from the master server.
	static const char header[] = { 0xff, 0xff, 0xff, 0xff, 0x64, 0x0a };

	const size_t header_size = sizeof(header);
	const int addr_size = 6;
	struct sockaddr_storage addr;
	netadr_t new_addr;
	qbool exists = false;
	char buf[1024*64] = {0};
	char *master = NULL;
	int ip_count = 0;
	int offset = 0;
	int ret = 0;
	int i = 0;

	NetadrToSockadr(naddr, &addr);
	master = NET_AdrToString(*naddr);

	ret = sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&addr,
		sizeof(struct sockaddr_in));
	if (ret < 0)
	{
		Con_Printf("SV_BroadcastQueryMaster: Unable to query %s\n", master);
		return;
	}

	ret = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
	if (ret <= 0)
	{
		Con_Printf("SV_BroadcastQueryMaster: No data received from %s\n", master);
		return;
	}

	if (ret < header_size)
	{
		Con_Printf("SV_BroadcastQueryMaster: Unexpected data returned from %s\n", master);
		return;
	}

	if (memcmp(buf, header, header_size) != 0)
	{
		Con_Printf("SV_BroadcastQueryMaster: Unexpected header returned from %s\n", master);
		return;
	}

	if ((ret - header_size) % addr_size != 0)
	{
		Con_Printf("SV_BroadcastQueryMaster: Invalid data received from %s\n", master);
		return;
	}

	for (offset = header_size; offset < ret; offset += addr_size)
	{
		if (*server_count >= BROADCAST_MAX_SERVERS)
		{
			Con_Printf("SV_BroadcastQueryMaster: Server list is full!\n");
			break;
		}

		memcpy(&new_addr.ip, &buf[offset], 4);
		memcpy(&new_addr.port, &buf[offset + 4], 2);
		exists = false;
		ip_count = 0;

		// There are so few servers, so we'll accept the cost of
		// iterating over all entries.
		for (i = 0; i < *server_count; i++)
		{
			if (NET_CompareAdr(new_addr, servers[i]))
			{
				exists = true;
				break;
			}

			if (NET_CompareBaseAdr(new_addr, servers[i]))
			{
				if (ip_count >= BROADCAST_SERVER_MAX_ENTRIES_PER_IP)
				{
					break;
				}

				ip_count++;
			}
		}

		if (exists)
		{
			continue;
		}

		if (ip_count >= BROADCAST_SERVER_MAX_ENTRIES_PER_IP)
		{
			Con_Printf("SV_BroadcastQueryMaster: Ignoring %s (IP limit reached)\n",
				NET_AdrToString(new_addr));
			continue;
		}

		memcpy(servers[*server_count].ip, &new_addr.ip, 4);
		servers[*server_count].port = new_addr.port;
		(*server_count)++;
	}
}

qbool SV_Broadcast(char *message)
{
	extern cvar_t sv_broadcast_enabled;
	args_t *args = NULL;

	if (!sv_broadcast_enabled.value)
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "Broadcasting is not enabled on this server\n");
		return false;
	}

	if (GameStarted())
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "Broadcasting is not available during games\n");
		return false;
	}

	// When the data in the server list is actually used, we'll acquire the
	// necessary lock to ensure that the data is correct. However, for this
	// use case, it should be fine to just check it without a lock.
	if (server_list_count == 0)
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "No broadcast servers configured\n");
		return false;
	}

	if (realtime - last_broadcast < BROADCAST_MESSAGE_INTERVAL - 1)
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "Wait %d seconds before broadcasting again\n",
			(int)(BROADCAST_MESSAGE_INTERVAL - (realtime - last_broadcast)));
		return false;
	}

	if (!Sys_MutexTryLock(&broadcast_lock))
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "A broadcast is already in progress\n");
		return false;
	}

	args = Q_malloc(sizeof(args_t));
	args->message = Q_strdup(message);
	args->name = Q_strdup(sv_client->name);

	if (Sys_CreateThread(SV_BroadcastSend, args))
	{
		SV_ClientPrintf(sv_client, PRINT_HIGH, "Unable to start broadcast thread\n");
		Sys_MutexUnlock(&broadcast_lock);
		Q_free(args->message);
		Q_free(args->name);
		Q_free(args);
	}

	return true;
}

static DWORD WINAPI SV_BroadcastSend(void *data)
{
	args_t *args = (args_t *)data;
	struct sockaddr_storage addr;
	char out[1024] = {0};
	int written = 0;
	int err_count = 0;
	int sock = 0;
	int len = 0;
	int ret = 0;
	int i = 0;

	memset(out, 0xff, 4);
	len = 4;

	written = snprintf(out+len, sizeof(out)-len,
		"broadcast \\hostport\\%s\\port\\%d\\name\\%s\\message\\%s\n",
		Cvar_String("hostport"), NET_UDPSVPort(), args->name, args->message);

	if (written < 0 || written >= sizeof(out) - len)
	{
		goto out;
	}

	len += written;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		Con_Printf("SV_BroadcastSend: Unable to initialize socket\n");
		goto out;
	}

	if (!Sys_MutexTryLockWithTimeout(&server_list_lock, BROADCAST_SERVER_LIST_LOCK_TIMEOUT))
	{
		Con_Printf("SV_BroadcastSend: Failed to acquire server_list_lock, aborting\n");
		goto close;
	}

	for (i = 0; i < server_list_count; i++)
	{
		NetadrToSockadr(&server_list[i], &addr);

		ret = sendto(sock, out, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if (ret < 0)
		{
			Con_Printf("SV_BroadcastSend: Unable to send broadcast to %s\n",
				NET_AdrToString(server_list[i]));

			if (err_count++ == BROADCAST_MAX_ERRORS)
			{
				goto cancel;
			}
		}
	}

cancel:
	Sys_MutexUnlock(&server_list_lock);
close:
	closesocket(sock);
out:
	Sys_MutexUnlock(&broadcast_lock);
	last_broadcast = realtime;

	Q_free(args->message);
	Q_free(args->name);
	Q_free(args);

	return 0;
}

void SVC_Broadcast(void)
{
	extern cvar_t sv_broadcast_sender_validation_enabled;
	extern cvar_t sv_broadcast_enabled;
	client_t *client = NULL;
	qbool spectalk = false;
	qbool started = false;
	qbool valid = false;
	char addrport[22] = {0};
	char log[1024] = {0};
	char out[1024] = {0};
	char *displayaddr = NULL;
	char *hostport = NULL;
	char *message = NULL;
	char *payload = NULL;
	char *addr = NULL;
	char *name = NULL;
	char *port = NULL;
	int i = 0;

	if (!sv_broadcast_enabled.value || Cmd_Argc() < 1)
	{
		return;
	}

	if (SVC_BroadcastIsRateLimited(&net_from))
	{
		return;
	}

	payload = Cmd_Args();
	addr = NET_AdrToString(net_from);

	// If enabled, ensure that the sender's address exists in the list of
	// addresses received from the master server.
	if (sv_broadcast_sender_validation_enabled.value)
	{
		valid = false;

		if (!Sys_MutexTryLockWithTimeout(&server_list_lock, BROADCAST_SERVER_LIST_LOCK_TIMEOUT))
		{
			Con_Printf("SVC_Broadcast: Failed to acquire server_list_lock, aborting\n");
			return;
		}

		for (i = 0; i < server_list_count; i++)
		{
			if (NET_CompareBaseAdr(net_from, server_list[i]))
			{
				valid = true;
				break;
			}
		}

		Sys_MutexUnlock(&server_list_lock);

		if (!valid)
		{
			Con_Printf("Rejected broadcast from address: %s (payload: %s)\n",
				addr , payload);
			return;
		}
	}

	hostport = Info_ValueForKey(payload, "hostport");
	name = Info_ValueForKey(payload, "name");
	message = Info_ValueForKey(payload, "message");
	port = Info_ValueForKey(payload, "port");

	if (strlen(name) == 0 || strlen(message) == 0)
	{
		Con_Printf("Rejected broadcast with payload: %s (%s)\n", payload, addr);
		return;
	}

	if (strlen(hostport) > 0)
	{
		displayaddr = hostport;
	}
	else if (strlen(port) > 0)
	{
		snprintf(addrport, sizeof(addrport), "%s:%s", NET_BaseAdrToString(net_from), port);
		displayaddr = addrport;
	}
	else
	{
		displayaddr = addr;
	}

	snprintf(out, sizeof(out), "[%s] %s: %s", displayaddr, name, message);
	snprintf(log, sizeof(log), "%s \\addr\\%s%s\n", BROADCAST_LOG_PREFIX, addr, payload);

	Con_Printf("%s\n", out);
	SV_Write_Log(CONSOLE_LOG, 0, log);

	started = GameStarted();

	// If the KTX spectalk feature is enabled, we'll allow the broadcast
	// message to be seen by players, even if a game has already started.
	spectalk = strstr(Cvar_String("qwm_name"), "KTX") && Cvar_Value("k_spectalk");

	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (client->state != cs_spawned || (started && !client->spectator && !spectalk))
		{
			continue;
		}

		SV_ClientPrintf2(client, PRINT_CHAT, "%s\n", out);
	}
}

qbool SVC_BroadcastIsRateLimited(netadr_t *from)
{
	struct sockaddr_storage addr;
	struct sockaddr_in *addr_in;
	time_t now = time(NULL);
	char *ip_str = NULL;
	unsigned int ip = 0;
	int next = -1;
	int i = 0;

	NetadrToSockadr(from, &addr);
	addr_in = (struct sockaddr_in *)&addr;
	ip = ntohl(addr_in->sin_addr.s_addr);
	ip_str = NET_BaseAdrToString(*from);

	for (i = 0; i < BROADCAST_RATELIMIT_MAX_ENTRIES; i++)
	{
		if (ratelimit[i].ip == ip)
		{
			// Enough time has passed, let's reset the window and
			// counter for the given address.
			if (now - ratelimit[i].window_start >= BROADCAST_RATELIMIT_WINDOW_SECONDS)
			{
				Con_DPrintf("SVC_BroadcastIsRateLimited: Reset ratelimit for %s\n",
					ip_str);
				ratelimit[i].count = 1;
				ratelimit[i].window_start = now;
				return false;
			}

			// The rate limit has been reached for the given IP,
			// let's reject it.
			if (ratelimit[i].count >= BROADCAST_RATELIMIT_LIMIT)
			{
				Con_DPrintf("SVC_BroadcastIsRateLimited: %s has been ratelimited\n",
					ip_str);
				return true;
			}

			// Increment the counter and proceed.
			ratelimit[i].count++;
			return false;
		}

		// Check whether or not we can evict the entry for this IP.
		if (ratelimit[i].ip != 0 &&
			now - ratelimit[i].window_start >= BROADCAST_RATELIMIT_WINDOW_SECONDS)
		{
			ratelimit[i].ip = 0;
			ratelimit[i].count = 0;
			ratelimit[i].window_start = 0;
		}

		// If next isn't set and the spot is available, we'll use it for
		// the next ratelimit entry.
		if (next == -1 && ratelimit[i].ip == 0)
		{
			next = i;
		}
	}

	// No available slots exists in the ratelimit array, if that's the case,
	// we'll just have to say that the IP is going to be ratelimited.
	if (next == -1)
	{
		Con_DPrintf("SVC_BroadcastIsRateLimited: All slots are full, unable to process %s\n",
			ip_str);
		return true;
	}

	ratelimit[next].ip = ip;
	ratelimit[next].count = 1;
	ratelimit[next].window_start = now;
	return false;
}
