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
#include <io.h>

cvar_t sys_nostdout = {"sys_nostdout", "0"};
cvar_t sys_sleep = {"sys_sleep", "8"};

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

static dir_t listdir_internal(const char *path, const char *ext,
	int sort_type, qbool read_metadata)
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
	dir.is_metadata_valid = read_metadata;
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
			if (read_metadata)
			{
				snprintf(pathname, sizeof(pathname), "%s/%s", path, fd.cFileName);
				list[dir.numfiles].time = Sys_FileTime(pathname);
				dir.size += (list[dir.numfiles].size = fd.nFileSizeLow);
			}
		}
		strlcpy (list[dir.numfiles].name, fd.cFileName, sizeof(list[0].name));

		if (dir.numfiles != MAX_DIRFILES - 1) dir.numfiles++;

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

dir_t Sys_listdir (const char *path, const char *ext, int sort_type)
{
	return listdir_internal(path, ext, sort_type, true);
}

dir_t Sys_listdir_no_metadata (const char *path, const char *ext)
{
	return listdir_internal(path, ext, SORT_NO, false);
}

int Sys_EnumerateFiles (char *gpath, char *match, int (*func)(char *, int, void *), void *parm)
{
	HANDLE r;
	WIN32_FIND_DATA fd;
	char apath[MAX_OSPATH];
	char apath2[MAX_OSPATH];
	char file[MAX_OSPATH];
	char *s;
	int go;
	if (!gpath)
		return 0;

	snprintf(apath, sizeof(apath), "%s/%s", gpath, match);
	for (s = apath+strlen(apath)-1; s> apath; s--)
	{
		if (*s == '/')
			break;
	}
	*s = '\0';

	// This is what we ask windows for.
	snprintf(file, sizeof(file), "%s/*.*", apath);

	// We need to make apath contain the path in match but not gpath
	strlcpy(apath2, match, sizeof(apath));
	match = s+1;
	for (s = apath2+strlen(apath2)-1; s> apath2; s--)
	{
		if (*s == '/')
			break;
	}
	*s = '\0';
	if (s != apath2)
		strlcat (apath2, "/", sizeof (apath2));

	r = FindFirstFile(file, &fd);
	if (r==(HANDLE)-1)
		return 1;
	go = true;
	do
	{
		if (*fd.cFileName == '.');  // Don't ever find files with a name starting with '.'
		else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    //is a directory
		{
			if (wildcmp(match, fd.cFileName))
			{
				snprintf(file, sizeof(file), "%s%s/", apath2, fd.cFileName);
				go = func(file, fd.nFileSizeLow, parm);
			}
		}
		else
		{
			if (wildcmp(match, fd.cFileName))
			{
				snprintf(file, sizeof(file), "%s%s", apath2, fd.cFileName);
				go = func(file, fd.nFileSizeLow, parm);
			}
		}
	}
	while(FindNextFile(r, &fd) && go);
	FindClose(r);

	return go;
}

uint64_t Sys_TimestampMilliseconds(void)
{
	FILETIME ft;
	ULARGE_INTEGER ticks;

	GetSystemTimeAsFileTime(&ft);

	ticks.LowPart = ft.dwLowDateTime;
	ticks.HighPart = ft.dwHighDateTime;

	return (uint64_t)((ticks.QuadPart - 116444736000000000ULL) / 10000ULL);
}

void Sys_Sleep (unsigned long ms)
{
	Sleep (ms);
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
        0,                  // creation flags
        &threadid);         // pointer to receive thread ID

    if (thread == NULL)
        return (int)GetLastError();

    CloseHandle(thread);

    return 0;
}

char *Sys_GetModuleName(void)
{
	static char	exename[1024] = {0}; // static
	int i;

	if (exename[0])
		return exename;

	if(!(i = GetModuleFileName(NULL, exename, sizeof(exename)-1))) // here we get loong string, with full path
	{
		exename[0] = 0; // oh, something bad
	}
	else
	{
		exename[i] = 0; // ensure null terminator
	}

	return exename;
}
