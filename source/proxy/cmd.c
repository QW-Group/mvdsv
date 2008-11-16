/*
	cmd.c
*/

#include "qwfwd.h"

#define MAX_ARGS 80


//=============================================================================

static	int			cmd_argc;
static	char		*cmd_argv[MAX_ARGS];
static	char		*cmd_null_string = "";
static	char		*cmd_args = NULL;


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
	if ( arg >= cmd_argc )
		return cmd_null_string;
	return cmd_argv[arg];
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


//=============================================================================

char com_token[MAX_COM_TOKEN];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	unsigned char c;
	int len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

	// skip whitespace
	while (true)
	{
		while ( (c = *data) == ' ' || c == '\t' || c == '\r' || c == '\n')
			data++;

		if (c == 0)
			return NULL; // end of file;

		// skip // comments
		if (c=='/' && data[1] == '/')
		{
			while (*data && *data != '\n')
				data++;
		}
		else
			break;
	}

	// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				if (!c)
					data--;
				return data;
			}
			if (len < MAX_COM_TOKEN-1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_COM_TOKEN-1)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	}
	while (c && c != ' ' && c != '\t' && c != '\n' && c != '\r');

	com_token[len] = 0;
	return data;
}
