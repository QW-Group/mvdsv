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

*/
#include <dirent.h>
#include "qwsvdef.h"

#ifdef NeXT
#include <libc.h>
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(sun) || defined(__GNUC__)
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
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
#include "version.h"
// Added by VVD }

cvar_t	sys_nostdout = {"sys_nostdout","0"};
cvar_t	sys_extrasleep = {"sys_extrasleep","0"};

qboolean isdaemon;
	
static qboolean	stdin_ready = false;
// Added by VVD {
static qboolean	iosock_ready = false;
static int	authenticated = 0;
static double	cur_time_auth;

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
		Sys_Error ("mkdir %s: %s",path, strerror(errno)); 
}

/*
================
Sys_remove
================
*/
int Sys_remove (char *path)
{
	return system(va("rm \"%s\"", path));
}

int Sys_compare(const void *a, const void *b)
{
	return (int)(((file_t *)a)->time - ((file_t *)b)->time);
}
/*
================
Sys_listdir
================
*/

dir_t Sys_listdir (char *path, char *ext, int sort_type)
{
	static file_t list[MAX_DIRFILES];
	dir_t	d;
	int	i;
//	int	extsize;
	DIR	*dir;
	struct dirent *oneentry;
	char	pathname[MAX_OSPATH];
	qboolean all;

	memset(list, 0, sizeof(list));
	memset(&d, 0, sizeof(d));
	d.files = list;
//	extsize = strlen(ext);
	all = !strncmp(ext, ".*", 3);

	dir=opendir(path);
	if (!dir) {
		return d;
	}

	while (1)
	{
		oneentry=readdir(dir);
		if(!oneentry) 
			break;
#if 1
		if (oneentry->d_type == DT_DIR || oneentry->d_type == DT_LNK)
		{
			d.numdirs++;
			continue;
		}
#endif

		snprintf(pathname, MAX_OSPATH, "%s/%s", path, oneentry->d_name);
		list[d.numfiles].size = Sys_FileSize(pathname);
		list[d.numfiles].time = Sys_FileTime(pathname);
		d.size += list[d.numfiles].size;

		i = strlen(oneentry->d_name);
		//if (!all && (i < extsize || (strcasecmp(oneentry->d_name+i-extsize, ext))))
		//	continue;
		if (!all && !strstr(oneentry->d_name, ext))
			continue;

		strlcpy (list[d.numfiles].name, oneentry->d_name, MAX_DEMO_NAME);

		if (++d.numfiles == MAX_DIRFILES - 1)
			break;
	}

	closedir(dir);
	switch (sort_type)
	{
		case SORT_NO: break;
		case SORT_BY_DATE:
			qsort((void *)list, d.numfiles, sizeof(file_t), Sys_compare_by_date);
			break;
		case SORT_BY_NAME:
			qsort((void *)list, d.numfiles, sizeof(file_t), Sys_compare_by_name);
			break;
	}

	return d;
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
	if (sys_nostdout.value)
		printf ("Fatal error: %s\n", string);
	SV_Write_Log(ERROR_LOG, 1, va("Fatal error: %s\n", string));
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
	char		text[4096];
	unsigned char	*p;

	va_start (argptr,fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
//	if (vsnprintf(text, sizeof(text), fmt, argptr) >= sizeof(text))
//        	Sys_Error("memory overwrite in Sys_Printf.\n");
	va_end (argptr);
	
	if (!(telnetport && telnet_connected && authenticated) && sys_nostdout.value)
		return;

	for (p = (unsigned char *)text; *p; p++) {
		if ((*p > 254 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
		{
			if (telnetport && telnet_connected && authenticated)
				write (telnet_iosock, va("[%02x]", *p), strlen (va("[%02x]", *p)));
			if (!sys_nostdout.value)
				fprintf(stdout, "[%02x]", *p);
		}
		else
		{
			if (telnetport && telnet_connected && authenticated)
			{
				write (telnet_iosock, p, 1);
				if (*p == '\n')
					write (telnet_iosock, "\r", 1);
					// demand for M$ WIN 2K telnet support
			}
			if (!sys_nostdout.value)
				putc(*p, stdout);
		}
	}
	if (telnetport && telnet_connected && authenticated)
		SV_Write_Log(TELNET_LOG, 3, text);
	if (!sys_nostdout.value)
		fflush(stdout);
}


/*
================
Sys_Quit
================
*/
void Sys_Quit (void)
{
	exit (0);		// appkit isn't running
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

#ifdef NEWWAY
	fd_set		fdset;
	struct timeval	timeout;
	int		max = 1;
	if (!(telnetport && telnet_connected) && !do_stdin)
		return NULL;

	FD_ZERO(&fdset);
	if (telnetport && telnet_connected)
	{
		FD_SET(telnet_iosock, &fdset);
		max += telnet_iosock;
	}
	if (do_stdin)
		FD_SET(0, &fdset); // stdin

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if (select (max + 1, &fdset, NULL, NULL, &timeout) == -1)
		return NULL;

	if (do_stdin)
		stdin_ready = FD_ISSET(0, &fdset);
	if (telnetport && telnet_connected)
		iosock_ready = FD_ISSET(telnet_iosock, &fdset);
#endif

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
	if (do_stdin && stdin_ready && !isdaemon)
	{
		stdin_ready = false;
		len = read (0, text, sizeof(text));
		if (len == 0) {	// end of file
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

static int only_digits(const char *s) {
	if (*s == '\0')
		return (0);
	while (*s != '\0') {
		if (!isdigit(*s))
			return (0);
		s++;
	}
	return (1);
}


/*
=============
main
=============
*/

int main (int argc, char *argv[])
{
	double		time, oldtime, newtime;
	quakeparms_t	parms;

//Added by VVD {
	int	j, tempsock;
	struct	sockaddr_in remoteaddr, remoteaddr_temp;
	int	sockaddr_len = sizeof(struct sockaddr_in);
	double	cur_time_not_auth = 0, not_auth_timeout_value, auth_timeout_value;
	uid_t	user_id;
	gid_t	group_id;
	struct passwd	*pw;
	struct group	*gr;
	char	*user_name, *group_name, *chroot_dir;
//Added by VVD }
#ifndef NEWWAY
	struct timeval timeout;
	fd_set	fdset;
#endif
	telnet_connected = 0;
	memset (&parms, 0, sizeof(parms));

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

	j = COM_CheckParm ("-d");
	if (j && j < com_argc)
	{
		if (daemon(0, 1))
		{
			Sys_Printf("daemon: %s\n", strerror(errno));
			isdaemon = 0;
		}
		else
			isdaemon = 1;
	}
	else 
		isdaemon = 0;

	parms.membase = Q_Malloc (parms.memsize);

	parms.basedir = ".";

// chroot
	j = COM_CheckParm ("-c");
	if (j && j + 1 < com_argc)
	{
		chroot_dir = com_argv[j + 1];
		if (chroot(chroot_dir) < 0) {
			Sys_Printf("chroot %s failed: %s\n", chroot_dir, strerror(errno));
			//exit(1);
		}
		if (chdir("/") < 0) {
			Sys_Printf("chdir(\"/\") to %s failed: %s\n", chroot_dir, strerror(errno));
			//exit(1);
		}
	}

	SV_Init (&parms);

// set[e]uid, set[e]gid
	j = COM_CheckParm ("-u");
	if (j && j + 1 < com_argc)
	{
		user_name = com_argv[j + 1];
		if (only_digits(user_name))
			user_id = Q_atoi(user_name);
		else {
			pw = getpwnam(user_name);
			if (pw == NULL) {
				Sys_Printf("user \"%s\" unknown\n", user_name);
				//exit(1);
			}
			user_id = pw->pw_uid;
		}
		if (setuid(user_id) < 0)
			Sys_Printf("Can't setuid to user \"%s\": %s\n", user_name, strerror(errno));
	}

	j = COM_CheckParm ("-g");
	if (j && j + 1 < com_argc)
	{
		group_name = com_argv[j + 1];
		if (only_digits(group_name))
			group_id = Q_atoi(group_name);
		else {
			gr = getgrnam(group_name);
			if (gr == NULL) {
				Sys_Printf("group \"%s\" unknown\n", group_name);
				//exit(1);
			}
			group_id = gr->gr_gid;
		}
		if (setgid(group_id) < 0)
			Sys_Printf("Can't setgid to group \"%s\": %s\n", group_name, strerror(errno));
	}

// run one frame immediately for first heartbeat
	SV_Frame (0.1);		

//
// main loop
//
	oldtime = Sys_DoubleTime () - 0.1;
#ifndef NEWWAY 
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
			if (telnet_connected)
			{
				if ((tempsock =
				accept (net_telnetsocket, (struct sockaddr*)&remoteaddr_temp, &sockaddr_len)) > 0)
				{
//					if (remoteaddr_temp.sin_addr.s_addr == inet_addr ("127.0.0.1"))
						write (tempsock, "Console busy by another user.\n", 31);
					close (tempsock);
					SV_Write_Log(TELNET_LOG, 1, va("Console busy by: %s. Refuse connection from: %s\n",
						inet_ntoa(remoteaddr.sin_addr), inet_ntoa(remoteaddr_temp.sin_addr)));
				}
				not_auth_timeout_value = Cvar_VariableValue("not_auth_timeout");
				auth_timeout_value = Cvar_VariableValue("auth_timeout");
				if ((!authenticated && not_auth_timeout_value &&
					Sys_DoubleTime () - cur_time_not_auth > not_auth_timeout_value) ||
					(authenticated && auth_timeout_value &&
					Sys_DoubleTime () - cur_time_auth > auth_timeout_value))
				{
					telnet_connected = 0;
					write (telnet_iosock, "Time for authentication finished.\n", 34);
					close(telnet_iosock);
					SV_Write_Log(TELNET_LOG, 1, va("Time for authentication finished. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
				}
			}
			else
			{
				if ((telnet_iosock =
				accept (net_telnetsocket, (struct sockaddr*)&remoteaddr, &sockaddr_len)) > 0)
				{
//					if (remoteaddr.sin_addr.s_addr == inet_addr ("127.0.0.1"))
//					{
						telnet_connected = 1;
						cur_time_not_auth = Sys_DoubleTime ();
						SV_Write_Log(TELNET_LOG, 1, va("Accept connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
						write (telnet_iosock, "# ", 2);
/*					}
					else
					{
						close (telnet_iosock);
						SV_Write_Log(TELNET_LOG, 1, va("IP not match. Refuse connection from: %s\n", inet_ntoa(remoteaddr.sin_addr)));
					}
*/
				}
			}
			FD_SET(net_telnetsocket, &fdset);
			j = max(j, net_telnetsocket);
			if (telnet_connected)
			{
				FD_SET(telnet_iosock, &fdset);
				j = max(j, telnet_iosock);
			}
		}
// Added by VVD }
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;

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
#else
	while (1)
	{
		do
		{
			newtime = Sys_DoubleTime ();
			time = newtime - oldtime;
		} while (time < 0.001);

		SV_Frame (time);
		oldtime = newtime;

		// extrasleep is just a way to generate a fucked up connection on purpose
		if (sys_extrasleep.value)
			usleep (sys_extrasleep.value);
	}
#endif
	return 1;
}

