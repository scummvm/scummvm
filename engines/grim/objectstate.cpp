/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/objectstate.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"
#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"

namespace Grim {

ObjectState::ObjectState(int setup, ObjectState::Position position, const char *bitmap, const char *zbitmap, bool transparency) :
		Object(), _setupID(setup), _pos(position), _visibility(false) {

	_bitmap = g_resourceloader->loadBitmap(bitmap);
	g_grim->registerBitmap(_bitmap);
	if (zbitmap) {
		_zbitmap = g_resourceloader->loadBitmap(zbitmap);
		g_grim->registerBitmap(_zbitmap);
	} else
		_zbitmap = NULL;
}

ObjectState::ObjectState() :
		Object(), _bitmap(NULL), _zbitmap(NULL) {

}

ObjectState::~ObjectState() {
	g_grim->killObjectState(this);

	delete _bitmap;
	delete _zbitmap;
}

const Common::String &ObjectState::getBitmapFilename() const {
	return _bitmap->getFilename();
}

void ObjectState::setNumber(int val) {
	if (val) {
		assert(_bitmap);
		_bitmap->setNumber(val);
		if (_zbitmap)
			_zbitmap->setNumber(val);
	}

	_visibility = val != 0;
}
void ObjectState::draw() {
	if (!_visibility)
		return;
	assert(_bitmap);
	_bitmap->draw();
	if (_zbitmap)
		_zbitmap->draw();
}

void ObjectState::saveState(SaveGame *savedState) const {
	savedState->writeLESint32(_visibility);
	savedState->writeLEUint32(_setupID);
	savedState->writeLEUint32(_pos);

	//_bitmap
	if (_bitmap) {
		savedState->writeLESint32(_bitmap->getId());
	} else {
		savedState->writeLEUint32(0);
	}

	//_zbitmap
	if (_zbitmap) {
		savedState->writeLESint32(_zbitmap->getId());
	} else {
		savedState->writeLEUint32(0);
	}
}

bool ObjectState::restoreState(SaveGame *savedState) {
	_visibility = savedState->readLEUint32();
	_setupID    = savedState->readLEUint32();
	_pos        = (Position) savedState->readLEUint32();

	_bitmap = g_grim->getBitmap(savedState->readLESint32());
	_zbitmap = g_grim->getBitmap(savedState->readLESint32());

	return true;
}

} // end of namespace Grim
