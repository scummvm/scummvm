// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef OSTATE_H
#define OSTATE_H

#include "vector3d.h"
#include "resource.h"
#include "bitmap.h"

#include <string>
#include <list>

class ObjectState {
public:
	enum Position {
		OBJSTATE_BACKGROUND = 0,
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
// TODO: Find out what ObjectState 6 is supposed to be
//		OBJSTATE_UNKNOWN = 6
	};

	ObjectState(int setupID, ObjectState::Position pos, const char *bitmap, const char *zbitmap, bool visible);
	~ObjectState();

	int setupID() const { return _setupID; }
	Position pos() const { return _pos; }
	void setPos(Position pos) { _pos = pos; }

	const char *bitmapFilename() const {
		return _bitmap->filename();
	}

	void setNumber(int val) {
		_bitmap->setNumber(val);
		if (_zbitmap)
			_zbitmap->setNumber(val);
	}
	void draw() {
		_bitmap->draw();
		if (_zbitmap)
			_zbitmap->draw();
	}

private:

	int _setupID;
	Position _pos;
	ResPtr<Bitmap> _bitmap, _zbitmap;
};

#endif
