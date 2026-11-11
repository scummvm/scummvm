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

#include "tesoro/core/objectlist.h"

#include "tesoro/core/object.h"

namespace Tesoro {

int MainList::runPass(bool second) {
	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i].object->setFlags(_entries[i].object->flags() & ~Object::kPassDone);

	for (;;) {
		bool edited = false;
		for (uint i = 0; i < _entries.size() && !edited; ++i) {
			Object *object = _entries[i].object;
			if (!(object->flags() & Object::kActive) || (object->flags() & Object::kPassDone))
				continue;

			object->setFlags(object->flags() | Object::kPassDone);
			const uint32 before = _edits;
			const int result = second ? object->update2() : object->update();
			if (result == 2 || result == 3)
				return result;
			// An edited list starts the walk again.
			edited = before != _edits;
		}
		if (!edited)
			return 1;
	}
}

} // End of namespace Tesoro
