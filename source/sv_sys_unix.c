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

extern cvar_t sys_restart_on_error;
extern cvar_t not_auth_timeout;
extern cvar_t auth_timeout;

cvar_t sys_nostdout = {"sys_nostdout", "0"};
cvar_t sys_extrasleep = {"sys_extrasleep", "0"};

static qbool	stdin_ready = false;
// Added by VVD {
static qbool	iosock_ready = false;
static double	cur_time_auth;
//static qbool	isdaemon = false;
static qbool	do_stdin = true;
// Added by VVD }
qbool	authenticated;

/*
===============================================================================

                         REQUIRED SYS FUNCTIONS

===============================================================================
*/


/*
============
Sys_FileTime

returns -1 if not present
============
*/
int	Sys_FileTime (const char *path)
{
	struct stat buf;
	return stat(path, &buf) == -1 ? -1 : buf.st_mtime;
}

int Sys_FileSizeTime (char *path, int *time1)
{
	struct stat buf;
	if (stat(path, &buf) == -1)
	{
		*time1 = -1;
		return 0;
	}
	else
	{
		*time1 = buf.st_mtime;
		return buf.st_size;
	}
}


/*
============
Sys_mkdir

============
*/
void Sys_mkdir (const char *path)
{
	if (mkdir (path, 0777) != -1)
		return;
	if (qerrno != EEXIST)
		Sys_Error ("mkdir %s: (%i): %s", path, qerrno, strerror(qerrno));
}

/*
================
Sys_remove
================
*/
int Sys_remove (const char *path)
{
	return unlink(path);
}

//bliP: rmdir ->
/*
================
Sys_rmdir
================
*/
int Sys_rmdir (const char *path)
{
	return rmdir(path);
}
//<-

/*
================
Sys_listdir
================
*/

dir_t Sys_listdir (const char *path, const char *ext, int sort_type)
{
	static file_t list[MAX_DIRFILES];
	dir_t dir;
	char pathname[MAX_OSPATH];
	DIR *d;
	DIR *testdir; //bliP: list dir
	struct dirent *oneentry;
	qbool all;

	int r;
	pcre *preg = NULL;
	const char *errbuf;

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

	if (!(d = opendir(path)))
	{
		if (!all)
			Q_free(preg);
		return dir;
	}
	while ((oneentry = readdir(d)))
	{
		if (!strncmp(oneentry->d_name, ".", 2) || !strncmp(oneentry->d_name, "..", 3))
			continue;
		if (!all)
		{
			switch (r = pcre_exec(preg, NULL, oneentry->d_name,
			                      strlen(oneentry->d_name), 0, 0, NULL, 0))
			{
			case 0: break;
			case PCRE_ERROR_NOMATCH: continue;
			default:
				Con_Printf("Sys_listdir: pcre_exec(%s, %s) error code: %d\n",
				           ext, oneentry->d_name, r);
				Q_free(preg);
				return dir;
			}
		}
		snprintf(pathname, sizeof(pathname), "%s/%s", path, oneentry->d_name);
		if ((testdir = opendir(pathname)))
		{
			dir.numdirs++;
			list[dir.numfiles].isdir = true;
			list[dir.numfiles].size = list[dir.numfiles].time = 0;
			closedir(testdir);
		}
		else
		{
			list[dir.numfiles].isdir = false;
			//list[dir.numfiles].time = Sys_FileTime(pathname);
			dir.size +=
				(list[dir.numfiles].size = Sys_FileSizeTime(pathname, &list[dir.numfiles].time));
		}
		strlcpy (list[dir.numfiles].name, oneentry->d_name, MAX_DEMO_NAME);

		if (++dir.numfiles == MAX_DIRFILES - 1)
			break;
	}
	closedir(d);
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
void Sys_Exit (int code)
{
	exit(code);		// appkit isn't running
}

/*
================
Sys_Quit
================
*/
char	*argv0;
void Sys_Quit (qbool restart)
{
	if (restart)
	{
// FIXME: restart are buggy atm: does't close sockets and file handlers...
// TODO: net.c(486): add close of QTV sockets and turn on restart back
//		if (execv(argv0, com_argv) == -1)
		{
			Sys_Printf("Restart failed: %s\n", strerror(qerrno));
			Sys_Exit(1);
		}
	}
	Sys_Exit(0);		// appkit isn't running
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
	char string[1024];

	sv_error = true;

	if (inerror)
		Sys_Exit (1);

	inerror = true;

	va_start (argptr,error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	if (!(int)sys_nostdout.value)
		Sys_Printf ("ERROR: %s\n", string);

	if (logs[ERROR_LOG].sv_logfile)
	{
		SV_Write_Log (ERROR_LOG, 1, va ("ERROR: %s\n", string));
//		fclose (logs[ERROR_LOG].sv_logfile);
	}

// FIXME: hack - checking SV_Shutdown with net_socket set in -1 NET_Shutdown
	if (net_socket != -1)
		SV_Shutdown ();

	if ((int)sys_restart_on_error.value)
		Sys_Quit (true);

	Sys_Exit (1);
}

/*
================
Sys_DoubleTime
================
*/
double Sys_DoubleTime (void)
{
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

#if (defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)) && defined(KQUEUE)
static const struct timespec zerotime = { 0, 0 };
static int kq;
static struct kevent kevs[4];
void closesocket_k (int socket)
{
	EV_SET(&kevs[3], (intptr_t) socket, EVFILT_READ, EV_DELETE, 0, 0, 0);
	kevent(kq, &kevs[3], 1, NULL, 0, &zerotime);
	closesocket(socket);
}
#else
#define closesocket_k(socket)	closesocket(socket)
#endif

/*
================
Sys_ConsoleInput
 
Checks for a complete line of text typed in at the console, then forwards
it to the host command processor
================
*/
char *Sys_ConsoleInput (void)
{
	static char text[256], *t;
	static unsigned int len = 0;

	if (!(telnetport && iosock_ready && telnet_connected) && !(do_stdin && stdin_ready))
		return NULL;		// the select didn't say it was ready

	if (telnetport && iosock_ready && telnet_connected)
	{
		iosock_ready = false;
		do
		{
			switch (read (telnet_iosock, text + len, 1))
			{
			case 0:
				len = telnet_connected = authenticated = false;
				closesocket_k (telnet_iosock);
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
				if (qerrno != EAGAIN) // demand for M$ WIN 2K telnet support
				{
					len = telnet_connected = authenticated = false;
					closesocket_k (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, va("Connection closed with error: (%i): %s.\n", qerrno, strerror(qerrno)));
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
				cur_time_auth = Sys_DoubleTime ();
				SV_Write_Log(TELNET_LOG, 1, "Authenticated: yes\n");
				len = 0;
				return "status";
			}
			else
				if (strlen(text) == 1 && text[0] == 4)
				{
					len = telnet_connected = authenticated = false;
					closesocket_k (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
				}
				else
					SV_Write_Log(TELNET_LOG, 1, "Authenticated: no\n");
			len = 0;
			return NULL;
		}

		if (strlen(text) == 1 && text[0] == 4)
		{
			len = telnet_connected = authenticated = false;
			closesocket_k (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
			len = 0;
			return NULL;
		}
		SV_Write_Log(TELNET_LOG, 2, va("%s\n", text));
	}
	if (do_stdin && stdin_ready)
	{
		stdin_ready = false;
		len = read (STDIN_FILENO, text, sizeof(text));
		if (len == 0)
		{	// end of file
			do_stdin = false;
			return NULL;
		}
		if (len < 1)
		{
			len = 0;
			return NULL;
		}
		text[len - 1] = 0;	// rip off the /n and terminate
	}
	len = 0;
	return text;
}

/*
================
Sys_Printf
================
*/
void Sys_Printf (char *fmt, ...)
{
	extern char	chartbl2[];
	va_list		argptr;
	unsigned char	text[4096];
	unsigned char	*p;

	va_start (argptr,fmt);
	vsnprintf((char*)text, sizeof(text), fmt, argptr);
	//	if (vsnprintf(text, sizeof(text), fmt, argptr) >= sizeof(text))
	//        	Sys_Error("memory overwrite in Sys_Printf.\n");
	va_end (argptr);

	if (!(telnetport && telnet_connected && authenticated) && (int)sys_nostdout.value)
		return;

	for (p = text; *p; p++)
	{
		*p = chartbl2[*p];
		if (telnetport && telnet_connected && authenticated)
		{
			if (write (telnet_iosock, p, 1) < 1)
				closesocket(telnet_iosock);

			if (*p == '\n') // demand for M$ WIN 2K telnet support
			{
				if (write (telnet_iosock, "\r", 1) < 1)
					closesocket(telnet_iosock);
			}
		}
		if (!(int)sys_nostdout.value)
			putc(*p, stdout);
	}

	if (telnetport && telnet_connected && authenticated)
		SV_Write_Log(TELNET_LOG, 3, (char*)text);
	if (!(int)sys_nostdout.value)
		fflush(stdout);
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
	Cvar_Register (&sys_nostdout);
	Cvar_Register (&sys_extrasleep);
}

inline void Sys_Telnet (void);
#if (defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)) && defined(KQUEUE)
struct timespec select_timeout;
void Sys_NET_Init()
{
	int i = 0;

	if ((kq = kqueue()) == -1)
		SV_Error("IO_Engine_Init: kqueue() failed");
	EV_SET(&kevs[i++], (intptr_t) net_socket, EVFILT_READ, EV_ADD, 0, 0, 0);

	if (telnetport)
		EV_SET(&kevs[i++], (intptr_t) net_telnetsocket, EVFILT_READ, EV_ADD, 0, 0, 0);

	if (do_stdin)
		EV_SET(&kevs[i++], (intptr_t) STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, 0);

	if (kevent(kq, kevs, i, NULL, 0, &zerotime) == -1)
		SV_Error("IO_Engine_Init: kevent() failed");
}
qbool NET_Sleep ()
{
	struct timespec timeout_cur;
	int num, i, fd;

	if (telnetport)
		Sys_Telnet();

	timeout_cur = select_timeout;

	do
	{
		switch ((num = kevent(kq, NULL, 0, kevs, 4, &timeout_cur)))
		{
			case -1:
				if (errno != EINTR)
				{
					/*SV_Write_Log(SV_ERRORLOG, "IO_Engine_Query: "
						"kevent() failed -- %d, %s\n", errno, strerror(errno));*/
					return true;
				}
				break;
			case 0:
				break;
			default:
				for (i = 0; i < num; i++)
				{
					fd = (int)kevs[i].ident;
					if (do_stdin && fd == STDIN_FILENO)
						stdin_ready = true;
					else if (telnetport && telnet_connected && fd == telnet_iosock)
						iosock_ready = true;
				}
		}
	} while (num == -1);

	return false;
}
#else
struct timeval select_timeout;
#define Sys_NET_Init()
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

	if (do_stdin)
		FD_SET(STDIN_FILENO, &fdset);

	timeout_cur = select_timeout;

	switch (select (++j, &fdset, NULL, NULL, &timeout_cur))
	{
		case -1: return true;
		case 0: break;
		default:
			if (telnetport && telnet_connected)
				iosock_ready = FD_ISSET(telnet_iosock, &fdset);
			if (do_stdin)
				stdin_ready = FD_ISSET(STDIN_FILENO, &fdset);
	}
	return false;
}
#endif

void Sys_Sleep(unsigned long ms)
{
	usleep(ms*1000);
}

int Sys_Script (const char *path, const char *args)
{
	char str[1024];

	snprintf(str, sizeof(str), "cd %s\n./%s.qws %s &\ncd ..", fs_gamedir, path, args);

	if (system(str) == -1)
		return 0;

	return 1;
}

DL_t Sys_DLOpen(const char *path)
{
	return dlopen(path,
#ifdef __OpenBSD__
	              DL_LAZY
#else
	              RTLD_NOW
#endif
	             );
}

qbool Sys_DLClose(DL_t dl)
{
	return !dlclose(dl);
}

void *Sys_DLProc(DL_t dl, const char *name)
{
	return dlsym(dl, name);
}

int  Sys_CreateThread(DWORD (WINAPI *func)(void *), void *param)
{
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);   // ale gowno

    pthread_create(&thread, &attr, (void *)func, param);
    return 1;
}

// Function only_digits was copied from bind (DNS server) sources.
static int only_digits(const char *s)
{
	if (*s == '\0')
		return (0);
	while (*s != '\0')
	{
		if (!isdigit(*s))
			return (0);
		s++;
	}
	return (1);
}

inline void Sys_Telnet (void)
{
	static int			tempsock;
	static struct		sockaddr_in remoteaddr, remoteaddr_temp;
	static socklen_t	sockaddr_len = sizeof(struct sockaddr_in);
	static double		cur_time_not_auth;
	if (telnet_connected)
	{
		if ((tempsock = accept (net_telnetsocket, (struct sockaddr*)&remoteaddr_temp, &sockaddr_len)) > 0)
		{
			//if (remoteaddr_temp.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			send (tempsock, "Console busy by another user.\n", 31, 0);
			closesocket (tempsock);
			SV_Write_Log(TELNET_LOG, 1, va("Console busy by: %s. Refuse connection from: %s\n",
			                               inet_ntoa(remoteaddr.sin_addr), inet_ntoa(remoteaddr_temp.sin_addr)));
		}
		if ((!authenticated && (int)not_auth_timeout.value &&
			 realtime - cur_time_not_auth > not_auth_timeout.value) ||
			(authenticated && (int)auth_timeout.value &&
			 realtime - cur_time_auth > auth_timeout.value))
		{
			authenticated = telnet_connected = false;
			send (telnet_iosock, "Time for authentication finished.\n", 34, 0);
			closesocket_k (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, va("Time for authentication finished. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
			cur_time_auth = cur_time_not_auth = realtime;
		}
	}
	else
	{
		if ((telnet_iosock = accept (net_telnetsocket, (struct sockaddr*)&remoteaddr, &sockaddr_len)) > 0)
		{
			//if (remoteaddr.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			//{
			telnet_connected = true;
			cur_time_not_auth = realtime;
			SV_Write_Log(TELNET_LOG, 1, va("Accept connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
			send (telnet_iosock, "# ", 2, 0);
#if (defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)) && defined(KQUEUE)
			EV_SET(&kevs[3], (intptr_t) telnet_iosock, EVFILT_READ, EV_ADD, 0, 0, 0);
			if (kevent(kq, &kevs[3], 1, NULL, 0, &zerotime) == -1)
				SV_Error("IO_Engine_Init: kevent() failed");
#endif
			/*}
			else
			{
				closesocket (telnet_iosock);
				SV_Write_Log(TELNET_LOG, 1, va("IP not match. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
			}
			*/
		}
	}
}

/*
=============
main
=============
*/
void PR_CleanLogText_Init(void);
int main (int argc, char *argv[])
{
	double time1, oldtime, newtime;
	quakeparms_t parms;

	int j;
	qbool ind;
	uid_t user_id = 0;
	gid_t group_id = 0;
	struct passwd *pw = NULL;
	struct group *gr;
	char *user_name = NULL, *group_name = NULL, *chroot_dir;

// Without signal(SIGPIPE, SIG_IGN); MVDSV crashes on *nix when qtvproxy will be disconnect.
	signal(SIGPIPE, SIG_IGN);

	argv0 = argv[0];

	memset (&parms, 0, sizeof(parms));

	PR_CleanLogText_Init();

	COM_InitArgv (argc, argv);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = DEFAULT_MEM_SIZE;

	j = COM_CheckParm ("-heapsize");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024;

	j = COM_CheckParm("-mem");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024 * 1024;

	parms.membase = Q_malloc (parms.memsize);

	SV_Init (&parms);

// Daemon, chroot, setgid and setuid code (-d, -t, -g, -u)
// was copied from bind (DNS server) sources.
	// daemon
	if (COM_CheckParm ("-d"))
	{
		switch (fork())
		{
		case -1:
			return (-1);
		case 0:
			break;
		default:
			_exit(0);
		}

		if (setsid() == -1)
			Sys_Printf("setsid: %s\n", strerror(qerrno));

		if ((j = open(_PATH_DEVNULL, O_RDWR)) != -1)
		{
			(void)dup2(j, STDIN_FILENO);
			(void)dup2(j, STDOUT_FILENO);
			(void)dup2(j, STDERR_FILENO);
			if (j > 2)
				(void)close(j);
			//isdaemon = true;
			do_stdin = false;
		}
	}

	// setgid
	j = COM_CheckParm ("-g");
	if (j && j + 1 < com_argc)
	{
		ind = true;
		group_name = com_argv[j + 1];
		if (only_digits(group_name))
			group_id = Q_atoi(group_name);
		else
		{
			if (!(gr = getgrnam(group_name)))
			{
				Sys_Printf("WARNING: group \"%s\" unknown\n", group_name);
				ind = false;
			}
			else
				group_id = gr->gr_gid;
		}
		if (ind)
			if (setgid(group_id) < 0)
				Sys_Printf("WARNING: Can't setgid to group \"%s\": %s\n",
							group_name, strerror(qerrno));
	}


	// setuid - only resolve name
	ind = false;
	j = COM_CheckParm ("-u");
	if (j && j + 1 < com_argc)
	{
		ind = true;
		user_name = com_argv[j + 1];
		j = only_digits(user_name);
		if (j)
		{
			user_id = Q_atoi(user_name);
			pw = getpwuid(user_id);
		}
		if (!j || !pw)
		{
			if (!(pw = getpwnam(user_name)))
			{
				if (j)
					Sys_Printf("WARNING: user with uid %u unknown, but we will try to setuid\n",
								(unsigned)user_id);
				else
				{
					Sys_Printf("WARNING: user \"%s\" unknown\n", user_name);
					ind = false;
				}
			}
			else
				user_id = pw->pw_uid;
		}

		if (ind)
		{
			if (pw)
			{
				if (!group_name)
				{
					group_id = pw->pw_gid;
					if (setgid(group_id) < 0)
						Sys_Printf("WARNING: Can't setgid to group \"%s\": %s\n",
									group_name, strerror(qerrno));
				}
				if (!getuid() && initgroups(pw->pw_name, group_id) < 0)
					Sys_Printf("WARNING: Can't initgroups(%s, %d): %s",
								user_name, (unsigned)group_id, strerror(qerrno));
			}
		}
	}

	// chroot
	j = COM_CheckParm ("-t");
	if (j && j + 1 < com_argc)
	{
		chroot_dir = com_argv[j + 1];
		if (chroot(chroot_dir) < 0)
			Sys_Printf("chroot %s failed: %s\n", chroot_dir, strerror(qerrno));
		else
			if (chdir("/") < 0)
				Sys_Printf("chdir(\"/\") to %s failed: %s\n", chroot_dir, strerror(qerrno));
	}

	// setuid - we can't setuid before chroot and
	// can't resolve uid/gid from user/group names after chroot
	if (ind)
	{
		if (setuid(user_id) < 0)
			Sys_Printf("WARNING: Can't setuid to user \"%s\": %s\n",
						user_name, strerror(qerrno));
	}

	// run one frame immediately for first heartbeat
	SV_Frame (0.1);

	// main loop
	oldtime = Sys_DoubleTime () - 0.1;

	Sys_NET_Init();

	while (1)
	{
		// select on the net socket and stdin
		// the only reason we have a timeout at all is so that if the last
		// connected client times out, the message would not otherwise
		// be printed until the next event.
		if (NET_Sleep ())
			continue;

		// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time1 = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time1);

		// extrasleep is just a way to generate a fucked up connection on purpose
		if ((int)sys_extrasleep.value)
			usleep ((unsigned long)sys_extrasleep.value);
	}

	return 0;
}
