/*
	main.c
	Original Author:	Chris Faherty <chrisf@america.com>
	Adopted by:			Ivan Bol'sunov aka qqshka
*/

#include "qwfwd.h"

#ifndef _WIN32
#define DWORD unsigned int
#define WINAPI
#endif

DWORD WINAPI FWD_proc(void *lpParameter)
{
	fwd_params_t *params = lpParameter;
	int port;
	const char *ip;

	if (!params)
		return 1;

	ip = params->ip;
	port = params->port;

	Sys_DoubleTime(); // init time
	NET_Init(ip, port);
	QRY_Init();

	Sys_Printf("QW FORWARD PROXY: Ready to rock at %s:%d\n", ip, port);

	while(1)
	{
		FWD_update_peers(); // do basic proxy job
		QRY_QueryMasters(); // request time to time server list from masters
		QRY_SV_PingServers(); // ping time to time normal qw servers
	}

	return 0;
}

int main(int argc, char *argv[])
{
	fwd_params_t params;

	#ifdef _CRTDBG_MAP_ALLOC
	{
		// report memory leaks on program exit in output window under MSVC
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	}
	#endif

	#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	#endif

	srand((unsigned) time (NULL));

	if (argc < 2)
	{
		Sys_Printf("Usage: %s <port> [ip]\n", argv[0]);
		return 1;
	}

	memset(&params, 0, sizeof(params));
	params.port = atoi(argv[1]);
	strlcpy(params.ip, argc > 2 ? argv[2] : "0.0.0.0", sizeof(params.ip));

	FWD_proc(&params);
	return 0;
}
