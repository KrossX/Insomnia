/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#pragma once

struct settings
{
	int posx = 200;
	int posy = 200;
	int loop_length = 50;

	bool mode_system = false;
	bool mode_display = false;
	bool mode_mouse = true;

	bool minimized = true;
	bool startup = false;
	bool whitelist = false;
	bool priority = false;
};

extern settings set;