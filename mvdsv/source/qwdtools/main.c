#include "defs.h"

int memsize, membase;
int	fps;

sizebuf_t	net_message;
byte		net_message_buffer[MAX_UDP_PACKET];

demo_t		demo;
source_t	from;

world_state_t	world;
static_world_state_t	sworld;
lightstyle_t	lightstyle[MAX_LIGHTSTYLES];
entity_state_t	baselines[MAX_EDICTS];
float			realtime;
sizebuf_t		stats_msg;
byte			stats_buf[MAX_MSGLEN];
char			currentDir[MAX_OSPATH];
char			qizmoDir[MAX_OSPATH];
char			outputDir[MAX_OSPATH];
char			sourceName[MAX_SOURCES][MAX_OSPATH];
HANDLE			ConsoleInHndl, ConsoleOutHndl;

void QWDToolsMsg(void);

/*
================
Sys_Error
================
*/

void Sys_fclose(FILE *hndl)
{
	if (hndl == NULL)
		return;
	if (hndl == stdin)
		return;
	if (hndl == stdout)
		return;
	fclose(hndl);
}

void Sys_Exit (int i)
{
	DWORD r;
	struct _INPUT_RECORD in;

	Dem_Stop();

	if ( sworld.options & O_WAITFORKBHIT)
	{
		if (i != 2)
			SetConsoleTitle("qwdtools  Done");

		Sys_Printf("\nPress any key to continue\n");
		do ReadConsoleInput( ConsoleInHndl, &in, 1, &r); while(in.EventType != KEY_EVENT);
	}

	Sys_fclose(sworld.demo.file);
	Sys_fclose(sworld.from.file);
	Sys_fclose(sworld.debug.file);
	Sys_fclose(sworld.log.file);
	Sys_fclose(sworld.analyse.file);

	exit(i);
}

void Sys_mkdir (char *path)
{
	_mkdir (path);
}


void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

	
	if (sworld.from.file != NULL && sworld.from.file != stdin) {
		Sys_Printf ("ERROR: %s, at:%d\n", text, ftell(sworld.from.file));
	} else {
		Sys_Printf ("ERROR: %s\n", text);
	}

	Sys_Exit (1);
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		buf[1024];
	DWORD		count;

	// stdout can be redirected to a file or other process
	if (ConsoleOutHndl != NULL && sworld.options & O_STDOUT) {
		va_start (argptr,fmt);
		vsprintf(buf, fmt, argptr);
		va_end (argptr);
		WriteFile(ConsoleOutHndl, buf, strlen(buf), &count, NULL);
		return;
	}

	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

/*
=================
ConnectionlessPacket

Responses to broadcasts, etc
=================
*/

extern int msg_startcount;

/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/

qboolean Netchan_Process (netchan_t *chan)
{
	unsigned		sequence, sequence_ack;
	unsigned		reliable_ack, reliable_message;

// get sequence numbers
	MSG_BeginReading ();
	sequence = MSG_ReadLong ();
	sequence_ack = MSG_ReadLong ();

	// read the qport if we are a server
	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);	
	sequence_ack &= ~(1<<31);	

//
// discard stale or duplicated packets
//
	if (sequence <= (unsigned)chan->incoming_sequence)
	{
		return false;
	}

//
// if the current outgoing reliable message has been acknowledged
// clear the buffer to make way for the next
//
	if (reliable_ack == (unsigned)chan->reliable_sequence)
		chan->reliable_length = 0;	// it has been received
	
//
// if this message contains a reliable message, bump incoming_reliable_sequence 
//
	chan->incoming_sequence = sequence;
	chan->incoming_acknowledged = sequence_ack;
	chan->incoming_reliable_acknowledged = reliable_ack;
	if (reliable_message)
		chan->incoming_reliable_sequence ^= 1;

	return true;
}


void ConnectionlessPacket (void)
{
	int		c;

    MSG_BeginReading ();
    MSG_ReadLong ();        // skip the -1

	c = MSG_ReadByte ();
	if (c == S2C_CONNECTION)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "connected\n");

		from.netchan.incoming_sequence = 0;
		from.netchan.incoming_acknowledged= 0;
		from.netchan.incoming_acknowledged = 0;
		from.netchan.incoming_reliable_acknowledged = 0;
		return;
	}

	// remote command from gui front end
	if (c == A2C_CLIENT_COMMAND)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "A2C\n");
		MSG_ReadString ();
		MSG_ReadString ();

		return;
	}

	// print command from somewhere
	if (c == A2C_PRINT)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "print\n");
		MSG_ReadString ();
		return;
	}

	// ping from somewhere
	if (c == A2A_PING)
	{
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "ping\n");
		return;
	}

	if (c == S2C_CHALLENGE) {
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "challenge\n");
		MSG_ReadString ();
		return;
	}

	if (c == svc_disconnect) {
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "disconnect\n");
		Dem_Stop();
		return;
	}
}

void Dem_Stop(void)
{
	StopQWZ();

	if (!from.running)
		return;

	from.running = false;
	msgbuf->cursize = 0;
	msgbuf->bufsize = 0;
	msgbuf->curtype = 0;

	QWDToolsMsg();

	DemoWrite_Begin(dem_all, 0, 2+strlen("EndOfDemo"));
	MSG_WriteByte(msgbuf, svc_disconnect);
	MSG_WriteString (msgbuf, "EndOfDemo");
}

/*
====================
GetDemoMessage
====================
*/

float olddemotime;

qboolean GetDemoMessage (void)
{
	int		r, i, j;
	float	f, demotime;
	byte	newtime;
	byte	c;
	usercmd_t *pcmd;
	static int oldnewtime = 0;

	if (from.prevtime < realtime)
		from.prevtime = realtime;

nextdemomessage:

	// read the time from the packet
	newtime = 0;
	if (!from.lasttime) 
	{
		if (from.format == mvd)
		{
			r = fread(&newtime, sizeof(newtime), 1, sworld.from.file);
			demotime =  from.prevtime + newtime*0.001;
		} else {
			r = fread(&demotime, sizeof(demotime), 1, sworld.from.file);
			demotime = LittleFloat(demotime);
		}

		if (r != 1)
		{
			Dem_Stop();
			return 0;
		}
	} else {
		demotime = from.lasttime;
		newtime = oldnewtime;
		from.lasttime = 0;
	}

// decide if it is time to grab the next message
	if (from.lastframe < 0)
		from.lastframe = demotime;
	else if (demotime > from.lastframe) {
		from.lastframe = demotime;
		from.lasttime = demotime;
		oldnewtime = newtime;
		return 0; // already read this frame's message
	}

	if (demotime - realtime > 0.0001 && from.format == mvd)
	{
		olddemotime = realtime;

		from.netchan.incoming_sequence++;
		from.netchan.incoming_acknowledged++;
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "sequence:%d\n", from.netchan.incoming_sequence);
	}

	from.time = realtime = demotime; // warp
	from.prevtime = demotime;

	if (sworld.options & O_DEBUG)
		fprintf(sworld.debug.file, "msec:%d\n", newtime);

	// get the msg type
	fread (&c, sizeof(c), 1, sworld.from.file);

	switch (c&7) {
	case dem_cmd :
		// user sent input
		i = from.netchan.outgoing_sequence & UPDATE_MASK;
		pcmd = &from.frames[i].cmd;
		r = fread (pcmd, sizeof(*pcmd), 1, sworld.from.file);
		if (r != 1)
		{
			Dem_Stop();
			return 0;
		}

		// byte order stuff
		for (j = 0; j < 3; j++) {
			pcmd->angles[j] = LittleFloat(pcmd->angles[j]);
			if (pcmd->angles[j] > 180)
				pcmd->angles[j] -= 360;
		}

		pcmd->forwardmove = LittleShort(pcmd->forwardmove);
		pcmd->sidemove    = LittleShort(pcmd->sidemove);
		pcmd->upmove      = LittleShort(pcmd->upmove);
		from.frames[i].senttime = demotime;
		from.frames[i].receivedtime = -1;		// we haven't gotten a reply yet
		from.netchan.outgoing_sequence++;
		from.frames[i].playerstate[from.playernum].command = *pcmd;
		for (j=0 ; j<3 ; j++)
		{
			r = fread (&f, 4, 1, sworld.from.file);
			from.frames[from.netchan.incoming_sequence&UPDATE_MASK].playerstate[from.playernum].command.angles[j] = LittleFloat (f);
		}

		break;

	case dem_read:
readit:
		// get the next message
		fread (&net_message.cursize, 4, 1, sworld.from.file);
		net_message.cursize = LittleLong (net_message.cursize);
		
		if (net_message.cursize > MAX_UDP_PACKET) {
			Sys_Printf ("ERROR: Demo message > MAX_UDP_PACKET (%d)\n", net_message.cursize);
			Dem_Stop();
			return 0;
		}
		r = fread (net_message.data, net_message.cursize, 1, sworld.from.file);
		if (r != 1)
		{
			Dem_Stop();
			return 0;
		}

		break;

	case dem_set :
		if (sworld.options & O_DEBUG)
			fprintf(sworld.debug.file, "dem_set\n");
		fread (&i, 4, 1, sworld.from.file);
		from.netchan.outgoing_sequence = LittleLong(i);
		fread (&i, 4, 1, sworld.from.file);
		from.netchan.incoming_sequence = LittleLong(i);
		if (from.format == mvd) {
			from.netchan.incoming_acknowledged = from.netchan.incoming_sequence;
			goto nextdemomessage;
		}

		break;

	case dem_multiple:
		r = fread (&i, 4, 1, sworld.from.file);
		if (r != 1)
		{
			Dem_Stop();
			return 0;
		}

		from.to = LittleLong(i);
		from.type = dem_multiple;
		goto readit;
		
	case dem_single:
		from.to = c>>3;
		from.type = dem_single;
		goto readit;
	case dem_stats:
		from.to = c>>3;
		from.type = dem_stats;
		goto readit;
	case dem_all:
		from.to = 0;
		from.type = dem_all;
		goto readit;
	default :
		Dem_Stop();
		return 0;
	}

	return 1;
}

int To(void);
void CheckSpectator (void)
{
	player_state_t	*state, *self;
	int i,j;

	if (!from.spectator)
		return;

	self = &from.frames[from.netchan.incoming_sequence&UPDATE_MASK].playerstate[from.playernum];
	for (i = 0, state = from.frames[from.netchan.incoming_sequence&UPDATE_MASK].playerstate; i < MAX_CLIENTS; i++, state++)
	{
		if (state->messagenum != from.parsecount)
			continue;

		if (i == from.playernum)
			continue;

		for (j = 0; j < 3; j++)
			if (abs(state->command.angles[j] - self->command.angles[j]) > 0.0) {
				break;
			}

		if (j != 3)
			continue;
		break;
	}

	if (i == MAX_CLIENTS) {
		from.spec_track = -1;
	} else 			
		from.spec_track = i;

	if (stats_msg.cursize) {
		DemoWrite_Begin(dem_stats, To(), stats_msg.cursize);
		SZ_Write(msgbuf, stats_msg.data, stats_msg.cursize);
		SZ_Clear(&stats_msg);
	}
}

void ReadPackets (void)
{
	qboolean change = false;
	static float old;

	while (GetDemoMessage())
	{
		//
		// remote command packet
		//
		if (*(int *)net_message.data == -1)
		{
			ConnectionlessPacket ();

			if (!from.running) {
				return;
			}
			continue;
		}

		if (net_message.cursize < 8 && from.format != mvd)
		{
			continue;
		}

		
		if (from.format == qwd) {
			if (!Netchan_Process(&from.netchan))
				continue;		// wasn't accepted for some reason
		} else {
			MSG_BeginReading ();
		}
			
		Dem_ParseDemoMessage ();
	}

	CheckSpectator();

	if (sworld.from.file != stdin && ftell(sworld.from.file) - world.oldftell > 256000) {
		int p;

		if ((p = (int)100.0*ftell(sworld.from.file)/sworld.from.filesize) != world.percentage)
			SetConsoleTitle(va("qwdtools  %d%% Done...", p));
		world.percentage = p;
		world.oldftell = ftell(sworld.from.file);
	}
	
}

void MainLoop(void)
{
	frame_t	*fframe, *tframe;
	int		oldvalid = 0;
	int		oldparse = 0;
	float	acc_time = 0;
	int		acc_count = 0;
	int		acc_count_o = 0;
	
	if (!(sworld.options & O_STDIN))
		SetConsoleTitle("qwdtools  0% Done...");

	if (sworld.options & O_CONVERT) // say hi
		QWDToolsMsg();

	while (from.running)
	{
		if  (sworld.options & O_SHUTDOWN)
			Sys_Exit(2);

		// read packet from demo
		ReadPackets();

		acc_count_o++;

		// if not converting, stop here
		if (!(sworld.options & O_CONVERT)) {
			SZ_Clear(msgbuf);
			continue;
		}

		// check if singon data is loaded
		if (world.validsequence != oldvalid)
		{
			oldvalid = world.validsequence;
			DemoWriteToDisk(0,0, demo.time);
			world.signonloaded = true;
			acc_time = world.time;
			msgmax = MAX_MSGLEN;
			if (sworld.options & O_DEBUG)
				fprintf(sworld.debug.file, "first packet, real:%f, demo:%f\n", realtime, demo.time);
		}

		// check if it's time for next frame
		if (world.time - demo.time >= 1.0/sworld.fps || !from.running)
		{
			if (!world.validsequence)
				continue;

			// Add packet entities, nails, and players
			tframe = &world.frames[world.parsecount&UPDATE_MASK];
			if (from.running) {
				if (oldparse == from.parsecount) {
					//Sys_Printf("%d\n", from.parsecount);
					continue;
				}
				oldparse = from.parsecount;
				fframe = &from.frames[from.parsecount&UPDATE_MASK];

				memcpy(&tframe->packet_entities, &fframe->packet_entities, sizeof(packet_entities_t));
				memcpy(tframe->playerstate, fframe->playerstate, sizeof(fframe->playerstate));
				memcpy(tframe->projectiles, fframe->projectiles, sizeof(fframe->projectiles));
				tframe->num_projectiles = fframe->num_projectiles;
				tframe->parsecount = from.parsecount;
				tframe->senttime = fframe->senttime;
			} else tframe->invalid = true;

			acc_count++;

			// decide next frame time
			if (world.time - demo.time > 3.0 / sworld.fps) {
				demo.time = world.time;
			} else
				demo.time += 1.0 / sworld.fps;

			tframe->time = world.time;
			world.parsecount++;

			if (world.parsecount - world.lastwritten > 60)
				WritePackets(50);

			msgbuf = &world.frames[world.parsecount&UPDATE_MASK].buf;
			msgbuf->data = demo.buffer + demo.bufsize;
			msgbuf->maxsize = sizeof(demo.buffer) - demo.bufsize;

			msgbuf->size = 0;
			msgbuf->bufsize = 0;
			msgbuf->curto = 0;
			msgbuf->curtype = 0;
			msgbuf->cursize = 0;

			if (!from.running)
				WritePackets(world.parsecount - world.lastwritten);
		}
	}

	if (sworld.options & O_CONVERT)
	{
		if (acc_count)
			Sys_Printf(" average demo fps:%.1f (originally %.1f)\n", acc_count/(world.time - acc_time), acc_count_o/(world.time - acc_time));
	}
}

void QWDToolsMsg(void)
{
	char str[1024];

	strcpy(str, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n"
		        " This demo was converted\n"
				" from QWD by QWDTools\n"
				"\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n");

	DemoWrite_Begin(dem_all, 0, strlen(str) + 3);
	MSG_WriteByte(msgbuf, svc_print);
	MSG_WriteByte(msgbuf, PRINT_CHAT);
	MSG_WriteString(msgbuf, str);
}

qboolean ClearWorld(void)
{
	char *ext;
	extern char stdintype[32];

	memset(&from, 0, sizeof(from));
	memset(&world, 0, sizeof(world));
	memset(&demo, 0, sizeof(demo));

	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	// for converting spectator demos
	stats_msg.maxsize = sizeof(stats_buf);
	stats_msg.data = stats_buf;

	msgmax = MAX_MSGLEN/2; // signon has shortened msg length
	msgbuf = &world.frames[0].buf;
	msgbuf->data = demo.buffer;
	msgbuf->maxsize = sizeof(demo.buffer);

	realtime = 0;
	from.running = true;
	from.lastframe = -1;

	if (sworld.options & O_STDIN) {
		strcpy(sworld.from.name, "stdin");
		ext = va(".%s", stdintype);
	} else
		ext = FileExtension(sworld.from.name);

	// qwz -> qwd
	if (!_stricmp(ext, ".qwz")) {
		*(sworld.from.name + strlen(sworld.from.name) - 1) = 'd';
		ext[3] = 'd';
		from.qwz = true;
	}

	if (!_stricmp(ext, ".qwd"))
		from.format = qwd;
	else if (!_stricmp(ext, ".mvd"))
		from.format = mvd;
	else
	{
		Sys_Printf("Ignoring %s\n", sworld.from.name);
		return false;
	}

	return true;
}

char *getPath(char *path)
{
	static char dir[MAX_OSPATH];
	char *p;

	strcpy(dir, path);
	p = dir + strlen(dir);
	while (p > dir) {
		if (*p == '\\' || *p == '/') {
			p++;
			break;
		}
		p--;
	}
	*p = 0;

	return dir;
}

/*
==================
main
==================
*/

char *sourcePath;

int main (int argc, char **argv)
{
	int			i, options;
	struct		_finddata_t c_file;
    long		hFile;

	InitArgv(argc, argv);

	CtrlH_Init();

	World_Init();
	Load_ini();
	Tools_Init();

	ParseArgv();

	options = sworld.options & JOB_TODO;

	if (sworld.options & O_FC)
		Sys_Printf("   -filter_chats\n");
	else {
		if (sworld.options & O_FS)
			Sys_Printf("   -filter_spectalk\n");
		if (sworld.options & O_FS)
			Sys_Printf("   -filter_qizmotalk\n");
		if (sworld.options & O_FT)
			Sys_Printf("   -filter_teamchats\n");
	}

	Sys_Printf("   -fps %d\n", sworld.fps);
	Sys_Printf("   -msglevel %d\n", sworld.msglevel);
	
	i = 0;
	// serve all source files in loop
	for (i = 0; sourceName[i][0]; i++) {

		sourcePath = getPath(sourceName[i]);
		if (!(sworld.options & O_STDIN)) {
			if ((hFile = _findfirst( sourceName[i], &c_file )) == -1L)
			{
				Sys_Printf("Couldn't find file(s) %s\n", sourceName[i]);
				continue;
			}
		}

		do {
			if  (sworld.options & O_SHUTDOWN)
				Sys_Exit(2);

			if (c_file.attrib & _A_SUBDIR || c_file.attrib & _A_SYSTEM)
				continue;

			strcpy(sworld.from.name, c_file.name);

			if (!ClearWorld())
				continue;

			sworld.options &= ~(JOB_TODO);
			sworld.options |=  Files_Init(options);

			if (!(sworld.options & JOB_TODO))
				continue;

			// run program
			MainLoop();

			Sys_fclose(sworld.demo.file);
			Sys_fclose(sworld.from.file);
			Sys_fclose(sworld.debug.file);
			Sys_fclose(sworld.log.file);
			Sys_fclose(sworld.analyse.file);

			if (sworld.options & O_STDIN)
				Sys_Exit(1);

		} while(_findnext( hFile, &c_file ) == 0 );

		_findclose( hFile );

	}

	Sys_Printf("\nDone...\n\n");

	Sys_Exit(0);
	return false; // to happy compiler
}
