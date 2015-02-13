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

#include "graphics/surface.h"

#include <SDL_opengl.h>

namespace Stark {
namespace Gfx {

OpenGlTexture::OpenGlTexture() :
	Texture(),
	_id(0) {
	glGenTextures(1, &_id);
}

OpenGlTexture::~OpenGlTexture() {
	glDeleteTextures(1, &_id);
}

void OpenGlTexture::bind() const {
	glBindTexture(GL_TEXTURE_2D, _id);
}

OpenGlMipMapTexture::OpenGlMipMapTexture() :
	OpenGlTexture(),
	MipMapTexture(),
	_levelCount(0) {

	bind();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

OpenGlMipMapTexture::~OpenGlMipMapTexture() {
}

void OpenGlMipMapTexture::setLevelCount(uint32 count) {
	_levelCount = count;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1);
}

void OpenGlMipMapTexture::addLevel(uint32 level, const Graphics::Surface *surface, const byte *palette) {
	assert(level < _levelCount);

	if (surface->format.bytesPerPixel != 4) {
		// Convert the surface to texture format
		Graphics::Surface *convertedSurface = surface->convertTo(Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), palette);

		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, convertedSurface->w, convertedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedSurface->getPixels());

		convertedSurface->free();
		delete convertedSurface;
	} else {
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	}
}

void OpenGlMipMapTexture::bind() const {
	OpenGlTexture::bind();
}

} // End of namespace Gfx
} // End of namespace Stark
