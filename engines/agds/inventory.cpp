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

#include "agds/inventory.h"
#include "agds/object.h"
#include "common/debug.h"
#include "common/textconsole.h"

namespace AGDS {
Inventory::Inventory(): _entries(kMaxSize) { }
Inventory::~Inventory() { }

int Inventory::free() const {
	int free = 0;
	for(uint i = 0; i < _entries.size(); ++i)
		if (!_entries[i])
			++free;
	return free;
}

int Inventory::add(ObjectPtr object) {
	for(uint i = 0; i < _entries.size(); ++i) {
		if (!_entries[i]) {
			_entries[i] = object;
			return i;
		}
	}
	error("inventory overflow");
}

int Inventory::find(const Common::String &name) const {
	for(uint i = 0; i < _entries.size(); ++i)
		if (_entries[i] && _entries[i]->getName() == name)
			return i;
	return -1;
}

void Inventory::clear() {
	for(uint i = 0; i < _entries.size(); ++i) {
		_entries[i].reset();
	}
}


}
