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

#if defined(__ANDROID__)

#include "base/main.h"
#include "graphics/surface.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

#include "backends/platform/android/texture.h"
#include "backends/platform/android/android.h"

// Unfortunately, Android devices are too varied to make broad assumptions :/
#define TEXSUBIMAGE_IS_EXPENSIVE 0

// Supported GL extensions
static bool npot_supported = false;
#ifdef GL_OES_draw_texture
static bool draw_tex_supported = false;
#endif

static inline GLfixed xdiv(int numerator, int denominator) {
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

void GLESTexture::initGLExtensions() {
	const char *ext_string =
		reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

	LOGI("Extensions: %s", ext_string);

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

GLESTexture::GLESTexture(byte bytesPerPixel, GLenum glFormat, GLenum glType,
		size_t paletteSize) :
	_bytesPerPixel(bytesPerPixel),
	_glFormat(glFormat),
	_glType(glType),
	_paletteSize(paletteSize),
	_texture_width(0),
	_texture_height(0),
	_all_dirty(true)
{
	GLCALL(glGenTextures(1, &_texture_name));

	// This all gets reset later in allocBuffer:
	_surface.w = 0;
	_surface.h = 0;
	_surface.pitch = 0;
	_surface.pixels = 0;
	_surface.bytesPerPixel = 0;
}

GLESTexture::~GLESTexture() {
	release();
}

void GLESTexture::release() {
	debug("Destroying texture %u", _texture_name);
	GLCALL(glDeleteTextures(1, &_texture_name));
}

void GLESTexture::reinit() {
	GLCALL(glGenTextures(1, &_texture_name));

	if (_paletteSize) {
		// paletted textures are in a local buffer, don't wipe it
		initSize();
	} else {
		// bypass allocBuffer() shortcut to reinit the texture properly
		_texture_width = 0;
		_texture_height = 0;

		allocBuffer(_surface.w, _surface.h);
	}

	setDirty();
}

void GLESTexture::initSize() {
	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, _glFormat,
						_texture_width, _texture_height,
						0, _glFormat, _glType, 0));
}

void GLESTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = _bytesPerPixel;

	// Already allocated a sufficiently large buffer?
	if (w <= _texture_width && h <= _texture_height)
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}

	_surface.pitch = _texture_width * _bytesPerPixel;

	initSize();
}

void GLESTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
								const void *buf, int pitch) {
	ENTER("%u, %u, %u, %u, %p, %d", x, y, w, h, buf, pitch);

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	if (static_cast<int>(w) * _bytesPerPixel == pitch) {
		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
								_glFormat, _glType, buf));
	} else {
		// GLES removed the ability to specify pitch, so we
		// have to do this ourselves.
#if TEXSUBIMAGE_IS_EXPENSIVE
		byte *tmp = new byte[w * h * _bytesPerPixel];
		assert(tmp);

		const byte *src = static_cast<const byte *>(buf);
		byte *dst = tmp;
		GLuint count = h;

		do {
			memcpy(dst, src, w * _bytesPerPixel);
			dst += w * _bytesPerPixel;
			src += pitch;
		} while (--count);

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
								_glFormat, _glType, tmp));

		delete[] tmp;
#else
		// This version avoids the intermediate copy at the expense of
		// repeat glTexSubImage2D calls.  On some devices this is worse.
		const byte *src = static_cast<const byte *>(buf);
		do {
			GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
									w, 1, _glFormat, _glType, src));
			++y;
			src += pitch;
		} while (--h);
#endif
	}
}

void GLESTexture::fillBuffer(byte x) {
	uint rowbytes = _surface.w * _bytesPerPixel;

	byte *tmp = new byte[_surface.h * rowbytes];
	assert(tmp);

	memset(tmp, x, _surface.h * rowbytes);
	updateBuffer(0, 0, _surface.w, _surface.h, tmp, rowbytes);

	delete[] tmp;
}

void GLESTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

#ifdef GL_OES_draw_texture
	// Great extension, but only works under specific conditions.
	// Still a work-in-progress - disabled for now.
	if (false && draw_tex_supported && _paletteSize == 0) {
		//GLCALL(glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE));
		const GLint crop[4] = { 0, _surface.h, _surface.w, -_surface.h };

		GLCALL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop));

		// Android GLES bug?
		GLCALL(glColor4ub(0xff, 0xff, 0xff, 0xff));

		GLCALL(glDrawTexiOES(x, y, 0, w, h));
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

		GLCALL(glTexCoordPointer(2, GL_FIXED, 0, texcoords));

		const GLshort vertices[] = {
			x, y,
			x + w, y,
			x, y + h,
			x + w, y + h,
		};

		GLCALL(glVertexPointer(2, GL_SHORT, 0, vertices));

		assert(ARRAYSIZE(vertices) == ARRAYSIZE(texcoords));
		GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, ARRAYSIZE(vertices) / 2));
	}

	_all_dirty = false;
	_dirty_rect = Common::Rect();
}

GLES4444Texture::GLES4444Texture() :
	GLESTexture(2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 0) {
}

GLES4444Texture::~GLES4444Texture() {
}

GLES565Texture::GLES565Texture() :
	GLESTexture(2, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0) {
}

GLES565Texture::~GLES565Texture() {
}

GLESPaletteTexture::GLESPaletteTexture(byte bytesPerPixel, GLenum glFormat,
										GLenum glType, size_t paletteSize) :
	GLESTexture(bytesPerPixel, glFormat, glType, paletteSize),
	_texture(0)
{
}

GLESPaletteTexture::~GLESPaletteTexture() {
	delete[] _texture;
}

void GLESPaletteTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = _bytesPerPixel;

	// Already allocated a sufficiently large buffer?
	if (w <= _texture_width && h <= _texture_height)
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}
	_surface.pitch = _texture_width * _bytesPerPixel;

	// Texture gets uploaded later (from drawTexture())

	byte *new_buffer = new byte[_paletteSize +
		_texture_width * _texture_height * _bytesPerPixel];
	if (_texture) {
		// preserve palette
		memcpy(new_buffer, _texture, _paletteSize);
		delete[] _texture;
	}

	_texture = new_buffer;
	_surface.pixels = _texture + _paletteSize;
}

void GLESPaletteTexture::fillBuffer(byte x) {
	assert(_surface.pixels);
	memset(_surface.pixels, x, _surface.pitch * _surface.h);
	setDirty();
}

void GLESPaletteTexture::updateBuffer(GLuint x, GLuint y,
										GLuint w, GLuint h,
										const void *buf, int pitch) {
	_all_dirty = true;

	const byte * src = static_cast<const byte *>(buf);
	byte *dst = static_cast<byte *>(_surface.getBasePtr(x, y));

	do {
		memcpy(dst, src, w * _bytesPerPixel);
		dst += _surface.pitch;
		src += pitch;
	} while (--h);
}

void GLESPaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w,
										GLshort h) {
	if (_all_dirty) {
		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
								GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
								GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
								GL_CLAMP_TO_EDGE));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
								GL_CLAMP_TO_EDGE));

		const size_t texture_size =
			_paletteSize + _texture_width * _texture_height * _bytesPerPixel;

		GLCALL(glCompressedTexImage2D(GL_TEXTURE_2D, 0, _glType,
										_texture_width, _texture_height,
										0, texture_size, _texture));

		_all_dirty = false;
	}

	GLESTexture::drawTexture(x, y, w, h);
}

GLESPalette888Texture::GLESPalette888Texture() :
	GLESPaletteTexture(1, GL_RGB, GL_PALETTE8_RGB8_OES, 256 * 3) {
}

GLESPalette888Texture::~GLESPalette888Texture() {
}

GLESPalette8888Texture::GLESPalette8888Texture() :
	GLESPaletteTexture(1, GL_RGBA, GL_PALETTE8_RGBA8_OES, 256 * 4) {
}

GLESPalette8888Texture::~GLESPalette8888Texture() {
}

#endif

