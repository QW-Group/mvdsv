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
 
	$Id: sv_windows.c,v 1.16 2006/06/19 16:46:16 vvd0 Exp $
*/

#ifndef _CONSOLE //bliP: console compile

#include "qwsvdef.h"

COLORREF EditBoxBgColor, EditBoxColor;
HBRUSH g_hbrBackground;

HINSTANCE	global_hInstance;
HWND		DlgHwnd;
HWND		HEdit1 = NULL, HEdit2 = NULL;
HMENU		Menu;
unsigned int	HEdit1_size = 0;
char		*HEdit1_buf = NULL;
qbool minimized = false;

qbool DrawConsole = false;

/*
=================
ConsoleAddText
 
Appends text to the main console
Changes will be redrawn in CheckIdle function
So that screen doesn't blink when there are more updates in one frame
also works much faster
=================
*/

void ConsoleAddText(char *text)
{
	int l, size;
	extern char chartbl2[]; // quake char translation map
	DrawConsole = true;

	SendMessage(HEdit1, WM_SETREDRAW, 0, 0);
	SendMessage(HEdit1, EM_SETREADONLY, 0, 0);

	//  set the carriage in the end of the text
	l = SendMessage(HEdit1, WM_GETTEXTLENGTH, 0, 0);

	size = l + strlen(text) + strchrn(text, '\n');
	if (HEdit1_size <= size)
	{
		SendMessage(HEdit1, WM_GETTEXT, HEdit1_size, (LPARAM)HEdit1_buf);
		SendMessage(HEdit1, WM_SETTEXT, 0, (LPARAM)(HEdit1_buf + size + 1 - HEdit1_size));
		l = SendMessage(HEdit1, WM_GETTEXTLENGTH, 0, 0);
	}

	SendMessage(HEdit1, EM_SETSEL, l, l);

	while (*text)
		SendMessage(HEdit1, WM_CHAR, chartbl2[(byte)(*text++)], 1);
	SendMessage(HEdit1, EM_SETREADONLY, 1, 0);
	SendMessage(HEdit1, WM_SETREDRAW, 1, 0);
}

#define WM_TRAY WM_USER + 19
static HICON icon;

/*
=================
CreateMainWindow
=================
*/

BOOL CreateMainWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASS		wc;

	icon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON2));

	/* Register the frame class */
	wc.style         = 0;
	wc.lpfnWndProc   = DefDlgProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = DLGWINDOWEXTRA;
	wc.hInstance     = hInstance;
	wc.hIcon         = icon;
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = SERVER_NAME;

	if (!RegisterClass (&wc) )
		Sys_Error ("Couldn't register window class");

	global_hInstance = hInstance;

	DlgHwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), NULL, (DLGPROC)DialogFunc);

	if (!DlgHwnd)
	{
		MessageBox(NULL, TEXT("Could not create dialog window"), TEXT("Error"), MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	EditBoxBgColor = RGB(0, 64, 64);
	EditBoxColor = RGB(255, 255, 255);
	g_hbrBackground = CreateSolidBrush(EditBoxBgColor);

	// is started with -minimize option, start in tray

	if (COM_CheckParm("-minimize") || (nCmdShow == SW_SHOWMINNOACTIVE))
	{
		ShowNotifyIcon();
		ShowWindow(DlgHwnd,SW_HIDE);
		UpdateWindow(DlgHwnd);
	}
	else
	{
		ShowWindow(DlgHwnd,SW_SHOWNORMAL);
		UpdateWindow(DlgHwnd);
	}

	// popup menu of tray icon
	Menu = CreatePopupMenu();
	AppendMenu(Menu, MF_STRING, IDC_RESTORE, "Restore");
	//AppendMenu(Menu, MF_STRING, 0, "about");
	AppendMenu(Menu, MF_SEPARATOR, 0, NULL);
	AppendMenu(Menu, MF_STRING, IDC_QUIT, "Quit");

	return 1;
}

/*
=================
RemoveNotifyIcon
=================
*/
void RemoveNotifyIcon(void)
{
	NOTIFYICONDATA tnid;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = DlgHwnd;
	tnid.uID = IDI_ICON2;

	Shell_NotifyIcon(NIM_DELETE, &tnid);

	minimized = false;
}


/*
=================
ShowNotifyIcon
=================
*/

void ShowNotifyIcon(void)
{
	NOTIFYICONDATA tnid;
	extern int sv_port;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = DlgHwnd;
	tnid.uID = IDI_ICON2;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = WM_TRAY;
	tnid.hIcon = icon;
	lstrcpyn(tnid.szTip, va(SERVER_NAME ":%d", sv_port), sizeof(tnid.szTip));

	Shell_NotifyIcon(NIM_ADD, &tnid);

	minimized = true;
}

void UpdateNotifyIconMessage(char *msg)
{
	NOTIFYICONDATA tnid;
	extern int sv_port;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = DlgHwnd;
	tnid.uID = IDI_ICON2;
	tnid.uFlags = NIF_TIP;
	lstrcpyn(tnid.szTip, msg, sizeof(tnid.szTip));

	Shell_NotifyIcon(NIM_MODIFY, &tnid);
}

/*
=================
TrackPopup
 
Tracks popup menu, this is called as thread, so it doesn't lag server
=================
*/

DWORD WINAPI TrackPopup(LPVOID param)
{
	static qbool running = false;
	POINT point;
	HWND win;
	int result;

	// one pup at one time is much enough
	if (running)
		return 0;

	running = true;

	// we can't create menu of a window from another thread,
	// so we need to make a temporary window here

	win = CreateWindow(SERVER_NAME, "", 0,0,0,0,0,NULL, NULL, global_hInstance, NULL);

	GetCursorPos(&point);
	result = TrackPopupMenu(Menu, TPM_RETURNCMD|TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x, point.y, 0, win, NULL);

	DestroyWindow( win );

	if (result)
		PostMessage(DlgHwnd, WM_COMMAND, result, 0);

	running = false;

	return 0;
}

/*
=================
SetWindowText_
=================
*/

void SetWindowText_(char *text)
{
	SetWindowText(DlgHwnd, text);
}

/*
=================
CheckIdle
 
Called every frame
=================
*/


void CheckIdle(void)
{
	// we update scroll bar position here, and draw console edit box
	if (DrawConsole)
	{
		int i;

		i = SendMessage(HEdit1, EM_GETLINECOUNT , 0,0) - 22;
		if (i > 0)
			SendMessage(HEdit1, EM_LINESCROLL, 0, (LPARAM)i);
		//SendMessage(HEdit1, WM_SETREDRAW, 1, 0);
		DrawConsole = false;
	}
}

/*
=================
DialogFunc
 
Main window procedure
=================
*/
void SV_Quit_f(void);

BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		HEdit1 = GetDlgItem(hwndDlg, IDC_EDIT1);
		HEdit2 = GetDlgItem(hwndDlg, IDC_EDIT2);

		SetFocus(HEdit2);

//		SendMessage(HEdit1, EM_LIMITTEXT, 1000, 0);
		HEdit1_size = SendMessage(HEdit1, EM_GETLIMITTEXT, 0, 0) + 1;
		HEdit1_buf = (char *) Q_malloc (HEdit1_size);
//Sys_Printf("%d\n", HEdit1_size);
		break;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam != HEdit1)
			break;

		SetTextColor((HDC)wParam, EditBoxColor);
		SetBkColor((HDC)wParam, EditBoxBgColor);

		return (LONG)g_hbrBackground;
	case WM_TRAY:
		switch (lParam)
		{
		case 515:
			ShowWindow(hwndDlg,SW_RESTORE);
			SetForegroundWindow(hwndDlg);
			RemoveNotifyIcon();
			break;
		case 516:
			{
				static DWORD id;

				CreateThread(NULL, 0, TrackPopup, NULL, 0, &id);
				break;
			}
		}
		break;
	case WM_SIZE:
		// we don't care until window is fully created
		if (DlgHwnd == NULL)
			break;

		if ((int)wParam == SIZE_MINIMIZED)
		{
			ShowWindow(hwndDlg,SW_HIDE);
			ShowNotifyIcon();
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_OK:
			{
				char str[1024];

				SendMessage(HEdit2, WM_GETTEXT, (WPARAM)sizeof(str),(LPARAM)str);

				if (!str[0])
					break;

				SendMessage(HEdit2, WM_SETTEXT, 0, (LPARAM)0);

				ConsoleAddText(va("] %s\n", str));

				Cbuf_AddText (str);
				Cbuf_AddText ("\n");

				return TRUE;
			}
		case IDC_QUIT:
			Cbuf_AddText("quit\n");
			return TRUE;
		case IDC_RESTORE:
			ShowWindow(hwndDlg,SW_RESTORE);
			RemoveNotifyIcon();
			return TRUE;
		case IDC_CLEAR:
			SendMessage(HEdit1, WM_SETTEXT, 0, (LPARAM)0);
			SetFocus(HEdit2);
			break;
		}
		break;
		/*case WM_LBUTTONDOWN:{
			RECT metrix;

			GetWindowRect(HEdit1, &metrix);
			ConsoleAddText(va("%ld %ld %ld %ld, %d %d\n", metrix.left,
				metrix.right,
				metrix.top,
				metrix.bottom,
				(int) LOWORD(lParam),
				(int) HIWORD(lParam)));
			if  (metrix.left > LOWORD(lParam) ||
				metrix.right < LOWORD(lParam) ||
				metrix.top > HIWORD(lParam) ||
				metrix.bottom < HIWORD(lParam))
				break;

			return TRUE;
			break;
							}
		*/
	case WM_ACTIVATE:
		break;

	case WM_CLOSE:
		SV_Quit_f();
		break;
	}

	return FALSE;
}

#endif /* !_CONSOLE */ //bliP: console compile
