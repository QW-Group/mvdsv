/*
	build.c
 
	Build number and version strings
 
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
	along with this program; if not, write to:
 
		 Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA
 
*/

#include "qwsvdef.h"

#ifndef BUILD_NUMBER
#define BUILD_NUMBER "0"
#endif

/*
=======================
VersionString
======================
*/
char *VersionString (void)
{
	return VERSION_NUMBER ", (build " BUILD_NUMBER ")";
}

/*
=======================
VersionStringFull
======================
*/
char *VersionStringFull (void)
{
	static char str[256];

	if (!str[0]) {
		snprintf(str, sizeof(str), SERVER_NAME " %s " "(" QW_PLATFORM ")" "\n" BUILD_DATE "\n", VersionString());
	}

	return str;
}

/*
=======================
Version_f
======================
*/
void Version_f (void)
{
	Con_Printf ("%s", VersionStringFull());
	Con_Printf (PROJECT_NAME " Project home page: " PROJECT_URL "\n\n");
}

