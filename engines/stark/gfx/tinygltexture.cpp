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

#include "engines/stark/gfx/tinygltexture.h"
#include "engines/stark/gfx/driver.h"

#include "graphics/surface.h"

namespace Stark {
namespace Gfx {

TinyGlTexture::TinyGlTexture() :
	Texture(),
	_id(0),
	_levelCount(0) {
	tglGenTextures(1, &_id);

	bind();

	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);

	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP_TO_EDGE);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP_TO_EDGE);
}

TinyGlTexture::~TinyGlTexture() {
	tglDeleteTextures(1, &_id);
}

void TinyGlTexture::bind() const {
	tglBindTexture(TGL_TEXTURE_2D, _id);
}

void TinyGlTexture::updateLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	if (level == 0) {
		_width = surface->w;
		_height = surface->h;
	}

	if (surface->format.bytesPerPixel != 4) {
		// Convert the surface to texture format
		Graphics::Surface *convertedSurface = surface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), palette);

		tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, convertedSurface->w, convertedSurface->h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, (char *)(convertedSurface->getPixels()));

		convertedSurface->free();
		delete convertedSurface;
	} else {
		if (surface->format != Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) {
			// Convert the surface to texture format
			Graphics::Surface *convertedSurface = surface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

			tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, convertedSurface->w, convertedSurface->h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, (char *)(convertedSurface->getPixels()));

			convertedSurface->free();
			delete convertedSurface;
		} else {
			tglTexImage2D(TGL_TEXTURE_2D, 0, TGL_RGBA, surface->w, surface->h, 0, TGL_RGBA, TGL_UNSIGNED_BYTE, const_cast<void *>(surface->getPixels()));
		}
	}
}

void TinyGlTexture::update(const Graphics::Surface *surface, const byte *palette) {
	bind();
	updateLevel(0, surface, palette);
}

void TinyGlTexture::setSamplingFilter(Texture::SamplingFilter filter) {
	assert(_levelCount == 0);

	switch (filter) {
	case kNearest:
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);
		break;
	case kLinear:
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);
		break;
	default:
		warning("Unhandled sampling filter %d", filter);
	}
}

void TinyGlTexture::setLevelCount(uint32 count) {
	_levelCount = count;

	if (count >= 1) {
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_MIRRORED_REPEAT);
		tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_MIRRORED_REPEAT);
	}
}

void TinyGlTexture::addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	assert(level < _levelCount);

	if (level == 0) {
		updateLevel(level, surface, palette);
	}
}

} // End of namespace Gfx
} // End of namespace Stark
