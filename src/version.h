/*

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
// version.h

#ifndef __VERSION_H__
#define __VERSION_H__


#if defined(_WIN32)
#define QW_PLATFORM			"Win32"
#define QW_PLATFORM_SHORT	"(w)"

#elif defined(__FreeBSD__)
#define QW_PLATFORM			"FreeBSD"
#define QW_PLATFORM_SHORT	"(f)"

#elif defined(__OpenBSD__)
#define QW_PLATFORM			"OpenBSD"
#define QW_PLATFORM_SHORT	"(o)"

#elif defined(__NetBSD__)
#define QW_PLATFORM			"NetBSD"
#define QW_PLATFORM_SHORT	"(n)"

#elif defined(__DragonFly__)
#define QW_PLATFORM			"DragonFly"
#define QW_PLATFORM_SHORT	"(d)"

#elif defined(__linux__)
#define QW_PLATFORM			"Linux"
#define QW_PLATFORM_SHORT	"(l)"

#elif defined(__sun__)
#define QW_PLATFORM			"SunOS"
#define QW_PLATFORM_SHORT	"(s)"

#elif defined(__APPLE__)
#define QW_PLATFORM			"MacOSX" // Darwin was changed to MacOSX
#define QW_PLATFORM_SHORT	"(m)"

#else
#define QW_PLATFORM			"Unknown"
#define QW_PLATFORM_SHORT	"(u)"
#endif


#define QW_VERSION          "2.40"
#define VERSION_NUMBER      "0.32"
#define VERSION_NUM         0.32
#define SERVER_NAME         "MVDSV"

#define PROJECT_NAME        SERVER_NAME
#define PROJECT_URL         "https://github.com/deurk/mvdsv"
#define BUILD_DATE          "Build date: " __DATE__ ", " __TIME__

#define QWDTOOLS_NAME       "QWDtools"

char *VersionString (void);
char *VersionStringFull (void);
void Version_f (void);

#endif /* !__VERSION_H__ */

