// version.h

#define GLQUAKE_VERSION 1.00
#define	QW_VERSION		2.40
#define QWE_VERSION	"0.1723 Beta"
#define QWE_VERNUM 0.1723
#define LINUX_VERSION 0.98

#define RELEASE_VERSION

#ifdef _WIN32
#define QW_PLATFORM	"Win32"
#endif

#ifdef __FreeBSD__
#define QW_PLATFORM	"FreeBSD"
#endif

#ifdef __linux__
#define QW_PLATFORM	"Linux"
#endif

#ifdef sun
#define QW_PLATFORM	"Sun"
#endif

#ifdef GLQUAKE
#define QW_RENDERER	"GL"
#else
#define QW_RENDERER "Soft"
#endif


int build_number (void);
void CL_Version_f (void);
