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

#ifndef SLUDGE_COLOURS_H
#define SLUDGE_COLOURS_H

namespace Sludge {

// Simple colour conversion routines to deal with 16-bit graphics
//unsigned short int makeColour (byte r, byte g, byte b);

inline unsigned short redValue(unsigned short c) {
	return (c >> 11) << 3;
}
inline unsigned short greenValue(unsigned short c) {
	return ((c >> 5) & 63) << 2;
}
inline unsigned short blueValue(unsigned short c) {
	return (c & 31) << 3;
}

inline unsigned short makeGrey(unsigned short int r) {
	return ((r >> 3) << 11) | ((r >> 2) << 5) | (r >> 3);
}

inline unsigned short makeColour(unsigned short int r, unsigned short int g,
		unsigned short int b) {
	return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

} // End of namespace Sludge

#endif
