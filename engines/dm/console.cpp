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

#include "dm/console.h"
#include "dm/dm.h"
#include "dm/champion.h"
#include "dm/dungeonman.h"
#include "dm/movesens.h"
#include "dm/objectman.h"


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

const char *Console::debugGetDirectionName(int16 dir) {
	static const char *directionNames[] = {"North", "East", "South", "West"};
	if (dir < 0 || dir > 3)
		return "Invalid direction";
	return directionNames[dir];
}

Console::Console(DM::DMEngine* vm) : _vm(vm) {
	_debugGodmodeMana = false;
	_debugGodmodeHP = false;
	_debugGodmodeStamina = false;

	_debugNoclip = false;

	registerCmd("godmode", WRAP_METHOD(Console, Cmd_godmode));
	registerCmd("noclip", WRAP_METHOD(Console, Cmd_noclip));
	registerCmd("pos", WRAP_METHOD(Console, Cmd_pos));
	registerCmd("map", WRAP_METHOD(Console, Cmd_map));
	registerCmd("listItems", WRAP_METHOD(Console, Cmd_listItems));
	registerCmd("gimme", WRAP_METHOD(Console, Cmd_gimme));
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
		static SingleUseFlag haventWarned;
		if (haventWarned.check())
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
	DungeonMan &dm = *_vm->_dungeonMan;
	if (argc == 2 && cstrEquals("get", argv[1])) {
		debugPrintf("Position: (%d, %d)  Direction: %s\n", dm._partyMapX + dm._currMap->_offsetMapX,
					dm._partyMapY + dm._currMap->_offsetMapY, debugGetDirectionName(_vm->_dungeonMan->_partyDir));
	} else if (argc == 4 && cstrEquals("set", argv[1])) {
		int x = atoi(argv[2]);
		int y = atoi(argv[3]);
		if ((x == 0 && !cstrEquals("0", argv[2])) || (y == 0 && !cstrEquals("0", argv[3]))) {
			debugPrintf("Error, supply two numbers to '%s set' command\n", argv[0]);
			return true;
		}

		Map &currMap = *_vm->_dungeonMan->_currMap;
		// not >= because dimensions are inslucsive
		if (x < currMap._offsetMapX || x > currMap._width + currMap._offsetMapX
			|| y < currMap._offsetMapY || y > currMap._height + currMap._offsetMapY) {
			debugPrintf("Position (%d, %d) is out of bounds, possible values: ([1-%d],[1-%d])\n", x, y,
						currMap._width + currMap._offsetMapX, currMap._height + currMap._offsetMapY);
			return true;
		}

		static SingleUseFlag haventWarned;
		if (haventWarned.check())
			debugPrintf("Setting position directly can cause glitches and crashes.\n");
		debugPrintf("Position set to (%d, %d)\n", x, y);
		_vm->_moveSens->getMoveResult(_vm->_thingParty, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY,
										   x - currMap._offsetMapX, y - currMap._offsetMapY);
	} else
		goto argumentError;

	return true;

argumentError:
	debugPrintf("Usage: %s get\n", argv[0]);
	debugPrintf("Usage: %s set <#> <#>\n", argv[0]);
	return true;
}

bool Console::Cmd_map(int argc, const char** argv) {
	if (argc == 2 && cstrEquals("get", argv[1])) {
		debugPrintf("Map index: %d\n", _vm->_dungeonMan->_partyMapIndex);
	} else if (argc == 3 && cstrEquals("set", argv[1])) {
		int index = atoi(argv[2]);
		if (index == 0 && !cstrEquals("0", argv[2])) {
			debugPrintf("Error, supply a number to '%s set' command\n", argv[0]);
			return true;
		}

		// not >= because dimensions are inslucsive
		if (index < 0 || index >= _vm->_dungeonMan->_dungeonFileHeader._mapCount) {
			debugPrintf("Map index %d is out of bounds, possible values [0, %d]\n", index, _vm->_dungeonMan->_dungeonFileHeader._mapCount - 1);
			return true;
		}

		static SingleUseFlag haventWarned;
		if (haventWarned.check())
			debugPrintf("Setting map directly can cause glitches and crashes.\n");
		debugPrintf("Map set to %d\n", index);

		_vm->_moveSens->getMoveResult(_vm->_thingParty, _vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY, kDMMapXNotOnASquare, 0);
		_vm->_newPartyMapIndex = _vm->_dungeonMan->getLocationAfterLevelChange(
			_vm->_dungeonMan->_partyMapIndex, index - _vm->_dungeonMan->_partyMapIndex,
			&_vm->_dungeonMan->_partyMapX, &_vm->_dungeonMan->_partyMapY);
		if (_vm->_newPartyMapIndex == -1)
			_vm->_newPartyMapIndex = index;
		_vm->_dungeonMan->setCurrentMap(_vm->_newPartyMapIndex);
		_vm->_championMan->setPartyDirection(_vm->_dungeonMan->getStairsExitDirection(_vm->_dungeonMan->_partyMapX, _vm->_dungeonMan->_partyMapY));
		_vm->_dungeonMan->setCurrentMap(_vm->_dungeonMan->_partyMapIndex);
	} else
		goto argumentError;

	return true;

argumentError:
	debugPrintf("Usage: %s get\n", argv[0]);
	debugPrintf("Usage: %s set <#>\n", argv[0]);
	return true;
}

bool Console::Cmd_listItems(int argc, const char** argv) {
	Common::String searchedString = "";
	for (int16 i = 1; i < argc; ++i) {
		searchedString += argv[i];
		searchedString += " ";
	}
	searchedString.deleteLastChar();

	bool atleastOneFound = false;
	int16 namesPrintedInLine = 0;

	if(strstr(_vm->_objectMan->_objectNames[0], searchedString.c_str()) != nullptr)
		debugPrintf("| %s", _vm->_objectMan->_objectNames[0]);

	for (uint16 i = 1; i < kDMObjectNameCount; ++i) {
		const char *name = _vm->_objectMan->_objectNames[i - 1];
		const char *prevName = _vm->_objectMan->_objectNames[i];

		if (!cstrEquals(name, prevName) && (strstr(name, searchedString.c_str()) != nullptr)) {
			debugPrintf(" | %s", name);
			atleastOneFound = true;

			if ((++namesPrintedInLine % 6) == 0) {
				namesPrintedInLine = 0;
				debugPrintf("\n");
			}
		}
	}

	if (atleastOneFound) {
		debugPrintf("\n");
	} else {
		debugPrintf("No itemnames found containing '%s'\n", searchedString.c_str());
	}

	return true;
}

bool Console::Cmd_gimme(int argc, const char** argv) {
	if (argc < 2) {
		debugPrintf("Usage: gimme <item name>   // item name can have spaces\n");
		return true;
	}

	Common::String requestedItemName;
	for (int16 i = 1; i < argc; ++i) {
		requestedItemName += argv[i];
		requestedItemName += " ";
	}
	requestedItemName.deleteLastChar();

	for (int16 thingType = 0; thingType < 16; ++thingType) { // 16 number of item types
		uint16 *thingDataArray = _vm->_dungeonMan->_thingData[thingType];
		uint16 thingTypeSize = _vm->_dungeonMan->_thingDataWordCount[thingType];
		uint16 thingCount = _vm->_dungeonMan->_dungeonFileHeader._thingCounts[thingType];

		Thing dummyThing(0);
		dummyThing.setType(thingType);
		for (int16 thingIndex = 0; thingIndex < thingCount; ++thingIndex) {
			dummyThing.setIndex(thingIndex);
			int16 iconIndex = _vm->_objectMan->getIconIndex(dummyThing);
			if (iconIndex != -1) {
				const char *displayName = _vm->_objectMan->_objectNames[iconIndex];
				if (cstrEquals(displayName, requestedItemName.c_str())) {
					uint16 *newThingData = new uint16[(thingCount + 1) * thingTypeSize];
					memcpy(newThingData, thingDataArray, sizeof(uint16) * thingTypeSize * thingCount);
					delete[] thingDataArray;
					for (uint16 i = 0; i < thingTypeSize; ++i)
						newThingData[thingCount * thingTypeSize + i] = newThingData[thingIndex * thingTypeSize + i];
					_vm->_dungeonMan->_dungeonFileHeader._thingCounts[thingType]++;
					_vm->_dungeonMan->_thingData[thingType] = newThingData;
					_vm->_championMan->addObjectInSlot((ChampionIndex)0, dummyThing, (ChampionSlot)29);
					debugPrintf("Item gimmed to the first champion, last slot\n");
					return true;
				}
			}
		}
	}

	debugPrintf("No item found with name '%s'\n", requestedItemName.c_str());
	return true;
}

}
