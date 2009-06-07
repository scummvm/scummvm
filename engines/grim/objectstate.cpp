/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

namespace Grim {

ObjectState::ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible) :
		_setupID(setupID), _pos(pos) {
	_bitmap = g_resourceloader->loadBitmap(bitmap);
	if (zbitmap)
		_zbitmap = g_resourceloader->loadBitmap(zbitmap);

	int initialImage = 0;
	if (visible)
		initialImage = 1;
//	_bitmap->setNumber(initialImage);
//	if (_zbitmap)
//		_zbitmap->setNumber(initialImage);
}

ObjectState::~ObjectState() {
	g_resourceloader->uncache(_bitmap->getFilename());
	if (_zbitmap)
		g_resourceloader->uncache(_zbitmap->getFilename());
}

} // end of namespace Grim
