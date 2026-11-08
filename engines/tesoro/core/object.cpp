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


#include "common/endian.h"
#include "common/textconsole.h"

#include "tesoro/core/display.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/object.h"

namespace Tesoro {

void Object::initPlain() {
	_flags = kActive | kVisible;
	_sortValue = 5;
}

void Object::initAnimated(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, uint16 stateSize) {
	_flags |= kActive;
	_x = record.x;
	_y = record.y;
	_sortValue = (int16)(record.sortOffset + roomSortOffset + 5);
	_nameKey = nameKey64(record.name);

	_anim.init(levelPlayers, record.banks.empty() ? nullptr : &record.banks[0], record.banks.size());
	if (!record.banks.empty())
		_flags |= kHasAnimation;

	_previousX = _x;
	_previousY = _y;
	_stateSize = stateSize;
	_hasStateBlob = stateSize != 0;
	_stateBlob.clear();
	_stateBlob.resize(stateSize);
}

void Object::updateAnimation(int32 now) {
	if (_flags & kHasAnimation)
		_anim.tick(now);
}

void Object::updateBounds(int32 now) {
	AnimPlayer *current = currentPlayer();
	if (!current)
		return;

	AnimPlayer &player = *current;
	_movedArea.x1 = player.movementX() + _x;
	_movedArea.y1 = player.movementY() + _y;

	updateAnimation(now);

	// The area the object moved through, from where it was drawn to where it is now.
	AnimPlayer &after = *currentPlayer();
	_movedArea.x2 = after.movementX() + _x;
	_movedArea.y2 = after.movementY() + _y;
	if (_previousX != _x || _previousY != _y) {
		_movedArea.x2 = _previousX + after.movementX();
		_movedArea.y2 = _previousY + after.movementY();
	}

	const AnimStep &seq = after.bank()->step(after.current());
	_hitRect.x1 = seq.hitX;
	_hitRect.x2 = seq.hitW + seq.hitX;
	_hitRect.y1 = seq.hitY;
	_hitRect.y2 = seq.hitY - seq.hitH;
}

void Object::moveTo(int32 x, int32 y, bool setCurrent) {
	if (setCurrent) {
		_x = x;
		_y = y;
	}
	_previousX = x;
	_previousY = y;
}

} // End of namespace Tesoro
