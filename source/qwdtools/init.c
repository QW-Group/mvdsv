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

#include "defs.h"

#define DEFAULT_FPS 20
#define DEBUG_FILE	"debug.txt"
#define LOG_FILE	"log.txt"
#ifdef _WIN32
#define BINARY_NAME "qwdtools.exe"
#else
#define BINARY_NAME "qwdtools"
#endif

extern int com_argc;
extern char *com_argv[MAX_NUM_ARGVS];


void Help (void)
{
	Sys_Printf("usage: " BINARY_NAME " [-options] demoname1 demoname2 ...\n"
	           "example:\n"
	           BINARY_NAME " -c -o * -od out mydemo.qwd\nwill convert demo 'mydemo.qwd' to 'out/mydemo.mvd'\n\n");

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
	Sys_Printf("-m                     marges multiple demos to one mvd demo\n");
	//Sys_Printf("-analyse/-a            - analysing demo\n");
	Sys_Printf("-log                   creates log file\n");
	Sys_Printf("-debug                 prints way too much messages\n");
#ifdef _WIN32
	Sys_Printf("-wait                  \"Press any key to continue\"\n");
#endif
	Sys_Printf("-noini                 ini file won't be executed\n");
	Sys_Printf("-ini path              name of ini file (default: qwdtools.ini)\n");
	Sys_Printf("-stdin [format]        file from stdin  (default format: qwd)\n");
	Sys_Printf("-stdout                result to stdout (first found option: -c/-log/-debug)\n");
	//Sys_Printf("\n" VERSION " (c) 2001 Bartlomiej Rychtarski (highlander@gracz.net)\nhttp://qwex.n3.net/\n");
}

#ifdef _WIN32
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
#else
void CtrlC( int sig )
{
	if (sworld.options & O_SHUTDOWN)
		exit(3);

	Sys_Printf("Shutting down...\n");

	sworld.options |= O_SHUTDOWN;

	signal(sig, CtrlC);
}

void CtrlH_Init (void)
{
	signal(SIGINT, CtrlC);
	signal(SIGTERM, CtrlC);
}
#endif
/*
===================
World_Init
 
Initial values
===================
*/
void CleanName_Init ();
void World_Init(void)
{
	int i;

	memset(sourceName, 0, sizeof(sourceName));

	strlcpy(qizmoDir, "", sizeof(qizmoDir));
	strlcpy(outputDir, "", sizeof(outputDir));

	sworld.fps = DEFAULT_FPS;
	sworld.range = 5;
	strlcpy (sworld.debug.name, DEBUG_FILE, MAX_OSPATH);
	strlcpy (sworld.log.name, LOG_FILE, MAX_OSPATH);
	strlcpy (sworld.demo.name, "*", MAX_OSPATH);
	strlcpy (sworld.analyse.name, "*.txt", MAX_OSPATH);

	net_message.maxsize = sizeof(net_message_buffer);
	net_message.data = net_message_buffer;

	if ((i = CheckParm("-bd")) == 0)
		i = CheckParm("-base_dir");

	if (i && i + 1 < com_argc)
	{
#ifdef _WIN32
		SetCurrentDirectory(com_argv[i+1]);
#else
		chdir(com_argv[i+1]);
#endif

	}

#ifdef _WIN32
	GetCurrentDirectory(sizeof(currentDir), currentDir);
#else
	getcwd(currentDir, sizeof(currentDir));
#endif

	//Sys_Printf("arg:%s\ncur:%s\n", com_argv[0], currentDir);

#ifdef _WIN32
	ConsoleInHndl = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,NULL);
	ConsoleOutHndl = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,NULL);
	if (CheckParm("-wait") || CheckParm("-waitforkbhit"))
		sworld.options |= O_WAITFORKBHIT;
#endif

	if (CheckParm("-stdout"))
		sworld.options |= O_STDOUT;

	CleanName_Init ();
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

#ifdef _MSC_VER
#pragma warning( disable : 4133 4047)
#endif
param_t params[] =
	{
		{"-qizmo_dir",			"-qd",	TYPE_S, {(char *) qizmoDir}, sizeof(qizmoDir)},
		{"-output_dir",			"-od",	TYPE_S, {(char *) outputDir}, sizeof(outputDir)},
		{"-output",				"-o",	TYPE_S, {(char *) sworld.demo.name}, sizeof(sworld.demo.name)},
		{"-debug_file",			NULL,	TYPE_S, {(char *) sworld.debug.name}, sizeof(sworld.debug.name)},
		{"-log_file",			NULL,	TYPE_S, {(char *) sworld.log.name}, sizeof(sworld.log.name)},
		{"-fps",				NULL,	TYPE_I, {(char *) &sworld.fps}},
		{"-filter_spectalk",	"-fs",	TYPE_O, {(char *) O_FS}},
		{"-filter_qizmotalk",	"-fq",	TYPE_O, {(char *) O_FQ}},
		{"-filter_chats",		"-fc",	TYPE_O, {(char *) O_FC}},
		{"-filter_teamchats",	"-ft",	TYPE_O, {(char *) O_FT}},
		{"-convert",			"-c",	TYPE_O, {(char *) O_CONVERT}},
		{"-analyse",			"-a",	TYPE_O, {(char *) O_ANALYSE}},
		{"-log",				NULL,	TYPE_O, {(char *) O_LOG}},
		{"-debug",				NULL,	TYPE_O, {(char *) O_DEBUG}},
#ifdef _WIN32
		{"-waitforkbhit",		"-wait",TYPE_O, {(char *) O_WAITFORKBHIT}},
#endif
		{"-stdin",				NULL,	(type_t) (TYPE_O | TYPE_S), {(char *) stdintype}, sizeof(stdintype), (int) O_STDIN},
		{"-stdout",				NULL,	TYPE_O, {(char *) O_STDOUT}},
		{"-noini"},
		{"-base_dir",			"-bd",	TYPE_S, {(char *) currentDir}, sizeof(currentDir)},
		{"-msglevel",			"-msg",	TYPE_I, {(char *) &sworld.msglevel}},
		{"-marge",				"-m",	TYPE_O, {(char *) O_MARGE}},
		{"-range",				"-r",	TYPE_I, {(char *) &sworld.range}},
		{NULL}
	};

#ifdef _MSC_VER
#pragma warning( default : 4133 4047)
#endif

param_t *FindParam(char *parm)
{
	param_t *param;

	for (param = params; param->name; param++)
		if (!strcasecmp(parm, param->name) || (param->shname && !strcasecmp(parm, param->shname)))
			return param;

	return NULL;
}

void ParseArgv(void)
{
	int	i, o;
	param_t	*param;
	char	qwz_files[1024] = "", *ext, *arg, tmp[MAX_OSPATH];

	if (CheckParm("-help"))
	{
		Help();
		Sys_Exit(1);
	}

	// get qizmo dir before args are parsed
	if ((i = CheckParm("-qd")) == 0)
		i = CheckParm("-qizmo_dir");

	if (i && i + 1 < com_argc)
		strlcpy(qizmoDir, com_argv[i+1], sizeof(qizmoDir));

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

			// it's a file name, first add default extension if needed
			// then add to file list
			strlcpy(tmp, arg, sizeof(tmp));
			COM_DefaultExtension(tmp, ".qwd");
			sworld.sources += AddToFileList(sworld.filelist, tmp);

			// check for qwz extension
			ext = COM_FileExtension (arg);
			if (!strcasecmp(ext, "qwz"))
			{
				if (!strcmp(qizmoDir, ".") || !strcmp(qizmoDir, "./")
#ifdef _WIN32
				        || !strcmp(qizmoDir, ".\\")
#endif
				   )
					qizmoDir[0] = 0;

				if (strstr(arg, ":") || !qizmoDir[0])
					strlcat(qwz_files, va("\"%s\" ", arg), sizeof(qwz_files));
				else
					strlcat(qwz_files, va("\"%s/%s\" ", currentDir, arg), sizeof(qwz_files));
			}

			continue;
		}

		// if started from -- ignore
		if (arg != com_argv[i])
			continue;

		// run option
		if (param->type & TYPE_S)
		{
			i++;
			if (i < com_argc && com_argv[i][0] != '-')
			{
				strlcpy(param->opt1.str, com_argv[i], param->str_len);
			}
			else
			{
				// if it's stdin assume qwd
				if (param->opt2 == O_STDIN)
				{
					strlcpy(param->opt1.str, "qwd", param->str_len);
					i--;
				}
				else
				{
					Sys_Printf("init: missing argument for %s\n", arg);
					Help();
					Sys_Exit(1);
				}
			}

			if ((param->opt1.str = currentDir))
			{
#ifdef _WIN32
				SetCurrentDirectory(currentDir);
				GetCurrentDirectory(sizeof(currentDir), currentDir);
#else
				chdir(currentDir);
				getcwd(currentDir, sizeof(currentDir));
#endif

			}
		}

		if (param->type & TYPE_I)
		{
			i++;
			if (i < com_argc && com_argv[i][0] != '-')
			{
				*param->opt1.Int = Q_atoi(com_argv[i]);
			}
			else
			{
				Sys_Printf("init: missing argument for %s\n", arg);
				Help();
				Sys_Exit(1);
			}
		}

		if (param->type & TYPE_O)
		{
			if (param->opt2)
				o = param->opt2;
			else
				o = param->opt1.Opt;
			if (!whatToStdout && (o & JOB_TODO))
				whatToStdout = o;
			sworld.options |= o;
		}
	}

	if (!(sworld.options & JOB_TODO))
	{
		sworld.options |= O_CONVERT;
		whatToStdout = O_CONVERT;
	}

	if (sworld.options & O_MARGE)
		sworld.options -= (sworld.options & (O_CONVERT | O_STDIN));

	// change translation mode for stdin
	if (sworld.options & O_STDIN)
	{
		sworld.sources = 1;
		sworld.from->file = stdin;

		// free file list, files from argument list won't be used
		FreeFileList(sworld.filelist);
		AddToFileList(sworld.filelist, "");
#ifdef _WIN32
		if (_setmode (_fileno(sworld.from->file), _O_BINARY) == -1)
			Sys_Error("failed to set binary mode for stdin");
#endif

	}
	else
	{
		if (qwz_files[0])
		{
			// decompress qwz files
			if (!OpenQWZ(qwz_files))
				Sys_Printf("Couldn't decompress qwz file(s)\n");
		}
	}

	if (sworld.filelist[0].list == NULL/* && !(sworld.options & O_STDIN)*/)
	{
		Sys_Printf("demo name not specified\n");
		Help();
		Sys_Exit(1);
	}

	// fps
	if (sworld.fps < 4) // 1000/4 -> 250ms
		sworld.fps = 4;
	if (sworld.fps > 1000)
		sworld.fps = 1000;

	if (sworld.range < 1) sworld.range = 1;
	if (sworld.range > 15) sworld.range = 15;
}


FILE *openFile (char *filename, char *name, int opt, int mode)
{
	FILE *file;

	if (sworld.options & O_STDOUT && whatToStdout == opt)
	{
		file = stdout;
		filename = "stdout";
#ifdef _WIN32
		if (_setmode (_fileno(stdout), mode) == -1)
		{
			Sys_Printf("failed to set %s mode for stdout", mode == _O_TEXT ? "text" : "binary");
			return NULL;
		}
#endif

	}
	else
#ifdef _WIN32
		file = fopen (filename, mode == _O_TEXT ? "wt" : "wb");
#else
		file = fopen (filename, "w");
#endif

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
	char *p, log1[MAX_OSPATH], debug[MAX_OSPATH], convert[MAX_OSPATH], out[MAX_OSPATH], analyse[MAX_OSPATH], d[MAX_OSPATH];
	int job = 0, i;

	if (!strcmp(currentDir, sworld.from->path))
		strlcpy(d, ".", sizeof(d));
	else
		strlcpy(d, sworld.from->path, sizeof(d));

	strlcpy(out, TemplateName(d, outputDir, "@"), sizeof(out));
	//Sys_Printf("%s, %s, %s, %s\n", sworld.from->path, d, outputDir, out);

	if (options & O_MARGE)
		p = "marge";
	else
		p = sworld.from->name;


	strlcpy(convert, TemplateName(p, sworld.demo.name, "*"), sizeof(convert));
	ForceExtension(convert, ".mvd");

	strlcpy(log1, TemplateName(p, sworld.log.name, "*"), sizeof(log1));
	strlcpy(debug, TemplateName(p, sworld.debug.name, "*"), sizeof(debug));
	strlcpy(out, TemplateName(p, out, "*"), sizeof(out));
	strlcpy(analyse, TemplateName(p, sworld.analyse.name, "*"), sizeof(analyse));

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

	snprintf(analyse, sizeof(analyse), "%s", va("%s%s", out, analyse));
	snprintf(convert, sizeof(convert), "%s", va("%s%s", out, convert));
	snprintf(log1, sizeof(log1), "%s", va("%s%s", out, log1));
	snprintf(debug, sizeof(debug), "%s", va("%s%s", out, debug));

	// open source file(s)
	for (i = 0, from = sources; i < sworld.fromcount; i++, from++)
	{
		if (strcmp(currentDir, sworld.from[i].path))
			strlcpy(sworld.from[i].name, va("%s%s", sworld.from[i].path, sworld.from[i].name), MAX_OSPATH);

		if (!strcmp(convert, sworld.from[i].name) && options & O_CONVERT)
		{
			Sys_Printf("Can't convert from %s to %s!\n", sworld.from[i].name, convert);
			options -= O_CONVERT;
		}

		if (sworld.options & O_STDIN)
		{
			if (from->format == qwd)
				strlcat(sworld.from[i].name, " (qwd)", sizeof(sworld.from[i].name));
			else
				strlcat(sworld.from[i].name, " (mvd)", sizeof(sworld.from[i].name));
		}
		else if ((sworld.from[i].filesize = FileOpenRead(sworld.from[i].name, &sworld.from[i].file)) == -1)
		{
			Sys_Printf("couldn't open for reading: %s\n", sworld.from[i].name);
			return 0;
		}

		world.demossize += sworld.from[i].filesize;
	}

	if (options & O_MARGE)
	{
		Sys_Printf("sources: %s\n", sworld.from[0].name);
		for (i = 1; i < sworld.fromcount; i++)
			Sys_Printf("         %s\n", sworld.from[i].name);
	}
	else
		Sys_Printf("source: %s\n", sworld.from[0].name);

	// open output files
	if (options & O_CONVERT)
	{
		sworld.demo.file = openFile(convert, " converting to", O_CONVERT, _O_BINARY);
		if (sworld.demo.file)
			job |= O_CONVERT;
	}

	if (options & O_MARGE)
	{
		sworld.demo.file = openFile(convert, " marging to", O_CONVERT, _O_BINARY);
		if (sworld.demo.file)
			job |= O_MARGE;
	}

	if (options & O_DEBUG)
	{
		sworld.debug.file = openFile(debug, " debug", O_DEBUG, _O_TEXT);
		if (sworld.debug.file)
			job |= O_DEBUG;
	}

	if (options & O_LOG)
	{
		sworld.log.file = openFile(log1, " log", O_LOG, _O_TEXT);
		if (sworld.log.file)
			job |= O_LOG;
	}

	if (options & O_ANALYSE)
	{
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
	strlcpy(path, getPath(com_argv[0]), sizeof(path));
	// if path == current dir, use short name
	if (!strcasecmp(path, va("%s/", currentDir)))
		strlcpy(name, com_argv[0] + strlen(path), sizeof(name));
	else
		strlcpy(name, com_argv[0], sizeof(name));

	COM_StripExtension(name, name);
	ForceExtension(name, ".ini");

	//Sys_Printf("name:%s\nargv:%s\n", name, com_argv[0]);

	if ( (i = CheckParm("-ini")) != 0)
	{
		if (i+1 < com_argc)
		{
			strlcpy(name,com_argv[i+1], sizeof(name));
			RemoveParm(i);
		}
		else
		{
			Help();
			Sys_Exit(1);
		}

		RemoveParm(i);
	}

	buf = (char*) LoadFile(name);
	if (!buf)
	{
		Sys_Printf("couldn't load %s\n", name);
		return;
	}

	ReadIni(buf);

	Q_free(buf);
}
