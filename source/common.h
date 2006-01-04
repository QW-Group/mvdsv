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

	$Id: common.h,v 1.11 2006/01/04 03:48:33 disconn3ct Exp $
*/
// common.h  -- general definitions

#ifndef _COMMON
#define _COMMON

#include "bothdefs.h"

typedef unsigned char byte;

// KJB Undefined true and false defined in SciTech's DEBUG.H header
#undef true
#undef false

typedef enum {false, true}	qboolean;

#define	MAX_INFO_STRING		196
#define	MAX_SERVERINFO_STRING	512
#define	MAX_LOCALINFO_STRING	32768
#define	MAX_KEY_STRING		64

//============================================================================
typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;	// set to true if the buffer size failed
	byte		*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

//============================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

//============================================================================

short	ShortSwap (short l);
int	LongSwap (int l);
float	FloatSwap (float f);

#ifdef __BIG_ENDIAN__
#define BigShort(x) (x)
#define BigLong(x) (x)
#define BigFloat(x) (x)
#define LittleShort(x) ShortSwap(x)
#define LittleLong(x) LongSwap(x)
#define LittleFloat(x) FloatSwap(x)
#else
#define BigShort(x) ShortSwap(x)
#define BigLong(x) LongSwap(x)
#define BigFloat(x) FloatSwap(x)
#define LittleShort(x) (x)
#define LittleLong(x) (x)
#define LittleFloat(x) (x)
#endif

//============================================================================

struct usercmd_s;

extern struct usercmd_s nullcmd;

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);
void MSG_WriteAngle16 (sizebuf_t *sb, float f);
void MSG_WriteDeltaUsercmd (sizebuf_t *sb, struct usercmd_s *from, struct usercmd_s *cmd);

extern	int		msg_readcount;
extern	qboolean	msg_badread; // set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_GetReadCount(void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);
char *MSG_ReadStringLine (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
float MSG_ReadAngle16 (void);
void MSG_ReadDeltaUsercmd (struct usercmd_s *from, struct usercmd_s *cmd);

//============================================================================

#ifdef _WIN32
#define strcasecmp(s1, s2)	_stricmp  ((s1),   (s2))
#define strncasecmp(s1, s2, n)	_strnicmp ((s1),   (s2),   (n))
int snprintf(char *str, size_t n, char const *fmt, ...);
int vsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
#endif

#if defined(__linux__) || defined(_WIN32)
size_t	strlcpy (char *dst, char *src, size_t siz);
size_t	strlcat (char *dst, char *src, size_t siz);
#endif
#ifndef __FreeBSD__
char	*strnstr (char *s, char *find, size_t slen);
char	*strcasestr(const char *s, const char *find);
#endif

int	Q_atoi (char *str);
float	Q_atof (char *str);

char	*Q_normalizetext(unsigned char *name); //bliP: red to white text
char	*Q_redtext(unsigned char *str); //bliP: white to red text
char	*Q_yelltext(unsigned char *str); //VVD: white to red text and yellow numbers

//============================================================================

#define MAX_COM_TOKEN	1024
extern	char com_token[MAX_COM_TOKEN];

char *COM_Parse (char *data);


extern	int	com_argc;
extern	char	**com_argv;

int COM_CheckParm (char *parm);

void COM_Init (void);
void COM_InitArgv (int argc, char **argv);

void COM_StripExtension (char *in, char *out);
void COM_FileBase (char *in, char *out);
void COM_DefaultExtension (char *path, char *extension);
int COM_FileLength (FILE *f); //bliP: init

char *va(char *format, ...);
// does a varargs printf into a temp buffer


//============================================================================

extern int com_filesize;
struct cache_user_s;

extern char	com_gamedir[MAX_OSPATH];
extern char	com_basedir[MAX_OSPATH];

void COM_WriteFile (char *filename, void *data, int len);
int COM_FOpenFile (char *filename, FILE **file);

byte *COM_LoadStackFile (char *path, void *buffer, int bufsize);
byte *COM_LoadTempFile (char *path);
byte *COM_LoadHunkFile (char *path);
void COM_CreatePath (char *path);
char *COM_NextPath (char *prevpath);
void COM_Gamedir (char *dir);

char *Info_ValueForKey (char *s, char *key);
char *Info_KeyNameForKeyNum (char *s, int key);
void Info_RemoveKey (char *s, char *key);
void Info_RemovePrefixedKeys (char *start, char prefix);
void Info_SetValueForKey (char *s, char *key, char *value, int maxsize);
void Info_SetValueForStarKey (char *s, char *key, char *value, int maxsize);
void Info_Print (char *s);
void Info_CopyStarKeys (char *from, char *to);

unsigned Com_BlockChecksum (void *buffer, int length);
byte	COM_BlockSequenceCRCByte (byte *base, int length, int sequence);
#endif //_COMMON
