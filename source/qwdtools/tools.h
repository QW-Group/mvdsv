typedef struct sizebuf_s
{
	byte	*data;
	int		maxsize;
	int		cursize;
	int		*size;
	int		bufsize;
	int		curtype, curto;
} sizebuf_t;

extern int			msgmax;
extern sizebuf_t	*msgbuf;


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

extern	qboolean		bigendien;

extern	short	(*BigShort) (short l);
extern	short	(*LittleShort) (short l);
extern	int	(*BigLong) (int l);
extern	int	(*LittleLong) (int l);
extern	float	(*BigFloat) (float l);
extern	float	(*LittleFloat) (float l);

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
void InitArgv (int argc, char **argv);

void StripExtension (char *in, char *out);
char *FileExtension (char *in);
void DefaultExtension (char *path, char *extension);
void ForceExtension (char *path, char *extension);
char *TemplateName (char *dst, char *src);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer

int fileLength (FILE *f);
int FileOpenRead (char *path, FILE **hndl);
byte *LoadFile(char *path);

void DemoWrite_Begin(byte type, int to, int size);
void DemoWriteToDisk(int type, int to, float time);
void WriteDemoMessage (sizebuf_t *msg, int type, int to, float time);

vec_t Length(vec3_t v);




