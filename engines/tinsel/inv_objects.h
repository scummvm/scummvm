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

#ifndef TINSEL_INV_OBJECT_H
#define TINSEL_INV_OBJECT_H

#include "common/memstream.h"
#include "tinsel/tinsel.h"
#include "tinsel/dw.h"

namespace Tinsel {

// attribute values - not a bit bit field to prevent portability problems
enum class InvObjAttr {
	IO_DROPCODE = 0x01,
	IO_ONLYINV1 = 0x02,
	IO_ONLYINV2 = 0x04,
	DEFINV1 = 0x08,
	DEFINV2 = 0x10,
	PERMACONV = 0x20,
	CONVENDITEM = 0x40,

	// Noir only
	V3ATTR_X80 = 0x80,
	V3ATTR_X200 = 0x200,
	V3ATTR_X400 = 0x400,
	NOTEBOOK_TITLE = 0x800, // is a notebook title
	V3ATTR_X1000 = 0x1000,
	V3ATTR_X2000 = 0x2000,
};

class InventoryObject {
public:
	InventoryObject(Common::MemoryReadStreamEndian &stream);
	int32 getId() const { return _id; }
	SCNHANDLE getIconFilm() const { return _hIconFilm; };
	void setIconFilm(SCNHANDLE hIconFilm) { _hIconFilm = hIconFilm; }
	SCNHANDLE getScript() const { return _hScript; }
	// Tinsel1+
	bool hasAttribute(InvObjAttr attribute) const {
		return getAttribute() & (int32)attribute;
	}

	// Data size consumed by constructor
	static int SIZE() {
		return (TinselVersion == 0 ? T0_SIZE : T1_SIZE);
	}
protected:
	static const int T0_SIZE = 3 * 4;
	static const int T1_SIZE = T0_SIZE + 4; // Versions above 0 have attributes
	// Tinsel 1+
	int32 getAttribute() const {
		return _attribute;
	};
private:
	int32 _id;            // inventory objects id
	SCNHANDLE _hIconFilm; // inventory objects animation film
	SCNHANDLE _hScript;   // inventory objects event handling script
	int32 _attribute = 0;
};

class InventoryObjectT3 : public InventoryObject {
public:
	InventoryObjectT3(Common::MemoryReadStreamEndian &stream) : InventoryObject(stream) {
		_unknown = stream.readUint32();
		_title = stream.readUint32();
	}
	// Noir:
	bool isNotebookTitle() const {
		return (getAttribute() & (int)InvObjAttr::NOTEBOOK_TITLE) != 0;
	}
	int32 getUnknown() const {
		return _unknown;
	}
	int32 getTitle() const {
		return _title;
	}
	// Data size consumed by constructor
	static int SIZE() {
		return InventoryObject::SIZE() + 8;
	}
private:
	int32 _unknown;
	int32 _title;
};

class InventoryObjects {
public:
	virtual ~InventoryObjects() {};
	virtual const InventoryObject *GetInvObject(int id) = 0;
	virtual const InventoryObjectT3 *GetInvObjectT3(int id) = 0;
	virtual const InventoryObject *GetObjectByIndex(int index) const = 0;
	virtual void SetObjectFilm(int id, SCNHANDLE hFilm) = 0;
	virtual int GetObjectIndexIfExists(int id) const = 0;
	virtual int numObjects() const = 0;
};

InventoryObjects *InstantiateInventoryObjects(const byte *invObjects, int numObjects);

} // End of namespace Tinsel

#endif // TINSEL_INV_OBJECT_H
