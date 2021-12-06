/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/gfx/tinyglbitmap.h"

#include "engines/stark/gfx/driver.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

TinyGlBitmap::TinyGlBitmap() :
	Texture() {
	_blitImage = tglGenBlitImage();
}

TinyGlBitmap::~TinyGlBitmap() {
	tglDeleteBlitImage(_blitImage);
}

void TinyGlBitmap::bind() const {
}

void TinyGlBitmap::updateLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	_width = surface->w;
	_height = surface->h;

	if (surface->format.bytesPerPixel != 4) {
		// Convert the surface to texture format
		Graphics::Surface *convertedSurface = surface->convertTo(Driver::getRGBAPixelFormat(), palette);
		tglUploadBlitImage(_blitImage, *convertedSurface, 0, false);
		convertedSurface->free();
		delete convertedSurface;
	} else {
		assert(surface->format == Driver::getRGBAPixelFormat());
		tglUploadBlitImage(_blitImage, *surface, 0, false);
	}
}

void TinyGlBitmap::update(const Graphics::Surface *surface, const byte *palette) {
	updateLevel(0, surface, palette);
}

void TinyGlBitmap::setSamplingFilter(Texture::SamplingFilter filter) {
}

void TinyGlBitmap::setLevelCount(uint32 count) {
}

void TinyGlBitmap::addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
}

TinyGL::BlitImage *TinyGlBitmap::getBlitTexture() const {
	return _blitImage;
}

} // End of namespace Gfx
} // End of namespace Stark
