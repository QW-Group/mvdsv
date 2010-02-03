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

// cmd.h -- Command buffer and command execution

//===========================================================================

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.

The game starts with a Cbuf_AddText ("exec quake.rc\n"); Cbuf_Execute ();

*/

#ifndef __CMD_H__
#define __CMD_H__

#define MAXCMDBUF 16384

typedef struct cbuf_s {
	char	text_buf[MAXCMDBUF];
	int	text_start;
	int	text_end;
	qbool	wait;
} cbuf_t;

extern cbuf_t	cbuf_main;
extern cbuf_t	*cbuf_current;

void Cbuf_AddTextEx (cbuf_t *cbuf, char *text);
void Cbuf_InsertTextEx (cbuf_t *cbuf, char *text);
void Cbuf_ExecuteEx (cbuf_t *cbuf);

void Cbuf_Init (void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText (char *text);
// as new commands are generated from the console or keybindings,
// the text is added to the end of the command buffer.

void Cbuf_InsertText (char *text);
// when a command wants to issue other commands immediately, the text is
// inserted at the beginning of the buffer, before any remaining unexecuted
// commands.

void Cbuf_Execute (void);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function!

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) (void);

typedef struct cmd_function_s
{
	struct cmd_function_s	*hash_next;
	struct cmd_function_s	*next;
	char			*name;
	xcommand_t		function;
} cmd_function_t;

void Cmd_DeInit (void);
void Cmd_Init (void);

void Cmd_AddCommand (char *cmd_name, xcommand_t function);
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_stringcmd instead of executed locally

qbool Cmd_Exists (char *cmd_name);
// used by the cvar code to check for cvar / command name overlap

int Cmd_Argc (void);
char *Cmd_Argv (int arg);
char *Cmd_Args (void);
// Cmd_Args_Range( 0, -1 ) return all params
char *Cmd_Args_Range(int from, int to, char *buf, size_t buf_size);
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are always safe.

void Cmd_ExpandString (char *data, char *dest);
// Expands all $cvar or $macro expressions.
// dest should point to a 1024-byte buffer

void Cmd_TokenizeString (char *text);
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void Cmd_ExecuteString (char *text);
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console

void Cmd_StuffCmds (int argc, char *argv[]);


//char *Cmd_RconCommand(char *cmd, char *buf, int buf_size);
// put command in to execution buffer and run execution buffer, output will be in "buf"

//===========================================================================

#define	MAX_ALIAS_NAME	32

typedef struct cmd_alias_s
{
	struct cmd_alias_s	*hash_next;
	struct cmd_alias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmd_alias_t;

qbool Cmd_DeleteAlias (char *name);	// return true if successful

#define MAX_ARGS 80

#endif /* !__CMD_H__ */
