/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#include "insomnia.h"
#include "settings.h"
#include "file_io.h"

#include <iostream>
#include <fstream>

#define LINE_LENGTH 80

bool filename_acquired = false;
char ini_filename[MAX_PATH];

extern settings set;
FILETIME wlModtime;

namespace fileio
{
	std::fstream ini_file;


	void get_filename()
	{
		int length = GetModuleFileNameA(NULL, ini_filename, MAX_PATH);

		if (length > 4)
		{
			char extension[] = "ini";
			memcpy(&ini_filename[length - 3], extension, 3);
		}
	}


	bool save_entry(std::string section, std::string key, int value)
	{
		static std::string prev_section;
		
		std::string line = key + "=" + std::to_string(value) + "\n";
		section = "[" + section + "]";

		if(section != prev_section)
		{
			prev_section = section;
			section.append("\n");
			ini_file << section;
		}

		ini_file << line;
		return true;
	}

	int read_entry(std::string section, std::string key)
	{
		std::string line;

		section = "[" + section + "]";

		int value = -1;
		bool sectionFound = false;

		ini_file.seekg(0);
		while(std::getline(ini_file, line))
		{
			if(line == section)
			{
				sectionFound = true;
			}
			else if(sectionFound)
			{
				size_t length = key.length();

				if(line.compare(0, length, key) == 0 && (line[length] == '='))
				{
					value = std::stoi(&line[length + 1]);
					return value;
				}
			}
		}

		return value;
	}

	void load_whitelist(std::list<std::string> &whitelist)
	{
		whitelist.clear();

		std::string line;
		std::string section("[Whitelist]");
		bool sectionFound = false;

		ini_file.seekg(0);
		while (std::getline(ini_file, line))
		{
			if (line == section)
			{
				sectionFound = true;
			}
			else if(sectionFound && line.length() > 3)
			{
				std::string entry = line.substr(0, line.find_last_of('e') +1);
				whitelist.push_back(entry);
			}
		}
	}

	void save_whitelist(std::list<std::string> &whitelist)
	{
		std::string line;

		ini_file << "[Whitelist]\n";

		if (whitelist.empty())
		{
			ini_file << "Insomnia.exe";
		}
		else for each (std::string entry in whitelist)
		{
			ini_file << entry + "\n";
		}
	}

	void load_settings(std::list<std::string> &whitelist)
	{
		ini_file = std::fstream(ini_filename, std::ios::in);

		if(ini_file.is_open())
		{
			set.posx        = read_entry("General", "WindowPosX");
			set.posy        = read_entry("General", "WindowPosY");
			set.loop_length = read_entry("General", "LoopLength");
			set.whitelist   = read_entry("General", "Whitelist") == 1;
			set.minimized   = read_entry("General", "Minimized") == 1;
			set.priority    = read_entry("General", "AboveNormalPriority") == 1;

			set.mode_system  = read_entry("General", "Mode_System") == 1;
			set.mode_display = read_entry("General", "Mode_Display") == 1;
			set.mode_mouse   = read_entry("General", "Mode_Mouse") == 1;

			set.loop_length = max(set.loop_length, 1);
			load_whitelist(whitelist);

			ini_file.close();
		}

	}

	void save_settings(std::list<std::string> &whitelist)
	{
		ini_file = std::fstream(ini_filename, std::ios::out);

		if(ini_file.is_open())
		{
			std::string current_section;

			save_entry("General", "WindowPosX", set.posx);
			save_entry("General", "WindowPosY", set.posy);
			save_entry("General", "LoopLength", set.loop_length);
			save_entry("General", "Whitelist", set.whitelist ? 1 : 0);
			save_entry("General", "Minimized", set.minimized ? 1 : 0);
			save_entry("General", "AboveNormalPriority", set.priority ? 1 : 0);

			save_entry("General", "Mode_System", set.mode_system ? 1 : 0);
			save_entry("General", "Mode_Display", set.mode_display ? 1 : 0);
			save_entry("General", "Mode_Mouse", set.mode_mouse ? 1 : 0);

			save_whitelist(whitelist);

			ini_file.close();
		}
	}
		
} // End namespace FileIO
