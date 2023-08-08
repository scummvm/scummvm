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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Common::SeekableReadStream *DarkEngine::decryptFile(const Common::String filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

    uint32 d7 = 0;
    uint32 d6 = 0;
	byte *a6 = encryptedBuffer;
    byte *a5 = encryptedBuffer + size - 1;

	while (a6 <= a5) {
		uint64 d0 = (a6[0] << 24) | (a6[1] << 16) | (a6[2] << 8) | a6[3];
		d0 = d0 + d6;
		d0 = uint32(d0);
		d0 = ((d0 << 3) & 0xFFFFFFFF) | ((d0 >> 29) & 0xFFFFFFFF);
		d0 ^= 0x71049763;
		d0 -= d7;
		d0 = ((d0 << 16) & 0xFFFF0000) | ((d0 >> 16) & 0xFFFF);

        a6[0] = byte((d0 >> 24) & 0xFF);
		//debug("%c", a6[0]);
        a6[1] = byte((d0 >> 16) & 0xFF);
		//debug("%c", a6[1]);
        a6[2] = byte((d0 >> 8) & 0xFF);
		//debug("%c", a6[2]);
        a6[3] = byte(d0 & 0xFF);
		//debug("%c", a6[3]);

		d6 += 5;
		d6 = ((d6 >> 3) & 0xFFFFFFFF) | ((d6 << 29) & 0xFFFFFFFF);
		d6 ^= 0x04000000;
		d7 += 4;
		a6 += 4;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

void DarkEngine::loadAssetsAmigaFullGame() {
	Common::SeekableReadStream *stream = decryptFile("1.drk");
	_border = loadAndConvertNeoImage(stream, 0x1b762);
	load8bitBinary(stream, 0x2e96a, 16);
	loadPalettes(stream, 0x2e638);
}

void DarkEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
}

void DarkEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 33, 287, 130);
}

} // End of namespace Freescape