#ifndef _CONSOLE

#include "resource.h"

#define MAX_NUM_ARGVS	50

extern HWND HEdit1;
extern COLORREF EditBoxBgColor, EditBoxColor;
extern HBRUSH g_hbrBackground;

extern HINSTANCE	global_hInstance;
extern HWND			DlgHwnd, mainWindow;
extern HWND			HEdit1, HEdit2;
extern HMENU		Menu;
extern qboolean		minimized;

BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CreateMainWindow(HINSTANCE hInstance, int nCmdShow);

void ConsoleAddText(char *text);
void ShowNotifyIcon(void);
void RemoveNotifyIcon(void);
void UpdateNotifyIconMessage(char *msg);
void CheckIdle(void);

#endif // _CONSOLE