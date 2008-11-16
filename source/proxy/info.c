/*
	info.c
*/

#include "qwfwd.h"

qbool ValidateUserInfo (char *userinfo)
{
	while (*userinfo)
	{
		if (*userinfo == '\\')
			userinfo++;

		if (*userinfo++ == '\\')
			return false;

		while (*userinfo && *userinfo != '\\')
			userinfo++;
	}

	return true;
}

char *Info_ValueForKey (const char *s, const char *const key, char *const buffer, size_t buffersize)
{
	size_t keylen = strlen(key);
	
	// check arguments
	if (s[0] != '\\' || keylen == 0 || buffersize < 2) 
		goto notfound;
	
	buffersize--; // make space for the null-terminator

	do {
		size_t matched = 0;
		const char* keyp = key;

		s++; // skip backslash

		while (*s && *keyp && *s == *keyp && *s != '\\') {
			matched++;
			++keyp;
			++s;
		}
		if (matched == keylen && *s == '\\') {
			// match value
			size_t copied = 0;

			s++;
			while (*s && *s != '\\' && copied < buffersize) {	
				buffer[copied++] = *s++;
			}
			buffer[copied] = '\0'; // space for this is always available, see above
			return buffer;
		}
		else {
			while (*s && *s != '\\') {
				s++; // skip the key
			}
			if (*s) {
				s++; // skip backslash
			}
			while (*s && *s != '\\') {
				s++; // skip value
			}
		}
	} while (*s);

notfound:
	buffer[0] = '\0';
	return buffer;
}

void Info_RemoveKey (char *s, const char *key)
{
	char	*start;
	char	pkey[1024];
	char	value[1024];
	char	*o;

	if (strstr (key, "\\"))
	{
//		printf ("Key has a slash\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey))
		{
			strcpy (start, s);	// FIXME: remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_SetValueForStarKey (char *s, const char *key, const char *value, int maxsize)
{
	char	newv[1024], *v;
	int		c;

	if (strstr (key, "\\") || strstr (value, "\\") )
	{
//		printf ("Key has a slash\n");
		return;
	}

	if (strstr (key, "\"") || strstr (value, "\"") )
	{
//		printf ("Key has a quote\n");
		return;
	}

	if (strlen(key) >= MAX_INFO_KEY || strlen(value) >= MAX_INFO_KEY)
	{
//		printf ("Key or value is too long\n");
		return;
	}

	// this next line is kinda trippy
	if (*(v = Info_ValueForKey(s, key, newv, sizeof(newv))))
	{
		// key exists, make sure we have enough room for new value, if we don't,
		// don't change it!
		if ((int)strlen(value) - (int)strlen(v) + (int)strlen(s) + 1 > maxsize)
		{
	//		Con_TPrintf (TL_INFOSTRINGTOOLONG);
			return;
		}
	}


	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	snprintf (newv, sizeof(newv), "\\%s\\%s", key, value);

	if ((int)(strlen(newv) + strlen(s) + 1) > maxsize)
	{
//		printf ("info buffer is too small\n");
		return;
	}

	// only copy ascii values
	s += strlen(s);
	v = newv;
	while (*v)
	{
		c = (unsigned char)*v++;

//		c &= 127;		// strip high bits
		if (c > 13) // && c < 127)
			*s++ = c;
	}
	*s = 0;
}
