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
 
	$Id: sv_sys_unix.c,v 1.15 2006/02/15 17:54:34 vvd0 Exp $
*/

#include <dirent.h>
#include <dlfcn.h>
#include "qwsvdef.h"

#ifdef NeXT
#include <libc.h>
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(sun) || defined(__GNUC__) || defined(__APPLE__)
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "pcre/pcre.h"

#else
#include <sys/dir.h>
#endif
#include <time.h>

// Added by VVD {
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#ifdef sun
#define _PATH_DEVNULL "/dev/null"
#else
#include <paths.h>
#endif

#include "version.h"
// Added by VVD }

extern cvar_t sys_select_timeout;
extern cvar_t sys_restart_on_error;
extern cvar_t not_auth_timeout;
extern cvar_t auth_timeout;

cvar_t	sys_nostdout = {"sys_nostdout", "0"};
cvar_t	sys_extrasleep = {"sys_extrasleep", "0"};

static qboolean	stdin_ready = false;
// Added by VVD {
static qboolean	iosock_ready = false;
static int	authenticated = 0;
static double	cur_time_auth;
static qboolean	isdaemon = 0;
// Added by VVD }

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
int	Sys_FileTime (char *path)
{
	struct	stat	buf;

	if (stat (path,&buf) == -1)
		return -1;

	return buf.st_mtime;
}

int	Sys_FileSize (char *path)
{
	struct	stat	buf;

	if (stat (path,&buf) == -1)
		return 0;

	return buf.st_size;
}


/*
============
Sys_mkdir
 
============
*/
void Sys_mkdir (char *path)
{
	if (mkdir (path, 0777) != -1)
		return;
	if (errno != EEXIST)
		Sys_Error ("mkdir %s: %s", path, strerror(errno));
}

/*
================
Sys_remove
================
*/
int Sys_remove (char *path)
{
	return unlink(path);
}

//bliP: rmdir ->
/*
================
Sys_rmdir
================
*/
int Sys_rmdir (char *path)
{
	return rmdir(path);
}
//<-

/*
================
Sys_listdir
================
*/

dir_t Sys_listdir (char *path, char *ext, int sort_type)
{
	static file_t list[MAX_DIRFILES];
	dir_t	dir;
	char	pathname[MAX_OSPATH];
	DIR	*d;
	DIR	*testdir; //bliP: list dir
	struct dirent *oneentry;
	qboolean all;

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
			Q_Free(preg);
			return dir;
		}

	if (!(d = opendir(path)))
	{
		if (!all)
			Q_Free(preg);
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
				Q_Free(preg);
				return dir;
			}
		}
		snprintf(pathname, sizeof(pathname), "%s/%s", path, oneentry->d_name);
		if ((testdir = opendir(pathname)))
		{
			dir.numdirs++;
			list[dir.numfiles].isdir = true;
			list[dir.numfiles].size = 0;
			list[dir.numfiles].time = 0;
			closedir(testdir);
		}
		else
		{
			list[dir.numfiles].isdir = false;
			list[dir.numfiles].time = Sys_FileTime(pathname);
			dir.size += (list[dir.numfiles].size = Sys_FileSize(pathname));
		}
		strlcpy (list[dir.numfiles].name, oneentry->d_name, MAX_DEMO_NAME);

		if (++dir.numfiles == MAX_DIRFILES - 1)
			break;
	}
	closedir(d);
	if (!all)
		Q_Free(preg);

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
void Sys_Quit (qboolean restart)
{
	if (restart)
		if (execv(argv0, com_argv) == -1)
		{
			Sys_Printf("Restart failed: %s\n", strerror(errno));
			Sys_Exit(1);
		}
	Sys_Exit(0);		// appkit isn't running
}

/*
================
Sys_Error
================
*/
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		string[1024];

	va_start (argptr,error);
	vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	if (!sys_nostdout.value)
		printf ("Fatal error: %s\n", string);
	SV_Write_Log(ERROR_LOG, 1, va("Fatal error: %s\n", string));
	if (sys_restart_on_error.value)
		Sys_Quit(true);
	Sys_Exit(1);
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

static int do_stdin = 1;

/*
================
Sys_ConsoleInput
 
Checks for a complete line of text typed in at the console, then forwards
it to the host command processor
================
*/
char *Sys_ConsoleInput (void)
{
	static char	text[256], *t;
	static int	len = 0;


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
				len = telnet_connected = authenticated = 0;
				close (telnet_iosock);
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
				if (errno != EAGAIN) // demand for M$ WIN 2K telnet support
				{
					len = telnet_connected = authenticated = 0;
					close (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, va("Connection closed with error: %s.\n", strerror(errno)));
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
			len = strlen(t = Cvar_VariableString ("telnet_password"));
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
					len = telnet_connected = authenticated = 0;
					close (telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
				}
				else
					SV_Write_Log(TELNET_LOG, 1, "Authenticated: no\n");
			len = 0;
			return NULL;
		}

		if (strlen(text) == 1  && text[0] == 4)
		{
			len = telnet_connected = authenticated = 0;
			close (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, "Connection closed by user.\n");
			len = 0;
			return NULL;
		}
		len = 0;
		SV_Write_Log(TELNET_LOG, 2, va("%s\n", text));
	}
	if (do_stdin && stdin_ready)
	{
		stdin_ready = false;
		len = read (0, text, sizeof(text));
		if (len == 0)
		{	// end of file
			do_stdin = 0;
			return NULL;
		}
		if (len < 1)
			return NULL;
		text[len - 1] = 0;	// rip off the /n and terminate
	}
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
	vsnprintf(text, sizeof(text), fmt, argptr);
	//	if (vsnprintf(text, sizeof(text), fmt, argptr) >= sizeof(text))
	//        	Sys_Error("memory overwrite in Sys_Printf.\n");
	va_end (argptr);

	if (!(telnetport && telnet_connected && authenticated) && sys_nostdout.value)
		return;

	for (p = text; *p; p++)
	{
		*p = chartbl2[*p];
		if (telnetport && telnet_connected && authenticated)
		{
			write (telnet_iosock, p, 1);
			if (*p == '\n') // demand for M$ WIN 2K telnet support
				write (telnet_iosock, "\r", 1);
		}
		if (!sys_nostdout.value)
			putc(*p, stdout);
	}

	if (telnetport && telnet_connected && authenticated)
		SV_Write_Log(TELNET_LOG, 3, text);
	if (!sys_nostdout.value)
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
	Cvar_RegisterVariable (&sys_nostdout);
	Cvar_RegisterVariable (&sys_extrasleep);
}

int NET_Sleep(double sec)
{
	struct timeval	timeout;
	fd_set		fdset;
	int		ret;
	int		m;

	FD_ZERO(&fdset);
	FD_SET(m = net_serversocket, &fdset); // network socket
	if (telnetport)
	{
		FD_SET(net_telnetsocket, &fdset);
		m = max(m, net_telnetsocket);
		if (telnet_connected)
		{
			FD_SET(telnet_iosock, &fdset);
			m = max(m, telnet_iosock);
		}
	}
	if (do_stdin)
		FD_SET(0, &fdset); // stdin is processed too

	timeout.tv_sec = (long) sec;
	timeout.tv_usec = (sec - floor(sec))*1000000L;

	ret = select (m + 1, &fdset, NULL, NULL, &timeout);

	if (telnetport && telnet_connected)
		iosock_ready = FD_ISSET(telnet_iosock, &fdset);

	if (do_stdin)
		stdin_ready = FD_ISSET(0, &fdset);

	return ret;
}

void Sys_Sleep(unsigned long ms)
{
	usleep(ms*1000);
}

int Sys_Script(char *path, char *args)
{
	char str[1024];

	snprintf(str, sizeof(str), "cd %s\n./%s.qws %s &\ncd ..", com_gamedir, path, args);

	if (system(str) == -1)
		return 0;

	return 1;
}

DL_t Sys_DLOpen(const char *path)
{
	return dlopen(path,
#ifdef OS_OPENBSD
	              DL_LAZY
#else
	              RTLD_NOW
#endif
	             );
}

qboolean Sys_DLClose(DL_t dl)
{
	return !dlclose(dl);
}

void *Sys_DLProc(DL_t dl, const char *name)
{
	return dlsym(dl, name);
}


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

#define closesocket close
inline void Sys_Telnet (void)
{
	static int			tempsock;
	static struct		sockaddr_in remoteaddr, remoteaddr_temp;
	static int			sockaddr_len = sizeof(struct sockaddr_in);
	static double		cur_time_not_auth;
	if (telnet_connected)
	{
		if ((tempsock =
		            accept (net_telnetsocket, (struct sockaddr*)&remoteaddr_temp, &sockaddr_len)) > 0)
		{
			//			if (remoteaddr_temp.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			send (tempsock, "Console busy by another user.\n", 31, 0);
			closesocket (tempsock);
			SV_Write_Log(TELNET_LOG, 1, va("Console busy by: %s. Refuse connection from: %s\n",
			                               inet_ntoa(remoteaddr.sin_addr), inet_ntoa(remoteaddr_temp.sin_addr)));
		}
		if (	(!authenticated && not_auth_timeout.value &&
		        realtime - cur_time_not_auth > not_auth_timeout.value) ||
		        (authenticated && auth_timeout.value &&
		         realtime - cur_time_auth > auth_timeout.value))
		{
			telnet_connected = 0;
			send (telnet_iosock, "Time for authentication finished.\n", 34, 0);
			closesocket (telnet_iosock);
			SV_Write_Log(TELNET_LOG, 1, va("Time for authentication finished. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
		}
	}
	else
	{
		if ((telnet_iosock =
		            accept (net_telnetsocket, (struct sockaddr*)&remoteaddr, &sockaddr_len)) > 0)
		{
			//			if (remoteaddr.sin_addr.s_addr == inet_addr ("127.0.0.1"))
			//			{
			telnet_connected = 1;
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

/*
=============
main
=============
*/
void PR_CleanLogText_Init(void);
int main (int argc, char *argv[])
{
	double		time, oldtime, newtime;
	quakeparms_t	parms;

	//Added by VVD {
	int	j;
	uid_t   user_id;
	gid_t   group_id;
	struct passwd	*pw;
	struct group	*gr;
	char	*user_name, *group_name = NULL, *chroot_dir;
	//Added by VVD }

	struct timeval timeout;
	fd_set	fdset;

	argv0 = argv[0];
	telnet_connected = 0;

	memset (&parms, 0, sizeof(parms));

	PR_CleanLogText_Init();

	COM_InitArgv (argc, argv);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = 16*1024*1024;

	j = COM_CheckParm ("-heapsize");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024;

	j = COM_CheckParm("-mem");
	if (j && j + 1 < com_argc)
		parms.memsize = Q_atoi (com_argv[j + 1]) * 1024 * 1024;

	parms.membase = Q_Malloc (parms.memsize);

	SV_Init (&parms);

	// daemon
	j = COM_CheckParm ("-d");
	if (j && j < com_argc)
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
			Sys_Printf("setsid: %s\n", strerror(errno));

		if ((j = open(_PATH_DEVNULL, O_RDWR)) != -1)
		{
			(void)dup2(j, STDIN_FILENO);
			(void)dup2(j, STDOUT_FILENO);
			(void)dup2(j, STDERR_FILENO);
			if (j > 2)
				(void)close(j);
			isdaemon = 1;
		}
	}

	// chroot
	j = COM_CheckParm ("-t");
	if (j && j + 1 < com_argc)
	{
		chroot_dir = com_argv[j + 1];
		if (chroot(chroot_dir) < 0)
			Sys_Printf("chroot %s failed: %s\n", chroot_dir, strerror(errno));
		else
			if (chdir("/") < 0)
				Sys_Printf("chdir(\"/\") to %s failed: %s\n", chroot_dir, strerror(errno));
	}

	// setgid
	j = COM_CheckParm ("-g");
	if (j && j + 1 < com_argc)
	{
		group_name = com_argv[j + 1];
		if (only_digits(group_name))
			group_id = Q_atoi(group_name);
		else
		{
			if ((gr = getgrnam(group_name)) == NULL)
				Sys_Printf("group \"%s\" unknown\n", group_name);
			group_id = gr->gr_gid;
		}
		if (setgid(group_id) < 0)
			Sys_Printf("Can't setgid to group \"%s\": %s\n", group_name, strerror(errno));
	}
	// setuid
	j = COM_CheckParm ("-u");
	if (j && j + 1 < com_argc)
	{
		user_name = com_argv[j + 1];
		if (only_digits(user_name))
			user_id = Q_atoi(user_name);
		else
		{
			pw = getpwnam(user_name);
			if (pw == NULL)
				Sys_Printf("user \"%s\" unknown\n", user_name);
			else
			{
				user_id = pw->pw_uid;
				if (!group_name)
				{
					group_id = pw->pw_gid;
					if (setgid(group_id) < 0)
						Sys_Printf("Can't setgid to group \"%s\": %s\n", group_name, strerror(errno));
				}
				if (!getuid() && initgroups(user_name, group_id) < 0)
					Sys_Printf("Can't initgroups(%s, %d): %s", user_name, (int)group_id, strerror(errno));
				if (setuid(user_id) < 0)
					Sys_Printf("Can't setuid to user \"%s\": %s\n", user_name, strerror(errno));
			}
		}
	}

	// run one frame immediately for first heartbeat
	SV_Frame (0.1);

	//
	// main loop
	//
	oldtime = Sys_DoubleTime () - 0.1;

	while (1)
	{
		// select on the net socket and stdin
		// the only reason we have a timeout at all is so that if the last
		// connected client times out, the message would not otherwise
		// be printed until the next event.
		FD_ZERO(&fdset);
		FD_SET(j = net_serversocket, &fdset);
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
		timeout.tv_sec  = ((int)sys_select_timeout.value) / 1000000;
		timeout.tv_usec = ((int)sys_select_timeout.value) - timeout.tv_sec;

		if (do_stdin)
			FD_SET(0, &fdset);

		switch (select (j + 1, &fdset, NULL, NULL, &timeout))
		{
		case -1: continue;
		case 0: break;
		default:
			if (do_stdin)
				stdin_ready = FD_ISSET(0, &fdset);
			if (telnetport && telnet_connected)
				iosock_ready = FD_ISSET(telnet_iosock, &fdset);
		}

		// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;
		oldtime = newtime;

		SV_Frame (time);

		// extrasleep is just a way to generate a fucked up connection on purpose
		if (sys_extrasleep.value)
			usleep (sys_extrasleep.value);
	}

	return 1;
}
