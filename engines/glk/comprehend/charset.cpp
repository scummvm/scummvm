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

#include "glk/comprehend/charset.h"
#include "common/file.h"
#include "common/md5.h"
#include "graphics/surface.h"

namespace Glk {
namespace Comprehend {

void FixedFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(dst->format.bytesPerPixel == 4);
	assert(chr >= 32 && chr < 128);

	for (int yp = 0; yp < 8; ++yp) {
		if ((y + yp) < 0 || (y + yp) >= dst->h)
			continue;

		uint32 *lineP = (uint32 *)dst->getBasePtr(x, y + yp);
		byte bits = _data[chr - 32][yp];

		for (int xp = x; xp < (x + 8); ++xp, ++lineP, bits >>= 1) {
			if ((xp >= 0) && (xp < dst->w) && (bits & 1))
				*lineP = color;
		}
	}
}

/*-------------------------------------------------------*/

CharSet::CharSet() : FixedFont() {
	Common::File f;
	if (!f.open("charset.gda"))
		error("Could not open char set");

	uint version = f.readUint16LE();
	if (version != 0x1100)
		error("Unknown char set version");

	f.seek(4);
	for (int idx = 0; idx < 128 - 32; ++idx)
		f.read(&_data[idx][0], 8);

	f.close();
}

/*-------------------------------------------------------*/

TalismanFont::TalismanFont() : FixedFont() {
	// Extra strings are (annoyingly) stored in the game binary
	Common::File f;
	if (!f.open("novel.exe"))
		error("novel.exe is a required file");

	Common::String md5 = Common::computeStreamMD5AsString(f, 1024);

	if (md5 == "0e7f002971acdb055f439020363512ce" || md5 == "2e18c88ce352ebea3e14177703a0485f") {
		for (int idx = 0; idx < 128 - 32; ++idx)
			f.read(&_data[idx][0], 8);
	} else {
		error("Unrecognised novel.exe encountered");
	}

	f.close();
}

} // namespace Comprehend
} // namespace Glk
