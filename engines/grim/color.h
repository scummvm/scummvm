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

#ifndef GRIM_COLOR_H
#define GRIM_COLOR_H

#include "common/scummsys.h"
#include "engines/grim/object.h"

namespace Grim {

class Color : public Object {
public:
	byte _vals[3];

	Color() : Object() {}
	Color(byte r, byte g, byte b) :
			Object() {
		_vals[0] = r; _vals[1] = g; _vals[2] = b;
	}
	Color(const Color& c) :
			Object() {
		_vals[0] = c._vals[0]; _vals[1] = c._vals[1]; _vals[2] = c._vals[2];
	}
	byte &getRed() { return _vals[0]; }
	byte getRed() const { return _vals[0]; }
	byte &getGreen() { return _vals[1]; }
	byte getGreen() const { return _vals[1]; }
	byte &getBlue() { return _vals[2]; }
	byte getBlue() const { return _vals[2]; }

	Color& operator =(const Color &c) {
		_vals[0] = c._vals[0]; _vals[1] = c._vals[1]; _vals[2] = c._vals[2];
		return *this;
	}

	Color& operator =(Color *c) {
		_vals[0] = c->_vals[0]; _vals[1] = c->_vals[1]; _vals[2] = c->_vals[2];
		return *this;
	}
};

} // end of namespace Grim

#endif
