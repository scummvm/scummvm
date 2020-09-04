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

#include "image/image_decoder.h"

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
	~DIBDecoder() override;

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return _surface; }
	const byte *getPalette() const override { return _palette; }
	void loadPalette(Common::SeekableReadStream &stream);
	uint16 getPaletteColorCount() const override { return _paletteColorCount; }

private:
	Image::Codec *_codec;
	const Graphics::Surface *_surface;
	byte *_palette;
	uint8 _paletteColorCount;
};

class BITDDecoder : public Image::ImageDecoder {
public:
	BITDDecoder(int w, int h, uint16 bitsPerPixel, uint16 pitch, const byte *palette);
	~BITDDecoder() override;

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return _surface; }
	const byte *getPalette() const override { return _palette; }
	void loadPalette(Common::SeekableReadStream &stream);
	uint16 getPaletteColorCount() const override { return _paletteColorCount; }
	void convertPixelIntoSurface(void* surfacePointer, uint fromBpp, uint toBpp, int red, int green, int blue);

private:
	Graphics::Surface *_surface;
	const byte *_palette;
	uint8 _paletteColorCount;
	uint16 _bitsPerPixel;
	uint16 _pitch;
};

} // End of namespace Director

#endif
