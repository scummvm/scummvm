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

void GLESBaseTexture::initGLExtensions() {
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

GLESBaseTexture::GLESBaseTexture(GLenum glFormat, GLenum glType,
									Graphics::PixelFormat pixelFormat) :
	_glFormat(glFormat),
	_glType(glType),
	_glFilter(GL_NEAREST),
	_texture_name(0),
	_surface(),
	_texture_width(0),
	_texture_height(0),
	_draw_rect(),
	_all_dirty(false),
	_dirty_rect(),
	_pixelFormat(pixelFormat),
	_palettePixelFormat()
{
	GLCALL(glGenTextures(1, &_texture_name));
}

GLESBaseTexture::~GLESBaseTexture() {
	release();
}

void GLESBaseTexture::release() {
	if (_texture_name) {
		LOGD("Destroying texture %u", _texture_name);

		GLCALL(glDeleteTextures(1, &_texture_name));
		_texture_name = 0;
	}
}

void GLESBaseTexture::reinit() {
	GLCALL(glGenTextures(1, &_texture_name));

	initSize();

	setDirty();
}

void GLESBaseTexture::initSize() {
	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, _glFormat,
						_texture_width, _texture_height,
						0, _glFormat, _glType, 0));
}

void GLESBaseTexture::setLinearFilter(bool value) {
	if (value)
		_glFilter = GL_LINEAR;
	else
		_glFilter = GL_NEAREST;

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void GLESBaseTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.bytesPerPixel = _pixelFormat.bytesPerPixel;

	if (w == _texture_width && h == _texture_height)
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}

	initSize();
}

void GLESBaseTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

#ifdef GL_OES_draw_texture
	// Great extension, but only works under specific conditions.
	// Still a work-in-progress - disabled for now.
	if (false && draw_tex_supported && !hasPalette()) {
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

	clearDirty();
}

const Graphics::PixelFormat &GLESBaseTexture::getPixelFormat() const {
	return _pixelFormat;
}

GLESTexture::GLESTexture(GLenum glFormat, GLenum glType,
							Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_pixels(0),
	_buf(0) {
}

GLESTexture::~GLESTexture() {
	delete[] _buf;
	delete[] _pixels;
}

void GLESTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.pitch = w * _pixelFormat.bytesPerPixel;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _buf;
	delete[] _pixels;

	_pixels = new byte[w * h * _surface.bytesPerPixel];
	assert(_pixels);

	_surface.pixels = _pixels;

	fillBuffer(0);

	_buf = new byte[w * h * _surface.bytesPerPixel];
	assert(_buf);
}

void GLESTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
								const void *buf, int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x * _surface.bytesPerPixel;

	do {
		memcpy(dst, src, w * _surface.bytesPerPixel);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESTexture::fillBuffer(uint32 color) {
	assert(_surface.pixels);

	if (_pixelFormat.bytesPerPixel == 1 ||
			((color & 0xff) == ((color >> 8) & 0xff)))
		memset(_pixels, color & 0xff, _surface.pitch * _surface.h);
	else
		Common::set_to(_pixels, _pixels + _surface.pitch * _surface.h,
						(uint16)color);

	setDirty();
}

void GLESTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {
		byte *_tex;

		int16 dwidth = _dirty_rect.width();
		int16 dheight = _dirty_rect.height();

		if (dwidth == _surface.w) {
			_tex = _pixels + _dirty_rect.top * _surface.pitch;
		} else {
			_tex = _buf;

			byte *src = _pixels + _dirty_rect.top * _surface.pitch +
						_dirty_rect.left * _surface.bytesPerPixel;
			byte *dst = _buf;

			uint16 l = dwidth * _surface.bytesPerPixel;

			for (uint16 i = 0; i < dheight; ++i) {
				memcpy(dst, src, l);
				src += _surface.pitch;
				dst += l;
			}
		}

		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
		GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
								_dirty_rect.left, _dirty_rect.top,
								dwidth, dheight, _glFormat, _glType, _tex));
	}

	GLESBaseTexture::drawTexture(x, y, w, h);
}

GLES4444Texture::GLES4444Texture() :
	GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixelFormat()) {
}

GLES4444Texture::~GLES4444Texture() {
}

GLES5551Texture::GLES5551Texture() :
	GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixelFormat()) {
}

GLES5551Texture::~GLES5551Texture() {
}

GLES565Texture::GLES565Texture() :
	GLESTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixelFormat()) {
}

GLES565Texture::~GLES565Texture() {
}

GLESPaletteTexture::GLESPaletteTexture(GLenum glFormat, GLenum glType,
									Graphics::PixelFormat palettePixelFormat) :
	GLESBaseTexture(glFormat, glType,
				Graphics::PixelFormat::createFormatCLUT8()),
	_texture(0)
{
	_palettePixelFormat = palettePixelFormat;
	_paletteSize = _palettePixelFormat.bytesPerPixel * 256;
}

GLESPaletteTexture::~GLESPaletteTexture() {
	delete[] _texture;
}

void GLESPaletteTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.pitch = _texture_width;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	byte *old_texture = _texture;

	_texture = new byte[_paletteSize + _texture_width * _texture_height];
	assert(_texture);

	_surface.pixels = _texture + _paletteSize;

	fillBuffer(0);

	if (old_texture) {
		// preserve palette
		memcpy(_texture, old_texture, _paletteSize);
		delete[] old_texture;
	}
}

void GLESPaletteTexture::fillBuffer(uint32 color) {
	assert(_surface.pixels);
	memset(_surface.pixels, color & 0xff, _surface.pitch * _surface.h);
	setDirty();
}

void GLESPaletteTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
										const void *buf, int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte * src = static_cast<const byte *>(buf);
	byte *dst = static_cast<byte *>(_surface.getBasePtr(x, y));

	do {
		memcpy(dst, src, w);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESPaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w,
										GLshort h) {
	if (dirty()) {
		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

		const size_t texture_size = _paletteSize +
									_texture_width * _texture_height;

		GLCALL(glCompressedTexImage2D(GL_TEXTURE_2D, 0, _glType,
										_texture_width, _texture_height,
										0, texture_size, _texture));
	}

	GLESBaseTexture::drawTexture(x, y, w, h);
}

GLESPalette888Texture::GLESPalette888Texture() :
	GLESPaletteTexture(GL_RGB, GL_PALETTE8_RGB8_OES,
						Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0)) {
}

GLESPalette888Texture::~GLESPalette888Texture() {
}

GLESPalette8888Texture::GLESPalette8888Texture() :
	GLESPaletteTexture(GL_RGBA, GL_PALETTE8_RGBA8_OES,
						Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) {
}

GLESPalette8888Texture::~GLESPalette8888Texture() {
}

GLESPalette565Texture::GLESPalette565Texture() :
	GLESPaletteTexture(GL_RGB, GL_PALETTE8_R5_G6_B5_OES,
						Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) {
}

GLESPalette565Texture::~GLESPalette565Texture() {
}

GLESPalette4444Texture::GLESPalette4444Texture() :
	GLESPaletteTexture(GL_RGBA, GL_PALETTE8_RGBA4_OES,
						Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0)) {
}

GLESPalette4444Texture::~GLESPalette4444Texture() {
}

GLESPalette5551Texture::GLESPalette5551Texture() :
	GLESPaletteTexture(GL_RGBA, GL_PALETTE8_RGB5_A1_OES,
						Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)) {
}

GLESPalette5551Texture::~GLESPalette5551Texture() {
}

GLESFakePaletteTexture::GLESFakePaletteTexture(GLenum glFormat, GLenum glType,
									Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_palette(0),
	_pixels(0),
	_buf(0)
{
	_palettePixelFormat = pixelFormat;
	_fake_format = Graphics::PixelFormat::createFormatCLUT8();

	_palette = new uint16[256];
	assert(_palette);

	memset(_palette, 0, 256 * 2);
}

GLESFakePaletteTexture::~GLESFakePaletteTexture() {
	delete[] _buf;
	delete[] _pixels;
	delete[] _palette;
}

void GLESFakePaletteTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.bytesPerPixel = 1;
	_surface.pitch = w;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _buf;
	delete[] _pixels;

	_pixels = new byte[w * h];
	assert(_pixels);

	// fixup surface, for the outside this is a CLUT8 surface
	_surface.pixels = _pixels;

	fillBuffer(0);

	_buf = new uint16[w * h];
	assert(_buf);
}

void GLESFakePaletteTexture::fillBuffer(uint32 color) {
	assert(_surface.pixels);
	memset(_surface.pixels, color & 0xff, _surface.pitch * _surface.h);
	setDirty();
}

void GLESFakePaletteTexture::updateBuffer(GLuint x, GLuint y, GLuint w,
											GLuint h, const void *buf,
											int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x;

	do {
		memcpy(dst, src, w);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESFakePaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w,
										GLshort h) {
	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {
		int16 dwidth = _dirty_rect.width();
		int16 dheight = _dirty_rect.height();

		byte *src = _pixels + _dirty_rect.top * _surface.pitch +
					_dirty_rect.left;
		uint16 *dst = _buf;
		uint pitch_delta = _surface.pitch - dwidth;

		for (uint16 j = 0; j < dheight; ++j) {
			for (uint16 i = 0; i < dwidth; ++i)
				*dst++ = _palette[*src++];
			src += pitch_delta;
		}

		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
								_dirty_rect.left, _dirty_rect.top,
								dwidth, dheight, _glFormat, _glType, _buf));
	}

	GLESBaseTexture::drawTexture(x, y, w, h);
}

const Graphics::PixelFormat &GLESFakePaletteTexture::getPixelFormat() const {
	return _fake_format;
}

GLESFakePalette565Texture::GLESFakePalette565Texture() :
	GLESFakePaletteTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
							GLES565Texture::pixelFormat()) {
}

GLESFakePalette565Texture::~GLESFakePalette565Texture() {
}

#endif

