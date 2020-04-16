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

/*
=======================
VersionString
======================
*/
char *VersionString (void)
{
	return VERSION_NUMBER " (build " GIT_COMMIT ")";
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
		snprintf(str, sizeof(str), SERVER_NAME " " VERSION_NUMBER " (" QW_PLATFORM " build " GIT_COMMIT ")");
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
	Con_Printf ("%s\n%s\nHome page: %s\n", VersionStringFull(), BUILD_DATE, HOMEPAGE_URL);
}

