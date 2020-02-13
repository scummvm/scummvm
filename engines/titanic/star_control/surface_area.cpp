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
#include "graphics/primitives.h"

namespace Titanic {

CSurfaceArea::CSurfaceArea(CVideoSurface *surface) {
	_width = surface->getWidth();
	_height = surface->getHeight();
	_pitch = surface->getPitch();
	_field0 = 0;
	_colorMask = _color = 0;
	_mode = SA_SOLID;
	_surface = nullptr;

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
	_mode = SA_SOLID;
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

Graphics::PixelFormat CSurfaceArea::getPixelFormat() const {
	switch (_bpp) {
	case 1:
	case 2:
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	case 4:
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	default:
		return Graphics::PixelFormat::createFormatCLUT8();
	}
}

void CSurfaceArea::pixelToRGB(uint pixel, uint *rgb) {
	switch (_bpp) {
	case 0:
		*rgb = 0;
		break;

	case 1:
	case 2: {
		Graphics::PixelFormat pf = getPixelFormat();
		*rgb = pf.RGBToColor(pixel & 0xff, (pixel >> 8) & 0xff, (pixel >> 16) & 0xff);
		break;
	}

	case 3:
	case 4:
		*rgb = pixel;
		break;

	default:
		break;
	}
}

double CSurfaceArea::drawLine(const FPoint &pt1, const FPoint &pt2) {
	if (pt1 == pt2)
		return pt1._y;

	FPoint p1 = pt1, p2 = pt2;
	double xp = pt1._x, yp = pt1._y;

	int flags1 = (p1._x < _bounds.left ? 1 : 0)
		| (p1._x > _bounds.right ? 2 : 0)
		| (p1._y < _bounds.top ? 4 : 0)
		| (p1._y > _bounds.bottom ? 8 : 0);
	int flags2 = (p2._x < _bounds.left ? 1 : 0)
		| (p2._x > _bounds.right ? 2 : 0)
		| (p2._y < _bounds.top ? 4 : 0)
		| (p2._y > _bounds.bottom ? 8 : 0);

	// Handle any clipping
	while (flags1 | flags2) {
		if (flags1 & flags2)
			return p1._y;

		int flags = flags1 ? flags1 : flags2;
		if ((flags & 1) || (flags & 2)) {
			xp = (flags & 1) ? _bounds.left : _bounds.right;
			yp = (p2._y - p1._y) * (xp - p1._x) / (p2._x - p1._x) + p1._y;
		} else if ((flags & 4) || (flags & 8)) {
			yp = (flags & 4) ? _bounds.top : _bounds.bottom;
			xp = (p2._x - p1._x) * (yp - p1._y) / (p2._y - p1._y) + p1._x;
		}

		if (flags == flags1) {
			p1._x = xp;
			p1._y = yp;

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
			p2._x = xp;
			p2._y = yp;

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

	Common::Point srcPos((int)(p1._x - 0.5), (int)(p1._y - 0.5));
	Common::Point destPos((int)(p2._x - 0.5), (int)(p2._y - 0.5));

	Graphics::Surface s;
	s.setPixels(_pixelsPtr);
	s.pitch = _pitch;
	s.format = getPixelFormat();
	s.w = _width;
	s.h = _height;
	_surface = &s;

	switch (_bpp) {
	case 0:
		if (_mode != SA_SOLID) {
			Graphics::drawLine(srcPos.x, srcPos.y, destPos.x, destPos.y, 0, plotPoint<byte>, this);
			return p1._y;
		}
		break;
	case 1:
	case 2:
		if (_mode != SA_SOLID) {
			Graphics::drawLine(srcPos.x, srcPos.y, destPos.x, destPos.y, 0, plotPoint<uint16>, this);
			return p1._y;
		}
		break;
	case 4:
		if (_mode != SA_SOLID) {
			Graphics::drawLine(srcPos.x, srcPos.y, destPos.x, destPos.y, 0, plotPoint<uint32>, this);
			return p1._y;
		}
		break;
	default:
		error("Unknown bpp");
	}

	s.drawLine(srcPos.x, srcPos.y, destPos.x, destPos.y, _rgb);
	return p1._y;
}

} // End of namespace Titanic
