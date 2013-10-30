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

HANDLE hMouseThread = NULL;

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
			SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
			hMouseThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MouseThread, 0, 0, NULL);
			ShowWindow(hwndDlg, SW_SHOW);
		} break;    

	case WM_COMMAND:
		{
			short command = LOWORD(wParam);

			switch(command)
			{
			case IDCANCEL:
			case IDSTOP:
				SetThreadExecutionState(ES_CONTINUOUS);
				if(hMouseThread != NULL) TerminateThread(hMouseThread,0);
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
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_INSOMNIA), NULL, DialogProc, NULL);
	
	MSG message;

	while(GetMessage(&message, NULL, NULL, NULL))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	} 
	
	return 0;
}
