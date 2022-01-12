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

#ifdef _WIN32

static HANDLE	hQizmoProcess = NULL;
static DWORD ExitCode;

static qbool QizmoRunning ()
{
	if (sworld.options & O_SHUTDOWN)
		Sys_Exit(2);

	if (!hQizmoProcess)
		return false;

	if (!GetExitCodeProcess (hQizmoProcess, &ExitCode))
	{
		Sys_Printf ("WARINING: GetExitCodeProcess failed\n");
		hQizmoProcess = NULL;
		return false;
	}

	if (ExitCode == STILL_ACTIVE)
		return true;

	hQizmoProcess = NULL;

	return false;
}
#define QIZMO_BIN "qizmo.exe"
#else
#define QIZMO_BIN "qizmo"
#endif

void StopQWZ (source_t *s)
{
	int num;

	num = s - sources;

	if (s->qwz && sworld.from[num].file)
	{
		Sys_fclose(&sworld.from[num].file);
		s->qwz = false;
		if (remove (sworld.from[num].name) != 0)
			Sys_Printf ("Couldn't delete %s\n", sworld.from[num].name);
		else Sys_Printf (" deleted %s\n", sworld.from[num].name);
	}
}


qbool OpenQWZ (char *files)
{
#ifdef _WIN32
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;
#endif
	char	cmdline[2048];
	char	curdir[MAX_OSPATH];
	char	*p;

#ifdef _WIN32
	if (hQizmoProcess)
		return false;
#endif

	Sys_Printf ("decompressing qwz file(s)...\n");

#ifdef _WIN32
	// start Qizmo to unpack the demo
	memset (&si, 0, sizeof(si));
	si.cb = sizeof(si);
#endif

	p = qizmoDir + strlen(qizmoDir) - 1;
	if (p >= qizmoDir && (*p == '\\' || *p == '/'))
		*p = 0;

	if (!qizmoDir[0])
		strlcpy(curdir, currentDir, sizeof(curdir));
	else
	{
		if (strstr(qizmoDir, ":") != NULL)
			snprintf(curdir, sizeof(curdir), "%s",  qizmoDir);
		else
			snprintf(curdir, sizeof(curdir), "%s/%s", currentDir, qizmoDir);
	}

	strlcpy (cmdline, va("%s/" QIZMO_BIN "-D %s", curdir,
	                     files), sizeof(cmdline));

#ifdef _WIN32
	if (!CreateProcess (NULL, cmdline, NULL, NULL,
	                    FALSE, 0, NULL, curdir, &si, &pi))
#else
	if (system(cmdline))
#endif
	{
		Sys_Printf ("Couldn't execute %s/" QIZMO_BIN "\n",
		            curdir);
		return false;
	}
#ifdef _WIN32
	hQizmoProcess = pi.hProcess;

	SetConsoleTitle("qwdtools  decompressing...");

	while (QizmoRunning());

	if (ExitCode == TIME_ZONE_ID_INVALID)
	{
		Sys_Printf("Error running qizmo\n");
		return false;
	}
#endif

	Sys_Printf ("\ndecompressing finished\n");

	return true;
}
