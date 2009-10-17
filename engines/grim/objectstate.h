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

#ifndef GRIM_OSTATE_H
#define GRIM_OSTATE_H

#include "engines/grim/bitmap.h"

namespace Grim {

class SaveGame;

class ObjectState {
public:
	enum Position {
		OBJSTATE_BACKGROUND = 0,
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
	};

	ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible);
	~ObjectState();

	void saveState(SaveGame *savedState);

	int setupID() const { return _setupID; }
	Position pos() const { return _pos; }
	void setPos(Position position) { _pos = position; }

	const char *bitmapFilename() const {
		return _bitmap->filename();
	}

	void setNumber(int val) {
		if (val) {
			assert(_bitmap);
			_bitmap->setNumber(val);
			if (_zbitmap)
				_zbitmap->setNumber(val);
		}

		_visibility = val != 0;
	}
	void draw() {
		if (!_visibility)
			return;
		assert(_bitmap);
		_bitmap->draw();
		if (_zbitmap)
			_zbitmap->draw();
	}

private:

	bool _visibility;
	int _setupID;
	Position _pos;
	Bitmap *_bitmap, *_zbitmap;
};

} // end of namespace Grim

#endif
