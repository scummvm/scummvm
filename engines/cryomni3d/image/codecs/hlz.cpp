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

#include "cryomni3d/image/codecs/hlz.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Image {

HLZDecoder::HLZDecoder(int width, int height) : Codec(),
	_width(width), _height(height), _surface(nullptr) {
}

HLZDecoder::~HLZDecoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

const Graphics::Surface *HLZDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (!_surface) {
		_surface = new Graphics::Surface();
	}

	_surface->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());

	byte *dst = (byte *)_surface->getPixels();
	decodeFrameInPlace(stream, uint32(-1), dst);

	return _surface;
}

Graphics::PixelFormat HLZDecoder::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}

static inline uint getReg(Common::SeekableReadStream &stream, uint32 *size, uint32 *reg,
                          int *regBits) {
	if (*regBits == 0) {
		if (*size < 4) {
			error("Can't feed register: not enough data");
		}
		*reg = stream.readUint32LE();
		*size -= 4;
		*regBits = 32;
	}
	uint ret = (*reg >> 31) & 0x1;
	*reg <<= 1;
	(*regBits)--;
	return ret;
}

void HLZDecoder::decodeFrameInPlace(Common::SeekableReadStream &stream, uint32 size, byte *dst) {
	bool eof = false;
	bool checkSize = (size != (uint32) - 1);
	byte *orig = dst;
	uint32 reg = 0;
	int regBits = 0;
#define GETREG() getReg(stream, &size, &reg, &regBits)

	while (!eof) {
		if (GETREG()) {
			if (size < 1) {
				error("Can't read pixel byte");
			}
			byte c = stream.readByte();
			*(dst++) = c;
			size--;
		} else {
			int offset, repeat_count;
			if (GETREG()) {
				// Long repeat
				if (size < 2) {
					error("Can't read repeat count/offset");
				}
				uint16 tmp = stream.readUint16LE();
				size -= 2;
				repeat_count = tmp & 0x7;
				offset = (tmp >> 3) - 0x2000;
				if (repeat_count == 0) {
					if (size < 1) {
						error("Can't read long repeat count");
					}
					repeat_count = stream.readByte();
					size--;
					if (repeat_count == 0) {
						eof = true;
						continue;
					}
				}
			} else {
				// Short repeat
				repeat_count = GETREG() << 1;
				repeat_count |= GETREG();
				if (size < 1) {
					error("Can't read offset byte");
				}
				offset = stream.readByte() - 0x100;
				size--;
			}
			repeat_count += 2;
			if (dst + offset < orig) {
				error("Invalid offset %d, dst is %d", offset, (int)(dst - orig));
			}
			for (; repeat_count > 0; repeat_count--) {
				// offset is always < 0
				*dst = *(dst + offset);
				dst++;
			}
		}
	}
	if (checkSize && size != 0) {
		stream.skip(size);
	}
#undef GETREG
}

} // End of namespace Image
