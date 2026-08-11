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

#include "agds/inventory.h"
#include "agds/agds.h"
#include "agds/object.h"
#include "agds/resourceManager.h"
#include "agds/systemVariable.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "graphics/managed_surface.h"

namespace AGDS {

Inventory::Inventory(AGDSEngine *engine) : _engine(engine), _entries(kMaxSize), _enabled(false), _visible(false) {}
Inventory::~Inventory() {}

void Inventory::visible(bool visible) {
	if (_visible == visible)
		return;

	_visible = visible;

	if (!visible) {
		debug("closing inventory...");
		Common::String inv_close = _engine->getSystemVariable("inv_close")->getString();
		if (!inv_close.empty())
			_engine->runObject(inv_close);
	} else {
		debug("opening inventory...");
		if (!_engine->currentInventoryObject())
			removeGaps();
		Common::String inv_open = _engine->getSystemVariable("inv_open")->getString();
		if (!inv_open.empty())
			_engine->runObject(inv_open);
	}
}

int Inventory::free() const {
	int free = 0;
	for (uint i = 0; i < _entries.size(); ++i)
		if (!_entries[i].hasObject)
			++free;
	return free;
}

bool Inventory::has(int index) const {
	if (index >= 0 && index < kMaxSize) {
		auto &entry = _entries[index];
		return entry.hasObject;
	}
	return {};
}

ObjectPtr Inventory::get(int index) {
	if (index >= 0 && index < kMaxSize) {
		auto &entry = _entries[index];
		if (entry.hasObject && !entry.object) {
			entry.object = _engine->runObject(entry.name);
			entry.object->persistent(false);
		}
		return entry.object;
	}
	return {};
}

int Inventory::add(const Common::String &name) {
	int idx = find(name);
	if (idx >= 0) {
		warning("Double adding object %s, skipping...", name.c_str());
		return idx;
	}
	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (!entry.hasObject) {
			entry.name = name;
			entry.object.reset();
			entry.hasObject = true;
			return i;
		}
	}
	return idx;
}

int Inventory::add(const ObjectPtr &object) {
	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (entry.hasObject && entry.name == object->getName()) {
			warning("Double adding object [pointer] %s, skipping...", object->getName().c_str());
			return i;
		}
	}
	object->persistent(false);
	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (!entry.hasObject) {
			entry.name = object->getName();
			entry.object = object;
			entry.hasObject = true;
			return i;
		}
	}
	error("inventory overflow");
}

bool Inventory::remove(const Common::String &name) {
	bool removed = false;
	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (entry.hasObject && entry.name == name) {
			entry.reset();
			removed = true;
		}
	}
	return removed;
}

void Inventory::removeGaps() {
	auto n = _entries.size();
	for (uint src = 0, dst = 0; src < n; ++src) {
		auto &entry = _entries[src];
		if (entry.hasObject) {
			if (dst != src) {
				debug("moving inventory object %u -> %u", src, dst);
				_entries[dst++] = _entries[src];
				_entries[src].reset();
			} else
				++dst;
		}
	}
}

int Inventory::find(const Common::String &name) const {
	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (entry.hasObject && entry.name == name)
			return i;
	}
	return -1;
}

ObjectPtr Inventory::find(const Common::Point pos) const {
	if (!_enabled)
		return {};

	for (uint i = 0; i < _entries.size(); ++i) {
		auto &entry = _entries[i];
		if (!entry.object)
			continue;

		auto &object = entry.object;
		auto picture = object->getPicture();
		if (picture) {
			auto rect = picture->getBounds();
			rect.moveTo(object->getPosition());
			if (rect.contains(pos))
				return object;
		}
	}
	return {};
}

void Inventory::clear() {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i].reset();
	}
}

void Inventory::load(Common::ReadStream &stream) {
	clear();
	for (int i = 0; i < kMaxSize; ++i) {
		Common::String name = readString(stream);
		int refcount = stream.readUint32LE();
		int objectPtr = stream.readUint32LE();
		if (!name.empty() && refcount) {
			debug("load inventory object: %s %d %d", name.c_str(), refcount, objectPtr);
			auto &entry = _entries[i];
			entry.name = name;
			entry.hasObject = true;
		}
	}
}

void Inventory::save(Common::WriteStream &stream) const {
	for (const auto &entry : _entries) {
		writeString(stream, entry.name);
		stream.writeUint32LE(entry.hasObject ? 1 : 0);
		stream.writeSint32LE(entry.hasObject ? -1 : 0);
	}
}

} // namespace AGDS
