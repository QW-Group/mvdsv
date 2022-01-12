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

#ifndef __TOOLS_H__
#define __TOOLS_H__

typedef struct
{
	int frame;
	byte source;
	byte type;
	byte full;
	int to;
	int size;
	byte data[1];
} header_t;

typedef struct sizebuf_s
{
	byte *data;
	int	maxsize;
	int	cursize;
	int	bufsize;
	header_t *h;
} sizebuf_t;

typedef struct
{
	byte *data;
	int	start;
	int end;
	int last;
	int	maxsize;
	sizebuf_t *msgbuf;
} dbuffer_t;

typedef struct
{
	char **list;
	char path[MAX_OSPATH];
	int  count;
} flist_t;

extern sizebuf_t *msgbuf;
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, const int length);
void SZ_Write (sizebuf_t *buf, const void *data, const int length);
void SZ_Print (sizebuf_t *buf, const char *data);

extern struct usercmd_s nullcmd;
void MSG_WriteChar (sizebuf_t *sb, const int c);
void MSG_WriteByte (sizebuf_t *sb, const int c);
void MSG_WriteShort (sizebuf_t *sb, const int c);
void MSG_WriteLong (sizebuf_t *sb, const int c);
void MSG_WriteFloat (sizebuf_t *sb, const float f);
void MSG_WriteString (sizebuf_t *sb, const char *s);
void MSG_WriteCoord (sizebuf_t *sb, const float f);
void MSG_WriteAngle (sizebuf_t *sb, const float f);
void MSG_WriteAngle16 (sizebuf_t *sb, const float f);
void MSG_WriteDeltaUsercmd (sizebuf_t *sb, const usercmd_t *from, const usercmd_t *cmd);
qbool MSG_Forward (sizebuf_t *sb, const int start, const int count);

extern int msg_readcount;
extern qbool msg_badread; // set if a read goes beyond end of message
void MSG_BeginReading (void);
int MSG_GetReadCount(void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);
char *MSG_ReadStringLine (void);
float MSG_ReadLongCoord(void);
float MSG_ReadCoord (void);
float MSG_ReadAngle (void);
float MSG_ReadAngle16 (void);
void MSG_ReadDeltaUsercmd (const usercmd_t *from, usercmd_t *cmd);

char *Info_ValueForKey (char *s, char *key);

extern int com_argc;
extern char *com_argv[MAX_NUM_ARGVS];
int CheckParm (char *parm);
void AddParm (char *parm);
void RemoveParm (int num);

void Tools_Init (void);
void Argv_Init (int argc, char **argv);

void ForceExtension (char *path, char *extension);
char *TemplateName (char *dst, char *src, char *ch);

char *getPath (char *path);
int AddToFileList (flist_t*filelist, char *file);
void FreeFileList (flist_t*filelist);

int fileLength (FILE *f);
int FileOpenRead (char *path, FILE **hndl);
byte *LoadFile (char *path);

#define DemoBuffer_Clear(b) {(b)->start = (b)->end = (b)->last = 0;(b)->msgbuf = NULL;}

void MVDBuffer_Init (dbuffer_t *dbuffer, byte *buf, size_t size, sizebuf_t *msg);
void DemoBuffer_Set (dbuffer_t *dbuffer);
void MVDSetMsgBuf (dbuffer_t *dbuffer, sizebuf_t *cur);
void MVDWrite_Begin (byte type, int to, int size);
void DemoWrite_Cat (sizebuf_t *buf);
void SV_MVDWriteToDisk (sizebuf_t *m, int type, int to, float time);
void WriteDemoMessage (sizebuf_t *msg, int type, int to, float time);

vec_t VectorLength (vec3_t v);

#define MAXSIZE(d) ((d)->end < (d)->last ? (d)->start - (d)->end : (d)->maxsize - (d)->end)

#endif /* !__TOOLS_H__ */
