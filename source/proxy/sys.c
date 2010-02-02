/*
System dependant stuff, live hard.
Also contain some "misc" functions, have no idea where to put it, u r welcome to sort out it.
*/
 
#include "qwfwd.h"

#ifdef APP_DLL
	#define QWFWD_PREFIX "QWFWD: "
#else
	#define QWFWD_PREFIX ""
#endif


#ifdef _WIN32

//FIXME: replace this shit with linux/FreeBSD code, so we will be equal on all OSes

int qsnprintf(char *buffer, size_t count, char const *format, ...)
{
	int ret;
	va_list argptr;
	if (!count)
		return 0;
	va_start(argptr, format);
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	va_end(argptr);
	return ret;
}

int qvsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	int ret;
	if (!count)
		return 0;
	ret = _vsnprintf(buffer, count, format, argptr);
	buffer[count - 1] = 0;
	return ret;
}

#endif // _WIN32

#if defined(__linux__) || defined(_WIN32) || defined(__CYGWIN__)

/* 
 * Functions strlcpy, strlcat
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

size_t strlcat(char *dst, char *src, size_t siz)
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

#endif // defined(__linux__) || defined(_WIN32) || defined(__CYGWIN__)

void Sys_Printf(char *fmt, ...)
{
	va_list		argptr;
	char		string[2048];
	unsigned char *t;
	
	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	for (t = (unsigned char*)string; *t; t++)
	{
		if (*t >= 146 && *t < 156)
			*t = *t - 146 + '0';
		if (*t == 143)
			*t = '.';
		if (*t == 157 || *t == 158 || *t == 159)
			*t = '-';
		if (*t >= 128)
			*t -= 128;
		if (*t == 16)
			*t = '[';
		if (*t == 17)
			*t = ']';
		if (*t == 29)
			*t = '-';
		if (*t == 30)
			*t = '-';
		if (*t == 31)
			*t = '-';
		if (*t == '\a')	//doh. :D
			*t = ' ';
	}

	printf(QWFWD_PREFIX "%s", string);
}

// print debug
// this is just wrapper for Sys_Printf(), but print nothing if developer 0
void Sys_DPrintf(char *fmt, ...)
{
	va_list		argptr;
	char		string[2048];
	
	if (!developer.integer)
		return;
	
	va_start (argptr, fmt);
	vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	Sys_Printf("%s", string);
}

void Sys_Exit(int code)
{
#ifdef APP_DLL
	#ifdef _WIN32
		ExitThread(code);
	#else
		pthread_exit(NULL); //hrm, that we should provide instead of NULL?
	#endif
#else
	exit(code);
#endif
}

void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[2048];

	va_start (argptr, error);
	vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	strlcat(text, "\n", sizeof(text));
	Sys_Printf("%s", text);

	Sys_Exit (1);
}


/*
===================
Sys_malloc

Use it instead of malloc so that if memory allocation fails,
the program exits with a message saying there's not enough memory
instead of crashing after trying to use a NULL pointer.
It also sets memory to zero.
===================
*/

#ifndef _CRTDBG_MAP_ALLOC

void *Sys_malloc (size_t size)
{
	void *p = malloc(size);

	if (!p)
		Sys_Error ("Sys_malloc: Not enough memory free");
	memset(p, 0, size);

	return p;
}

#endif // _CRTDBG_MAP_ALLOC

char *Sys_strdup (const char *src)
{
	char *p = strdup(src);

	if (!p)
		Sys_Error ("Sys_strdup: Not enough memory free");

	return p;
}

// qqshka - hmm, seems in C this is macroses, i don't like macroses,
// however this functions work wrong on unsigned types!!!

#ifdef KTX_MIN

double min( double a, double b )
{
	return ( a < b ? a : b );
}

#endif

#ifdef KTX_MAX

double max( double a, double b )
{
	return ( a > b ? a : b );
}

#endif

double bound( double a, double b, double c )
{
	return ( a >= c ? a : b < a ? a : b > c ? c : b);
}

// handle keyboard input
void Sys_ReadSTDIN(proxy_static_t *cluster, fd_set socketset)
{
#ifdef _WIN32

	for (;;)
	{
		char c;

		if (!_kbhit())
			break;
		c = _getch();

		if (c == '\n' || c == '\r')
		{
			Sys_Printf("\n");
			if (cluster->inputlength)
			{
				cluster->commandinput[cluster->inputlength] = '\0';
				Cbuf_InsertText(cluster->commandinput);

				cluster->inputlength = 0;
				cluster->commandinput[0] = '\0';
			}
		}
		else if (c == '\b')
		{
			if (cluster->inputlength > 0)
			{
				Sys_Printf("%c", c);
				Sys_Printf(" ", c);
				Sys_Printf("%c", c);

				cluster->inputlength--;
				cluster->commandinput[cluster->inputlength] = '\0';
			}
		}
		else
		{
			Sys_Printf("%c", c);
			if (cluster->inputlength < sizeof(cluster->commandinput)-1)
			{
				cluster->commandinput[cluster->inputlength++] = c;
				cluster->commandinput[cluster->inputlength] = '\0';
			}
		}
	}

#else

	if (FD_ISSET(STDIN, &socketset))
	{
		cluster->inputlength = read (STDIN, cluster->commandinput, sizeof(cluster->commandinput));
		if (cluster->inputlength >= 1)
		{
			cluster->commandinput[cluster->inputlength-1] = 0;        // rip off the /n and terminate
			cluster->inputlength--;

			if (cluster->inputlength)
			{
				cluster->commandinput[cluster->inputlength] = '\0';
				Cbuf_InsertText(cluster->commandinput);

				cluster->inputlength = 0;
				cluster->commandinput[0] = '\0';
			}
		}
	}

#endif
}

#ifdef _WIN32

static double pfreq;
static __int64 startcount;

static void Sys_InitDoubleTime (void)
{
	__int64 freq;

	if (QueryPerformanceFrequency ((LARGE_INTEGER *)&freq) && freq > 0)
	{
		// hardware timer available
		pfreq = (double)freq;
		QueryPerformanceCounter ((LARGE_INTEGER *)&startcount);

		Sys_DPrintf("Sys_InitDoubleTime: OK\n");
	}
	else
	{
		Sys_Error("could not initialize HW timer");
	}
}

double Sys_DoubleTime (void)
{

	__int64 pcount;

	if (!pfreq)
		Sys_InitDoubleTime();

	QueryPerformanceCounter ((LARGE_INTEGER *)&pcount);
	// TODO: check for wrapping; is it necessary?
	return (pcount - startcount) / pfreq;
}

#else

double Sys_DoubleTime (void)
{
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000000.0;
	}

	return (tp.tv_sec - secbase) + tp.tv_usec/1000000.0;
}

#endif

