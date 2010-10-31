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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/null/null.cpp $
 * $Id: null.cpp 34912 2008-11-06 15:02:50Z fingolfin $
 *
 */

#include "base/main.h"
#include "graphics/surface.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <android/log.h>

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

#include "backends/platform/android/video.h"

// Unfortunately, Android devices are too varied to make broad assumptions :/
#define TEXSUBIMAGE_IS_EXPENSIVE 0

#undef LOG_TAG
#define LOG_TAG "ScummVM-video"

#if 0
#define ENTER(args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, args)
#else
#define ENTER(args...) /**/
#endif

#if 0
#define CHECK_GL_ERROR() checkGlError(__FILE__, __LINE__)
static const char* getGlErrStr(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:		   return "GL_NO_ERROR";
	case GL_INVALID_ENUM:	   return "GL_INVALID_ENUM";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:	   return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:	   return "GL_OUT_OF_MEMORY";
	}

	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code 0x%x)", error);
	return buf;
}
static void checkGlError(const char* file, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		warning("%s:%d: GL error: %s", file, line, getGlErrStr(error));
}
#else
#define CHECK_GL_ERROR() do {} while (false)
#endif

// Supported GL extensions
static bool npot_supported = false;
#ifdef GL_OES_draw_texture
static bool draw_tex_supported = false;
#endif

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1<<16));
	return (numerator << 16) / denominator;
}

template <class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;
	for (uint i = 1; i < sizeof(T)*CHAR_BIT; i <<= 1)
		k = k | k >> i;
	return k + 1;
}

void GLESTexture::initGLExtensions() {
	const char* ext_string =
		reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG,
				"Extensions: %s", ext_string);
	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
#ifdef GL_OES_draw_texture
		if (token == "GL_OES_draw_texture")
			draw_tex_supported = true;
#endif
	}
}

GLESTexture::GLESTexture() :
	_texture_width(0),
	_texture_height(0),
	_all_dirty(true)
{
	glGenTextures(1, &_texture_name);
	// This all gets reset later in allocBuffer:
	_surface.w = 0;
	_surface.h = 0;
	_surface.pitch = _texture_width;
	_surface.pixels = NULL;
	_surface.bytesPerPixel = 0;
}

GLESTexture::~GLESTexture() {
	debug("Destroying texture %u", _texture_name);
	glDeleteTextures(1, &_texture_name);
}

void GLESTexture::reinitGL() {
	glGenTextures(1, &_texture_name);
	setDirty();
}

void GLESTexture::allocBuffer(GLuint w, GLuint h) {
	CHECK_GL_ERROR();
	int bpp = bytesPerPixel();
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = bpp;

	if (w <= _texture_width && h <= _texture_height)
		// Already allocated a sufficiently large buffer
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}
	_surface.pitch = _texture_width * bpp;

	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, _texture_name);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERROR();
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat(),
		     _texture_width, _texture_height,
		     0, glFormat(), glType(), NULL);
	CHECK_GL_ERROR();
}

void GLESTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
							   const void* buf, int pitch) {
	ENTER("updateBuffer(%u, %u, %u, %u, %p, %d)", x, y, w, h, buf, pitch);
	glBindTexture(GL_TEXTURE_2D, _texture_name);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	setDirtyRect(Common::Rect(x, y, x+w, y+h));

	if (static_cast<int>(w) * bytesPerPixel() == pitch) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
						glFormat(), glType(), buf);
	} else {
		// GLES removed the ability to specify pitch, so we
		// have to do this ourselves.
		if (h == 0)
			return;

#if TEXSUBIMAGE_IS_EXPENSIVE
		byte tmpbuf[w * h * bytesPerPixel()];
		const byte* src = static_cast<const byte*>(buf);
		byte* dst = tmpbuf;
		GLuint count = h;
		do {
			memcpy(dst, src, w * bytesPerPixel());
			dst += w * bytesPerPixel();
			src += pitch;
		} while (--count);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
						glFormat(), glType(), tmpbuf);
#else
		// This version avoids the intermediate copy at the expense of
		// repeat glTexSubImage2D calls.  On some devices this is worse.
		const byte* src = static_cast<const byte*>(buf);
		do {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
							w, 1, glFormat(), glType(), src);
			++y;
			src += pitch;
		} while (--h);
#endif
	}
}

void GLESTexture::fillBuffer(byte x) {
	int rowbytes = _surface.w * bytesPerPixel();
	byte tmpbuf[_surface.h * rowbytes];
	memset(tmpbuf, x, _surface.h * rowbytes);
	updateBuffer(0, 0, _surface.w, _surface.h, tmpbuf, rowbytes);
}

void GLESTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	glBindTexture(GL_TEXTURE_2D, _texture_name);

#ifdef GL_OES_draw_texture
	// Great extension, but only works under specific conditions.
	// Still a work-in-progress - disabled for now.
	if (false && draw_tex_supported && paletteSize() == 0) {
		//glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		const GLint crop[4] = {0, _surface.h, _surface.w, -_surface.h};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
		glColor4ub(0xff, 0xff, 0xff, 0xff);   // Android GLES bug?
		glDrawTexiOES(x, y, 0, w, h);
	} else
#endif
	{
		const GLfixed tex_width = xdiv(_surface.w, _texture_width);
		const GLfixed tex_height = xdiv(_surface.h, _texture_height);
		const GLfixed texcoords[] = {
			0, 0,
			tex_width, 0,
			0, tex_height,
			tex_width, tex_height,
		};
		glTexCoordPointer(2, GL_FIXED, 0, texcoords);

		const GLshort vertices[] = {
			x,	 y,
			x+w, y,
			x,	 y+h,
			x+w, y+h,
		};
		glVertexPointer(2, GL_SHORT, 0, vertices);

		assert(ARRAYSIZE(vertices) == ARRAYSIZE(texcoords));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, ARRAYSIZE(vertices)/2);
	}

	_all_dirty = false;
	_dirty_rect = Common::Rect();
}

GLESPaletteTexture::GLESPaletteTexture() :
	GLESTexture(),
	_texture(NULL)
{
}

GLESPaletteTexture::~GLESPaletteTexture() {
	delete[] _texture;
}

void GLESPaletteTexture::allocBuffer(GLuint w, GLuint h) {
	CHECK_GL_ERROR();
	int bpp = bytesPerPixel();
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = bpp;

	if (w <= _texture_width && h <= _texture_height)
		// Already allocated a sufficiently large buffer
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}
	_surface.pitch = _texture_width * bpp;

	// Texture gets uploaded later (from drawTexture())

	byte* new_buffer = new byte[paletteSize() +
		_texture_width * _texture_height * bytesPerPixel()];
	if (_texture) {
		memcpy(new_buffer, _texture, paletteSize()); // preserve palette
		delete[] _texture;
	}
	_texture = new_buffer;
	_surface.pixels = _texture + paletteSize();
}

void GLESPaletteTexture::fillBuffer(byte x) {
	assert(_surface.pixels);
	memset(_surface.pixels, x, _surface.pitch * _surface.h);
	setDirty();
}

void GLESPaletteTexture::updateBuffer(GLuint x, GLuint y,
									  GLuint w, GLuint h,
									  const void* buf, int pitch) {
	_all_dirty = true;

	const byte* src = static_cast<const byte*>(buf);
	byte* dst = static_cast<byte*>(_surface.getBasePtr(x, y));
	do {
		memcpy(dst, src, w * bytesPerPixel());
		dst += _surface.pitch;
		src += pitch;
	} while (--h);
}

void GLESPaletteTexture::uploadTexture() const {
	const size_t texture_size =
		paletteSize() + _texture_width * _texture_height * bytesPerPixel();
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, glType(),
						   _texture_width, _texture_height,
						   0, texture_size, _texture);
	CHECK_GL_ERROR();
}

void GLESPaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	if (_all_dirty) {
		glBindTexture(GL_TEXTURE_2D, _texture_name);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		CHECK_GL_ERROR();
		uploadTexture();
		_all_dirty = false;
	}

	GLESTexture::drawTexture(x, y, w, h);
}
