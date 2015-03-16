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
 */

#include "sherlock/sprite.h"
#include "sherlock/graphics.h"
#include "common/debug.h"

namespace Sherlock {

Sprite::Sprite(Common::SeekableReadStream &stream, bool skipPal) {
	Common::fill(&_palette[0], &_palette[PALETTE_SIZE], 0);
	load(stream, skipPal);
}

Sprite::~Sprite() {
	for (uint idx = 0; idx < size(); ++idx)
		(*this)[idx]._frame.free();
}

/**
 * Load the data of the sprite
 */
void Sprite::load(Common::SeekableReadStream &stream, bool skipPal) {
	loadPalette(stream);

    while (stream.pos() < stream.size()) {
		SpriteFrame frame;
		frame._width = stream.readUint16LE() + 1;
		frame._height = stream.readUint16LE() + 1;
		frame._flags = stream.readUint16LE();
        stream.readUint16LE();
        
		if (skipPal)
			frame._flags = 0;

		if (frame._flags & 0xFF) {
			// Nibble packed frame data
			frame._size = (frame._width * frame._height) / 2;
		} else if (frame._flags & RLE_ENCODED) {
            // this size includes the header size, which we subtract
			frame._size = stream.readUint16LE() - 11;
			frame._rleMarker = stream.readByte();
        } else {
			// Uncompressed data
			frame._size = frame._width * frame._height;
        }

		// Load data for frame and decompress it
		byte *data = new byte[frame._size];
		stream.read(data, frame._size);
        decompressFrame(frame, data);		
		delete data;

		push_back(frame);
    }
}

/**
 * Gets the palette at the start of the sprite file
 */
void Sprite::loadPalette(Common::SeekableReadStream &stream) {
	int v1 = stream.readUint16LE() + 1;
	int v2 = stream.readUint16LE() + 1;
	int size = v1 * v2;
	assert((size - 12) == PALETTE_SIZE);

	stream.seek(4 + 12, SEEK_CUR);
	stream.read(&_palette[0], PALETTE_SIZE);
}

/**
 * Decompress a single frame for the sprite
 */
void Sprite::decompressFrame(SpriteFrame &frame, const byte *src) {
	frame._frame.create(frame._width, frame._height, Graphics::PixelFormat::createFormatCLUT8());

	if (frame._flags & 0xFF) {
	    debug("TODO: Sprite::decompressFrame() 4-bits/pixel\n");
	} else if (frame._flags & RLE_ENCODED) {
		// RLE encoded
	    byte *dst = (byte *)frame._frame.getPixels();

		int size = frame._width * frame._height;
		while (size > 0) {
			if (*src == frame._rleMarker) {
			    byte rleColor = src[1];
			    byte rleCount = src[2];
			    src += 3;
			    size -= rleCount;
			    while (rleCount--)
			        *dst++ = rleColor;
			} else {
				*dst++ = *src++;
				--size;
			}
		}
		assert(size == 0);
	} else {
		// Uncompressed frame
		Common::copy(src, src + frame._width * frame._height,
			(byte *)frame._frame.getPixels());
	}
}

} // End of namespace Sherlock
