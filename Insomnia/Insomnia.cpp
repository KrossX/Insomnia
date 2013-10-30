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

Settings set;
sMode &Mode = set.Mode;
std::list<std::string> whitelist;

const wchar_t regName[] = L"KrossX's Insomnia";
const wchar_t regSub[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const wchar_t statusStr[][32] = {L"Insomnia is running...", L"Insomnia is NOT running..."};
wchar_t exePath[512] = {0};

bool iniFilePresent = false;

enum
{
	INSOMNIA_RUNNING,
	INSOMNIA_NOTRUNNING
};

bool AnotherInstance()
{
	DWORD process[2048] = {0}, outBytes = 0;

	if(!EnumProcesses(process, sizeof(process), &outBytes)) 
		return false;

	DWORD numProcs = outBytes / sizeof(DWORD);

	DWORD procID = GetCurrentProcessId();

	for(DWORD i = 0; i < numProcs; i++)
	{
		if(process[i])
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process[i]);

			if(hProcess)
			{
				if(GetProcessId(hProcess) != procID)
				{
					char name[MAX_PATH] = {0};
					GetProcessImageFileNameA(hProcess, name, MAX_PATH);
				
					std::string filename(name);
					filename = filename.substr(filename.find_last_of("\\/")+1);

					if(_stricmp(filename.c_str(), "Insomnia.exe") == 0)
					{
						CloseHandle(hProcess);
						return true;
					}
				}

				CloseHandle(hProcess);
			}
		}
	}

	
	return false;
}

bool CheckWhitelist()
{
	if(FileIO::isINIModified()) 
		FileIO::LoadSettings(whitelist);

	if(!set.wlEnabled || whitelist.empty())
	{
		SetWindowText(hStatus, statusStr[INSOMNIA_RUNNING]);
		return true;
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
				char name[MAX_PATH] = {0};
				GetProcessImageFileNameA(hProcess, name, MAX_PATH);
				
				std::string filename(name);
				filename = filename.substr(filename.find_last_of("\\/")+1);

				for each (std::string sProcess in whitelist)
				{
					if(_stricmp(filename.c_str(), sProcess.c_str()) == 0)
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
			if(Mode.Mouse)
				mouse_event( MOUSEEVENTF_MOVE, 0, 0, 0, NULL);

			if(Mode.Display || Mode.System)
			{
				EXECUTION_STATE state;

				state  = Mode.System ? ES_SYSTEM_REQUIRED : 0;
				state |= Mode.Display ? ES_DISPLAY_REQUIRED : 0;
				
				SetThreadExecutionState(state);
			}
		}

		Sleep(set.loopLength);
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

			CheckDlgButton(hwndDlg, IDC_MODE_SYSTEM, Mode.System ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MODE_DISPLAY, Mode.Display ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MODE_MOUSE, Mode.Mouse ? BST_CHECKED : BST_UNCHECKED);

			HKEY reg_run = NULL;
			LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, regSub, NULL, KEY_READ, &reg_run);

			if(result == ERROR_SUCCESS && RegQueryValueEx(reg_run, regName, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
				CheckDlgButton(hwndDlg, IDC_STARTUP, BST_CHECKED);

			CheckDlgButton(hwndDlg, IDC_WHITELIST, set.wlEnabled ? BST_CHECKED : BST_UNCHECKED);

			hCheckThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CheckThread, 0, 0, NULL);

			POINT position;
			position.x = set.posx;
			position.y = set.posy;

			ScreenToClient(hwndDlg, &position);

			SetWindowPos(hwndDlg, NULL, position.x, position.y, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
			ShowWindow(hwndDlg, set.minimized ? SW_MINIMIZE : 0);
		} 
		break;

	case WM_MOVE:
		set.posx = LOWORD(lParam);
		set.posy = HIWORD(lParam);
		break;

	case WM_COMMAND:
		{
			short command = LOWORD(wParam);

			switch(command)
			{
			case IDC_MODE_SYSTEM:
				Mode.System = IsDlgButtonChecked(hwndDlg, IDC_MODE_SYSTEM) == BST_CHECKED;
				break;

			case IDC_MODE_DISPLAY:
				Mode.Display = IsDlgButtonChecked(hwndDlg, IDC_MODE_DISPLAY) == BST_CHECKED;
				break;

			case IDC_MODE_MOUSE:
				Mode.Mouse = IsDlgButtonChecked(hwndDlg, IDC_MODE_MOUSE) == BST_CHECKED;
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
				set.wlEnabled = IsDlgButtonChecked(hwndDlg, IDC_WHITELIST) == BST_CHECKED;
				break;

			case ID_SETTINGS:
				FileIO::SaveSettings(whitelist);
				FileIO::OpenSettings();
				iniFilePresent = FileIO::LoadSettings(whitelist);
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

	if(AnotherInstance())
	{
		MessageBoxA(NULL, "Another instance is already running.", "Insomnia", MB_OK);
		return 0;
	}

	FileIO::GetFilename();
	iniFilePresent = FileIO::LoadSettings(whitelist);

	if(set.priority)
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	GetModuleFileName(NULL, exePath, 512);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_INSOMNIA), NULL, DialogProc, NULL);
	
	MSG message;

	while(GetMessage(&message, NULL, NULL, NULL))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if(iniFilePresent)
		FileIO::SaveSettings(whitelist);

	return 0;
}
