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

#include "engines/stark/gfx/openglbitmap.h"

#include "engines/stark/gfx/driver.h"

#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/context.h"

namespace Stark {
namespace Gfx {

OpenGlBitmap::OpenGlBitmap() :
	Bitmap(),
	_id(0) {
	glGenTextures(1, &_id);

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

OpenGlBitmap::~OpenGlBitmap() {
	glDeleteTextures(1, &_id);
}

void OpenGlBitmap::bind() const {
	glBindTexture(GL_TEXTURE_2D, _id);
}

void OpenGlBitmap::update(const Graphics::Surface *surface, const byte *palette) {
	bind();

	_width = surface->w;
	_height = surface->h;

	if (surface->format != Driver::getRGBAPixelFormat()) {
		// Convert the surface to texture format
		Graphics::Surface *convertedSurface = surface->convertTo(Driver::getRGBAPixelFormat(), palette);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, convertedSurface->w, convertedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedSurface->getPixels());

		convertedSurface->free();
		delete convertedSurface;
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->getPixels());
	}
}

void OpenGlBitmap::setSamplingFilter(Bitmap::SamplingFilter filter) {
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

Graphics::PixelFormat OpenGlBitmap::getBestPixelFormat() const {
	return Driver::getRGBAPixelFormat();
}

} // End of namespace Gfx
} // End of namespace Stark

#endif // defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
