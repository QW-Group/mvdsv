// Added by VVD
#ifndef _LOG
#define _LOG
#include <time.h>
#define	CONSOLE_LOG		0
#define	ERROR_LOG		1
#define	RCON_LOG		2
#define	TELNET_LOG		3
#define	FRAG_LOG		4
#define	MAX_LOG			5

typedef struct log_s {
	FILE		*sv_logfile;
	char		*command;
	char		*file_name;
	char		*message_off;
	char		*message_on;
	xcommand_t	function;
	int			log_level;
} log_t;
extern	log_t	logs[];
extern	cvar_t	frag_log_type;
extern	cvar_t	telnet_log_level;
extern	int		port;
void	SV_Write_Log(int sv_log, int level, char *msg);
#endif
