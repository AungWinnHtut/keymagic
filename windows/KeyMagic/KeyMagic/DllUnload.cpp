//Dll Injector
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

#include <iostream>
#include "DllUnload.h"
//#include "../global/global.h"

#ifndef _WIN64

TCHAR Path[MAX_PATH];
BYTE Injection[] = {
	0x68,0x90,0x90,0x90,0x90, // push address
	0xFF,0x15,0x90,0x90,0x90,0x90, //call dword ptr ds:[] GetModuleHandleA
	0x50,//push eax
	0xFF,0x15,0x90,0x90,0x90,0x90,//call dword ptr ds:[] FreeLibrary
	0xC3,//return
	0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};

void ScannerAndInject ( )
{
	FARPROC Function;

	Function = (FARPROC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetModuleHandleA");
	memcpy(&Injection[19], &Function, 4);
	Function = (FARPROC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "FreeLibrary");
	memcpy(&Injection[23], &Function, 4);

	// Get the list of process identifiers.

	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		return;

	// Calculate how many process identifiers were returned.

	cProcesses = cbNeeded / sizeof(DWORD);

	// Get the name of the modules for each process.

	for ( i = 0; i < cProcesses; i++ )
		GetModules( aProcesses[i] );
}

void GetModules( DWORD processID )
{
    HMODULE hMods[1024];
    HANDLE hProcess;
    DWORD cbNeeded;
	TCHAR szTemp[MAX_PATH];
    unsigned int i;

	if (GetCurrentProcessId() == processID)
		return;

    // Get a list of all the modules in this process.

    hProcess = OpenProcess( PROCESS_ALL_ACCESS,
                            FALSE, processID );
    if (NULL == hProcess)
        return;

    if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
        {
            TCHAR szModName[MAX_PATH];

            // Get the full path to the module's file

            if (int l = GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
				for (l;l > 0 ; l--){
					if (szModName[l] == TEXT('\\')){
						if (!lstrcmpi(szModName + l + 1, TEXT("KeymagicDLL.dll"))){
							//Unload KeymagicDll from this process ID
							UnloadDLL( szModName, hProcess );
							TCHAR szProcessFileName[MAX_PATH];
							GetModuleFileNameEx(hProcess, NULL, szProcessFileName, sizeof(szProcessFileName));
							wprintf(L"%s%s\n", L"KeymagicDLL.dll is unloaded from ", szProcessFileName);
							CloseHandle( hProcess );
							return;
						}
						break;
					}
				}
            }
        }
    }

    CloseHandle( hProcess );
}

void UnloadDLL ( TCHAR *ModulePath, HANDLE hProcess ){

	LPBYTE InjAddr, Pointer, PathAddress;
	HANDLE hThread;

	InjAddr = (LPBYTE)VirtualAllocEx(hProcess, NULL, 1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	WriteProcessMemory(hProcess, (LPVOID)InjAddr, &Injection, sizeof(Injection), NULL);

	WriteProcessMemory(hProcess, (LPVOID)(InjAddr + sizeof(Injection)), ModulePath, MAX_PATH, NULL);

	PathAddress = InjAddr + sizeof(Injection);
	WriteProcessMemory(hProcess, (LPVOID)(InjAddr + 1), &PathAddress, 4, NULL);

	Pointer = InjAddr + 19;
	WriteProcessMemory(hProcess, (LPVOID)(InjAddr + 7), &Pointer, 4, NULL);

	Pointer += 4;
	WriteProcessMemory(hProcess, (LPVOID)(InjAddr + 14), &Pointer, 4, NULL);

	hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)InjAddr, NULL, THREAD_PRIORITY_NORMAL, NULL);
	WaitForSingleObject(hThread, 100);
	CloseHandle(hThread);

	Sleep(50);

	VirtualFreeEx(hProcess, (LPVOID)InjAddr, 1000, MEM_DECOMMIT);

};

#endif