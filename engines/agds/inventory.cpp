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
#include "agds/resourceManager.h"
#include "agds/agds.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/transparent_surface.h"

namespace AGDS {

Inventory::Inventory(AGDSEngine* engine): _engine(engine), _entries(kMaxSize), _enabled(false), _visible(false) {}
Inventory::~Inventory() {}

int Inventory::free() const {
	int free = 0;
	for (uint i = 0; i < _entries.size(); ++i)
		if (!_entries[i])
			++free;
	return free;
}

ObjectPtr Inventory::get(int index) const {
	return index >= 0 && index < kMaxSize? _entries[index]: ObjectPtr();
}

int Inventory::add(const ObjectPtr & object) {
	object->persistent(false);
	for (uint i = 0; i < _entries.size(); ++i) {
		if (!_entries[i]) {
			_entries[i] = object;
			return i;
		}
	}
	error("inventory overflow");
}

bool Inventory::remove(const Common::String &name) {
	bool removed = false;
	for (uint i = 0; i < _entries.size(); ++i) {
		auto & object = _entries[i];
		if (object && object->getName() == name) {
			object.reset();
			removed = true;
		}
	}
	return removed;
}

int Inventory::find(const Common::String &name) const {
	for (uint i = 0; i < _entries.size(); ++i)
		if (_entries[i] && _entries[i]->getName() == name)
			return i;
	return -1;
}

ObjectPtr Inventory::find(const Common::Point pos) const {
	for (uint i = 0; i < _entries.size(); ++i) {
		auto & object = _entries[i];
		if (!object)
			continue;

		auto picture = object->getPicture();
		if (picture) {
			auto rect = picture->getRect();
			rect.moveTo(object->getPosition());
			if (rect.contains(pos))
				return object;
		}
	}
	return ObjectPtr();
}

void Inventory::clear() {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i].reset();
	}
}

void Inventory::load(Common::ReadStream* stream) {
	clear();
	int n = kMaxSize;
	while(n--) {
		Common::String name = readString(stream);
		int refcount = stream->readUint32LE();
		int objectPtr = stream->readUint32LE();
		if (!name.empty() && refcount) {
			debug("inventory: %s %d %d", name.c_str(), refcount, objectPtr);
			add(_engine->runObject(name));
		}
	}
}

void Inventory::save(Common::WriteStream* stream) const {
	for(auto & entry : _entries) {
		if (entry) {
			writeString(stream, entry->getName());
			stream->writeUint32LE(1);
			stream->writeSint32LE(-1);
		} else {
			writeString(stream, Common::String());
			stream->writeUint32LE(0);
			stream->writeSint32LE(0);
		}
	}
}

} // namespace AGDS
