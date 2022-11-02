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

#include "engines/myst3/gfx_tinygl_texture.h"

#include "graphics/tinygl/tinygl.h"

namespace Myst3 {

TinyGLTexture2D::TinyGLTexture2D(const Graphics::Surface *surface) {
	width = surface->w;
	height = surface->h;
	format = surface->format;

	_blitImage = tglGenBlitImage();

	update(surface);
}

TinyGLTexture2D::~TinyGLTexture2D() {
	tglDeleteBlitImage(_blitImage);
}

void TinyGLTexture2D::update(const Graphics::Surface *surface) {
	tglUploadBlitImage(_blitImage, *surface, 0, false);
}

void TinyGLTexture2D::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	// FIXME: TinyGL does not support partial texture update
	update(surface);
}

TinyGL::BlitImage *TinyGLTexture2D::getBlitTexture() const {
	return _blitImage;
}

TinyGLTexture3D::TinyGLTexture3D(const Graphics::Surface *surface) {
	width = surface->w;
	height = surface->h;
	format = surface->format;

	if (format.bytesPerPixel == 4) {
		internalFormat = TGL_RGBA;
		sourceFormat = TGL_UNSIGNED_BYTE;
	} else if (format.bytesPerPixel == 2) {
		internalFormat = TGL_RGB;
		sourceFormat = TGL_UNSIGNED_SHORT_5_6_5;
	} else
		error("Unknown pixel format");

	tglGenTextures(1, &id);
	tglBindTexture(TGL_TEXTURE_2D, id);
	tglTexImage2D(TGL_TEXTURE_2D, 0, internalFormat, width, height, 0, internalFormat, sourceFormat, nullptr);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_LINEAR);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_LINEAR);

	// NOTE: TinyGL doesn't have issues with white lines so doesn't need use TGL_CLAMP_TO_EDGE
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_REPEAT);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_REPEAT);
	update(surface);
}

TinyGLTexture3D::~TinyGLTexture3D() {
	tglDeleteTextures(1, &id);
}

void TinyGLTexture3D::update(const Graphics::Surface *surface) {
	tglBindTexture(TGL_TEXTURE_2D, id);
	tglTexImage2D(TGL_TEXTURE_2D, 0, internalFormat, width, height, 0,
	              internalFormat, sourceFormat, const_cast<void *>(surface->getPixels())); // TESTME: Not sure if it works.
}

void TinyGLTexture3D::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	// FIXME: TinyGL does not support partial texture update
	update(surface);
}

} // End of namespace Myst3
