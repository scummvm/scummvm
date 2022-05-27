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
}

int32 InventoryObject::getUnknown() const {
	error("Querying Noir-value from non-Noir game");
}

int32 InventoryObject::getTitle() const {
	error("Querying Noir-value from non-Noir game");
}

class InventoryObjectT1 : public InventoryObject {
public:
	InventoryObjectT1(Common::MemoryReadStreamEndian &stream) : InventoryObject(stream) {
		_attribute = stream.readUint32();
	}
	// Tinsel1+
	virtual int32 getAttribute() const {
		return _attribute;
	};
	static const int SIZE = InventoryObject::SIZE + 4;
private:
	int32 _attribute;
};

class InventoryObjectT3 : public InventoryObjectT1 {
public:
	InventoryObjectT3(Common::MemoryReadStreamEndian &stream) : InventoryObjectT1(stream) {
		_unknown = stream.readUint32();
		_title = stream.readUint32();
	}
	// Noir:
	virtual int32 getUnknown() const {
		return _unknown;
	}
	virtual int32 getTitle() const {
		return _title;
	}
	static const int SIZE = InventoryObjectT1::SIZE + 8;
private:
	int32 _unknown;
	int32 _title;
};

template<typename T>
class InventoryObjectsImpl : public InventoryObjects {
public:
	InventoryObjectsImpl(const byte *objects, int numObjects) {
		bool bigEndian = (TinselV1Mac || TinselV1Saturn);
		auto stream = new Common::MemoryReadStreamEndian(objects, T::SIZE * numObjects, bigEndian, DisposeAfterUse::NO);
		for (int i = 0; i < numObjects; i++) {
			_objects.push_back(T(*stream));
		}
		assert((!stream->eos()) && stream->pos() == stream->size());
		delete stream;
	}
	~InventoryObjectsImpl(){};
	const InventoryObject *GetInvObject(int id) {
		auto index = GetObjectIndexIfExists(id);
		if (index != -1) {
			return _objects.data() + index;
		}
		return nullptr;
	}
	const InventoryObject *GetObjectByIndex(int index) const {
		assert(index >= 0 && index < numObjects());
		return _objects.data() + index;
	}
	void SetObjectFilm(int id, SCNHANDLE hFilm) {
		int index = GetObjectIndexIfExists(id);
		_objects[index].setIconFilm(hFilm);
	}
	int GetObjectIndexIfExists(int id) const {
		for (uint i = 0; i < _objects.size(); i++) {
			if (_objects[i].getId() == id) {
				return i;
			}
		}
		return -1;
	};
	int numObjects() const {
		return _objects.size();
	}
private:
	Common::Array<T> _objects;
};

InventoryObjects *InstantiateInventoryObjects(const byte *invObjects, int numObjects) {
	switch (TinselVersion) {
	case 0:
		return new InventoryObjectsImpl<InventoryObject>(invObjects, numObjects);
	case 3:
		return new InventoryObjectsImpl<InventoryObjectT3>(invObjects, numObjects);
	default:
		return new InventoryObjectsImpl<InventoryObjectT1>(invObjects, numObjects);
	}
}

} // End of namespace Tinsel
