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

#ifndef IMAGE_CODECS_QTRLE_H
#define IMAGE_CODECS_QTRLE_H

#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "image/codecs/codec.h"

namespace Image {

/**
 * QuickTime Run-Length Encoding decoder.
 *
 * Used by PICT/QuickTime.
 */
class QTRLEDecoder : public Codec {
public:
	QTRLEDecoder(uint16 width, uint16 height, byte bitsPerPixel);
	~QTRLEDecoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override;

	bool containsPalette() const override { return _ditherPalette != 0; }
	const byte *getPalette() override { _dirtyPalette = false; return _ditherPalette->data; }
	bool hasDirtyPalette() const override { return _dirtyPalette; }
	bool canDither(DitherType type) const override;
	void setDither(DitherType type, const byte *palette) override;

private:
	byte _bitsPerPixel;
	Graphics::Surface *_surface;
	uint16 _width, _height;
	uint32 _paddedWidth;
	Graphics::Palette *_ditherPalette;
	bool _dirtyPalette;
	byte *_colorMap;

	void createSurface();

	void decode1(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
	void decode2_4(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange, byte bpp);
	void decode8(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
	void decode16(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
	void decode24(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
	void dither24(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
	void decode32(Common::SeekableReadStream &stream, uint32 rowPtr, uint32 linesToChange);
};

} // End of namespace Image

#endif
