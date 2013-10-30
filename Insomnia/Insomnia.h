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

#pragma once
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <list>

struct sMode
{
	bool System;
	bool Display;
	bool Mouse;

	sMode() :
		System(false),
		Display(false),
		Mouse(true)
	{};
};

struct Settings
{
	int posx, posy;
	int loopLength;

	sMode Mode;

	bool minimized;
	bool wlEnabled;
	bool priority;

	Settings() :
		posx(200),
		posy(200),
		loopLength(50000),
		wlEnabled(false),
		minimized(true)
	{};
};