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

#include <stdio.h>
#include <string.h>

#include "tesoro/kinds/slotlabel.h"
#include "tesoro/world.h"

namespace Tesoro {

void SlotLabel::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world) {
	_world = &world;
	initAnimated(record, roomSortOffset, levelPlayers, 0);
	if (AnimPlayer *player = initClickable())
		player->setRange(record.dataByte(0), record.dataByte(0), false, false, now);

	_sortValue = 2;
	_caption = record.dataByte(1) != 0;
	_flags = (_flags & ~kVisible) | (_caption ? kVisible : 0);
	if (_caption)
		_sortValue = 0;
}

void SlotLabel::draw(const Camera &camera, Display &display) {
	if (_caption && _world->interfaceCameraX() < 0) {
		const int slot = _world->selectedSlot();
		if (slot == 0) {
			_world->drawMenuText("POSICION VACIA", 0xffff, 0x136, 300, 200);
		} else {
			char number[16];
			snprintf(number, sizeof number, "%u", (uint)slot);
			_world->drawMenuText(number, 0xf800, 450, 255, 200);
			// The date is shown in two pieces: the first ten characters (day), and the rest from the twelfth (time).
			const Common::String date = _world->slotTime();
			_world->drawMenuText(date.substr(0, 10).c_str(), 0xffff, 450, 300, 200);
			_world->drawMenuText(date.size() > 11 ? date.substr(11).c_str() : "", 0xffff, 450, 0x159, 200);
		}
	}
	InteractiveObject::draw(camera, display);
}

} // End of namespace Tesoro
