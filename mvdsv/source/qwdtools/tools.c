#include "defs.h"

int com_argc;
char *com_argv[MAX_NUM_ARGVS];

usercmd_t nullcmd; // guarenteed to be zero

/*
============================================================================

					BYTE ORDER FUNCTIONS

============================================================================
*/

qboolean	bigendien;

short	(*BigShort) (short l);
short	(*LittleShort) (short l);
int	(*BigLong) (int l);
int	(*LittleLong) (int l);
float	(*BigFloat) (float l);
float	(*LittleFloat) (float l);

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	ShortNoSwap (short l)
{
	return l;
}

int    LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

int	LongNoSwap (int l)
{
	return l;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float FloatNoSwap (float f)
{
	return f;
}

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

//
// writing functions
//

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte	*buf;
	
	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte	*buf;
	
	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte	*buf;
	
	buf = SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte	*buf;
	
	buf = SZ_GetSpace (sb, 4);
	buf[0] = c&0xff;
	buf[1] = (c>>8)&0xff;
	buf[2] = (c>>16)&0xff;
	buf[3] = c>>24;
}

void MSG_WriteFloat (sizebuf_t *sb, float f)
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

void MSG_WriteString (sizebuf_t *sb, char *s)
{
	if (!s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

void MSG_WriteCoord (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*8));
}

void MSG_WriteAngle (sizebuf_t *sb, float f)
{
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}

void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, (int)(f*65536/360) & 65535);
}

void MSG_WriteDeltaUsercmd (sizebuf_t *buf, usercmd_t *from, usercmd_t *cmd)
{
	int		bits;

//
// send the movement message
//
	bits = 0;
	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse != from->impulse)
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
int			msg_readcount;
qboolean	msg_badread;

qboolean MSG_Forward (sizebuf_t *sb, int start, int count)
{
	msg_readcount = start;

	if (msg_readcount+count > net_message.cursize)
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
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadStringLine (void)
{
	static char	string[2048];
	int		l,c;
	
	l = 0;
	do
	{
		c = MSG_ReadChar ();
		if (c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
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

void MSG_ReadDeltaUsercmd (usercmd_t *from, usercmd_t *move)
{
	int bits;
	extern int stat_size;

	memcpy (move, from, sizeof(*move));

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

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void	*data;
	
	if (buf->cursize + length > buf->maxsize)
	{	
		Sys_Printf ("SZ_GetSpace: overflow\n");
		SZ_Clear (buf); 
		Dem_Stop();
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;
	
	return data;
}

void SZ_Write (sizebuf_t *buf, void *data, int length)
{
	memcpy (SZ_GetSpace(buf,length),data,length);		
}

void SZ_Print (sizebuf_t *buf, char *data)
{
	int		len;
	
	len = strlen(data)+1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}


/*
==============
DemoWriteToDisk

Writes to disk a message ment for specifc client
or all messages if type == 0
Message is cleared from demobuf after that
==============
*/

#define POS_TO 1
#define POS_SIZE 5
#define POS_DATA 9

#define BUF_FULL (1<<31)

int			msgmax;
sizebuf_t	*msgbuf;

void DemoWriteToDisk(int type, int to, float time)
{
	int pos = 0;
	byte *p;
	int	btype, bto, bsize;
	sizebuf_t msg;

	p = msgbuf->data;
	while (pos < msgbuf->bufsize)
	{

		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE) & (~BUF_FULL);
		pos += POS_DATA + bsize; //pos now points to next message

		// no type means we are writing to disk everything
		if (!type || (btype == type && bto == to))
		{
			if (bsize) {
				msg.data = p+POS_DATA;
				msg.cursize = bsize;

				WriteDemoMessage(&msg, btype, bto, time);
			}

			// data is written so it need to be cleard from demobuf
			memmove(p, p+POS_DATA+bsize, msgbuf->bufsize - pos);
			msgbuf->bufsize -= bsize+POS_DATA;
			pos -= bsize + POS_DATA;
			if (msgbuf->cursize > pos)
				msgbuf->cursize -= bsize + POS_DATA;

			if (btype == msgbuf->curtype && bto == msgbuf->curto)
			{
				msgbuf->curtype = 0;
				msgbuf->curto = 0;
				msgbuf->size = NULL;
			}

			// did we find what we were looking for ?
			//if (type)
			//	return;
		} else {
			// move along
			p += POS_DATA + bsize;
		}
	}
}

/*
==============
DemoSetBuf

Sets position in the buf for writing to specific client
==============
*/

static void DemoSetBuf(byte type, int to)
{
	byte *p;
	int pos = 0;
	int	btype, bto, bsize;

	p = msgbuf->data;
	msgbuf->curtype = -1;
	while (pos < msgbuf->bufsize)
	{
		btype = *p;
		bto = *(int*)(p+POS_TO);
		bsize = *(int*)(p+POS_SIZE);
		pos += POS_DATA + (bsize &(~BUF_FULL));

		if (type == btype && to == bto && !(bsize &(~BUF_FULL)))
		{
			msgbuf->cursize = pos;
			msgbuf->size = (int*)(p + POS_SIZE);
			msgbuf->curtype = type;
			msgbuf->curto = to;
			return;
		}
		p += POS_DATA + (bsize &(~BUF_FULL));
	}
	// type&&to not exist in the buf, so add it

	*p = type;
	*(int*)(p + POS_TO) = to;
	*(int*)(p + POS_SIZE) = 0;
	msgbuf->bufsize += POS_DATA;
	demo.bufsize += POS_DATA;
	msgbuf->size = (int*)(p + POS_SIZE);
	msgbuf->curtype = type;
	msgbuf->curto = to;
	msgbuf->cursize = msgbuf->bufsize;
}

void DemoWrite_Begin(byte type, int to, int size)
{
	byte *p;

	if (!(sworld.options & O_CONVERT))
		return;

	// signon message order cannot be changed
	if (!world.signonloaded) {
		if (msgbuf->curtype && (msgbuf->curtype != type || msgbuf->curto != to))
		{
			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "to disk (cur:%d, new:%d)\n", msgbuf->curtype, type);
			DemoWriteToDisk(0,0, demo.time);
		}
	}

	// will it fit? 
	while (msgbuf->bufsize + size + POS_DATA > msgbuf->maxsize) {
		if (!world.signonloaded)
			DemoWriteToDisk(0,0, demo.time); // dump everything on disk
		else WritePackets(1);
	}

	if (msgbuf->curtype != type || msgbuf->curto != to) {
		DemoSetBuf(type, to);
	}

	
	if (*msgbuf->size + size > msgmax)
	{
		*msgbuf->size |= BUF_FULL;
		DemoSetBuf(type, to);
	}
	

	// we have to make room for new data
	if (msgbuf->cursize != msgbuf->bufsize) {
		p = msgbuf->data + msgbuf->cursize;
		memmove(p+size, p, msgbuf->bufsize - msgbuf->cursize);
	}

	msgbuf->bufsize += size;
	*msgbuf->size += size;
	demo.bufsize += size;
}

/*
====================
WriteDemoMessage

Dumps the current net message, prefixed by the length and time
====================
*/
void WriteDemoMessage (sizebuf_t *msg, int type, int to, float time)
{
	int		len, i, msec;
	byte	c;
	static float prevtime;

	if (sworld.demo.file == NULL)
		return;

	msec = (time - prevtime)*1000;
	prevtime = time;
	
	if (msec > 255) msec = 255;

	if (msgmax != MAX_MSGLEN)
		msec = 0;

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
			Dem_Stop();
			return;
		}
	} else {
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
{
	byte	swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner	
	if ( *(short *)swaptest == 1)
	{
		bigendien = false;
		BigShort = ShortSwap;
		LittleShort = ShortNoSwap;
		BigLong = LongSwap;
		LittleLong = LongNoSwap;
		BigFloat = FloatSwap;
		LittleFloat = FloatNoSwap;
	}
	else
	{
		bigendien = true;
		BigShort = ShortNoSwap;
		LittleShort = ShortSwap;
		BigLong = LongNoSwap;
		LittleLong = LongSwap;
		BigFloat = FloatNoSwap;
		LittleFloat = FloatSwap;
	}
}

/*
============
StripExtension
============
*/
void StripExtension (char *in, char *out)
{
	char *p;

	strcpy(out, in);
	p = out + strlen(out);
	while (p > out && *p != '.') {
		p--;
	}

	if (*p == '.')
		*p = 0;
}

/*
============
FileExtension
============
*/

char *FileExtension (char *in)
{
	static char exten[8];
	int		i;
	char	*p;

	p = in + strlen(in);

	while (p > in && *p != '.')
		p--;

	if (p == in)
		return "";

	for (i=0 ; i<7 && *p; i++,p++)
		exten[i] = *p;
	exten[i] = 0;

	return exten;
}

/*
==================
DefaultExtension

If path doesn't have a .EXT, append extension
(extension should include the .)
==================
*/

void DefaultExtension (char *path, char *extension)
{
	char    *src;

	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return;                 // it has an extension
		src--;
	}

	strncat (path, extension, MAX_OSPATH);
}

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

	strncat (path, extension, MAX_OSPATH);
}

char *TemplateName (char *from, char *to)
{
	static char name[MAX_OSPATH];
	char tmp[MAX_OSPATH];
	char *p;

	memset(name, 0, sizeof(name));

	if ( !(p = strstr(to, "*")) )
		return to;

	StripExtension(from, tmp);
	strncpy(name, to, p - to);
	strcat(name,va("%s%s",tmp, p+1));

	return name;
}

//============================================================================

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
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
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
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

	strcpy(p ,parm);
	com_argv[com_argc++] = p;
	p += strlen(parm) + 1;
}

void RemoveParm (int num)
{
	if (num < 1 || num > com_argc)
		return;

	while(num < com_argc)
		com_argv[num++] = com_argv[num + 1];

	com_argc--;
}


void InitArgv (int argc, char **argv)
{
	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ;
		 com_argc++)
	{
		com_argv[com_argc] = argv[com_argc];
	}
}

void *Q_Malloc (size_t size)
{
	void *p;

	p = malloc(size);
	if (!p)
		Sys_Error ("Not enough memory free; check disk space\n");

	return p;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char	*va(char *format, ...)
{
	va_list		argptr;
	static char		string[4][1024];
	static int		index = 0;
	
	index = (index+1)&3;
	va_start (argptr, format);
	vsprintf (string[index], format,argptr);
	va_end (argptr);

	return string[index];	
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

	f = fopen(path, "rb");
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
	int	len;

	if ((len = FileOpenRead(path, &f)) == -1)
		return NULL;

	buf = Q_Malloc(len+1);

	fread(buf, 1, len, f);
	Sys_fclose(f);

	return buf;
}

vec_t Length(vec3_t v)
{
	int		i;
	float	length;
	
	length = 0;
	for (i=0 ; i< 3 ; i++)
		length += v[i]*v[i];
	length = sqrt (length);		// FIXME

	return length;
}
