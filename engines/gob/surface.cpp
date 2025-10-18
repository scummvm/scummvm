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

#include "gob/surface.h"

#include "common/system.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/frac.h"
#include "common/textconsole.h"
#include "common/stack.h"

#include "graphics/palette.h"
#include "graphics/primitives.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "image/bmp.h"
#include "image/brc.h"
#include "image/iff.h"
#include "image/jpeg.h"
#include "image/tga.h"

namespace Gob {

class SurfacePrimitives final : public Graphics::Primitives {
public:
        void drawPoint(int x, int y, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->putPixelRaw(x, y, color);
	}

        void drawHLine(int x1, int x2, int y, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->fillRectRaw(x1, y, x2, y, color);
	}

        void drawVLine(int x, int y1, int y2, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->fillRectRaw(x, y1, x, y2, color);
	}

	void drawFilledRect(const Common::Rect &rect, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->fillRectRaw(rect.left, rect.top, rect.right - 1, rect.bottom - 1, color);
	}

	void drawFilledRect1(const Common::Rect &rect, uint32 color, void *data) override {
		Surface *s = (Surface *)data;
		s->fillRectRaw(rect.left, rect.top, rect.right, rect.bottom, color);
	}
};

Pixel::Pixel(byte *vidMem, uint8 bpp, byte *min, byte *max) :
	_vidMem(vidMem), _bpp(bpp), _min(min), _max(max) {

	assert((_bpp == 1) || (_bpp == 2) || (_bpp == 4));
	assert(_vidMem >= _min);
	assert(_vidMem <  _max);
}

Pixel &Pixel::operator++() {
	_vidMem += _bpp;

	return *this;
}

Pixel Pixel::operator++(int x) {
	Pixel p = *this;
	++(*this);
	return p;
}

Pixel &Pixel::operator--() {
	_vidMem -= _bpp;
	return *this;
}

Pixel Pixel::operator--(int x) {
	Pixel p = *this;
	--(*this);
	return p;
}

Pixel &Pixel::operator+=(int x) {
	_vidMem += x * _bpp;
	return *this;
}

Pixel &Pixel::operator-=(int x) {
	_vidMem -= x * _bpp;
	return *this;
}

uint32 Pixel::get() const {
	assert(_vidMem >= _min);
	assert(_vidMem <  _max);

	if (_bpp == 1)
		return *((byte *) _vidMem);
	if (_bpp == 2)
		return *((uint16 *) _vidMem);
	if (_bpp == 4)
		return *((uint32 *) _vidMem);

	return 0;
}

void Pixel::set(uint32 p) {
	assert(_vidMem >= _min);
	assert(_vidMem <  _max);

	if (_bpp == 1)
		*((byte *) _vidMem) = (byte) p;
	if (_bpp == 2)
		*((uint16 *) _vidMem) = (uint16) p;
	if (_bpp == 4)
		*((uint32 *) _vidMem) = (uint32) p;
}

bool Pixel::isValid() const {
	return (_vidMem >= _min) && (_vidMem < _max);
}


ConstPixel::ConstPixel(const byte *vidMem, uint8 bpp, const byte *min, const byte *max) :
	_vidMem(vidMem), _bpp(bpp), _min(min), _max(max) {

	assert((_bpp == 1) || (_bpp == 2) || (_bpp == 4));
	assert(_vidMem >= _min);
	assert(_vidMem <  _max);
}


ConstPixel &ConstPixel::operator++() {
	_vidMem += _bpp;
	return *this;
}

ConstPixel ConstPixel::operator++(int x) {
	ConstPixel p = *this;
	++(*this);
	return p;
}

ConstPixel &ConstPixel::operator--() {
	_vidMem -= _bpp;
	return *this;
}

ConstPixel ConstPixel::operator--(int x) {
	ConstPixel p = *this;
	--(*this);
	return p;
}

ConstPixel &ConstPixel::operator+=(int x) {
	_vidMem += x * _bpp;
	return *this;
}

ConstPixel &ConstPixel::operator-=(int x) {
	_vidMem -= x * _bpp;
	return *this;
}

uint32 ConstPixel::get() const {
	assert(_vidMem >= _min);
	assert(_vidMem <  _max);

	if (_bpp == 1)
		return *((const byte *) _vidMem);
	if (_bpp == 2)
		return *((const uint16 *) _vidMem);
	if (_bpp == 4)
		return *((const uint32 *) _vidMem);

	return 0;
}

bool ConstPixel::isValid() const {
	return (_vidMem >= _min) && (_vidMem < _max);
}


Surface::Surface(uint16 width, uint16 height, uint8 bpp, byte *vidMem, const uint32 *highColorMap, bool ownHighColorMap) :
	_width(width), _height(height), _bpp(bpp), _vidMem(vidMem),
	_ownHighColorMap(ownHighColorMap),
	_highColorMap(highColorMap)  {

	assert((_width > 0) && (_height > 0));
	assert((_bpp == 1) || (_bpp == 2) || (_bpp == 4));

	if (!_vidMem) {
		_vidMem    = new byte[_bpp * _width * _height]();
		_ownVidMem = true;
	} else
		_ownVidMem = false;
}

Surface::Surface(uint16 width, uint16 height, uint8 bpp, const byte *vidMem, const uint32 *highColorMap, bool ownHighColorMap) :
	_width(width), _height(height), _bpp(bpp), _vidMem(nullptr),
	_ownHighColorMap(ownHighColorMap),
	_highColorMap(highColorMap) {

	assert((_width > 0) && (_height > 0));
	assert((_bpp == 1) || (_bpp == 2) || (_bpp == 4));

	_vidMem    = new byte[_bpp * _width * _height];
	_ownVidMem = true;

	memcpy(_vidMem, vidMem, _bpp * _width * _height);
}

Surface::~Surface() {
	if (_ownVidMem)
		delete[] _vidMem;

	if (_ownHighColorMap)
		delete[] _highColorMap;
}

uint16 Surface::getWidth() const {
	return _width;
}

uint16 Surface::getHeight() const {
	return _height;
}

uint8 Surface::getBPP() const {
	return _bpp;
}

void Surface::resize(uint16 width, uint16 height) {
	assert((width > 0) && (height > 0));

	if (_ownVidMem)
		delete[] _vidMem;

	_width  = width;
	_height = height;

	_vidMem    = new byte[_bpp * _width * _height]();
	_ownVidMem = true;
}

void Surface::setBPP(uint8 bpp) {
	if (_bpp == bpp)
		return;

	if (_ownVidMem) {
		delete[] _vidMem;

		_vidMem = new byte[bpp * _width * _height];
	} else
		_width = (_width * _bpp) / bpp;

	_bpp = bpp;

	memset(_vidMem, 0, _bpp * _width * _height);
}

byte *Surface::getData(uint16 x, uint16 y) {
	return _vidMem + (y * _width * _bpp) + (x * _bpp);
}

const byte *Surface::getData(uint16 x, uint16 y) const {
	return _vidMem + (y * _width * _bpp) + (x * _bpp);
}

Pixel Surface::get(uint16 x, uint16 y) {
	byte *vidMem = getData(x, y);

	return Pixel(vidMem, _bpp, _vidMem, _vidMem + _height * _width * _bpp);
}

ConstPixel Surface::get(uint16 x, uint16 y) const {
	const byte *vidMem = getData(x, y);

	return ConstPixel(vidMem, _bpp, _vidMem, _vidMem + _height * _width * _bpp);
}

bool Surface::clipBlitRect(int16 &left, int16 &top, int16 &right, int16 &bottom, int16 &x, int16 &y,
		uint16 dWidth, uint16 dHeight, uint16 sWidth, uint16 sHeight) {

	if ((x >= dWidth) || (y >= dHeight))
		// Nothing to do
		return false;

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= sWidth) || (top >= sHeight) || (right < 0) || (bottom < 0))
		// Nothing to do
		return false;

	// Adjust from coordinates
	if (left < 0) {
		x   -= left;
		left = 0;
	}
	if (top < 0) {
		y  -= top;
		top = 0;
	}

	// Adjust to coordinates
	if (x < 0) {
		left -= x;
		x     = 0;
	}
	if (y < 0) {
		top -= y;
		y    = 0;
	}

	// Limit by source and destination dimensions
	right  = MIN<int32>(right , MIN<int32>(sWidth , dWidth  - x + left) - 1);
	bottom = MIN<int32>(bottom, MIN<int32>(sHeight, dHeight - y + top ) - 1);

	if ((right < left) || (bottom < top))
		// Nothing to do
		return false;

	// Clip to sane values
	right  = MAX<int16>(right , 0);
	bottom = MAX<int16>(bottom, 0);

	return true;
}

uint32 Surface::getColorFromIndex(uint8 index) const {
	if (_bpp == 1 || !_highColorMap)
		return index;
	else
		return _highColorMap[index];
}

void Surface::blit(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
		int16 x, int16 y, int32 transp, bool yAxisReflection) {

	if (_bpp == 1 && from._bpp > 1) {
		// Sometimes we run into this case because of script bugs. The attempt should be just ignord, no dithering is supposed to happen here.
		warning("Error in Surface::blit(): trying to blit from a high-color source to a paletted destination");
		return;
	}

	// Color depths have to fit
	assert(_bpp == from._bpp || (from._bpp == 1 && from._highColorMap != nullptr));

	// Clip
	if (!clipBlitRect(left, top, right, bottom, x, y, _width, _height, from._width, from._height))
		return;

	// Area to actually copy
	uint16 width  = right  - left + 1;
	uint16 height = bottom - top  + 1;

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	if ((left == 0) && (_width == from._width) && (_width == width) && (transp == -1) && !yAxisReflection && (from._bpp == _bpp)) {
		// If these conditions are met, we can directly use memmove

		// Pointers to the blit destination and source start points
		      byte *dst =      getData(x   , y);
		const byte *src = from.getData(left, top);

		memmove(dst, src, width * height * _bpp);
		return;
	}

	if (transp == -1 && !yAxisReflection && from._bpp == _bpp && _bpp == 1) {
		// We don't have to look for transparency => we can use memmove line-wise

		// Pointers to the blit destination and source start points
		      byte *dst =      getData(x   , y);
		const byte *src = from.getData(left, top);

		while (height-- > 0) {
			memmove(dst, src, width * _bpp);

			dst +=      _width *      _bpp;
			src += from._width * from._bpp;
		}

		return;
	}

	// Otherwise, we have to copy by pixel

	// Pointers to the blit destination and source start points
	     Pixel dst =      get(x   , y);
	ConstPixel src = from.get(left, top);

	while (height-- > 0) {
		     Pixel dstRow = dst;
		ConstPixel srcRow = src;

		if (yAxisReflection) {
			srcRow += width - 1;
			for (uint16 i = 0; i < width; i++, ++dstRow, --srcRow) {
				if (srcRow.get() != ((uint32) transp)) {
					if (_bpp == from._bpp)
						dstRow.set(srcRow.get());
					else {
						uint32 index = srcRow.get();
						dstRow.set(from._highColorMap[index]);
					}
				}
			}
		} else {
			for (uint16 i = 0; i < width; i++, ++dstRow, ++srcRow) {
				if (srcRow.get() != ((uint32) transp)) {
					if (_bpp == from._bpp)
						dstRow.set(srcRow.get());
					else {
						uint32 index = srcRow.get();
						dstRow.set(from._highColorMap[index]);
					}
				}
			}
		}

		dst +=      _width;
		src += from._width;
	}
}

void Surface::blit(const Surface &from, int16 x, int16 y, int32 transp) {
	blit(from, 0, 0, from._width - 1, from._height - 1, x, y, transp);
}

void Surface::blit(const Surface &from, int32 transp) {
	blit(from, 0, 0, from._width - 1, from._height - 1, 0, 0, transp);
}

void Surface::blitScaled(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
		int16 x, int16 y, Common::Rational scale, int32 transp) {

	if (scale == 1) {
		// Yeah, "scaled"

		blit(from, left, top, right, bottom, x, y, transp);
		return;
	}

	// Color depths have to fit
	assert(_bpp == from._bpp);

	uint16 dWidth  = (uint16) floor((_width  / scale).toDouble());
	uint16 dHeight = (uint16) floor((_height / scale).toDouble());
	 int16 clipX   = ( int16) floor((x       / scale).toDouble());
	 int16 clipY   = ( int16) floor((y       / scale).toDouble());

	// Clip
	if (!clipBlitRect(left, top, right, bottom, clipX, clipY, dWidth, dHeight, from._width, from._height))
		return;

	// Area to actually copy
	uint16 width  = right  - left + 1;
	uint16 height = bottom - top  + 1;

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	width  = MIN<int32>((int32) floor((width  * scale).toDouble()), _width);
	height = MIN<int32>((int32) floor((height * scale).toDouble()), _height);

	// Pointers to the blit destination and source start points
	      byte *dst =      getData(x   , y);
	const byte *src = from.getData(left, top);

	frac_t step = scale.getInverse().toFrac();

	frac_t posW = 0, posH = 0;
	while (height-- > 0) {
		      byte *dstRow = dst;
		const byte *srcRow = src;

		posW = 0;

		for (uint16 i = 0; i < width; i++, dstRow += _bpp) {
			memmove(dstRow, srcRow, _bpp);

			posW += step;
			while (posW >= ((frac_t) FRAC_ONE)) {
				srcRow += from._bpp;
				posW   -= FRAC_ONE;
			}
		}

		posH += step;
		while (posH >= ((frac_t) FRAC_ONE)) {
			src  += from._width * from._bpp;
			posH -= FRAC_ONE;
		}

		dst += _width * _bpp;
	}

}

void Surface::blitScaled(const Surface &from, int16 x, int16 y, Common::Rational scale, int32 transp) {
	blitScaled(from, 0, 0, from._width - 1, from._height - 1, x, y, scale, transp);
}

void Surface::blitScaled(const Surface &from, Common::Rational scale, int32 transp) {
	blitScaled(from, 0, 0, from._width - 1, from._height - 1, 0, 0, scale, transp);
}

void Surface::blitShaded(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
						 int16 x, int16 y, uint8 strength, int32 transp, Graphics::PixelFormat pixelFormat) {

	// Color depths have to fit
	assert(_bpp == from._bpp);

	if (_bpp == 1) {
		// Cannot properly shade in paletted mode
		blit(from, left, top, right, bottom, x, y, transp);
		return;
	}

	// Clip
	if (!clipBlitRect(left, top, right, bottom, x, y, _width, _height, from._width, from._height))
		return;

	// Area to actually copy
	uint16 width  = right  - left + 1;
	uint16 height = bottom - top  + 1;

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	// Pointers to the blit destination and source start points
	     Pixel dst =      get(x   , y);
	ConstPixel src = from.get(left, top);

	while (height-- > 0) {
		     Pixel dstRow = dst;
		ConstPixel srcRow = src;
		for (uint16 i = 0; i < width; i++, ++dstRow, ++srcRow) {
			if (srcRow.get() == ((uint32) transp))
				continue;

			uint8 srcR = 0;
			uint8 srcG = 0;
			uint8 srcB = 0;
			pixelFormat.colorToRGB(srcRow.get(), srcR, srcG, srcB);

			uint8 dstR = 0;
			uint8 dstG = 0;
			uint8 dstB = 0;
			pixelFormat.colorToRGB(dstRow.get(), dstR, dstG, dstB);

			int shadeR = srcR * (16 - strength);
			int shadeB = srcB * (16 - strength);
			int shadeG = srcG * (16 - strength);

			dstR = CLIP<int>((shadeR + strength * dstR) >> 4, 0, 255);
			dstG = CLIP<int>((shadeG + strength * dstG) >> 4, 0, 255);
			dstB = CLIP<int>((shadeB + strength * dstB) >> 4, 0, 255);

			dstRow.set(pixelFormat.RGBToColor(dstR, dstG, dstB));
		}

		dst +=      _width;
		src += from._width;
	}
}

void Surface::fillRectRaw(int16 left, int16 top, int16 right, int16 bottom, uint32 color) {
	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	left   = CLIP<int32>(left  , 0, _width  - 1);
	top    = CLIP<int32>(top   , 0, _height - 1);
	right  = CLIP<int32>(right , 0, _width  - 1);
	bottom = CLIP<int32>(bottom, 0, _height - 1);

	// Area to actually fill
	uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	if ((left == 0) && (width == _width) && (_bpp == 1)) {
		// We can directly use memset

		byte *dst = getData(left, top);

		memset(dst, (byte) color, width * height);
		return;
	}

	if (_bpp == 1) {
		// We can use memset line-wise

		byte *dst = getData(left, top);

		while (height-- > 0) {
			memset(dst, (byte) color, width);
			dst += _width;
		}

		return;
	}

	assert((_bpp == 2) || (_bpp == 4));

	// Otherwise, we have to fill by pixel

	Pixel p = get(left, top);
	while (height-- > 0) {
		for (uint16 i = 0; i < width; i++, ++p)
			p.set(color);

		p += _width - width;
	}
}

void Surface::fillRect(int16 left, int16 top, int16 right, int16 bottom, uint8 colorIndex) {
	uint32 color = getColorFromIndex(colorIndex);
	fillRectRaw(left, top, right, bottom, color);
}

// Fill rectangle with fillColor, except pixels with backgroundColor
void Surface::fillArea(int16 left, int16 top, int16 right, int16 bottom, uint8 fillColorIndex, uint8 backgroundColorIndex) {
	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	left   = CLIP<int32>(left  , 0, _width  - 1);
	top    = CLIP<int32>(top   , 0, _height - 1);
	right  = CLIP<int32>(right , 0, _width  - 1);
	bottom = CLIP<int32>(bottom, 0, _height - 1);

	// Area to actually fill
	uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	uint32 fillColor = getColorFromIndex(fillColorIndex);
	uint32 backgroundColor = getColorFromIndex(backgroundColorIndex);

	Pixel p = get(left, top);
	while (height-- > 0) {
		for (uint16 i = 0; i < width; i++, ++p)
			if (p.get() != backgroundColor)
				p.set(fillColor);

		p += _width - width;
	}
}

Common::Rect Surface::fillAreaAtPoint(int16 left, int16 top, uint8 fillColorIndex) {
	uint32 fillColor = getColorFromIndex(fillColorIndex);
	Common::Rect modifiedArea;
	if (left < 0 || left >= _width || top < 0  || top >= _height)
		// Nothing to do
		return modifiedArea;

	Pixel pixel = get(left, top);
	uint32 initialColor = pixel.get();
	if (initialColor == fillColor)
		return modifiedArea;

	pixel.set(fillColor);
	modifiedArea.extend(Common::Rect(left, top, left + 1, top + 1));
	Common::Stack<Common::Point> pointsToScan;
	pointsToScan.push(Common::Point(left, top));
	int16 directions[4] = {1, 0, -1, 0};

	while (!pointsToScan.empty()) {
		Common::Point point = pointsToScan.pop();
		for (int i = 0; i < 4; i++) {
			int16 x = point.x + directions[i];
			int16 y = point.y + directions[(i + 1) % 4];
			if (x < 0 || x >= _width || y < 0 || y >= _height)
				continue;

			Pixel p = get(x, y);
			if (p.get() == initialColor) {
				p.set(fillColor);
				if (!modifiedArea.contains(x, y))
					modifiedArea.extend(Common::Rect(x, y, x + 1, y + 1));
				pointsToScan.push(Common::Point(x, y));
			}
		}
	}

	return modifiedArea;
}

void Surface::fill(uint32 color) {
	if (_bpp == 1) {
		// We can directly use memset

		memset(_vidMem, (byte) color, _width * _height);
		return;
	}

	fillRect(0, 0, _width - 1, _height - 1, color);
}

void Surface::clear() {
	fill(0);
}

void Surface::shadeRect(uint16 left, uint16 top, uint16 right, uint16 bottom,
		uint8 colorIndex, uint8 strength) {

	if (_bpp == 1) {
		// We can't properly shade in paletted mode, fill the rect instead
		fillRect(left, top, right, bottom, colorIndex);
		return;
	}

	uint32 color = getColorFromIndex(colorIndex);

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually shade
	uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

	uint8 cR, cG, cB;
	pixelFormat.colorToRGB(color, cR, cG, cB);

	int shadeR = cR * (16 - strength);
	int shadeG = cG * (16 - strength);
	int shadeB = cB * (16 - strength);

	Pixel p = get(left, top);
	while (height-- > 0) {
		for (uint16 i = 0; i < width; i++, ++p) {
			uint8 r, g, b;

			pixelFormat.colorToRGB(p.get(), r, g, b);

			r = CLIP<int>((shadeR + strength * r) >> 4, 0, 255);
			g = CLIP<int>((shadeG + strength * g) >> 4, 0, 255);
			b = CLIP<int>((shadeB + strength * b) >> 4, 0, 255);

			p.set(pixelFormat.RGBToColor(r, g, b));
		}

		p += _width - width;
	}

}

void Surface::recolor(uint8 from, uint8 to) {
	for (Pixel p = get(); p.isValid(); ++p)
		if (p.get() == from)
			p.set(to);
}

void Surface::putPixelRaw(uint16 x, uint16 y, uint32 color) {
	if ((x >= _width) || (y >= _height))
		return;

	get(x, y).set(color);
}

void Surface::putPixel(uint16 x, uint16 y, uint8 colorIndex) {
	uint32 color = getColorFromIndex(colorIndex);
	putPixelRaw(x, y, color);
}

void Surface::drawLineRaw(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint32 color) {
	SurfacePrimitives().drawLine(x0, y0, x1, y1, color, this);
}

void Surface::drawLine(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 colorIndex) {
	uint32 color = getColorFromIndex(colorIndex);
	drawLineRaw(x0, y0, x1, y1, color);
}

void Surface::drawRect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint8 colorIndex) {
	uint32 color = getColorFromIndex(colorIndex);
	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually draw
	const uint16 width  = CLIP<int32>(right  - left + 1, 0, _width  - left);
	const uint16 height = CLIP<int32>(bottom - top  + 1, 0, _height - top);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	right  = left + width  - 1;
	bottom = top  + height - 1;

	drawLineRaw(left , top   , left , bottom, color);
	drawLineRaw(right, top   , right, bottom, color);
	drawLineRaw(left , top   , right, top   , color);
	drawLineRaw(left , bottom, right, bottom, color);
}

/*
 * The original's version of the Bresenham Algorithm was a bit "unclean"
 * and produced strange edges at 45, 135, 225 and 315 degrees, so using the
 * version found in the Wikipedia article about the
 * "Bresenham's line algorithm" instead
 */
void Surface::drawCircle(uint16 x0, uint16 y0, uint16 radius, uint8 colorIndex, int16 pattern) {
	uint32 color = getColorFromIndex(colorIndex);
	int16 f = 1 - radius;
	int16 ddFx = 0;
	int16 ddFy = -2 * radius;
	int16 x = 0;
	int16 y = radius;

	switch (pattern) {
	case 0xFF:
		fillRectRaw(x0, y0 + radius, x0, y0 - radius, color);
		fillRectRaw(x0 + radius, y0, x0 - radius, y0, color);
		break ;
	case 0:
		putPixelRaw(x0, y0 + radius, color);
		putPixelRaw(x0, y0 - radius, color);
		putPixelRaw(x0 + radius, y0, color);
		putPixelRaw(x0 - radius, y0, color);
		break;
	default:
		break;
	}


	while (x < y) {
		if (f >= 0) {
			y--;
			ddFy += 2;
			f += ddFy;
		}
		x++;
		ddFx += 2;
		f += ddFx + 1;

		switch (pattern) {
		case 0xFF:
			// Fill circle
			fillRectRaw(x0 - y, y0 + x, x0 + y, y0 + x, color);
			fillRectRaw(x0 - x, y0 + y, x0 + x, y0 + y, color);
			fillRectRaw(x0 - y, y0 - x, x0 + y, y0 - x, color);
			fillRectRaw(x0 - x, y0 - y, x0 + x, y0 - y, color);
			break;
		case 0:
			putPixelRaw(x0 + x, y0 + y, color);
			putPixelRaw(x0 - x, y0 + y, color);
			putPixelRaw(x0 + x, y0 - y, color);
			putPixelRaw(x0 - x, y0 - y, color);
			putPixelRaw(x0 + y, y0 + x, color);
			putPixelRaw(x0 - y, y0 + x, color);
			putPixelRaw(x0 + y, y0 - x, color);
			putPixelRaw(x0 - y, y0 - x, color);
			break;
		default:
			fillRect(x0 + y - pattern, y0 + x - pattern, x0 + y, y0 + x, color);
			fillRect(x0 + x - pattern, y0 + y - pattern, x0 + x, y0 + y, color);
			fillRect(x0 - y, y0 + x - pattern, x0 - y + pattern, y0 + x, color);
			fillRect(x0 - x, y0 + y - pattern, x0 - x + pattern, y0 + y, color);
			fillRect(x0 + y - pattern, y0 - x, x0 + y, y0 - x + pattern, color);
			fillRect(x0 + x - pattern, y0 - y, x0 + x, y0 - y + pattern, color);
			fillRect(x0 - y, y0 - x, x0 - y + pattern, y0 - x + pattern, color);
			fillRect(x0 - x, y0 - y, x0 - x + pattern, y0 - y + pattern, color);
			break;
		}
	}
}

void Surface::blitToScreen(uint16 left, uint16 top, uint16 right, uint16 bottom, uint16 x, uint16 y) const {
	// Color depths have to fit
	assert(g_system->getScreenFormat().bytesPerPixel == _bpp);

	uint16 sWidth  = g_system->getWidth();
	uint16 sHeight = g_system->getHeight();

	if ((x >= sWidth) || (y >= sHeight))
		// Nothing to do
		return;

	// Just in case those are swapped
	if (left > right)
		SWAP(left, right);
	if (top  > bottom)
		SWAP(top, bottom);

	if ((left >= _width) || (top >= _height))
		// Nothing to do
		return;

	// Area to actually copy
	uint16 width  = MAX<int32>(MIN<int32>(MIN<int32>(right  - left + 1, _width  - left), sWidth  - x), 0);
	uint16 height = MAX<int32>(MIN<int32>(MIN<int32>(bottom - top  + 1, _height - top ), sHeight - y), 0);

	if ((width == 0) || (height == 0))
		// Nothing to do
		return;

	// Pointers to the blit destination and source start points
	const byte *src = getData(left, top);

	g_system->copyRectToScreen(src, _width * _bpp, x, y, width, height);
}

bool Surface::getImageInfo(Common::SeekableReadStream &stream, uint32 &width, uint32 &height, uint32 &bpp) {
	ImageType type = identifyImage(stream);
	if (type == kImageTypeNone)
		return false;

	Common::ScopedPtr<Image::ImageDecoder> decoder;
	switch (type) {
	case kImageTypeTGA:
		decoder.reset(new Image::TGADecoder());
		break;
	case kImageTypeIFF:
		decoder.reset(new Image::IFFDecoder());
		break;
	case kImageTypeBRC:
		decoder.reset(new Image::BRCDecoder());
		break;
	case kImageTypeBMP:
		decoder.reset(new Image::BitmapDecoder());
		break;
	case kImageTypeJPEG:
		decoder.reset(new Image::JPEGDecoder());
		break;
	default:
		warning("Surface::getImageInfo(): Unhandled image type: %d", (int)type);
		return false;
	}

	decoder->loadStream(stream);
	const Graphics::Surface *surf = decoder->getSurface();
	if (!surf)
		return false;

	width  = surf->w;
	height = surf->h;
	bpp    = surf->format.bytesPerPixel;

	return true;
}

bool Surface::loadImage(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom,
						int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	ImageType type = identifyImage(stream);
	if (type == kImageTypeNone)
		return false;

	return loadImage(stream, type, left, top, right, bottom, x, y, transp, format);
}

bool Surface::loadImage(Common::SeekableReadStream &stream, ImageType type, int16 left, int16 top, int16 right, int16 bottom,
						int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	if (type == kImageTypeNone)
		return false;

	switch (type) {
	case kImageTypeTGA:
		return loadTGA(stream, left, top, right, bottom, x, y, transp, format);
	case kImageTypeIFF:
		return loadIFF(stream, left, top, right, bottom, x, y, transp, format);
	case kImageTypeBRC:
		return loadBRC(stream, left, top, right, bottom, x, y, transp, format);
	case kImageTypeBMP:
		return loadBMP(stream, left, top, right, bottom, x, y, transp, format);
	case kImageTypeJPEG:
		return loadJPEG(stream, left, top, right, bottom, x, y, transp, format);

	default:
		warning("Surface::loadImage(): Unknown image type: %d", (int)type);
		return false;
	}

	return false;
}

ImageType Surface::identifyImage(Common::SeekableReadStream &stream) {
	uint32 startPos = stream.pos();

	if ((stream.size() - startPos) < 17)
		return kImageTypeNone;

	char buffer[10];
	if (!stream.read(buffer, 10))
		return kImageTypeNone;

	stream.seek(startPos);

	if (!strncmp(buffer    , "FORM", 4))
		return kImageTypeIFF;
	if (!strncmp(buffer + 6, "JFIF", 4))
		return kImageTypeJPEG;
	if (!strncmp(buffer    , "BRC" , 3))
		return kImageTypeBRC;
	if (!strncmp(buffer    , "BM"  , 2))
		return kImageTypeBMP;

	// Try to determine if it's maybe a TGA

	stream.skip(12);
	uint16 width  = stream.readUint16LE();
	uint16 height = stream.readUint16LE();
	uint8  bpp    = stream.readByte();

	// Check width, height and bpp for sane values
	if ((width == 0) || (height == 0) || (bpp == 0))
		return kImageTypeNone;
	if ((bpp != 8) && (bpp != 16) && (bpp != 24) && (bpp != 32))
		return kImageTypeNone;

	// This might be a TGA
	return kImageTypeTGA;
}

bool Surface::loadImage(Image::ImageDecoder &decoder, Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom,
						int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	decoder.loadStream(stream);

	if (!decoder.getSurface())
		return false;

	const Graphics::Surface *st = decoder.getSurface();
	bool needConversion = decoder.getSurface()->format.bytesPerPixel > 1 && decoder.getSurface()->format != format;
	if (needConversion) {
		Graphics::Surface *st2 = st->convertTo(format);
		if (format.aBits() > 0) {
			// Force the pixel value 0 (used by the engine as the special transparent color) to be still mapped to 0 in the new format
			for (int16 x2 = 0; x2 < st->w; ++x2) {
				for (int16 y2 = 0; y2 < st->h; ++y2) {
					uint32 p = st->getPixel(x2, y2);
					if (p == 0) {
						st2->setPixel(x2, y2, 0);
					}
				}
			}
		}

		st = st2;
	}

	uint32 *colorMap = nullptr;
	if (format.bytesPerPixel > 1 && decoder.getSurface()->format.bytesPerPixel == 1) {
		colorMap = new uint32[256];
		computeHighColorMap(colorMap, decoder.getPalette().data(), format, true);
	}

	const Surface src(st->w, st->h, st->format.bytesPerPixel,
					  static_cast<const byte *>(st->getPixels()),
					  colorMap);

	blit(src, left, top, right, bottom, x, y, (transp == 0) ? -1 : 0);
	if (colorMap) {
		if (_bpp == 1) {
			// The destination must retain the source color map for further conversions
			_highColorMap = colorMap;
			_ownHighColorMap = true;
		} else {
			delete[] colorMap;
		}
	}

	if (needConversion)
		delete st;

	return true;
}

bool Surface::loadTGA(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	Image::TGADecoder decoder;
	return loadImage(decoder, stream, left, top, right, bottom, x, y, transp, format);
}

bool Surface::loadIFF(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	Image::IFFDecoder decoder;
	return loadImage(decoder, stream, left, top, right, bottom, x, y, transp, format);
}

bool Surface::loadBRC(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	Image::BRCDecoder decoder;
	return loadImage(decoder, stream, left, top, right, bottom, x, y, transp, format);
}

bool Surface::loadBMP(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	Image::BitmapDecoder decoder;
	return loadImage(decoder, stream, left, top, right, bottom, x, y, transp, format);
}

bool Surface::loadJPEG(Common::SeekableReadStream &stream, int16 left, int16 top, int16 right, int16 bottom, int16 x, int16 y, int16 transp, Graphics::PixelFormat format) {
	Image::JPEGDecoder decoder;
	return loadImage(decoder, stream, left, top, right, bottom, x, y, transp, format);
}

void Surface::computeHighColorMap(uint32 *highColorMap, const byte *palette,
								  const Graphics::PixelFormat &format,
								  bool useSpecialBlackWhiteValues,
								  int16 startColor, int16 colorCount,
								  int16 startColorSrc) {
	if (!palette)
		return;

	if (startColorSrc < 0)
		startColorSrc = startColor;

	for (int16 i = 0; i < colorCount; i++) {
		int indexSrc = (startColorSrc + i ) * 3;
		byte red = palette[indexSrc];
		byte green = palette[indexSrc + 1];
		byte blue = palette[indexSrc + 2];

		int indexDest = startColor + i;

		if (!useSpecialBlackWhiteValues)
			highColorMap[indexDest] = format.RGBToColor(red, green, blue);
		else if (i == 0) // Trick from the original engine to handle transparency with high color surfaces
			highColorMap[indexDest] = 0; // Palette index 0 is always mapped to high color value 0, possibly interpreted as the special transparent color.
		else if (i == 255)
			highColorMap[indexDest] = format.RGBToColor(0xFF, 0xFF, 0xFF); // Palette index 255 is always mapped to white.
		else if (red == 0 && green == 0 && blue == 0)
			highColorMap[indexDest] = format.RGBToColor((1 << format.rLoss),
														(1 << format.gLoss),
														(1 << format.bLoss)); // Blacks at other indexes are mapped to rgb(1, 1, 1) value to prevent interpreting them as transparent.
		else
			highColorMap[indexDest] = format.RGBToColor(red, green, blue);
	}
}

} // End of namespace Gob
