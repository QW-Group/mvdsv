#include "defs.h"
#define DEFAULT_FPS 20

int memsize, membase;
int	fps;

#define	MAX_UDP_PACKET	(MAX_MSGLEN*2)	// one more than msg + header
sizebuf_t	net_message;
byte		net_message_buffer[MAX_UDP_PACKET];

demo_t		demo;
source_t	from;

world_state_t	world;
lightstyle_t	lightstyle[MAX_LIGHTSTYLES];
entity_state_t	baselines[MAX_EDICTS];
float			realtime;
sizebuf_t		stats_msg;
byte			stats_buf[MAX_MSGLEN];

qboolean		debug;
qboolean		filter_spectalk;
qboolean		filter_qizmotalk;

/*
================
Sys_Error
================
*/

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr,error);
	vsprintf (text, error,argptr);
	va_end (argptr);

	if (from.file != NULL) {
		printf ("ERROR: %s, at:%d\n", text, ftell(from.file));
	} else {
		printf ("ERROR: %s\n", text);
	}

	if (demo.file != NULL)
		fclose(demo.file);
	if (from.file != NULL)
		fclose(from.file);

	exit (1);
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
		
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
		if (debug)
			Sys_Printf("connected\n");

		from.netchan.incoming_sequence = 0;
		from.netchan.incoming_acknowledged= 0;
		from.netchan.incoming_acknowledged = 0;
		from.netchan.incoming_reliable_acknowledged = 0;
		return;
	}

	// remote command from gui front end
	if (c == A2C_CLIENT_COMMAND)
	{
		if (debug)
			Sys_Printf("A2C\n");
		MSG_ReadString ();
		MSG_ReadString ();

		return;
	}

	// print command from somewhere
	if (c == A2C_PRINT)
	{
		if (debug)
			Sys_Printf("print\n");
		MSG_ReadString ();
		return;
	}

	// ping from somewhere
	if (c == A2A_PING)
	{
		if (debug)
			Sys_Printf("ping\n");
		return;
	}

	if (c == S2C_CHALLENGE) {
		if (debug)
			Sys_Printf("challenge\n");
		MSG_ReadString ();
		return;
	}

	if (c == svc_disconnect) {
		if (debug)
			Sys_Printf("disconnect\n");
		Dem_Stop();
		return;
	}
}

void Dem_Stop(void)
{
	from.running = false;
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
	static float prevtime = 0.0;

	if (prevtime < realtime)
		prevtime = realtime;

nextdemomessage:

	// read the time from the packet
	newtime = 0;
	if (from.format == mvd)
	{
		r = fread(&newtime, sizeof(newtime), 1, from.file);
		demotime =  prevtime + newtime*0.001;
	} else {
		r = fread(&demotime, sizeof(demotime), 1, from.file);
		demotime = LittleFloat(demotime);
	}

	if (r != 1)
	{
		Dem_Stop();
		return 0;
	}

// decide if it is time to grab the next message
	if (from.lastframe < 0)
		from.lastframe = demotime;
	else if (demotime > from.lastframe) {
		from.lastframe = demotime;
		// rewind back to time
		if (from.format == mvd)
		{
			fseek(from.file, ftell(from.file) - sizeof(newtime),
				SEEK_SET);
		} else 
			fseek(from.file, ftell(from.file) - sizeof(demotime),
				SEEK_SET);
		return 0;		// already read this frame's message
	}

	if (demotime - realtime > 0.0001 && from.format == mvd)
	{
		olddemotime = realtime;

		from.netchan.incoming_sequence++;
		from.netchan.incoming_acknowledged++;
		if (debug)
			Sys_Printf("sequence:%d\n", from.netchan.incoming_sequence);
	}

	from.time = realtime = demotime; // warp
	prevtime = demotime;

	if (debug)
		Sys_Printf("msec:%d\n", newtime);

	// get the msg type
	fread (&c, sizeof(c), 1, from.file);

	switch (c&7) {
	case dem_cmd :
		// user sent input
		i = from.netchan.outgoing_sequence & UPDATE_MASK;
		pcmd = &from.frames[i].cmd;
		r = fread (pcmd, sizeof(*pcmd), 1, from.file);
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
			r = fread (&f, 4, 1, from.file);
			from.frames[from.netchan.incoming_sequence&UPDATE_MASK].playerstate[from.playernum].command.angles[j] = LittleFloat (f);
		}

		break;

	case dem_read:
readit:
		// get the next message
		fread (&net_message.cursize, 4, 1, from.file);
		net_message.cursize = LittleLong (net_message.cursize);
		
		if (net_message.cursize > MAX_MSGLEN) {
			Sys_Printf ("ERROR: Demo message > MAX_MSGLEN (%d)", net_message.cursize);
			Dem_Stop();
			return 0;
		}
		r = fread (net_message.data, net_message.cursize, 1, from.file);
		if (r != 1)
		{
			Dem_Stop();
			return 0;
		}

		break;

	case dem_set :
		if (debug)
			Sys_Printf("dem_set\n");
		fread (&i, 4, 1, from.file);
		from.netchan.outgoing_sequence = LittleLong(i);
		fread (&i, 4, 1, from.file);
		from.netchan.incoming_sequence = LittleLong(i);
		if (from.format == mvd) {
			from.netchan.incoming_acknowledged = from.netchan.incoming_sequence;
			goto nextdemomessage;
		}

		break;

	case dem_multiple:
		r = fread (&i, 4, 1, from.file);
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
			//FIXME: write connectionless msg to demo ?
			//DemoReliableWrite_Begin(from.type, from.to, msg_readcount);
			//MSG_Forward(&demo.buf, 0, msg_readcount);
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
}

extern int com_argc;
extern char *com_argv[MAX_NUM_ARGVS];

void ParseArgv(void)
{
	int	i;
	char *ext;

	// parse demoname
	for (i = 1; i < com_argc; i++)
	{
		if (*com_argv[i] == '-') {
			if (!strcmp(com_argv[i], "-debug"))
				continue;
			if (!strcmp(com_argv[i], "-filter_spectalk"))
				continue;
			if (!strcmp(com_argv[i], "-filter_qizmotalk"))
				continue;
			if (!strcmp(com_argv[i], "-fq"))
				continue;
			if (!strcmp(com_argv[i], "-fs"))
				continue;

			i++;
			continue;
		}

		strcpy(from.name, com_argv[i]);
		break;
	}

	if (i >= com_argc)
	{
		Sys_Printf("usage: qwdtools.exe <demoname> [-o <outputname> -fps # -filter_spectalk -filter_qizmotalk]\n");
		exit(1);
	}

	DefaultExtension(from.name, ".qwd");

	// parse output name
	if ((i = CheckParm("-o")) != 0 && i + 1 < com_argc)
	{
		StripExtension(com_argv[i+1], demo.name);
	} else {
		// use the same name
		StripExtension(from.name, demo.name);
	}

	ForceExtension(demo.name, ".mvd");

	// check demo format
	ext = from.name + strlen(from.name);
	while (*ext != '.')
		ext--;

	if (!strcmp(ext, ".qwd"))
		from.format = qwd;
	else if (!strcmp(ext, ".mvd"))
		from.format = mvd;
	else 
		Sys_Error("Unknown demo format: %s\n", ext);

	// parse converter settings setting
	if ((i = CheckParm("-fps")) != 0 && i + 1 < com_argc)
	{
		fps = atoi(com_argv[i+1]);
		if (fps < 4) // 1000/4 -> 250ms
			fps = 4;
		if (fps > 100)
			fps = 100;
	} else
		fps = DEFAULT_FPS;

	debug = CheckParm("-debug");
	filter_spectalk = CheckParm("-filter_spectalk") || CheckParm("-fs") ;
	filter_qizmotalk = CheckParm("-filter_qizmotalk") || CheckParm("-fq");
}

void QWDToolsMsg(void)
{
	char str[1024];

	strcpy(str, "\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n\n"
		        " This demo was converted\n"
				" from QWD by QWDTools\n\n"
				"\x1d\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1e\x1f\n");

	DemoWrite_Begin(dem_all, 0, strlen(str) + 3);
	MSG_WriteByte(msgbuf, svc_print);
	MSG_WriteByte(msgbuf, PRINT_CHAT);
	MSG_WriteString(msgbuf, str);
}

/*
==================
main
==================
*/

int main (int argc, char **argv)
{
	int			oldvalid, i;
	extern int	msgmax;
	frame_t	*frame, *fframe, *tframe;
	static oldparse = 0;

	Tools_Init();

	InitArgv (argc, argv);
	ParseArgv ();
	
	// file stuff
	if (!strcmp(demo.name, from.name))
	{
		Sys_Error("Can't convert from %s to %s!\n", from.name, demo.name);
	}
	
	if (FileOpenRead(from.name, &from.file) == -1)
	{
		Sys_Error("couldn't open for reading: %s\n", from.name);
	}

	demo.file = fopen (demo.name, "wb");
	if (demo.file == NULL)
	{
		Sys_Error ("couldn't open for writing: %s.\n", demo.name);
	}

	if (fps != DEFAULT_FPS)
		Sys_Printf("converting at %d fps\n", fps);

	// init values

	memset(&world, 0, sizeof(world));

	realtime = 0;
	from.running = true;
	from.lastframe = -1;
	olddemotime = 0;

	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	demo.buf.maxsize = sizeof(demo.buf_data);
	demo.buf.data = demo.buf_data;
	demo.datagram.maxsize = sizeof(demo.datagram_data);
	demo.datagram.data = demo.datagram_data;

	// for converting spectator demos
	stats_msg.maxsize = sizeof(stats_buf);
	stats_msg.data = stats_buf;


	for (i=0, frame = world.frames; i < UPDATE_BACKUP; i++, frame++)
	{
		frame->buf.data = frame->buf_data;
		frame->buf.maxsize = sizeof(frame->buf_data);
		frame->buf.cursize = 0;
		frame->buf.bufsize = 0;
		frame->buf.size = (int*)frame->buf_data;
	}

	oldvalid = 0;
	msgmax = MAX_MSGLEN/2;
	msgbuf = &world.frames[0].buf;

	QWDToolsMsg();

	// ok, we can start converting demo
	Sys_Printf("%s -> %s\n", from.name, demo.name);
	while (from.running)
	{
		// read packet from demo
		ReadPackets();

		// check if singon data is loaded
		//Sys_Printf("%f\n", realtime - world.time);
		if (world.validsequence != oldvalid)
		{
			oldvalid = world.validsequence;
			DemoWriteToDisk(0,0, demo.time);
			world.signonloaded = true;
			msgmax = MAX_MSGLEN;
			if (debug)
				Sys_Printf("first packet, real:%f, demo:%f\n", realtime, demo.time);
		}

		// check if it's time for next frame
		if (world.time - demo.time >= 1.0/fps || !from.running)
		{
			if (!world.validsequence)
				continue;

			// Add packet entities, nails, and player
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

			tframe->time = demo.time = world.time;
			world.parsecount++;
			
			if (!from.running)
				WritePackets(world.parsecount - world.lastwritten);
			else if (world.parsecount - world.lastwritten > 60)
				WritePackets(50);

			msgbuf = &world.frames[world.parsecount&UPDATE_MASK].buf;
		}
	}

	if (demo.file != NULL)
		fclose(demo.file);
	if (from.file != NULL)
		fclose(from.file);

	Sys_Printf("succesfully converted\n");
	return true;
}
