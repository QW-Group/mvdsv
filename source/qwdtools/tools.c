/*
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

    $Id$
*/

#include "defs.h"

int com_argc;
char *com_argv[MAX_NUM_ARGVS];

usercmd_t nullcmd; // guarenteed to be zero

//#define BUF_FULL (1<<31)

sizebuf_t	*msgbuf;
dbuffer_t	*demobuffer;
static int	header = (char *)&((header_t*)0)->data - (char *)NULL;


/*
==============================================================================
 
			MESSAGE IO FUNCTIONS
 
Handles byte ordering and avoids alignment errors
==============================================================================
*/

//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, const int c)
{
	byte *buf;

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, const int c)
{
	byte *buf;

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, const int c)
{
	byte *buf;

	buf = (byte *) SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, const int c)
{
	byte *buf;

	buf = (byte *) SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, const float f)
{
	union
	{
		float	f;
		int	l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write (sb, (void *) "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, const float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, const float f)
{
	MSG_WriteByte (sb, Q_rint(f*256.0/360.0) & 255);
}

void MSG_WriteAngle16 (sizebuf_t *sb, const float f)
{
	MSG_WriteShort (sb, Q_rint(f*65536.0/360.0) & 65535);
}

void MSG_WriteDeltaUsercmd (sizebuf_t *buf, const usercmd_t *from1, const usercmd_t *cmd)
{
	int bits;

	//
	// send the movement message
	//
	bits = 0;
	if (cmd->angles[0] != from1->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from1->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from1->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from1->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from1->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from1->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from1->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse != from1->impulse)
		bits |= CM_IMPULSE;

	MSG_WriteByte (buf, bits);

	if (bits & CM_ANGLE1)
		MSG_WriteAngle16 (buf, cmd->angles[0]);
	if (bits & CM_ANGLE2)
		MSG_WriteAngle16 (buf, cmd->angles[1]);
	if (bits & CM_ANGLE3)
		MSG_WriteAngle16 (buf, cmd->angles[2]);

	if (bits & CM_FORWARD)
		MSG_WriteShort (buf, cmd->forwardmove);
	if (bits & CM_SIDE)
		MSG_WriteShort (buf, cmd->sidemove);
	if (bits & CM_UP)
		MSG_WriteShort (buf, cmd->upmove);

	if (bits & CM_BUTTONS)
		MSG_WriteByte (buf, cmd->buttons);
	if (bits & CM_IMPULSE)
		MSG_WriteByte (buf, cmd->impulse);
	MSG_WriteByte (buf, cmd->msec);
}


//
// reading functions
//
int	msg_readcount;
qbool msg_badread;

qbool MSG_Forward (sizebuf_t *sb, const int start, const int count)
{
	msg_readcount = start;

	if (msg_readcount + count > net_message.cursize)
	{
		msg_badread = true;
		return false;
	}

	SZ_Write (sb, net_message.data + msg_readcount, count);
	msg_readcount += count;
	return true;
}


void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

int MSG_GetReadCount(void)
{
	return msg_readcount;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (signed char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadByte (void)
{
	int	c;

	if (msg_readcount+1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (unsigned char)net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadShort (void)
{
	int	c;

	if (msg_readcount+2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (short)(net_message.data[msg_readcount]
	            + (net_message.data[msg_readcount+1]<<8));

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong (void)
{
	int	c;

	if (msg_readcount+4 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = net_message.data[msg_readcount]
	    + (net_message.data[msg_readcount+1]<<8)
	    + (net_message.data[msg_readcount+2]<<16)
	    + (net_message.data[msg_readcount+3]<<24);

	msg_readcount += 4;

	return c;
}

float MSG_ReadFloat (void)
{
	union
	{
		byte	b[4];
		float	f;
		int	l;
	} dat;

	dat.b[0] =	net_message.data[msg_readcount];
	dat.b[1] =	net_message.data[msg_readcount+1];
	dat.b[2] =	net_message.data[msg_readcount+2];
	dat.b[3] =	net_message.data[msg_readcount+3];
	msg_readcount += 4;

	dat.l = LittleLong (dat.l);

	return dat.f;
}

char *MSG_ReadString (void)
{
	static char string[2048];
	int c;
	size_t l = 0;

	do
	{
		c = MSG_ReadByte ();
		if (c == 255) // skip these to avoid security problems
			continue; // with old clients and servers
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	}
	while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

char *MSG_ReadStringLine (void)
{
	static char string[2048];
	int c;
	size_t l = 0;

	do
	{
		c = MSG_ReadByte ();
		if (c == 255)
			continue;
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	}
	while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

float MSG_ReadCoord (void)
{
	return MSG_ReadShort() * (1.0/8);
}

float MSG_ReadAngle (void)
{
	return MSG_ReadChar() * (360.0/256);
}

float MSG_ReadAngle16 (void)
{
	return MSG_ReadShort() * (360.0/65536);
}

void MSG_ReadDeltaUsercmd (const usercmd_t *from1, usercmd_t *move)
{
	int bits;

	memcpy (move, from1, sizeof(*move));

	bits = MSG_ReadByte ();

	// read current angles
	if (bits & CM_ANGLE1)
		move->angles[0] = MSG_ReadAngle16 ();
	if (bits & CM_ANGLE2)
		move->angles[1] = MSG_ReadAngle16 ();
	if (bits & CM_ANGLE3)
		move->angles[2] = MSG_ReadAngle16 ();

	// read movement
	if (bits & CM_FORWARD)
		move->forwardmove = MSG_ReadShort ();
	if (bits & CM_SIDE)
		move->sidemove = MSG_ReadShort ();
	if (bits & CM_UP)
		move->upmove = MSG_ReadShort ();

	// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte ();
	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte ();

	// read time to run command
	move->msec = MSG_ReadByte ();
}

//===========================================================================

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
}

void *SZ_GetSpace (sizebuf_t *buf, const int length)
{
	void *data;

	if (buf->cursize + length > buf->maxsize)
	{
		Sys_Printf ("SZ_GetSpace: overflow\n");
		SZ_Clear (buf);
		Dem_Stop(from);
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, const int length)
{
	memcpy (SZ_GetSpace(buf,length), data, length);
}

void SZ_Print (sizebuf_t *buf, const char *data)
{
	int len = strlen (data) + 1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}

void MVDBuffer_Init(dbuffer_t *dbuffer, byte *buf, size_t size, sizebuf_t *msg)
{
	demobuffer = dbuffer;

	demobuffer->data = buf;
	demobuffer->maxsize = size;
	demobuffer->start = 0;
	demobuffer->end = 0;
	demobuffer->last = 0;
	demobuffer->msgbuf = msgbuf = msg;
	if (msgbuf)
	{
		memset(msgbuf, 0, sizeof(*msgbuf));
		msgbuf->data = demobuffer->data;
		msgbuf->maxsize = MAXSIZE(demobuffer);
	}
}

void DemoBuffer_Set(dbuffer_t *dbuffer)
{
	demobuffer = dbuffer;
	msgbuf = demobuffer->msgbuf;
}


/*
==============
Demo_SetMsgBuf
 
Sets the frame message buffer
==============
*/

void MVDSetMsgBuf(dbuffer_t *dbuffer, sizebuf_t *cur)
{
	demobuffer = dbuffer;
	// fix the maxsize of previous msg buffer,
	// we won't be able to write there anymore
	if (demobuffer->msgbuf != NULL)
		demobuffer->msgbuf->maxsize = demobuffer->msgbuf->bufsize;

	demobuffer->msgbuf = msgbuf = cur;
	memset(msgbuf, 0, sizeof(*msgbuf));

	msgbuf->data = demobuffer->data + demobuffer->end;
	msgbuf->maxsize = MAXSIZE(demobuffer);
}

/*
==============
SV_MVDWriteToDisk
 
Writes to disk a message meant for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

void SV_MVDWriteToDisk(sizebuf_t *buf, int type, int to, float time1)
{
	int pos = 0;
	header_t *p;
	int	size;
	sizebuf_t msg;

	p = (header_t *)buf->data;
	buf->h = NULL;

	while (pos < buf->bufsize)
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

				WriteDemoMessage(&msg, p->type, p->to, time1);
			}

			// data is written so it need to be cleard from demobuf
			if (buf->data != (byte*)p)
				memmove(buf->data + size + header, buf->data, (byte*)p - buf->data);

			buf->bufsize -= size + header;
			buf->data += size + header;
			pos -= size + header;
			buf->maxsize -= size + header;
			//demobuffer->start += size + header;
		}
		// move along
		p = (header_t *)(p->data + size);
	}

	/*
	if (demobuffer->start == demobuffer->last) {
		if (demobuffer->start == demobuffer->end) {
			demobuffer->end = 0; // demobuffer is empty
			msgbuf->data = demobuffer->data;
		}

		// go back to begining of the buffer
		demobuffer->last = demobuffer->end;
		demobuffer->start = 0;
	}
	*/
}

/*
==============
MVDSetBuf
 
Sets position in the buf for writing to specific client
==============
*/

static void MVDSetBuf(byte type, int to)
{
	header_t *p;
	int pos = 0;

	p = (header_t *)msgbuf->data;

	while (pos < msgbuf->bufsize)
	{
		pos += header + p->size;

		if (type == p->type && to == p->to && !p->full)
		{
			msgbuf->cursize = pos;
			msgbuf->h = p;
			return;
		}

		p = (header_t *)(p->data + p->size);
	}
	// type&&to not exist in the buf, so add it

	p->frame = world.parsecount;
	p->source = 1 << (from - sources);
	p->type = type;
	p->to = to;
	p->size = 0;
	p->full = 0;

	msgbuf->bufsize += header;
	msgbuf->cursize = msgbuf->bufsize;
	demobuffer->end += header;
	msgbuf->h = p;
}

void MVDMoveBuf(void)
{
	// set the last message mark to the previous frame (i/e begining of this one)
	demobuffer->last = demobuffer->end - msgbuf->bufsize;

	// move buffer to the begining of demo buffer
	memmove(demobuffer->data, msgbuf->data, msgbuf->bufsize);
	msgbuf->data = demobuffer->data;
	demobuffer->end = msgbuf->bufsize;
	msgbuf->h = NULL; // it will be setup again
	msgbuf->maxsize = MAXSIZE(demobuffer) + msgbuf->bufsize;
}

void DemoWrite_Cat(sizebuf_t *buf)
{
	qbool move = false;
	// will it fit?
	while (msgbuf->bufsize + buf->cursize > msgbuf->maxsize)
	{
		// if we reached the end of buffer move msgbuf to the begining
		if (!move && demobuffer->end > demobuffer->start)
			move = true;

		WritePackets(1);
		if (move && demobuffer->start > msgbuf->bufsize + buf->cursize)
			MVDMoveBuf();
	}

	msgbuf->h = NULL;
	msgbuf->cursize = msgbuf->bufsize;

	SZ_Write(msgbuf, buf->data, buf->cursize);
	msgbuf->bufsize += buf->cursize;

	if ((demobuffer->end += buf->cursize) > demobuffer->last)
		demobuffer->last = demobuffer->end;


}

void MVDWrite_Begin(byte type, int to, int size)
{
	byte *p;
	qbool move = false;

	if (!(sworld.options & (O_CONVERT | O_MARGE)))
		return;

	if (sworld.options & O_QWDSYNC)
		return;

	// signon message order cannot be changed
	if (!world.signonloaded && msgbuf->h != NULL && (msgbuf->h->type != type || msgbuf->h->to != to))
		msgbuf->h->full = 1;

	if (sworld.options & O_MARGE && msgbuf->h != NULL)
		msgbuf->h->full = 1;

	// will it fit?
	while (msgbuf->bufsize + size + header > msgbuf->maxsize)
	{
		// if we reached the end of buffer move msgbuf to the begining
		if (!move && demobuffer->end > demobuffer->start)
			move = true;

		WritePackets(1);
		if (move && demobuffer->start > msgbuf->bufsize + header + size)
			MVDMoveBuf();
	}

	if (msgbuf->h == NULL || msgbuf->h->type != type || msgbuf->h->to != to || msgbuf->h->full)
	{
		MVDSetBuf(type, to);
	}

	if (msgbuf->h->size + size > MAX_MSGLEN)
	{
		msgbuf->h->full = 1;
		MVDSetBuf(type, to);
	}

	// we have to make room for new data
	if (msgbuf->cursize != msgbuf->bufsize)
	{
		p = msgbuf->data + msgbuf->cursize;
		memmove(p+size, p, msgbuf->bufsize - msgbuf->cursize);
	}

	msgbuf->bufsize += size;
	msgbuf->h->size += size;
	if ((demobuffer->end += size) > demobuffer->last)
		demobuffer->last = demobuffer->end;
}

/*
====================
WriteDemoMessage

Dumps the current net message, prefixed by the length and time
====================
*/
void WriteDemoMessage (sizebuf_t *msg, int type, int to, float time1)
{
	int		len, i, msec;
	byte	c;
	static float prevtime = 0;

	if (sworld.demo.file == NULL)
		return;

	msec = (int)((time1 - prevtime) * 1000);
	prevtime += msec * 0.001;

	//Sys_Printf("%f %f\n", time1, prevtime);

	if (!world.signonloaded)
		msec = 0;

#if 0
	if (msec > 255)
	{
		Sys_Printf("lag:%d\n", msec);
		if (msec > 5000) msec = 5000;

		for (len = 0; msec > 255; msec -= 255)
			fwrite (&len, 4, 1, sworld.demo.file);

		//msec = 255;
	}
	else
#endif
		if (msec < 2) msec = 0;

	c = msec;
	fwrite(&c, sizeof(c), 1, sworld.demo.file);

	if (type != demo.lasttype || to != demo.lastto)
	{
		demo.lastto = to;
		demo.lasttype = type;

		switch (demo.lasttype)
		{
		case dem_all:
			c = dem_all;
			fwrite (&c, sizeof(c), 1, sworld.demo.file);
			break;
		case dem_multiple:
			c = dem_multiple;
			fwrite (&c, sizeof(c), 1, sworld.demo.file);

			i = LittleLong(demo.lastto);
			fwrite (&i, sizeof(i), 1, sworld.demo.file);
			break;
		case dem_single:
		case dem_stats:
			c = demo.lasttype + (demo.lastto << 3);
			fwrite (&c, sizeof(c), 1, sworld.demo.file);
			break;
		default:
			Sys_Printf("ERROR:bad demo message type:%d\n", type);
			Dem_Stop(from);
			return;
		}
	}
	else
	{
		c = dem_read;
		fwrite (&c, sizeof(c), 1, sworld.demo.file);
	}

	len = LittleLong (msg->cursize);
	fwrite (&len, 4, 1, sworld.demo.file);
	fwrite (msg->data, msg->cursize, 1, sworld.demo.file);

	fflush (sworld.demo.file);
}

//============================================================================


void Tools_Init (void)
{}

/*
==================
ForceExtension
 
If path doesn't have an extension or has a different extension,
append(!) specified extension
Extension should include the .
==================
*/
void ForceExtension (char *path, char *extension)
{
	char    *src;

	src = path + strlen(path) - strlen(extension);
	if (src >= path && !strcmp(src, extension))
		return;

	strlcat (path, extension, MAX_OSPATH);
}

char *TemplateName (char *from1, char *to, char *ch)
{
	static char name[MAX_OSPATH];
	char tmp[MAX_OSPATH];
	char *p;

	memset(name, 0, sizeof(name));

	if ( !(p = strstr(to, ch)) )
		return to;

	COM_StripExtension(from1, tmp);
	strlcpy(name, to, p - to);
	strlcat(name, va("%s%s", tmp, p + 1), sizeof(name));

	return name;
}

char *getPath(char *path)
{
	static char dir[MAX_OSPATH];
	char *p;

	strlcpy(dir, path, sizeof(dir));
	p = dir + strlen(dir);
	while (p > dir)
	{
		if (*p == '\\' || *p == '/')
		{
			p++;
			break;
		}
		p--;
	}
	*p = 0;

	return dir;
}

/*
==============
GetFileList
 
Reads wildcards to get full file list
==============
*/
#ifdef _WIN32
int AddToFileList(flist_t *filelist, char *file)
{
	char	**flist = NULL, *p;
	int		count;
	long	hFile;
	struct	_finddata_t c_file;

	// move to the end of list
	while (filelist->list != NULL)
		filelist++;

	count = 0;

	// get first file, add it to list even if file is not found
	hFile = _findfirst( file, &c_file );
	if (hFile == -1L)
		filelist->path[0] = 0;
	else
		strlcpy(filelist->path, getPath(file), sizeof(filelist->path));

	do
	{
		if (hFile == -1L)
		{
			strlcpy(c_file.name, file, sizeof(c_file.name));
		}
		else if (c_file.attrib & _A_SUBDIR || c_file.attrib & _A_SYSTEM)
			continue;

		// realloc flist table
		if ( (flist = (char**) realloc (flist, sizeof(char*) * count+1)) == NULL)
			Sys_Error("faild to alloc memory for file list");

		// alloc memory for file name
		p = (char*) Q_malloc (strlen(c_file.name)+1);

		// copy the name
		strlcpy(p, c_file.name, strlen(c_file.name)+1);

		flist[count++] = p;
	}
	while ( hFile != -1L && _findnext( hFile, &c_file ) == 0 );
		_findclose( hFile );

	filelist->list = flist;
	filelist->count = count;
	if (!count)
		count++;
	return count;
}
#else
int AddToFileList(flist_t *filelist, char *file)
{
	char *tmp, *name;

	// move to the end of list
	while (filelist->list != NULL)
		filelist++;

	tmp = (char *) Q_malloc (strlen(file)+1);
	strlcpy(tmp, file, (strlen(file)+1));
	name = basename(tmp);

	strlcpy(filelist->path, getPath(file), sizeof(filelist->path));

	filelist->list = (char **) Q_malloc (sizeof(char *));
	filelist->list[0] = (char *) Q_malloc (strlen(name)+1);
	strlcpy(filelist->list[0], name, (strlen(name)+1));

	filelist->count = 1;

	Q_free(tmp);

	return 1;
}
#endif

void FreeFileList(flist_t *flist)
{
	char **p;


	for ( ; flist->list != NULL; flist++)
	{
		for (p = flist->list; flist->count; p++, flist->count--)
			Q_free(*p);

		Q_free(flist->list);
		flist->list = NULL;
	}
}

//============================================================================

char *Info_ValueForKey (char *s, char *key)
{
	char	pkey[512];
	static	char value[4][512];	// use two buffers so compares
	// work without stomping on each other
	static	int	valueindex;
	char	*o;


	valueindex = (valueindex + 1) % 4;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s || o >= pkey + sizeof(pkey) - 1)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s || o >= value[valueindex] + sizeof(value[valueindex]) - 1)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strncmp (key, pkey, sizeof(pkey)) )
			return value[valueindex];

		if (!*s)
			return "";
		s++;
	}
}

/*
================
CheckParm
 
Returns the position (1 to argc-1) in the program's argument list
where the given parameter appears, or 0 if not present
================
*/

int CheckParm (char *parm)
{
	int		i;

	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
		if (!strcmp (parm,com_argv[i]))
			return i;
	}

	return 0;
}

/*
================
AddParm
 
Adds the given string at the end of the current argument list
================
*/
void AddParm (char *parm)
{
	static char parmbuf[2048];
	static char *p = parmbuf;

	strlcpy(p, parm, sizeof(parmbuf));
	com_argv[com_argc++] = p;
	p += strlen(parm) + 1;
}

void RemoveParm (int num)
{
	if (num < 1 || num > com_argc)
		return;

	for(; num < com_argc; num++)
		com_argv[num] = com_argv[num + 1];

	com_argc--;
}


void Argv_Init (int argc, char **argv)
{
	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ;
	        com_argc++)
	{
		com_argv[com_argc] = argv[com_argc];
	}
}

/*
==================================================
 
  FILE SYSTEM
 
==================================================
*/

/*
================
fileLength
================
*/

int fileLength (FILE *f)
{
	int		pos;
	int		end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int FileOpenRead (char *path, FILE **hndl)
{
	FILE	*f;

#ifdef _WIN32
	f = fopen(path, "rb");
#else
	f = fopen(path, "r");
#endif
	if (!f)
	{
		*hndl = NULL;
		return -1;
	}
	*hndl = f;

	return fileLength(f);
}

byte *LoadFile(char *path)
{
	FILE *f;
	byte *buf;
	int len;

	if ((len = FileOpenRead(path, &f)) == -1)
		return NULL;

	buf = (byte *) Q_malloc (len+1);

	fread(buf, 1, len, f);
	Sys_fclose(&f);

	return buf;
}

vec_t VectorLength(vec3_t v)
{
	return sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}
