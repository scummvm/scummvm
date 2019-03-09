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

	Object() : _valid(false), _idCA(-1), _idCl(-1), _idSA(-1), _idSl(-1), _idOBJ(-1),
		_viewCallback(nullptr) {}

	Object(const Sprites &sprites, unsigned int idCA, unsigned int idOBJ) : _idCA(idCA),
		_idCl(sprites.calculateSpriteId(idCA, 1)), _idSA(sprites.calculateSpriteId(idCA, 2)),
		_idSl(sprites.calculateSpriteId(idCA, 3)),
		_valid(true), _idOBJ(idOBJ), _viewCallback(nullptr) {}

	~Object() { delete _viewCallback; }

	unsigned int valid() const { return _valid; }
	unsigned int idCA() const { return _idCA; }
	unsigned int idCl() const { return _idCl; }
	unsigned int idSA() const { return _idSA; }
	unsigned int idSl() const { return _idSl; }
	unsigned int idOBJ() const { return _idOBJ; }
	ViewCallback viewCallback() const { return _viewCallback; }
	// Takes ownership of the pointer
	void setViewCallback(ViewCallback callback) { _viewCallback = callback; }

	void rename(unsigned int newIdOBJ) { _idOBJ = newIdOBJ; }

private:
	unsigned int _idOBJ;
	unsigned int _idCA;
	unsigned int _idCl;
	unsigned int _idSA;
	unsigned int _idSl;
	bool _valid;
	ViewCallback _viewCallback;
};

class Objects : public Common::Array<Object> {
public:
	Object *findObjectByNameID(unsigned int nameID);
	Object *findObjectByIconID(unsigned int iconID);
private:
};

class Inventory : public Common::Array<Object *> {
public:
	Inventory() : _selectedObject(nullptr), _changeCallback(nullptr) { }
	~Inventory() { delete _changeCallback; }
	void init(unsigned int count, Common::Functor1<unsigned int, void> *changeCallback) { _changeCallback = changeCallback; resize(count); }

	void clear();
	void add(Object *);
	void remove(unsigned int position);
	void removeByCursorId(unsigned int cursorId);
	void removeByNameId(unsigned int nameId);
	bool inInventoryByCursorId(unsigned int cursorId) const;
	bool inInventoryByNameId(unsigned int nameId) const;

	Object *selectedObject() const { return _selectedObject; }
	void setSelectedObject(Object *obj) { _selectedObject = obj; }
	void deselectObject() { _selectedObject = nullptr; }

private:
	Object *_selectedObject;
	Common::Functor1<unsigned int, void> *_changeCallback;
};

} // End of namespace CryOmni3D

#endif

