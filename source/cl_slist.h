/*
	cl_slist.h

	serverlist addressbook interface

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

	$Id: cl_slist.h,v 1.1.1.1 2004/09/28 18:56:38 vvd0 Exp $
*/

// #include <quakeio.h>
//#include "common.h"
#define MAX_SERVER_LIST 256

typedef struct {
	char *server;
	char *description;
	int ping;
} server_entry_t;

extern server_entry_t	slist[MAX_SERVER_LIST];

void Server_List_Init(void);
void Server_List_Shutdown(void);
int Server_List_Set(int i,char *addr,char *desc);
int Server_List_Reset_NoFree(int i);
int Server_List_Reset(int i);
void Server_List_Switch(int a,int b);
int Server_List_Len(void);
int Server_List_Load(FILE *f);
int Server_List_Save(FILE *f);
char *gettokstart (char *str, int req, char delim);
int gettoklen(char *str, int req, char delim);
