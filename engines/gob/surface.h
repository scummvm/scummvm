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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

#ifndef GOB_SURFACE_H
#define GOB_SURFACE_H

#include "common/scummsys.h"
#include "common/ptr.h"
#include "common/rational.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {
class ImageDecoder;
}

namespace Gob {

enum ImageType {
	kImageTypeNone = -1,
	kImageTypeTGA  =  0,
	kImageTypeIFF,
	kImageTypeBRC,
	kImageTypeBMP,
	kImageTypeJPEG
};

/** An iterator over a surface's image data, automatically handles different color depths. */
class Pixel {
public:
	Pixel(byte *vidMem, uint8 bpp, byte *min, byte *max);

	Pixel &operator++();
	Pixel operator++(int x);

	Pixel &operator--();
	Pixel operator--(int x);

	Pixel &operator+=(int x);
	Pixel &operator-=(int x);

	uint32 get() const;
	void set(uint32 p);

	bool isValid() const;

private:
	byte *_vidMem;
	byte *_min, *_max;
	uint8 _bpp;
};

/** A const iterator over a surface's image data, automatically handles different color depths. */
class ConstPixel {
public:
	ConstPixel(const byte *vidMem, uint8 bpp, const byte *min, const byte *max);

	ConstPixel &operator++();
	ConstPixel operator++(int x);

	ConstPixel &operator--();
	ConstPixel operator--(int x);

	ConstPixel &operator+=(int x);
	ConstPixel &operator-=(int x);

	uint32 get() const;

	bool isValid() const;

private:
	const byte *_vidMem;
	const byte *_min, *_max;
	uint8 _bpp;
};

class Surface {
public:
	Surface(uint16 width, uint16 height, uint8 bpp, byte *vidMem = nullptr, const uint32 *highColorMap = nullptr, bool ownHighColorMap = false);
	Surface(uint16 width, uint16 height, uint8 bpp, const byte *vidMem, const uint32 *highColorMap = nullptr, bool ownHighColorMap = false);
	~Surface();

	uint16 getWidth () const;
	uint16 getHeight() const;
	uint8  getBPP   () const;

	byte *getData(uint16 x = 0, uint16 y = 0);
	const byte *getData(uint16 x = 0, uint16 y = 0) const;

	void resize(uint16 width, uint16 height);

	void setBPP(uint8 bpp);

	Pixel get(uint16 x = 0, uint16 y = 0);
	ConstPixel get(uint16 x = 0, uint16 y = 0) const;

	void blit(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
	          int16 x, int16 y, int32 transp = -1, bool yAxisReflexion = false);
	void blit(const Surface &from, int16 x, int16 y, int32 transp = -1);
	void blit(const Surface &from, int32 transp = -1);

	void blitScaled(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
	                int16 x, int16 y, Common::Rational scale, int32 transp = -1);
	void blitScaled(const Surface &from, int16 x, int16 y, Common::Rational scale, int32 transp = -1);
	void blitScaled(const Surface &from, Common::Rational scale, int32 transp = -1);

	void blitShaded(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
					int16 x, int16 y, uint8 strength, int32 transp, Graphics::PixelFormat pixelFormat);

	void fillRectRaw(int16 left, int16 top, int16 right, int16 bottom, uint32 color);
	void fillRect(int16 left, int16 top, int16 right, int16 bottom, uint8 colorIndex);
	void fillArea(int16 left, int16 top, int16 right, int16 bottom, uint8 fillColorIndex, uint8 backgroundColorIndex);
	Common::Rect fillAreaAtPoint(int16 left, int16 top, uint8 fillColorIndex);
	void fill(uint32 color);
	void clear();

	void shadeRect(uint16 left, uint16 top, uint16 right, uint16 bottom,
			uint8 colorIndex, uint8 strength);

	void recolor(uint8 from, uint8 to);

	void putPixelRaw(uint16 x, uint16 y, uint32 color);
	void putPixel(uint16 x, uint16 y, uint8 colorIndex);
	void drawLineRaw(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint32 colorIndex);
	void drawLine(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 colorIndex);
	void drawRect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint8 colorIndex);
	void drawCircle(uint16 x0, uint16 y0, uint16 radius, uint8 colorIndex, int16 pattern = 0);

	void blitToScreen(uint16 left, uint16 top, uint16 right, uint16 bottom, uint16 x, uint16 y) const;

	bool loadImage(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom,
				   int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
	bool loadImage(Common::SeekableReadStream &stream, ImageType type, int16 left, int16 top, int16 right, int16 bottom,
				   int16 x, int16 y, int16 transp, Graphics::PixelFormat format);

	uint32 getColorFromIndex(uint8 index) const;

	static ImageType identifyImage(Common::SeekableReadStream &stream);
	static bool getImageInfo(Common::SeekableReadStream &stream, uint32 &width, uint32 &height, uint32 &bpp);
	static void computeHighColorMap(uint32 *highColorMap, const byte *palette,
									const Graphics::PixelFormat &format,
									bool useSpecialBlackWhiteValues,
									int16 startColor = 0, int16 colorCount = 256,
									int16 startColorSrc = -1);

private:
	uint16 _width;
	uint16 _height;
	uint8  _bpp;

	bool  _ownVidMem;
	byte *_vidMem;

	bool _ownHighColorMap;
	const uint32 *_highColorMap;

	static bool clipBlitRect(int16 &left, int16 &top, int16 &right, int16 &bottom, int16 &x, int16 &y,
	                         uint16 dWidth, uint16 dHeight, uint16 sWidth, uint16 sHeight);

	bool loadImage(Image::ImageDecoder &decoder, Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom,
				   int16 x, int16 y, int16 transp, Graphics::PixelFormat format);

	bool loadTGA (Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
	bool loadIFF (Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
	bool loadBRC (Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
	bool loadBMP (Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
	bool loadJPEG(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format);
};

typedef Common::SharedPtr<Surface> SurfacePtr;

} // End of namespace Gob

#endif // GOB_SURFACE_H
