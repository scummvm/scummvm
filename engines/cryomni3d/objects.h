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

#ifndef CRYOMNI3D_OBJECTS_H
#define CRYOMNI3D_OBJECTS_H

#include "common/array.h"
#include "common/func.h"
#include "common/str.h"

#include "cryomni3d/sprites.h"

namespace CryOmni3D {

class Object {
public:
	typedef Common::Functor0<void> *ViewCallback;

	Object() : _valid(false), _idCA(uint(-1)), _idCl(uint(-1)), _idSA(uint(-1)), _idSl(uint(-1)),
		_idOBJ(uint(-1)),
		_viewCallback(nullptr) {}

	Object(const Sprites &sprites, uint id_CA, uint id_OBJ) : _idCA(id_CA),
		_idCl(sprites.calculateSpriteId(id_CA, 1)), _idSA(sprites.calculateSpriteId(id_CA, 2)),
		_idSl(sprites.calculateSpriteId(id_CA, 3)),
		_valid(true), _idOBJ(id_OBJ), _viewCallback(nullptr) {}

	~Object() { delete _viewCallback; }

	uint valid() const { return _valid; }
	uint idCA() const { return _idCA; }
	uint idCl() const { return _idCl; }
	uint idSA() const { return _idSA; }
	uint idSl() const { return _idSl; }
	uint idOBJ() const { return _idOBJ; }
	ViewCallback viewCallback() const { return _viewCallback; }
	// Takes ownership of the pointer
	void setViewCallback(ViewCallback callback) { _viewCallback = callback; }

	void rename(uint newIdOBJ) { _idOBJ = newIdOBJ; }

private:
	uint _idOBJ;
	uint _idCA;
	uint _idCl;
	uint _idSA;
	uint _idSl;
	bool _valid;
	ViewCallback _viewCallback;
};

class Objects : public Common::Array<Object> {
public:
	Object *findObjectByNameID(uint nameID);
	Object *findObjectByIconID(uint iconID);
private:
};

class Inventory : public Common::Array<Object *> {
public:
	Inventory() : _selectedObject(nullptr), _changeCallback(nullptr) { }
	~Inventory() { delete _changeCallback; }
	void init(uint count, Common::Functor1<uint, void> *changeCallback) { _changeCallback = changeCallback; resize(count); }

	void clear();
	void add(Object *);
	void remove(uint position);
	void removeByNameID(uint nameID);
	void removeByIconID(uint iconID);
	bool inInventoryByNameID(uint nameID) const;
	bool inInventoryByIconID(uint iconID) const;

	Object *selectedObject() const { return _selectedObject; }
	void setSelectedObject(Object *obj) { _selectedObject = obj; }
	void deselectObject() { _selectedObject = nullptr; }

private:
	Object *_selectedObject;
	Common::Functor1<uint, void> *_changeCallback;
};

} // End of namespace CryOmni3D

#endif

