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

#include "graphics/tinygl/zblit.h"

#include "freescape/gfx_tinygl_texture.h"

namespace Freescape {

const Graphics::PixelFormat TinyGLTexture::getRGBAPixelFormat() {
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
}

TinyGLTexture::TinyGLTexture(const Graphics::Surface *surface) {
	_width = surface->w;
	_height = surface->h;
	_format = surface->format;
	_internalFormat = 0;
	_sourceFormat = 0;

	_blitImage = tglGenBlitImage();

	update(surface);
}

TinyGLTexture::~TinyGLTexture() {
	tglDeleteBlitImage(_blitImage);
}

void TinyGLTexture::update(const Graphics::Surface *surface) {
	uint32 keyColor = getRGBAPixelFormat().RGBToColor(0xA0, 0xA0, 0xA0);
	tglUploadBlitImage(_blitImage, *surface, keyColor, true);
}

void TinyGLTexture::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	// FIXME: TinyGL does not support partial texture update
	update(surface);
}

TinyGL::BlitImage *TinyGLTexture::getBlitTexture() const {
	return _blitImage;
}

} // End of namespace Freescape
