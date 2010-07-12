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

#include "backends/graphics/opengl/gltexture.h"
#include "backends/graphics/opengl/glerrorcheck.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

// Supported GL extensions
static bool npot_supported = false;

static inline GLint xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

template <class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;
	for (uint i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1)
		k = k | k >> i;
	return k + 1;
}

void GLTexture::initGLExtensions() {
	const char* ext_string =
		reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}
}

GLTexture::GLTexture(byte bpp, GLenum format, GLenum type)
	:
	_bytesPerPixel(bpp),
	_glFormat(format),
	_glType(type),
	_textureWidth(0),
	_textureHeight(0) {

	refresh();

	// This all gets reset later in allocBuffer:
	_surface.w = 0;
	_surface.h = 0;
	_surface.pitch = 0;
	_surface.pixels = NULL;
	_surface.bytesPerPixel = 0;
}

GLTexture::~GLTexture() {
	debug("Destroying texture %u", _textureName);
	CHECK_GL_ERROR( glDeleteTextures(1, &_textureName) );
}

void GLTexture::refresh() {
	// Generates the texture ID for GL
	//CHECK_GL_ERROR( glGenTextures(1, &_textureName) );
	updateBuffer(_surface.pixels, _surface.bytesPerPixel, 0, 0, _surface.w, _surface.h);
}

void GLTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = _bytesPerPixel;

	if (w <= _textureWidth && h <= _textureHeight)
		// Already allocated a sufficiently large buffer
		return;

	if (npot_supported) {
		_textureWidth = w;
		_textureHeight = h;
	} else {
		_textureWidth = nextHigher2(w);
		_textureHeight = nextHigher2(h);
	}
	_surface.pitch = w * _bytesPerPixel;

	//_surface.create(w, h, _bytesPerPixel);

	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	CHECK_GL_ERROR( glBindTexture(GL_TEXTURE_2D, _textureName) );
	CHECK_GL_ERROR( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
	CHECK_GL_ERROR( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
	CHECK_GL_ERROR( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
	CHECK_GL_ERROR( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
	CHECK_GL_ERROR( glTexImage2D(GL_TEXTURE_2D, 0, _glFormat,
		     _textureWidth, _textureHeight,
		     0, _glFormat, _glType, NULL) );
}

void GLTexture::updateBuffer(const void *buf, int pitch, GLuint x, GLuint y, GLuint w, GLuint h) {
	CHECK_GL_ERROR( glBindTexture(GL_TEXTURE_2D, _textureName) );

	if (static_cast<int>(w) * _bytesPerPixel == pitch) {
		CHECK_GL_ERROR( glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
						_glFormat, _glType, buf) );
		//memcpy(_surface.pixels, buf, w * pitch);
	} else {
		// GLES removed the ability to specify pitch, so we
		// have to do this row by row.
		const byte* src = static_cast<const byte*>(buf);
		do {
			CHECK_GL_ERROR( glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
							w, 1, _glFormat, _glType, src) );
			//memcpy(_surface.pixels, src, pitch);
			++y;
			src += pitch;
		} while (--h);
	}
}

void GLTexture::fillBuffer(byte x) {
	byte* tmpbuf = new byte[_surface.h * _surface.w * _bytesPerPixel];
	memset(tmpbuf, 0, _surface.h * _surface.w * _bytesPerPixel);
	CHECK_GL_ERROR( glBindTexture(GL_TEXTURE_2D, _textureName) );
	CHECK_GL_ERROR( glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _surface.w, _surface.h,
					_glFormat, _glType, tmpbuf) );
	delete[] tmpbuf;
}

void GLTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	CHECK_GL_ERROR( glBindTexture(GL_TEXTURE_2D, _textureName) );

	const GLint tex_width = xdiv(_surface.w, _textureWidth);
	const GLint tex_height = xdiv(_surface.h, _textureHeight);
	const GLint texcoords[] = {
		0, 0,
		tex_width, 0,
		0, tex_height,
		tex_width, tex_height,
	};
	CHECK_GL_ERROR( glTexCoordPointer(2, GL_INT, 0, texcoords) );

	const GLshort vertices[] = {
		x,	   y,
		x + w, y,
		x,	   y + h,
		x + w, y + h,
	};
	CHECK_GL_ERROR( glVertexPointer(2, GL_SHORT, 0, vertices) );

	assert(ARRAYSIZE(vertices) == ARRAYSIZE(texcoords));
	CHECK_GL_ERROR( glDrawArrays(GL_TRIANGLE_STRIP, 0, ARRAYSIZE(vertices) / 2) );
}
