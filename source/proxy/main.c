/*
	main.c
	Original Author:	Chris Faherty <chrisf@america.com>
	Adopted by:			Ivan Bol'sunov aka qqshka
*/

#include "qwfwd.h"

DWORD WINAPI FWD_proc(void *lpParameter)
{
	fwd_params_t *params = lpParameter;
	int port;

	if (!params)
		return 1;

	port = params->port;

	NET_Init(port);

	Sys_Printf("QW FORWARD PROXY: Ready to rock at port %d\n", port);

	while(1)
	{
		FWD_update_peers();
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
		Sys_Printf("Usage: %s <port>\n", argv[0]);
		return 1;
	}

	memset(&params, 0, sizeof(params));
	params.port = atoi(argv[1]);

	FWD_proc(&params);
	return 0;
}
