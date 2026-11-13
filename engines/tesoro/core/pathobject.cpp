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


#include "tesoro/core/pathobject.h"

#include "tesoro/core/animbank.h"
#include "tesoro/core/level.h"

namespace Tesoro {

void PathObject::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	_flags |= kCollidable | kInRoomList;

	if (_anim.bankCount() == 0) {
		warning("PathObject: object without animation");
		return;
	}

	// Visible only if the first step has an image.
	AnimPlayer &player = _anim.player(0);
	setVisible(player.bank() && player.bank()->step(0).hasImage());

	const byte first = record.dataByte(0);
	player.setRange(first, first, false, false, now);
	player.bank()->clearStepMovement(first);
}

WalkArea &PathObject::area(Jim &jim) {
	if (!_area)
		_area.reset(new WalkArea(jim));
	AnimPlayer &player = _anim.player(0);
	_area->setShape(&player.bank()->step(player.current()));
	_area->setPosition(_x, _y);
	return *_area;
}

int PathObject::update() {
	updateAnimation(_level ? _level->frameTime() : 0);
	return 1;
}

} // End of namespace Tesoro
