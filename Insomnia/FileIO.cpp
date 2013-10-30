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
#include <fstream>

//#include <stdlib.h>

#define LINE_LENGTH 80

char iniFilename[MAX_PATH];
const char wlDefault[] = "Insomnia.exe\n";

extern Settings set;
FILETIME wlModtime;

namespace FileIO
{
	bool isINIModified()
	{
		bool isModified = false;

		WIN32_FIND_DATAA finder;
		HANDLE hFile = FindFirstFileA(iniFilename, &finder);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME current = finder.ftLastWriteTime;

			if((current.dwHighDateTime != wlModtime.dwHighDateTime) ||
				(current.dwLowDateTime != wlModtime.dwLowDateTime))
			{
				isModified = true;
				wlModtime.dwHighDateTime = current.dwHighDateTime;
				wlModtime.dwLowDateTime = current.dwLowDateTime;
			}
		}

		return isModified;
	}

	bool SaveEntry(const char *sec, const char *key, int value, FILE *iniFile, char *secbuff)
	{
		char newsec[LINE_LENGTH+1], line[LINE_LENGTH+1];

		memset(newsec, 0, sizeof(newsec));
		memset(line, 0, sizeof(line));

		sprintf(newsec, "[%s]", sec);

		if(strcmp(newsec, secbuff) != 0)
		{
			memcpy(secbuff, newsec, LINE_LENGTH);
			sprintf(line, "%s\n", newsec);
			fputs(line, iniFile);
		}

		sprintf(line, "%s=%d\n", key, value);
		fputs(line, iniFile);

		return true;
	}

	int ReadEntry(const char *sec, const char *key, FILE *iniFile)
	{
		char section[LINE_LENGTH+1], line[LINE_LENGTH+1];

		memset(section, 0, sizeof(section));
		memset(line, 0, sizeof(line));

		int value = -1, length;
		bool sectionFound = false;

		fseek(iniFile, 0, SEEK_SET);

		sprintf(section, "[%s]", sec);

		while(fgets(line, LINE_LENGTH, iniFile) != NULL)
		{
			if(memcmp(line, section, strlen(section)) == 0)
			{
				sectionFound = true;
			}
			else if(sectionFound)
			{
				length = strlen(key);

				if(memcmp(line, key, length) == 0)
				{
					if(line[length] == '=')
					{
						value = atoi(&line[strlen(key)+1]);
						return value;
					}
				}
			}
		}

		return value;
	}

	void LoadWhitelist(std::list<std::string> &whitelist, FILE *iniFile)
	{
		whitelist.clear();

		bool sectionFound = false;

		char section[] = "[Whitelist]";

		char line[LINE_LENGTH+1];
		memset(line, 0, sizeof(line));

		fseek(iniFile, 0, SEEK_SET);

		while(fgets(line, LINE_LENGTH, iniFile) != NULL)
		{

			if(memcmp(line, section, strlen(section)) == 0)
			{
				sectionFound = true;
			}
			else if(sectionFound)
			{
				if(strlen(line) > 3)
				{
					std::string sLine(line);
					sLine = sLine.substr(0, sLine.find_last_of('e') +1);
					whitelist.push_back(sLine);
				}
			}
		}
	}

	void SaveWhitelist(std::list<std::string> &whitelist, FILE *iniFile, char *secbuff)
	{
		char newsec[LINE_LENGTH+1] = "[Whitelist]";
		char line[LINE_LENGTH+1];

		memset(line, 0, sizeof(line));

		if(strcmp(newsec, secbuff) != 0)
		{
			memcpy(secbuff, newsec, LINE_LENGTH);
			sprintf(line, "%s\n", newsec);
			fputs(line, iniFile);
		}

		if(whitelist.empty())
			fputs("Insomnia.exe\n", iniFile);
		else
		for each (std::string sProcess in whitelist)
		{
			sprintf(line, "%s\n", sProcess.c_str());
			fputs(line, iniFile);
		}
	}

	bool LoadSettings(std::list<std::string> &whitelist)
	{
		bool loaded = false;
		FILE* iniFile = fopen(iniFilename, "r");
		
		if(iniFile)
		{
			set.posx = ReadEntry("General", "WindowPosX", iniFile);
			set.posy = ReadEntry("General", "WindowPosY", iniFile);
			set.loopLength = ReadEntry("General", "LoopLength", iniFile);
			set.wlEnabled = ReadEntry("General", "Whitelist", iniFile) == 1;
			set.minimized = ReadEntry("General", "Minimized", iniFile) == 1;
			set.priority = ReadEntry("General", "AboveNormalPriority", iniFile) == 1;

			set.Mode.System = ReadEntry("General", "Mode_System", iniFile) == 1;
			set.Mode.Display = ReadEntry("General", "Mode_Display", iniFile) == 1;
			set.Mode.Mouse = ReadEntry("General", "Mode_Mouse", iniFile) == 1;

			LoadWhitelist(whitelist, iniFile);

			loaded = true;
			fclose(iniFile);
		}

		return loaded;
	}

	void SaveSettings(std::list<std::string> &whitelist)
	{
		FILE* iniFile = fopen(iniFilename, "w");

		if(iniFile)
		{
			char secbuff[LINE_LENGTH+1];

			SaveEntry("General", "WindowPosX", set.posx, iniFile, secbuff);
			SaveEntry("General", "WindowPosY", set.posy, iniFile, secbuff);
			SaveEntry("General", "LoopLength", set.loopLength, iniFile, secbuff);
			SaveEntry("General", "Whitelist", set.wlEnabled ? 1 : 0, iniFile, secbuff);
			SaveEntry("General", "Minimized", set.minimized ? 1 : 0, iniFile, secbuff);
			SaveEntry("General", "AboveNormalPriority", set.priority ? 1 : 0, iniFile, secbuff);

			SaveEntry("General", "Mode_System", set.Mode.System ? 1 : 0, iniFile, secbuff);
			SaveEntry("General", "Mode_Display", set.Mode.Display ? 1 : 0, iniFile, secbuff);
			SaveEntry("General", "Mode_Mouse", set.Mode.Mouse ? 1 : 0, iniFile, secbuff);

			SaveWhitelist(whitelist, iniFile, secbuff);

			fclose(iniFile);
		}
	}

	void OpenSettings()
	{
		char command[512];
		sprintf_s(command, "start /wait notepad.exe %s", iniFilename);
		system(command); // Meh
	}

	void GetFilename()
	{
		memset(iniFilename, 0, sizeof(iniFilename));
		int nLength = GetModuleFileNameA(NULL, iniFilename, MAX_PATH);

		if(nLength > 4)
		{
			char extension[] = "ini";
			memcpy(&iniFilename[nLength-3], extension, sizeof(extension));
		}
	}

	
} // End namespace FileIO
