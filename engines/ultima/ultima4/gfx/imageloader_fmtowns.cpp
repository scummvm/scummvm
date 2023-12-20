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

#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/imageloader_fmtowns.h"
#include "ultima/ultima4/gfx/imageloader_u4.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Ultima {
namespace Ultima4 {

bool FMTOWNSImageDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (_width == -1 || _height == -1 || _bpp == -1) {
		error("dimensions not set for fmtowns image");
	}

	assertMsg((_bpp == 16) | (_bpp == 4), "invalid bpp: %d", _bpp);

	int rawLen = stream.size() - _offset;
	int requiredLength = (_width * _height * _bpp / 8);
	if (rawLen < requiredLength) {
		warning("FMTOWNS Image of size %d does not fit anticipated size %d", rawLen, requiredLength);
		return false;
	}

	stream.seek(_offset, 0);
	byte *raw = (byte *) malloc(rawLen);
	stream.read(raw, rawLen);


	if (_bpp == 4) {
		_surface = new Graphics::Surface();
		_surface->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());
		setFromRawData(raw);

		U4PaletteLoader pal;
		_palette = pal.loadEgaPalette();
		_paletteColorCount = 16;
	}


	if (_bpp == 16) {
		_surface = new Graphics::Surface();
		_surface->create(_width, _height, Graphics::PixelFormat(2, 5, 5, 5, 1, 5, 10, 0, 15));

		uint16 *dst = (uint16 *)_surface->getPixels();
		const uint16 *src = (const uint16 *)raw;

		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width; x++) {
				dst[x] = FROM_LE_16(*src++) ^ 0x8000;
			}

			dst = (uint16 *)((uint8 *)dst + _surface->pitch);
		}
	}

	free(raw);

	return true;
}

} // End of namespace Ultima4
} // End of namespace Ultima
