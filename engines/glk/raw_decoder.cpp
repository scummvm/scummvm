/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/raw_decoder.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Glk {

RawDecoder::RawDecoder() : Image::ImageDecoder(), _palette(nullptr), _paletteColorCount(0),
	_transColor(0) {
}

RawDecoder::~RawDecoder() {
	destroy();
}

void RawDecoder::destroy() {
	_surface.free();
	delete[] _palette;
	_palette = nullptr;
}

bool RawDecoder::loadStream(Common::SeekableReadStream &stream) {
	// Reset member variables from previous decodings
	destroy();

	uint width = stream.readUint16LE();
	uint height = stream.readUint16LE();
	_paletteColorCount = stream.readUint16LE();
	assert(_paletteColorCount == 0 || _paletteColorCount <= 0x100);

	if (_paletteColorCount != 0) {
		// Read in the palette
		_palette = new byte[_paletteColorCount * 3];
		stream.read(_palette, _paletteColorCount * 3);

		// Get the transparent color
		byte transColor = stream.readByte();
		if (transColor < _paletteColorCount)
			_transColor = transColor;
	} else {
		_transColor = 0;
	}

	// Set up the surface
	_surface.create(width, height, (_paletteColorCount == 0) ?
	    Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0) :
		Graphics::PixelFormat::createFormatCLUT8());

	assert((stream.size() - stream.pos()) ==
		(int)(width * height * _surface.format.bytesPerPixel));
	byte *pixels = (byte *)_surface.getPixels();
	stream.read(pixels, width * height * _surface.format.bytesPerPixel);

	if (_palette) {
		for (uint idx = 0; idx < width * height; ++idx, ++pixels) {
			assert(*pixels != 0xff);
			if (*pixels >= _paletteColorCount)
				*pixels = _paletteColorCount - 1;
		}
	}

	return true;
}

} // End of namespace Glk
