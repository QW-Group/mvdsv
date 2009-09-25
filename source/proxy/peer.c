/*
	peer.c
*/

#include "qwfwd.h"

peer_t *peers = NULL;
static int userid = 0;


peer_t	*FWD_peer_new(const char *remote_host, int remote_port, struct sockaddr_in *from, const char *userinfo, int qport, qbool link)
{
	peer_t *p;
	struct sockaddr_in to;
	int s;

	if (!NET_GetSockAddrIn_ByHostAndPort(&to, remote_host, remote_port))
		return NULL; // failed to resolve host name?

	if ((s = NET_UDP_OpenSocket(NULL, 0, false)) == INVALID_SOCKET)
		return NULL; // out of sockets?

	p = Sys_malloc(sizeof(*p));
	p->s		= s;
	p->from		= *from;
	p->to		= to;
	p->ps		= ps_challenge;
	p->qport	= qport;
	strlcpy(p->userinfo, userinfo, sizeof(p->userinfo));
	Info_ValueForKey(userinfo, "name", p->name, sizeof(p->name));
	p->userid	= ++userid;

	time(&p->last);

	if (link)
	{
		p->next = peers;
		peers = p;
	}

	return p;
}

// free peer data, perform unlink if requested
static void FWD_peer_free(peer_t *peer, qbool unlink)
{
	if (!peer)
		return;

	if (unlink)
	{
		peer_t *next, *prev, *current;

		prev = NULL;
		current = peers;

		for ( ; current; )
		{
			next = current->next;

			if (peer == current)
			{
				if (prev)
					prev->next = next;
				else
					peers = next;

				break;
			}

			prev = current;
			current = next;
		}
	}

	// free all data related to peer
	if (peer->s) // there should be no zero socket, it's stdin
		closesocket(peer->s);
	Sys_free(peer);
}

static void FWD_check_timeout(void)
{
	time_t cur_time;
	peer_t *p;

	cur_time = time(NULL);

	for (p = peers; p; p = p->next)
	{
		if (cur_time - p->last < 15) // few seconds timeout
			continue;

		Sys_Printf("peer %s:%d timed out\n", inet_ntoa(p->from.sin_addr), (int)ntohs(p->from.sin_port));

		p->ps = ps_drop;
	}
}

static void FWD_check_drop(void)
{
	peer_t *p, *next;

	for (p = peers; p; p = next)
	{
		next = p->next;

		if (p->ps != ps_drop)
			continue;

		Sys_Printf("peer %s:%d dropped\n", inet_ntoa(p->from.sin_addr), (int)ntohs(p->from.sin_port));
		FWD_peer_free(p, true); // NOTE: 'p' is not valid after this function, so we remember 'next' before this function
	}
}

static void FWD_network_update(void)
{
	fd_set rfds;
	struct timeval tv;
	int retval;
	int i1;
	peer_t *p;

	FD_ZERO(&rfds);

	// select on main server socket
	FD_SET(net_socket, &rfds);
	i1 = net_socket + 1;

	for (p = peers; p; p = p->next)
	{
		// select on peers sockets
		FD_SET(p->s, &rfds);
		if (p->s >= i1)
			i1 = p->s + 1;
	}

	/* Sleep for some time, wake up immidiately if there input packet. */
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 100 ms
	retval = select(i1, &rfds, (fd_set *)0, (fd_set *)0, &tv);

	if (retval <= 0)
		return;

	// if we have input packet on main server/proxy socket, then read it
	if(FD_ISSET(net_socket, &rfds))
	{
		// read it
		for(;;)
		{
			if (!NET_GetPacket(net_socket, &net_message))
				break;

			if (net_message.cursize == 1 && net_message.data[0] == A2A_ACK)
			{
				QRY_SV_PingReply();

				continue;
			}

			MSG_BeginReading();
			if (MSG_ReadLong() == -1)
			{
				if (!MSG_BadRead())
					SV_ConnectionlessPacket();

				continue;
			}

			// search in peers
			for (p = peers; p; p = p->next)
			{
				// we have this peer alredy, so forward/send packet to remote server
				if (NET_CompareAddress(&p->from, &net_from))
					break;
			}

			if (!p)
				continue;

			if (p->ps >= ps_connected)
				NET_SendPacket(p->s, net_message.cursize, net_message.data, &p->to);

			time(&p->last);
		}
	}

	// now lets check peers sockets, perhaps we have input packets too
	for (p = peers; p; p = p->next)
	{
		if(FD_ISSET(p->s, &rfds))
		{
			// yeah, we have packet, read it then
			for (;;)
			{
				if (!NET_GetPacket(p->s, &net_message))
					break;

				// we should check is this packet from remote server, this may be some evil packet from haxors...
				if (!NET_CompareAddress(&p->to, &net_from))
					continue;

				MSG_BeginReading();
				if (MSG_ReadLong() == -1)
				{
					if (!MSG_BadRead())
						CL_ConnectionlessPacket(p);

					continue;
				}

				if (p->ps >= ps_connected)
					NET_SendPacket(net_socket, net_message.cursize, net_message.data, &p->from);

				time(&p->last);
			} // for (;;)
		} // if(FD_ISSET(p->s, &rfds))

		if (p->ps == ps_challenge)
		{
			// send challenge time to time
			if (time(NULL) - p->connect > 2)
			{
				p->connect = time(NULL);
				Netchan_OutOfBandPrint(p->s, &p->to, "getchallenge\n");
			}
		}
	} // for (p = peers; p; p = p->next)
}

//======================================================

void FWD_update_peers(void)
{
	FWD_network_update();
	FWD_check_timeout();
	FWD_check_drop();
}

