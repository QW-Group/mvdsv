// Added by VVD
#ifndef _LOG
#define _LOG
#include <time.h>
enum {	MIN_LOG = 0, CONSOLE_LOG = 0, ERROR_LOG,  RCON_LOG,
	TELNET_LOG,  FRAG_LOG,        PLAYER_LOG, MAX_LOG};

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

//bliP: logging
void SV_Logfile (int sv_log, qboolean newlog);
void SV_LogPlayer(client_t *cl, char *msg, int level);
//<-
void	SV_Write_Log(int sv_log, int level, char *msg);
#endif
