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

	// Original supported other pixel depths
	_bpp = surface->getPixelDepth();
	assert(_bpp == 2);
	_pixelsPtr = surface->getPixels();

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
	case SA_MODE3:
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
	setColor(_color);
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

} // End of namespace Titanic
