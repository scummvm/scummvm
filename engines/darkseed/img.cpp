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

#include "img.h"
#include "common/file.h"

namespace Darkseed {

bool Img::load(const Common::String &filename) {
	Common::File file;
	if(!file.open(filename)) {
		return false;
	}
	int fIdx = 0;
	uint16 size = file.readUint16LE();
	fIdx += 2;
	uint16 idx = 0;
	pixels.resize(size+1);

	while (idx <= size) {
		uint8 byte = file.readByte();
		assert(!file.err());
		fIdx++;
		if (byte & 0x80) {
			uint8 count = byte & 0x7f;
			count++;
			byte = file.readByte();
			fIdx++;
			for (int i = 0; i < count; i++) {
				pixels[idx + i] = byte;
			}
			idx += count;
		} else {
			uint8 count = byte + 1;
			for (int i = 0; i < count; i++) {
				pixels[idx + i] = file.readByte();
				fIdx++;
			}
			idx += count;
		}
	}
	return true;
}

} // namespace Darkseed