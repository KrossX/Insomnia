/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#include "insomnia.h"
#include "insomnia_gui.h"
#include "settings.h"
#include "file_io.h"

//EnumProcesses
#include <psapi.h>
#pragma comment(lib,"Psapi.lib")

#include <thread>
bool run_check_thread;

window *gui;
settings set;
std::list<std::string> whitelist;

const char status_message[][32] = {"Insomnia is running...", "Insomnia is NOT running..."};
enum { INSOMNIA_RUNNING, INSOMNIA_NOTRUNNING };

bool check_whitelist()
{
	if(!set.whitelist || whitelist.empty())
	{
		gui->set_status_message(status_message[INSOMNIA_RUNNING]);
		return true;
	}

	DWORD process[2048] = {0}, outBytes = 0;

	if(!EnumProcesses(process, sizeof(process), &outBytes))
	{
		gui->set_status_message(status_message[INSOMNIA_NOTRUNNING]);
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
						gui->set_status_message(status_message[INSOMNIA_RUNNING]);
						return true;
					}
				}

				CloseHandle(hProcess);
			}
		}
	}

	gui->set_status_message(status_message[INSOMNIA_NOTRUNNING]);
	return false;
}

void check_thread()
{
	int counter = 0;

	while(run_check_thread)
	{
		if(counter == 0 && check_whitelist())
		{
			if(set.mode_mouse)
				mouse_event( MOUSEEVENTF_MOVE, 0, 0, 0, NULL);

			if(set.mode_display || set.mode_system)
			{
				EXECUTION_STATE state;

				state  = ES_SYSTEM_REQUIRED * set.mode_display;
				state |= ES_DISPLAY_REQUIRED * set.mode_system;
				
				SetThreadExecutionState(state);
			}

			counter = set.loop_length;
		}

		counter--;
		Sleep(1000);
	}
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HWND h_another = FindWindowA("FLTK", "Insomnia");
	if(h_another != NULL)
	{

		ShowWindow(h_another, SW_SHOW);
		SetForegroundWindow(h_another);
		return 0;
	}

	fileio::get_filename();
	fileio::load_settings(whitelist);
	
	gui = new window();
	
	gui->show(0, 0);

	if (set.priority)
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);

	run_check_thread = true;
	std::thread main_thread(check_thread);

	Fl::run();

	fileio::save_settings(whitelist);

	run_check_thread = false;
	main_thread.join();

	// do cleanup ?
	return 0;
}
