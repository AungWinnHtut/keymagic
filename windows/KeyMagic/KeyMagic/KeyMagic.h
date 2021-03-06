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

#ifndef KEYMGAIC_H_
#define KEYMGAIC_H_

#pragma once

#if defined(_WIN64)
#define MUTEX_STRING "\u1000\u102E\u1038\u1019\u1000\u1039\u1002\u103A\u1005\u1039_64"
#else
#define MUTEX_STRING "\u1000\u102E\u1038\u1019\u1000\u1039\u1002\u103A\u1005\u1039_32"
#endif

#include "resource.h"

#include <Userenv.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <Commdlg.h>
#include <gdiplus.h>
#include <tchar.h>
#include <vector>

#include "../global/global.h"

using namespace std;

//Custom message IDs
#define TRAY_ID 100
#define WM_TRAY WM_USER + 10
#define IDKM_NORMAL 500
#define IDKM_ID 501

#define KM_MESSAGE WM_USER+5000
#define KM_SETKBID KM_MESSAGE+1
#define KM_KILLFOCUS KM_MESSAGE+2
#define KM_GETFOCUS KM_MESSAGE+3
#define KM_RESCAN KM_MESSAGE+4
#define KM_ERR_KBLOAD KM_MESSAGE+5
#define MAX_LOADSTRING 100

extern TCHAR szINIFile[MAX_PATH];
extern TCHAR wcCurDir[MAX_PATH];

extern void	GetKeyBoards();
extern void	SetHook (HWND hwnd);

struct KeyFileData{
	bool isNew;
	std::wstring Name;
	std::wstring Display;
	std::wstring Path;
	WORD wHotkey;
};

struct strDelete {
	std::wstring Path;
};

INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				SetHook (HWND hwnd);
void				UnHook ();
BOOL				AddKeyBoard(TCHAR* lpKBPath);
BOOL				WorkOnCommand(LPTSTR lpCmdLine);
BOOL				IsAdmin();
HANDLE				LoadFontFromRes(LPCWSTR ResName);

extern BOOL bAdmin;

extern TCHAR szKBP[];
extern TCHAR szMS[];
extern TCHAR szSC[];
extern TCHAR szNeedRestart[];
extern TCHAR szKeymagic[];
TCHAR szError[];
TCHAR szKBLoad_ERR[];

extern HINSTANCE hInst;								// current instance
extern TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
extern HMENU hKeyMenu;
extern UINT KeyBoardNum;
extern UINT wmsgTaskbarCreated;

#endif