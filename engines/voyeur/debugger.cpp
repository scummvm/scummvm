/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/debugger.h"
#include "voyeur/graphics.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

Debugger::Debugger() : GUI::Debugger() {
	// Register methods
	DCmd_Register("continue", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("exit", WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("time", WRAP_METHOD(Debugger, Cmd_Time));

	// Set fields
	_isTimeActive = true;
}

bool Debugger::Cmd_Time(int argc, const char **argv) {
	if (argc < 2) {
		// Get the current day and time of day 
		Common::String dtString = _vm->getDayName();
		Common::String timeString = _vm->getTimeOfDay();
		if (!timeString.empty())
			dtString += " " + timeString;

		DebugPrintf("Current date/time is: %s, time is %s\n", 
			dtString.c_str(), _isTimeActive ? "on" : "off");
		DebugPrintf("Format: %s [on | off | 1..17]\n\n", argv[0]);
	} else {
		if (!strcmp(argv[1], "on")) {
			_isTimeActive = true;
			DebugPrintf("Time is now on\n\n");
		} else if (!strcmp(argv[1], "off")) {
			_isTimeActive = false;
			DebugPrintf("Time is now off\n\n");
		} else {
			int timeId = atoi(argv[1]);
			if (timeId >= 1 && timeId <= 17) {
				_vm->_voy._transitionId = timeId;
				_vm->_gameHour = LEVEL_H[timeId - 1];
				_vm->_gameMinute = LEVEL_M[timeId - 1];
				_vm->_voy._isAM = (timeId == 6);

				// Camera back to full charge
				_vm->_voy._RTVNum = 0;
				_vm->_voy._RTANum = 255;

				// Get the new current day and time of day 
				Common::String dtString = _vm->getDayName();
				Common::String timeString = _vm->getTimeOfDay();
				if (!timeString.empty())
					dtString += " " + timeString;

				DebugPrintf("Current date/time is now: %s\n\n", dtString.c_str());
			} else {
				DebugPrintf("Unknown parameter\n\n");
			}
		}
	}

	return true;
}

} // End of namespace Voyeur
