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

#ifndef IMAGE_CODECS_CINEPAK_H
#define IMAGE_CODECS_CINEPAK_H

#include "common/scummsys.h"
#include "common/rect.h"
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
	~CinepakDecoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const { return _pixelFormat; }

	bool containsPalette() const { return _ditherPalette != 0; }
	const byte *getPalette() { _dirtyPalette = false; return _ditherPalette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }
	bool canDither(DitherType type) const;
	void setDither(DitherType type, const byte *palette);

private:
	CinepakFrame _curFrame;
	int32 _y;
	int _bitsPerPixel;
	Graphics::PixelFormat _pixelFormat;
	byte *_clipTable, *_clipTableBuf;

	byte *_ditherPalette;
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
