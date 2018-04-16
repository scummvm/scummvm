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

// Based off ffmpeg's msrledec.c
#include "common/debug.h"
#include "image/codecs/msrle4.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
namespace Image {

MSRLE4Decoder::MSRLE4Decoder(uint16 width, uint16 height, byte bitsPerPixel) {
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_bitsPerPixel = bitsPerPixel;
}

MSRLE4Decoder::~MSRLE4Decoder() {
	_surface->free();
	delete _surface;
}

const Graphics::Surface *MSRLE4Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (_bitsPerPixel == 4) {
		decode4(stream);
	} else
		error("Unhandled %d bit Microsoft RLE encoding", _bitsPerPixel);

	return _surface;
}

void MSRLE4Decoder::decode4(Common::SeekableReadStream &stream) {
	int x = 0;
	int y = _surface->h - 1;

	byte *output     = (byte *)_surface->getBasePtr(x, y);
	byte *output_end = (byte *)_surface->getBasePtr(_surface->w, y);

	while (!stream.eos()) {
		byte count = stream.readByte();

		if (count == 0) {
			byte value = stream.readByte();

			if (value == 0) {
				// End of line

				x = 0;
				y--;
				output = (byte *)_surface->getBasePtr(x, y);
			} else if (value == 1) {
				// End of image

				return;
			} else if (value == 2) {
				// Skip

				count = stream.readByte();
				value = stream.readByte();

				x += count;
				y -= value;

				if (y < 0) {
					warning("MS RLE Codec: Skip beyond picture bounds");
					return;
				}

				output = (byte *)_surface->getBasePtr(x, y);

			} else {
				// Copy data

				int odd_pixel = value & 1;
				int rle_code = (value + 1) / 2;
				int extra_byte = rle_code & 0x01;

				if (output + value > output_end) {
					stream.skip(rle_code + extra_byte);
					continue;
				}

				for (int i = 0; i < rle_code; i++) {
					byte color = stream.readByte();
					*output++ = (color & 0xf0) >> 4;
					if (i + 1 == rle_code && odd_pixel) {
						break;
					}
					*output++ = color & 0x0f;
				}

				if (extra_byte)
					stream.skip(1);

				x += value;
			}

		} else {
			// Run data

			if (output + count > output_end)
				continue;

			byte color = stream.readByte();

			for (int i = 0; i < count; i++, x++) {
				*output++ = (color & 0xf0) >> 4;
				i++;
				x++;
				if (i == count)
					break;
				*output++ = color & 0x0f;
			}
		}

	}

	warning("MS RLE Codec: No end-of-picture code");
}

} // End of namespace Image
