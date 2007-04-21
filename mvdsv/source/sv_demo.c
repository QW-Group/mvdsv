/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    $Id: sv_demo.c,v 1.74 2007/04/21 16:28:26 disconn3ct Exp $
*/

#include "qwsvdef.h"

// last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
static char *lastdemosname[16];
static int lastdemospos;

#define demo_size_padding 0x1000

//qtv proxies are meant to send a small header now, bit like http
//this header gives supported version numbers and stuff
typedef struct mvdpendingdest_s
{
	qbool error; //disables writers, quit ASAP.
	int socket;

	char inbuffer[2048];
	char outbuffer[2048];

	char challenge[64];
	int hasauthed;

	int insize;
	int outsize;

	struct mvdpendingdest_s *nextdest;
} mvdpendingdest_t;

typedef struct mvddest_s
{
	qbool error; //disables writers, quit ASAP.

	enum {DEST_NONE, DEST_FILE, DEST_BUFFEREDFILE, DEST_STREAM} desttype;

	int socket;
	FILE *file;

	char name[MAX_QPATH];
	char path[MAX_QPATH];

	char *cache;
	int cacheused;
	int maxcachesize;

	unsigned int totalsize;

	struct mvddest_s *nextdest;
} mvddest_t;

mvddest_t *singledest;
static mvddest_t *SV_InitStream (int socket1);
static qbool SV_MVD_Record (mvddest_t *dest);

#define MAXSIZE (demobuffer->end < demobuffer->last ? \
				demobuffer->start - demobuffer->end : \
				demobuffer->maxsize - demobuffer->end)

cvar_t	sv_demoUseCache = {"sv_demoUseCache", "0"};
cvar_t	sv_demoCacheSize = {"sv_demoCacheSize", "0", CVAR_ROM};
cvar_t	sv_demoMaxDirSize = {"sv_demoMaxDirSize", "102400"};
cvar_t	sv_demoClearOld = {"sv_demoClearOld", "0"}; //bliP: 24/9 clear old demos
qbool	sv_demoDir_OnChange(cvar_t *cvar, char *value);
cvar_t	sv_demoDir = {"sv_demoDir", "demos", 0, sv_demoDir_OnChange};
cvar_t	sv_demofps = {"sv_demofps", "30"};
cvar_t	sv_demoPings = {"sv_demopings", "3"};
cvar_t	sv_demoNoVis = {"sv_demonovis", "1"};
cvar_t	sv_demoMaxSize  = {"sv_demoMaxSize", "20480"};
cvar_t	sv_demoExtraNames = {"sv_demoExtraNames", "0"};

cvar_t	qtv_streamport = {"qtv_streamport", "0"};
cvar_t	qtv_maxstreams = {"qtv_maxstreams", "1"};
cvar_t	qtv_password = {"qtv_password", ""};

cvar_t	sv_demoPrefix = {"sv_demoPrefix", ""};
cvar_t	sv_demoSuffix = {"sv_demoSuffix", ""};
cvar_t	sv_demotxt = {"sv_demotxt", "1"};
cvar_t	sv_onrecordfinish = {"sv_onRecordFinish", ""};

cvar_t	sv_ondemoremove = {"sv_onDemoRemove", ""};
cvar_t	sv_demoRegexp = {"sv_demoRegexp", "\\.mvd(\\.(gz|bz2|rar|zip))?$"};

static void SV_WriteMVDMessage (sizebuf_t *msg, int type, int to, float time1);

static dbuffer_t *demobuffer;
static int header = (char *)&((header_t*)0)->data - (char *)NULL;

entity_state_t demo_entities[UPDATE_MASK+1][MAX_DEMO_PACKET_ENTITIES];
client_frame_t demo_frames[UPDATE_MASK+1];

// only one .. is allowed (security)
qbool sv_demoDir_OnChange (cvar_t *cvar, char *value)
{
	if (!value[0])
		return true;

	if (value[0] == '.' && value[1] == '.')
		value += 2;
	if (strstr(value,"/.."))
		return true;

	return false;
}

static mvddest_t *DestByName (char *name)
{
	mvddest_t *d;

	for (d = demo.dest; d; d = d->nextdest)
		if (!strncmp(name, d->name, sizeof(d->name)-1))
			return d;

	return NULL;
}

static void DestClose (mvddest_t *d, qbool destroyfiles)
{
	char path[MAX_OSPATH];

	if (d->cache)
		Q_free(d->cache);
	if (d->file)
		fclose(d->file);
	if (d->socket)
		closesocket(d->socket);

	if (destroyfiles)
	{
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, d->path, d->name);
		Sys_remove(path);
		strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);
		Sys_remove(path);
	}

	Q_free(d);
}

void DestFlush (qbool compleate)
{
	int len;
	mvddest_t *d, *t;

	if (!demo.dest)
		return;
	while (demo.dest->error)
	{
		d = demo.dest;
		demo.dest = d->nextdest;

		DestClose(d, false);

		if (!demo.dest)
		{
			SV_MVDStop(3, false);
			return;
		}
	}
	for (d = demo.dest; d; d = d->nextdest)
	{
		switch(d->desttype)
		{
		case DEST_FILE:
			fflush (d->file);
			break;
		case DEST_BUFFEREDFILE:
			if (d->cacheused+demo_size_padding > d->maxcachesize || compleate)
			{
				len = fwrite(d->cache, 1, d->cacheused, d->file);
				if (len < d->cacheused)
					d->error = true;
				fflush(d->file);

				d->cacheused = 0;
			}
			break;

		case DEST_STREAM:
			if (d->cacheused && !d->error)
			{
				len = send(d->socket, d->cache, d->cacheused, 0);
				if (len == 0) //client died
					d->error = true;
				else if (len > 0) //we put some data through
				{ //move up the buffer
					d->cacheused -= len;
					memmove(d->cache, d->cache+len, d->cacheused);
				}
				else
				{ //error of some kind. would block or something
					if (qerrno != EWOULDBLOCK)
						d->error = true;
				}
			}
			break;

		case DEST_NONE:
			Sys_Error("DestFlush encoundered bad dest.");
		}

		if ((unsigned int)sv_demoMaxSize.value && d->totalsize > ((unsigned int)sv_demoMaxSize.value * 1024))
			d->error = 2;	//abort, but don't kill it.

		while (d->nextdest && d->nextdest->error)
		{
			t = d->nextdest;
			d->nextdest = t->nextdest;

			DestClose(t, false);
		}
	}
}

void SV_MVD_RunPendingConnections (void)
{
	unsigned short ushort_result;
	char *e;
	int len;
	mvdpendingdest_t *p;
	mvdpendingdest_t *np;

	if (!demo.pendingdest)
		return;

	while (demo.pendingdest && demo.pendingdest->error)
	{
		np = demo.pendingdest->nextdest;

		if (demo.pendingdest->socket != -1)
			closesocket(demo.pendingdest->socket);
		Q_free(demo.pendingdest);
		demo.pendingdest = np;
	}

	for (p = demo.pendingdest; p && p->nextdest; p = p->nextdest)
	{
		if (p->nextdest->error)
		{
			np = p->nextdest->nextdest;
			if (p->nextdest->socket != -1)
				closesocket(p->nextdest->socket);
			Q_free(p->nextdest);
			p->nextdest = np;
		}
	}

	for (p = demo.pendingdest; p; p = p->nextdest)
	{
		if (p->outsize && !p->error)
		{
			len = send(p->socket, p->outbuffer, p->outsize, 0);
			if (len == 0) //client died
				p->error = true;
			else if (len > 0)	//we put some data through
			{	//move up the buffer
				p->outsize -= len;
				memmove(p->outbuffer, p->outbuffer+len, p->outsize );
			}
			else
			{ //error of some kind. would block or something
				if (qerrno != EWOULDBLOCK)
					p->error = true;
			}
		}
		if (!p->error)
		{
			len = recv(p->socket, p->inbuffer + p->insize, sizeof(p->inbuffer) - p->insize - 1, 0);
			if (len > 0)
			{ //fixme: cope with extra \rs
				char *end;
				p->insize += len;
				p->inbuffer[p->insize] = 0;

				for (end = p->inbuffer; ; end++)
				{
					if (*end == '\0')
					{
						end = NULL;
						break;	//not enough data
					}

					if (end[0] == '\n')
					{
						if (end[1] == '\n')
						{
							end[1] = '\0';
							break;
						}
					}
				}
				if (end)
				{ //we found the end of the header
					char *start, *lineend;
					int versiontouse = 0;
					int raw = 0;
					char password[256] = "";
					enum {
						QTVAM_NONE,
						QTVAM_PLAIN,
						QTVAM_CCITT,
						QTVAM_MD4,
						QTVAM_MD5,
					} authmethod = QTVAM_NONE;

					start = p->inbuffer;

					lineend = strchr(start, '\n');
					if (!lineend)
					{
//						char *e;
//						e = "This is a QTV server.";
//						send(p->socket, e, strlen(e), 0);

						p->error = true;
						continue;
					}
					*lineend = '\0';
					COM_ParseToken(start, NULL);
					start = lineend+1;
					if (strcmp(com_token, "QTV"))
					{ //it's an error if it's not qtv.
						p->error = true;
						lineend = strchr(start, '\n');
						continue;
					}

					for(;;)
					{
						lineend = strchr(start, '\n');
						if (!lineend)
							break;
						*lineend = '\0';
						start = COM_ParseToken(start, NULL);
						if (*start == ':')
						{
//VERSION: a list of the different qtv protocols supported. Multiple versions can be specified. The first is assumed to be the prefered version.
//RAW: if non-zero, send only a raw mvd with no additional markup anywhere (for telnet use). Doesn't work with challenge-based auth, so will only be accepted when proxy passwords are not required.
//AUTH: specifies an auth method, the exact specs varies based on the method
//		PLAIN: the password is sent as a PASSWORD line
//		MD4: the server responds with an "AUTH: MD4\n" line as well as a "CHALLENGE: somerandomchallengestring\n" line, the client sends a new 'initial' request with CHALLENGE: MD4\nRESPONSE: hexbasedmd4checksumhere\n"
//		MD5: same as md4
//		CCITT: same as md4, but using the CRC stuff common to all quake engines.
//		if the supported/allowed auth methods don't match, the connection is silently dropped.
//SOURCE: which stream to play from, DEFAULT is special. Without qualifiers, it's assumed to be a tcp address.
//COMPRESSION: Suggests a compression method (multiple are allowed). You'll get a COMPRESSION response, and compression will begin with the binary data.

							start = start+1;
							Con_Printf("qtv, got (%s) (%s)\n", com_token, start);
							if (!strcmp(com_token, "VERSION"))
							{
								start = COM_ParseToken(start, NULL);
								if (atoi(com_token) == 1)
									versiontouse = 1;
							}
							else if (!strcmp(com_token, "RAW"))
							{
								start = COM_ParseToken(start, NULL);
								raw = atoi(com_token);
							}
							else if (!strcmp(com_token, "PASSWORD"))
							{
								start = COM_ParseToken(start, NULL);
								strlcpy(password, com_token, sizeof(password));
							}
							else if (!strcmp(com_token, "AUTH"))
							{
								unsigned int thisauth;
								start = COM_ParseToken(start, NULL);
								if (!strcmp(com_token, "NONE"))
									thisauth = QTVAM_PLAIN;
								else if (!strcmp(com_token, "PLAIN"))
									thisauth = QTVAM_PLAIN;
								else if (!strcmp(com_token, "CCIT"))
									thisauth = QTVAM_CCITT;
								else if (!strcmp(com_token, "MD4"))
									thisauth = QTVAM_MD4;
//								else if (!strcmp(com_token, "MD5"))
//									thisauth = QTVAM_MD5;
								else
								{
									thisauth = QTVAM_NONE;
									Con_DPrintf("qtv: received unrecognised auth method (%s)\n", com_token);
								}

								if (authmethod < thisauth)
									authmethod = thisauth;
							}
							else if (!strcmp(com_token, "SOURCE"))
							{
								//servers don't support source, and ignore it.
								//source is only useful for qtv proxy servers.
							}
							else if (!strcmp(com_token, "COMPRESSION"))
							{
								//compression not supported yet
							}
							else
							{
								//not recognised.
							}
						}
						start = lineend+1;
					}

					len = (end - p->inbuffer)+2;
					p->insize -= len;
					memmove(p->inbuffer, p->inbuffer + len, p->insize);
					p->inbuffer[p->insize] = 0;

					e = NULL;
					if (p->hasauthed)
					{
					}
					else if (!*qtv_password.string)
						p->hasauthed = true; //no password, no need to auth.
					else if (*password)
					{
						switch (authmethod)
						{
						case QTVAM_NONE:
							e = ("QTVSV 1\n"
								 "PERROR: You need to provide a common auth method.\n\n");
							break;
						case QTVAM_PLAIN:
							p->hasauthed = !strcmp(qtv_password.string, password);
							break;
						case QTVAM_CCITT:
							CRC_Init(&ushort_result);
							CRC_AddBlock(&ushort_result, (byte *) p->challenge, strlen(p->challenge));
							CRC_AddBlock(&ushort_result, (byte *) qtv_password.string, strlen(qtv_password.string));
							p->hasauthed = (ushort_result == atoi(password));
							break;
						case QTVAM_MD4:
							{
								char hash[512];
								int md4sum[4];
								
								snprintf (hash, sizeof(hash), "%s%s", p->challenge, qtv_password.string);
								Com_BlockFullChecksum (hash, strlen(hash), (unsigned char*)md4sum);
								snprintf (hash, sizeof(hash), "%X%X%X%X", md4sum[0], md4sum[1], md4sum[2], md4sum[3]);
								p->hasauthed = !strcmp(password, hash);
							}
							break;
						case QTVAM_MD5:
						default:
							e = ("QTVSV 1\n"
								 "PERROR: FTEQWSV bug detected.\n\n");
							break;
						}
						if (!p->hasauthed && !e)
						{
							if (raw)
								e = "";
							else
								e = ("QTVSV 1\n"
									 "PERROR: Bad password.\n\n");
						}
					}
					else
					{
						//no password, and not automagically authed
						switch (authmethod)
						{
						case QTVAM_NONE:
							if (raw)
								e = "";
							else
								e = ("QTVSV 1\n"
									 "PERROR: You need to provide a common auth method.\n\n");
							break;
						case QTVAM_PLAIN:
							p->hasauthed = !strcmp(qtv_password.string, password);
							break;

							if (0)
							{
						case QTVAM_CCITT:
									e =	("QTVSV 1\n"
										"AUTH: CCITT\n"
										"CHALLENGE: ");
							}
							else if (0)
							{
						case QTVAM_MD4:
									e = ("QTVSV 1\n"
										"AUTH: MD4\n"
										"CHALLENGE: ");
							}
							else
							{
						case QTVAM_MD5:
									e = ("QTVSV 1\n"
										"AUTH: MD5\n"
										"CHALLENGE: ");
							}

							send(p->socket, e, strlen(e), 0);
							send(p->socket, p->challenge, strlen(p->challenge), 0);
							e = "\n\n";
							send(p->socket, e, strlen(e), 0);
							continue;

						default:
							e = ("QTVSV 1\n"
								 "PERROR: FTEQWSV bug detected.\n\n");
							break;
						}
					}

					if (e)
					{
					}
					else if (!versiontouse)
					{
						e = ("QTVSV 1\n"
							 "PERROR: Incompatable version (valid version is v1)\n\n");
					}
					else if (raw)
					{
						if (p->hasauthed == false)
						{
							e =	"";
						}
						else
						{
							SV_MVD_Record(SV_InitStream(p->socket));
							p->socket = -1;	//so it's not cleared wrongly.
						}
						p->error = true;
					}
					else
					{
						if (p->hasauthed == true)
						{
							e = ("QTVSV 1\n"
								 "BEGIN\n"
								 "\n");
							send(p->socket, e, strlen(e), 0);
							e = NULL;
							SV_MVD_Record(SV_InitStream(p->socket));
							p->socket = -1;	//so it's not cleared wrongly.
						}
						else
						{
							e = ("QTVSV 1\n"
								"PERROR: You need to provide a password.\n\n");
						}
						p->error = true;
					}

					if (e)
					{
						send(p->socket, e, strlen(e), 0);
						p->error = true;
					}
				}
			}
			else if (len == 0)
				p->error = true;
			else
			{	//error of some kind. would block or something
				int err;
				err = qerrno;
				if (err != EWOULDBLOCK)
					p->error = true;
			}
		}
	}
}

static char *SV_PrintTeams(void);
static void Run_sv_demotxt_and_sv_onrecordfinish (mvddest_t *d, qbool destroyfiles)
{
	char path[MAX_OSPATH];
	
	snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, d->path, d->name);
	strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);

	if ((int)sv_demotxt.value && !destroyfiles) // dont keep txt's for deleted demos
	{
		FILE *f;
		char *text;

		if (sv_demotxt.value == 2)
		{
			if ((f = fopen (path, "a+t")))
				fclose(f); // at least made empty file, but do not owerwite
		}
		else if ((f = fopen (path, "w+t")))
		{
			text = SV_PrintTeams();
			fwrite(text, strlen(text), 1, f);
			fflush(f);
			fclose(f);
		}
	}

	if (sv_onrecordfinish.string[0] && !destroyfiles) // dont gzip deleted demos
	{
		extern redirect_t sv_redirected;
		int old = sv_redirected;
		char *p;
	
		if ((p = strstr(sv_onrecordfinish.string, " ")) != NULL)
			*p = 0; // strip parameters
	
		strlcpy(path, d->name, MAX_OSPATH);
		strlcpy(path + strlen(d->name) - 3, "txt", MAX_OSPATH - strlen(d->name) + 3);
	
		sv_redirected = RD_NONE; // onrecord script is called always from the console
		Cmd_TokenizeString(va("script %s \"%s\" \"%s\" \"%s\" %s", sv_onrecordfinish.string, d->path, d->name, path, p != NULL ? p+1 : ""));
		if (p) *p = ' ';
		SV_Script_f();
	
		sv_redirected = old;
	}
}

static int DestCloseAllFlush (qbool destroyfiles, qbool mvdonly)
{
	int numclosed = 0;
	mvddest_t *d, **prev, *next;
	DestFlush(true); //make sure it's all written.

	prev = &demo.dest;
	d = demo.dest;
	while (d)
	{
		next = d->nextdest;
		if (!mvdonly || d->desttype != DEST_STREAM)
		{
			*prev = d->nextdest;
			DestClose(d, destroyfiles);
			Run_sv_demotxt_and_sv_onrecordfinish (d, destroyfiles);
			numclosed++;
		}
		else
			prev = &d->nextdest;

		d = next;
	}

	return numclosed;
}

static int DemoWriteDest (void *data, int len, mvddest_t *d)
{
	if (d->error)
		return 0;
	d->totalsize += len;
	switch(d->desttype)
	{
		case DEST_FILE:
			fwrite(data, len, 1, d->file);
			break;
		case DEST_BUFFEREDFILE:	//these write to a cache, which is flushed later
		case DEST_STREAM:
			if (d->cacheused+len > d->maxcachesize)
			{
				d->error = true;
				return 0;
			}
			memcpy(d->cache+d->cacheused, data, len);
			d->cacheused += len;
			break;
		case DEST_NONE:
			Sys_Error("DemoWriteDest encoundered bad dest.");
	}
	return len;
}

int DemoWrite (void *data, int len) //broadcast to all proxies/mvds
{
	mvddest_t *d;
	for (d = demo.dest; d; d = d->nextdest)
	{
		if (singledest && singledest != d)
			continue;
		DemoWriteDest(data, len, d);
	}
	return len;
}

void DemoWriteQTVTimePad (int msecs)	//broadcast to all proxies
{
	mvddest_t *d;
	unsigned char buffer[6];
	while (msecs > 0)
	{
		//duration
		if (msecs > 255)
			buffer[0] = 255;
		else
			buffer[0] = msecs;
		msecs -= buffer[0];
		//message type
		buffer[1] = dem_read;
		//length
		buffer[2] = 0;
		buffer[3] = 0;
		buffer[4] = 0;
		buffer[5] = 0;

		for (d = demo.dest; d; d = d->nextdest)
		{
			if (d->desttype == DEST_STREAM)
			{
				DemoWriteDest(buffer, sizeof(buffer), d);
			}
		}
	}
}

void SV_MVDPings (void)
{
	client_t *client;
	int j;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		MVDWrite_Begin (dem_all, 0, 7);
		MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updateping);
		MSG_WriteByte((sizebuf_t*)demo.dbuf,  j);
		MSG_WriteShort((sizebuf_t*)demo.dbuf,  SV_CalcPing(client));
		MSG_WriteByte((sizebuf_t*)demo.dbuf, svc_updatepl);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, j);
		MSG_WriteByte ((sizebuf_t*)demo.dbuf, client->lossage);
	}
}

void MVDBuffer_Init (dbuffer_t *dbuffer, byte *buf, size_t size)
{
	demobuffer = dbuffer;

	demobuffer->data = buf;
	demobuffer->maxsize = size;
	demobuffer->start = 0;
	demobuffer->end = 0;
	demobuffer->last = 0;
}

/*
==============
MVDSetMsgBuf

Sets the frame message buffer
==============
*/

void MVDSetMsgBuf (demobuf_t *prev,demobuf_t *cur)
{
	// fix the maxsize of previous msg buffer,
	// we won't be able to write there anymore
	if (prev != NULL)
		prev->maxsize = prev->bufsize;

	demo.dbuf = cur;
	memset(demo.dbuf, 0, sizeof(*demo.dbuf));

	demo.dbuf->data = demobuffer->data + demobuffer->end;
	demo.dbuf->maxsize = MAXSIZE;
}

/*
==============
SV_MVDWriteToDisk

Writes to disk a message meant for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

void SV_MVDWriteToDisk (int type, int to, float time1)
{
	int pos = 0, oldm, oldd;
	header_t *p;
	int size;
	sizebuf_t msg;

	p = (header_t *)demo.dbuf->data;
	demo.dbuf->h = NULL;

	oldm = demo.dbuf->bufsize;
	oldd = demobuffer->start;
	while (pos < demo.dbuf->bufsize)
	{
		size = p->size;
		pos += header + size;

		// no type means we are writing to disk everything
		if (!type || (p->type == type && p->to == to))
		{
			if (size)
			{
				msg.data = p->data;
				msg.cursize = size;

				SV_WriteMVDMessage(&msg, p->type, p->to, time1);
			}

			// data is written so it need to be cleard from demobuf
			if (demo.dbuf->data != (byte*)p)
				memmove(demo.dbuf->data + size + header, demo.dbuf->data, (byte*)p - demo.dbuf->data);

			demo.dbuf->bufsize -= size + header;
			demo.dbuf->data += size + header;
			pos -= size + header;
			demo.dbuf->maxsize -= size + header;
			demobuffer->start += size + header;
		}
		// move along
		p = (header_t *)(p->data + size);
	}

	if (demobuffer->start == demobuffer->last)
	{
		if (demobuffer->start == demobuffer->end)
		{
			demobuffer->end = 0; // demobuffer is empty
			demo.dbuf->data = demobuffer->data;
		}

		// go back to begining of the buffer
		demobuffer->last = demobuffer->end;
		demobuffer->start = 0;
	}
}

/*
==============
MVDSetBuf

Sets position in the buf for writing to specific client
==============
*/
static void MVDSetBuf (byte type, int to)
{
	header_t *p;
	int pos = 0;

	p = (header_t *)demo.dbuf->data;

	while (pos < demo.dbuf->bufsize)
	{
		pos += header + p->size;

		if (type == p->type && to == p->to && !p->full)
		{
			demo.dbuf->cursize = pos;
			demo.dbuf->h = p;
			return;
		}

		p = (header_t *)(p->data + p->size);
	}
	// type&&to not exist in the buf, so add it

	p->type = type;
	p->to = to;
	p->size = 0;
	p->full = 0;

	demo.dbuf->bufsize += header;
	demo.dbuf->cursize = demo.dbuf->bufsize;
	demobuffer->end += header;
	demo.dbuf->h = p;
}

void MVDMoveBuf (void)
{
	// set the last message mark to the previous frame (i/e begining of this one)
	demobuffer->last = demobuffer->end - demo.dbuf->bufsize;

	// move buffer to the begining of demo buffer
	memmove(demobuffer->data, demo.dbuf->data, demo.dbuf->bufsize);
	demo.dbuf->data = demobuffer->data;
	demobuffer->end = demo.dbuf->bufsize;
	demo.dbuf->h = NULL; // it will be setup again
	demo.dbuf->maxsize = MAXSIZE + demo.dbuf->bufsize;
}

void MVDWrite_Begin (byte type, int to, int size)
{
	byte *p;
	qbool move = false;

	// will it fit?
	while (demo.dbuf->bufsize + size + header > demo.dbuf->maxsize)
	{
		// if we reached the end of buffer move msgbuf to the begining
		if (!move && demobuffer->end > demobuffer->start)
			move = true;

		SV_MVDWritePackets(1);
		if (move && demobuffer->start > demo.dbuf->bufsize + header + size)
			MVDMoveBuf();
	}

	if (demo.dbuf->h == NULL || demo.dbuf->h->type != type || demo.dbuf->h->to != to || demo.dbuf->h->full)
	{
		MVDSetBuf(type, to);
	}

	if (demo.dbuf->h->size + size > MAX_MSGLEN)
	{
		demo.dbuf->h->full = 1;
		MVDSetBuf(type, to);
	}

	// we have to make room for new data
	if (demo.dbuf->cursize != demo.dbuf->bufsize)
	{
		p = demo.dbuf->data + demo.dbuf->cursize;
		memmove(p+size, p, demo.dbuf->bufsize - demo.dbuf->cursize);
	}

	demo.dbuf->bufsize += size;
	demo.dbuf->h->size += size;
	if ((demobuffer->end += size) > demobuffer->last)
		demobuffer->last = demobuffer->end;
}

/*
====================
SV_WriteMVDMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
static void SV_WriteMVDMessage (sizebuf_t *msg, int type, int to, float time1)
{
	int		len, i, msec;
	byte	c;
	static double prevtime;

	if (!sv.mvdrecording)
		return;

	msec = (time1 - prevtime)*1000;
	prevtime += msec*0.001;
	if (msec > 255) msec = 255;
	if (msec < 2) msec = 0;

	c = msec;
	DemoWrite(&c, sizeof(c));

	if (demo.lasttype != type || demo.lastto != to)
	{
		demo.lasttype = type;
		demo.lastto = to;
		switch (demo.lasttype)
		{
		case dem_all:
			c = dem_all;
			DemoWrite (&c, sizeof(c));
			break;
		case dem_multiple:
			c = dem_multiple;
			DemoWrite (&c, sizeof(c));

			i = LittleLong(demo.lastto);
			DemoWrite (&i, sizeof(i));
			break;
		case dem_single:
		case dem_stats:
			c = demo.lasttype + (demo.lastto << 3);
			DemoWrite (&c, sizeof(c));
			break;
		default:
			SV_MVDStop_f ();
			Con_Printf("bad demo message type:%d", type);
			return;
		}
	}
	else
	{
		c = dem_read;
		DemoWrite (&c, sizeof(c));
	}


	len = LittleLong (msg->cursize);
	DemoWrite (&len, 4);
	DemoWrite (msg->data, msg->cursize);

	DestFlush(false);
}


/*
====================
SV_MVDWritePackets

Interpolates to get exact players position for current frame
and writes packets to the disk/memory
====================
*/
static float adjustangle (float current, float ideal, float fraction) // FIXME move to bothtools.c ?
{
	float move;

	move = ideal - current;
	if (ideal > current)
	{

		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	move *= fraction;

	return (current + move);
}

#define DF_ORIGIN		1
#define DF_ANGLES		(1<<3)
#define DF_EFFECTS		(1<<6)
#define DF_SKINNUM		(1<<7)
#define DF_DEAD			(1<<8)
#define DF_GIB			(1<<9)
#define DF_WEAPONFRAME	(1<<10)
#define DF_MODEL		(1<<11)

void SV_MVDWritePackets (int num)
{
	demo_frame_t	*frame, *nextframe;
	demo_client_t	*cl, *nextcl = NULL;
	int				i, j, flags;
	qbool			valid;
	double			time1, playertime, nexttime;
	float			f;
	vec3_t			origin, angles;
	sizebuf_t		msg;
	byte			msg_buf[MAX_MSGLEN];
	demoinfo_t		*demoinfo;

	if (!sv.mvdrecording)
		return;

	msg.data = msg_buf;
	msg.maxsize = sizeof(msg_buf);

	if (num > demo.parsecount - demo.lastwritten + 1)
		num = demo.parsecount - demo.lastwritten + 1;

	// 'num' frames to write
	for ( ; num; num--, demo.lastwritten++)
	{
		frame = &demo.frames[demo.lastwritten&DEMO_FRAMES_MASK];
		time1 = frame->time;
		nextframe = frame;
		msg.cursize = 0;

		demo.dbuf = &frame->buf;

		// find two frames
		// one before the exact time (time - msec) and one after,
		// then we can interpolte exact position for current frame
		for (i = 0, cl = frame->clients, demoinfo = demo.info; i < MAX_CLIENTS; i++, cl++, demoinfo++)
		{
			if (cl->parsecount != demo.lastwritten)
				continue; // not valid

			nexttime = playertime = time1 - cl->sec;

			for (j = demo.lastwritten+1, valid = false; nexttime < time1 && j < demo.parsecount; j++)
			{
				nextframe = &demo.frames[j&DEMO_FRAMES_MASK];
				nextcl = &nextframe->clients[i];

				if (nextcl->parsecount != j)
					break; // disconnected?
				if (nextcl->fixangle)
					break; // respawned, or walked into teleport, do not interpolate!
				if (!(nextcl->flags & DF_DEAD) && (cl->flags & DF_DEAD))
					break; // respawned, do not interpolate

				nexttime = nextframe->time - nextcl->sec;

				if (nexttime >= time1)
				{
					// good, found what we were looking for
					valid = true;
					break;
				}
			}

			if (valid)
			{
				f = (time1 - nexttime)/(nexttime - playertime);
				for (j=0;j<3;j++)
				{
					angles[j] = adjustangle(cl->info.angles[j], nextcl->info.angles[j],1.0+f);
					origin[j] = nextcl->info.origin[j] + f*(nextcl->info.origin[j]-cl->info.origin[j]);
				}
			}
			else
			{
				VectorCopy(cl->info.origin, origin);
				VectorCopy(cl->info.angles, angles);
			}

			// now write it to buf
			flags = cl->flags;

			if (cl->fixangle)
			{
				demo.fixangletime[i] = cl->cmdtime;
			}

			for (j=0; j < 3; j++)
				if (origin[j] != demoinfo->origin[i])
					flags |= DF_ORIGIN << j;

			if (cl->fixangle || demo.fixangletime[i] != cl->cmdtime)
			{
				for (j=0; j < 3; j++)
					if (angles[j] != demoinfo->angles[j])
						flags |= DF_ANGLES << j;
			}

			if (cl->info.model != demoinfo->model)
				flags |= DF_MODEL;
			if (cl->info.effects != demoinfo->effects)
				flags |= DF_EFFECTS;
			if (cl->info.skinnum != demoinfo->skinnum)
				flags |= DF_SKINNUM;
			if (cl->info.weaponframe != demoinfo->weaponframe)
				flags |= DF_WEAPONFRAME;

			MSG_WriteByte (&msg, svc_playerinfo);
			MSG_WriteByte (&msg, i);
			MSG_WriteShort (&msg, flags);

			MSG_WriteByte (&msg, cl->frame);

			for (j=0 ; j<3 ; j++)
				if (flags & (DF_ORIGIN << j))
					MSG_WriteCoord (&msg, origin[j]);

			for (j=0 ; j<3 ; j++)
				if (flags & (DF_ANGLES << j))
					MSG_WriteAngle16 (&msg, angles[j]);


			if (flags & DF_MODEL)
				MSG_WriteByte (&msg, cl->info.model);

			if (flags & DF_SKINNUM)
				MSG_WriteByte (&msg, cl->info.skinnum);

			if (flags & DF_EFFECTS)
				MSG_WriteByte (&msg, cl->info.effects);

			if (flags & DF_WEAPONFRAME)
				MSG_WriteByte (&msg, cl->info.weaponframe);

			VectorCopy(cl->info.origin, demoinfo->origin);
			VectorCopy(cl->info.angles, demoinfo->angles);
			demoinfo->skinnum = cl->info.skinnum;
			demoinfo->effects = cl->info.effects;
			demoinfo->weaponframe = cl->info.weaponframe;
			demoinfo->model = cl->info.model;
		}

		SV_MVDWriteToDisk(demo.lasttype,demo.lastto, (float)time1); // this goes first to reduce demo size a bit
		SV_MVDWriteToDisk(0,0, (float)time1); // now goes the rest
		if (msg.cursize)
			SV_WriteMVDMessage(&msg, dem_all, 0, (float)time1);
	}

	if (demo.lastwritten > demo.parsecount)
		demo.lastwritten = demo.parsecount;

	demo.dbuf = &demo.frames[demo.parsecount&DEMO_FRAMES_MASK].buf;
	demo.dbuf->maxsize = MAXSIZE + demo.dbuf->bufsize;
}

static char chartbl[256];
void CleanName_Init ();

#define MIN_DEMO_MEMORY 0x100000
static void MVD_Init (void)
{
	int p, size = MIN_DEMO_MEMORY;
	
	Cvar_Register (&sv_demofps);
	Cvar_Register (&sv_demoPings);
	Cvar_Register (&sv_demoNoVis);
	Cvar_Register (&sv_demoUseCache);
	Cvar_Register (&sv_demoCacheSize);
	Cvar_Register (&sv_demoMaxSize);
	Cvar_Register (&sv_demoMaxDirSize);
	Cvar_Register (&sv_demoClearOld); //bliP: 24/9 clear old demos
	Cvar_Register (&sv_demoDir);
	Cvar_Register (&sv_demoPrefix);
	Cvar_Register (&sv_demoSuffix);
	Cvar_Register (&sv_onrecordfinish);
	Cvar_Register (&sv_ondemoremove);
	Cvar_Register (&sv_demotxt);
	Cvar_Register (&sv_demoExtraNames);
	Cvar_Register (&sv_demoRegexp);

	p = COM_CheckParm ("-democache");
	if (p)
	{
		if (p < com_argc-1)
			size = Q_atoi (com_argv[p+1]) * 1024;
		else
			Sys_Error ("MVD_Init: you must specify a size in KB after -democache");
	}

	if (size < MIN_DEMO_MEMORY)
	{
		Con_Printf("Minimum memory size for demo cache is %dk\n", MIN_DEMO_MEMORY / 1024);
		size = MIN_DEMO_MEMORY;
	}

	Cvar_SetROM(&sv_demoCacheSize, va("%d", size/1024));
	CleanName_Init();

	// clean last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
	for (p = 0; p < 16; p++)
		lastdemosname[p] = NULL;
	lastdemospos = 0;
}




void SV_TimeOfDay(date_t *date);
static char *SV_PrintTeams (void)
{
	char			*teams[MAX_CLIENTS], *p;
	int				i, j, numcl = 0, numt = 0, scores;
	client_t		*clients[MAX_CLIENTS];
	char			buf[2048];
	static char		lastscores[2048];
	extern cvar_t	teamplay;
	extern char		chartbl2[];
	date_t			date;
	SV_TimeOfDay(&date);

	// count teams and players
	for (i=0; i < MAX_CLIENTS; i++)
	{
		if (svs.clients[i].state != cs_spawned)
			continue;
		if (svs.clients[i].spectator)
			continue;

		clients[numcl++] = &svs.clients[i];
		for (j = 0; j < numt; j++)
			if (!strcmp(svs.clients[i].team, teams[j]))
				break;
		if (j != numt)
			continue;

		teams[numt++] = svs.clients[i].team;
	}

	// create output
	lastscores[0] = 0;
	snprintf(buf, sizeof(buf),
		"date %s\nmap %s\nteamplay %d\ndeathmatch %d\ntimelimit %d\n",
		date.str, sv.mapname, (int)teamplay.value, (int)deathmatch.value,
		(int)timelimit.value);
	if (numcl == 2) // duel
	{
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"player1: %s (%i)\nplayer2: %s (%i)\n",
			clients[0]->name, clients[0]->old_frags,
			clients[1]->name, clients[1]->old_frags);
		snprintf(lastscores, sizeof(lastscores), "duel: %s vs %s @ %s - %i:%i\n",
			clients[0]->name, clients[1]->name, sv.mapname,
			clients[0]->old_frags, clients[1]->old_frags);
	}
	else if (!(int)teamplay.value) // ffa
	{
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "players:\n");
		snprintf(lastscores, sizeof(lastscores), "ffa:");
		for (i = 0; i < numcl; i++)
		{
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"  %s (%i)\n", clients[i]->name, clients[i]->old_frags);
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"  %s(%i)", clients[i]->name, clients[i]->old_frags);
		}
		snprintf(lastscores + strlen(lastscores),
			sizeof(lastscores) - strlen(lastscores), " @ %s\n", sv.mapname);
	}
	else
	{ // teamplay
		snprintf(lastscores, sizeof(lastscores), "tp:");
		for (j = 0; j < numt; j++)
		{
			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"team[%i] %s:\n", j, teams[j]);
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"%s[", teams[j]);
			scores = 0;
			for (i = 0; i < numcl; i++)
				if (!strcmp(clients[i]->team, teams[j]))
				{
					snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
						"  %s (%i)\n", clients[i]->name, clients[i]->old_frags);
					snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
						" %s(%i) ", clients[i]->name, clients[i]->old_frags);
					scores += clients[i]->old_frags;
				}
			snprintf(lastscores + strlen(lastscores), sizeof(lastscores) - strlen(lastscores),
				"](%i)  ", scores);

		}
		snprintf(lastscores + strlen(lastscores),
			sizeof(lastscores) - strlen(lastscores), "@ %s\n", sv.mapname);
	}

	for (p = buf; *p; p++) *p = chartbl2[(byte)*p];
	for (p = lastscores; *p; p++) *p = chartbl2[(byte)*p];
	strlcat(lastscores, buf, sizeof(lastscores));
	return lastscores;
}

/*
====================
SV_InitRecord
====================
*/
static mvddest_t *SV_InitRecordFile (char *name)
{
	char *s;
	mvddest_t *dst;
	FILE *file;

	char path[MAX_OSPATH];

	file = fopen (name, "wb");
	if (!file)
	{
		Con_Printf ("ERROR: couldn't open \"%s\"\n", name);
		return NULL;
	}

	dst = (mvddest_t*) Q_malloc (sizeof(mvddest_t));

	if (!(int)sv_demoUseCache.value)
	{
		dst->desttype = DEST_FILE;
		dst->file = file;
		dst->maxcachesize = 0;
	}
	else
	{
		dst->desttype = DEST_BUFFEREDFILE;
		dst->file = file;
		dst->maxcachesize = (int) sv_demoCacheSize.value; // 0x81000
		dst->cache = (char *) Q_malloc (dst->maxcachesize);
	}

	s = name + strlen(name);
	while (*s != '/') s--;
	strlcpy(dst->name, s+1, sizeof(dst->name));
	strlcpy(dst->path, sv_demoDir.string, sizeof(dst->path));

// unused, and wrong anyway, since we do memset(&demo, 0, sizeof(demo)) after this...
//	if (!*demo.path)
//		strlcpy(demo.path, ".", MAX_OSPATH);

	SV_BroadcastPrintf (PRINT_CHAT, "Server starts recording (%s):\n%s\n",
						(dst->desttype == DEST_BUFFEREDFILE) ? "memory" : "disk", s+1);
	Cvar_SetROM(&serverdemo, dst->name);

	strlcpy(path, name, MAX_OSPATH);
	strlcpy(path + strlen(path) - 3, "txt", MAX_OSPATH - strlen(path) + 3);

	if ((int)sv_demotxt.value)
	{
		FILE *f;
		char *text;

		if (sv_demotxt.value == 2)
		{
			if ((f = fopen (path, "a+t")))
				fclose(f); // at least made empty file
		}
		else if ((f = fopen (path, "w+t")))
		{
			text = SV_PrintTeams();
			fwrite(text, strlen(text), 1, f);
			fflush(f);
			fclose(f);
		}
	}
	else
		Sys_remove(path);

	return dst;
}

static mvddest_t *SV_InitStream (int socket1)
{
	mvddest_t *dst;

	dst = (mvddest_t *) Q_malloc (sizeof(mvddest_t));

	dst->desttype = DEST_STREAM;
	dst->socket = socket1;
	dst->maxcachesize = 0x8000;	//is this too small?
	dst->cache = (char *) Q_malloc(dst->maxcachesize);

	SV_BroadcastPrintf (PRINT_CHAT, "Smile, you're on QTV!\n");

	return dst;
}

static void SV_MVD_InitPendingStream (int socket1, char *ip)
{
	mvdpendingdest_t *dst;
	unsigned int i;
	dst = (mvdpendingdest_t*) Q_malloc(sizeof(mvdpendingdest_t));
	dst->socket = socket1;

	strlcpy(dst->challenge, ip, sizeof(dst->challenge));
	for (i = strlen(dst->challenge); i < sizeof(dst->challenge)-1; i++)
		dst->challenge[i] = rand()%(127-33) + 33;	//generate a random challenge

	dst->nextdest = demo.pendingdest;
	demo.pendingdest = dst;
}

/*
====================
SV_MVDStop

stop recording a demo
====================
*/
void SV_MVDStop (int reason, qbool mvdonly)
{
	size_t name_len;
	int numclosed;

	if (!sv.mvdrecording)
	{
		Con_Printf ("Not recording a demo.\n");
		return;
	}

	if (reason == 2 || reason == 3)
	{
		DestCloseAllFlush(true, mvdonly);
		// stop and remove

		if (!demo.dest)
			sv.mvdrecording = false;

		if (reason == 3)
			SV_BroadcastPrintf (PRINT_CHAT, "QTV disconnected\n");
		else
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording canceled, demo removed\n");

		Cvar_SetROM(&serverdemo, "");

		return;
	}
	
	// write a disconnect message to the demo file
	// clearup to be sure message will fit
	demo.dbuf->cursize = 0;
	demo.dbuf->h = NULL;
	demo.dbuf->bufsize = 0;
	MVDWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte ((sizebuf_t*)demo.dbuf, svc_disconnect);
	MSG_WriteString ((sizebuf_t*)demo.dbuf, "EndOfDemo");

	SV_MVDWritePackets(demo.parsecount - demo.lastwritten + 1);
	// finish up

	// last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
	name_len = strlen(demo.dest->name) + 1;
	if (++lastdemospos > 15)
		lastdemospos &= 0xF;
	if (lastdemosname[lastdemospos])
		Q_free(lastdemosname[lastdemospos]);
	lastdemosname[lastdemospos] = Q_malloc(name_len);
	strlcpy(lastdemosname[lastdemospos], demo.dest->name, name_len);

	numclosed = DestCloseAllFlush(false, mvdonly);

	if (!demo.dest)
		sv.mvdrecording = false;
	if (numclosed)
	{
		if (!reason)
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording completed\n");
		else
			SV_BroadcastPrintf (PRINT_CHAT, "Server recording stoped\nMax demo size exceeded\n");
	}

	Cvar_SetROM(&serverdemo, "");
}

/*
====================
SV_MVDStop_f
====================
*/
void SV_MVDStop_f (void)
{
	SV_MVDStop(0, true);
}

/*
====================
SV_MVD_Cancel_f

Stops recording, and removes the demo
====================
*/
void SV_MVD_Cancel_f (void)
{
	SV_MVDStop(2, true);
}

/*
====================
SV_WriteMVDMessage

Dumps the current net message, prefixed by the length and view angles
====================
*/
static void SV_WriteRecordMVDMessage (sizebuf_t *msg, int seq)
{
	int len;
	byte c;

	if (!sv.mvdrecording)
		return;

	if (!msg->cursize)
		return;

	c = 0;
	DemoWrite (&c, sizeof(c));

	c = dem_read;
	DemoWrite (&c, sizeof(c));

	len = LittleLong (msg->cursize);
	DemoWrite (&len, 4);

	DemoWrite (msg->data, msg->cursize);

	DestFlush(false);
}

static void SV_WriteSetMVDMessage (void)
{
	int len;
	byte c;

	//Con_Printf("write: %ld bytes, %4.4f\n", msg->cursize, realtime);

	if (!sv.mvdrecording)
		return;

	c = 0;
	DemoWrite (&c, sizeof(c));

	c = dem_set;
	DemoWrite (&c, sizeof(c));


	len = LittleLong(0);
	DemoWrite (&len, 4);
	len = LittleLong(0);
	DemoWrite (&len, 4);

	DestFlush(false);
}

// mvd/qtv related stuff
// Well, here is a chance what player connect after demo recording started,
// so demo.info[edictnum - 1].model == player_model so SV_MVDWritePackets() will not wrote player model index,
// so client during playback this demo will got invisible model, because model index will be 0.
// Fixing that.
// Btw, struct demo contain different client specific structs, may be they need clearing too, not sure.
void MVD_PlayerReset(int player)
{
	if (player < 0 || player >= MAX_CLIENTS) { // protect from lamers
		Con_Printf("MVD_PlayerReset: wrong player num %d\n", player);
		return;
	}

	memset(&(demo.info[player]), 0, sizeof(demo.info[0]));
}

void SV_MVD_SendInitialGamestate(mvddest_t *dest);

static qbool SV_MVD_Record (mvddest_t *dest)
{
	int i;

	if (!dest)
		return false;

	DestFlush(true);

	if (!sv.mvdrecording)
	{
		memset(&demo, 0, sizeof(demo));
		for (i = 0; i < UPDATE_BACKUP; i++)
		{
			demo.recorder.frames[i].entities.entities = demo_entities[i];
		}

		MVDBuffer_Init(&demo.dbuffer, demo.buffer, sizeof(demo.buffer));
		MVDSetMsgBuf(NULL, &demo.frames[0].buf);

		demo.datagram.maxsize = sizeof(demo.datagram_data);
		demo.datagram.data = demo.datagram_data;
	}
	//	else
	//		SV_WriteRecordMVDMessage(&buf, dem_read);

	if (dest != demo.dest) {
		//
		// seems we initializing new dest
		//
		dest->nextdest = demo.dest;
		demo.dest = dest;

		SV_MVD_SendInitialGamestate(dest);
	}
	else
	{
		//
		// map change, sent initial stats to dests
		//
		SV_MVD_SendInitialGamestate(NULL);
	}

	// done
	return true;
}

// we change map - clear whole demo struct and sent initial state to all dest if any (for QTV only I thought)
qbool SV_MVD_Re_Record(void)
{
	return SV_MVD_Record (demo.dest);
}

void SV_MVD_SendInitialGamestate(mvddest_t *dest)
{
//	qbool first_dest = !sv.mvdrecording; // if we are not recording yet, that must be first dest
	sizebuf_t	buf;
	unsigned char buf_data[MAX_MSGLEN];
	unsigned int n;
	char *s, info[MAX_INFO_STRING];

	client_t *player;
	char *gamedir;
	int seq = 1, i;

	if (!demo.dest)
		return;


	sv.mvdrecording = true; // NOTE:  afaik wrongly set to false on map change, so restore it here
	
	
	demo.pingtime = demo.time = sv.time;


	singledest = dest;

	/*-------------------------------------------------*/

	// serverdata
	// send the info about the new client to all connected clients
	memset(&buf, 0, sizeof(buf));
	buf.data = buf_data;
	buf.maxsize = sizeof(buf_data);

	// send the serverdata

	gamedir = Info_ValueForKey (svs.info, "*gamedir");
	if (!gamedir[0])
		gamedir = "qw";

	MSG_WriteByte (&buf, svc_serverdata);
	MSG_WriteLong (&buf, PROTOCOL_VERSION);
	MSG_WriteLong (&buf, svs.spawncount);
	MSG_WriteString (&buf, gamedir);


	MSG_WriteFloat (&buf, sv.time);

	// send full levelname
	MSG_WriteString (&buf,
#ifdef USE_PR2
	                 PR2_GetString
#else
					 PR_GetString
#endif
	                (sv.edicts->v.message));

	// send the movevars
	MSG_WriteFloat(&buf, movevars.gravity);
	MSG_WriteFloat(&buf, movevars.stopspeed);
	MSG_WriteFloat(&buf, movevars.maxspeed);
	MSG_WriteFloat(&buf, movevars.spectatormaxspeed);
	MSG_WriteFloat(&buf, movevars.accelerate);
	MSG_WriteFloat(&buf, movevars.airaccelerate);
	MSG_WriteFloat(&buf, movevars.wateraccelerate);
	MSG_WriteFloat(&buf, movevars.friction);
	MSG_WriteFloat(&buf, movevars.waterfriction);
	MSG_WriteFloat(&buf, movevars.entgravity);

	// send music
	MSG_WriteByte (&buf, svc_cdtrack);
	MSG_WriteByte (&buf, 0); // none in demos

	// send server info string
	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("fullserverinfo \"%s\"\n", svs.info) );

	// flush packet
	SV_WriteRecordMVDMessage (&buf, seq++);
	SZ_Clear (&buf);

	// soundlist
	MSG_WriteByte (&buf, svc_soundlist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.sound_precache[n+1];
	while (s)
	{
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2)
		{
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
			MSG_WriteByte (&buf, svc_soundlist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.sound_precache[n+1];
	}

	if (buf.cursize)
	{
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// modellist
	MSG_WriteByte (&buf, svc_modellist);
	MSG_WriteByte (&buf, 0);

	n = 0;
	s = sv.model_precache[n+1];
	while (s)
	{
		MSG_WriteString (&buf, s);
		if (buf.cursize > MAX_MSGLEN/2)
		{
			MSG_WriteByte (&buf, 0);
			MSG_WriteByte (&buf, n);
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
			MSG_WriteByte (&buf, svc_modellist);
			MSG_WriteByte (&buf, n + 1);
		}
		n++;
		s = sv.model_precache[n+1];
	}
	if (buf.cursize)
	{
		MSG_WriteByte (&buf, 0);
		MSG_WriteByte (&buf, 0);
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// prespawn

	for (n = 0; n < sv.num_signon_buffers; n++)
	{
		if (buf.cursize+sv.signon_buffer_size[n] > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
		}
		SZ_Write (&buf,
		          sv.signon_buffers[n],
		          sv.signon_buffer_size[n]);
	}

	if (buf.cursize > MAX_MSGLEN/2)
	{
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, va("cmd spawn %i 0\n",svs.spawncount) );

	if (buf.cursize)
	{
		SV_WriteRecordMVDMessage (&buf, seq++);
		SZ_Clear (&buf);
	}

	// send current status of all other players

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		player = svs.clients + i;

		MSG_WriteByte (&buf, svc_updatefrags);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, player->old_frags);

		MSG_WriteByte (&buf, svc_updateping);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, SV_CalcPing(player));

		MSG_WriteByte (&buf, svc_updatepl);
		MSG_WriteByte (&buf, i);
		MSG_WriteByte (&buf, player->lossage);

		MSG_WriteByte (&buf, svc_updateentertime);
		MSG_WriteByte (&buf, i);
		MSG_WriteFloat (&buf, realtime - player->connection_started);

		strlcpy (info, player->userinfoshort, MAX_INFO_STRING);
		Info_RemovePrefixedKeys (info, '_');	// server passwords, etc

		MSG_WriteByte (&buf, svc_updateuserinfo);
		MSG_WriteByte (&buf, i);
		MSG_WriteLong (&buf, player->userid);
		MSG_WriteString (&buf, info);

		if (buf.cursize > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
		}
	}

	// that need only if that non first dest, demo code suppose we alredy have this, and do not send
	// this set proper model origin and angles etc for players
	for (i = 0; i < MAX_CLIENTS /* && !first_dest */ ; i++)
	{
		vec3_t origin, angles;
		edict_t *ent;
		int j, flags;

		player = svs.clients + i;
		ent = player->edict;

		if (player->state != cs_spawned)
			continue;

		flags =   (DF_ORIGIN << 0) | (DF_ORIGIN << 1) | (DF_ORIGIN << 2)
				| (DF_ANGLES << 0) | (DF_ANGLES << 1) | (DF_ANGLES << 2)
				| DF_EFFECTS | DF_SKINNUM 
				| (ent->v.health <= 0 ? DF_DEAD : 0)
				| (ent->v.mins[2] != -24 ? DF_GIB : 0)
				| DF_WEAPONFRAME | DF_MODEL;

		VectorCopy(ent->v.origin, origin);
		VectorCopy(ent->v.angles, angles);
		angles[0] *= -3;
#ifdef USE_PR2
		if( player->isBot )
			VectorCopy(ent->v.v_angle, angles);
#endif
		angles[2] = 0; // no roll angle

		if (ent->v.health <= 0)
		{	// don't show the corpse looking around...
			angles[0] = 0;
			angles[1] = ent->v.angles[1];
			angles[2] = 0;
		}

		MSG_WriteByte (&buf, svc_playerinfo);
		MSG_WriteByte (&buf, i);
		MSG_WriteShort (&buf, flags);

		MSG_WriteByte (&buf, ent->v.frame);

		for (j = 0 ; j < 3 ; j++)
			if (flags & (DF_ORIGIN << j))
				MSG_WriteCoord (&buf, origin[j]);

		for (j = 0 ; j < 3 ; j++)
			if (flags & (DF_ANGLES << j))
				MSG_WriteAngle16 (&buf, angles[j]);

		if (flags & DF_MODEL)
			MSG_WriteByte (&buf, ent->v.modelindex);

		if (flags & DF_SKINNUM)
			MSG_WriteByte (&buf, ent->v.skin);

		if (flags & DF_EFFECTS)
			MSG_WriteByte (&buf, ent->v.effects);

		if (flags & DF_WEAPONFRAME)
			MSG_WriteByte (&buf, ent->v.weaponframe);

		if (buf.cursize > MAX_MSGLEN/2)
		{
			SV_WriteRecordMVDMessage (&buf, seq++);
			SZ_Clear (&buf);
		}
	}

	// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteByte (&buf, svc_lightstyle);
		MSG_WriteByte (&buf, (char)i);
		MSG_WriteString (&buf, sv.lightstyles[i]);
	}

	// get the client to check and download skins
	// when that is completed, a begin command will be issued
	MSG_WriteByte (&buf, svc_stufftext);
	MSG_WriteString (&buf, "skins\n");

	SV_WriteRecordMVDMessage (&buf, seq++);

	SV_WriteSetMVDMessage();

	singledest = NULL;
}

/*
====================
CleanName_Init

sets chararcter table for quake text->filename translation
====================
*/
void CleanName_Init ()
{
	int i;

	for (i = 0; i < 256; i++)
		chartbl[i] = (((i&127) < 'a' || (i&127) > 'z') && ((i&127) < '0' || (i&127) > '9')) ? '_' : (i&127);

	// special cases

	// numbers
	for (i = 18; i < 29; i++)
		chartbl[i] = chartbl[i + 128] = i + 30;

	// allow lowercase only
	for (i = 'A'; i <= 'Z'; i++)
		chartbl[i] = chartbl[i+128] = i + 'a' - 'A';

	// brackets
	chartbl[29] = chartbl[29+128] = chartbl[128] = '(';
	chartbl[31] = chartbl[31+128] = chartbl[130] = ')';
	chartbl[16] = chartbl[16 + 128]= '[';
	chartbl[17] = chartbl[17 + 128] = ']';

	// dot
	chartbl[5] = chartbl[14] = chartbl[15] = chartbl[28] = chartbl[46] = '.';
	chartbl[5 + 128] = chartbl[14 + 128] = chartbl[15 + 128] = chartbl[28 + 128] = chartbl[46 + 128] = '.';

	// !
	chartbl[33] = chartbl[33 + 128] = '!';

	// #
	chartbl[35] = chartbl[35 + 128] = '#';

	// %
	chartbl[37] = chartbl[37 + 128] = '%';

	// &
	chartbl[38] = chartbl[38 + 128] = '&';

	// '
	chartbl[39] = chartbl[39 + 128] = '\'';

	// (
	chartbl[40] = chartbl[40 + 128] = '(';

	// )
	chartbl[41] = chartbl[41 + 128] = ')';

	// +
	chartbl[43] = chartbl[43 + 128] = '+';

	// -
	chartbl[45] = chartbl[45 + 128] = '-';

	// @
	chartbl[64] = chartbl[64 + 128] = '@';

	// ^
	//	chartbl[94] = chartbl[94 + 128] = '^';


	chartbl[91] = chartbl[91 + 128] = '[';
	chartbl[93] = chartbl[93 + 128] = ']';

	chartbl[16] = chartbl[16 + 128] = '[';
	chartbl[17] = chartbl[17 + 128] = ']';

	chartbl[123] = chartbl[123 + 128] = '{';
	chartbl[125] = chartbl[125 + 128] = '}';
}

/*
====================
SV_CleanName

Cleans the demo name, removes restricted chars, makes name lowercase
====================
*/
static char *SV_CleanName (unsigned char *name)
{
	static char text[1024];
	char *out = text;

	if (!*name)
	{
		*out = '\0';
		return text;
	}

	*out = chartbl[*name++];

	while (*name && ((out - text) < (int) sizeof(text)))
		if (*out == '_' && chartbl[*name] == '_')
			name++;
		else *++out = chartbl[*name++];

	*++out = 0;
	return text;
}

//bliP: 24/9 clear old demos ->
/*
====================
SV_DirSizeCheck

Deletes sv_demoClearOld files from demo dir if out of space
====================
*/
static qbool SV_DirSizeCheck (void)
{
	dir_t	dir;
	file_t	*list;
	int	n;

	if ((int)sv_demoMaxDirSize.value)
	{
		dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string), ".*", SORT_NO/*BY_DATE*/);
		if (dir.size > ((int)sv_demoMaxDirSize.value * 1024))
		{
			if ((int)sv_demoClearOld.value <= 0)
			{
				Con_Printf("Insufficient directory space, increase sv_demoMaxDirSize\n");
				return false;
			}
			list = dir.files;
			n = (int) sv_demoClearOld.value;
			Con_Printf("Clearing %d old demos\n", n);
			// HACK!!! HACK!!! HACK!!!
			if ((int)sv_demotxt.value) // if our server record demos and txts, then to remove
				n <<= 1;  // 50 demos, we have to remove 50 demos and 50 txts = 50*2 = 100 files

			qsort((void *)list, dir.numfiles, sizeof(file_t), Sys_compare_by_date);
			for (; list->name[0] && n > 0; list++)
			{
				if (list->isdir)
					continue;
				Sys_remove(va("%s/%s/%s", fs_gamedir, sv_demoDir.string, list->name));
				//Con_Printf("Remove %d - %s/%s/%s\n", n, fs_gamedir, sv_demoDir.string, list->name);
				n--;
			}
		}
	}
	return true;
}
//<-

/*
====================
SV_MVD_Record_f

record <demoname>
====================
*/
static void SV_MVD_Record_f (void)
{
	int c;
	char name[MAX_OSPATH+MAX_DEMO_NAME];
	char newname[MAX_DEMO_NAME];

	c = Cmd_Argc();
	if (c != 2)
	{
		Con_Printf ("record <demoname>\n");
		return;
	}

	if (sv.state != ss_active)
	{
		Con_Printf ("Not active yet.\n");
		return;
	}

	//bliP: 24/9 clear old demos
	if (!SV_DirSizeCheck())
		return;
	//<-

	strlcpy(newname, va("%s%s%s", sv_demoPrefix.string, SV_CleanName((unsigned char*)Cmd_Argv(1)),
						sv_demoSuffix.string), sizeof(newname) - 4);

	Sys_mkdir(va("%s/%s", fs_gamedir, sv_demoDir.string));

	snprintf (name, sizeof(name), "%s/%s/%s", fs_gamedir, sv_demoDir.string, newname);

//Sys_Printf("%s\n", name);
	//COM_StripExtension(name, name);
	if ((c = strlen(name)) > 3)
		if (strcmp(name + c - 4, ".mvd"))
			strlcat(name, ".mvd", sizeof(name));
	//Sys_Printf("%s\n", name)
	//COM_DefaultExtension(name, ".mvd");
//Sys_Printf("%s\n", name);
	//COM_CreatePath(name);
	//Sys_Printf("%s\n", name)

	//
	// open the demo file and start recording
	//
	SV_MVD_Record (SV_InitRecordFile(name));
}

/*
====================
SV_MVDEasyRecord_f

easyrecord [demoname]
====================
*/
static int Dem_CountPlayers ()
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++)
	{
		if (svs.clients[i].name[0] && !svs.clients[i].spectator)
			count++;
	}

	return count;
}

static char *Dem_Team (int num)
{
	int i;
	static char *lastteam[2];
	qbool first = true;
	client_t *client;
	static int index1 = 0;

	index1 = 1 - index1;

	for (i = 0, client = svs.clients; num && i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (first || strcmp(lastteam[index1], client->team))
		{
			first = false;
			num--;
			lastteam[index1] = client->team;
		}
	}

	if (num)
		return "";

	return lastteam[index1];
}

static char *Dem_PlayerName (int num)
{
	int i;
	client_t *client;

	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (!--num)
			return client->name;
	}

	return "";
}

// -> scream
static char *Dem_PlayerNameTeam (char *t)
{
	int	i;
	client_t *client;
	static char	n[1024];
	int	sep;

	n[0] = 0;

	sep = 0;

	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (!client->name[0] || client->spectator)
			continue;

		if (strcmp(t, client->team)==0)
		{
			if (sep >= 1)
				strlcat (n, "_", sizeof(n));
			//				snprintf (n, sizeof(n), "%s_", n);
			strlcat (n, client->name, sizeof(n));
			//			snprintf (n, sizeof(n),"%s%s", n, client->name);
			sep++;
		}
	}

	return n;
}

static int Dem_CountTeamPlayers (char *t)
{
	int	i, count;

	count = 0;
	for (i = 0; i < MAX_CLIENTS ; i++)
	{
		if (svs.clients[i].name[0] && !svs.clients[i].spectator)
			if (strcmp(&svs.clients[i].team[0], t)==0)
				count++;
	}

	return count;
}

// <-
char *quote (char *str)
{
	char *out, *s;
	if (!str)
		return NULL;
	if (!*str)
		return NULL;

	s = out = (char *) Q_malloc (strlen(str) * 2 + 1);
	while (*str)
	{
		if (!isdigit(*str) && !isalpha(*str))
			*s++ = '\\';
		*s++ = *str++;
	}
	*s = '\0';
	return out;
}

static void SV_MVDEasyRecord_f (void)
{
	int		c;
	char	name[MAX_DEMO_NAME];
	char	name2[MAX_OSPATH*7]; // scream
	//char	name2[MAX_OSPATH*2];
	int		i;
	dir_t	dir;
	char	*name3;

	c = Cmd_Argc();
	if (c > 2)
	{
		Con_Printf ("easyrecord [demoname]\n");
		return;
	}

	if (!SV_DirSizeCheck()) //bliP: 24/9 clear old demos
		return;

	// -> scream
	/*	if (c == 2)
			strlcpy (name, Cmd_Argv(1), sizeof(name));
	 
		else {
			i = Dem_CountPlayers();
			if (teamplay.value >= 1 && i > 2)
			{
				// Teamplay
				snprintf (name, sizeof(name), "%don%d_", Dem_CountTeamPlayers(Dem_Team(1)), Dem_CountTeamPlayers(Dem_Team(2)));
				if (sv_demoExtraNames.value)
				{
					strlcat (name, va("[%s]_%s_vs_[%s]_%s_%s", 
										Dem_Team(1), Dem_PlayerNameTeam(Dem_Team(1)), 
										Dem_Team(2), Dem_PlayerNameTeam(Dem_Team(2)),
										sv.mapname), sizeof(mapname));
				} else
					strlcat (name, va("%s_vs_%s_%s", Dem_Team(1), Dem_Team(2), sv.mapname), sizeof(name));
			} else {
				if (i == 2) {
					// Duel
					snprintf (name, sizeof(name), "duel_%s_vs_%s_%s",
						Dem_PlayerName(1),
						Dem_PlayerName(2),
						sv.mapname);
				} else {
					// FFA
					snprintf (name, sizeof(name), "ffa_%s(%d)", sv.mapname, i);
				}
			}
		}*/


	if (c == 2)
		strlcpy (name, Cmd_Argv(1), sizeof(name));
	else
	{
		i = Dem_CountPlayers();
		if ((int)teamplay.value >= 1 && i > 2)
		{
			// Teamplay
			snprintf (name, sizeof(name), "%don%d_", Dem_CountTeamPlayers(Dem_Team(1)), Dem_CountTeamPlayers(Dem_Team(2)));
			if ((int)sv_demoExtraNames.value > 0)
			{
				strlcat (name, va("[%s]_%s_vs_[%s]_%s_%s",
				                  Dem_Team(1), Dem_PlayerNameTeam(Dem_Team(1)),
				                  Dem_Team(2), Dem_PlayerNameTeam(Dem_Team(2)),
				                  sv.mapname), sizeof(name));
			}
			else
				strlcat (name, va("%s_vs_%s_%s", Dem_Team(1), Dem_Team(2), sv.mapname), sizeof(name));
		}
		else
		{
			if (i == 2)
			{
				// Duel
				snprintf (name, sizeof(name), "duel_%s_vs_%s_%s",
				          Dem_PlayerName(1),
				          Dem_PlayerName(2),
				          sv.mapname);
			}
			else
			{
				// FFA
				snprintf (name, sizeof(name), "ffa_%s(%d)", sv.mapname, i);
			}
		}
	}

	// <-

	// Make sure the filename doesn't contain illegal characters
	strlcpy(name, va("%s%s%s", sv_demoPrefix.string, SV_CleanName((unsigned char*)name),
			sv_demoSuffix.string), MAX_DEMO_NAME);
//	strlcat(name, sv_demoSuffix.string, sizeof(name));
//	strlcpy(name, va("%s/%s/%s", fs_gamedir, sv_demoDir.string, name), sizeof(name));
	// find a filename that doesn't exist yet
//Sys_Printf("%s, %s\n", name, name2);
	strlcpy(name2, name, sizeof(name2));
//Sys_Printf("%s, %s\n", name, name2);
	Sys_mkdir(va("%s/%s", fs_gamedir, sv_demoDir.string));
	//	COM_StripExtension(name2, name2);

//#define TMP_EASYRECORD_SPEEDUP_HACK
#ifndef TMP_EASYRECORD_SPEEDUP_HACK

// FIXME: very SLOW

	if (!(name3 = quote(name2)))
		return;
	dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string),
					  va("^%s%s", name3, sv_demoRegexp.string), SORT_NO);
	Q_free(name3);
	for (i = 1; dir.numfiles; )
	{
		snprintf(name2, sizeof(name2), "%s_%02i", name, i++);
		if (!(name3 = quote(name2)))
			return;
		dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string),
						  va("^%s%s", name3, sv_demoRegexp.string), SORT_NO);
		Q_free(name3);
	}

	snprintf(name2, sizeof(name2), va("%s/%s/%s.mvd", fs_gamedir, sv_demoDir.string, name2));

#else

// HACK, because ignoring sv_demoRegexp variable
// serve only demoname.mvd and demoname.mvd.gz files

	for (i = 0; ; i++) {
		if (i > 10000)
			return; // 10000 demos, insane ?

		if (!i)
			snprintf(name2, sizeof(name2), "%s/%s/%s.mvd",      fs_gamedir, sv_demoDir.string, name);
		else
			snprintf(name2, sizeof(name2), "%s/%s/%s_%02i.mvd", fs_gamedir, sv_demoDir.string, name, i);

		if (Sys_FileTime(name2) == -1 && Sys_FileTime(va("%s.gz", name2)) == -1)
			break; // ok, found free name
	}

#endif

	SV_MVD_Record (SV_InitRecordFile(name2));
}

static int MVD_StreamStartListening (int port)
{
	int sock;

	struct sockaddr_in	address;
	//	int fromlen;

	unsigned long nonblocking = true;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((short)port);



	if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		Sys_Error ("MVD_StreamStartListening: socket: (%i): %s\n", qerrno, strerror(qerrno));
	}

	if (ioctlsocket (sock, FIONBIO, &nonblocking) == -1)
	{
		closesocket(sock);
		Sys_Error ("MVD_StreamStartListening: ioctl FIONBIO: (%i): %s\n", qerrno, strerror(qerrno));
	}

	if( bind (sock, (void *)&address, sizeof(address)) == -1)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	listen(sock, 2);

	return sock;
}

void SV_MVDStream_Poll (void)
{
	static int listensocket=INVALID_SOCKET;
	static int listenport;

	int client;
	netadr_t na;
	struct sockaddr_qstorage addr;
	socklen_t addrlen;
	int count;
	qbool wanted;
	mvddest_t *dest;
	char *ip;

	if (!sv.state || !(int)qtv_streamport.value)
		wanted = false;
	else if (listenport && (int)qtv_streamport.value != listenport)	//easy way to switch... disable for a frame. :)
	{
		listenport = (int)qtv_streamport.value;
		wanted = false;
	}
	else
	{
		listenport = (int)qtv_streamport.value;
		wanted = true;
	}

	if (wanted && listensocket==INVALID_SOCKET)
	{
		listensocket = MVD_StreamStartListening(listenport);
/*		if (listensocket==INVALID_SOCKET && qtv_streamport.modified) @@@
		{
			Con_Printf("Cannot open TCP port %i for QTV\n", listenport);
			qtv_streamport.modified = false;
		}
*/
	}
	else if (!wanted && listensocket!=INVALID_SOCKET)
	{
		closesocket(listensocket);
		listensocket = INVALID_SOCKET;
		return;
	}
	if (listensocket==INVALID_SOCKET)
		return;

	addrlen = sizeof(addr);
	client = accept(listensocket, (struct sockaddr *)&addr, &addrlen);

	if (client == INVALID_SOCKET)
		return;

	if ((int)qtv_maxstreams.value > 0)
	{
		count = 0;
		for (dest = demo.dest; dest; dest = dest->nextdest)
		{
			if (dest->desttype == DEST_STREAM)
			{
				count++;
			}
		}

		if (count >= (int)qtv_maxstreams.value)
		{	//sorry
			char *goawaymessage = "QTVSV 1\nERROR: This server enforces a limit on the number of proxies connected at any one time. Please try again later\n\n";

			send(client, goawaymessage, strlen(goawaymessage), 0);
			closesocket(client);
			return;
		}
	}

	SockadrToNetadr(&addr, &na);
	ip = NET_AdrToString(na);
	Con_Printf("MVD streaming client connected from %s\n", ip);

	SV_MVD_InitPendingStream(client, ip);

//	SV_MVD_Record (SV_InitStream(client));
}

void SV_DemoList (qbool use_regex)
{
	mvddest_t *d;
	dir_t	dir;
	file_t	*list;
	float	free_space;
	int		i, j, n;
	int		files[MAX_DIRFILES + 1];

	int	r;
	pcre	*preg;
	const char	*errbuf;

	memset(files, 0, sizeof(files));

	Con_Printf("content of %s/%s/%s\n", fs_gamedir, sv_demoDir.string, sv_demoRegexp.string);
	dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string), sv_demoRegexp.string, SORT_BY_DATE);
	list = dir.files;
	if (!list->name[0])
	{
		Con_Printf("no demos\n");
	}

	for (i = 1, n = 0; list->name[0]; list++, i++)
	{
		for (j = 1; j < Cmd_Argc(); j++)
		{
			if (use_regex)
			{
				if (!(preg = pcre_compile((char *)Q_normalizetext((unsigned char*)Cmd_Argv(j)),
											PCRE_CASELESS, &errbuf, &r, NULL)))
				{
					Con_Printf("Sys_listdir: pcre_compile(%s) error: %s at offset %d\n",
					           Cmd_Argv(j), errbuf, r);
					Q_free(preg);
					break;
				}
				switch (r = pcre_exec(preg, NULL, list->name,
				                      strlen(list->name), 0, 0, NULL, 0))
				{
				case 0:
					Q_free(preg);
					continue;
				case PCRE_ERROR_NOMATCH:
					break;
				default:
					Con_Printf("Sys_listdir: pcre_exec(%s, %s) error code: %d\n",
					           Cmd_Argv(j), list->name, r);
				}
				Q_free(preg);
				break;
			}
			else
				if (strstr(list->name, Cmd_Argv(j)) == NULL)
					break;
		}

		if (Cmd_Argc() == j)
		{
			files[n++] = i;
		}
	}

	list = dir.files;
	for (j = (GameStarted() && n > 100) ? n - 100 : 0; files[j]; j++)
	{
		i = files[j];

		if ((d = DestByName(list[i - 1].name)))
			Con_Printf("*%d: %s %dk\n", i, list[i - 1].name, d->totalsize / 1024);
		else
			Con_Printf("%d: %s %dk\n", i, list[i - 1].name, list[i - 1].size / 1024);
	}

	for (d = demo.dest; d; d = d->nextdest)
		dir.size += d->totalsize;

	Con_Printf("\ndirectory size: %.1fMB\n", (float)dir.size / (1024 * 1024));
	if ((int)sv_demoMaxDirSize.value)
	{
		free_space = (sv_demoMaxDirSize.value * 1024 - dir.size) / (1024 * 1024);
		if (free_space < 0)
			free_space = 0;
		Con_Printf("space available: %.1fMB\n", free_space);
	}
}

void SV_DemoList_f (void)
{
	SV_DemoList (false);
}

void SV_DemoListRegex_f (void)
{
	SV_DemoList (true);
}

char *SV_MVDNum (int num)
{
	file_t	*list;
	dir_t	dir;

	if (!num)
		return NULL;

	// last recorded demo's names for command "cmd dl . .." (maximum 15 dots)
	if (num & 0xFF000000)
		return lastdemosname[(lastdemospos - (num >> 24) + 1) & 0xF];

	dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string), sv_demoRegexp.string, SORT_BY_DATE);
	list = dir.files;

	if (num & 0x00800000)
	{
		num |= 0xFF000000;
		num += dir.numfiles;
	}
	else
		--num;

	if (num > dir.numfiles)
		return NULL;

	while (list->name[0] && num) {list++; num--;}

	return list->name[0] ? list->name : NULL;
}

#define OVECCOUNT 3
static char *SV_MVDName2Txt (char *name)
{
	char	s[MAX_OSPATH];
	int		len;

	int		r, ovector[OVECCOUNT];
	pcre	*preg;
	const char	*errbuf;

	if (!name)
		return NULL;

	if (!*name)
		return NULL;

	strlcpy(s, name, MAX_OSPATH);
	len = strlen(s);

	if (!(preg = pcre_compile(sv_demoRegexp.string, PCRE_CASELESS, &errbuf, &r, NULL)))
	{
		Con_Printf("SV_MVDName2Txt: pcre_compile(%s) error: %s at offset %d\n",
					sv_demoRegexp.string, errbuf, r);
		Q_free(preg);
		return NULL;
	}
	r = pcre_exec(preg, NULL, s, len, 0, 0, ovector, OVECCOUNT);
	Q_free(preg);
	if (r < 0)
	{
		switch (r)
		{
		case PCRE_ERROR_NOMATCH:
			return NULL;
		default:
			Con_Printf("SV_MVDName2Txt: pcre_exec(%s, %s) error code: %d\n",
						sv_demoRegexp.string, s, r);
			return NULL;
		}
	}
	else
	{
		if (ovector[0] + 5 > MAX_OSPATH)
			len = MAX_OSPATH - 5;
		else
			len = ovector[0];
	}
	s[len++] = '.';
	s[len++] = 't';
	s[len++] = 'x';
	s[len++] = 't';
	s[len]   = '\0';

	//Con_Printf("%d) %s, %s\n", r, name, s);
	return va("%s", s);
}

static char *SV_MVDTxTNum (int num)
{
	return SV_MVDName2Txt (SV_MVDNum(num));
}

static void SV_MVDRemove_f (void)
{
	char name[MAX_DEMO_NAME], *ptr;
	char path[MAX_OSPATH];
	int i;

	if (Cmd_Argc() != 2)
	{
		Con_Printf("rmdemo <demoname> - removes the demo\nrmdemo *<token>   - removes demo with <token> in the name\nrmdemo *          - removes all demos\n");
		return;
	}

	ptr = Cmd_Argv(1);
	if (*ptr == '*')
	{
		dir_t dir;
		file_t *list;

		// remove all demos with specified token
		ptr++;

		dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string), sv_demoRegexp.string, SORT_BY_DATE);
		list = dir.files;
		for (i = 0;list->name[0]; list++)
		{
			if (strstr(list->name, ptr))
			{
				if (sv.mvdrecording && DestByName(list->name)/*!strcmp(list->name, demo.name)*/)
					SV_MVDStop_f(); // FIXME: probably we must stop not all demos, but only partial dest

				// stop recording first;
				snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, list->name);
				if (!Sys_remove(path))
				{
					Con_Printf("removing %s...\n", list->name);
					i++;
				}

				Sys_remove(SV_MVDName2Txt(path));
			}
		}

		if (i)
		{
			Con_Printf("%d demos removed\n", i);
		}
		else
		{
			Con_Printf("no matching found\n");
		}

		return;
	}

	strlcpy(name, Cmd_Argv(1), MAX_DEMO_NAME);
	COM_DefaultExtension(name, ".mvd");

	snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, name);

	if (sv.mvdrecording && DestByName(name) /*!strcmp(name, demo.name)*/)
		SV_MVDStop_f(); // FIXME: probably we must stop not all demos, but only partial dest

	if (!Sys_remove(path))
	{
		Con_Printf("demo %s successfully removed\n", name);

		if (*sv_ondemoremove.string)
		{
			extern redirect_t sv_redirected;
			int old = sv_redirected;

			sv_redirected = RD_NONE; // this script is called always from the console
			Cmd_TokenizeString(va("script %s \"%s\" \"%s\"", sv_ondemoremove.string, sv_demoDir.string, name));
			SV_Script_f();

			sv_redirected = old;
		}
	}
	else
		Con_Printf("unable to remove demo %s\n", name);

	Sys_remove(SV_MVDName2Txt(path));
}

static void SV_MVDRemoveNum_f (void)
{
	int		num;
	char	*val, *name;
	char path[MAX_OSPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf("rmdemonum <#>\n");
		return;
	}

	val = Cmd_Argv(1);
	if ((num = Q_atoi(val)) == 0 && val[0] != '0')
	{
		Con_Printf("rmdemonum <#>\n");
		return;
	}

	name = SV_MVDNum(num);

	if (name != NULL)
	{
		if (sv.mvdrecording && DestByName(name)/*!strcmp(name, demo.name)*/)
			SV_MVDStop_f(); // FIXME: probably we must stop not all demos, but only partial dest

		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, name);
		if (!Sys_remove(path))
		{
			Con_Printf("demo %s succesfully removed\n", name);
			if (*sv_ondemoremove.string)
			{
				extern redirect_t sv_redirected;
				int old = sv_redirected;

				sv_redirected = RD_NONE; // this script is called always from the console
				Cmd_TokenizeString(va("script %s \"%s\" \"%s\"", sv_ondemoremove.string, sv_demoDir.string, name));
				SV_Script_f();

				sv_redirected = old;
			}
		}
		else
			Con_Printf("unable to remove demo %s\n", name);

		Sys_remove(SV_MVDName2Txt(path));
	}
	else
		Con_Printf("invalid demo num\n");
}

static void SV_MVDInfoAdd_f (void)
{
	char *name, *args, path[MAX_OSPATH];
	FILE *f;

	if (Cmd_Argc() < 3)
	{
		Con_Printf("usage:demoInfoAdd <demonum> <info string>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*") || !strcmp(Cmd_Argv(1), "**"))
	{
		if (!sv.mvdrecording || !demo.dest)
		{
			Con_Printf("Not recording demo!\n");
			return;
		}

//		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, demo.path, SV_MVDName2Txt(demo.name));
// FIXME: dunno is this right, just using first dest, also may be we must use demo.dest->path instead of sv_demoDir
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, SV_MVDName2Txt(demo.dest->name));
	}
	else
	{
		name = SV_MVDTxTNum(Q_atoi(Cmd_Argv(1)));

		if (!name)
		{
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, name);
	}

	if ((f = fopen(path, !strcmp(Cmd_Argv(1), "**") ? "a+b" : "a+t")) == NULL)
	{
		Con_Printf("failed to open the file\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "**"))
	{ // put content of one file to another
		FILE *src;

		snprintf(path, MAX_OSPATH, "%s/%s", fs_gamedir, Cmd_Argv(2));

		if ((src = fopen(path, "rb")) == NULL) // open src
		{
			Con_Printf("failed to open input file\n");
		}
		else
		{
			char buf[1024*200] = {0}; // 200 kb
			int sz = fread((void*) buf, 1, sizeof(buf), src); // read from src

			if (sz <= 0)
				Con_Printf("failed to read or empty input file\n");
			else if (sz != fwrite((void*) buf, 1, sz, f)) // write to f
				Con_Printf("failed write to file\n");

			fclose(src); // close src
		}
	}
	else
	{
		// skip demonum
		args = Cmd_Args();

		while (*args > 32) args++;
		while (*args && *args <= 32) args++;

		fwrite(args, strlen(args), 1, f);
		fwrite("\n", 1, 1, f);
	}

	fflush(f);
	fclose(f);
}

static void SV_MVDInfoRemove_f (void)
{
	char *name, path[MAX_OSPATH];

	if (Cmd_Argc() < 2)
	{
		Con_Printf("usage:demoInfoRemove <demonum>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*"))
	{
		if (!sv.mvdrecording || !demo.dest)
		{
			Con_Printf("Not recording demo!\n");
			return;
		}

//		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, demo.path, SV_MVDName2Txt(demo.name));
// FIXME: dunno is this right, just using first dest, also may be we must use demo.dest->path instead of sv_demoDir
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, SV_MVDName2Txt(demo.dest->name));
	}
	else
	{
		name = SV_MVDTxTNum(Q_atoi(Cmd_Argv(1)));

		if (!name)
		{
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, name);
	}

	if (Sys_remove(path))
		Con_Printf("failed to remove the file\n");
	else Con_Printf("file removed\n");
}

void SV_MVDInfo_f (void)
{
	unsigned char buf[512];
	FILE *f = NULL;
	char *name, path[MAX_OSPATH];

	if (Cmd_Argc() < 2)
	{
		Con_Printf("usage: demoinfo <demonum>\n<demonum> = * for currently recorded demo\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "*"))
	{
		if (!sv.mvdrecording || !demo.dest)
		{
			Con_Printf("Not recording demo!\n");
			return;
		}

//		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, demo.path, SV_MVDName2Txt(demo.name));
// FIXME: dunno is this right, just using first dest, also may be we must use demo.dest->path instead of sv_demoDir
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, SV_MVDName2Txt(demo.dest->name));
	}
	else
	{
		name = SV_MVDTxTNum(Q_atoi(Cmd_Argv(1)));

		if (!name)
		{
			Con_Printf("invalid demo num\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, name);
	}

	if ((f = fopen(path, "rt")) == NULL)
	{
		Con_Printf("(empty)\n");
		return;
	}

	while (!feof(f))
	{
		buf[fread (buf, 1, sizeof(buf) - 1, f)] = 0;
		Con_Printf("%s", Q_yelltext(buf));
	}

	fclose(f);
}

#define MAXDEMOS			10
#define MAXDEMOS_RD_PACKET	100
void SV_LastScores_f (void)
{
	int		demos = MAXDEMOS, i;
	char	buf[512];
	FILE	*f = NULL;
	char	path[MAX_OSPATH];
	dir_t	dir;
	extern redirect_t sv_redirected;

	if (Cmd_Argc() > 2)
	{
		Con_Printf("usage: lastscores [<numlastdemos>]\n<numlastdemos> = '0' for all demos\n<numlastdemos> = '' for last %i demos\n", MAXDEMOS);
		return;
	}

	if (Cmd_Argc() == 2)
		if ((demos = Q_atoi(Cmd_Argv(1))) <= 0)
			demos = MAXDEMOS;

	dir = Sys_listdir(va("%s/%s", fs_gamedir, sv_demoDir.string),
	                  sv_demoRegexp.string, SORT_BY_DATE);
	if (!dir.numfiles)
	{
		Con_Printf("No demos.\n");
		return;
	}

	if (demos > dir.numfiles)
		demos = dir.numfiles;

	if (demos > MAXDEMOS && GameStarted())
		Con_Printf("<numlastdemos> was decreased to %i: match is in progress.\n",
					demos = MAXDEMOS);

	if (demos > MAXDEMOS_RD_PACKET && sv_redirected == RD_PACKET)
		Con_Printf("<numlastdemos> was decreased to %i: command from connectionless packet.\n",
					demos = MAXDEMOS_RD_PACKET);

	Con_Printf("List of %d last demos:\n", demos);

	for (i = dir.numfiles - demos; i < dir.numfiles; )
	{
		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string,
					SV_MVDName2Txt(dir.files[i].name));

		Con_Printf("%i. ", ++i);
		if ((f = fopen(path, "rt")) == NULL)
			Con_Printf("(empty)\n");
		else
		{
			if (!feof(f))
			{
				buf[fread (buf, 1, sizeof(buf) - 1, f)] = 0;
				*strchr(buf, '\n') = 0;
				Con_Printf("%s\n", Q_yelltext((unsigned char*)buf));
			}
			else
				Con_Printf("(empty)\n");
			fclose(f);
		}
	}
}


void SV_MVDInit (void)
{
	MVD_Init();

	Cmd_AddCommand ("record", SV_MVD_Record_f);
	Cmd_AddCommand ("easyrecord", SV_MVDEasyRecord_f);
	Cmd_AddCommand ("stop", SV_MVDStop_f);
	Cmd_AddCommand ("cancel", SV_MVD_Cancel_f);
	Cmd_AddCommand ("lastscores", SV_LastScores_f);
	Cmd_AddCommand ("dlist", SV_DemoList_f);
	Cmd_AddCommand ("dlistr", SV_DemoListRegex_f);
	Cmd_AddCommand ("dlistregex", SV_DemoListRegex_f);
	Cmd_AddCommand ("demolist", SV_DemoList_f);
	Cmd_AddCommand ("demolistr", SV_DemoListRegex_f);
	Cmd_AddCommand ("demolistregex", SV_DemoListRegex_f);
	Cmd_AddCommand ("rmdemo", SV_MVDRemove_f);
	Cmd_AddCommand ("rmdemonum", SV_MVDRemoveNum_f);
	Cmd_AddCommand ("script", SV_Script_f);
	Cmd_AddCommand ("demoInfoAdd", SV_MVDInfoAdd_f);
	Cmd_AddCommand ("demoInfoRemove", SV_MVDInfoRemove_f);
	Cmd_AddCommand ("demoInfo", SV_MVDInfo_f);

	Cvar_Register (&qtv_streamport);
	Cvar_Register (&qtv_maxstreams);
	Cvar_Register (&qtv_password);

}
