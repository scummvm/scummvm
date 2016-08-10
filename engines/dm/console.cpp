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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "console.h"
#include "dm.h"
#include "champion.h"
#include "dungeonman.h"
#include "movesens.h"


namespace DM {

bool cstrEquals(const char* a, const char *b) { return strcmp(a, b) == 0; }

class SingleUseFlag {
	bool _flag;
public:
	SingleUseFlag() : _flag(true) {}
	bool check() {
		bool currFlagState = _flag;
		_flag = false;
		return currFlagState;
	}
};

Console::Console(DM::DMEngine* vm) : _vm(vm) {
	_debugGodmodeMana = false;
	_debugGodmodeHP = false;
	_debugGodmodeStamina = false;

	_debugNoclip = false;

	registerCmd(".godmode", WRAP_METHOD(Console, Cmd_godmode));
	registerCmd(".noclip", WRAP_METHOD(Console, Cmd_noclip));
	registerCmd(".pos", WRAP_METHOD(Console, Cmd_pos));
}

bool Console::Cmd_godmode(int argc, const char** argv) {
	if (argc != 3)
		goto argumentError;

	bool setFlagTo;

	if (cstrEquals("on", argv[2])) {
		setFlagTo = true;
	} else if (cstrEquals("off", argv[2])) {
		setFlagTo = false;
	} else
		goto argumentError;

	if (cstrEquals("all", argv[1])) {
		_debugGodmodeHP = _debugGodmodeMana = _debugGodmodeStamina = setFlagTo;
	} else if (cstrEquals("mana", argv[1])) {
		_debugGodmodeMana = setFlagTo;
	} else if (cstrEquals("hp", argv[1])) {
		_debugGodmodeHP = setFlagTo;
	} else if (cstrEquals("stamina", argv[1])) {
		_debugGodmodeStamina = setFlagTo;
	} else
		goto argumentError;

	debugPrintf("God mode set for %s to %s\n", argv[1], argv[2]);
	return true;

argumentError:
	debugPrintf("Usage: %s <all/mana/hp/stamina> <on/off>\n", argv[0]);
	return true;
}

bool Console::Cmd_noclip(int argc, const char** argv) {
	if (argc != 2)
		goto argumentError;

	if (cstrEquals("on", argv[1])) {
		_debugNoclip = true;
		static SingleUseFlag warnedForNoclip;
		if (warnedForNoclip.check())
			debugPrintf("Noclip can cause glitches and crashes.\n");
	} else if (cstrEquals("off", argv[1])) {
		_debugNoclip = false;
	} else
		goto argumentError;

	debugPrintf("Noclip set to %s\n", argv[1]);
	return true;

argumentError:
	debugPrintf("Usage: %s <on/off>\n", argv[0]);
	return true;
}

bool Console::Cmd_pos(int argc, const char** argv) {
	if (argc == 2 && cstrEquals("get", argv[1])) {
		debugPrintf("Position: (%d, %d)  Direction: %s\n", _vm->_dungeonMan->_g306_partyMapX, 
					_vm->_dungeonMan->_g307_partyMapY, debugGetDirectionName(_vm->_dungeonMan->_g308_partyDir));
	} else if (argc == 4 && cstrEquals("set", argv[1])) {
		int x = atoi(argv[2]);
		int y = atoi(argv[3]);
		if (x == 0 || y == 0) {
			debugPrintf("Error, supply two non-null numbers to '%s set' command\n", argv[0]);
			return true;
		}

		Map &currMap = *_vm->_dungeonMan->_g269_currMap;
		// not >= because dimensions are inslucsive
		if (x > currMap._width || y > currMap._height) {
			debugPrintf("Position (%d, %d) is out of bounds, possible values: ([1-%d],[1-%d])\n", x, y,
						currMap._width, currMap._height);
			return true;
		}

		static SingleUseFlag warnForSettingPos;
		if (warnForSettingPos.check())
			debugPrintf("Setting position directly can cause glitches and crashes.\n");
		debugPrintf("Position set to (%d, %d)\n", x, y);
		_vm->_moveSens->f267_getMoveResult(Thing::_party, _vm->_dungeonMan->_g306_partyMapX, _vm->_dungeonMan->_g307_partyMapY, x, y);
	} else
		goto argumentError;

	return true;

argumentError:
	debugPrintf("Usage: %s get\nUsage: %s set <#> <#>\n", argv[0], argv[0]);
	return true;
}
}
