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

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "freescape/gfx_opengl_texture.h"

#include "graphics/opengl/context.h"

namespace Freescape {

// From Bit Twiddling Hacks
static uint32 upperPowerOfTwo(uint32 v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

OpenGLTexture::OpenGLTexture() :
		_internalFormat(0),
		_sourceFormat(0),
		_internalWidth(0),
		_internalHeight(0),
		_upsideDown(false) {
	glGenTextures(1, &_id);
}

OpenGLTexture::OpenGLTexture(const Graphics::Surface *surface) {
	_width = surface->w;
	_height = surface->h;
	_format = surface->format;
	_upsideDown = false;

	// Pad the textures if non power of two support is unavailable
	if (OpenGLContext.NPOTSupported) {
		_internalHeight = _height;
		_internalWidth = _width;
	} else {
		_internalHeight = upperPowerOfTwo(_height);
		_internalWidth = upperPowerOfTwo(_width);
	}

	if (_format.bytesPerPixel == 4) {
		assert(surface->format == getRGBAPixelFormat());
		_format = surface->format;
		_internalFormat = GL_RGBA;
		_sourceFormat = GL_UNSIGNED_BYTE;
	} else if (_format.bytesPerPixel == 2) {
		_internalFormat = GL_RGB;
		_sourceFormat = GL_UNSIGNED_SHORT_5_6_5;
	} else
		error("Unknown pixel format");

	glGenTextures(1, &_id);
	glBindTexture(GL_TEXTURE_2D, _id);
	glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _internalWidth, _internalHeight, 0, _internalFormat, _sourceFormat, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// TODO: If non power of two textures are unavailable this clamping
	// has no effect on the padded sides (resulting in white lines on the edges)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	update(surface);
}

OpenGLTexture::~OpenGLTexture() {
	glDeleteTextures(1, &_id);
}

void OpenGLTexture::update(const Graphics::Surface *surface) {
	updatePartial(surface, Common::Rect(surface->w, surface->h));
}

void OpenGLTexture::updateTexture(const Graphics::Surface *surface, const Common::Rect &rect) {
	assert(surface->format == _format);

	glBindTexture(GL_TEXTURE_2D, _id);
	glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, surface->w, surface->h, 0, _internalFormat, _sourceFormat, const_cast<void *>(surface->getPixels()));
}

void OpenGLTexture::updatePartial(const Graphics::Surface *surface, const Common::Rect &rect) {
	updateTexture(surface, rect);
}

} // End of namespace Freescape

#endif
