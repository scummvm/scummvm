/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/objectstate.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"

namespace Grim {

ObjectState::ObjectState(int setup, ObjectState::Position position, const char *bitmap, const char *zbitmap, bool transparency) :
		_setupID(setup), _pos(position), _visibility(false) {

	_bitmap = Bitmap::create(bitmap);
	if (zbitmap) {
		_zbitmap = Bitmap::create(zbitmap);
	} else
		_zbitmap = nullptr;
}

ObjectState::ObjectState() :
		_bitmap(nullptr), _zbitmap(nullptr) {

}

ObjectState::~ObjectState() {
	delete _bitmap;
	delete _zbitmap;
}

const Common::String &ObjectState::getBitmapFilename() const {
	return _bitmap->getFilename();
}

void ObjectState::setActiveImage(int val) {
	if (val) {
		assert(_bitmap);
		_bitmap->setActiveImage(val);
		if (_zbitmap && val <= _zbitmap->getNumImages()) {
			_zbitmap->setActiveImage(val);
		}
	}

	_visibility = val != 0;
}

void ObjectState::draw() {
	if (!_visibility)
		return;

	assert(_bitmap);
	_bitmap->draw();
	// The only OBJSTATE_UNDERLAY which has a zbuffer is the bonewagon in set bv. Original doesn't draw that zbuffer,
	// so we don't either, otherwise this bug will appear: https://github.com/residualvm/residualvm/issues/143
	if (_zbitmap && _pos != OBJSTATE_UNDERLAY)
		_zbitmap->draw();
}

void ObjectState::saveState(SaveGame *savedState) const {
	savedState->writeBool(_visibility);
	savedState->writeLESint32(_setupID);
	savedState->writeLESint32(_pos);

	//_bitmap
	if (_bitmap) {
		savedState->writeLESint32(_bitmap->getId());
	} else {
		savedState->writeLESint32(0);
	}

	//_zbitmap
	if (_zbitmap) {
		savedState->writeLESint32(_zbitmap->getId());
	} else {
		savedState->writeLESint32(0);
	}
}

bool ObjectState::restoreState(SaveGame *savedState) {
	_visibility = savedState->readBool();
	_setupID    = savedState->readLESint32();
	_pos        = (Position) savedState->readLESint32();

	_bitmap = Bitmap::getPool().getObject(savedState->readLESint32());
	_zbitmap = Bitmap::getPool().getObject(savedState->readLESint32());

	return true;
}

} // end of namespace Grim
