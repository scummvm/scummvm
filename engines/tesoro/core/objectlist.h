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

#ifndef TESORO_CORE_OBJECTLIST_H
#define TESORO_CORE_OBJECTLIST_H

#include "common/array.h"

namespace Tesoro {

class Object;

/** Any list of objects */
typedef Common::Array<Object *> ObjectList;

/** Takes the first entry for the object out of the list; false if it is not there. */
inline bool removeFromList(ObjectList &list, const Object *object) {
	for (uint i = 0; i < list.size(); ++i) {
		if (list[i] == object) {
			list.remove_at(i);
			return true;
		}
	}
	return false;
}

/** The main list: objects in priority order, each taking its turn in runUpdatePass. */
class MainList {
public:
	/** Inserts in front of the first entry of greater priority, so equal priorities keep their order. */
	void insertByPriority(Object *object, int32 priority) { insert(object, priority, false); }
	/** Inserts in front of the first entry of greater or equal priority. */
	void insertByPriorityFront(Object *object, int32 priority) { insert(object, priority, true); }

	bool remove(const Object *object) {
		for (uint i = 0; i < _entries.size(); ++i) {
			if (_entries[i].object == object) {
				_entries.remove_at(i);
				++_edits;
				return true;
			}
		}
		return false;
	}

	void clear() {
		_entries.clear();
		++_edits;
	}

	uint count() const { return _entries.size(); }
	Object *at(uint index) const { return _entries[index].object; }
	int32 priorityAt(uint index) const { return _entries[index].priority; }
	/** Increases with every insertion or removal, so a pass can tell its turn edited the list. */
	uint32 edits() const { return _edits; }

private:
	struct Entry {
		Object *object;
		int32 priority;
	};

	void insert(Object *object, int32 priority, bool beforeEqual) {
		uint i = 0;
		while (i < _entries.size() && !(beforeEqual ? priority <= _entries[i].priority : priority < _entries[i].priority))
			++i;

		Entry entry;
		entry.object = object;
		entry.priority = priority;
		_entries.insert_at(i, entry);
		++_edits;
	}

	Common::Array<Entry> _entries;
	uint32 _edits = 0;
};

} // End of namespace Tesoro

#endif
