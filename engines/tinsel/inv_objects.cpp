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

#include "tinsel/inv_objects.h"
#include "common/memstream.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

InventoryObject::InventoryObject(Common::MemoryReadStreamEndian &stream) {
	_id = stream.readUint32();
	_hIconFilm = stream.readUint32();
	_hScript = stream.readUint32();
	if (TinselVersion == 0) {
		_attribute = 0;
	} else {
		_attribute = stream.readUint32();
	}
}

template<typename T>
class InventoryObjectsImpl : public InventoryObjects {
public:
	InventoryObjectsImpl(const byte *objects, int numObjects) {
		bool bigEndian = (TinselV1Mac || TinselV1Saturn);
		auto stream = new Common::MemoryReadStreamEndian(objects, T::SIZE() * numObjects, bigEndian, DisposeAfterUse::NO);
		for (int i = 0; i < numObjects; i++) {
			_objects.push_back(T(*stream));
		}
		assert((!stream->eos()) && stream->pos() == stream->size());
		delete stream;
	}
	~InventoryObjectsImpl(){};
	const InventoryObject *GetInvObject(int id) override {
		auto index = GetObjectIndexIfExists(id);
		if (index != -1) {
			return _objects.data() + index;
		}
		return nullptr;
	}
	const InventoryObjectT3 *GetInvObjectT3(int id) override;

	const InventoryObject *GetObjectByIndex(int index) const override {
		assert(index >= 0 && index < numObjects());
		return _objects.data() + index;
	}
	void SetObjectFilm(int id, SCNHANDLE hFilm) override {
		int index = GetObjectIndexIfExists(id);
		_objects[index].setIconFilm(hFilm);
	}
	int GetObjectIndexIfExists(int id) const override {
		for (uint i = 0; i < _objects.size(); i++) {
			if (_objects[i].getId() == id) {
				return i;
			}
		}
		return -1;
	};
	int numObjects() const override {
		return _objects.size();
	}
private:
	Common::Array<T> _objects;
};

template<>
const InventoryObjectT3 *InventoryObjectsImpl<InventoryObjectT3>::GetInvObjectT3(int id) {
	auto index = GetObjectIndexIfExists(id);
	if (index != -1) {
		return _objects.data() + index;
	}
	return nullptr;
}

template<>
const InventoryObjectT3 *InventoryObjectsImpl<InventoryObject>::GetInvObjectT3(int id) {
	error("Can't query Noir inventory objects from non Noir-game");
}

InventoryObjects *InstantiateInventoryObjects(const byte *invObjects, int numObjects) {
	switch (TinselVersion) {
	case 3:
		return new InventoryObjectsImpl<InventoryObjectT3>(invObjects, numObjects);
	default:
		return new InventoryObjectsImpl<InventoryObject>(invObjects, numObjects);
	}
}

} // End of namespace Tinsel
