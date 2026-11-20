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

#include "tesoro/core/objectgroup.h"

#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"

namespace Tesoro {

void ObjectGroup::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers) {
	initAnimated(record, roomSortOffset, levelPlayers, 0);

	const uint count = record.dataByte(2) | (record.dataByte(3) << 8);
	for (uint i = 0; i < count; ++i) {
		char name[13];
		for (uint c = 0; c < 12; ++c)
			name[c] = (char)record.dataByte(0x10 + 12 * i + c);
		name[12] = '\0';
		_keys.push_back(nameKey64(name));
	}
}

void ObjectGroup::onAttach() {
	_members.clear();
	for (uint i = 0; i < _keys.size(); ++i)
		_members.push_back(_level ? _level->findObject(_keys[i]) : nullptr);
}

} // End of namespace Tesoro
