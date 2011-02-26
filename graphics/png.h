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
 * $URL$
 * $Id$
 *
 */

/*
 * PNG decoder used in engines:
 *  - sword25
 */

#ifndef GRAPHICS_PNG_H
#define GRAPHICS_PNG_H

#include "graphics/surface.h"

// PNG decoder, based on the W3C specs:
// http://www.w3.org/TR/PNG/
// Parts of the code have been adapted from LodePNG, by Lode Vandevenne:
// http://members.gamedev.net/lode/projects/LodePNG/

// All the numbers are BE: http://www.w3.org/TR/PNG/#7Integers-and-byte-order

// Note: At the moment, this decoder only supports non-interlaced images, and
// does not support truecolor/grayscale images with 16bit depth.
//
// Theoretically, interlaced images shouldn't be needed for games, as
// interlacing is only useful for images in websites.
//
// PNG images with 16bit depth (i.e. 48bit images) are quite rare, and can
// theoretically contain more than 16.7 millions of colors (the so-called "deep
// color" representation). In essence, each of the R, G, B and A components in
// them is specified with 2 bytes, instead of 1. However, the PNG specification
// always refers to color components with 1 byte each, so this part of the spec
// is a bit unclear. For now, these won't be supported, until a suitable sample
// is found.

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

enum PNGColorType {
	kGrayScale          = 0,	// bit depths: 1, 2, 4, 8, 16
	kTrueColor          = 2,	// bit depths: 8, 16
	kIndexed            = 3,	// bit depths: 1, 2, 4, 8
	kGrayScaleWithAlpha = 4,	// bit depths: 8, 16
	kTrueColorWithAlpha = 6		// bit depths: 8, 16
};

enum PNGInterlaceType {
	kNonInterlaced      = 0,
	kInterlaced         = 1
};

struct PNGHeader {
	uint32 width;
	uint32 height;
	byte bitDepth;
	PNGColorType colorType;
	byte compressionMethod;
	byte filterMethod;
	PNGInterlaceType interlaceType;
};

struct PixelFormat;

class PNG {
public:
	PNG();
	~PNG();

	/**
	 * Reads a PNG image from the specified stream
	 */
	bool read(Common::SeekableReadStream *str);

	/**
	 * Returns the information obtained from the PNG header.
	 */
	PNGHeader getHeader() const { return _header; }

	/**
	 * Returns the PNG image, formatted for the specified pixel format.
	 */
	Graphics::Surface *getSurface(const PixelFormat &format);

	/**
	 * Returns the indexed PNG8 image. Used for PNGs with an indexed 256 color
	 * palette, when they're shown on an 8-bit color screen, as no translation
	 * is taking place.
	 */
	Graphics::Surface *getIndexedSurface() {
		if (_header.colorType != kIndexed)
			error("Indexed surface requested for a non-indexed PNG");
		return _unfilteredSurface;
	}

	/**
	 * Returns the palette of the specified PNG8 image.
	 *
	 * Note that the palette's format is RGBA.
	 */
	void getPalette(byte *palette, uint16 &entries) {
		if (_header.colorType != kIndexed)
			error("Palette requested for a non-indexed PNG");
		palette = _palette;
		entries = _paletteEntries;
	}

private:
	void readHeaderChunk();
	byte getNumColorChannels();

	void readPaletteChunk();
	void readTransparencyChunk(uint32 chunkLength);

	void constructImage();
	void unfilterScanLine(byte *dest, const byte *scanLine, const byte *prevLine, uint16 byteWidth, byte filterType, uint16 length);
	byte paethPredictor(int16 a, int16 b, int16 c);

	// The original file stream
	Common::SeekableReadStream *_stream;
	// The unzipped image data stream
	Common::SeekableReadStream *_imageData;

	PNGHeader _header;

	byte _palette[256 * 4];	// RGBA
	uint16 _paletteEntries;
	uint16 _transparentColor[3];
	bool _transparentColorSpecified;

	byte *_compressedBuffer;
	uint32 _compressedBufferSize;

	Graphics::Surface *_unfilteredSurface;
};

} // End of Graphics namespace

#endif // GRAPHICS_PNG_H
