//Windows Procedures
//Copyright (C) 2008  KeyMagic Project
//http://keymagic.googlecode.com
//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "MyMenu.h"
#include "MyButton.h"
#include "WndProc.h"
#include "CGdiPlusBitmap.h"
#include <sys/stat.h>
#include <errno.h>
#include "DllUnload.h"
#include "../global/global.h"
#include "StrTypeFunc.h"

using namespace std;

//strDelete *szFileToDelete;
vector<TCHAR*> szFileToDelete;
//int cbFileToDelete = 0;
int kbindex=-1;
static HWND LastHWND;
static UINT KeyBoardNum;
static DWORD StartupFlag;

CGdiPlusBitmapResource *Logo;
Gdiplus::Bitmap *Bmpbk;

int WndHeight, WndWidth;
HWND	hList, hPath,
		hDisplay, hShortcut,
		hAdd, hRemove,
		hDone, hCancel,
		hApply, hPathChk,
		//hLogo, hBK,
		hgbKeyboard, hgbOption,
		hgbHotkey, hgbDisplay,
		hgbLocation;

DWORD WINAPI TWM (LPVOID lpParameter);
HANDLE hThread=0;
DWORD ThreadID=0;
int txPos=0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;
	int wmId, wmEvent;
	KeyFileData *Data;
	wchar_t name[100];

	switch (message)
	{
	case KM_KILLFOCUS:
		LastHWND = (HWND)lParam;
		break;

	case KM_GETFOCUS:
		GetClassName((HWND)lParam, name, 100);
		Debug(TEXT("lParam(HWND)=0x%.8x, wParam=%d, Class=%s\n"), lParam, wParam, name);
		if (wParam == -1) wParam = 0;
		CheckMenuRadioItem(hKeyMenu, IDKM_NORMAL,
			KeyBoardNum + IDKM_ID , 
			wParam + IDKM_NORMAL, 
			MF_BYCOMMAND);
		SendMessage((HWND)lParam, KM_RESCAN, 0, 0);
		break;

	case KM_ERR_KBLOAD:
			ShowBallonTip(hWnd, szError, szKBLoad_ERR, NIIF_ERROR);
		break;

	case WM_CREATE:
		OnCreate(hWnd,(LPCREATESTRUCT) lParam);
		Up.Fill = RGB(240,250,255);
		Up.Frame = RGB(153,217,234);
		Up.Text = RGB(10,120,245);
		Down.Fill = RGB(210,240,255);
		Down.Frame = RGB(110,205,220);
		Down.Text = RGB(10,120,245);
		break;

	case WM_SYSCOMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		}
		break;

	case WM_SHOWWINDOW:
		ShowNotifyIcon(hWnd, TEXT("KeyMagic"), MAKEINTRESOURCE(IDI_KEYMAGIC));
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId){
		case IDC_APPLY:
			
			if (!bAdmin)
			{
				MessageBox (hWnd,
					TEXT("Sorry! You do not have permission to change.\n")
					TEXT("Please turn off UAC or run Keymagic as an Administrator."),
					TEXT("Keymagic"), MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			__try
			{
				if (kbindex == -1)
					__leave;

				Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, kbindex, 0);
				SendDlgItemMessage(hWnd, IDC_DISPLAY, WM_GETTEXT, 30, (LPARAM)Data->Display);
				Data->wHotkey = (WORD)SendDlgItemMessage(hWnd, IDC_SHORTCUT, HKM_GETHOTKEY, 0, 0);
				SendDlgItemMessage(hWnd, IDC_PATH, WM_GETTEXT, MAX_PATH, (LPARAM)Data->Path);
			}

			__finally
			{
				SetKbData(hWnd);
				DeleteKeyFile();
				restart(hWnd);
				SendMessage(LastHWND, KM_RESCAN, 0, 0);
			}

			break;

		case IDC_DONE:

			if (!bAdmin)
			{
				MessageBox (hWnd,
					TEXT("Sorry! You do not have permission to change.\n")
					TEXT("If you are on Windows Vista, turn off UAC or run Keymagic as an Administrator."),
					TEXT("Keymagic"), MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			__try
			{
				if (kbindex == -1)
					__leave;

				hList = GetDlgItem(hWnd, IDC_KEYBOARDS);

				Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, kbindex, 0);
				SendDlgItemMessage(hWnd, IDC_DISPLAY, WM_GETTEXT, 30, (LPARAM)Data->Display);
				Data->wHotkey = (WORD)SendDlgItemMessage(hWnd, IDC_SHORTCUT, HKM_GETHOTKEY, 0, 0);
				SendDlgItemMessage(hWnd, IDC_PATH, WM_GETTEXT, MAX_PATH, (LPARAM)Data->Path);
			}

			__finally
			{
				SetKbData(hWnd);
				ShowWindow(hWnd, SW_HIDE);
				DeleteKeyFile();
				restart(hWnd);
				SendMessage(LastHWND, KM_RESCAN, 0, 0);
			}

			break;

		case IDC_CANCEL:
			restart(hWnd);
			ShowWindow(hWnd, SW_HIDE);
			break;

		case IDC_KEYBOARDS:
			if (wmEvent == LBN_SELCHANGE){

				CheckDlgButton(hWnd, IDC_DIR, false);
				EnableWindow(hPath, false);
				if (kbindex == -1)
					goto next;
				
				Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, kbindex, 0);
				if (!Data)
					goto next;

				SendMessage(hDisplay, WM_GETTEXT, 30, (LPARAM)Data->Display);
				Data->wHotkey = (WORD)SendMessage(hShortcut, HKM_GETHOTKEY, 0, 0);
				SendMessage(hPath, WM_GETTEXT, MAX_PATH, (LPARAM)Data->Path);
next:
				kbindex = SendMessage(hList, LB_GETCURSEL, 0, 0);
				if (kbindex == -1)
					break;
				
				Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, kbindex, 0);
				if (!Data)
					break;
				SendMessage(hDisplay, WM_SETTEXT, 0, (LPARAM)Data->Display);
				SendMessage(hShortcut, HKM_SETHOTKEY, Data->wHotkey, 0);
				SendMessage(hPath, WM_SETTEXT, 0, (LPARAM)Data->Path);
				if (Data->Path[1] == ':'){
					EnableWindow(hPath, true);
					CheckDlgButton(hWnd, IDC_DIR, true);
				}
			}
			break;

		case IDC_DIR:
			UINT state;
			state = IsDlgButtonChecked(hWnd, IDC_DIR);
			if (state == BST_CHECKED)
				EnableWindow(hPath, true);
			else
				EnableWindow(hPath, false);
			break;

		case IDC_ADD:
			TCHAR szFileName[MAX_PATH];
			if (!OpenDialog(hWnd, szFileName, MAX_PATH))
				break;

			AddKeyBoardToList(hWnd ,szFileName);
			SendMessage(hDisplay, EM_SETMODIFY, TRUE, 0);
			
			break;

		case IDC_REMOVE:

			RemoveKeyBoard();
			
			break;

		case IDKM_NORMAL:

			Debug(TEXT("LastHWND=0x%.8x, wmId=%d, Active=%d\n"), LastHWND, -1, false);
			SendMessage(LastHWND, KM_SETKBID, -1, false);
			CheckMenuRadioItem(hKeyMenu, IDKM_NORMAL,
				KeyBoardNum + IDKM_ID, 
				IDKM_NORMAL, 
				MF_BYCOMMAND);

			break;

		default:
			if (wmId >= IDKM_ID && wmId <= IDKM_ID + KeyBoardNum)
			{

				Debug(TEXT("LastHWND=0x%.8x, wmId=%d, Active=%d\n"),LastHWND, wmId, true);
				SendMessage(LastHWND, KM_SETKBID, wmId - IDKM_NORMAL, true);

				if (MF_CHECKED & GetMenuState(hKeyMenu, wmId, MF_BYCOMMAND))
					wmId = IDKM_NORMAL;

				CheckMenuRadioItem(hKeyMenu, IDKM_NORMAL,
					KeyBoardNum + IDKM_ID,
					wmId,
					MF_BYCOMMAND);
			}
		}
		break;

	case WM_PAINT:
		onPaint(hWnd);
		break;

	case WM_TRAY:

		if (lParam==WM_LBUTTONDBLCLK){
			if (IsWindowVisible(hWnd))
				break;
			if (!bAdmin)
				MessageBox(hWnd, TEXT("Attention: To save changes, please run Keymagic as an administration. ")
				TEXT("If not, any changes you have made will be unsaved."), TEXT("Keymagic"), MB_OK | MB_ICONEXCLAMATION);
			ShowWindow(hWnd, SW_SHOW);
		}

		else if (lParam==WM_LBUTTONDOWN) {
			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hKeyMenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
				pt.x, pt.y, 0, hWnd, NULL);
		}

		else if (lParam==WM_RBUTTONDOWN) {
			hMenu = CreatePopupMenu();
			int isPortable = GetPrivateProfileInt(TEXT("Settings"), TEXT("Portable"), 0, szINIFile);

			AppendMenu(hMenu, MF_BYCOMMAND, RMCMD_MANAGE, TEXT("Manage &Keyboards"));
			if (!isPortable)
				AppendMenu(hMenu, MF_BYCOMMAND, RMCMD_STARTUP, TEXT("&Run at Startup"));
			AppendMenu(hMenu, MF_BYCOMMAND, RMCMD_ABOUT, TEXT("&About"));
			AppendMenu(hMenu, MF_BYCOMMAND, RMCMD_EXIT, TEXT("E&xit"));
			CreateMyMenu(hMenu);

			if (!isPortable){
				StartupFlag = GetPrivateProfileInt(TEXT("Settings"), TEXT("Startup"), 0, szINIFile);
				StartupFlag ? CheckMenuItem(hMenu, 102, MF_CHECKED) : CheckMenuItem(hMenu, 102, MF_UNCHECKED);
			}
			else{
				WritePrivateProfileString(TEXT("Settings"), TEXT("Startup"), TEXT("0"), szINIFile);
				SetStartup(false);
			}

			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);

			RightMenuCmd CMD_RETURN = (RightMenuCmd)TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_RETURNCMD,
				pt.x, pt.y, 0, hWnd, NULL);

			switch (CMD_RETURN){
				case RMCMD_EXIT:
					UnHook();
					DeleteNotifyIcon(hWnd);
					DestroyMyMenu(hKeyMenu);
					DeleteDlgData(hWnd);
					EndDialog(hWnd, 0);
					PostQuitMessage(0);
					//ScannerAndInject();
					break;
				case RMCMD_MANAGE:
					if (IsWindowVisible(hWnd))
						break;
					if (!bAdmin)
						MessageBox(hWnd, TEXT("Attention: To save changes, please run Keymagic as an administration. ")
						TEXT("If not, any changes you have made will be unsaved."), TEXT("Keymagic"), MB_OK | MB_ICONEXCLAMATION);
					ShowWindow(hWnd, SW_SHOW);
					break;
				case RMCMD_STARTUP:
					if (bAdmin)
					{
						StartupFlag =! StartupFlag;
						SetStartup(StartupFlag);
					}
					else
					MessageBox (hWnd,
						TEXT("Sorry! You do not have permission to change.\n")
						TEXT("Please turn off UAC or run Keymagic as an Administrator."),
						TEXT("Keymagic"), MB_OK | MB_ICONEXCLAMATION);
					break;
				case RMCMD_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
			}
			DestroyMyMenu(hMenu);
			DestroyMenu(hMenu);
			PostMessage(hWnd, WM_NULL, 0, 0);
		}
		break;

	case WM_CTLCOLOREDIT:
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	case WM_CTLCOLORSTATIC:
	//	SetBkColor((HDC)wParam, RGB(236, 244, 255));
		return (INT_PTR)CreateSolidBrush(RGB(236, 244, 255));

	case WM_MEASUREITEM:
		OnMenuMeasure(hWnd, (LPMEASUREITEMSTRUCT)lParam);
		break;

	case WM_DRAWITEM:
		DrawMyMenu((LPDRAWITEMSTRUCT)lParam);
		DrawMyButton((LPDRAWITEMSTRUCT)lParam);

		break;

	case WM_GETMINMAXINFO:
		LPMINMAXINFO mminfo;
		mminfo = (LPMINMAXINFO)lParam;
		mminfo->ptMinTrackSize.x = 350;
		mminfo->ptMinTrackSize.y = 350;
		break;

	case WM_SIZE:
		OnSize(wParam, lParam);
		break;

	case WM_CLOSE:
		UnHook();
		restart(hWnd);
		ShowWindow(hWnd, SW_HIDE);
		if (hThread){
			PostThreadMessage(ThreadID, WM_QUIT, 0, 0);
			TerminateThread(hThread, 0);
			hThread=0;
		}
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void OnCreate(HWND hWnd, LPCREATESTRUCT lpcs)
{
	//OSVERSIONINFO osvi;
	//BOOL bIsWindowsVistaLater;
	HMENU hMenu;
	KeyFileData *Data;
	TCHAR szKBPath[MAX_PATH];

	//ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    //osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    //GetVersionEx(&osvi);
    //bIsWindowsVistaLater = osvi.dwMajorVersion >= 6;

	LoadIcon(hInst, (LPCTSTR)IDI_KEYMAGIC);

	if (!GetModuleFileName(GetModuleHandle(NULL), szKBPath, MAX_PATH))
		goto next;

	for (int i=lstrlen(szKBPath); szKBPath[i] != '\\'; i--){
		szKBPath[i] = NULL;
		if (szKBPath[i-1] == '\\'){
			szKBPath[i-1] = NULL;
			break;
		}
	}

	CreateDirectory(szKBPath, NULL);
	PathAppend(szKBPath, TEXT("Keyboards"));
	CreateDirectory(szKBPath, NULL);
next:
	GetKeyBoards();

	if (hKeyMenu){
		ShowNotifyIcon(hWnd, TEXT("KeyMagic"), MAKEINTRESOURCE(IDI_KEYMAGIC));
		UpdateWindow(hWnd);
	}

	SendMessage(hWnd,WM_SETICON, 1,(LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_KEYMAGIC)));

	hMenu = GetSystemMenu(hWnd, FALSE);

	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_BYCOMMAND, IDM_ABOUT, TEXT("&About"));
	CreateMyMenu(hKeyMenu);

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -MulDiv(8, GetDeviceCaps(GetDC(hWnd), LOGPIXELSY), 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
	lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	lf.lfQuality = PROOF_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE;
	lstrcpy(lf.lfFaceName, TEXT("Parabaik"));

	HFONT hf = CreateFontIndirect(&lf);

	hgbKeyboard = CreateWindowEx(0, TEXT("Button"), TEXT("Installed Keyboard"),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0,0,0,0, hWnd, (HMENU) 1, lpcs->hInstance, NULL);

	SendMessage(hgbKeyboard, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hgbOption = CreateWindowEx(0, TEXT("Button"), TEXT("Options"),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0,0,0,0, hWnd, (HMENU) 1, lpcs->hInstance, NULL);

	SendMessage(hgbOption, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hgbHotkey = CreateWindowEx(0, TEXT("Button"), TEXT("Hotkey"),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0,0,0,0, hWnd, (HMENU) 1, lpcs->hInstance, NULL);

	SendMessage(hgbHotkey, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hgbDisplay = CreateWindowEx(0, TEXT("Button"), TEXT("Menu Text"),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0,0,0,0, hWnd, (HMENU) 1, lpcs->hInstance, NULL);

	SendMessage(hgbDisplay, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hgbLocation = CreateWindowEx(0, TEXT("Button"), TEXT("Keyboard Path"),
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
		0,0,0,0, hWnd, (HMENU) 1, lpcs->hInstance, NULL);

	SendMessage(hgbLocation, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hList = CreateWindowEx (0, TEXT("ListBox"), NULL,
		WS_VSCROLL | WS_BORDER | WS_CHILD | WS_VISIBLE | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY ,
		0, 0, 0, 0, hWnd, (HMENU)IDC_KEYBOARDS,
		lpcs -> hInstance, NULL);

	SendMessage(hList, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hPath = CreateWindowEx (WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		0, 0, 0, 0, hWnd, (HMENU)IDC_PATH,
		lpcs -> hInstance, NULL);

	SendMessage(hPath, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));
	EnableWindow(hPath, false);

	hDisplay = CreateWindowEx (WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
		WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		0, 0, 0, 0, hWnd, (HMENU)IDC_DISPLAY,
		lpcs -> hInstance, NULL);

	SendMessage(hDisplay, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hShortcut = CreateWindowEx (WS_EX_CLIENTEDGE, HOTKEY_CLASS, NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hWnd, (HMENU)IDC_SHORTCUT,
		lpcs -> hInstance, NULL);

	SendMessage(hShortcut, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hAdd = CreateWindowEx (0, TEXT("Button"), TEXT("Add"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		0, 0, 0, 0, hWnd, (HMENU)IDC_ADD,
		lpcs -> hInstance, NULL);

	SendMessage(hAdd, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hRemove = CreateWindowEx (0, TEXT("Button"), TEXT("Remove"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		0, 0, 0, 0, hWnd, (HMENU)IDC_REMOVE,
		lpcs -> hInstance, NULL);

	SendMessage(hRemove, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hDone = CreateWindowEx (0,TEXT("Button"), bAdmin ? TEXT("OK") : TEXT("(SI) OK"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		0, 0, 0, 0, hWnd, (HMENU)IDC_DONE,
		lpcs -> hInstance, NULL);

	SendMessage(hDone, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hCancel = CreateWindowEx (0, TEXT("Button"), TEXT("Cancel"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		0, 0, 0, 0, hWnd, (HMENU)IDC_CANCEL,
		lpcs -> hInstance, NULL);

	SendMessage(hCancel, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hApply = CreateWindowEx (0, TEXT("Button"), bAdmin ? TEXT("Apply") : TEXT("(SI) Apply"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		0, 0, 0, 0, hWnd, (HMENU)IDC_APPLY,
		lpcs -> hInstance, NULL);

	SendMessage(hApply, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	hPathChk = CreateWindowEx (0, TEXT("Button"), TEXT("Use from this path"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_AUTOCHECKBOX,
		0, 0, 0, 0, hWnd, (HMENU)IDC_DIR,
		lpcs -> hInstance, NULL);

	SendMessage(hPathChk, WM_SETFONT, (WPARAM)hf, MAKEWORD(0,1));

	//hBK = CreateWindowEx (0, TEXT("Static"), NULL,
	//	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_REALSIZEIMAGE,
	//	0, 0, 0, 0, hWnd, (HMENU)0,
	//	lpcs -> hInstance, NULL);

	//SendMessage(hBK, STM_SETIMAGE, IMAGE_BITMAP,
	//	(LPARAM)LoadImage(lpcs->hInstance,
	//	MAKEINTRESOURCE(IDB_BK), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE));

	Logo = new CGdiPlusBitmapResource;
	Logo->Load(IDB_LOGO, TEXT("PNG"), lpcs -> hInstance);

	Bmpbk = new Gdiplus::Bitmap(lpcs -> hInstance, MAKEINTRESOURCE(IDB_BK));

	UpdateDlgData(hWnd);

	SetHook(hWnd);
}

DWORD WINAPI TWM (LPVOID lpParameter)
{
	PAINTSTRUCT ps;
	MSG message;

	TCHAR szTWM[] = TEXT("type with me....");
	int lnTWM = lstrlen(szTWM);

	//Sleep(1000);

	HDC local_hdc_window = GetDC((HWND)lpParameter);
	HDC local_hdc_text = CreateCompatibleDC(GetDC(0));
	HDC local_hdc_window_copy = CreateCompatibleDC(GetDC(0));

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -MulDiv(10, GetDeviceCaps(local_hdc_text, LOGPIXELSY), 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
	lf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	lf.lfQuality = PROOF_QUALITY;
	lf.lfPitchAndFamily = FF_DONTCARE;
	lstrcpy(lf.lfFaceName, TEXT("Microsoft Sans Serif"));

	HFONT hf = CreateFontIndirect(&lf);
	SelectObject(local_hdc_text, hf);

	SIZE size;
	GetTextExtentPoint(local_hdc_text, szTWM, lnTWM, &size);

	SetBkMode(local_hdc_text, TRANSPARENT);
	SetTextColor(local_hdc_text, RGB(255,255,255));

	//restart:

	SelectObject(local_hdc_text, CreateCompatibleBitmap(local_hdc_window, size.cx, size.cy));

	SelectObject(local_hdc_window_copy, CreateCompatibleBitmap(local_hdc_window, size.cx, size.cy));
	BitBlt(local_hdc_window_copy, 0,0, size.cx, size.cy, local_hdc_window, 210, 33, SRCCOPY);

	loop:
	for (txPos; txPos < lnTWM; txPos++){
		BitBlt(local_hdc_text, 0, 0, size.cx,size.cy,local_hdc_window_copy, 0, 0, SRCCOPY);
		TextOut(local_hdc_text, 0, 0, szTWM, txPos);
		BitBlt(local_hdc_window, 210, 33, size.cx, size.cy, local_hdc_text, 0,0, SRCCOPY);

		if (szTWM[txPos] != ' ') Sleep(400);
		else Sleep(100);

		if (PeekMessage(&message, (HWND)-1, WM_QUIT, WM_QUIT, PM_REMOVE))
			goto Exit;
	}
	txPos=0;
	Sleep(700);
	goto loop;

	Exit:

	BitBlt(local_hdc_window, 210, 33, size.cx, size.cy, local_hdc_window_copy, 0,0, SRCCOPY);

	ReleaseDC((HWND)lpParameter, local_hdc_window);
	DeleteDC(local_hdc_text);
	DeleteDC(local_hdc_window_copy);

	ExitThread(true);
}

VOID onPaint(HWND hWnd)
{
	HDC	hdc;
	PAINTSTRUCT ps;
	Gdiplus::Graphics*	pGraphics;

	//hdc = BeginPaint(hWnd, &ps);
	pGraphics = new Gdiplus::Graphics(GetDC(hWnd));
	pGraphics->FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(236, 244, 255)), 0, 0, WndWidth, Bmpbk->GetHeight());

	pGraphics->DrawImage(Bmpbk, 0, 0, WndWidth, Bmpbk->GetHeight());
	pGraphics->DrawImage(*Logo, 0, 0, Logo->m_pBitmap->GetWidth(), Logo->m_pBitmap->GetHeight());

	pGraphics->~Graphics();
	delete pGraphics;
	
	if (hThread){
		PostThreadMessage(ThreadID, WM_QUIT, 0, 0);
		TerminateThread(hThread, 0);
		hThread=0;
	}
	
	hThread = CreateThread(NULL, NULL, TWM, hWnd, 0, &ThreadID);

	//EndPaint(hWnd, &ps);

}

VOID OnSize(WPARAM wParam, LPARAM lParam)
{
	int TopBorder,
		BottomBorder,
		RightBorder,
		LeftBorder,
		
		Width,
		Height,
		HalfWidth,
		HalfHeight,

		BtnHeight,
		EditHeight;

	TopBorder = 70;
	BottomBorder = 5;
	RightBorder = 5;
	LeftBorder = 5;

	BtnHeight = 24;
	EditHeight = 24;

	WndHeight = HIWORD(lParam);
	WndWidth = LOWORD(lParam);

	Height = HIWORD(lParam) - (TopBorder + BottomBorder);
	Width = LOWORD(lParam) - (LeftBorder + RightBorder);

	HalfWidth = Width / 2;
	HalfHeight = Height / 2;

	float ListWidth = Width;

	ListWidth /= 6;

	//MoveWindow(hBK,
	//	0, 0, LOWORD(lParam), 67, TRUE);

	//InvalidateRect(hLogo, 0, TRUE);

	MoveWindow(hgbKeyboard,
		LeftBorder,
		TopBorder,
		HalfWidth - LeftBorder - ListWidth,
		Height - (BtnHeight + BottomBorder) - 10,
		TRUE);

	MoveWindow(hgbOption,
		HalfWidth - ListWidth + LeftBorder,
		TopBorder,
		HalfWidth + ListWidth,
		Height - (BtnHeight + BottomBorder) - 10,
		TRUE);

	MoveWindow(hgbHotkey,
		HalfWidth - ListWidth + LeftBorder + 10,
		TopBorder + 15,
		HalfWidth + ListWidth - 20,
		50,
		TRUE);

	MoveWindow(hgbDisplay,
		HalfWidth - ListWidth + LeftBorder + 10,
		Height / 2 + 22,
		HalfWidth + ListWidth - 20,
		50,
		TRUE);

	MoveWindow(hList,
		LeftBorder + 10,
		TopBorder + 20,
		HalfWidth - LeftBorder - ListWidth - 20,
		Height - (BtnHeight + BottomBorder) - 40,
		TRUE);

	MoveWindow(hShortcut,
		HalfWidth + LeftBorder - ListWidth + 20,
		TopBorder + 32,
		ListWidth + HalfWidth - RightBorder - 33,
		EditHeight,
		TRUE);

	MoveWindow(hDisplay,
		HalfWidth + LeftBorder - ListWidth + 20,
		(TopBorder + HalfHeight) - (EditHeight + BottomBorder) - 3,
		ListWidth + HalfWidth - RightBorder - 33,
		EditHeight, TRUE);

	MoveWindow(hgbLocation,
		HalfWidth - ListWidth + LeftBorder + 10,
		(TopBorder + Height) - (EditHeight + BottomBorder + BtnHeight) - 60,
		HalfWidth + ListWidth - 20,
		70,
		TRUE);

	MoveWindow(hPath,
		HalfWidth + LeftBorder - ListWidth + 20,
		(TopBorder + Height) - (EditHeight + BottomBorder + BtnHeight) - 20,
		ListWidth + HalfWidth - RightBorder - 33,
		EditHeight,
		TRUE);

	MoveWindow(hPathChk,
		HalfWidth + LeftBorder - ListWidth + 20,
		(TopBorder + Height) - (EditHeight + BottomBorder + BtnHeight) - 20 - BtnHeight,
		ListWidth + HalfWidth - RightBorder - 33,
		EditHeight,
		TRUE);

	int BtnSpace = 3;
	int BtnWidth = HalfWidth / 3 - BtnSpace;

	MoveWindow(hAdd,
		LeftBorder,
		(TopBorder + Height) - (BtnHeight + BottomBorder),
		BtnWidth,
		BtnHeight,
		TRUE);

	MoveWindow(hRemove,
		BtnWidth + LeftBorder + BtnSpace,
		(TopBorder + Height) - (BtnHeight + BottomBorder),
		BtnWidth,
		BtnHeight,
		TRUE);

	POINT pt;
	RECT rc;
	GetWindowRect (hgbOption, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient (GetParent(hgbOption), &pt);
	GetClientRect (hgbOption, &rc);
	rc.left = pt.x;
	rc.top = pt.y;
	rc.right += rc.left;
	rc.bottom += rc.top;


	BtnSpace = 1;
	BtnWidth = (rc.right - rc.left) / 3 - (BtnSpace * 2);

	MoveWindow(hDone,
		HalfWidth + LeftBorder - ListWidth,
		(TopBorder + Height) - (BtnHeight + BottomBorder),
		BtnWidth,
		BtnHeight,
		TRUE);

	MoveWindow(hCancel,
		(rc.right - rc.left) / 2 + rc.left - (BtnWidth/2),
		(TopBorder + Height) - (BtnHeight + BottomBorder),
		BtnWidth,
		BtnHeight,
		TRUE);

	MoveWindow(hApply,
		rc.right - BtnWidth,
		(TopBorder + Height) - (BtnHeight + BottomBorder),
		BtnWidth,
		BtnHeight,
		TRUE);

}

BOOL UpdateDlgData(HWND hWnd){
	KeyFileData *Data;
	TCHAR szKBPath[MAX_PATH], szKBNames[500], wcCurDir[MAX_PATH], szMenuDisplay[MAX_PATH], szShortCut[10];

	if (!GetModuleFileName(GetModuleHandle(NULL), szKBPath, MAX_PATH))
		return false;

	for (int i=lstrlen(szKBPath); szKBPath[i] != '\\'; i--){
		szKBPath[i] = NULL;
		if (szKBPath[i-1] == '\\'){
			szKBPath[i-1] = NULL;
			break;
		}
	}

	GetPrivateProfileString(szKBP, NULL, NULL, (LPTSTR)szKBNames, 500, szINIFile);

	for (int i=0, j=0,Length = lstrlen(&szKBNames[i]);
		Length > 0; 
		i+=Length+1, Length = lstrlen(&szKBNames[i]), j++){

			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)&szKBNames[i]);

			GetPrivateProfileString(szMS, &szKBNames[i], NULL, szMenuDisplay, 30, szINIFile);
			GetPrivateProfileString(szKBP, &szKBNames[i], NULL, szKBPath, MAX_PATH, szINIFile);
			WORD wHotkey = GetPrivateProfileInt(szSC, &szKBNames[i], 0, szINIFile);

			//Data = (KeyFileData*)LocalAlloc(LPTR, sizeof(KeyFileData));
			Data = new KeyFileData;
			Data->isNew = FALSE;
			lstrcpy(Data->Name,  &szKBNames[i]);
			lstrcpy(Data->Display, szMenuDisplay);
			lstrcpy(Data->Path, szKBPath);
			Data->wHotkey = wHotkey;
			SendMessage(hList, LB_SETITEMDATA, j, (LPARAM)Data);
	}
	return true;

}

void DeleteDlgData(HWND hWnd){
	KeyFileData *Data;
	int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
	for (int i=0;count > i; i++){
		Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, i, NULL);
		//LocalFree(Data);
		delete Data;
	}
	SendMessage(hList, LB_RESETCONTENT, 0, 0);
}

void SetKbData(HWND hWnd){

	TCHAR shortcut[10], szNormal[] = TEXT("Normal");
	KeyFileData *Data;
	int count = SendMessage(hList, LB_GETCOUNT, 0, 0);

	WritePrivateProfileSection(szKBP, NULL, szINIFile);
	WritePrivateProfileSection(szMS, NULL, szINIFile);
	WritePrivateProfileSection(szSC, NULL, szINIFile);

	for (int i=0;count > i; i++){
		Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, i, NULL);

		WritePrivateProfileString(szKBP, Data->Name, Data->Path, szINIFile);

		WritePrivateProfileString(szMS, Data->Name, Data->Display, szINIFile);

		wsprintf(shortcut, TEXT("%d"), Data->wHotkey);

		WritePrivateProfileString(szSC, Data->Name, shortcut, szINIFile);
	}
}

BOOL OpenDialog(HWND hwnd, TCHAR* szFileName,DWORD nMaxFile)
{
	OPENFILENAME ofn = {0};

	RtlZeroMemory(szFileName, nMaxFile);

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = TEXT("Keymap File(*.km2)\0*.km2\0\0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = nMaxFile;
	ofn.lpstrTitle = TEXT("Open File...");
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | 
		OFN_LONGNAMES | OFN_HIDEREADONLY;

	if(!GetOpenFileName(&ofn)) return false;
	return true;
}

VOID restart (HWND hWnd){
	DeleteDlgData(hWnd);
	UpdateDlgData(hWnd);
	kbindex = -1;

	SetDlgItemText(hWnd, IDC_DISPLAY, NULL);
	SetDlgItemText(hWnd, IDC_PATH, NULL);
	SendDlgItemMessage(hWnd, IDC_SHORTCUT, HKM_SETHOTKEY ,0, 0);

	DestroyMyMenu(hKeyMenu);

	SendMessage(hList, LB_SETSEL, 0, 0);

	GetKeyBoards();
	CreateMyMenu(hKeyMenu);
}

BOOL AddKeyBoardToList(HWND hWnd,TCHAR* lpKBPath){
	TCHAR lpPath[MAX_PATH];
	TCHAR lpName[MAX_PATH];
	TCHAR szKBPath[MAX_PATH];

	KeyFileData *Data;
	
	GetFileTitle(lpKBPath, lpName, MAX_PATH);

	if (lpName [lstrlen(lpName)-4] != '.')
		lstrcat(lpName, TEXT(".km2"));

	wsprintf(lpPath, TEXT("KeyBoards\\%s"), lpName);

	lpName [lstrlen(lpName)-4] = NULL;

	//if (SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, szKBPath)){
	//	MessageBox(hWnd, "Cannot locate \"Common Application Data\" Path!", szKeymagic, MB_ICONERROR);
	//	return false;
	//}

	if (!GetModuleFileName(GetModuleHandle(NULL), szKBPath, MAX_PATH))
		return false;

	for (int i=lstrlen(szKBPath); szKBPath[i] != '\\'; i--){
		szKBPath[i] = NULL;
		if (szKBPath[i-1] == '\\'){
			szKBPath[i-1] = NULL;
			break;
		}
	}

	//PathAppend(szKBPath, TEXT("KeyMagic"));
	PathAppend(szKBPath, lpPath);

	if (lstrcmpi(lpKBPath, szKBPath))
	{
		if (!CopyFile(lpKBPath, szKBPath, false)){
			if (IDNO == MessageBox(hWnd, TEXT("File copying fail! \n Do you want to use from current path?"), szKeymagic, MB_ICONERROR | MB_YESNO))
				return false;
			lstrcpy(lpPath, lpKBPath);
		}
	}

	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)lpName);
	//Data = (KeyFileData*)LocalAlloc(LPTR, sizeof(KeyFileData));
	Data = new KeyFileData;
	lstrcpy(Data->Name, lpName);
	lstrcpy(Data->Display, lpName);
	lstrcpy(Data->Path, lpPath);
	Data->wHotkey = 0;

	SendMessage(hList, LB_SETITEMDATA, SendMessage(hList, LB_GETCOUNT, 0, 0)-1, (LPARAM)Data);

	return true;
}

BOOL RemoveKeyBoard(){
	KeyFileData *Data;

	kbindex = SendMessage(hList, LB_GETCURSEL, 0, 0);

	if (kbindex == -1 || kbindex == 0)
		return false;

	Data = (KeyFileData*)SendMessage(hList, LB_GETITEMDATA, kbindex, 0);

	if (Data->Path[1] != ':'){
		//if (cbFileToDelete == 0){
		//	szFileToDelete = (strDelete*)VirtualAlloc(NULL, sizeof(strDelete)*20, MEM_COMMIT, PAGE_READWRITE);
		//}

		//cbFileToDelete++;
		//lstrcpy(szFileToDelete[cbFileToDelete].Path, Data->Path);
		TCHAR * wcPath;
		int wclen = lstrlen(Data->Path);
		wcPath = new TCHAR[wclen];
		lstrcpy(wcPath, Data->Path);
		szFileToDelete.push_back(wcPath);
	}

	//LocalFree(Data);
	delete Data;
	SendMessage(hList, LB_DELETESTRING, kbindex, 0);
	kbindex=-1;
	SendMessage(hList, LB_SETCURSEL, 0, 0);
	return true;
}

BOOL DeleteKeyFile(){
	TCHAR szKBPath[MAX_PATH],szToDelete[MAX_PATH];

	//if (!cbFileToDelete)
	//	return false;
	if (!szFileToDelete.size())

	if (SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, szKBPath)){
		//cbFileToDelete=0;
		error(TEXT("SHGetFolderPath"));
		return false;
	}

	if (!GetModuleFileName(GetModuleHandle(NULL), szKBPath, MAX_PATH))
		return false;

	for (int i=lstrlen(szKBPath); szKBPath[i] != '\\'; i--){
		szKBPath[i] = NULL;
		if (szKBPath[i-1] == '\\'){
			szKBPath[i-1] = NULL;
			break;
		}
	}
	//PathAppend(szKBPath, TEXT("KeyMagic"));

	//for (int i=1; i <= cbFileToDelete; i++){
	//	lstrcpy(szToDelete, szKBPath);
	//	PathAppend(szToDelete, szFileToDelete[i].Path);
	//	DeleteFile(szToDelete);
	//}

	vector<TCHAR*>::iterator it;

	for ( it=szFileToDelete.begin() ; it < szFileToDelete.end(); it++ )
	{
		lstrcpy(szToDelete, szKBPath);
		PathAppend(szToDelete, (LPCTSTR)*it);
		DeleteFile(szToDelete);
	}

	szFileToDelete.clear();

	//VirtualFree(szFileToDelete, sizeof(strDelete)*20, MEM_DECOMMIT);

	//cbFileToDelete=0;
	return true;
}

VOID GetHotKey(WORD wHotkey, LPTSTR ShortCutDisplay){

	BYTE *vkey, modkey;

	vkey = new BYTE[2];
	vkey[0] = wHotkey;

	modkey = wHotkey >> 8;
	ShortCutDisplay[0] = NULL;

	if (modkey & HOTKEYF_SHIFT){
		lstrcat(ShortCutDisplay, TEXT("Shift+"));
	}

	if (modkey & HOTKEYF_CONTROL){
		lstrcat(ShortCutDisplay, TEXT("Ctrl+"));
	}

	if (modkey & HOTKEYF_ALT){
		lstrcat(ShortCutDisplay, TEXT("Alt+"));
	}

	vkey[1] = 0;

	if (vkey[0] >= VK_F1 && vkey[0] <= VK_F12){
		TCHAR FunctionKey[3];
		int Function = vkey[0] - 0x6F;
		wsprintf(FunctionKey, TEXT("F%x"), Function);
		lstrcat(ShortCutDisplay, FunctionKey);
		return;
	}

	WORD TransedChar = MapVirtualKey(vkey[0], MAPVK_VK_TO_CHAR);
	wsprintf(ShortCutDisplay,TEXT("%s%c"), ShortCutDisplay, TransedChar);
}

VOID error(LPCTSTR lpszFunction) 
{
	TCHAR* Buffer = 0;
	//Get Last Error Code and Translate to text
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | 
		FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
			GetLastError(),LANG_NEUTRAL, Buffer, 0, NULL);
	
	//Show the error message box
	MessageBox(NULL, Buffer, lpszFunction, MB_OK | MB_ICONHAND | MB_APPLMODAL);
}

VOID GetKeyBoards(){

	TCHAR szMenuDisplay[MAX_PATH];
	TCHAR szKBNames[500];
	TCHAR szShortCut[20];
	WORD wHotKey;
	MENUITEMINFO mii = {0};

	mii.cbSize = sizeof(MENUITEMINFO) ;
	mii.fMask = MIIM_STRING;
	mii.fState = MFS_DEFAULT;

	if (hKeyMenu)
		DestroyMenu(hKeyMenu);

	hKeyMenu = CreatePopupMenu();
	if (!hKeyMenu)
		return;

	GetPrivateProfileStringW(szKBP, NULL, NULL, (LPTSTR)szKBNames, 500, szINIFile);

	GetPrivateProfileStringW(szMS, &szKBNames[0], NULL, (LPTSTR)szMenuDisplay, MAX_PATH, szINIFile);
	wHotKey = GetPrivateProfileInt(szSC, &szKBNames[0], 0, szINIFile);
	lstrcat(szMenuDisplay, TEXT("\t"));

	GetHotKey(wHotKey, szShortCut);
	lstrcat(szMenuDisplay, szShortCut);

	AppendMenu(hKeyMenu, NULL, IDKM_NORMAL, szMenuDisplay);
	//mii.dwItemData = (ULONG_PTR)szMenuDisplay;
	//InsertMenuItem(hKeyMenu, IDKM_NORMAL, FALSE, &mii);
	KeyBoardNum=0;
	for (int i=lstrlen(&szKBNames[0])+1,Length = lstrlen(&szKBNames[i]);
		Length > 0; 
		i+=Length+1, Length = lstrlen(&szKBNames[i])){
			GetPrivateProfileStringW(szMS, &szKBNames[i], NULL, szMenuDisplay, MAX_PATH, szINIFile);
			wHotKey = GetPrivateProfileIntW(szSC, &szKBNames[i], 0, szINIFile);

			lstrcat(szMenuDisplay, TEXT("\t"));
			GetHotKey(wHotKey, szShortCut);
			lstrcat(szMenuDisplay, szShortCut);

			AppendMenu(hKeyMenu, NULL, IDKM_ID+KeyBoardNum, szMenuDisplay);
			KeyBoardNum++;
	}

	CheckMenuRadioItem(hKeyMenu, IDKM_NORMAL, 
		KeyBoardNum + IDKM_ID,
		IDKM_NORMAL,
		MF_BYCOMMAND);

};

BOOL Run(LPTSTR lpszFileName, LPTSTR lpszDirectory, LPTSTR lpParameters)
{
    SHELLEXECUTEINFO TempInfo = {0};

    TempInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	TempInfo.nShow = SW_HIDE;
    TempInfo.fMask = 0;
    TempInfo.hwnd = NULL;
    TempInfo.lpFile = lpszFileName;
    TempInfo.lpParameters = lpParameters;
    TempInfo.lpDirectory = lpszDirectory;
    TempInfo.nShow = SW_HIDE;

    BOOL bRet = ::ShellExecuteEx(&TempInfo);

    return bRet;
}

VOID SetStartup(BOOL isEnable){

	OSVERSIONINFO osvi;
	BOOL bIsWindowsVistaLater;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	bIsWindowsVistaLater = osvi.dwMajorVersion >= 6;

	if (!bIsWindowsVistaLater){
		HKEY hkHLM, hkRun;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, NULL, NULL, KEY_ALL_ACCESS, &hkHLM) != ERROR_SUCCESS)
			return;

		if (RegOpenKeyEx(hkHLM, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), NULL, KEY_ALL_ACCESS, &hkRun) != ERROR_SUCCESS)
			return;

		if (!isEnable){
			RegDeleteValue(hkRun, TEXT("Keymagic"));
			WritePrivateProfileString(TEXT("Settings"), TEXT("Startup"), TEXT("0"), szINIFile);
		}

		else{
			TCHAR FileName[MAX_PATH];
			GetModuleFileName(hInst, FileName, MAX_PATH);
			lstrcat(FileName, TEXT(" -s"));
			RegSetValueEx(hkRun, TEXT("Keymagic"), NULL, REG_SZ, (BYTE*)FileName, lstrlen(FileName));
			WritePrivateProfileString(TEXT("Settings"), TEXT("Startup"), TEXT("1"), szINIFile);
		}
		RegCloseKey(hkRun);
		RegCloseKey(hkHLM);
	}
	else {
		TCHAR TaskScheduler[MAX_PATH];
		/*PROCESS_INFORMATION pi;
		STARTUPINFO si;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		si.wShowWindow = SW_MINIMIZE;
		ZeroMemory( &pi, sizeof(pi) );*/

		lstrcpy(TaskScheduler, wcCurDir);
		lstrcat(TaskScheduler, TEXT("\\SetElevatedStartupTask.exe"));

		if (!isEnable){
			if (Run(TaskScheduler, wcCurDir, TEXT("-d"))){
				WritePrivateProfileString(TEXT("Settings"), TEXT("Startup"), TEXT("0"), szINIFile);
			}
		}

		else {
			TCHAR FileName[MAX_PATH];
			GetModuleFileName(hInst, FileName, MAX_PATH);

			if (Run(TaskScheduler, wcCurDir, NULL))
				WritePrivateProfileString(TEXT("Settings"), TEXT("Startup"), TEXT("1"), szINIFile);
		}
	}
}

VOID ShowNotifyIcon(HWND hWnd, LPCTSTR szTip, LPCTSTR lpIconName){
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = TRAY_ID;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAY;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(lpIconName));
	lstrcpy(nid.szTip, szTip);
	Shell_NotifyIcon(NIM_ADD, &nid);
}

VOID DeleteNotifyIcon(HWND hWnd) {
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = TRAY_ID;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

VOID ChangeNotifyIcon(HWND hWnd, HICON hIcon){
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON;
	nid.uID = TRAY_ID;
	nid.hIcon = hIcon;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

VOID ShowBallonTip(HWND hWnd, LPCTSTR szInfoTitle, LPCTSTR szInfo, DWORD dwInfoFlags){
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = TRAY_ID;

	nid.uFlags = NIF_INFO;
	nid.uTimeout = 10;
	nid.dwInfoFlags = dwInfoFlags;
	wcscpy(nid.szInfo, szInfo);
	wcscpy(nid.szInfoTitle, szInfoTitle);

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}