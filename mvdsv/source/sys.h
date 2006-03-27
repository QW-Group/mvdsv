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

	$Id: sys.h,v 1.9 2006/03/27 22:54:39 disconn3ct Exp $
*/
// sys.h -- non-portable functions


//
// file IO
//

#ifndef _SYS_H_
#define _SYS_H_

// returns the file size
// return -1 if file is not present
// the file should be in BINARY mode for stupid OSs that care
#define MAX_DIRFILES 4096
#define MAX_DEMO_NAME 64

typedef struct
{
	char	name[MAX_DEMO_NAME];
	int	size;
	int	time;
	qbool	isdir; //bliP: list dir
} file_t;

typedef struct
{
	file_t *files;
	int	size;
	int	numfiles;
	int	numdirs;
} dir_t;

int Sys_FileTime (char *path);
void Sys_mkdir (char *path);
int Sys_rmdir (char *path);
int Sys_remove (char *path);
dir_t Sys_listdir (char *path, char *ext, int sort_type);
int Sys_compare_by_date(const void *a, const void *b);
int Sys_compare_by_name(const void *a, const void *b);
#define SORT_NO		0
#define SORT_BY_DATE	1
#define SORT_BY_NAME	2


//
// system IO
//
void Sys_Error (char *error, ...);
// an error will cause the entire program to exit

void Sys_Printf (char *fmt, ...);
// send text to the console

void Sys_Quit (qbool restart);

double Sys_DoubleTime (void);

char *Sys_ConsoleInput (void);

//void Sys_Sleep (void);
// called to yield for a little bit so as
// not to hog cpu when paused or debugging

void Sys_Init (void);

void Sys_Sleep (unsigned long ms);

int Sys_Script(char *path, char *args);

#ifdef _WIN32
#include "winsock2.h"
typedef HMODULE DL_t;
#define DLEXT "dll"
#else
typedef void *DL_t;
#define DLEXT "so"
#endif

DL_t Sys_DLOpen(const char *path);
qbool Sys_DLClose(DL_t dl);
void *Sys_DLProc(DL_t dl, const char *name);

#endif // _SYS_H_
