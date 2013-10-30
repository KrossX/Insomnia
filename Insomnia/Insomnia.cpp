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
#include "resource.h"

#include <CommCtrl.h>

HANDLE hMouseThread = NULL;
HINSTANCE g_hInstance = NULL;
const wchar_t regName[] = L"KrossX's Insomnia";
const wchar_t regSub[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
wchar_t exePath[512] = {0};

void MouseThread()
{
	while(true)
	{
		mouse_event( MOUSEEVENTF_MOVE, 0, 0, 0, NULL);
		Sleep(1000);
	}
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG: 
		{			
			HICON hIcon;
			
			hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, 0);    
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

			hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);    
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_ADDSTRING, 0, (LPARAM)L"Display On");
			SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_ADDSTRING, 0, (LPARAM)L"System On");
			SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_ADDSTRING, 0, (LPARAM)L"Display & System On");

			SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_SETCURSEL, 2, 0);
			CheckDlgButton(hwndDlg, IDC_SCREENSAVER, BST_CHECKED);
	
			SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
			hMouseThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MouseThread, 0, 0, NULL);

			HKEY reg_run = NULL;	
			LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, regSub, NULL, KEY_READ, &reg_run);

			if(result == ERROR_SUCCESS && RegQueryValueEx(reg_run, regName, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
				CheckDlgButton(hwndDlg, IDC_STARTUP, BST_CHECKED);

			ShowWindow(hwndDlg, SW_MINIMIZE);
		} break;    

	case WM_COMMAND:
		{
			short command = LOWORD(wParam);

			switch(command)
			{
			case IDC_MODE: if(HIWORD(wParam) == CBN_SELCHANGE)
				{				
					short selection = (short)SendMessage(GetDlgItem(hwndDlg, IDC_MODE), CB_GETCURSEL, 0, 0);
					switch(selection)
					{
					case 0: SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED); break;
					case 1: SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED); break;
					case 2: SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED); break;
					}				
				}
				break;

			case IDC_SCREENSAVER:
				{
					if(IsDlgButtonChecked(hwndDlg, IDC_SCREENSAVER) == BST_CHECKED)
					{
						if(hMouseThread != NULL) TerminateThread(hMouseThread,0);
						hMouseThread = NULL;
					}
					else
					{
						if(hMouseThread == NULL)
							hMouseThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MouseThread, 0, 0, NULL);
					}
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

			case IDCANCEL:
			case IDSTOP:
				SetThreadExecutionState(ES_CONTINUOUS);
				if(hMouseThread != NULL) TerminateThread(hMouseThread,0);
				hMouseThread = NULL;
				EndDialog(hwndDlg, command);
				PostQuitMessage(0);
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
