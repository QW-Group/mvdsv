// version.h

#define GLQUAKE_VERSION 1.00
#define	QW_VERSION		2.40
#define QWE_VERSION	"0.172 Beta"
#define QWE_VERNUM 0.172
#define LINUX_VERSION 0.98

#define RELEASE_VERSION

#ifdef _WIN32
#define QW_PLATFORM	"Win32"
#else 
#define QW_PLATFORM	"Linux"
#endif

#ifdef GLQUAKE
#define QW_RENDERER	"GL"
#else
#define QW_RENDERER "Soft"
#endif


int build_number (void);
void CL_Version_f (void);
