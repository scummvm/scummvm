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

TinyGL2DTexture::TinyGL2DTexture(const Graphics::Surface *surface) {
	_width = surface->w;
	_height = surface->h;
	_format = surface->format;
	_internalFormat = 0;
	_sourceFormat = 0;

	_blitImage = tglGenBlitImage();

	update(surface);
}

TinyGL2DTexture::~TinyGL2DTexture() {
	tglDeleteBlitImage(_blitImage);
}

void TinyGL2DTexture::update(const Graphics::Surface *surface) {
	uint32 keyColor = getRGBAPixelFormat().RGBToColor(0xA0, 0xA0, 0xA0);
	tglUploadBlitImage(_blitImage, *surface, keyColor, true);
}

void TinyGL2DTexture::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	// FIXME: TinyGL does not support partial texture update
	update(surface);
}

TinyGL::BlitImage *TinyGL2DTexture::getBlitTexture() const {
	return _blitImage;
}

TinyGL3DTexture::TinyGL3DTexture(const Graphics::Surface *surface) {
	_width = surface->w;
	_height = surface->h;
	_format = surface->format;
	_upsideDown = false;

	if (_format.bytesPerPixel == 4) {
		assert(surface->format == getRGBAPixelFormat());
		_format = surface->format;
		_internalFormat = TGL_RGBA;
		_sourceFormat = TGL_UNSIGNED_BYTE;
	} else if (_format.bytesPerPixel == 2) {
		_internalFormat = TGL_RGB;
		_sourceFormat = TGL_UNSIGNED_SHORT_5_6_5;
	} else
		error("Unknown pixel format");


	tglGenTextures(1, &_id);
	tglBindTexture(TGL_TEXTURE_2D, _id);
	tglTexImage2D(TGL_TEXTURE_2D, 0, _internalFormat, _width, _height, 0, _internalFormat, _sourceFormat, nullptr);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MIN_FILTER, TGL_NEAREST);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_MAG_FILTER, TGL_NEAREST);

	// NOTE: TinyGL doesn't have issues with white lines so doesn't need use TGL_CLAMP_TO_EDGE
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP_TO_EDGE);
	tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP_TO_EDGE);
	update(surface);
}

TinyGL3DTexture::~TinyGL3DTexture() {
	tglDeleteTextures(1, &_id);
}

void TinyGL3DTexture::update(const Graphics::Surface *surface) {
	assert(surface->format == _format);

	tglBindTexture(TGL_TEXTURE_2D, _id);
	tglTexImage2D(TGL_TEXTURE_2D, 0, _internalFormat, surface->w, surface->h, 0, _internalFormat, _sourceFormat, const_cast<void *>(surface->getPixels()));
}

void TinyGL3DTexture::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	// FIXME: TinyGL does not support partial texture update
	update(surface);
}

} // End of namespace Freescape
