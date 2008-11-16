/*
	msg.c
*/

#include "qwfwd.h"

//===========================================================================

void SZ_InitEx (sizebuf_t *buf, byte *data, const int length, qbool allowoverflow)
{
	memset (buf, 0, sizeof (*buf));
	buf->data = data;
	buf->maxsize = length;
	buf->allowoverflow = allowoverflow;
}

void SZ_Init (sizebuf_t *buf, byte *data, const int length)
{
	SZ_InitEx (buf, data, length, false);
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void *SZ_GetSpace (sizebuf_t *buf, const int length)
{
	void *data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set (%d/%d/%d)",
			           buf->cursize, length, buf->maxsize);

		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %d/%d is > full buffer size",
			           length, buf->maxsize);

		// because Sys_Printf may be redirected
		Sys_Printf ("SZ_GetSpace: overflow: cur = %d, len = %d, max = %d\n",
		            buf->cursize, length, buf->maxsize);
		SZ_Clear (buf);
		buf->overflowed = true;
	}

	data = buf->data + buf->cursize;
	buf->cursize += length;

	return data;
}

void SZ_Write (sizebuf_t *buf, const void *data, int length)
{
	memcpy (SZ_GetSpace (buf, length), data, length);
}

void SZ_Print (sizebuf_t *buf, const char *data)
{
	int len = strlen (data) + 1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace (buf, len), data, len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace (buf, len - 1) - 1, data, len); // write over trailing 0
}

//===========================================================================

// reading

static int msg_readcount;
static qbool msg_badread;

void MSG_BeginReading (void)
{
	msg_readcount = 0;
	msg_badread = false;
}

int MSG_GetReadCount (void)
{
	return msg_readcount;
}

qbool MSG_BadRead (void)
{
	return msg_badread;
}

// returns -1 and sets msg_badread if no more characters are available
int MSG_ReadChar (void)
{
	int c;

	if (msg_readcount + 1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (signed char) net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadByte (void)
{
	int c;

	if (msg_readcount + 1 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (unsigned char) net_message.data[msg_readcount];
	msg_readcount++;

	return c;
}

int MSG_ReadShort (void)
{
	int c;

	if (msg_readcount + 2 > net_message.cursize)
	{
		msg_badread = true;
		return -1;
	}

	c = (short) (net_message.data[msg_readcount]
		+ (net_message.data[msg_readcount+1]<<8));

	msg_readcount += 2;

	return c;
}

int MSG_ReadLong (void)
{
	int c;

	if (msg_readcount + 4 > net_message.cursize)
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
		byte b[4];
		float f;
		int l;
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
	} while (l < sizeof (string) - 1);

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
	} while (l < sizeof (string) - 1);

	string[l] = 0;

	return string;
}

//===========================================================================

// writing functions

void MSG_WriteChar (sizebuf_t *sb, const int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, const int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = (byte *) SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, const int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

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
		float f;
		int l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, const char *s)
{
	if (!s || !*s)
		SZ_Write (sb, "", 1);
	else
		SZ_Write (sb, s, strlen(s)+1);
}

