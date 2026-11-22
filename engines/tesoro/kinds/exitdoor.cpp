/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"

#include "tesoro/core/namekey.h"
#include "tesoro/kinds/exitdoor.h"
#include "tesoro/world.h"

namespace Tesoro {

enum Place { kInterface = -1, kTavern = 2, kKitchen = 6, kRisco = 16 };

// The amount state bytes each subtype keeps (a subtype beyond the table keeps none).
static const byte kState[11] = { 0, 1, 1, 2, 1, 1, 0, 1, 1, 0, 1 };

void ExitDoor::init(int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	_subtype = subtype;
	InteractiveObject::init(record, roomSortOffset, levelPlayers, now, world, subtype >= 0 && subtype <= 10 ? kState[subtype] : 0);
	_destLevel = (int8)record.dataByte(0x67);
	char name[13];
	for (uint i = 0; i < 12; ++i)
		name[i] = (char)record.dataByte(0x68 + i);
	name[12] = '\0';
	_destKey = nameKey64(name);
	_arrivalFacing = record.dataByte(0x74) + 1;
}

int ExitDoor::runWith(const char *block, const char *label, byte *variables) {
	return _world->startScript(*this, block, label, variables);
}

/*
finds the destination object by level and key, gets level, jim faces the door, puts him in new level, walk to target object. */
int ExitDoor::goThrough() {
	if (_verb != 0 || _world->jimPhase() != 0)
		return 0;
	Object *target = _world->findObjectByKey(_destLevel, _destKey);
	if (!target) {
		warning("Tesoro: an exit leads to an object that is not there");
		return 0;
	}
	_world->jimFace(_facing);
	_world->resetRenderQueue();
	_world->drawRooms(_world->jimLevel());
	_world->composeFrame();
	_world->clearScreens();
	_world->fadeOut(4, 100);
	_world->wait(300);
	_world->jimFace(_arrivalFacing);
	_world->putJimInLevel(_world->levelAt(_destLevel), target->approachX(), target->approachY());
	_world->jimUpdate();
	_world->setHoverText(nullptr);
	target->exitArrival();
	return 3;
}

int ExitDoor::cabinArrival() {
	byte *s = state();
	if (_world->jimStep() == 0) {
		Object *plate = _world->findObject(kInterface, "IPLATO");
		_world->setWalkAreaFrames(_level, World::kFloor, 0, 0);
		_world->setWalkAreaFrames(_level, World::kRoute, 1, 1);
		if (plate && !plate->stateBlob().empty() && plate->stateBlob()[0] == 2) {
			_world->setJimStepKeep(1);
			s[0] = 6;
		}
		if (!plate || (plate->flags() & kActive) == 0) {
			s[0] = (byte)_world->randomNumber(0, 3);
			_world->setWalkAreaFrames(_level, World::kFloor, 2, 2);
			_world->setWalkAreaFrames(_level, World::kRoute, 3, 3);
			_world->jimUpdate();
		} else if ((int8)s[0] < 4) {
			s[0] = 4;
		}
		runWith("FANCAMAROTE", "main", s);
	}
	return 1;
}

int ExitDoor::labArrival() {
	Level *where = _world->jimLevel();
	if (where == _world->levelAt(23)) {
		if (state()[0] != 0)
			runWith("FPUERTALAB", "entrada", nullptr);
		state()[0] = 1;
		return 1;
	}
	if (where == _world->levelAt(34))
		runWith("FPUERTALAB", "drascula", nullptr);
	else if (where == _world->levelAt(43))
		runWith("FANMLGTES", "main", nullptr);
	return 1;
}

int ExitDoor::exitArrival() {
	if (_subtype == 3)
		return cabinArrival();
	if (_subtype == 6)
		return 0;
	if (_subtype == 10)
		return labArrival();
	if (_world->jimLevel() == _world->levelAt(kRisco) && !_world->jimEntered()) {
		_world->startMovie(9);
		_world->setJimEntered(true);
	}
	return 0;
}

int ExitDoor::jumpThrough(int verb) {
	if (_world->jimLevelIndex() < 0x17 && verb == 0) {
		_world->jimMoveTo(_approachX, _approachY);
		_world->jimUpdate();
		_world->jimRemoveFromMainList();
		return arrived();
	}
	return 0;
}

int ExitDoor::doubleClick(int verb) {
	if (_subtype == 8 && state()[0] == 0)
		return 1;
	return jumpThrough(verb);
}

int ExitDoor::verb(int verb) {
	byte *s = state();
	byte flag = 0;
	if (verb == 4 && _subtype == 8) {
		s[0] = s[0] == 0;
		setVisible(s[0] != 0);
		return 1;
	}
	if (verb != 0)
		return InteractiveObject::verb(verb);
	switch (_subtype) { // subtype here is relevant to decide whether to trigger a script.
	case 1:
		if (_world->isActive(kInterface, "IJARRALLENA")) {
			s[0] = 1;
		} else if (_world->isActive(kInterface, "IJARRA")) {
			s[0] = 0;
		} else {
			goThrough();
			return 1;
		}
		runWith("FSALTABERNA", "main", s);
		return 1;
	case 2:
		if (goThrough() == 0)
			return 0;
		if (s[0] == 0) {
			s[0] = 1;
			_world->startScriptInLevel(kTavern, "FANTABERNA", "main", nullptr);
		}
		return 3;
	case 3:
	case 9:
		if (_world->jimPhase() != 0) {
			if (_subtype == 3 && _world->isActive(kInterface, "IMAPA"))
				flag = 1;
			runWith("FSALCAMAROTE", "ira", &flag);
			return 3;
		}
		return goThrough();
	case 4:
		if (goThrough() == 0)
			return 0;
		if (s[0] == 0) {
			s[0] = 1;
			Object *dick = _world->findObject(kKitchen, "DICK");
			_world->startScriptInLevel(kKitchen, "FANPATATAS", "main", dick && !dick->stateBlob().empty() ? dick->stateBlob().data() : nullptr);
		}
		return 1;
	case 5:
		if (s[0] == 0) {
			runWith("FANPLAYA", "main", nullptr);
			s[0] = 1;
			return 1;
		}
		return goThrough();
	case 7:
		runWith("FENTCUEVA", "ir", nullptr);
		return 1;
	case 8:
		if (s[0] != 0)
			return 1;
		return goThrough();
	default:
		return goThrough();
	}
}

} // End of namespace Tesoro
