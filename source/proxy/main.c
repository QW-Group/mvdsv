/*
	main.c
	Original Author:	Chris Faherty <chrisf@america.com>
	Adopted by:			Ivan Bol'sunov aka qqshka
*/

#include "qwfwd.h"

#ifndef _WIN32
#define DWORD unsigned int
#define WINAPI
#endif

cvar_t *developer;
cvar_t *version;
cvar_t *hostname;
cvar_t *maxclients;
cvar_t *sys_readstdin;

proxy_static_t ps;

// { FIXME: HACK
static int argc;
static char **argv;
// }

/*
===========
SV_Serverinfo_f

Examine or change the serverinfo string
===========
*/
static void SV_Serverinfo_f (void)
{
	cvar_t	*var;
	char	*s;
	char	*key, *value;

	if (Cmd_Argc() == 1)
	{
		Sys_Printf ("Server info settings:\n");
		Info_Print (ps.info);
		Sys_Printf ("[%d/%d]\n", strlen(ps.info), sizeof(ps.info));
		return;
	}

	if (Cmd_Argc() == 2)
	{
		char buf[MAX_INFO_KEY];

		s = Info_ValueForKey(ps.info, Cmd_Argv(1), buf, sizeof(buf));
		if (*s)
			Sys_Printf ("Serverinfo %s: \"%s\"\n", Cmd_Argv(1), s);
		else
			Sys_Printf ("No such key %s\n", Cmd_Argv(1));
		return;
	}

	if (Cmd_Argc() != 3)
	{
		Sys_Printf ("Usage: serverinfo [ <key> [ <value> ] ]\n");
		return;
	}

	key = Cmd_Argv(1);
	value = Cmd_Argv(2);

	if (key[0] == '*')
	{
		Sys_Printf ("Star variables cannot be changed.\n");
		return;
	}

	// if the key is also a serverinfo cvar, change it too
	var = Cvar_Find(key);
	if (var && (var->flags & CVAR_SERVERINFO))
	{
		Cvar_Set (var->name, value);
	}
	else
	{
		Info_SetValueForKey (ps.info, key, value, sizeof(ps.info));	
	}
}

static void Cmd_Quit_f(void)
{
	// if at least some parameter provided, then use non clean exit
	if (Cmd_Argc() > 1)
		Sys_Exit(0); // immidiate, non clean
	else
		ps.wanttoexit = true; // delayed exit, clean
}

DWORD WINAPI FWD_proc(void *lpParameter)
{
	if (!lpParameter)
		return 1;

	memset(&ps, 0, sizeof(ps));
	ps.params = *(fwd_params_t*)lpParameter;

	// init basic systems
	Cbuf_Init();			// Command buffer init.
	Cmd_Init();				// Register basic commands.
	Cvar_Init();			// Variable system init.

	// register basic cvars
	developer		= Cvar_Get("developer",		"0", 0);
	version			= Cvar_Get("*version",		QWFWD_VERSION, CVAR_READONLY | CVAR_SERVERINFO);
	hostname		= Cvar_Get("hostname",		"unnamed qwfwd", CVAR_SERVERINFO);
	maxclients		= Cvar_Get("maxclients",	"128", CVAR_SERVERINFO);
	sys_readstdin	= Cvar_Get("sys_readstdin",	"1", 0);

	// register basic commands
	Cmd_AddCommand("quit", Cmd_Quit_f);
	Cmd_AddCommand("serverinfo", SV_Serverinfo_f);

	// now exec our cfg
	Cbuf_InsertText ("exec qwfwd.cfg\n");
	Cbuf_Execute();

	// init rest systems
	Sys_DoubleTime();		// init time
	NET_Init();				// init network
	FWD_Init();				// init peers
	QRY_Init();				// init query 

	ps.initialized = true;

	// Process command line arguments.
	Cmd_StuffCmds(argc, argv);
	Cbuf_Execute();

	Sys_Printf("qwfwd: ready to rock at %s:%d\n", net_ip->string, net_port->integer);

	while(!ps.wanttoexit)
	{
		Cbuf_Execute();			// Process console commands.
		FWD_update_peers();		// do basic proxy job
		QRY_Frame();			// do query related job
	}

	Cmd_DeInit();		// this is optional, but helps me check memory leaks
	Cvar_DeInit();		// this is optional, but helps me check memory leaks

	return 0;
}

int main(int _argc, char *_argv[])
{
	fwd_params_t params;

	argc = _argc;
	argv = _argv;

	#ifdef _CRTDBG_MAP_ALLOC
	{
		// report memory leaks on program exit in output window under MSVC
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	}
	#endif

	#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	#endif

	srand((unsigned) time (NULL));

	// show help ffs
	if (argc > 1 && (	   !stricmp("-h", argv[1])
						|| !stricmp("-?", argv[1])
						|| !stricmp("/?", argv[1])
						|| !stricmp("/h", argv[1])
						|| !stricmp("/help", argv[1])
						|| !stricmp("-help", argv[1])
						|| !stricmp("--help", argv[1])
					)
		)
	{
		Sys_Printf("Usage: %s [port [ip]]\n", argv[0]);
		return 1;
	}

	memset(&params, 0, sizeof(params));
	params.port = (argc > 1) ? atoi(argv[1]) : 0;
	strlcpy(params.ip, (argc > 2 && argv[2][0] != '-' && argv[2][0] != '+')  ? argv[2] : "", sizeof(params.ip));

	FWD_proc(&params);
	return 0;
}
