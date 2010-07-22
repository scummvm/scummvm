/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifdef USE_OPENGL

#include "backends/graphics/opengl/gltexture.h"
#include "backends/graphics/opengl/glerrorcheck.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

// Supported GL extensions
static bool npot_supported = false;

/*static inline GLint xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}*/

static GLuint nextHigher2(GLuint v) {
	if (v == 0)
		return 1;
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}

void GLTexture::initGLExtensions() {
	static bool inited = false;

	if (inited)
		return;

	const char* ext_string =
		reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	CHECK_GL_ERROR();
	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}
	inited = true;
}

GLTexture::GLTexture(byte bpp, GLenum format, GLenum type)
	:
	_bytesPerPixel(bpp),
	_glFormat(format),
	_glType(type),
	_textureWidth(0),
	_textureHeight(0),
	_realWidth(0),
	_realHeight(0),
	_refresh(false),
	_filter(GL_NEAREST) {

	// Generates the texture ID for GL
	glGenTextures(1, &_textureName); CHECK_GL_ERROR();

	// This all gets reset later in allocBuffer:
	_surface.w = 0;
	_surface.h = 0;
	_surface.pitch = 0;
	_surface.pixels = NULL;
	_surface.bytesPerPixel = 0;
}

GLTexture::~GLTexture() {
	glDeleteTextures(1, &_textureName); CHECK_GL_ERROR();
}

void GLTexture::refresh() {
	// Generates the texture ID for GL
	glGenTextures(1, &_textureName); CHECK_GL_ERROR();
	_refresh = true;
}

void GLTexture::allocBuffer(GLuint w, GLuint h) {
	_realWidth = w;
	_realHeight = h;
	
	if (w <= _textureWidth && h <= _textureHeight && !_refresh)
		// Already allocated a sufficiently large buffer
		return;

	if (npot_supported) {
		_textureWidth = w;
		_textureHeight = h;
	} else {
		_textureWidth = nextHigher2(w);
		_textureHeight = nextHigher2(h);
	}

	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECK_GL_ERROR();
	glTexImage2D(GL_TEXTURE_2D, 0, _glFormat,
		     _textureWidth, _textureHeight, 0, _glFormat, _glType, NULL); CHECK_GL_ERROR();

	if (_surface.w != _textureWidth || _surface.h != _textureHeight)
		_surface.create(_textureWidth, _textureHeight, _bytesPerPixel);
	else if (_refresh)
		updateBuffer(_surface.pixels, _surface.pitch, 0, 0, _surface.w, _surface.h);

	_refresh = false;
}

void GLTexture::updateBuffer(const void *buf, int pitch, GLuint x, GLuint y, GLuint w, GLuint h) {
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();

	if (static_cast<int>(w) * _bytesPerPixel == pitch) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
						_glFormat, _glType, buf); CHECK_GL_ERROR();
		if (buf != _surface.pixels)
			memcpy(_surface.getBasePtr(x, y), buf, h * pitch);
	} else {
		const byte* src = static_cast<const byte*>(buf);
		do {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
							w, 1, _glFormat, _glType, src); CHECK_GL_ERROR();
			if (buf != _surface.pixels)
				memcpy(_surface.getBasePtr(x, y), src, w * _bytesPerPixel);
			++y;
			src += pitch;
		} while (--h);
	}
}

void GLTexture::fillBuffer(byte x) {
	memset(_surface.pixels, x, _surface.h * _surface.pitch);
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _surface.w, _surface.h,
		_glFormat, _glType, _surface.pixels);  CHECK_GL_ERROR();
}

void GLTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();

	const GLfloat texWidth = (GLfloat)_realWidth / _textureWidth;//xdiv(_surface.w, _textureWidth);
	const GLfloat texHeight = (GLfloat)_realHeight / _textureHeight;//xdiv(_surface.h, _textureHeight);
	const GLfloat texcoords[] = {
		0, 0,
		texWidth, 0,
		0, texHeight,
		texWidth, texHeight,
	};
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords); CHECK_GL_ERROR();

	const GLshort vertices[] = {
		x,	   y,
		x + w, y,
		x,	   y + h,
		x + w, y + h,
	};
	glVertexPointer(2, GL_SHORT, 0, vertices); CHECK_GL_ERROR();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); CHECK_GL_ERROR();
}

#endif
