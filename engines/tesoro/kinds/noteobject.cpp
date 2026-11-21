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

#include "tesoro/kinds/noteobject.h"
#include "tesoro/world.h"

namespace Tesoro {

void NoteObject::init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host) {
	InteractiveObject::init(record, roomSortOffset, levelPlayers, now, host, 0);
	_data = record.dataByte(1);
}

int NoteObject::verb(int verb) {
	if (verb == 1 || verb == 3) {
		_host->startScript(*this, "FNOTALAB", "mirar", reinterpret_cast<byte *>(&_data));
		return 3;
	}
	return InteractiveObject::verb(verb);
}

} // End of namespace Tesoro
