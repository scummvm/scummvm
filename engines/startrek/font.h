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
 */

#ifndef STARTREK_FONT_H
#define STARTREK_FONT_H

#include "startrek/startrek.h"

namespace StarTrek {

class StarTrekEngine;

class Font {
public:
	Font(StarTrekEngine *vm);
	~Font();

	byte *getCharData(int i);
	bool isDisplayableCharacter(char c);

private:
	StarTrekEngine *_vm;

	struct Character {
		byte data[0x40];
	} *_characters;

	static const byte _fontProperties[256];
};

} // End of namespace StarTrek

#endif
