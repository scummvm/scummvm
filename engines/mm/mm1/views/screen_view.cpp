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

#include "common/file.h"
#include "mm/mm1/views/screen_view.h"

namespace MM {
namespace MM1 {
namespace Views {

#define IMAGE_SIZE 16000

void ScreenView::loadScreen(int screenNum) {
	byte bytes[IMAGE_SIZE];
	byte v;
	int len;
	const byte *srcP;
	byte *destP = &bytes[0];
	int index = 0;

	_screenNum = screenNum;

	Common::File f;
	if (!f.open(Common::String::format("screen%d", screenNum)))
		error("Could not open screen%d", screenNum);
	int size = f.readUint16LE();

	// Decompress the image bytes
	while (size > 0) {
		v = f.readByte();
		if (v != 0x7B) {
			len = 1;
			--size;
		} else {
			len = f.readByte() + 1;
			v = f.readByte();
			size -= 3;
		}

		for (; len > 0; --len) {
			destP[index] = v;

			index += 80;
			if (index >= IMAGE_SIZE) {
				index = 0;
				++destP;
			}
		}
	}

	// Create surface from splitting up the nibbles
	_surface.create(320, 200);
	srcP = &bytes[0];
	destP = (byte *)_surface.getPixels();

	for (size_t i = 0; i < IMAGE_SIZE; ++i, ++srcP) {
		*destP++ = *srcP & 0xf;
		*destP++ = *srcP >> 4;
	}
}

void ScreenView::draw() {
	if (_surface.empty())
		loadScreen(0);

	getScreen()->blitFrom(_surface);
}


} // namespace Views
} // namespace MM1
} // namespace MM
