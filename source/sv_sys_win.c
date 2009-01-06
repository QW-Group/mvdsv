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
 
	$Id$
*/

#include "qwsvdef.h"
#include <mmsystem.h>

extern cvar_t sys_restart_on_error;
extern cvar_t not_auth_timeout;
extern cvar_t auth_timeout;

struct timeval select_timeout;

cvar_t	sys_nostdout = {"sys_nostdout", "0"};
cvar_t	sys_sleep = {"sys_sleep", "8"};

static char title[16];

static qbool	iosock_ready = false;
static qbool	authenticated = false;
static double	cur_time_auth;
static qbool	isdaemon = false;

//==============================================================================
// WINDOWS CMD LINE CRAP
//==============================================================================

int			argc;
char		*argv[MAX_NUM_ARGVS];
static char	exename[1024] = {0}; // static, so it seen in this file only

void ParseCommandLine (char *lpCmdLine) 
{
	int i;
	argc = 1;
	argv[0] = exename;

	if(!(i = GetModuleFileName(NULL, exename, sizeof(exename)-1))) // here we get loong string, with full path
		exename[0] = 0; // oh, something bad
	else 
	{
		exename[i] = 0; // ensure null terminator
//		strlcpy(exename, COM_SkipPath(exename), sizeof(exename));
	}

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			if (*lpCmdLine == '\"')
			{
				lpCmdLine++;

				argv[argc] = lpCmdLine;
				argc++;

				while (*lpCmdLine && *lpCmdLine != '\"') // this include chars less that 32 and greate than 126... is that evil?
					lpCmdLine++;
			}
			else
			{
				argv[argc] = lpCmdLine;
				argc++;

				while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
					lpCmdLine++;
			}

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
		}
	}
}

/*
================
Sys_FileTime
================
*/
int	Sys_FileTime (const char *path)
{
	struct _stat buf;
	return _stat (path, &buf) == -1 ? -1 : buf.st_mtime;
}

/*
================
Sys_mkdir
================
*/
void Sys_mkdir (const char *path)
{
	_mkdir(path);
}

/*
================
Sys_remove
================
*/
int Sys_remove (const char *path)
{
	return remove(path);
}

//bliP: rmdir ->
/*
================
Sys_rmdir
================
*/
int Sys_rmdir (const char *path)
{
	return _rmdir(path);
}
//<-

/*
================
Sys_listdir
================
*/

dir_t Sys_listdir (const char *path, const char *ext, int sort_type)
{
	static file_t	list[MAX_DIRFILES];
	dir_t	dir;
	HANDLE	h;
	WIN32_FIND_DATA fd;
	char	pathname[MAX_DEMO_NAME];
	qbool all;

	int	r;
	pcre	*preg;
	const char	*errbuf;

	memset(list, 0, sizeof(list));
	memset(&dir, 0, sizeof(dir));

	dir.files = list;
	all = !strncmp(ext, ".*", 3);
	if (!all)
		if (!(preg = pcre_compile(ext, PCRE_CASELESS, &errbuf, &r, NULL)))
		{
			Con_Printf("Sys_listdir: pcre_compile(%s) error: %s at offset %d\n",
			           ext, errbuf, r);
			Q_free(preg);
			return dir;
		}

	snprintf(pathname, sizeof(pathname), "%s/*.*", path);
	if ((h = FindFirstFile (pathname , &fd)) == INVALID_HANDLE_VALUE)
	{
		if (!all)
			Q_free(preg);
		return dir;
	}

	do
	{
		if (!strncmp(fd.cFileName, ".", 2) || !strncmp(fd.cFileName, "..", 3))
			continue;
		if (!all)
		{
			switch (r = pcre_exec(preg, NULL, fd.cFileName,
			                      strlen(fd.cFileName), 0, 0, NULL, 0))
			{
			case 0: break;
			case PCRE_ERROR_NOMATCH: continue;
			default:
				Con_Printf("Sys_listdir: pcre_exec(%s, %s) error code: %d\n",
				           ext, fd.cFileName, r);
				if (!all)
					Q_free(preg);
				return dir;
			}
		}

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //bliP: list dir
		{
			dir.numdirs++;
			list[dir.numfiles].isdir = true;
			list[dir.numfiles].size = list[dir.numfiles].time = 0;
		}
		else
		{
			list[dir.numfiles].isdir = false;
			snprintf(pathname, sizeof(pathname), "%s/%s", path, fd.cFileName);
			list[dir.numfiles].time = Sys_FileTime(pathname);
			dir.size += (list[dir.numfiles].size = fd.nFileSizeLow);
		}
		strlcpy (list[dir.numfiles].name, fd.cFileName, sizeof(list[0].name));

		if (++dir.numfiles == MAX_DIRFILES - 1)
			break;

	}
	while (FindNextFile(h, &fd));

	FindClose (h);
	if (!all)
		Q_free(preg);

	switch (sort_type)
	{
	case SORT_NO: break;
	case SORT_BY_DATE:
		qsort((void *)list, dir.numfiles, sizeof(file_t), Sys_compare_by_date);
		break;
	case SORT_BY_NAME:
		qsort((void *)list, dir.numfiles, sizeof(file_t), Sys_compare_by_name);
		break;
	}
	return dir;
}

/*
================
Sys_Exit
================
*/
void Sys_Exit(int code)
{
#ifndef _CONSOLE
	RemoveNotifyIcon();
#endif
	exit(code);
}

/*
================
Sys_Quit
================
*/
void Sys_Quit (qbool restart)
{
	if (restart)
	{
// FIXME: restart are buggy atm: does't close sockets and file handlers...
// TODO: net.c(486): add close of QTV sockets and turn on restart back
//		if (execv(argv[0], com_argv) == -1)
		{
#ifdef _CONSOLE
			if (!((int)sys_nostdout.value || isdaemon))
				printf("Restart failed: (%i): %s\n", qerrno, strerror(qerrno));
#else
			if (!(COM_CheckParm("-noerrormsgbox") || isdaemon))
				MessageBox(NULL, strerror(qerrno), "Restart failed", 0 /* MB_OK */ );
#endif
			Sys_Exit(1);
		}
	}
	Sys_Exit(0);
}

/*
================
Sys_Error
================
*/
void Sys_Error (const char *error, ...)
{
	static qbool inerror = false;
	va_list argptr;
	char text[1024];

	sv_error = true;

	if (inerror)
		Sys_Exit (1);

	inerror = true;

	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

#ifdef _CONSOLE
	if (!((int)sys_nostdout.value || isdaemon))
		printf ("ERROR: %s\n", text);
#else
	if (!(COM_CheckParm ("-noerrormsgbox") || isdaemon))
		MessageBox (NULL, text, "Error", 0 /* MB_OK */ );
	else
		Sys_Printf ("ERROR: %s\n", text);

#endif

	if (logs[ERROR_LOG].sv_logfile)
	{
		SV_Write_Log (ERROR_LOG, 1, va ("ERROR: %s\n", text));
//		fclose (logs[ERROR_LOG].sv_logfile);
	}

// FIXME: hack - checking SV_Shutdown with net_socket set in -1 NET_Shutdown
	if (net_socket != -1)
		SV_Shutdown ();

	if ((int)sys_restart_on_error.value)
		Sys_Quit (true);

	Sys_Exit (1);
}

static double pfreq;
static qbool hwtimer = false;
static __int64 startcount;
void Sys_InitDoubleTime (void)
{
	__int64 freq;

	if (!COM_CheckParm("-nohwtimer") &&
		QueryPerformanceFrequency ((LARGE_INTEGER *)&freq) && freq > 0)
	{
		// hardware timer available
		pfreq = (double)freq;
		hwtimer = true;
		QueryPerformanceCounter ((LARGE_INTEGER *)&startcount);
	}
	else
	{
		// make sure the timer is high precision, otherwise
		// NT gets 18ms resolution
		timeBeginPeriod (1);
	}
}

double Sys_DoubleTime (void)
{
	__int64 pcount;

	static DWORD starttime;
	static qbool first = true;
	DWORD now;

	if (hwtimer)
	{
		QueryPerformanceCounter ((LARGE_INTEGER *)&pcount);
		if (first) {
			first = false;
			startcount = pcount;
			return 0.0;
		}
		// TODO: check for wrapping; is it necessary?
		return (pcount - startcount) / pfreq;
	}

	now = timeGetTime();

	if (first)
	{
		first = false;
		starttime = now;
		return 0.0;
	}

	if (now < starttime) // wrapped?
		return (now / 1000.0) + (LONG_MAX - starttime / 1000.0);

	if (now - starttime == 0)
		return 0.0;

	return (now - starttime) / 1000.0;
}
#if 0
double Sys_DoubleTime (void)
{
	double t;
	struct _timeb tstruct;
	static int	starttime;

	_ftime( &tstruct );
 
	if (!starttime)
		starttime = tstruct.time;
	t = (tstruct.time-starttime) + tstruct.millitm*0.001;
	
	return t;
}
#endif
/*
================
Sys_ConsoleInput
================
*/
char *Sys_ConsoleInput (void)
{
	static char	text[256], *t;
	static int	len = 0;
#ifdef _CONSOLE
	int		c;

	// read a line out
	if (!isdaemon)
		while (_kbhit())
		{
			c = _getch();

			if (c == 224)
			{
				if (_kbhit())
				{
					// assume escape sequence (arrows etc), skip
					_getch();
					continue;
				}
				// assume character
			}

			if (c < 32 && c != '\r' && c != 8)
				continue;

			putch (c);
			if (c == '\r')
			{
				text[len] = 0;
				putch ('\n');
				len = 0;
				return text;
			}
			if (c == 8)
			{
				if (len)
				{
					putch (' ');
					putch (c);
					len--;
					text[len] = 0;
				}
				continue;
			}

			text[len] = c;
			len++;
			text[len] = 0;
			if (len == sizeof(text))
				len = 0;
		}
#endif

	if (telnetport)
	{
		if (!iosock_ready || !telnet_connected)
			return NULL;		// the select didn't say it was ready
		iosock_ready = false;
		do
		{
			switch (recv (telnet_iosock, text + len, 1, 0))
			{
			case 0:
				len = telnet_connected = authenticated = false;
				closesocket (telnet_iosock);
				SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
				return NULL;
			case 1:
				if (text[len] == 8)
				{
					if(len > 0) --len;
				}
				else
					++len;
				break;
			default:
				if (errno != EAGAIN && errno != ENOENT && errno != EBADF && qerrno != EWOULDBLOCK) // demand for M$ WIN 2K telnet support
				{
					len = telnet_connected = authenticated = false;
					closesocket (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, va("Connection closed with error: (%i): %s; (%i): %s.\n", qerrno, strerror(qerrno), errno, strerror(errno)));
				}
				return NULL;
			}// switch
		}// do
		while (len < sizeof(text) - 1 && text[len - 1] != '\n' && text[len - 1] != '\r' && text[len - 1] != 0);

		if (len == 0)
			return NULL;

		text[len] = 0;

		if (text[len - 1] == '\r' || text[len - 1] == '\n')
			text[len - 1] = 0;

		if (text[0] == 0)
		{
			len = 0;
			return NULL;
		}

		if (!authenticated)
		{
			len = strlen(t = Cvar_String ("telnet_password"));
			if (len && (authenticated = (!strncmp(text, t, min(sizeof(text), len + 1)))))
			{
				cur_time_auth = realtime;
				SV_Write_Log(TELNET_LOG, 1, "Authenticated: yes\n");
				len = 0;
				return "status";
			}
			else
			{
				if (strlen(text) == 1 && text[0] == 4)
				{
					len = telnet_connected = authenticated = false;
					closesocket (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
				}
				else
					SV_Write_Log(TELNET_LOG, 1, "Authenticated: no\n");
				len = 0;
				return NULL;
			}
		}

		if (strlen(text) == 1 && text[0] == 4)
		{
			len = telnet_connected = authenticated = false;
			closesocket (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
			len = 0;
			return NULL;
		}
		len = 0;
		SV_Write_Log(TELNET_LOG, 2, va("%s\n", text));
		return text;
	}
	return NULL;
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	extern char	chartbl2[];
	va_list argptr;
	unsigned char text[MAXCMDBUF];
	unsigned char *p;

	if ((
#ifdef _CONSOLE
	            (int)sys_nostdout.value ||
#endif //_CONSOLE
	            isdaemon) && !(telnetport && telnet_connected && authenticated))
		return;

	va_start (argptr, fmt);
	vsnprintf ((char *)text, MAXCMDBUF, fmt, argptr);
	va_end (argptr);

#ifndef _CONSOLE
	if (!isdaemon)
		ConsoleAddText((char *)text);
#endif //_CONSOLE

	for (p = text; *p; p++)
	{
		*p = chartbl2[*p];
		if (telnetport && telnet_connected && authenticated)
		{
			send (telnet_iosock, (char *) p, 1, 0);
			if (*p == '\n') // demand for M$ WIN 2K telnet support
				send (telnet_iosock, "\r", 1, 0);
		}
#ifdef _CONSOLE
		if (!((int)sys_nostdout.value || isdaemon))
			putc(*p, stdout);
#endif //_CONSOLE

	}

	if (telnetport && telnet_connected && authenticated)
		SV_Write_Log(TELNET_LOG, 3, (char *) text);
#ifdef _CONSOLE
	if (!((int)sys_nostdout.value || isdaemon))
		fflush(stdout);
#endif //_CONSOLE
}

/*
=============
Sys_Init
 
Quake calls this so the system can register variables before host_hunklevel
is marked
=============
*/
void Sys_Init (void)
{
	qbool	WinNT;
	OSVERSIONINFO	vinfo;

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod( 1 );

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if (vinfo.dwMajorVersion < 4 || vinfo.dwPlatformId == VER_PLATFORM_WIN32s)
		Sys_Error (SERVER_NAME " requires at least Win95 or NT 4.0");

	WinNT = (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ? true : false);

	Cvar_Register (&sys_nostdout);
	Cvar_Register (&sys_sleep);

	if (COM_CheckParm ("-nopriority"))
	{
		Cvar_Set (&sys_sleep, "0");
	}
	else
	{
		if ( ! SetPriorityClass (GetCurrentProcess(), HIGH_PRIORITY_CLASS))
			Con_Printf ("SetPriorityClass() failed\n");
		else
			Con_Printf ("Process priority class set to HIGH\n");

		// sys_sleep > 0 seems to cause packet loss on WinNT (why?)
		if (WinNT)
			Cvar_Set (&sys_sleep, "0");
	}

	Sys_InitDoubleTime ();
}

__inline void Sys_Telnet (void);
qbool NET_Sleep ()
{
	struct timeval timeout_cur;
	fd_set	fdset;
	int j = net_socket;

	FD_ZERO (&fdset);
	FD_SET(j, &fdset); // network socket

	// Added by VVD {
	if (telnetport)
	{
		Sys_Telnet();
		FD_SET(net_telnetsocket, &fdset);
		j = max(j, net_telnetsocket);
		if (telnet_connected)
		{
			FD_SET(telnet_iosock, &fdset);
			j = max(j, telnet_iosock);
		}
	}
	// Added by VVD }
	timeout_cur.tv_sec  = select_timeout.tv_sec;
	timeout_cur.tv_usec = select_timeout.tv_usec;

	switch (select (++j, &fdset, NULL, NULL, &timeout_cur))
	{
		case -1: return true;
		case 0: break;
		default:
			if (telnetport && telnet_connected)
				iosock_ready = FD_ISSET(telnet_iosock, &fdset);
#ifdef _CONSOLE
			if (do_stdin)
				stdin_ready = FD_ISSET(0, &fdset);
#endif //_CONSOLE
	}
	return false;
}

void Sys_Sleep (unsigned long ms)
{
	Sleep (ms);
}

int Sys_Script (const char *path, const char *args)
{
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
	char cmdline[1024], curdir[MAX_OSPATH];

	memset (&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWMINNOACTIVE;

	GetCurrentDirectory(sizeof(curdir), curdir);


	snprintf(cmdline, sizeof(cmdline), "%s\\sh.exe %s.qws %s", curdir, path, args);
	strlcat(curdir, va("\\%s", fs_gamedir+2), MAX_OSPATH);

	return CreateProcess (NULL, cmdline, NULL, NULL,
	                      FALSE, 0/*DETACHED_PROCESS /*CREATE_NEW_CONSOLE*/ , NULL, curdir, &si, &pi);
}

DL_t Sys_DLOpen (const char *path)
{
	return LoadLibrary (path);
}

qbool Sys_DLClose (DL_t dl)
{
	return FreeLibrary (dl);
}

void *Sys_DLProc (DL_t dl, const char *name)
{
	return (void *) GetProcAddress (dl, name);
}

int Sys_CreateThread(DWORD (WINAPI *func)(void *), void *param)
{
    DWORD threadid;
    HANDLE thread;

    thread = CreateThread (
        NULL,               // pointer to security attributes
        0,                  // initial thread stack size
        func,               // pointer to thread function
        param,              // argument for new thread
        CREATE_SUSPENDED,   // creation flags
        &threadid);         // pointer to receive thread ID

    SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
    ResumeThread(thread);

    return 1;
}

__inline void Sys_Telnet (void)
{
	static int			tempsock;
	static struct		sockaddr_in remoteaddr, remoteaddr_temp;
	static socklen_t	sockaddr_len = sizeof(struct sockaddr_in);
	static double		cur_time_not_auth;
	if (telnet_connected)
	{
		if ((tempsock = accept (net_telnetsocket, (struct sockaddr*)&remoteaddr_temp, &sockaddr_len)) > 0)
		{
			//			if (remoteaddr_temp.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			send (tempsock, "Console busy by another user.\n", 31, 0);
			closesocket (tempsock);
			SV_Write_Log(TELNET_LOG, 1, va("Console busy by: %s. Refuse connection from: %s\n",
										   inet_ntoa(remoteaddr.sin_addr), inet_ntoa(remoteaddr_temp.sin_addr)));
		}
		if (	(!authenticated && (int)not_auth_timeout.value &&
				realtime - cur_time_not_auth > not_auth_timeout.value) ||
				(authenticated && (int)auth_timeout.value &&
				 realtime - cur_time_auth > auth_timeout.value))
		{
			telnet_connected = false;
			send (telnet_iosock, "Time for authentication finished.\n", 34, 0);
			closesocket (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, va("Time for authentication finished. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
		}
	}
	else
	{
		if ((telnet_iosock = accept (net_telnetsocket, (struct sockaddr*)&remoteaddr, (int *) &sockaddr_len)) > 0)
		{
			//			if (remoteaddr.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			//			{
			telnet_connected = true;
			cur_time_not_auth = realtime;
			SV_Write_Log(TELNET_LOG, 1, va("Accept connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
			send (telnet_iosock, "# ", 2, 0);
			/*			}
			else
			{
			closesocket (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, va("IP not match. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
			}
			*/
		}
	}
}

#ifdef _CONSOLE
/*
==================
main
 
==================
*/
char	*newargv[256];

int main (int argc, char **argv)
{
	quakeparms_t	parms;
	double		newtime, time, oldtime;
	static	char	cwd[1024];
	int		t;
	int		sleep_msec;

	GetConsoleTitle(title, sizeof(title));
	COM_InitArgv (argc, argv);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = DEFAULT_MEM_SIZE;

	if ((t = COM_CheckParm ("-heapsize")) != 0 &&
	        t + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[t + 1]) * 1024;

	if ((t = COM_CheckParm ("-mem")) != 0 &&
	        t + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[t + 1]) * 1024 * 1024;

	parms.membase = Q_malloc (parms.memsize);

	SV_Init (&parms);

	// run one frame immediately for first heartbeat
	SV_Frame (0.1);

	//
	// main loop
	//
	oldtime = Sys_DoubleTime () - 0.1;

	while (1)
	{
		sleep_msec = (int)sys_sleep.value;
		if (sleep_msec > 0)
		{
			if (sleep_msec > 13)
				sleep_msec = 13;
			Sleep (sleep_msec);
		}

		// select on the net socket and stdin
		// the only reason we have a timeout at all is so that if the last
		// connected client times out, the message would not otherwise
		// be printed until the next event.
		if (NET_Sleep ())
			continue;

		// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time);
	}

	return true;
}

#else  // _CONSOLE

void PR_CleanLogText_Init(void);
int APIENTRY WinMain(   HINSTANCE   hInstance,
						HINSTANCE   hPrevInstance,
						LPSTR       lpCmdLine,
						int         nCmdShow)
{

	static MSG			msg;
	static quakeparms_t	parms;
	static double		newtime, time, oldtime;
	static char			cwd[1024];
	register int		sleep_msec;
	int					j;

	static struct		timeval	timeout;
	static fd_set		fdset;
	static qbool		disable_gpf = false;

	ParseCommandLine(lpCmdLine);

	memset (&parms, 0, sizeof(parms));

	PR_CleanLogText_Init();

	COM_InitArgv (argc, argv);
	parms.argc = com_argc;
	parms.argv = com_argv;

	// create main window
	if (!CreateMainWindow(hInstance, nCmdShow))
		return 1;

	parms.memsize = DEFAULT_MEM_SIZE;

	j = COM_CheckParm ("-heapsize");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024;

	j = COM_CheckParm ("-mem");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024 * 1024;

	if (COM_CheckParm("-noerrormsgbox"))
		disable_gpf = true;

	if (COM_CheckParm ("-d"))
	{
		isdaemon = disable_gpf = true;
		//close(0); close(1); close(2);
	}

	if (disable_gpf)
	{
		DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
		SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
	}

	parms.membase = Q_malloc (parms.memsize);
	
	SV_Init (&parms);

	// if stared miminize update notify icon message (with correct port)
	if (minimized)
		UpdateNotifyIconMessage(va(SERVER_NAME ":%d", sv_port));

	// run one frame immediately for first heartbeat
	SV_Frame (0.1);

	//
	// main loop
	//
	oldtime = Sys_DoubleTime () - 0.1;

	while(1)
	{
		// get messeges sent to windows
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if( !GetMessage( &msg, NULL, 0, 0 ) )
				break;
			if(!IsDialogMessage(DlgHwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		CheckIdle();

		// server frame

		sleep_msec = (int)sys_sleep.value;
		if (sleep_msec > 0)
		{
			if (sleep_msec > 13)
				sleep_msec = 13;
			Sleep (sleep_msec);
		}

		// select on the net socket and stdin
		// the only reason we have a timeout at all is so that if the last
		// connected client times out, the message would not otherwise
		// be printed until the next event.

		if (NET_Sleep ())
			continue;

		// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time);
	}


	Sys_Exit(msg.wParam);

	return msg.wParam;
}

#endif // _CONSOLE
