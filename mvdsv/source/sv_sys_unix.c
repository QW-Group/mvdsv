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

#if defined(__linux__) || defined(sun) || defined(__GNUC__)
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#else
#include <sys/dir.h>
#endif
#include <time.h>

cvar_t	sys_nostdout = {"sys_nostdout","0"};
cvar_t	sys_extrasleep = {"sys_extrasleep","0"};

qboolean	stdin_ready;

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

/*
================
Sys_listdir
================
*/

dir_t Sys_listdir (char *path, char *ext)
{
	static file_t list[MAX_DIRFILES];
	dir_t	d;
	int		i, extsize;
	DIR		*dir;
    struct dirent *oneentry;
	char	pathname[MAX_OSPATH];
	qboolean all;

	memset(list, 0, sizeof(list));
	memset(&d, 0, sizeof(d));
	d.files = list;
	extsize = strlen(ext);
	all = !strcmp(ext, ".*");

	dir=opendir(path);
	if (!dir) {
		return d;
	}

	for(;;)
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

		sprintf(pathname, "%s/%s", path, oneentry->d_name);
		list[d.numfiles].size = Sys_FileSize(pathname);
		d.size += list[d.numfiles].size;

		i = strlen(oneentry->d_name);
		//if (!all && (i < extsize || (Q_strcasecmp(oneentry->d_name+i-extsize, ext))))
		//	continue;
		if (!all && !strstr(oneentry->d_name, ext))
			continue;

		Q_strncpyz (list[d.numfiles].name, oneentry->d_name, MAX_DEMO_NAME);


		if (++d.numfiles == MAX_DIRFILES - 1)
			break;
	}

	closedir(dir);

	return d;
}

void Sys_TimeOfDay(date_t *date)
{
	struct tm *newtime;
	time_t long_time;
	
	time( &long_time );
	newtime = localtime( &long_time );

	date->day = newtime->tm_mday;
	date->mon = newtime->tm_mon;
	date->year = newtime->tm_year + 1900;
	date->hour = newtime->tm_hour;
	date->min = newtime->tm_min;
	date->sec = newtime->tm_sec;
	strftime( date->str, 128,
         "%a %b %d, %H:%M %Y", newtime);
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
	vsprintf (string,error,argptr);
	va_end (argptr);
	printf ("Fatal error: %s\n",string);
	if (sv_errorlogfile)
		fprintf(sv_errorlogfile, "Fatal error: %s\n", string);
	
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
	static char		text[4096];
	unsigned char		*p;

	va_start (argptr,fmt);
	vsprintf (text,fmt,argptr);
	va_end (argptr);

	if (strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

    if (sys_nostdout.value)
        return;

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
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
	static char	text[256];
	int		len;
#ifdef NEWWAY
	fd_set	fdset;
    struct timeval timeout;

	if (!do_stdin)
		return;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset); // stdin
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select (1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;
#else

	if (!stdin_ready || !do_stdin)
		return NULL;		// the select didn't say it was ready
	stdin_ready = false;
#endif

	len = read (0, text, sizeof(text));
	if (len == 0) {
		// end of file
		do_stdin = 0;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len-1] = 0;	// rip off the /n and terminate
	
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
    struct timeval timeout;
	fd_set	fdset;
	extern	int		net_socket;

	FD_ZERO(&fdset);
	if (do_stdin)
		FD_SET(0, &fdset); // stdin is processed too
	FD_SET(net_socket, &fdset); // network socket

	timeout.tv_sec = (long) sec;
	timeout.tv_usec = (sec - floor(sec))*1000000L;
	return select(net_socket+1, &fdset, NULL, NULL, &timeout);
}

void Sys_Sleep(unsigned long ms)
{
	usleep(ms*1000);
}

int Sys_Script(char *path, char *args)
{
	char str[1024];
	
	sprintf(str,"cd %s\n./%s.qws %s &\ncd ..", com_gamedir, path, args);
	
	if (system(str) == -1)
		return 0;
	
	return 1;
}



/*
=============
main
=============
*/

int main (int argc, char *argv[])
{
	double			time, oldtime, newtime;
	quakeparms_t	parms;
	extern	int		net_socket;
#ifndef NEWWAY
    struct timeval timeout;
	fd_set	fdset;
#endif
	int j;

	memset (&parms, 0, sizeof(parms));

	COM_InitArgv (argc, argv);	
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = 16*1024*1024;

	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
	parms.membase = Q_Malloc (parms.memsize);

	parms.basedir = ".";

	SV_Init (&parms);

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
		if (do_stdin)
			FD_SET(0, &fdset);
		FD_SET(net_socket, &fdset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		if (select (net_socket+1, &fdset, NULL, NULL, &timeout) == -1)
			continue;
		stdin_ready = FD_ISSET(0, &fdset);

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

