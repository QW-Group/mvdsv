#include "defs.h"

#define MAX_TOKEN	1024

static char	token[MAX_TOKEN];

/*
==============
Parse

Parse a token out of a string
==============
*/
char *Parse (char *data, qboolean newline)
{
	unsigned char c;
	int		len;

	len = 0;
	token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ( (c = *data) == ' ' || c == '\t' || (newline && (c == '\r' || c == '\n')))
		data++;

	if (c == 0)
		return NULL;			// end of file;

// skip # comments
	if (c=='#')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
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
				token[len] = 0;
				if (!c)
					data--;
				return data;
			}
			token[len] = c;
			len++;
		}
	}

// parse a regular word
	do
	{
		token[len] = c;
		data++;
		len++;
		if (len >= MAX_TOKEN-1)
			break;
		c = *data;
	} while (c && c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '#');

	token[len] = 0;
	return data;
}

extern param_t params[];

void ReadIni(char *buf)
{
	param_t	*param;

	while (buf)
	{
		buf = Parse(buf, true);

		if (!token[0])
			return;

		if (token[0] == '[')
			goto finish;

		// settings
		for (param = params; param->name; param++)
		{
			if (!_stricmp(token, param->name+1) || (param->shname && !_stricmp(param->shname+1, token)))
			{
				if (!CheckParm(param->name) && (!param->shname || !CheckParm(param->shname)))
				{
					// if --option is set in cmdline ignore this option
					if (CheckParm(va("-%s", param->name)) || (param->shname && CheckParm(va("-%s", param->shname))))
						goto finish;

					AddParm(va("-%s",token));
					if (param->type & (TYPE_S | TYPE_I))
					{
						buf = Parse(buf, false);
						AddParm(token);
					}
				}

				goto finish;
			}
		}

		// parse argv will scream if it doesn't know it, we don't do it here.
		AddParm(va("-%s", token));
finish:
		// skip line
		while (*buf && *buf != '\n')
			buf++;
	}
}