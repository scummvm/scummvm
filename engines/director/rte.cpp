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

#include "common/stream.h"
#include "common/substream.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/rte.h"

namespace Director {

RTE0::RTE0(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE1::RTE1(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE2::RTE2(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(2, kDebugText))
		stream.hexdump(stream.size());
	if (!stream.size())
		return;

	width = stream.readUint16BE();
	height = stream.readUint16BE();
	bpp = stream.readUint16BE();
	int checkMax = (1 << bpp) - 1;
	debugC(5, kDebugLoading, "RTE2: width: %d, height: %d, bpp: %d", width, height, bpp);
	alphaMap.resize(width*height, 0);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width;) {
			uint32 pos = stream.pos();
			byte check = stream.readByte();
			if (check == 0x1f) {
				debugC(9, kDebugLoading, "(%d, %d): %x -> color %d %d %d", x, y, pos, stream.readByte(), stream.readByte(), stream.readByte());
				continue;
			}
			if (check == 0 || check == checkMax) {
				byte count = stream.readByte();
				debugC(9, kDebugLoading, "(%d, %d): %x -> %02x %02x", x, y, pos, check, count);
				if (count == 0x00 && check == 0x00) {
					// end of line
					break;
				}
				for (byte j = 0; j < count; j++) {
					alphaMap[width*y + x] = check;
					x += 1;
					if (x >= width)
						break;
				}
			} else {
				debugC(9, kDebugLoading, "(%d, %d): %x -> %02x", x, y, pos, check);
				alphaMap[width*y + x] = check;
				x += 1;
			}
		}
	}
}

} // End of namespace Director
