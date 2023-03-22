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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRAPHICS_FONTS_DOSFONT_H
#define GRAPHICS_FONTS_DOSFONT_H

#include <graphics/font.h>

namespace Graphics {

// For now just a holder for static data. May become a child of Font if needed.
class DosFont {
public:
// 8x8 font patterns

// this is basically the standard PC BIOS font, taken from Dos-Box, with a few modifications
static const uint8 fontData_PCBIOS[256 * 8];
static const uint8 fontData_ExtendedRussian[128 * 8];
};

}

#endif
