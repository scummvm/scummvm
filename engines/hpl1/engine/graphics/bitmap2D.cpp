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

#include "hpl1/engine/graphics/bitmap2D.h"
#include "hpl1/debug.h"
#include "common/rect.h"

namespace hpl {

static uint32 quantizeColor(const cColor &col, const Graphics::PixelFormat &format) {
	uint8 r = static_cast<uint8>(col.r * 255);
	uint8 g = static_cast<uint8>(col.g * 255);
	uint8 b = static_cast<uint8>(col.b * 255);
	uint8 a = static_cast<uint8>(col.a * 255);
	return format.ARGBToColor(a, r, g, b);
}

void Bitmap2D::drawToBitmap(Bitmap2D &dest, const cVector2l &pos) {
	if (_surface.format != dest._surface.format)
		error("call to Bitmap2D::drawToBitmap with different pixel formats"); 
	if (_surface.w + pos.x > dest._surface.w || _surface.h + pos.y > dest._surface.h)
		error("call to Bitmap2D::drawToBitmap would go out of bounds");

	dest._surface.copyRectToSurface(_surface, pos.x, pos.y, Common::Rect(0, 0, _surface.w, _surface.h)); 
}

bool Bitmap2D::create(const cVector2l &size, const Graphics::PixelFormat &format) {
	_surface.create(size.x, size.y, format);
	return true;
}

void Bitmap2D::fillRect(const cRect2l &rect, const cColor &color) {
	const uint32 qcol = quantizeColor(color, _surface.format);
	const Common::Rect surfaceRect(
		rect.x, 
		rect.y,
		rect.w <= 0 ? rect.x + _surface.w : rect.x + rect.w, 
		rect.h <= 0 ? rect.y + _surface.w : rect.y + rect.h
	);  
	_surface.fillRect(surfaceRect, qcol);
}

void *Bitmap2D::getRawData() {
	return _surface.getPixels();
}

int Bitmap2D::getNumChannels() {
	return _surface.format.bpp() / _surface.format.bytesPerPixel;
}

bool Bitmap2D::HasAlpha() {
	return _surface.format.aBits() > 0; 
}

Bitmap2D::~Bitmap2D() {
	_surface.free();
}


} // namespace hpl