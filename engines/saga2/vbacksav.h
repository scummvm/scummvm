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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VBACKSAV_H
#define SAGA2_VBACKSAV_H

#include "saga2/vdraw.h"

namespace Saga2 {

//  Manages a backsave buffer for menus, windows, and other
//  "things that appear in fron of other things"

class gBackSave : private gPort {
	Rect16          savedRegion;        // extent of saved region
	gPixelMap       savedPixels;        // buffer of saved pixels
	bool            saved;              // true = saved.

public:
	gBackSave(const Rect16 &extent);
	virtual ~gBackSave();

	void save(gDisplayPort &port);
	void restore(gDisplayPort &port);
	void setPos(Point16 pos) {
		savedRegion.x = pos.x;
		savedRegion.y = pos.y;
	}
	bool valid() {
		return savedPixels.data != NULL;
	}
};

} // end of namespace Saga2

#endif
