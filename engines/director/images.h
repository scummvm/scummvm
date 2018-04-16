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

#ifndef DIRECTOR_IMAGES_H
#define DIRECTOR_IMAGES_H

#include "common/scummsys.h"
#include "common/str.h"
#include "image/image_decoder.h"
#include "image/codecs/bmp_raw.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {
class Codec;
}

namespace Director {

class DIBDecoder : public Image::ImageDecoder {
public:
	DIBDecoder();
	virtual ~DIBDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	void loadPalette(Common::SeekableReadStream &stream);
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	Image::Codec *_codec;
	const Graphics::Surface *_surface;
	byte *_palette;
	uint8 _paletteColorCount;
};

class BITDDecoder : public Image::ImageDecoder {
public:
	BITDDecoder(int w, int h);
	virtual ~BITDDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	void loadPalette(Common::SeekableReadStream &stream);
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	Graphics::Surface *_surface;
	byte *_palette;
	uint8 _paletteColorCount;
};

class BITDDecoderV4 : public Image::ImageDecoder {
public:
	BITDDecoderV4(int w, int h, uint16 bitsPerPixel);
	virtual ~BITDDecoderV4();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	void loadPalette(Common::SeekableReadStream &stream);
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	Graphics::Surface *_surface;
	byte *_palette;
	uint8 _paletteColorCount;
	uint16 _bitsPerPixel;
};

} // End of namespace Director

#endif
