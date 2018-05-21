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
// cmd.c -- Quake script command processing module

#include "qwsvdef.h"

cbuf_t cbuf_main;
cbuf_t *cbuf_current = NULL;

//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
void Cmd_Wait_f (void)
{
	if (cbuf_current)
		cbuf_current->wait = true;
}

/*
=============================================================================
 
						COMMAND BUFFER
 
=============================================================================
*/


void Cbuf_AddText (const  char *text) { Cbuf_AddTextEx (&cbuf_main, text); }
void Cbuf_InsertText (const char *text) { Cbuf_InsertTextEx (&cbuf_main, text); }
void Cbuf_Execute () { Cbuf_ExecuteEx (&cbuf_main); }

/*
============
Cbuf_Init
============
*/
void Cbuf_Init (void)
{
	cbuf_main.text_start = cbuf_main.text_end = MAXCMDBUF / 2;
	cbuf_main.wait = false;
}

/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void Cbuf_AddTextEx (cbuf_t *cbuf, const char *text)
{
	size_t len;
	unsigned int new_start;
	unsigned int new_bufsize;

	len = strlen (text);

	if (cbuf->text_end + len <= MAXCMDBUF)
	{
		memcpy (cbuf->text_buf + cbuf->text_end, text, len);
		cbuf->text_end += len;
		return;
	}

	new_bufsize = cbuf->text_end-cbuf->text_start+len;
	if (new_bufsize > MAXCMDBUF)
	{
		Con_Printf ("Cbuf_AddText: overflow\n");
		return;
	}

	// Calculate optimal position of text in buffer
	new_start = (MAXCMDBUF - new_bufsize) / 2;

	memcpy (cbuf->text_buf + new_start, cbuf->text_buf + cbuf->text_start, cbuf->text_end-cbuf->text_start);
	memcpy (cbuf->text_buf + new_start + cbuf->text_end-cbuf->text_start, text, len);
	cbuf->text_start = new_start;
	cbuf->text_end = cbuf->text_start + new_bufsize;
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void Cbuf_InsertTextEx (cbuf_t *cbuf, const char *text)
{
	size_t len;
	unsigned int new_start;
	unsigned int new_bufsize;

	len = strlen(text);

	if (len < cbuf->text_start)
	{
		memcpy (cbuf->text_buf + (cbuf->text_start - len - 1), text, len);
		cbuf->text_buf[cbuf->text_start-1] = '\n';
		cbuf->text_start -= len + 1;
		return;
	}

	new_bufsize = cbuf->text_end - cbuf->text_start + len + 1;
	if (new_bufsize > MAXCMDBUF)
	{
		Con_Printf ("Cbuf_InsertText: overflow\n");
		return;
	}

	// Calculate optimal position of text in buffer
	new_start = (MAXCMDBUF - new_bufsize) / 2;

	memmove (cbuf->text_buf + (new_start + len + 1), cbuf->text_buf + cbuf->text_start, cbuf->text_end-cbuf->text_start);
	memcpy (cbuf->text_buf + new_start, text, len);
	cbuf->text_buf[new_start + len] = '\n';
	cbuf->text_start = new_start;
	cbuf->text_end = cbuf->text_start + new_bufsize;
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_ExecuteEx (cbuf_t *cbuf)
{
	int i;
	char *text;
	char line[1024];
	int quotes;
	int cursize;
	int semicolon = 0;

	cbuf_current = cbuf;

	while (cbuf->text_end > cbuf->text_start)
	{
		// find a \n or ; line break
		text = (char *)cbuf->text_buf + cbuf->text_start;

		cursize = cbuf->text_end - cbuf->text_start;
		quotes = 0;
		for (i = 0; i < cursize; i++)
		{
			if (text[i] == '"')
				quotes++;
/* EXPERIMENTAL: Forbid ';' as commands separator, because ktpro didn't quote arguments
   from admin users. Example: cmd fkick "N;quit" => kick N;quit => server will exit.*/
			if (!(quotes & 1) && text[i] == ';')	// don't break if inside a quoted string
			{
				switch (semicolon)
				{
					case 0:
					case 3: semicolon = 1; break;
					case 1: semicolon = 2; break;
					default:;
				}
				break;
			}

			if (text[i] == '\n')
			{
				switch (semicolon)
				{
					case 1:
					case 2: semicolon = 3; break;
					case 3: semicolon = 0; break;
					default:;
				}
				break;
			}
		}

		// don't execute lines without ending \n; this fixes problems with
		// partially stuffed aliases not being executed properly

		if (i < sizeof(line))
		{
			memcpy(line, text, i);
			line[i] = 0;

			if (i > 0 && line[i - 1] == '\r')
				line[i - 1] = 0;	// remove DOS ending CR
		}
		else
		{
			line[0] = 0;
			Sys_Printf("Cbuf_ExecuteEx: too long\n");
		}

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer

		if (i == cursize)
		{
			cbuf->text_start = cbuf->text_end = MAXCMDBUF / 2;
		}
		else
		{
			i++;
			cbuf->text_start += i;
		}

		// security bugfix in ktpro
		if (semicolon > 1)
			Sys_Printf("ATTENTION: possibly tried to use security hole, "
						"server don't run command after ';'!\nCommand: %s\n", line);
		else
			// execute the command line
			Cmd_ExecuteString (line);

		if (cbuf->wait)
		{	// skip out while text still remains in buffer, leaving it
			// for next frame
			cbuf->wait = false;
			break;
		}
	}

	cbuf_current = NULL;
}


/*
==============================================================================
 
						SCRIPT COMMANDS
 
==============================================================================
*/

/*
===============
Cmd_StuffCmds_f

Adds command line parameters as script statements
Commands lead with a +, and continue until a - or another +
quake +prog jctest.qp +cmd amlev1
quake -nosound +cmd amlev1
===============
*/
void Cmd_StuffCmds_f (void)
{
	int i, j;
	int s;
	char *text, *build, c;

	// build the combined string to parse from
	s = 0;
	for (i = 1; i < com_argc; i++)
		s += strlen (com_argv[i]) + 1;

	if (!s)
		return;

	text = (char *) Q_malloc (s+1);
	text[0] = 0;
	for (i = 1; i < com_argc; i++)
	{
		strlcat (text, com_argv[i], s + 1);
		if (i != com_argc-1)
			strlcat (text, " ", s + 1);
	}

	// pull out the commands
	build = (char *) Q_malloc (s+1);
	build[0] = 0;

	for (i=0 ; i<s-1 ; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j=i ; (text[j] != '+') && (text[j] != '-') && (text[j] != 0) ; j++)
				;

			c = text[j];
			text[j] = 0;

			strlcat (build, text + i, s + 1);
			strlcat (build, "\n", s + 1);
			text[j] = c;
			i = j-1;
		}
	}

	if (build[0])
		Cbuf_AddText (build);

	Q_free (text);
	Q_free (build);
}


/*
===============
Cmd_Exec_f
===============
*/
void Cmd_Exec_f (void)
{
	char	*f;
	int		mark;

	if (Cmd_Argc () != 2)
	{
		Con_Printf ("exec <filename> : execute a script file\n");
		return;
	}

	// FIXME: is this safe freeing the hunk here???
	mark = Hunk_LowMark ();
	f = (char *)FS_LoadHunkFile (Cmd_Argv(1), NULL);
	if (!f)
	{
		Con_Printf ("couldn't exec %s\n",Cmd_Argv(1));
		return;
	}

	Con_Printf ("execing %s\n",Cmd_Argv(1));

	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
void Cmd_Echo_f (void)
{
	int		i;

	for (i=1 ; i<Cmd_Argc() ; i++)
		Con_Printf ("%s ",Cmd_Argv(i));
	Con_Printf ("\n");
}

/*
=============================================================================
 
								ALIASES
 
=============================================================================
*/

static cmd_alias_t	*cmd_alias_hash[32];
static cmd_alias_t	*cmd_alias;

/*
===============
Cmd_Alias_f
 
Creates a new command that executes a command string (possibly ; seperated)
===============
*/

void Cmd_Alias_f (void)
{
	cmd_alias_t	*a;
	char		cmd[1024];
	int			i, c;
	int			key;
	char		*s;
	//	cvar_t		*var;

	c = Cmd_Argc();
	if (c == 1)
	{
		Con_Printf ("Current alias commands:\n");
		for (a = cmd_alias ; a ; a=a->next)
			Con_Printf ("%s : %s\n\n", a->name, a->value);
		return;
	}

	s = Cmd_Argv(1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Con_Printf ("Alias name is too long\n");
		return;
	}

#if 0
	if ( (var = Cvar_Find(s)) != NULL )
	{
		if (var->flags & CVAR_USER_CREATED)
			Cvar_Delete (var->name);
		else
		{
			//			Con_Printf ("%s is a variable\n");
			return;
		}
	}
#endif

	key = Com_HashKey (s);

	// if the alias already exists, reuse it
	for (a = cmd_alias_hash[key] ; a ; a=a->hash_next)
	{
		if (!strcasecmp(a->name, s))
		{
			Q_free (a->value);
			break;
		}
	}

	if (!a)
	{
		a = (cmd_alias_t*) Q_malloc (sizeof(cmd_alias_t));
		a->next = cmd_alias;
		cmd_alias = a;
		a->hash_next = cmd_alias_hash[key];
		cmd_alias_hash[key] = a;
	}
	strlcpy (a->name, s, MAX_ALIAS_NAME);

	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i<c ; i++)
	{
		if (i > 2)
			strlcat (cmd, " ", sizeof(cmd));
		strlcat (cmd, Cmd_Argv(i), sizeof(cmd));
	}

	a->value = Q_strdup (cmd);
}


qbool Cmd_DeleteAlias (const char *name)
{
	cmd_alias_t	*a, *prev;
	int			key;

	key = Com_HashKey (name);

	prev = NULL;
	for (a = cmd_alias_hash[key] ; a ; a = a->hash_next)
	{
		if (!strcasecmp(a->name, name))
		{
			// unlink from hash
			if (prev)
				prev->hash_next = a->hash_next;
			else
				cmd_alias_hash[key] = a->hash_next;
			break;
		}
		prev = a;
	}

	if (!a)
		return false;	// not found

	prev = NULL;
	for (a = cmd_alias ; a ; a = a->next)
	{
		if (!strcasecmp(a->name, name))
		{
			// unlink from alias list
			if (prev)
				prev->next = a->next;
			else
				cmd_alias = a->next;

			// free
			Q_free (a->value);
			Q_free (a);
			return true;
		}
		prev = a;
	}

	Sys_Error ("Cmd_DeleteAlias: alias list broken");
	return false;	// shut up compiler
}

void Cmd_UnAlias_f (void)
{
	char		*s;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("unalias <alias>: erase an existing alias\n");
		return;
	}

	s = Cmd_Argv(1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Con_Printf ("Alias name is too long\n");
		return;
	}

	if (!Cmd_DeleteAlias(s))
		Con_Printf ("Unknown alias \"%s\"\n", s);
}

// remove all aliases
void Cmd_UnAliasAll_f (void)
{
	cmd_alias_t	*a, *next;
	int i;

	for (a=cmd_alias ; a ; a=next)
	{
		next = a->next;
		Q_free (a->value);
		Q_free (a);
	}
	cmd_alias = NULL;

	// clear hash
	for (i=0 ; i<32 ; i++)
	{
		cmd_alias_hash[i] = NULL;
	}
}


/*
=============================================================================
 
					COMMAND EXECUTION
 
=============================================================================
*/

static	int			cmd_argc;
static	char		*cmd_argv[MAX_ARGS];
static	char		*cmd_null_string = "";
static	char		*cmd_args = NULL;

static cmd_function_t	*cmd_hash_array[32];
static cmd_function_t	*cmd_functions;		// possible commands to execute

/*
============
Cmd_Argc
============
*/
int Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char *Cmd_Argv (int arg)
{
	return (arg >= cmd_argc) ? cmd_null_string : cmd_argv[arg];
}

/*
============
Cmd_Args
 
Returns a single string containing argv(1) to argv(argc()-1)
============
*/
char *Cmd_Args (void)
{
	if (!cmd_args)
		return "";
	return cmd_args;
}


/*
============
Cmd_TokenizeString
 
Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString (char *text)
{
	size_t idx, token_len;
	static char argv_buf[MAX_MSGLEN + MAX_ARGS];

	idx = 0;

	cmd_argc = 0;
	cmd_args = NULL;

	while (1)
	{
		// skip whitespace
		while (*text == ' ' || *text == '\t' || *text == '\r')
		{
			text++;
		}

		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		if (cmd_argc == 1)
			cmd_args = (char *) text;

		text = COM_Parse (text);
		if (!text)
			return;

		if (cmd_argc >= MAX_ARGS)
			return;			

		token_len = strlen(com_token);

		if (idx + token_len + 1 > sizeof(argv_buf))
			return;

		cmd_argv[cmd_argc] = argv_buf + idx;
		strlcpy (cmd_argv[cmd_argc], com_token, sizeof(argv_buf) - idx);
		cmd_argc++;

		idx += token_len + 1;
	}
}


/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand (const char *cmd_name, xcommand_t function)
{
	cmd_function_t	*cmd;
	int	key;

	if (host_initialized)	// because hunk allocation would get stomped
		Sys_Error ("Cmd_AddCommand after host_initialized");

	// fail if the command is a variable name
	if (Cvar_Find(cmd_name))
	{
		Con_Printf ("Cmd_AddCommand: %s already defined as a var\n", cmd_name);
		return;
	}

	key = Com_HashKey (cmd_name);

	// fail if the command already exists
	for (cmd=cmd_hash_array[key] ; cmd ; cmd=cmd->hash_next)
	{
		if (!strcasecmp (cmd_name, cmd->name))
		{
			Con_Printf ("Cmd_AddCommand: %s already defined\n", cmd_name);
			return;
		}
	}

	cmd = (cmd_function_t *) Hunk_Alloc (sizeof(cmd_function_t));
	cmd->name = cmd_name;
	cmd->function = function;
	cmd->next = cmd_functions;
	cmd_functions = cmd;
	cmd->hash_next = cmd_hash_array[key];
	cmd_hash_array[key] = cmd;
}


/*
============
Cmd_Exists
============
*/
qbool Cmd_Exists (const char *cmd_name)
{
	int	key;
	cmd_function_t	*cmd;

	key = Com_HashKey (cmd_name);
	for (cmd=cmd_hash_array[key] ; cmd ; cmd=cmd->hash_next)
	{
		if (!strcasecmp (cmd_name, cmd->name))
			return true;
	}

	return false;
}

static int Cmd_CommandCompare (const void *p1, const void *p2)
{
	return strcmp ((*((cmd_function_t **) p1))->name, (*((cmd_function_t **) p2))->name);
}

static void Cmd_CmdList_f(void)
{
	cmd_function_t **sorted_cmds = NULL;
	cmd_function_t *cmd;
	const char *pattern;
	int i = 0;
	int num_cmds = 0;
	int pattern_matched = 0;

	pattern = (Cmd_Argc() > 1) ? Cmd_Argv(1) : NULL;

	for (cmd = cmd_functions; cmd; cmd = cmd->next) {
		num_cmds++;
	}

	if (num_cmds > 0) {
		sorted_cmds = malloc(num_cmds * sizeof(cmd_function_t*));
		if (!sorted_cmds) {
			Sys_Error("Failed to allocate memory");
		}
	}
	else {
		Con_Printf("No commands found\n");
		return;
	}

	for (cmd = cmd_functions; cmd; cmd= cmd->next) {
		sorted_cmds[i++] = cmd;
	}
	qsort(sorted_cmds, num_cmds, sizeof(cmd_function_t *), Cmd_CommandCompare);

	Con_Printf ("List of commands:\n");
	for (i = 0; i < num_cmds; i++) {
		cmd = sorted_cmds[i];
		if (pattern && !Q_glob_match(pattern, cmd->name)) {
			continue;
		}

		Con_Printf ("%s\n", cmd->name);
		pattern_matched++;
	}

	Con_Printf ("------------\n%d/%d %scommands\n", pattern_matched, num_cmds, (pattern) ? "matching " : "");

	free(sorted_cmds);
}

/*
================
Cmd_ExpandString

Expands all $cvar expressions to cvar values
Note: dest must point to a 1024 byte buffer
================
*/
void Cmd_ExpandString (const char *data, char *dest)
{
	unsigned int	c;
	char	buf[255];
	int	i, len;
	cvar_t	*var, *bestvar;
	int	quotes = 0;
	char	*str;
	int	name_length = 0;

	len = 0;

	while ( (c = *data) != 0)
	{
		if (c == '"')
			quotes++;

		if (c == '$' && !(quotes&1))
		{
			data++;

			// Copy the text after '$' to a temp buffer
			i = 0;
			buf[0] = 0;
			bestvar = NULL;
			while ((c = *data) > 32)
			{
				if (c == '$')
					break;
				data++;
				buf[i++] = c;
				buf[i] = 0;
				if ( (var = Cvar_Find(buf)) != NULL )
					bestvar = var;

				if (i >= (int)sizeof(buf)-1)
					break; // there no more space in buf
			}

			if (bestvar)
			{
				str = bestvar->string;
				name_length = strlen(bestvar->name);
			}
			else
				str = NULL;

			if (str)
			{
				// check buffer size
				if (len + strlen(str) >= 1024-1)
					break;

				strlcpy(dest + len, str, 1024 - len);
				len += strlen(str);
				i = name_length;
				while (buf[i])
					dest[len++] = buf[i++];
			}
			else
			{
				// no matching cvar or macro
				dest[len++] = '$';
				if (len + strlen(buf) >= 1024-1)
					break;
				strlcpy (dest + len, buf, 1024 - len);
				len += strlen(buf);
			}
		}
		else
		{
			dest[len] = c;
			data++;
			len++;
			if (len >= 1024-1)
				break;
		}
	};

	dest[len] = 0;
}


/*
============
Cmd_ExecuteString
 
A complete command line has been parsed, so try to execute it
FIXME: lookupnoadd the token to speed search?
============
*/
extern qbool PR_ConsoleCmd(void);

void Cmd_ExecuteString (const char *text)
{
	cmd_function_t	*cmd;
	cmd_alias_t	*a;
	int		key;
	static char	buf[1024];

	Cmd_ExpandString (text, buf);
	Cmd_TokenizeString (buf);

	// execute the command line
	if (!Cmd_Argc())
		return;		// no tokens

	key = Com_HashKey (cmd_argv[0]);

	// check functions
	for (cmd=cmd_hash_array[key] ; cmd ; cmd=cmd->hash_next)
	{
		if (!strcasecmp (cmd_argv[0], cmd->name))
		{
			if (cmd->function)
				cmd->function ();

			return;
		}
	}

	// check cvars
	if (Cvar_Command())
		return;

	// check alias
	for (a=cmd_alias_hash[key] ; a ; a=a->hash_next)
	{
		if (!strcasecmp (cmd_argv[0], a->name))
		{
			Cbuf_InsertText ("\n");
			Cbuf_InsertText (a->value);
			return;
		}
	}

	if (PR_ConsoleCmd())
		return;

	Con_Printf ("Unknown command \"%s\"\n", Cmd_Argv(0));
}


static qbool is_numeric (const char *c)
{
	return (*c >= '0' && *c <= '9') ||
	       ((*c == '-' || *c == '+') && (c[1] == '.' || (c[1]>='0' && c[1]<='9'))) ||
	       (*c == '.' && (c[1]>='0' && c[1]<='9'));
}
/*
================
Cmd_If_f
================
*/
void Cmd_If_f (void)
{
	int		i, c;
	char	*op;
	qbool	result;
	char	buf[256];

	c = Cmd_Argc ();
	if (c < 5)
	{
		Con_Printf ("usage: if <expr1> <op> <expr2> <command> [else <command>]\n");
		return;
	}

	op = Cmd_Argv (2);
	if (!strcmp(op, "==") || !strcmp(op, "=") || !strcmp(op, "!=")
	        || !strcmp(op, "<>"))
	{
		if (is_numeric(Cmd_Argv(1)) && is_numeric(Cmd_Argv(3)))
			result = Q_atof(Cmd_Argv(1)) == Q_atof(Cmd_Argv(3));
		else
			result = !strcmp(Cmd_Argv(1), Cmd_Argv(3));

		if (op[0] != '=')
			result = !result;
	}
	else if (!strcmp(op, ">"))
		result = Q_atof(Cmd_Argv(1)) > Q_atof(Cmd_Argv(3));
	else if (!strcmp(op, "<"))
		result = Q_atof(Cmd_Argv(1)) < Q_atof(Cmd_Argv(3));
	else if (!strcmp(op, ">="))
		result = Q_atof(Cmd_Argv(1)) >= Q_atof(Cmd_Argv(3));
	else if (!strcmp(op, "<="))
		result = Q_atof(Cmd_Argv(1)) <= Q_atof(Cmd_Argv(3));
	else if (!strcmp(op, "isin"))
		result = strstr(Cmd_Argv(3), Cmd_Argv(1)) != NULL;
	else if (!strcmp(op, "!isin"))
		result = strstr(Cmd_Argv(3), Cmd_Argv(1)) == NULL;
	else
	{
		Con_Printf ("unknown operator: %s\n", op);
		Con_Printf ("valid operators are ==, =, !=, <>, >, <, >=, <=, isin, !isin\n");
		return;
	}

	buf[0] = '\0';
	if (result)
	{
		for (i=4; i < c ; i++)
		{
			if ((i == 4) && !strcasecmp(Cmd_Argv(i), "then"))
				continue;
			if (!strcasecmp(Cmd_Argv(i), "else"))
				break;
			if (buf[0])
				strlcat (buf, " ", sizeof(buf));
			strlcat (buf, Cmd_Argv(i), sizeof(buf));
		}
	}
	else
	{
		for (i=4; i < c ; i++)
		{
			if (!strcasecmp(Cmd_Argv(i), "else"))
				break;
		}

		if (i == c)
			return;

		for (i++ ; i < c ; i++)
		{
			if (buf[0])
				strlcat (buf, " ", sizeof(buf));
			strlcat (buf, Cmd_Argv(i), sizeof(buf));
		}
	}

	Cbuf_InsertText (buf);
}

/*
============
Cmd_Init
============
*/
void Cmd_Init (void)
{
	//
	// register our commands
	//
	Cmd_AddCommand ("exec",Cmd_Exec_f);
	Cmd_AddCommand ("echo",Cmd_Echo_f);
	Cmd_AddCommand ("alias",Cmd_Alias_f);
	Cmd_AddCommand ("wait", Cmd_Wait_f);
	Cmd_AddCommand ("cmdlist", Cmd_CmdList_f);
	Cmd_AddCommand ("unaliasall", Cmd_UnAliasAll_f);
	Cmd_AddCommand ("unalias", Cmd_UnAlias_f);
	Cmd_AddCommand ("if", Cmd_If_f);
}
