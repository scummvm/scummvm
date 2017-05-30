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

#include "titanic/star_control/surface_area.h"

namespace Titanic {

CSurfaceArea::CSurfaceArea(CVideoSurface *surface) {
	_width = surface->getWidth();
	_height = surface->getHeight();
	_pitch = surface->getPitch();
	_field0 = 0;
	_colorMask = _color = 0;
	_mode = SA_NONE;

	// Original supported other pixel depths
	_bpp = surface->getPixelDepth();
	_pixelsPtr = (byte *)surface->getPixels();
	assert(_bpp == 2 && _pixelsPtr);

	initialize();
}

void CSurfaceArea::initialize() {
	_bounds = Rect(0, 0, _width - 1, _height - 1);
	_centroid = FPoint(_width / 2.0, _height / 2.0);
	_pixel = 0xffffff;
	_field27 = _field26 = _field25 = 0;
	_field24 = 0;
	_rgb = _field2C = 0;
	_mode = SA_NONE;
}

void CSurfaceArea::setColor(uint rgb) {
	switch (_mode) {
	case SA_MODE1:
		_color = 0;
		_colorMask = rgb;
		break;
	case SA_MODE2:
		_color = rgb;
		_colorMask = ~rgb;
		break;
	case SA_XOR:
		_color = rgb;
		_colorMask = 0xFFFFFFFF;
		break;
	case SA_MODE4:
		_color = 0;
		_colorMask = ~rgb;
		break;
	default:
		break;
	}
}

SurfaceAreaMode CSurfaceArea::setMode(SurfaceAreaMode mode) {
	SurfaceAreaMode oldMode = _mode;
	_mode = mode;
	setColor(_rgb);
	return oldMode;
}

void CSurfaceArea::setColorFromPixel() {
	pixelToRGB(_pixel, &_rgb);
	setColor(_rgb);
}

void CSurfaceArea::pixelToRGB(uint pixel, uint *rgb) {
	switch (_bpp) {
	case 0:
		*rgb = 0;
		break;

	case 1:
	case 2: {
		uint r = pixel & 0xF8;
		uint g = (pixel >> 8) & 0xf8;
		uint b = ((pixel >> 16) & 0xff) >> 3;
		uint value = (((r << 5) | g) << 2) | b;
		value &= 0xffff;
		*rgb = (value << 16) | value;
		break;
	}

	case 3:
	case 4:
		*rgb = pixel;
		break;
	}
}

double CSurfaceArea::fillRect(const FRect &rect) {
	if (rect.empty())
		return rect.top;

	double xp = rect.left, yp = rect.top;
	FRect r = rect;

	// edx=flags1, esi=flags2
	int flags1 = (r.left < _bounds.left ? 1 : 0)
		| (r.left > _bounds.right ? 2 : 0)
		| (r.top < _bounds.top ? 4 : 0)
		| (r.top > _bounds.bottom ? 8 : 0);
	int flags2 = (r.right < _bounds.left ? 1 : 0)
		| (r.right > _bounds.right ? 2 : 0)
		| (r.bottom < _bounds.top ? 4 : 0)
		| (r.bottom > _bounds.bottom ? 8 : 0);

	// Handle any clipping
	while (flags1 | flags2) {
		if (flags1 & flags2)
			return r.top;

		int flags = flags1 ? flags1 : flags2;
		if ((flags & 1) || (flags & 2)) {
			xp = (flags & 1) ? _bounds.left : _bounds.right;
			yp = (r.bottom - r.top) * (xp - r.left) / (r.right - r.left) + r.top;
		} else if ((flags & 4) || (flags & 8)) {
			yp = (flags & 4) ? _bounds.top : _bounds.bottom;
			xp = (r.right - r.left) * (yp - r.top) / (r.bottom - r.top) + r.left;
		}

		if (flags == flags1) {
			r.left = xp;
			r.top = yp;

			flags1 = 0;
			if (xp < _bounds.left)
				flags1 |= 1;
			if (xp > _bounds.right)
				flags1 |= 2;
			if (yp < _bounds.top)
				flags1 |= 4;
			if (yp > _bounds.bottom)
				flags1 |= 8;
		} else {
			r.right = xp;
			r.bottom = yp;

			flags2 = 0;
			if (xp < _bounds.left)
				flags2 |= 1;
			if (xp > _bounds.right)
				flags2 |= 2;
			if (yp < _bounds.top)
				flags2 |= 4;
			if (yp > _bounds.bottom)
				flags2 |= 8;
		}
	}

	Common::Rect rr((int)(MIN(r.left, r.right) - 0.5), (int)(MIN(r.top, r.bottom) - 0.5),
		(int)(MAX(r.left, r.right) - 0.5) + 1, (int)(MAX(r.top, r.bottom) - 0.5) + 1);
	
	Graphics::Surface s;
	s.setPixels(_pixelsPtr);
	s.pitch = _pitch;
	s.w = _width;
	s.h = _height;

	switch (_bpp) {
	case 0:
		s.format = Graphics::PixelFormat::createFormatCLUT8();
		break;
	case 1:
	case 2:
		s.format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
		break;
	case 4:
		s.format = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		break;
	default:
		error("Unknown bpp");
	}

	// Fill area
	if (_mode == SA_NONE) {
		s.fillRect(rr, _rgb);
	} else {
		colorRect(s, rr, _colorMask, _color);
	}

	return r.top;
}

template<typename T>
static void colorRectFn(Graphics::Surface &s, const Common::Rect &r,
		uint andMask, uint xorMask) {
	for (int yp = r.top; yp < r.bottom; ++yp) {
		T *pixelP = (T *)s.getBasePtr(r.left, yp);
		for (int xp = r.left; xp < r.right; ++xp, ++pixelP)
			*pixelP = (*pixelP & andMask) ^ xorMask;
	}
}

void CSurfaceArea::colorRect(Graphics::Surface &s, const Common::Rect &r,
		uint andMask, uint xorMask) {
	switch (s.format.bytesPerPixel) {
	case 1:
		colorRectFn<byte>(s, r, andMask, xorMask);
		break;
	case 2:
		colorRectFn<uint16>(s, r, andMask, xorMask);
		break;
	default:
		colorRectFn<uint32>(s, r, andMask, xorMask);
		break;
	}
}

} // End of namespace Titanic
