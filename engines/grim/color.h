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

#ifndef GRIM_COLOR_H
#define GRIM_COLOR_H

#include "common/endian.h"

namespace Grim {

class Color {
public:
	byte _vals[3];

	Color();
	Color(byte r, byte g, byte b);
	Color(const Color &c);
	Color(uint32 c);

	byte &getRed() { return _vals[0]; }
	byte getRed() const { return _vals[0]; }
	byte &getGreen() { return _vals[1]; }
	byte getGreen() const { return _vals[1]; }
	byte &getBlue() { return _vals[2]; }
	byte getBlue() const { return _vals[2]; }

	uint32 toEncodedValue();

	Color &operator =(const Color &c);
	Color &operator =(const Color *c);
};


} // end of namespace Grim

#endif
