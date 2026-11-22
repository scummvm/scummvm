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

#include "tesoro/kinds/mapplace.h"
#include "tesoro/world.h"

namespace Tesoro {

enum { kFirstPlace = 7, kSacredIsland = 15, kCanoaLevel = 7 };

void MapPlace::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	_world = &world;
	initAnimated(record, roomSortOffset, levelPlayers, 0);

	// The data's third byte is the level; the first levels of the list are not places.
	const int level = (int8)record.dataByte(2);
	_destination = level < kFirstPlace ? -1 : level;
	if (record.dataByte(3) != 0) {
		char name[13];
		for (uint i = 0; i < 12; ++i)
			name[i] = (char)record.dataByte(3 + i);
		name[12] = '\0';
		_hoverName = world.phrase(name);
	}

	if (_anim.bankCount() > 0) {
		const byte first = record.dataByte(0);
		AnimPlayer &player = _anim.current();
		player.setRange(first, first, false, false, now);
		player.bank()->clearStepMovement(player.current());
	}
	_flags |= kCollidable | kInRoomList | kHitter;
}

int MapPlace::setVerbAndClick(int verb) {
	if (_destination < 0)
		return 0;

	if (_destination == kSacredIsland) {
		Object *canoe = _world->findObject(kCanoaLevel, "CANOA");
		if (!canoe)
			warning("Tesoro: the map place of the sacred island found no canoe");
		else if (!canoe->stateBlob().empty() && canoe->stateBlob()[0] == 0) {
			_world->startScript(*this, "FMAPA", "tsagrada", nullptr);
			return 1;
		}
	}

	Level *level = _world->levelAt(_destination);
	Object *exit = _world->findObject(_destination, "SALIDA");
	if (!exit) {
		warning("Tesoro: a map place leads to a level without an exit");
		return 0;
	}
	_world->putJimInLevel(level, exit->approachX(), exit->approachY());
	_world->jimFace(((exit->facing() - 1) ^ 1) + 1);
	_world->setHoverText(nullptr);
	exit->exitArrival();
	return 3;
}

} // End of namespace Tesoro
