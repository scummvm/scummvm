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

#include "engines/stark/gfx/opengltexture.h"

#include "engines/stark/gfx/driver.h"

#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

namespace Stark {
namespace Gfx {

OpenGlTexture::OpenGlTexture() :
	Texture(),
	_id(0),
	_levelCount(0) {
	glGenTextures(1, &_id);

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

OpenGlTexture::~OpenGlTexture() {
	glDeleteTextures(1, &_id);
}

void OpenGlTexture::bind() const {
	glBindTexture(GL_TEXTURE_2D, _id);
}

void OpenGlTexture::updateLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	if (level == 0) {
		_width = surface->w;
		_height = surface->h;
	}

	if (surface->format.bytesPerPixel != 4) {
		// Convert the surface to texture format
		Graphics::Surface *convertedSurface = surface->convertTo(Driver::getRGBAPixelFormat(), palette);

		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, convertedSurface->w, convertedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedSurface->getPixels());

		convertedSurface->free();
		delete convertedSurface;
	} else {
		assert(surface->format == Driver::getRGBAPixelFormat());

		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	}
}

void OpenGlTexture::update(const Graphics::Surface *surface, const byte *palette) {
	bind();
	updateLevel(0, surface, palette);
}

void OpenGlTexture::setSamplingFilter(Texture::SamplingFilter filter) {
	assert(_levelCount == 0);

	switch (filter) {
	case kNearest:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case kLinear:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		warning("Unhandled sampling filter %d", filter);
	}
}

void OpenGlTexture::setLevelCount(uint32 count) {
	_levelCount = count;

	if (count >= 1) {
#if !defined(USE_GLES2)
		// GLES2 does not allow setting the max provided mipmap level.
		// It expects all the levels to be provided, which is not the case in TLJ.
		// FIXME: Enable mipmapping on GLES2
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	}
}

void OpenGlTexture::addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	assert(level < _levelCount);

#if defined(USE_GLES2)
	if (level == 0)
#endif
	{
		updateLevel(level, surface, palette);
	}
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME) || defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
