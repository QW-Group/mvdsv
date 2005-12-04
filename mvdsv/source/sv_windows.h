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

	$Id: sv_windows.h,v 1.4 2005/12/04 07:46:59 disconn3ct Exp $
*/

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