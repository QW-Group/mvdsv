/*
Some token parse functions, too expansive for stand alone file?
*/

#include "qwfwd.h"

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


#define DEFAULT_PUNCTUATION "(,{})(\':;=!><&|+"

char *COM_ParseToken (char *data, char *out, int outsize, const char *punctuation)
{
	int		c;
	int		len;

	if (!punctuation)
		punctuation = DEFAULT_PUNCTUATION;

	len = 0;
	out[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;			// end of file;
		data++;
	}

// skip // comments
	if (c=='/')
	{
		if (data[1] == '/')
		{
			while (*data && *data != '\n')
				data++;
			goto skipwhite;
		}
		else if (data[1] == '*')
		{
			data+=2;
			while (*data && (*data != '*' || data[1] != '/'))
				data++;
			data+=2;
			goto skipwhite;
		}
	}


// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			if (len >= outsize-1)
			{
				out[len] = '\0';
				return data;
			}
			c = *data++;
			if (c=='\"' || !c)
			{
				out[len] = 0;
				return data;
			}
			out[len] = c;
			len++;
		}
	}

// parse single characters
	if (strchr(punctuation, c))
	{
		out[len] = c;
		len++;
		out[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		if (len >= outsize-1)
			break;
		out[len] = c;
		data++;
		len++;
		c = *data;
		if (strchr(punctuation, c))
			break;
	} while (c>32);

	out[len] = 0;
	return data;
}
