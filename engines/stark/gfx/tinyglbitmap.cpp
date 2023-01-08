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

#include "engines/stark/gfx/tinyglbitmap.h"
#include "engines/stark/gfx/driver.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

TinyGlBitmap::TinyGlBitmap() :
		Bitmap(),
		 _1x1Color(0) {
	_blitImage = tglGenBlitImage();
}

TinyGlBitmap::~TinyGlBitmap() {
	tglDeleteBlitImage(_blitImage);
}

void TinyGlBitmap::bind() const {
}

void TinyGlBitmap::update(const Graphics::Surface *surface, const byte *palette) {
	_width = surface->w;
	_height = surface->h;

	if (surface->format.bytesPerPixel != 4) {
		// Convert the surface to bitmap format
		Graphics::Surface *convertedSurface = surface->convertTo(Driver::getRGBAPixelFormat(), palette);
		tglUploadBlitImage(_blitImage, *convertedSurface, 0, false);
		convertedSurface->free();
		delete convertedSurface;
	} else {
		assert(surface->format == Driver::getRGBAPixelFormat());
		// W/A for 1x1 size bitmap
		// store pixel color used later fo creating scalled bitmap
		if (_width == 1 && _height == 1) {
			_1x1Color = surface->getPixel(0, 0);
		}
		tglUploadBlitImage(_blitImage, *surface, 0, false);
	}
}

void TinyGlBitmap::setSamplingFilter(Bitmap::SamplingFilter filter) {
}

TinyGL::BlitImage *TinyGlBitmap::getBlitImage() const {
	return _blitImage;
}

} // End of namespace Gfx
} // End of namespace Stark
