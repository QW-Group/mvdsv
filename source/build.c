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
 
	$Id: build.c 636 2007-07-20 05:07:57Z disconn3ct $
*/

#include "qwsvdef.h"

char full_version[SIZEOF_FULL_VERSION];

#if 1

//returns SVN revision number
int build_number ()
{
	static int b = 0;

	if (b)
		return b;

	{
		char rev_num[] = "$Revision$";

		if (!strncasecmp(rev_num, "$Revision:", sizeof("$Revision:") - 1))
			b = atoi(rev_num + sizeof("$Revision:") - 1);
	}

	return b;
}

#else

char *date = "Oct 24 1996";
static char *date = __DATE__ ;
static char *mon[12] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char mond[12] =
    { 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

// returns days since Dec 21 1999
int build_number (void)
{
	static int b = 0;

	if (b)
		return b;

	{
		int m = 0;
		int d = -1;
		int y = -1901;
		for (m = 0; m < 11; m++)
		{
			if (strncasecmp(&date[0], mon[m], 3) == 0)
				break;
			d += mond[m];
		}

		d += Q_atoi(&date[4]);
		y += Q_atoi(&date[7]);
		b = d + y * 365 + y / 4;

		if ((y % 4) == 3 && m > 1)
			++b;

		b -= 36148; // Dec 21 1999
	}
	return b;
}

#endif

/*
=======================
Version_f
======================
*/
void Version_f (void)
{
	Con_Printf ("%s\n", full_version);
	Con_Printf (PROJECT_NAME " Project home page: " PROJECT_URL "\n\n");
}
