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

#include "tesoro/core/clickcatcher.h"
#include "tesoro/world.h"

#include "tesoro/core/room.h"

namespace Tesoro {

void ClickCatcher::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host) {
	//
	_host = &host;
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	if (_anim.bankCount() > 0) {
		AnimPlayer &player = _anim.current();
		const bool image = player.bank() && player.bank()->step(player.current()).hasImage();
		_flags = (_flags & ~kVisible) | (image ? kVisible : 0) | kCollidable | kInRoomList | kHitter;
	}
	_sortValue = 50;
	_flags |= kCollidable;
}

void ClickCatcher::onAttach() {
	if (_room)
		_room->insertSorted(this);
}

} // End of namespace Tesoro
