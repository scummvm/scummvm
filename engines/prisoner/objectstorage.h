/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PRISONER_OBJECTSTORAGE_H
#define PRISONER_OBJECTSTORAGE_H

#include "common/array.h"

namespace Prisoner {

class PrisonerEngine;

template<class T, int16 N>
class ObjectStorage {
public:
	ObjectStorage() {
		clear();
	}
	int16 getFreeSlot() {
		for (int16 i = 0; i < N; i++)
			if (_objects[i].isEmpty())
				return i;
		error("ObjectStorage::getFreeSlot() No free slots found");
		return -1;
	}
	void clear() {
		for (int16 i = 0; i < N; i++)
			_objects[i].clear();
	}
	T& operator[](int16 index) {
		return _objects[index];
	}
	const T& operator[](int16 index) const {
		return _objects[index];
	}
	int16 count() const { return N; }
	void save(PrisonerEngine *vm, Common::WriteStream *out) {
		for (int16 i = 0; i < N; i++)
			_objects[i].save(vm, out);
	}
	void load(PrisonerEngine *vm, Common::ReadStream *in) {
		for (int16 i = 0; i < N; i++)
			_objects[i].load(vm, in);
	}
protected:
	T _objects[N];
};

} // End of namespace Prisoner

#endif /* PRISONER_OBJECTSTORAGE_H */
