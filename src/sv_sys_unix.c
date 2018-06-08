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

#include "qwsvdef.h"

extern cvar_t sys_restart_on_error;
extern cvar_t sys_select_timeout, sys_simulation;

cvar_t sys_nostdout = {"sys_nostdout", "0"};
cvar_t sys_extrasleep = {"sys_extrasleep", "0"};

static qbool	stdin_ready = false;
//static qbool	isdaemon = false;
static qbool	do_stdin = true;

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

int Sys_EnumerateFiles (char *gpath, char *match, int (*func)(char *, int, void *), void *parm)
{
	DIR *dir, *dir2;
	char apath[MAX_OSPATH];
	char file[MAX_OSPATH];
	char truepath[MAX_OSPATH];
	char *s;
	struct dirent *ent;

	//printf("path = %s\n", gpath);
	//printf("match = %s\n", match);

	if (!gpath)
		gpath = "";
	*apath = '\0';

	strncpy(apath, match, sizeof(apath));
	for (s = apath+strlen(apath)-1; s >= apath; s--)
	{
		if (*s == '/')
		{
			s[1] = '\0';
			match += s - apath+1;
			break;
		}
	}
	if (s < apath)  //didn't find a '/'
		*apath = '\0';

	snprintf(truepath, sizeof(truepath), "%s/%s", gpath, apath);


	//printf("truepath = %s\n", truepath);
	//printf("gamepath = %s\n", gpath);
	//printf("apppath = %s\n", apath);
	//printf("match = %s\n", match);
	dir = opendir(truepath);
	if (!dir)
	{
		Con_DPrintf("Failed to open dir %s\n", truepath);
		return true;
	}
	do
	{
		ent = readdir(dir);
		if (!ent)
			break;
		if (*ent->d_name != '.')
			if (wildcmp(match, ent->d_name))
			{
				snprintf(file, sizeof(file), "%s/%s", truepath, ent->d_name);
				//would use stat, but it breaks on fat32.

				if ((dir2 = opendir(file)))
				{
					closedir(dir2);
					snprintf(file, sizeof(file), "%s%s/", apath, ent->d_name);
					//printf("is directory = %s\n", file);
				}
				else
				{
					snprintf(file, sizeof(file), "%s%s", apath, ent->d_name);
					//printf("file = %s\n", file);
				}

				if (!func(file, -2, parm))
				{
					closedir(dir);
					return false;
				}
			}
	} while(1);
	closedir(dir);

	return true;
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
void Sys_Quit (qbool restart)
{
	if (restart)
	{
		int maxfd = getdtablesize() - 1;

		// close all file descriptors besides stdin stdout and stderr, I am not sure, perhaps I can safely close even those descriptors too.
		for (; maxfd > 2; maxfd--)
			close(maxfd);

		if (execv(com_argv[0], com_argv) == -1)
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
	char text[1024];

	sv_error = true;

	if (inerror)
		Sys_Exit (1);

	inerror = true;

	va_start (argptr,error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	if (!(int)sys_nostdout.value)
		Sys_Printf ("ERROR: %s\n", text);

	if (logs[ERROR_LOG].sv_logfile)
	{
		SV_Write_Log (ERROR_LOG, 1, va ("ERROR: %s\n", text));
//		fclose (logs[ERROR_LOG].sv_logfile);
	}

// FIXME: hack - checking SV_Shutdown with net_socket set in -1 NET_Shutdown
	if (svs.socketip != -1)
		SV_Shutdown (va("ERROR: %s\n", text));

	if ((int)sys_restart_on_error.value)
		Sys_Quit (true);

	Sys_Exit (1);
}

/*
================
Sys_DoubleTime
================
*/
#if (_POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK)
#include <time.h>
double Sys_DoubleTime(void)
{
	static unsigned int secbase;
	struct timespec ts;

#ifdef __linux__
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

	if (!secbase) {
		secbase = ts.tv_sec;
		return ts.tv_nsec / 1000000000.0;
	}

	return (ts.tv_sec - secbase) + ts.tv_nsec / 1000000000.0;
}
#else
double Sys_DoubleTime(void)
{
	struct timeval tp;
	struct timezone tzp;
	static int secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase) {
	    secbase = tp.tv_sec;
	    return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}
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
	static char text[256];
	ssize_t len = 0;

	if (!do_stdin || !stdin_ready)
		return NULL; // the select didn't say it was ready

	stdin_ready = false;

	len = read (STDIN_FILENO, text, sizeof(text));

	if (len < 0)
		return NULL; // error.

	if (len == 0)
	{	// end of file
		do_stdin = false;
		return NULL;
	}

	text[len - 1] = 0;	// rip off the /n and terminate

	return text;
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

	va_start (argptr,fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end (argptr);

	if (sys_nostdout.value)
		return;

	// normalize text before add to console.
	Q_normalizetext(text);

	fprintf(stdout, "%s", text);
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

void Sys_Sleep(unsigned long ms)
{
	usleep(ms*1000);
}

int Sys_Script (const char *path, const char *args)
{
	char exec_path[1024];
	char *exec_args[1024];
	char *tmp_args, *p;
	int i;

	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
		return 0;

	switch(fork()) {
		case -1:
			/* oops, we cannot fork */
			return 0;
		case 0:
			break;
		default:
			return 1;
	}

	strlcpy(exec_path, "./", sizeof(exec_path));
	strlcat(exec_path, path, sizeof(exec_path));
	strlcat(exec_path, ".qws", sizeof(exec_path));

	tmp_args = strdup(args);
	if (tmp_args == NULL)
		goto err_exit;

	memset(exec_args, 0, sizeof(exec_args));

	/* translate args into array of arguments for execv(2) call */
	i = 0;
	exec_args[i++] = exec_path;
	for (p = tmp_args; *p;)
	{
		while(*p == ' ') {
			*(p++) = '\0';
		}

		if (*p)
			exec_args[i++] = p;

		/* go to another space or end of string */
		while(*p && *p != ' ')
			p++;
	}

	if (chdir(fs_gamedir) == -1)
		goto err_exit;
	if (execv(exec_path, exec_args) == -1)
		goto err_exit;

err_exit:
	/* indicate error */
	exit(1);
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

int Sys_CreateThread(DWORD (WINAPI *func)(void *), void *param)
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

// Init unix related stuff.
// Daemon, chroot, setgid and setuid code (-d, -t, -g, -u)
// was copied from bind (DNS server) sources.
static void SV_System_Init(void)
{
	int j;
	qbool ind;
	uid_t user_id = 0;
	gid_t group_id = 0;
	struct passwd *pw = NULL;
	struct group *gr;
	char *user_name = NULL, *group_name = NULL, *chroot_dir;

	// daemon
	if (COM_CheckParm ("-d"))
	{
		switch (fork())
		{
		case -1:
			exit(-1); // Error, do normal exit().
		case 0:
			break; // Child, continue process.
		default:
			_exit(0); // Parent, for some reason we prefer here "limited" clean up with _exit().
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
}

/*
=============
main
=============
*/
int main (int argc, char *argv[])
{
	double time1, oldtime, newtime;

// Without signal(SIGPIPE, SIG_IGN); MVDSV crashes on *nix when qtvproxy will be disconnect.
	signal(SIGPIPE, SIG_IGN);

	COM_InitArgv (argc, argv);
	SV_System_Init(); // daemonize and so...
	Host_Init(argc, argv, DEFAULT_MEM_SIZE);

	// run one frame immediately for first heartbeat
	SV_Frame (0.1);

	// main loop
	oldtime = Sys_DoubleTime () - 0.1;

	while (1)
	{
		// select on the net socket and stdin
		// the only reason we have a timeout at all is so that if the last
		// connected client times out, the message would not otherwise
		// be printed until the next event.
		if (!sys_simulation.value) {
			stdin_ready = NET_Sleep((int)sys_select_timeout.value / 1000, do_stdin);
		}

		// find time passed since last cycle
		newtime = Sys_DoubleTime ();
		time1 = newtime - oldtime;
		oldtime = newtime;

		curtime = newtime;
		SV_Frame (time1);

		// extrasleep is just a way to generate a fucked up connection on purpose
		if ((int)sys_extrasleep.value)
			usleep ((unsigned long)sys_extrasleep.value);
	}

	return 0;
}
