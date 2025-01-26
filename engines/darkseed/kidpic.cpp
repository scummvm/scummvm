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

#include "common/debug.h"
#include "common/file.h"
#include "darkseed/darkseed.h"
#include "darkseed/kidpic.h"

namespace Darkseed {

static constexpr int BYTES_PER_LINE = 320;
static constexpr int BYTES_PER_PLANE = 80;

static constexpr int KID_WIDTH = 640;
static constexpr int KID_HEIGHT = 350;

KidPic::KidPic() {
	_pixels.resize(KID_WIDTH * KID_HEIGHT);
	Common::File file;
	if (!file.open("kid.pic")) {
		error("Failed to open kid.pic");
	}

	file.seek(0x10);
	Pal pal;
	pal.loadFromStream(file, false);
	pal.swapEntries(14, 4); // not sure why we need to swap these palette entries. All the other entries line up correctly.
	pal.installPalette();
	unpackRLE(file);
}

bool KidPic::unpackRLE(Common::SeekableReadStream &readStream) {
	uint idx = 0;
	uint unpackedSize = _pixels.size() / 2;

	readStream.seek(0x80);

	while (idx < unpackedSize && !readStream.eos()) {
		uint8 byte = readStream.readByte();
		assert(!readStream.err());

		uint repeat = 1;
		if ((byte & 192) == 192) {
			repeat = (byte & 63);
			byte = readStream.readByte();
			assert(!readStream.err());
		}

		for (uint j = 0; j < repeat; j++) {
			unpackByte(byte);
		}
		idx += repeat;
	}

	return true;
}

// image is stored as 4 x 1-bit planes per line.
void KidPic::unpackByte(uint8 byte) {
	int planeOffset = _lineByteIdx / BYTES_PER_PLANE;
	int x = _lineByteIdx % BYTES_PER_PLANE;
	for (int i = 0; i < 8; i++) {
		if (byte & 1 << (7 - i)) {
			_pixels[_lineNum * KID_WIDTH + (x * 8) + i] |= 1 << planeOffset;
		}
	}
	_lineByteIdx++;
	if (_lineByteIdx == BYTES_PER_LINE) {
		_lineByteIdx = 0;
		_lineNum++;
	}
}

void KidPic::draw() {
	g_engine->_screen->copyRectToSurface(_pixels.data(), KID_WIDTH, 0, 0, KID_WIDTH, KID_HEIGHT);
}

} // namespace Darkseed
