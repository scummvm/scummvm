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

#include "glk/raw_decoder.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Glk {

RawDecoder::RawDecoder() : Image::ImageDecoder(), _palette(0),
	_transColor(0) {
}

RawDecoder::~RawDecoder() {
	destroy();
}

void RawDecoder::destroy() {
	_surface.free();
	_palette.clear();
}

bool RawDecoder::loadStream(Common::SeekableReadStream &stream) {
	// Reset member variables from previous decodings
	destroy();

	uint width = stream.readUint16LE();
	uint height = stream.readUint16LE();
	uint paletteColorCount = stream.readUint16LE();
	assert(paletteColorCount == 0 || paletteColorCount <= 0x100);

	if (paletteColorCount != 0) {
		// Read in the palette
		_palette.resize(paletteColorCount, false);
		for (uint16 i = 0; i < paletteColorCount; i++) {
			byte r = stream.readByte();
			byte g = stream.readByte();
			byte b = stream.readByte();
			_palette.set(i, r, g, b);
		}

		// Get the transparent color
		byte transColor = stream.readByte();
		if (transColor < paletteColorCount)
			_transColor = transColor;
	} else {
		_transColor = 0;
	}

	// Set up the surface
	_surface.create(width, height, (paletteColorCount == 0) ?
	    Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0) :
		Graphics::PixelFormat::createFormatCLUT8());

	assert((stream.size() - stream.pos()) ==
		(int)(width * height * _surface.format.bytesPerPixel));
	byte *pixels = (byte *)_surface.getPixels();
	stream.read(pixels, width * height * _surface.format.bytesPerPixel);

	if (paletteColorCount > 0) {
		for (uint idx = 0; idx < width * height; ++idx, ++pixels) {
			assert(*pixels != 0xff);
			if (*pixels >= paletteColorCount)
				*pixels = paletteColorCount - 1;
		}
	}

	return true;
}

} // End of namespace Glk
