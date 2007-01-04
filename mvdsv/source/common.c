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

    $Id: common.c,v 1.33 2007/01/04 07:38:12 qqshka Exp $
*/
// common.c -- misc functions used in client and server

#include "qwsvdef.h"

usercmd_t nullcmd; // guarenteed to be zero

static char *largv[MAX_NUM_ARGVS + 1];

//============================================================================



/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

// writing functions

void MSG_WriteChar (sizebuf_t *sb, int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < -128 || c > 127)
		Sys_Error ("MSG_WriteChar: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteByte (sizebuf_t *sb, int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < 0 || c > 255)
		Sys_Error ("MSG_WriteByte: range error");
#endif

	buf = SZ_GetSpace (sb, 1);
	buf[0] = c;
}

void MSG_WriteShort (sizebuf_t *sb, int c)
{
	byte *buf;

#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Sys_Error ("MSG_WriteShort: range error");
#endif

	buf = SZ_GetSpace (sb, 2);
	buf[0] = c&0xff;
	buf[1] = c>>8;
}

void MSG_WriteLong (sizebuf_t *sb, int c)
{
	byte *buf;

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
		float f;
		int l;
	} dat;


	dat.f = f;
	dat.l = LittleLong (dat.l);

	SZ_Write (sb, &dat.l, 4);
}

void MSG_WriteString (sizebuf_t *sb, char *s)
{
	if (!s || !*s)
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
	MSG_WriteByte (sb, Q_rint(f*256.0/360.0) & 255);
}

void MSG_WriteAngle16 (sizebuf_t *sb, float f)
{
	MSG_WriteShort (sb, Q_rint(f*65536.0/360.0) & 65535);
}

void MSG_WriteDeltaUsercmd (sizebuf_t *buf, usercmd_t *from, usercmd_t *cmd)
{
	int bits;

	// send the movement message
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


// reading functions

int msg_readcount;
qbool msg_badread;

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
	int c;

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
	int c;

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
	int c;

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
	int c;

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

void MSG_ReadDeltaUsercmd (usercmd_t *from, usercmd_t *move)
{
	int bits;

	memcpy (move, from, sizeof(*move));

	bits = MSG_ReadByte ();

	// read current angles
	if (bits & CM_ANGLE1)
	{
		move->angles[0] = MSG_ReadAngle16 ();
	}
	if (bits & CM_ANGLE2)
	{
		move->angles[1] = MSG_ReadAngle16 ();
	}
	if (bits & CM_ANGLE3)
	{
		move->angles[2] = MSG_ReadAngle16 ();
	}

	// read movement
	if (bits & CM_FORWARD)
	{
		move->forwardmove = MSG_ReadShort ();
	}
	if (bits & CM_SIDE)
	{
		move->sidemove = MSG_ReadShort ();
	}
	if (bits & CM_UP)
	{
		move->upmove = MSG_ReadShort ();
	}

	// read buttons
	if (bits & CM_BUTTONS)
		move->buttons = MSG_ReadByte ();

	if (bits & CM_IMPULSE)
		move->impulse = MSG_ReadByte ();

	// read time to run command
	move->msec = MSG_ReadByte ();
}


//===========================================================================

void SZ_Init (sizebuf_t *buf, byte *data, int length)
{
	memset (buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
}

void SZ_Clear (sizebuf_t *buf)
{
	buf->cursize = 0;
	buf->overflowed = false;
}

void *SZ_GetSpace (sizebuf_t *buf, int length)
{
	void *data;

	if (buf->cursize + length > buf->maxsize)
	{
		if (!buf->allowoverflow)
			Sys_Error ("SZ_GetSpace: overflow without allowoverflow set (%i/%i/%i)",
			           buf->cursize, length, buf->maxsize);

		if (length > buf->maxsize)
			Sys_Error ("SZ_GetSpace: %i/%i is > full buffer size",
			           length, buf->maxsize);

		// because Con_Printf may be redirected
		Sys_Printf ("SZ_GetSpace: overflow: cur = %i, len = %i, max = %i\n",
		            buf->cursize, length, buf->maxsize);
		SZ_Clear (buf);
		buf->overflowed = true;
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
	int len;

	len = strlen(data)+1;

	if (!buf->cursize || buf->data[buf->cursize-1])
		memcpy ((byte *)SZ_GetSpace(buf, len),data,len); // no trailing 0
	else
		memcpy ((byte *)SZ_GetSpace(buf, len-1)-1,data,len); // write over trailing 0
}


//============================================================================

#define TOKENSIZE sizeof(com_token)
char com_token[TOKENSIZE];
int com_argc;
char **com_argv;

com_tokentype_t com_tokentype;

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	unsigned char c;
	int len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

	// skip whitespace
	while (true)
	{
		while ( (c = *data) == ' ' || c == '\t' || c == '\r' || c == '\n')
			data++;

		if (c == 0)
			return NULL; // end of file;

		// skip // comments
		if (c=='/' && data[1] == '/')
		{
			while (*data && *data != '\n')
				data++;
		}
		else
			break;
	}

	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				if (!c)
					data--;
				return data;
			}
			if (len < MAX_COM_TOKEN-1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_COM_TOKEN-1)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	}
	while (c && c != ' ' && c != '\t' && c != '\n' && c != '\r');

	com_token[len] = 0;
	return data;
}

#define DEFAULT_PUNCTUATION "(,{})(\':;=!><&|+"
char *COM_ParseToken (const char *data, const char *punctuation)
{
	int		c;
	int		len;

	if (!punctuation)
		punctuation = DEFAULT_PUNCTUATION;

	len = 0;
	com_token[0] = 0;

	if (!data)
	{
		com_tokentype = TTP_UNKNOWN;
		return NULL;
	}

// skip whitespace
skipwhite:
	while ( (c = *(unsigned char*)data) <= ' ')
	{
		if (c == 0)
		{
			com_tokentype = TTP_UNKNOWN;
			return NULL;			// end of file;
		}
		data++;
	}

// skip // comments
	if (c=='/')
	{
		if (data[1] == '/')
		{
			while (*data && *data != '\n')
				data++;
			goto skipwhite;
		}
		else if (data[1] == '*')
		{
			data+=2;
			while (*data && (*data != '*' || data[1] != '/'))
				data++;
			data+=2;
			goto skipwhite;
		}
	}


// handle quoted strings specially
	if (c == '\"')
	{
		com_tokentype = TTP_STRING;
		data++;
		while (1)
		{
			if (len >= TOKENSIZE-1)
			{
				com_token[len] = '\0';
				return (char*)data;
			}
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				return (char*)data;
			}
			com_token[len] = c;
			len++;
		}
	}

	com_tokentype = TTP_UNKNOWN;

// parse single characters
	if (strchr(punctuation, c))
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return (char*)(data+1);
	}

// parse a regular word
	do
	{
		if (len >= TOKENSIZE-1)
			break;
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		if (strchr(punctuation, c))
			break;
	} while (c>32);

	com_token[len] = 0;
	return (char*)data;
}

/*
================
COM_InitArgv

================
*/
void COM_InitArgv (int argc, char **argv)
{
	for (com_argc=0 ; (com_argc<MAX_NUM_ARGVS) && (com_argc < argc) ; com_argc++)
	{
		if (argv[com_argc])
			largv[com_argc] = argv[com_argc];
		else
			largv[com_argc] = "";
	}

	largv[com_argc] = "";
	com_argv = largv;
}

/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter appears, or 0 if not present
================
*/
int COM_CheckParm (char *parm)
{
	int i;

	for (i=1 ; i<com_argc ; i++)
	{
		if (!strcmp (parm,com_argv[i]))
			return i;
	}

	return 0;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

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
	static	char value[4][512]; // use two buffers so compares
	// work without stomping on each other
	static	int valueindex;
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


char *Info_KeyNameForKeyNum (char *s, int key)
{
	static char pkey[4][512]; // use two buffers so compares
	// work without stomping on each other
	static int keyindex;
	char *o;

	keyindex = (keyindex + 1) % 4;
	if (*s == '\\')
		s++;

	// ?
	if (key < 1)
		return NULL;

	while (1)
	{
		key--;

		// get key name
		o = pkey[keyindex];
		while (*s != '\\')
		{
			if (!*s || o >= pkey[keyindex] + sizeof(pkey[keyindex]) - 1)
				return NULL;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		// skip key value
		while (*s != '\\' && *s) s++;

		if (!key)
			return pkey[keyindex];

		if (!*s)
			return NULL;
		s++;
	}
}


void Info_RemoveKey (char *s, char *key)
{
	char *start;
	char pkey[512];
	char value[512];
	char *o;

	if (strstr (key, "\\"))
	{
		Con_Printf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s || o >= pkey + sizeof(pkey) - 1)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s || o >= value + sizeof(value) - 1)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strncmp (key, pkey, sizeof(pkey)) )
		{
			strlcpy (start, s, strlen(s) + 1);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_RemovePrefixedKeys (char *start, char prefix)
{
	char *s;
	char pkey[512];
	char value[512];
	char *o;

	s = start;

	while (1)
	{
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s || o >= pkey + sizeof(pkey) - 1)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s || o >= value + sizeof(value) - 1)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (pkey[0] == prefix)
		{
			Info_RemoveKey (start, pkey);
			s = start;
		}

		if (!*s)
			return;
	}

}


void Info_SetValueForStarKey (char *s, char *key, char *value, unsigned int maxsize)
{
	char _new[1024], *v;
	int c;
//	extern cvar_t sv_highchars;

	if (strstr (key, "\\") || strstr (value, "\\") )
	{
		Con_Printf ("Can't use keys or values with a \\\n");
		return;
	}

	if (strstr (key, "\"") || strstr (value, "\"") )
	{
		Con_Printf ("Can't use keys or values with a \"\n");
		return;
	}

	if (strlen(key) >= MAX_KEY_STRING || strlen(value) >= MAX_KEY_STRING)
	{
		Con_Printf ("Keys and values must be < %d characters.\n", MAX_KEY_STRING);
		return;
	}

	// this next line is kinda trippy
	if (*(v = Info_ValueForKey(s, key)))
	{
		// key exists, make sure we have enough room for new value, if we don't,
		// don't change it!
		if (strlen(value) - strlen(v) + strlen(s) >= maxsize)
		{
			Con_Printf ("Info string length exceeded (change: key = '%s', old value = '%s', new value = '%s', strlen info = '%d', maxsize = '%d')\nFull info string: '%s'\n",
			            key, v, value, strlen(s), maxsize, s);
			return;
		}
	}
	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	snprintf (_new, sizeof(_new), "\\%s\\%s", key, value);

	if (strlen(_new) + strlen(s) >= maxsize)
	{
		Con_Printf ("Info string length exceeded (add: key = '%s', value = '%s', strlen info = '%d', maxsize = '%d')\nFull info string: '%s'\n",
		            key, value, strlen(s), maxsize, s);
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = _new;
	while (*v)
	{
		c = (unsigned char)*v++;
/*
		if (!(int)sv_highchars.value)
		{
			c &= 127;
			if (c < 32 || c > 127)
				continue;
		}
*/
		// c &= 127; // strip high bits
		if (c > 13) // && c < 127)
			*s++ = c;
	}
	*s = 0;
}

void Info_SetValueForKey (char *s, char *key, char *value, unsigned int maxsize)
{
	if (key[0] == '*')
	{
		Con_Printf ("Can't set * keys\n");
		return;
	}

	Info_SetValueForStarKey (s, key, value, maxsize);
}

void Info_Print (char *s)
{
	char key[512];
	char value[512];
	char *o;
	int l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Con_Printf ("%s ", key);

		if (!*s)
		{
			Con_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Con_Printf ("%s\n", value);
	}
}

void Info_CopyStarKeys (char *from, char *to)
{
	char key[512];
	char value[512];
	char *o;

	if (*from == '\\')
		from++;

	while (*from)
	{
		o = key;
		while (*from && *from != '\\')
			*o++ = *from++;

		*o = 0;

		o = value;
		from++;
		while (*from && *from != '\\')
			*o++ = *from++;
		*o = 0;

		if (*from)
			from++;
		if (key[0] == '*')
			Info_SetValueForStarKey (to, key, value, MAX_INFO_STRING);
	}
}

static byte chktbl[1024] = {
	0x78,0xd2,0x94,0xe3,0x41,0xec,0xd6,0xd5,0xcb,0xfc,0xdb,0x8a,0x4b,0xcc,0x85,0x01,
	0x23,0xd2,0xe5,0xf2,0x29,0xa7,0x45,0x94,0x4a,0x62,0xe3,0xa5,0x6f,0x3f,0xe1,0x7a,
	0x64,0xed,0x5c,0x99,0x29,0x87,0xa8,0x78,0x59,0x0d,0xaa,0x0f,0x25,0x0a,0x5c,0x58,
	0xfb,0x00,0xa7,0xa8,0x8a,0x1d,0x86,0x80,0xc5,0x1f,0xd2,0x28,0x69,0x71,0x58,0xc3,
	0x51,0x90,0xe1,0xf8,0x6a,0xf3,0x8f,0xb0,0x68,0xdf,0x95,0x40,0x5c,0xe4,0x24,0x6b,
	0x29,0x19,0x71,0x3f,0x42,0x63,0x6c,0x48,0xe7,0xad,0xa8,0x4b,0x91,0x8f,0x42,0x36,
	0x34,0xe7,0x32,0x55,0x59,0x2d,0x36,0x38,0x38,0x59,0x9b,0x08,0x16,0x4d,0x8d,0xf8,
	0x0a,0xa4,0x52,0x01,0xbb,0x52,0xa9,0xfd,0x40,0x18,0x97,0x37,0xff,0xc9,0x82,0x27,
	0xb2,0x64,0x60,0xce,0x00,0xd9,0x04,0xf0,0x9e,0x99,0xbd,0xce,0x8f,0x90,0x4a,0xdd,
	0xe1,0xec,0x19,0x14,0xb1,0xfb,0xca,0x1e,0x98,0x0f,0xd4,0xcb,0x80,0xd6,0x05,0x63,
	0xfd,0xa0,0x74,0xa6,0x86,0xf6,0x19,0x98,0x76,0x27,0x68,0xf7,0xe9,0x09,0x9a,0xf2,
	0x2e,0x42,0xe1,0xbe,0x64,0x48,0x2a,0x74,0x30,0xbb,0x07,0xcc,0x1f,0xd4,0x91,0x9d,
	0xac,0x55,0x53,0x25,0xb9,0x64,0xf7,0x58,0x4c,0x34,0x16,0xbc,0xf6,0x12,0x2b,0x65,
	0x68,0x25,0x2e,0x29,0x1f,0xbb,0xb9,0xee,0x6d,0x0c,0x8e,0xbb,0xd2,0x5f,0x1d,0x8f,
	0xc1,0x39,0xf9,0x8d,0xc0,0x39,0x75,0xcf,0x25,0x17,0xbe,0x96,0xaf,0x98,0x9f,0x5f,
	0x65,0x15,0xc4,0x62,0xf8,0x55,0xfc,0xab,0x54,0xcf,0xdc,0x14,0x06,0xc8,0xfc,0x42,
	0xd3,0xf0,0xad,0x10,0x08,0xcd,0xd4,0x11,0xbb,0xca,0x67,0xc6,0x48,0x5f,0x9d,0x59,
	0xe3,0xe8,0x53,0x67,0x27,0x2d,0x34,0x9e,0x9e,0x24,0x29,0xdb,0x69,0x99,0x86,0xf9,
	0x20,0xb5,0xbb,0x5b,0xb0,0xf9,0xc3,0x67,0xad,0x1c,0x9c,0xf7,0xcc,0xef,0xce,0x69,
	0xe0,0x26,0x8f,0x79,0xbd,0xca,0x10,0x17,0xda,0xa9,0x88,0x57,0x9b,0x15,0x24,0xba,
	0x84,0xd0,0xeb,0x4d,0x14,0xf5,0xfc,0xe6,0x51,0x6c,0x6f,0x64,0x6b,0x73,0xec,0x85,
	0xf1,0x6f,0xe1,0x67,0x25,0x10,0x77,0x32,0x9e,0x85,0x6e,0x69,0xb1,0x83,0x00,0xe4,
	0x13,0xa4,0x45,0x34,0x3b,0x40,0xff,0x41,0x82,0x89,0x79,0x57,0xfd,0xd2,0x8e,0xe8,
	0xfc,0x1d,0x19,0x21,0x12,0x00,0xd7,0x66,0xe5,0xc7,0x10,0x1d,0xcb,0x75,0xe8,0xfa,
	0xb6,0xee,0x7b,0x2f,0x1a,0x25,0x24,0xb9,0x9f,0x1d,0x78,0xfb,0x84,0xd0,0x17,0x05,
	0x71,0xb3,0xc8,0x18,0xff,0x62,0xee,0xed,0x53,0xab,0x78,0xd3,0x65,0x2d,0xbb,0xc7,
	0xc1,0xe7,0x70,0xa2,0x43,0x2c,0x7c,0xc7,0x16,0x04,0xd2,0x45,0xd5,0x6b,0x6c,0x7a,
	0x5e,0xa1,0x50,0x2e,0x31,0x5b,0xcc,0xe8,0x65,0x8b,0x16,0x85,0xbf,0x82,0x83,0xfb,
	0xde,0x9f,0x36,0x48,0x32,0x79,0xd6,0x9b,0xfb,0x52,0x45,0xbf,0x43,0xf7,0x0b,0x0b,
	0x19,0x19,0x31,0xc3,0x85,0xec,0x1d,0x8c,0x20,0xf0,0x3a,0xfa,0x80,0x4d,0x2c,0x7d,
	0xac,0x60,0x09,0xc0,0x40,0xee,0xb9,0xeb,0x13,0x5b,0xe8,0x2b,0xb1,0x20,0xf0,0xce,
	0x4c,0xbd,0xc6,0x04,0x86,0x70,0xc6,0x33,0xc3,0x15,0x0f,0x65,0x19,0xfd,0xc2,0xd3,
	// Only the first 512 bytes of the table are initialized, the rest
	// is just zeros.
	// This is an idiocy in QW but we can't change this, or checksums
	// will not match.
};

/*
====================
COM_BlockSequenceCRCByte

For proxy protecting
====================
*/
byte COM_BlockSequenceCRCByte (byte *base, int length, int sequence)
{
	unsigned short crc;
	byte *p;
	byte chkb[60 + 4];

	p = chktbl + ((unsigned int)sequence % (sizeof(chktbl) - 4));

	if (length > 60)
		length = 60;
	memcpy (chkb, base, length);

	chkb[length] = (sequence & 0xff) ^ p[0];
	chkb[length+1] = p[1];
	chkb[length+2] = ((sequence>>8) & 0xff) ^ p[2];
	chkb[length+3] = p[3];

	length += 4;

	crc = CRC_Block (chkb, length);

	crc &= 0xff;

	return crc;
}


//============================================================================

///////////////////////////////////////////////////////////////
//	MD4-based checksum utility functions
//
//	Copyright (C) 2000       Jeff Teunissen <d2deek@pmail.net>
//
//	Author: Jeff Teunissen	<d2deek@pmail.net>
//	Date: 01 Jan 2000

unsigned Com_BlockChecksum (void *buffer, int length)
{
	int digest[4];
	unsigned val;

	mdfour ((unsigned char *)digest, (unsigned char *)buffer, length);

	val = digest[0] ^ digest[1] ^ digest[2] ^ digest[3];

	return val;
}

void Com_BlockFullChecksum (void *buffer, int len, unsigned char *outbuf)
{
	mdfour ( outbuf, (unsigned char *) buffer, len );
}
