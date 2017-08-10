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

#include "qwsvdef.h"

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
============
*/
#define STRING_SIZE 1024
char *va (const char *format, ...)
{
	va_list argptr;
	static char string[MAX_STRINGS][STRING_SIZE];
	static int index1 = 0;

	index1 &= (MAX_STRINGS - 1);
	va_start (argptr, format);
	vsnprintf (string[index1], STRING_SIZE, format, argptr);
	va_end (argptr);

	return string[index1++];
}

/*
============================================================================

			LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_atoi (const char *str)
{
	int	val;
	int	sign;
	int	c;

	if (!str)
		return 0;

	for (; *str && *str <= ' '; str++);

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
	{
		if (*str == '+')
			str++;

		sign = 1;
	}

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = (int)(unsigned char)*str++;
			if ( isdigit(c) )
				val = (val<<4) + c - '0';
			else if ( isxdigit(c) )
				val = (val<<4) + tolower(c) - 'a' + 10;
			else
				return val*sign;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}

	//
	// assume decimal
	//
	while (1)
	{
		c = (int)(unsigned char)*str++;
		if ( !isdigit(c) )
			return val*sign;
		val = val*10 + c - '0';
	}

	return 0;
}


float Q_atof (const char *str)
{
	double	val;
	int		sign;
	int		c;
	int		decimal, total;

	if (!str)
		return 0;

	for (; *str && *str <= ' '; str++);

	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else
	{
		if (*str == '+')
			str++;

		sign = 1;
	}

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X') )
	{
		str += 2;
		while (1)
		{
			c = (int)(unsigned char)*str++;
			if ( isdigit(c) )
				val = (val*16) + c - '0';
			else if ( isxdigit(c) )
				val = (val*16) + tolower(c) - 'a' + 10;
			else
				return val*sign;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return sign * str[1];
	}

	//
	// assume decimal
	//
	decimal = -1;
	total = 0;
	while (1)
	{
		c = *str++;
		if (c == '.')
		{
			decimal = total;
			continue;
		}
		if ( !isdigit(c) )
			break;
		val = val*10 + c - '0';
		total++;
	}

	if (decimal == -1)
		return val*sign;
	while (total > decimal)
	{
		val /= 10;
		total--;
	}

	return val*sign;
}

// removes trailing zeros
/*char *Q_ftos (float value)
{
	static char str[128];
	int	i;

	snprintf (str, sizeof(str), "%f", value);

	for (i=strlen(str)-1 ; i>0 && str[i]=='0' ; i--)
		str[i] = 0;
	if (str[i] == '.')
		str[i] = 0;

	return str;
}*/

#if defined(_MSC_VER) && (_MSC_VER < 1900)
int snprintf(char *buffer, size_t count, char const *format, ...)
{
	int ret;
	va_list argptr;
	if (!count) return 0;
	va_start(argptr, format);
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	va_end(argptr);
	return ret;
}
#endif // !(Visual Studio 2015+)

#if defined(_MSC_VER) && (_MSC_VER < 1400)
int vsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	int ret;
	if (!count) return 0;
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	return ret;
}
#endif

#if defined(__linux__) || defined(_WIN32)
/* 
 * Functions strlcpy, strlcat, strnstr and strcasestr
 * was copied from FreeBSD 4.10 libc: src/lib/libc/string/
 *
 *  // VVD
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0)
	{
		do
		{
			if ((*d++ = *s++) == 0)
				break;
		}
		while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0)
	{
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

size_t strlcat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0')
	{
		if (n != 1)
		{
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));       /* count does not include NUL */
}
#endif

#if !defined(__FreeBSD__) && !defined(__APPLE__) && !defined(__DragonFly__)
char *strnstr (const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0')
	{
		len = strlen (find);
		do
		{
			do
			{
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			}
			while (sc != c);
			if (len > slen)
				return (NULL);
		}
		while (strncmp (s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *strcasestr(register const char *s, register const char *find)
{
	register char c, sc;
	register size_t len;

	if ((c = *find++) != 0)
	{
		c = tolower((unsigned char)c);
		len = strlen(find);
		do
		{
			do
			{
				if ((sc = *s++) == 0)
					return (NULL);
			}
			while ((char)tolower((unsigned char)sc) != c);
		}
		while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}
#endif

#ifdef _WIN32
int strchrn (const char* str, const char c)
{
	int i = 0;
	while (*str)
		if (*str++ == c)
			++i;
	return i;
}
#endif

/*
============================================================================

			BYTE ORDER FUNCTIONS

============================================================================
*/

/*short ShortSwap (short s)
{
	byte    b1,b2;

	b1 = s&255;
	b2 = (s>>8)&255;

	return (b1<<8) + b2;
}

int LongSwap (int l)
{
	byte    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}*/

#ifndef id386
#ifdef __cplusplus
extern "C" {
#endif
short ShortSwap (short s)
{
	union
	{
		short	s;
		byte	b[2];
	} dat1, dat2;
	dat1.s = s;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	return dat2.s;
}

int LongSwap (int l)
{
	union
	{
		int		l;
		byte	b[4];
	} dat1, dat2;
	dat1.l = l;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.l;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}
#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif

#ifdef __PDP_ENDIAN__Q__
int LongSwapPDP2Big (int l)
{
	union
	{
		int		l;
		byte	b[4];
	} dat1, dat2;
	dat1.l = l;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	dat2.b[2] = dat1.b[3];
	dat2.b[3] = dat1.b[2];
	return dat2.l;
}

int LongSwapPDP2Lit (int l)
{
	union
	{
		int		l;
		short	s[2];
	} dat1, dat2;
	dat1.l = l;
	dat2.s[0] = dat1.s[1];
	dat2.s[1] = dat1.s[0];
	return dat2.l;
}

float FloatSwapPDP2Big (float f)
{
	union
	{
		float	f;
		byte	b[4];
	} dat1, dat2;
	dat1.f = f;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	dat2.b[2] = dat1.b[3];
	dat2.b[3] = dat1.b[2];
	return dat2.f;
}

float FloatSwapPDP2Lit (float f)
{
	union
	{
		float	f;
		short	s[2];
	} dat1, dat2;
	dat1.f = f;
	dat2.s[0] = dat1.s[1];
	dat2.s[1] = dat1.s[0];
	return dat2.f;
}
#endif

/*
===================
Q_malloc

Use it instead of malloc so that if memory allocation fails,
the program exits with a message saying there's not enough memory
instead of crashing after trying to use a NULL pointer.
It also sets memory to zero.
===================
*/
void *Q_malloc (size_t size)
{
	void *p = malloc(size);
	//p = calloc(1, size); //malloc & memset or just calloc?
	if (!p)
		Sys_Error ("Q_malloc: Not enough memory free");
	memset(p, 0, size);

	return p;
}

void *Q_calloc (size_t n, size_t size)
{
	void *p = calloc(n, size);

	if (!p)
		Sys_Error ("Q_calloc: Not enough memory free");

	return p;
}

/*
===================
Q_strdup
===================
*/
char *Q_strdup (const char *src)
{
	char *p = strdup(src);

	if (!p)
		Sys_Error ("Q_strdup: Not enough memory free");
	return p;
}


/*
============
COM_StripExtension
============
*/
char *COM_StripExtension (char *str)
{
	char *p = strrchr(str, '.');

    /* truncate extension */
    if (p)
        *p = '\0';

    return str;
}

/*
============
COM_FileExtension
============
*/
char *COM_FileExtension (const char *in)
{
	static char exten[8];
	int i;

	in = strrchr(in, '.');
	if (!in || strchr(in, '/'))
		return "";
	in++;
	for (i=0 ; i<7 && *in ; i++,in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

/*
==================
COM_DefaultExtension

If path doesn't have a .EXT, append extension
(extension should include the .)
==================
*/
void COM_DefaultExtension (char *path, const char *extension)
{
	char *src;

	src = path + strlen (path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return; // it has an extension
		src--;
	}

	strlcat (path, extension, MAX_OSPATH);
}

//=====================================================

float AdjustAngle(float current, float ideal, float fraction)
{
	float move = ideal - current;

	if (move >= 180)
		move -= 360;
	else if (move <= -180)
		move += 360;

	return current + fraction * move;
}

//=======================================================

int wildcmp(char *wild, char *string)
{
	char *cp=NULL, *mp=NULL;

	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
		{
			return 0;
		}
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)   //a * at the end of the wild string matches anything the checked string has
			{
				return 1;
			}
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}

