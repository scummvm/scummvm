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
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/objectstate.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"
#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"

namespace Grim {

ObjectState::ObjectState(int setup, ObjectState::Position position, const char *bitmap, const char *zbitmap, bool transparency) :
		Object(), _setupID(setup), _pos(position), _visibility(false) {
	_bitmap = g_resourceloader->getBitmap(bitmap);
	if (zbitmap) {
		_zbitmap = g_resourceloader->getBitmap(zbitmap);
	} else
		_zbitmap = NULL;
}

ObjectState::ObjectState() :
		Object(), _bitmap(NULL), _zbitmap(NULL) {

}

ObjectState::~ObjectState() {
	g_grim->killObjectState(this);
//	g_resourceloader->uncache(_bitmap->getFilename());
//	if (_zbitmap)
//		g_resourceloader->uncache(_zbitmap->getFilename());
}

void ObjectState::saveState(SaveGame *savedState) const {
	savedState->writeLESint32(_visibility);
	savedState->writeLEUint32(_setupID);
	savedState->writeLEUint32(_pos);

	//_bitmap
	if (_bitmap) {
		savedState->writeLEUint32(1);
		savedState->writeCharString(_bitmap->filename());
	} else {
		savedState->writeLEUint32(0);
	}

	//_zbitmap
	if (_zbitmap) {
		savedState->writeLEUint32(1);
		savedState->writeCharString(_zbitmap->filename());
	} else {
		savedState->writeLEUint32(0);
	}
}

bool ObjectState::restoreState(SaveGame *savedState) {
	_visibility = savedState->readLEUint32();
	_setupID    = savedState->readLEUint32();
	_pos        = (Position) savedState->readLEUint32();

	if (savedState->readLEUint32()) {
		const char *name = savedState->readCharString();
		_bitmap = g_resourceloader->getBitmap(name);
		delete[] name;
	} else {
		_bitmap = 0;
	}

	if (savedState->readLEUint32()) {
		const char *name = savedState->readCharString();
		_zbitmap = g_resourceloader->getBitmap(name);
		delete[] name;
	} else {
		_zbitmap = 0;
	}

	return true;
}

} // end of namespace Grim
