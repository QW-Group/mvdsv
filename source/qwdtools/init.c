#include "defs.h"
#include "version.h"

#define DEFAULT_FPS 20
#define DEBUG_FILE	"debug.txt"
#define LOG_FILE	"log.txt"

extern int com_argc;
extern char *com_argv[MAX_NUM_ARGVS];


void Help (void)
{
	Sys_Printf("usage: qwdtools.exe [-options] demoname1 demoname2 ...\n"
			   "example:\n"
			   "qwdtools.exe -c -o * -od out mydemo.qwd\nwill convert demo 'mydemo.qwd' to 'out/mydemo.mvd'\n\n");

	Sys_Printf("available options: (argument in [] is optional)\n");
	Sys_Printf("-bd path               changes current directory\n");
	Sys_Printf("-qd path               path to qizmo dir, e/g: -qd ../qizmo\n");
	Sys_Printf("-od path               output directory path\n");
	Sys_Printf("-o name                output file name (default: *)\n");
	Sys_Printf("-debug_file name       debug file name (default: debug.txt)\n");
	Sys_Printf("-log_file name         log file name (default: log.txt)\n");
	Sys_Printf("-fps num               demo fps (default: 20)\n");
	Sys_Printf("-fs                    filters spectator chats\n");
	Sys_Printf("-fq                    filters qizmo observers chats\n");
	Sys_Printf("-fc                    filters all chats\n");
	Sys_Printf("-ft                    filters team chats\n");
	Sys_Printf("-msg                   msg level (same as QW's msg command)\n");
	Sys_Printf("-c                     converts to mvd (assumed if no options are given)\n");
	//Sys_Printf("-analyse/-a            - analysing demo\n");
	Sys_Printf("-log                   creates log file\n");
	Sys_Printf("-debug                 prints way too much messages\n");
	Sys_Printf("-wait                  \"Press any key to continue\"\n");
	Sys_Printf("-noini                 ini file won't be executed\n");
	Sys_Printf("-ini path              name of ini file (default: qwdtools.ini)\n");
	Sys_Printf("-stdin [format]        file from stdin  (default format: qwd)\n");
	Sys_Printf("-stdout                result to stdout (first found option: -c/-log/-debug)\n");
	Sys_Printf("\n" VERSION " (c) 2001 Bartlomiej Rychtarski (highlander@gracz.net)\nhttp://qwex.n3.net/\n");
}

BOOL WINAPI CtrlC( DWORD dwCtrlType )
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		if (sworld.options |= O_SHUTDOWN)
			exit(3);

		Sys_Printf("Shutting down...\n");
		if (sworld.options & O_WAITFORKBHIT)
			SetConsoleTitle("qwdtools  ctrl-c issued");

		sworld.options |= O_SHUTDOWN;
		return true;
	case CTRL_BREAK_EVENT:
		if (sworld.options |= O_SHUTDOWN)
			exit(3);

		Sys_Printf("Shutting down...\n");
		if (sworld.options & O_WAITFORKBHIT)
			SetConsoleTitle("qwdtools  ctrl-break issued");

		sworld.options |= O_SHUTDOWN;
		return true;
	}

	return false;
}

void CtrlH_Init (void)
{
	SetConsoleCtrlHandler(&CtrlC, true);
}
/*
===================
World_Init

Initial values
===================
*/
extern float olddemotime;
void World_Init(void)
{
	int i;

	memset(sourceName, 0, sizeof(sourceName));

	strcpy(qizmoDir, "");
	strcpy(outputDir, "");

	sworld.fps = DEFAULT_FPS;
	strcpy (sworld.debug.name, DEBUG_FILE);
	strcpy (sworld.log.name, LOG_FILE);
	strcpy (sworld.demo.name, "*");
	strcpy (sworld.analyse.name, "*.txt");

	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	if ((i = CheckParm("-bd")) == 0)
		i = CheckParm("-base_dir");

	if (i && i + 1 < com_argc) {
		SetCurrentDirectory(com_argv[i+1]);
	}

	GetCurrentDirectory(sizeof(currentDir), currentDir);

	//Sys_Printf("arg:%s\ncur:%s\n", com_argv[0], currentDir);

	ConsoleInHndl = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,NULL);
	ConsoleOutHndl = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,NULL);

	if (CheckParm("-wait") || CheckParm("-waitforkbhit"))
		sworld.options |= O_WAITFORKBHIT;

	if (CheckParm("-stdout"))
		sworld.options |= O_STDOUT;
}

/*
===================
ParseArgv

Parse cmdline options
===================
*/

char stdintype[32] = "";
char dummy[64];
int  whatToStdout = 0;

#pragma warning( disable : 4133 4047)
param_t params[] =
{
	{"-qizmo_dir", "-qd", TYPE_S, qizmoDir},
	{"-output_dir", "-od", TYPE_S, outputDir},
	{"-output", "-o", TYPE_S, sworld.demo.name},
	{"-debug_file", NULL, TYPE_S, sworld.debug.name},
	{"-log_file", NULL, TYPE_S, sworld.log.name},
	{"-fps", NULL, TYPE_I, &sworld.fps},
	{"-filter_spectalk", "-fs", TYPE_O, O_FS},
	{"-filter_qizmotalk", "-fq", TYPE_O,O_FQ},
	{"-filter_chats", "-fc", TYPE_O, O_FC},
	{"-filter_teamchats", "-ft", TYPE_O, O_FT},
	{"-convert", "-c", TYPE_O, O_CONVERT},
	{"-analyse", "-a", TYPE_O, O_ANALYSE},
	{"-log", NULL, TYPE_O, O_LOG},
	{"-debug", NULL, TYPE_O, O_DEBUG},
	{"-waitforkbhit", "-wait", TYPE_O, O_WAITFORKBHIT},
	{"-stdin",NULL, TYPE_O | TYPE_S, stdintype, O_STDIN},
	{"-stdout", NULL, TYPE_O, O_STDOUT},
	{"-noini"},
	{"-base_dir", "-bd", TYPE_S, currentDir},
	{"-msglevel", "-msg", TYPE_I, &sworld.msglevel},
	NULL
};
#pragma warning( default : 4133 4047)

param_t *FindParam(char *parm)
{
	param_t *param;

	for (param = params; param->name; param++)
		if (!_stricmp(parm, param->name) || (param->shname && !_stricmp(parm, param->shname)))
			return param;

	return NULL;
}

void ParseArgv(void)
{
	int		i, o;
	param_t	*param;
	char	qwz_files[1024] = "", *ext, *arg;

	if (CheckParm("-help"))
	{
		Help();
		Sys_Exit(1);
	}

	// get qizmo dir before args are parsed
	if ((i = CheckParm("-qd")) == 0)
		i = CheckParm("-qizmo_dir");

	if (i && i + 1 < com_argc)
		strcpy(qizmoDir, com_argv[i+1]);

	// parse args
	for (i = 1; i < com_argc; i++)
	{
		arg = com_argv[i];
		// if starts from -- ignore or scream that unknown
		if (arg[0] == '-' && arg[1] == '-')
			arg++;

		if ( (param = FindParam(arg)) == NULL)
		{
			// if starts from '-' it's an unrecognized option
			if (arg[0] == '-')
			{
				Sys_Printf("unrecognized option: %s\n", arg);
				Help();
				Sys_Exit(1);
			}

			// it's a file name
			strcpy(sourceName[sworld.sources], arg);
			DefaultExtension(sourceName[sworld.sources], ".qwd");
			// check for qwz extension
			ext = FileExtension (arg);
			if (!_stricmp(ext, ".qwz")) {
				if (!strcmp(qizmoDir, ".") || !strcmp(qizmoDir, "./") || !strcmp(qizmoDir, ".\\"))
					qizmoDir[0] = 0;

				if (strstr(arg, ":") || !qizmoDir[0])
					strcat(qwz_files, va("\"%s\" ", arg));
				else
					strcat(qwz_files, va("\"%s\\%s\" ", currentDir, arg));
			}
			sworld.sources++;

			continue;
		}

		// if started from -- ignore
		if (arg != com_argv[i])
			continue;

		// run option
		if (param->type & TYPE_S) {
			i++;
			if (i < com_argc && com_argv[i][0] != '-') {
				strcpy(param->str, com_argv[i]);
			} else {
				// if it's stdin assume qwd
				if (param->opt2 == O_STDIN)
					strcpy(param->str, "qwd");
				else {
					Sys_Printf("init: missing argument for %s\n", arg);
					Help();
					Sys_Exit(1);
				}
			}

			if (param->str = currentDir) {
				SetCurrentDirectory(currentDir);
				GetCurrentDirectory(sizeof(currentDir), currentDir);
			}
		}

		if (param->type & TYPE_I) {
			i++;
			if (i < com_argc && com_argv[i][0] != '-')
			{
				*param->Int = atoi(com_argv[i]);
			} else {
				Sys_Printf("init: missing argument for %s\n", arg);
				Help();
				Sys_Exit(1);
			}
		}

		if (param->type & TYPE_O) {
			if (param->opt2)
				o = param->opt2;
			else
				o = param->opt;
			if (!whatToStdout && (o & JOB_TODO))
				whatToStdout = o;
			sworld.options |= o;
		}
	}
	
	if (!(sworld.options & JOB_TODO)) {
		sworld.options |= O_CONVERT;
		whatToStdout = O_CONVERT;
	}

	// change translation mode for stdin
	if (sworld.options & O_STDIN)
	{	
		sworld.sources = 1;
		sworld.from.file = stdin;
		strcpy(sourceName[0], "stdin");
		DefaultExtension(sworld.from.name, va(".%s", stdintype));
		if (_setmode (_fileno(sworld.from.file), _O_BINARY) == -1)
			Sys_Error("failed to set binary mode for stdin");
	} else if (qwz_files[0]) {
		// decompress qwz files
		if (!OpenQWZ(qwz_files))
			Sys_Printf("Couldn't decompress qwz file(s)\n");
	}

	if (!sworld.sources)
	{
		Sys_Printf("demo name not specified\n");
		Help();
		Sys_Exit(1);
	}

// fps
	if (sworld.fps < 4) // 1000/4 -> 250ms
		sworld.fps = 4;
	if (sworld.fps > 100)
		sworld.fps = 100;
}


FILE *openFile (char *filename, char *name, int opt, int mode)
{
	FILE *file;
	
	if (sworld.options & O_STDOUT && whatToStdout == opt) {
		file = stdout;
		filename = "stdout";
		if (_setmode (_fileno(stdout), mode) == -1)
		{
			Sys_Printf("failed to set %s mode for stdout", mode == _O_TEXT ? "text" : "binary");
			return NULL;
		}
	} else
		file = fopen (filename, mode == _O_TEXT ? "wt" : "wb");

	if (file == NULL)
		Sys_Printf ("couldn't open for writing: %s.\n", filename);
	else
		Sys_Printf("%s: %s\n", name, filename);

	return file;
}

/*
===================
Files_Init

Init file system
===================
*/

int Files_Init (int options)
{
	char *p, log[MAX_OSPATH], debug[MAX_OSPATH], convert[MAX_OSPATH], out[MAX_OSPATH], analyse[MAX_OSPATH];
	int job = 0;
	extern char *sourcePath;

	strcpy(convert, TemplateName(sworld.from.name, sworld.demo.name));
	ForceExtension(convert, ".mvd");

	strcpy(log, TemplateName(sworld.from.name, sworld.log.name));
	strcpy(debug, TemplateName(sworld.from.name, sworld.debug.name));
	strcpy(out, TemplateName(sworld.from.name, outputDir));
	strcpy(analyse, TemplateName(sworld.from.name, sworld.analyse.name));
	
	if (out[0]) 
	{
		Sys_mkdir(out);
		p = out + strlen(out) - 1;
		if (*p != '/' && *p != '\\')
		{
			*++p = '/';
			*++p = 0;
		}
	}

	sprintf(analyse, "%s", va("%s%s", out, analyse));
	sprintf(convert, "%s", va("%s%s", out, convert));
	sprintf(log, "%s", va("%s%s", out, log));
	sprintf(debug, "%s", va("%s%s", out, debug));

	if (strcmp(currentDir, sourcePath))
		strcpy(sworld.from.name, va("%s%s", sourcePath, sworld.from.name));

	if (!strcmp(convert, sworld.from.name) && options & O_CONVERT) {
		Sys_Printf("Can't convert from %s to %s!\n", sworld.from.name, convert);
		options -= O_CONVERT;
	}

	// open source file

	if (sworld.options & O_STDIN) {
		if (from.format == qwd)
			strcat(sworld.from.name, " (qwd)");
		else
			strcat(sworld.from.name, " (mvd)");
	} else if ((sworld.from.filesize = FileOpenRead(sworld.from.name, &sworld.from.file)) == -1)
	{
		Sys_Printf("couldn't open for reading: %s\n", sworld.from.name);
		return 0;
	}

	Sys_Printf("\nsource: %s\n", sworld.from.name);

	// open output files
	if (options & O_CONVERT) {
		sworld.demo.file = openFile(convert, " converting to", O_CONVERT, _O_BINARY);
		if (sworld.demo.file)
			job |= O_CONVERT;
	}

	if (options & O_DEBUG) {
		sworld.debug.file = openFile(debug, " debug", O_DEBUG, _O_TEXT);
		if (sworld.debug.file)
			job |= O_DEBUG;
	}

	if (options & O_LOG) {
		sworld.log.file = openFile(log, " log", O_LOG, _O_TEXT);
		if (sworld.log.file)
			job |= O_LOG;
	}

	if (options & O_ANALYSE) {
		sworld.analyse.file = openFile(analyse, " analyse", O_ANALYSE, _O_TEXT);
		if (sworld.analyse.file)
			job |= O_ANALYSE;
	}

	return job;
}

void Load_ini (void)
{
	char *buf;
	char name[MAX_OSPATH];
	char path[MAX_OSPATH];
	int i;

	if (CheckParm("-noini"))
		return;

	//get path to program
	strcpy(path, getPath(com_argv[0]));
	// if path == current dir, use short name
	if (!_stricmp(path, va("%s\\", currentDir)))
		strcpy(name, com_argv[0] + strlen(path));
	else
		strcpy(name, com_argv[0]);

	StripExtension(name, name);
	ForceExtension(name, ".ini");

	//Sys_Printf("name:%s\nargv:%s\n", name, com_argv[0]);

	if ( (i = CheckParm("-ini")) != 0)
	{
		if (i+1 < com_argc) {
			strcpy(name,com_argv[i+1]);
			RemoveParm(i);
		} else {
			Help();
			Sys_Exit(1);
		}

		RemoveParm(i);
	}

	buf = (char*) LoadFile(name);
	if (!buf) {
		Sys_Printf("couldn't load %s\n", name);
		return;
	}

	ReadIni(buf);

	free(buf);
}