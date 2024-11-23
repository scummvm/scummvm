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

TinyGL3DTexture::TinyGL3DTexture(byte *stipple, uint32 c1, uint32 c2) {
	Graphics::Surface *surface = new Graphics::Surface();
	int width = 32;
	int height = 32;

	surface->create(width, height, getRGBAPixelFormat());
	surface->fillRect(Common::Rect(0, 0, width, height), surface->format.RGBToColor(0, 0, 0xFF));

	const int stippleWidth = 32;
	const int stippleHeight = 32;


	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {

			// Match OpenGL's stipple bit layout
			int stippleX = x % stippleWidth; // Bit position in the row
			int stippleY = y % stippleHeight; // Row index

			int byteIndex = stippleY * 4 + (stippleX / 8); // 4 bytes per row
			int bitIndex = stippleX % 8;

			byte bitmask = 1 << (7 - bitIndex);
			bool isForeground = stipple[byteIndex] & bitmask;
			//debug("stippleX=%d stippleY=%d byteIndex=%d bitIndex=%d stipple[byteIndex]: %x color: %s", stippleX, stippleY, byteIndex, bitIndex, stipple[byteIndex], isForeground ? "X" : " ");
			surface->setPixel(stippleX, stippleY, isForeground ? c1 : c2);
		}
	}
	//assert(0);
	Graphics::Surface *texture = new Graphics::Surface();
	texture->create(320, 200, getRGBAPixelFormat());
	texture->fillRect(Common::Rect(0, 0, 320, 200), texture->format.RGBToColor(0, 0, 0xFF));
	//texture->copyRectToSurface(*surface, 0, 0, Common::Rect(0, 0, 32, 32));

	// Replicate the stipple pattern to fill the entire texture
	for (int x = 0; x < 320; x += width) {
		for (int y = 0; y < 200; y += height) {
			if (x + width >= 320 || y + height >= 200) {
				texture->copyRectToSurface(*surface, x, y, Common::Rect(0, 0, MIN(width, 320 - x), MIN(height, 200 - y)));
			} else
				texture->copyRectToSurface(*surface, x, y, Common::Rect(0, 0, width, height));
		}
	}

	// This surface is no longer needed
	surface->free();
	delete surface;

	_width = texture->w;
	_height = texture->h;
	_format = texture->format;
	_upsideDown = false;

	if (_format.bytesPerPixel == 4) {
		assert(texture->format == getRGBAPixelFormat());
		_format = texture->format;
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

	update(texture);

	texture->free();
	delete texture;
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
	//tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_S, TGL_CLAMP_TO_EDGE);
	//tglTexParameteri(TGL_TEXTURE_2D, TGL_TEXTURE_WRAP_T, TGL_CLAMP_TO_EDGE);
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
