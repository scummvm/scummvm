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

#ifndef IMAGE_CODECS_CINEPAK_H
#define IMAGE_CODECS_CINEPAK_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "image/codecs/codec.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {

struct CinepakCodebook {
	// These are not in the normal YUV colorspace, but in the Cinepak YUV colorspace instead.
	byte y[4]; // [0, 255]
	int8 u, v; // [-128, 127]
};

struct CinepakStrip {
	uint16 id;
	uint16 length;
	Common::Rect rect;
	CinepakCodebook v1_codebook[256], v4_codebook[256];
	byte v1_dither[256 * 4 * 4 * 4], v4_dither[256 * 4 * 4 * 4];
};

struct CinepakFrame {
	byte flags;
	uint32 length;
	uint16 width;
	uint16 height;
	uint16 stripCount;
	CinepakStrip *strips;

	Graphics::Surface *surface;
};

/**
 * Cinepak decoder.
 *
 * Used by BMP/AVI and PICT/QuickTime.
 *
 * Used in engines:
 *  - sherlock
 */
class CinepakDecoder : public Codec {
public:
	CinepakDecoder(int bitsPerPixel = 24);
	~CinepakDecoder() override;

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) override;
	Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
	bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;

	bool containsPalette() const override { return _ditherPalette != 0; }
	const byte *getPalette() override { _dirtyPalette = false; return _ditherPalette->data; }
	bool hasDirtyPalette() const override { return _dirtyPalette; }
	bool canDither(DitherType type) const override;
	void setDither(DitherType type, const byte *palette) override;

private:
	CinepakFrame _curFrame;
	int32 _y;
	int _bitsPerPixel;
	Graphics::PixelFormat _pixelFormat;
	byte *_clipTable, *_clipTableBuf;

	Graphics::Palette *_ditherPalette;
	bool _dirtyPalette;
	byte *_colorMap;
	DitherType _ditherType;

	void initializeCodebook(uint16 strip, byte codebookType);
	void loadCodebook(Common::SeekableReadStream &stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize);
	void decodeVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize);

	byte findNearestRGB(int index) const;
	void ditherVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize);
	void ditherCodebookQT(uint16 strip, byte codebookType, uint16 codebookIndex);
};

} // End of namespace Image

#endif
