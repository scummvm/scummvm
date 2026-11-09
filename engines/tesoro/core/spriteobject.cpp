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

#include "tesoro/core/spriteobject.h"

#include "tesoro/core/camera.h"
#include "tesoro/core/display.h"
#include "tesoro/core/room.h"

namespace Tesoro {

void SpriteObject::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);

	// the first two data bytes are the step to start on and a mode (0 holds that step hidden, 1 shown, 2 plays it on to the step without an image)
	const byte first = record.dataByte(0);
	const byte mode = record.dataByte(1);

	if (_anim.bankCount() == 0) {
		warning("SpriteObject: object without animation");
		return;
	}

	AnimPlayer &player = _anim.player(0);
	const bool hasImages = player.bank() && player.bank()->imageCount() != 0;
	setVisible(hasImages);

	switch (mode) {
	case 0:
		setVisible(false);
		// fall through
	case 1:
		player.setRange(first, first, false, false, now);
		player.bank()->clearStepMovement(first);
		break;
	case 2:
		player.setRange(first, -1, false, false, now);
		break;
	case 3:
		player.setRange(first, -1, true, false, now);
		break;
	default:
		break;
	}
}

void SpriteObject::updateAnimation(int32 now) {
	_anim.tick(now);
}

void SpriteObject::onAttach() {
	if (_room)
		_room->insertSorted(this);
}

void SpriteObject::draw(const Camera &camera, Display &display) {
	drawAnimation(camera, display, false);
}

} // End of namespace Tesoro
