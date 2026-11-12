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


#include "tesoro/core/walkfloor.h"

#include "common/textconsole.h"
#include "tesoro/core/animbank.h"
#include "tesoro/core/cameraframe.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/pathobject.h"

namespace Tesoro {

void WalkFloor::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	for (uint i = 0; i < 12; ++i)
		_pathName[i] = (char)record.dataByte(1 + i);
	_pathName[12] = '\0';
	_flags |= kCollidable | kInRoomList;

	if (_anim.bankCount() == 0) {
		warning("WalkFloor: object without animation");
		return;
	}

	const byte first = record.dataByte(0);
	AnimPlayer &player = _anim.player(0);
	player.setRange(first, first, false, false, now);
	player.bank()->clearStepMovement(first);
}

int32 WalkFloor::scaleAt(int32 x, int32 y) {
	if (_anim.bankCount() == 0 || !_anim.player(0).bank())
		return 0x100;
	AnimPlayer &player = _anim.player(0);
	if (player.current() >= player.bank()->stepCount())
		return 0x100;
	_area.setShape(&player.bank()->step(player.current()));
	_area.setPosition(_x, _y);
	return _area.scaleAt(x, y);
}

void WalkFloor::onAttach() {
	if (!_level)
		return;

	// Find the path name object by name, keep it only if it's the right kind
	Object *path = _pathName[0] ? _level->findObject(nameKey64(_pathName)) : nullptr;
	_path = path && path->kindId() == 8 ? static_cast<PathObject *>(path) : nullptr;
}

} // End of namespace Tesoro
