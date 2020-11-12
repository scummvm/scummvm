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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGDS_INVENTORY_H
#define AGDS_INVENTORY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/ptr.h"
#include "common/rect.h"

namespace AGDS {
class Object;
typedef Common::SharedPtr<Object> ObjectPtr;

class Inventory {
	typedef Common::Array<ObjectPtr> EntriesType;
	EntriesType _entries;
	bool _enabled;
	bool _visible;

public:
	static const int kMaxSize = 35;

	Inventory();
	~Inventory();

	bool enabled() const {
		return _enabled;
	}
	void enable(bool enabled) {
		_enabled = enabled;
	}

	bool visible() const {
		return _visible;
	}

	void visible(bool visible) {
		_visible = visible;
	}

	int add(const ObjectPtr & object);

	ObjectPtr get(int index) const;
	int find(const Common::String &name) const;
	ObjectPtr find(const Common::Point pos) const;

	int free() const;
	void clear();

	int maxSize() const {
		return _entries.size();
	}

};


} // End of namespace AGDS

#endif /* AGDS_INVENTORY_H */
