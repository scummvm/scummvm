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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_RECT_H
#define SWORD25_RECT_H

// Includes
#include "common/rect.h"
#include "sword25/kernel/common.h"
#include "sword25/math/vertex.h"

namespace Sword25 {

// Class definitions

/**
 * Rect class. Currently this encapsultes the ScummVM Rect class. Eventually all usage of this
 * class should be replaced with Common::Rect directly.
*/
class BS_Rect: public Common::Rect {
public:
	BS_Rect() : Common::Rect() {}
	BS_Rect(int16 w, int16 h) : Common::Rect(w, h) {}
	BS_Rect(int16 x1, int16 y1, int16 x2, int16 y2) : Common::Rect(x1, y1, x2, y2) {}

	void Move(int DeltaX, int DeltaY) { translate(DeltaX, DeltaY); }

	bool DoesIntersect(const BS_Rect &Rect) const { return intersects(Rect); }

	bool Intersect(const BS_Rect &Rect, BS_Rect &Result) const {
		Result = Rect;
		Result.clip(*this);
	}

	void Join(const BS_Rect &Rect, BS_Rect &Result) const {
		Result = Rect;
		Result.extend(*this);
	}

	int GetWidth() const { return width(); }

	int GetHeight() const { return height(); }

	int GetArea() const { return width() * height(); }

	bool operator==(const BS_Rect &rhs) const { return equals(rhs); }

	bool operator!= (const BS_Rect &rhs) const { return !equals(rhs); }

	bool IsValid() const { return isValidRect(); }

	bool IsPointInRect(const BS_Vertex &Vertex) const { return contains(Vertex.X, Vertex.Y); }

	bool IsPointInRect(int X, int Y) const { return contains(X, Y); }

	bool ContainsRect(const BS_Rect &OtherRect) const { return contains(OtherRect); }
};

} // End of namespace Sword25

#endif
