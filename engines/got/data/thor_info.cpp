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
 * as.syncAsUint32LE(with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "got/data/thor_info.h"
#include "common/algorithm.h"
#include "got/game/back.h"

namespace Got {

void THOR_INFO::clear() {
	magic = 0;
	keys = 0;
	jewels = 0;
	last_area = 0;
	last_screen = 0;
	last_icon = 0;
	last_dir = 0;
	inventory = 0;
	item = 0;
	last_health = 0;
	last_magic = 0;
	last_jewels = 0;
	last_keys = 0;
	last_item = 0;
	last_inventory = 0;
	level = 0;
	score = 0;
	last_score = 0;
	object = 0;
	object_name = nullptr;
	last_object = 0;
	last_object_name = nullptr;
	armor = 0;
	Common::fill(future, future + 65, 0);
}

void THOR_INFO::sync(Common::Serializer &s) {
	uint32 ptr = 0;

	s.syncAsByte(magic);
	s.syncAsByte(keys);
	s.syncAsSint16LE(jewels);
	s.syncAsByte(last_area);
	s.syncAsByte(last_screen);
	s.syncAsByte(last_icon);
	s.syncAsByte(last_dir);
	s.syncAsSint16LE(inventory);
	s.syncAsByte(item);
	s.syncAsByte(last_health);
	s.syncAsByte(last_magic);
	s.syncAsSint16LE(last_jewels);
	s.syncAsByte(last_keys);
	s.syncAsByte(last_item);
	s.syncAsSint16LE(last_inventory);
	s.syncAsByte(level);
	s.syncAsUint32LE(score);
	s.syncAsUint32LE(last_score);

	s.syncAsByte(object);
	s.syncAsUint16LE(ptr);
	s.syncAsByte(last_object);
	s.syncAsUint16LE(ptr);

	s.syncAsByte(armor);
	s.syncBytes(future, 65);

	if (s.isLoading()) {
		object_name = (object == 0) ? nullptr : OBJECT_NAMES[object - 1];
		last_object_name = (last_object == 0) ? nullptr : OBJECT_NAMES[last_object - 1];
	}
}

} // namespace Got
