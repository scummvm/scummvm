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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/font.cpp $
 * $Id: font.cpp 2 2009-09-12 20:13:40Z clone2727 $
 *
 */

#include "startrek/font.h"

namespace StarTrek {

static const byte CHARACTER_COUNT = 0x80;
static const byte CHARACTER_SIZE = 0x40;

Font::Font(StarTrekEngine *vm) : _vm(vm) {
	SharedPtr<Common::SeekableReadStream> fontStream = _vm->loadFile("FONT.FNT");

	_characters = new Character[CHARACTER_COUNT];

	for (byte i = 0; i < CHARACTER_COUNT; i++)
		fontStream->read(_characters[i].data, CHARACTER_SIZE);

#if 0
	// Code to dump the font
	printf ("DUMPING FONT");
	for (byte i = 0; i < CHARACTER_COUNT; i++) {
		printf ("\n\nCHARACTER %02x (%d):\n", i, i);
		for (byte j = 0; j < CHARACTER_SIZE; j++) {
			if (!(j % 8))
				printf ("\n");
			if (_characters[i].data[j] == 0x7d)
				printf ("1 ");
			else if (_characters[i].data[j] == 0x78)
				printf ("0 ");
			else if (_characters[i].data[j] == 0)
				printf ("  ");
			else
				printf ("? ");
		}
	}
	printf("\n\n");
#endif
}

Font::~Font() {
	delete[] _characters;
}

byte *Font::getCharData(int i) {
	return _characters[i].data;
}

}
