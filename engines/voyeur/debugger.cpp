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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/debugger.h"
#include "voyeur/screen.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

Debugger::Debugger(VoyeurEngine *vm) : GUI::Debugger(), _vm(vm) {
	// Register methods
	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("time", WRAP_METHOD(Debugger, Cmd_Time));
	registerCmd("hotspots", WRAP_METHOD(Debugger, Cmd_Hotspots));
	registerCmd("mouse", WRAP_METHOD(Debugger, Cmd_Mouse));

	// Set fields
	_isTimeActive = true;
	_showMousePosition = false;
}

static const int TIME_STATES[] = {
	0, 31, 0, 43, 59, 0, 67, 75, 85, 93, 0, 0, 111, 121, 0, 0
};

bool Debugger::Cmd_Time(int argc, const char **argv) {
	if (argc < 2) {
		// Get the current day and time of day
		Common::String dtString = _vm->getDayName();
		Common::String timeString = _vm->getTimeOfDay();
		if (!timeString.empty())
			dtString += " " + timeString;

		debugPrintf("Time period = %d, date/time is: %s, time is %s\n",
			_vm->_voy->_transitionId, dtString.c_str(), _isTimeActive ? "on" : "off");
		debugPrintf("Format: %s [on | off | 1..17 | val <amount>]\n\n", argv[0]);
	} else {
		if (!strcmp(argv[1], "on")) {
			_isTimeActive = true;
			debugPrintf("Time is now on\n\n");
		} else if (!strcmp(argv[1], "off")) {
			_isTimeActive = false;
			debugPrintf("Time is now off\n\n");
		} else if (!strcmp(argv[1], "val")) {
			if (argc < 3) {
				debugPrintf("Time expired is currently %d.\n", _vm->_voy->_RTVNum);
			} else {
				_vm->_voy->_RTVNum = atoi(argv[2]);
				debugPrintf("Time expired is now %d.\n", _vm->_voy->_RTVNum);
			}
		} else {
			int timeId = atoi(argv[1]);
			if (timeId >= 1 && timeId < 17) {
				int stateId = TIME_STATES[timeId - 1];
				if (!stateId) {
					debugPrintf("Given time period is not used in-game\n");
				} else {
					debugPrintf("Changing to time period: %d\n", timeId);
					if (_vm->_mainThread->goToState(-1, stateId))
						_vm->_mainThread->parsePlayCommands();

					return false;
				}
			} else {
				debugPrintf("Unknown parameter\n\n");
			}
		}
	}

	return true;
}

bool Debugger::Cmd_Hotspots(int argc, const char **argv) {
	if (_vm->_voy->_computerTextId >= 0) {
		debugPrintf("Hotspot Computer Screen %d - %d,%d->%d,%d\n",
			_vm->_voy->_computerTextId,
			_vm->_voy->_computerScreenRect.left,
			_vm->_voy->_computerScreenRect.top,
			_vm->_voy->_computerScreenRect.right,
			_vm->_voy->_computerScreenRect.bottom);
	}

#if 0
	// Room hotspots
	BoltEntry &boltEntry = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 4);
	if (boltEntry._rectResource) {
		Common::Array<RectEntry> &hotspots = boltEntry._rectResource->_entries;
		for (uint hotspotIdx = 0; hotspotIdx < hotspots.size(); ++hotspotIdx) {
			Common::String pos = Common::String::format("(%d,%d->%d,%d)",
				hotspots[hotspotIdx].left, hotspots[hotspotIdx].top,
				hotspots[hotspotIdx].right, hotspots[hotspotIdx].bottom);
			int arrIndex = hotspots[hotspotIdx]._arrIndex;
			if (_vm->_voy->_roomHotspotsEnabled[arrIndex - 1]) {
				debugPrintf("Hotspot Room %d - %s - Enabled\n", arrIndex, pos);
			} else {
				debugPrintf("Hotspot Room - %s - Disabled\n", pos);
			}
		}
	}
#endif

	// Outside view hotspots
	BoltEntry &boltEntry = _vm->_bVoy->boltEntry(_vm->_playStampGroupId + 1);
	if (boltEntry._rectResource) {
		Common::Array<RectEntry> &hotspots = boltEntry._rectResource->_entries;

		for (uint hotspotIdx = 0; hotspotIdx < hotspots.size(); ++hotspotIdx) {
			Common::String pos = Common::String::format("(%d,%d->%d,%d)",
				hotspots[hotspotIdx].left, hotspots[hotspotIdx].top,
				hotspots[hotspotIdx].right, hotspots[hotspotIdx].bottom);

			for (int arrIndex = 0; arrIndex < 3; ++arrIndex) {
				if (_vm->_voy->_audioHotspotTimes._min[arrIndex][hotspotIdx] != 9999) {
					debugPrintf("Hotspot %d %s Audio slot %d, time: %d to %d\n",
						hotspotIdx, pos.c_str(), arrIndex,
						_vm->_voy->_audioHotspotTimes._min[arrIndex][hotspotIdx],
						_vm->_voy->_audioHotspotTimes._max[arrIndex][hotspotIdx]);
				}

				if (_vm->_voy->_evidenceHotspotTimes._min[arrIndex][hotspotIdx] != 9999) {
					debugPrintf("Hotspot %d %s Evidence slot %d, time: %d to %d\n",
						hotspotIdx, pos.c_str(), arrIndex,
						_vm->_voy->_evidenceHotspotTimes._min[arrIndex][hotspotIdx],
						_vm->_voy->_evidenceHotspotTimes._max[arrIndex][hotspotIdx]);
				}
			}

			for (int arrIndex = 0; arrIndex < 8; ++arrIndex) {
				if (_vm->_voy->_videoHotspotTimes._min[arrIndex][hotspotIdx] != 9999) {
					debugPrintf("Hotspot %d %s Video slot %d, time: %d to %d\n",
						hotspotIdx, pos.c_str(), arrIndex,
						_vm->_voy->_videoHotspotTimes._min[arrIndex][hotspotIdx],
						_vm->_voy->_videoHotspotTimes._max[arrIndex][hotspotIdx]);
				}
			}
		}
	}

	debugPrintf("\nEnd of list\n");
	return true;
}

bool Debugger::Cmd_Mouse(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("mouse [ on | off ]\n");
	} else {
		_showMousePosition = !strcmp(argv[1], "on");
		debugPrintf("Mouse position is now %s\n", _showMousePosition ? "on" : "off");
	}

	return true;
}

} // End of namespace Voyeur
