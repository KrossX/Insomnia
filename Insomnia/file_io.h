/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#pragma once

namespace fileio
{
	void get_filename();

	void save_settings(std::list<std::string> &whitelist);
	void load_settings(std::list<std::string> &whitelist);
}
