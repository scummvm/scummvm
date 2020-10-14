/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

#include "graphics/opengl/texture.h"
#include "graphics/opengl/context.h"

namespace OpenGL {

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

const Graphics::PixelFormat TextureGL::getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

const Graphics::PixelFormat TextureGL::get565PixelFormat() {
	return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
}

TextureGL::TextureGL(const Graphics::Surface &srf) :
		_managedTexture(true), _width(srf.w), _height(srf.h) {
	if (OpenGLContext.NPOTSupported) {
		_texWidth  = _width;
		_texHeight = _height;
	} else {
		_texWidth  = nextHigher2(_width);
		_texHeight = nextHigher2(_height);
	}

	const Graphics::Surface *surfaceToUpload = &srf;
	Graphics::Surface *convertedSurface = nullptr;
	Graphics::Surface *copiedSurface = nullptr;

	GLenum type;
	GLenum format;

	if (srf.format == getRGBAPixelFormat()) {
		type = GL_UNSIGNED_BYTE;
		format = GL_RGBA;
	} else if (srf.format == get565PixelFormat()) {
		type = GL_UNSIGNED_SHORT_5_6_5;
		format = GL_RGB;
	} else {
		type = GL_UNSIGNED_BYTE;
		format = GL_RGBA;
		convertedSurface = srf.convertTo(getRGBAPixelFormat());
		surfaceToUpload = convertedSurface;
	}

	if (surfaceToUpload->pitch != surfaceToUpload->w * surfaceToUpload->format.bytesPerPixel) {
		if (OpenGLContext.unpackSubImageSupported) {
			glPixelStorei(GL_UNPACK_ROW_LENGTH, surfaceToUpload->pitch / surfaceToUpload->format.bytesPerPixel);
		} else {
			// When unpack sub-image is not supported we can't specify the pitch
			// and have to copy the subimage
			copiedSurface = new Graphics::Surface();
			copiedSurface->copyFrom(*surfaceToUpload);
			surfaceToUpload = copiedSurface;
		}
	}

	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, _texWidth, _texHeight, 0, format, type, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, format, type, const_cast<void *>(surfaceToUpload->getPixels()));

	if (OpenGLContext.unpackSubImageSupported) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}

	if (convertedSurface) {
		convertedSurface->free();
		delete convertedSurface;
	}

	if (copiedSurface) {
		copiedSurface->free();
		delete copiedSurface;
	}
}

TextureGL::TextureGL(uint width, uint height) :
		_managedTexture(true), _width(width), _height(height) {
	if (OpenGLContext.NPOTSupported) {
		_texWidth  = _width;
		_texHeight = _height;
	} else {
		_texWidth  = nextHigher2(_width);
		_texHeight = nextHigher2(_height);
	}
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

TextureGL::TextureGL(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height) :
		_managedTexture(false), _texture(texture_name), _width(width), _height(height),
		_texWidth(texture_width), _texHeight(texture_height) {
}

TextureGL::~TextureGL() {
	if (_managedTexture) {
		glDeleteTextures(1, &_texture);
	}
}

} // End of namespace OpenGL

#endif
