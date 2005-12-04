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
 
	$Id: tools.h,v 1.4 2005/12/04 05:39:33 disconn3ct Exp $
*/

typedef struct
{
	int frame;
	byte source;
	byte type;
	byte full;
	int to;
	int size;
	byte data[1];
}
header_t;

typedef struct sizebuf_s
{
	byte	*data;
	int		maxsize;
	int		cursize;
	int		bufsize;
	header_t *h;
}
sizebuf_t;

typedef struct
{
	byte	*data;
	int		start, end, last;
	int		maxsize;
	sizebuf_t *msgbuf;
}
dbuffer_t;

extern sizebuf_t	*msgbuf;

typedef struct
{
	char **list;
	char path[MAX_OSPATH];
	int  count;
}
flist_t;


void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf


#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)

#define	MAX_INFO_STRING	196
#define	MAX_SERVERINFO_STRING	512
#define	MAX_LOCALINFO_STRING	32768

//============================================================================

short	ShortSwap (short l);
int		LongSwap (int l);
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
qboolean MSG_Forward (sizebuf_t *sb, int start, int count);

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

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

char *Info_ValueForKey (char *s, char *key);

#define MAX_NUM_ARGVS	50

extern	int		com_argc;
extern	char	*com_argv[MAX_NUM_ARGVS];

int CheckParm (char *parm);
void AddParm (char *parm);
void RemoveParm (int num);

void Tools_Init (void);
void Argv_Init (int argc, char **argv);

void StripExtension (char *in, char *out);
char *FileExtension (char *in);
void DefaultExtension (char *path, char *extension);
void ForceExtension (char *path, char *extension);
char *TemplateName (char *dst, char *src, char *ch);

char *getPath(char *path);
int AddToFileList(flist_t*filelist, char *file);
void FreeFileList(flist_t*filelist);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer

int fileLength (FILE *f);
int FileOpenRead (char *path, FILE **hndl);
byte *LoadFile(char *path);

#define DemoBuffer_Clear(b)	{(b)->start = (b)->end = (b)->last = 0;(b)->msgbuf = NULL;}

void MVDBuffer_Init(dbuffer_t *dbuffer, byte *buf, size_t size, sizebuf_t *msg);
void DemoBuffer_Set(dbuffer_t *dbuffer);
void MVDSetMsgBuf(dbuffer_t *dbuffer, sizebuf_t *cur);
void MVDWrite_Begin(byte type, int to, int size);
void DemoWrite_Cat(sizebuf_t *buf);
void SV_MVDWriteToDisk(sizebuf_t *m, int type, int to, float time);
void WriteDemoMessage (sizebuf_t *msg, int type, int to, float time);

vec_t Length(vec3_t v);

/*
#define MAXSIZE (demobuffer->end < demobuffer->last ? \
				demobuffer->start - demobuffer->end : \
				demobuffer->maxsize - demobuffer->end)
*/
#define MAXSIZE(d) ((d)->end < (d)->last ? \
				(d)->start - (d)->end : \
				(d)->maxsize - (d)->end)






