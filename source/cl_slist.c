/*
	cl_slist.c

	serverlist addressbook

	Copyright (C) 1999,2000  contributors of the QuakeForge project
	Please see the file "AUTHORS" for a list of contributors

	Author: Brian Koropoff
	Date: 03 May 2000

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

	$Id: cl_slist.c,v 1.1.1.6 2004/10/18 18:10:50 vvd0 Exp $
*/

#include "quakedef.h"
#include "cl_slist.h"

//Better watch out for buffer overflows
server_entry_t	slist[MAX_SERVER_LIST];

char *gettokstart (char *str, int req, char delim);
int gettoklen(char *str, int req, char delim);


void SList_Init(void)
{
	int i;
	for(i=0; i < MAX_SERVER_LIST; i++) {
		slist[i].server = NULL;
		slist[i].description = NULL;
		slist[i].ping = 0;
	}
}


void SList_Shutdown(void) // I am the liberator of memory.
{  
	int i;
	FILE *f;

	// FIXME: if the list was deleted, the changes will not be saved
	for (i=0; i < MAX_SERVER_LIST; i++) {
		if (slist[i].server)
			break;
	}
	if (i < MAX_SERVER_LIST)	// the list is not empty
	{
		if (!(f = fopen(va("%s/servers.txt", com_basedir),"w"))) {
			Con_Printf("Couldn't open servers.txt.\n");
			return;
		}
		SList_Save(f);
		fclose(f);
	}
	for(i=0;i < MAX_SERVER_LIST;i++) {
		if (slist[i].server)
			free(slist[i].server);
		if (slist[i].description)
			free(slist[i].description);
	}
}
			

void SList_Set (int i, char *addr, char *desc)
{
	if ((unsigned)i >= MAX_SERVER_LIST)
		return;

	// Free old strings
	if (slist[i].server)
		free(slist[i].server);
	if (slist[i].description)
		free(slist[i].description);

	slist[i].server = Q_Malloc (strlen(addr) + 1);
	slist[i].description = Q_Malloc (strlen(desc) + 1);
	strcpy (slist[i].server, addr);
	strcpy (slist[i].description, desc);
}


//NEVER USE THIS UNLESS REALLY NEEDED
void SList_Reset_NoFree (int i)
{ 
	if ((unsigned)i >= MAX_SERVER_LIST)
		return;

	slist[i].server = '\0';
	slist[i].description = '\0';
	slist[i].ping = 0;
}


void SList_Reset (int i)
{
	if ((unsigned)i >= MAX_SERVER_LIST)
		return;

	if (slist[i].server)
		free(slist[i].server);
	if (slist[i].description)
		free(slist[i].description);
	slist[i].server = '\0';
	slist[i].description = '\0';
	slist[i].ping = 0;
}


void SList_Switch (int a,int b)
{
	server_entry_t temp;

	if ((unsigned)a >= MAX_SERVER_LIST || (unsigned)b >= MAX_SERVER_LIST)
		return;

	memcpy(&temp, &slist[a], sizeof(temp));
	memcpy(&slist[a], &slist[b], sizeof(temp));
	memcpy(&slist[b], &temp, sizeof(temp));
}

int SList_Len (void)
{
	int i;
	for (i = 0; i < MAX_SERVER_LIST && slist[i].server;i++)
		;
	return i;
}

void SList_Load ()	 // This could get messy
{
	int serv = 0;
	char line[256]; /* Long lines get truncated. */
	int c = ' ';    /* int so it can be compared to EOF properly*/
	char *start;
	int len;
	int i;
	char *addr;
	FILE *f;

	f = fopen (va("%s/servers.txt", com_basedir), "r");
	if (f == NULL)
		return;

	while (serv < MAX_SERVER_LIST) {
		//First, get a line
		i = 0;
		c = ' ';
		while (c != '\n' && c != EOF) {
			c = fgetc(f);
			if (i < 255) {
				line[i] = c;
				i++;
			}
		}

		line[i - 1] = '\0';
		// Now we can parse it
		if ((start = gettokstart(line,1,' ')) != NULL) {
			len = gettoklen(line,1,' ');
			addr = Q_Malloc (len + 1);
			strlcpy (addr, line, len + 1);
			if ((start = gettokstart(line,2,' '))) {
				SList_Set (serv, addr, start);
			}
			else {
				SList_Set (serv, addr, "Unknown");
			}
			serv++;
		} 

		if (c == EOF)
			break;
	}

	fclose (f);
}


void SList_Save (FILE *f)
{
	int i;

	for (i=0; i < MAX_SERVER_LIST; i++) {
		if (slist[i].server)
			fprintf(f,"%s        %s\n",
				slist[i].server,
				slist[i].description);
	}
}


char *gettokstart (char *str, int req, char delim)
{
	char *start = str;
	
	int tok = 1;

	while (*start == delim) {
		start++;
	}
	if (*start == '\0')
		return '\0';
	while (tok < req) { //Stop when we get to the requested token
		if (*++start == delim) { //Increment pointer and test
			while (*start == delim) { //Get to next token
				start++;
			}
			tok++;
		}
		if (*start == '\0') {
			return '\0';
		}
	}
	return start;
}

int gettoklen (char *str, int req, char delim) {
	char *start = 0;
	
	int len = 0;
	
	start = gettokstart(str,req,delim);
	if (start == '\0') {
		return 0;
	}
	while (*start != delim && *start != '\0') {
		start++;
		len++;
	}
	return len;
}
