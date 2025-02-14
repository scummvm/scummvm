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
#include "common/stream.h"
#include "common/substream.h"

#include "director/director.h"
#include "director/cast.h"
#include "graphics/pixelformat.h"
#include "director/rte.h"

namespace Director {

RTE0::RTE0(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "RTE0:");
		stream.hexdump(stream.size());
	}
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE1::RTE1(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "RTE1:");
		stream.hexdump(stream.size());
	}
	data.resize(stream.size(), 0);
	if (stream.size())
		stream.read(&data[0], stream.size());
}

RTE2::RTE2(Cast *cast, Common::SeekableReadStreamEndian &stream) : _cast(cast) {
	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "RTE2:");
		stream.hexdump(stream.size());
	}
	_surface = nullptr;
	if (!stream.size())
		return;

	width = stream.readUint16BE();
	height = stream.readUint16BE();
	bpp = stream.readUint16BE();
	int checkMax = (1 << bpp) - 1;
	debugC(5, kDebugLoading, "RTE2: width: %d, height: %d, bpp: %d", width, height, bpp);
	// Create a 32-bit alpha surface for the decoded image
	_surface = new Graphics::Surface();
	_surface->create((int16)width, (int16)height, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
	uint8 r = 0;
	uint8 g = 0;
	uint8 b = 0;
	uint8 a = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width;) {
			uint32 pos = stream.pos();
			byte check = stream.readByte();
			if (check == 0x1f) {
				r = stream.readByte();
				g = stream.readByte();
				b = stream.readByte();
				debugC(9, kDebugLoading, "[%04x] (%d, %d): color %d %d %d", pos, x, y, r, g, b);
				continue;
			}
			a = ((uint32)check*0xff/((1 << bpp) - 1));
			if (check == 0 || check == checkMax) {
				byte count = stream.readByte();
				debugC(9, kDebugLoading, "[%04x] (%d, %d): %02x, count %d", pos, x, y, check, count);
				if (count == 0x00 && check == 0x00) {
					// end of line, fill the remaining colour
					a = 0;
					while (x < width) {
						*(uint32 *)_surface->getBasePtr(x, y) = ((uint32)r << 24) + ((uint32)g << 16) + ((uint32)b << 8) + a;
						x += 1;
					}
					break;
				}
				for (byte j = 0; j < count; j++) {
					*(uint32 *)_surface->getBasePtr(x, y) = ((uint32)r << 24) + ((uint32)g << 16) + ((uint32)b << 8) + a;
					x += 1;
					if (x >= width)
						break;
				}
			} else {
				debugC(9, kDebugLoading, "[%04x] (%d, %d): %02x", pos, x, y, check);
				*(uint32 *)_surface->getBasePtr(x, y) = ((uint32)r << 24) + ((uint32)g << 16) + ((uint32)b << 8) + a;
				x += 1;
			}
		}
	}
}

RTE2::~RTE2() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

} // End of namespace Director
