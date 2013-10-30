===============================================================================
		Insomnia : https://code.google.com/p/magical-tools/
===============================================================================

-------------------------------------------------------------------------------
Description / Usage
-------------------------------------------------------------------------------

A stand alone version of Pokopom's "Prevent Screensaver" option.

By default, just starting the application will be enough.


Settings
--------

Mode - System / Display Required: In theory, should avoid sleep of the system
and display respectively. See link 1 for more info.

Mode - Bogus Mouse Even: Sends a null mouse event on each loop (59 seconds) to
reset the idle counters. Effectively avoiding the screensaver from showing up.

Run on Startup: Creates a user registry entry to run Insomnia from its current
location on each start.

Whitelist: The checkbox enables/disables the usage of the whitelist, and can
also create and load a new one if there was none loaded.

When the button is enabled, the whitelist file (Insomnia.whitelist)
has been loaded. 


Links
-----
1.- http://msdn.microsoft.com/en-us/library/windows/desktop/aa373208%28v=vs.85%29.aspx

-------------------------------------------------------------------------------
License / Disclaimer
-------------------------------------------------------------------------------

Copyright (C) 2012  KrossX (krossx@live.com)

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 3 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.