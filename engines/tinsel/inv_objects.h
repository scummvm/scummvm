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
	virtual ~InventoryObject() {}
	int32 getId() const { return _id; }
	SCNHANDLE getIconFilm() const { return _hIconFilm; };
	void setIconFilm(SCNHANDLE hIconFilm) { _hIconFilm = hIconFilm; }
	SCNHANDLE getScript() const { return _hScript; }
	// Tinsel1+
	bool hasAttribute(InvObjAttr attribute) const {
		return getAttribute() & (int32)attribute;
	}
	// Noir:
	virtual int32 getUnknown() const;
	virtual int32 getTitle() const;
	static const int SIZE = 12;
protected:
	// Tinsel 1+
	virtual int32 getAttribute() const {
		return 0;
	};
private:
	int32 _id;            // inventory objects id
	SCNHANDLE _hIconFilm; // inventory objects animation film
	SCNHANDLE _hScript;   // inventory objects event handling script
};

class InventoryObjects {
public:
	virtual ~InventoryObjects() {};
	virtual const InventoryObject *GetInvObject(int id) = 0;
	virtual const InventoryObject *GetObjectByIndex(int index) const = 0;
	virtual void SetObjectFilm(int id, SCNHANDLE hFilm) = 0;
	virtual int GetObjectIndexIfExists(int id) const = 0;
	virtual int numObjects() const = 0;
};

InventoryObjects *InstantiateInventoryObjects(const byte *invObjects, int numObjects);

} // End of namespace Tinsel

#endif // TINSEL_INV_OBJECT_H
