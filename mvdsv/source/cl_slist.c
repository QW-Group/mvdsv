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

	$Id: cl_slist.c,v 1.1.1.1 2004/09/28 18:56:38 vvd0 Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quakedef.h"
//#include "common.h"
//#include "console.h"
#include "cl_slist.h"

//Better watch out for buffer overflows
server_entry_t	slist[MAX_SERVER_LIST];
extern char	com_basedir[MAX_OSPATH];	// Tonik

void Server_List_Init(void) { // Do this or everything else will sig11
	int i;
	for(i=0; i < MAX_SERVER_LIST; i++) {
		slist[i].server = '\0';
		slist[i].description = '\0';
		slist[i].ping = 0;
	}
}


void Server_List_Shutdown(void) // I am the liberator of memory.
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
		Server_List_Save(f);
		fclose(f);
	}
	for(i=0;i < MAX_SERVER_LIST;i++) {
		if (slist[i].server)
			free(slist[i].server);
		if (slist[i].description)
			free(slist[i].description);
	}
}
			

int Server_List_Set(int i,char *addr,char *desc) {
	int len;
	if (i < MAX_SERVER_LIST && i >= 0) {
		if (slist[i].server)	// (Re)allocate memory first
			free(slist[i].server);
		if (slist[i].description)
			free(slist[i].description);
		len = strlen(desc);
		slist[i].server = malloc(strlen(addr) + 1);
		slist[i].description = malloc(len + 1);
		strcpy(slist[i].server,addr);
		strcpy(slist[i].description,desc);
		return 0;  // Yay, we haven't segfaulted yet.
	}
	return 1; // Out of range
}
int Server_List_Reset_NoFree (int i) { //NEVER USE THIS UNLESS REALLY NEEDED
	if (i < MAX_SERVER_LIST && i >= 0) {
		slist[i].server = '\0';
		slist[i].description = '\0';
		slist[i].ping = 0;
		return 0;
	}
	return 1;
}

int Server_List_Reset (int i) {
	if (i < MAX_SERVER_LIST && i >= 0) {
		if (slist[i].server)
			free(slist[i].server);
		if (slist[i].description)
			free(slist[i].description);
		slist[i].server = '\0';
		slist[i].description = '\0';
		slist[i].ping = 0;
		return 0;
	}
	return 1;
}

void Server_List_Switch(int a,int b) {
	server_entry_t temp;
	memcpy(&temp,&slist[a],sizeof(temp));
	memcpy(&slist[a],&slist[b],sizeof(temp));
	memcpy(&slist[b],&temp,sizeof(temp));
}

int Server_List_Len (void) {
	int i;
	for (i = 0; i < MAX_SERVER_LIST && slist[i].server;i++)
		;
	return i;
}

int Server_List_Load (FILE *f) { // This could get messy
	int serv = 0;
	char line[256]; /* Long lines get truncated. */
	int c = ' ';    /* int so it can be compared to EOF properly*/
	char *start;
	int len;
	int i;
	char *addr;

	// Init again to clear the list
//	Server_List_Shutdown();
//	Server_List_Init();
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
		line[i - 1] = '\0'; // Now we can parse it
		if ((start = gettokstart(line,1,' ')) != NULL) {
			len = gettoklen(line,1,' ');
			addr = malloc(len + 1);
			strncpy(addr,&line[0],len);
			addr[len] = '\0';
			if ((start = gettokstart(line,2,' '))) {
				Server_List_Set(serv,addr,start);
			}
			else {
				Server_List_Set(serv,addr,"Unknown");
			}
			serv++;
		} 
		if (c == EOF)  // We're done
			return 0;
	}
	return 0;
}

int Server_List_Save(FILE *f) {
	int i;
	for(i=0;i < MAX_SERVER_LIST;i++) {
		if (slist[i].server)
			fprintf(f,"%s        %s\n",
				slist[i].server,
				slist[i].description);
	}
	return 0;
}
char *gettokstart (char *str, int req, char delim) {
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
