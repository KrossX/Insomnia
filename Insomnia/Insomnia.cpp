/*  Insomnia
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Insomnia.h"
#include "FileIO.h"

#include "resource.h"
#include <CommCtrl.h>

#include <psapi.h>
#pragma comment(lib,"Psapi.lib")

HINSTANCE g_hInstance = NULL;

HANDLE hCheckThread = NULL;
HWND hDialog = NULL;
HWND hStatus = NULL;

bool whitelistEnabled = false;
bool whitelistLoaded = false;
std::list<std::string> whitelist;

const wchar_t regName[] = L"KrossX's Insomnia";
const wchar_t regSub[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
wchar_t exePath[512] = {0};

enum
{
	INSOMNIA_RUNNING,
	INSOMNIA_NOTRUNNING
};

enum
{
	MODE_NONE,
	MODE_SYSTEM,
	MODE_DISPLAY,
	MODE_BOTH,
	MODE_COUNT
};

const wchar_t statusStr[][32] = {L"Insomnia is running...", L"Insomnia is NOT running..."};
const wchar_t modeStr[][32] = {L"None", L"System On", L"Display On", L"Display & System On"};

EXECUTION_STATE modeState[] = {	ES_CONTINUOUS,
								ES_SYSTEM_REQUIRED, 
								ES_DISPLAY_REQUIRED, 
								ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED};

int modeCurr = MODE_BOTH;

bool CheckWhitelist()
{
	if(!whitelistEnabled)
	{
		SetWindowText(hStatus, statusStr[INSOMNIA_RUNNING]);
		return true;
	}
	else if(FileIO::isWhitelistModified())
	{
		whitelist.clear();
		FileIO::LoadWhitelist(whitelist);
	}
	
	DWORD process[2048] = {0}, outBytes = 0;

	if(!EnumProcesses(process, sizeof(process), &outBytes))
	{
		SetWindowText(hStatus, statusStr[INSOMNIA_NOTRUNNING]);
		return false;
	}
	
	DWORD numProcs = outBytes / sizeof(DWORD);

	for(DWORD i = 0; i < numProcs; i++)
	{
		if(process[i])
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process[i]);

			if(hProcess) 
			{
				char name[80] = {0};
				GetModuleBaseNameA(hProcess, NULL, name, 80);

				for each (std::string sProcess in whitelist)
				{
					if(_stricmp(name, sProcess.c_str()) == 0)
					{
						CloseHandle(hProcess);
						SetWindowText(hStatus, statusStr[INSOMNIA_RUNNING]);
						return true;
					}
				}

				CloseHandle(hProcess);
			}
		}
	}

	SetWindowText(hStatus, statusStr[INSOMNIA_NOTRUNNING]);
	return false;
}

void CheckThread()
{
	while(true)
	{
		if(CheckWhitelist())
		{
			SetThreadExecutionState(modeState[modeCurr]);

			if(modeCurr & ES_DISPLAY_REQUIRED)
				mouse_event( MOUSEEVENTF_MOVE, 0, 0, 0, NULL);
		}

		Sleep(59000);
	}
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			HICON hIcon;
			
			hDialog = hwndDlg;
			hStatus = GetDlgItem(hwndDlg, IDC_STATUS);

			hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, 0);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

			hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			for(int i = 0; i < MODE_COUNT; i++)
				SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_ADDSTRING, 0, (LPARAM)modeStr[i]);

			SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_SETCURSEL, MODE_BOTH, 0);
			CheckDlgButton(hwndDlg, IDC_SCREENSAVER, BST_CHECKED);

			HKEY reg_run = NULL;
			LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, regSub, NULL, KEY_READ, &reg_run);

			if(result == ERROR_SUCCESS && RegQueryValueEx(reg_run, regName, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
				CheckDlgButton(hwndDlg, IDC_STARTUP, BST_CHECKED);

			whitelistEnabled = whitelistLoaded;
			EnableWindow(GetDlgItem(hwndDlg, ID_WHITELIST), whitelistLoaded);

			if(whitelistLoaded)
				CheckDlgButton(hwndDlg, IDC_WHITELIST, BST_CHECKED);

			hCheckThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CheckThread, 0, 0, NULL);

			SetWindowPos(hwndDlg, NULL, 200, 200, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
			ShowWindow(hwndDlg, SW_MINIMIZE);
		} break;

	case WM_COMMAND:
		{
			short command = LOWORD(wParam);

			switch(command)
			{
			case IDC_MODE: if(HIWORD(wParam) == CBN_SELCHANGE)
				{
					short mode = (short)SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_GETCURSEL, 0, 0);
					modeCurr = modeState[mode];
				}
				break;

			case IDC_STARTUP:
				{
					HKEY reg_run = NULL;
					LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, regSub, NULL, KEY_WRITE, &reg_run);

					if(result == ERROR_SUCCESS)
					{
						if(IsDlgButtonChecked(hwndDlg, IDC_STARTUP) == BST_CHECKED)
							RegSetValueEx(reg_run, regName, NULL, REG_SZ, (BYTE*)exePath, 512);
						else
							RegDeleteValue(reg_run, regName);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_STARTUP, BST_UNCHECKED);
						EnableWindow(GetDlgItem(hwndDlg, IDC_STARTUP), false);
					}
				}
				break;

			case IDC_WHITELIST:
				{
					bool bWhite = IsDlgButtonChecked(hwndDlg, IDC_WHITELIST) == BST_CHECKED;

					if(bWhite)
					{
						if(!whitelistLoaded)
						{
							if(FileIO::CreateWhitelist())
								whitelistLoaded = FileIO::LoadWhitelist(whitelist);
						}

						whitelistEnabled = whitelistLoaded;
					}
					else
						whitelistEnabled = false;

					CheckDlgButton(hwndDlg, IDC_WHITELIST, whitelistEnabled? BST_CHECKED : BST_UNCHECKED);
					EnableWindow(GetDlgItem(hwndDlg, ID_WHITELIST), whitelistLoaded);
				}
				break;

			case ID_WHITELIST:
				system( "start notepad.exe Insomnia.whitelist" ); // Meh
				break;

			case IDCANCEL:
			case IDSTOP:
				PostQuitMessage(0);
				break;
			
			case WM_QUIT:
				if(hCheckThread) TerminateThread(hCheckThread, 0);
				EndDialog(hwndDlg, command);
				break;
			}

		} break;

	default: return FALSE;
	}

	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;
	whitelistLoaded = FileIO::LoadWhitelist(whitelist);

	GetModuleFileName(NULL, exePath, 512);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_INSOMNIA), NULL, DialogProc, NULL);
	
	MSG message;

	while(GetMessage(&message, NULL, NULL, NULL))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 0;
}
