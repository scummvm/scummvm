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

#ifndef FREESCAPE_NEO_H
#define FREESCAPE_NEO_H

#include "common/scummsys.h"
#include "common/str.h"
#include "image/image_decoder.h"

/*
Atari-ST Neochrome decoder based on NEOLoad by Jason "Joefish" Railton
*/

namespace Common {
class SeekableReadStream;
}

namespace Image {

class NeoDecoder : public ImageDecoder {
public:
	NeoDecoder(byte *palette = nullptr);
	virtual ~NeoDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	Graphics::Surface *_surface;
	bool _paletteDestroy;
	byte *_palette;
	uint16 _paletteColorCount;
};
} // End of namespace Image

#endif // FREESCAPE_NEO_H
