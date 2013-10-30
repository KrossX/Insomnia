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
const char wlFilename[] = "Insomnia.whitelist";
const char wlDefault[] = "Insomnia.exe\n";

FILETIME wlModtime;

namespace FileIO
{
	bool isWhitelistModified()
	{
		bool isModified = false;

		WIN32_FIND_DATAA finder;
		HANDLE hFile = FindFirstFileA(wlFilename, &finder);

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

	bool CreateWhitelist()
	{
		bool created = false;
		
		std::fstream file;
		file.open(wlFilename, std::ios::out);
		
		if(file.is_open())
		{
			file.write(wlDefault, sizeof(wlDefault));
			file.close();
			created = true;
		}

		return created;
	}

	bool LoadWhitelist(std::list<std::string> &whitelist)
	{
		bool whitelistLoaded = false;

		std::fstream file;
		file.open(wlFilename, std::ios::in);
		
		if(file.is_open())
		{
			char line[LINE_LENGTH+1] = {0};

			while(!file.eof())
			{
				file.getline(line, LINE_LENGTH);

				if(strlen(line) > 3)
				{
					std::string sLine(line);
					whitelist.push_back(line);
				}
			}

			file.close();
			
			if(!whitelist.empty()) 
			{
				whitelistLoaded = true;
				isWhitelistModified();
			}
		}

		return whitelistLoaded;
	}
} // End namespace FileIO
