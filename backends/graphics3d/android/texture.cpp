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

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

#include "base/main.h"
#include "graphics/surface.h"
#include "graphics/opengl/shader.h"
#include "graphics/opengl/context.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"

#include "backends/graphics3d/android/texture.h"


AndroidFrameBuffer::AndroidFrameBuffer(GLenum glIntFormat, GLenum glFormat, GLenum glType, GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height) :
		OpenGL::FrameBuffer(glIntFormat, glFormat, glType, false) {
	if (!OpenGLContext.framebufferObjectSupported) {
		error("FrameBuffer Objects are not supported by the current OpenGL context");
	}

	_logicalWidth = width;
	_logicalHeight = height;
	_width = texture_width;
	_height = texture_height;
	_glTexture = texture_name;

	if (_width != 0 && _height != 0) {
		const GLfloat texWidth = (GLfloat)_logicalWidth / _width;
		const GLfloat texHeight = (GLfloat)_logicalHeight / _height;

		_texCoords[0] = 0;
		_texCoords[1] = 0;

		_texCoords[2] = texWidth;
		_texCoords[3] = 0;

		_texCoords[4] = 0;
		_texCoords[5] = texHeight;

		_texCoords[6] = texWidth;
		_texCoords[7] = texHeight;
	}

	enableLinearFiltering(true);

	init();
}

AndroidFrameBuffer::~AndroidFrameBuffer() {
	// Prevent the texture from being deleted by the parent class
	_glTexture = 0;
}

// Supported GL extensions
bool GLESBaseTexture::_npot_supported = false;
OpenGL::Shader *GLESBaseTexture::_box_shader = nullptr;
GLuint GLESBaseTexture::_verticesVBO = 0;

template<class T>
static T nextHigher2(T k) {
	if (k == 0) {
		return 1;
	}
	--k;

	for (uint i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1) {
		k = k | k >> i;
	}

	return k + 1;
}

static const GLfloat vertices[] = {
	0.0, 0.0,
	1.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
};

static const char *controlVertex =
	"#version 100\n"
	"attribute vec2 position;\n"
	"attribute vec2 texcoord;\n"
	"uniform vec2 offsetXY;\n"
	"uniform vec2 sizeWH;\n"
	"uniform vec4 clip;\n"
	"uniform bool flipY;\n"
	"varying vec2 Texcoord;\n"
	"void main() {\n"
		"Texcoord = clip.xy + texcoord * (clip.zw - clip.xy);\n"
		"vec2 pos = offsetXY + position * sizeWH;\n"
		"pos.x = pos.x * 2.0 - 1.0;\n"
		"pos.y = pos.y * 2.0 - 1.0;\n"
		"if (flipY)\n"
			"pos.y *= -1.0;\n"
		"gl_Position = vec4(pos, 0.0, 1.0);\n"
	"}\n";

static const char *controlFragment =
	"#version 100\n"
	"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
		"precision highp float;\n"
	"#else\n"
		"precision mediump float;\n"
	"#endif\n"
	"varying vec2 Texcoord;\n"
	"uniform float alpha;\n"
	"uniform sampler2D tex;\n"
	"void main() {\n"
		"gl_FragColor = texture2D(tex, Texcoord) * vec4(1.0, 1.0, 1.0, alpha);\n"
	"}\n";

void GLESBaseTexture::initGL() {
	_npot_supported = OpenGLContext.NPOTSupported;

	const char *attributes[] = { "position", "texcoord", NULL };
	_box_shader = OpenGL::Shader::fromStrings("control", controlVertex, controlFragment, attributes);
	_verticesVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices);
	_box_shader->enableVertexAttribute("position", _verticesVBO, 2, GL_FLOAT, GL_TRUE,
	                                   2 * sizeof(float), 0);
	_box_shader->enableVertexAttribute("texcoord", _verticesVBO, 2, GL_FLOAT, GL_TRUE,
	                                   2 * sizeof(float), 0);
}

void GLESBaseTexture::unbindShader() {
	if (_box_shader) {
		_box_shader->unbind();
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
	_alpha(1.f),
	_draw_rect(),
	_all_dirty(false),
	_dirty_rect(),
	_pixelFormat(pixelFormat),
	_palettePixelFormat(),
	_is_game_texture(false) {
}

GLESBaseTexture::~GLESBaseTexture() {
	release();
}

void GLESBaseTexture::release() {
	if (_texture_name) {
		GLCALL(glDeleteTextures(1, &_texture_name));
		_texture_name = 0;
	}
}

void GLESBaseTexture::reinit() {
	if (_texture_name) {
		release();
	}

	GLCALL(glGenTextures(1, &_texture_name));

	initSize();

	setDirty();
}

void GLESBaseTexture::initSize() {
	if (!_texture_name) {
		return;
	}

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
	if (value) {
		_glFilter = GL_LINEAR;
	} else {
		_glFilter = GL_NEAREST;
	}

	if (!_texture_name) {
		return;
	}

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void GLESBaseTexture::allocBuffer(GLuint w, GLuint h) {
	if (w == _surface.w && h == _surface.h) {
		return;
	}

	_surface.w = w;
	_surface.h = h;
	_surface.format = _pixelFormat;

	if (_npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}

	initSize();
}

void GLESBaseTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h,
                                  const Common::Rect &clip) {
	if (!_texture_name) {
		return;
	}

	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {
		void *tex = prepareTextureBuffer(_dirty_rect);

		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
		GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
		                       _dirty_rect.left, _dirty_rect.top,
		                       _dirty_rect.width(), _dirty_rect.height(),
		                       _glFormat, _glType, tex));
	}


//	LOGD("*** Texture %p: Drawing %dx%d rect to (%d,%d)", this, w, h, x, y);

	assert(_box_shader);
	_box_shader->use();

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	const GLfloat offsetX    = float(x) / float(JNI::egl_surface_width);
	const GLfloat offsetY    = float(y) / float(JNI::egl_surface_height);
	const GLfloat sizeW      = float(w) / float(JNI::egl_surface_width);
	const GLfloat sizeH      = float(h) / float(JNI::egl_surface_height);
	Math::Vector4d clipV = Math::Vector4d(clip.left, clip.top, clip.right, clip.bottom);
	clipV.x() /= _texture_width;
	clipV.y() /= _texture_height;
	clipV.z() /= _texture_width;
	clipV.w() /= _texture_height;
//	LOGD("*** Drawing at (%f,%f) , size %f x %f", float(x) / float(_surface.w), float(y) / float(_surface.h),  tex_width, tex_height);

	_box_shader->setUniform1f("alpha", _alpha);
	_box_shader->setUniform("offsetXY", Math::Vector2d(offsetX, offsetY));
	_box_shader->setUniform("sizeWH", Math::Vector2d(sizeW, sizeH));
	_box_shader->setUniform("clip", clipV);
	_box_shader->setUniform("flipY", !_is_game_texture);

	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	clearDirty();
}

const Graphics::PixelFormat &GLESBaseTexture::getPixelFormat() const {
	return _pixelFormat;
}

GLESTexture::GLESTexture(GLenum glFormat, GLenum glType,
                         Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_pixels(nullptr),
	_buf(nullptr) {
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

	_pixels = new byte[w * h * _surface.format.bytesPerPixel];

	_surface.setPixels(_pixels);

	fillBuffer(0);

	_buf = new byte[w * h * _surface.format.bytesPerPixel];
}

void GLESTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
                               const void *buf, int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x * _surface.format.bytesPerPixel;

	do {
		memcpy(dst, src, w * _surface.format.bytesPerPixel);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESTexture::fillBuffer(uint32 color) {
	assert(_surface.getPixels());

	if (_pixelFormat.bytesPerPixel == 1 ||
	        ((color & 0xff) == ((color >> 8) & 0xff))) {
		memset(_pixels, color & 0xff, _surface.pitch * _surface.h);
	} else {
		Common::fill(_pixels, _pixels + _surface.pitch * _surface.h,
		             (uint16)color);
	}

	setDirty();
}

void *GLESTexture::prepareTextureBuffer(const Common::Rect &rect) {
	if (rect.width() == _surface.w) {
		return _pixels + rect.top * _surface.pitch;
	} else {
		byte *tex = _buf;

		byte *src = _pixels + rect.top * _surface.pitch +
			    rect.left * _surface.format.bytesPerPixel;
		byte *dst = _buf;

		uint16 l = rect.width() * _surface.format.bytesPerPixel;

		for (uint16 i = rect.height(); i > 0; --i) {
			memcpy(dst, src, l);
			src += _surface.pitch;
			dst += l;
		}

		return tex;
	}
}

void GLESTexture::readPixels() {
	GLCALL(glReadPixels(0, 0, _surface.w, _surface.h, _glFormat, _glType,
			    _pixels));
	setDirty();
}

GLESFakePaletteTexture::GLESFakePaletteTexture(GLenum glFormat, GLenum glType,
        Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_pixels(nullptr) {
	_palettePixelFormat = pixelFormat;
	_fake_format = Graphics::PixelFormat::createFormatCLUT8();
}

GLESFakePaletteTexture::~GLESFakePaletteTexture() {
	delete[] _pixels;
}

void GLESFakePaletteTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.format = Graphics::PixelFormat::createFormatCLUT8();
	_surface.pitch = w;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _pixels;
	_pixels = nullptr;

	_pixels = new byte[w * h];

	// fixup surface, for the outside this is a CLUT8 surface
	_surface.setPixels(_pixels);

	fillBuffer(0);
}

void GLESFakePaletteTexture::fillBuffer(uint32 color) {
	assert(_surface.getPixels());
	memset(_surface.getPixels(), color & 0xff, _surface.pitch * _surface.h);
	setDirty();
}

void GLESFakePaletteTexture::updateBuffer(GLuint x, GLuint y,
        GLuint w, GLuint h, const void *buf, int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x;

	do {
		memcpy(dst, src, w);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

GLESFakePalette16Texture::GLESFakePalette16Texture(GLenum glFormat, GLenum glType,
        Graphics::PixelFormat pixelFormat) :
	GLESFakePaletteTexture(glFormat, glType, pixelFormat),
	_palette(nullptr),
	_buf(nullptr) {
	_palette = new uint16[256];
	memset(_palette, 0, sizeof(*_palette) * 256);
}

GLESFakePalette16Texture::~GLESFakePalette16Texture() {
	delete[] _buf;
	delete[] _palette;
}

void GLESFakePalette16Texture::allocBuffer(GLuint w, GLuint h) {
	delete[] _buf;
	_buf = nullptr;

	GLESFakePaletteTexture::allocBuffer(w, h);

	_buf = new uint16[w * h];
}

void *GLESFakePalette16Texture::prepareTextureBuffer(const Common::Rect &rect) {
	int16 w = rect.width();

	byte *src = _pixels + rect.top * _surface.pitch +
		    rect.left;
	uint16 *dst = _buf;
	uint pitch_delta = _surface.pitch - w;

	for (uint16 j = rect.height(); j > 0; --j) {
		for (uint16 i = 0; i < w; ++i) {
			*dst++ = _palette[*src++];
		}
		src += pitch_delta;
	}

	return _buf;
}

void GLESFakePalette16Texture::setPalette(const byte *colors, uint start, uint num) {
	uint16 *p = _palette + start;

	for (uint i = 0; i < num; ++i, colors += 3, ++p) {
		*p = _palettePixelFormat.RGBToColor(colors[0], colors[1], colors[2]);
	}
}

void GLESFakePalette16Texture::grabPalette(byte *colors, uint start, uint num) const {
        const uint16 *p = _palette + start;

        for (uint i = 0; i < num; ++i, colors += 3, ++p) {
                _palettePixelFormat.colorToRGB(*p, colors[0], colors[1], colors[2]);
        }
}

GLESFakePalette565Texture::GLESFakePalette565Texture() :
	GLESFakePalette16Texture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
	                         GLES565Texture::pixelFormat()) {
}

GLESFakePalette5551Texture::GLESFakePalette5551Texture() :
	GLESFakePalette16Texture(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
	                         GLES5551Texture::pixelFormat()),
	_keycolor(0) {
}

void GLESFakePalette5551Texture::setKeycolor(byte color) {
	uint16 *p = _palette + _keycolor;
	*p |= 1;

	_keycolor = color;

	p = _palette + _keycolor;
	*p &= ~1;
}

GLESFakePalette888Texture::GLESFakePalette888Texture() :
	GLESFakePaletteTexture(GL_RGB, GL_UNSIGNED_BYTE,
	                       GLES888Texture::pixelFormat()),
	_palette(nullptr),
	_buf(nullptr) {
	_palette = new byte[256 * 3];
	memset(_palette, 0, 256 * 3);
}

GLESFakePalette888Texture::~GLESFakePalette888Texture() {
	delete[] _buf;
	delete[] _palette;
}

void GLESFakePalette888Texture::allocBuffer(GLuint w, GLuint h) {
	delete[] _buf;
	_buf = nullptr;

	GLESFakePaletteTexture::allocBuffer(w, h);

	_buf = new byte[w * h * 3];
}

void *GLESFakePalette888Texture::prepareTextureBuffer(const Common::Rect &rect) {
	int16 w = rect.width();

	byte *src = _pixels + rect.top * _surface.pitch +
		    rect.left;
	byte *dst = _buf;
	uint pitch_delta = _surface.pitch - w;

	for (uint16 j = rect.height(); j > 0; --j) {
		for (uint16 i = 0; i < w; ++i) {
			byte c = *src++;
			byte *p = _palette + c * 3;
			*dst++ = *p++;
			*dst++ = *p++;
			*dst++ = *p++;
		}
		src += pitch_delta;
	}

	return _buf;
}

void GLESFakePalette888Texture::setPalette(const byte *colors, uint start, uint num) {
	memcpy(_palette + start * 3, colors, num * 3);
}

void GLESFakePalette888Texture::grabPalette(byte *colors, uint start, uint num) const {
	memcpy(colors, _palette + start * 3, num * 3);
}

GLESFakePalette8888Texture::GLESFakePalette8888Texture() :
	GLESFakePaletteTexture(GL_RGBA, GL_UNSIGNED_BYTE,
	                       GLES8888Texture::pixelFormat()),
	_palette(nullptr),
	_buf(nullptr),
	_keycolor(0) {
	_palette = new uint32[256];
	memset(_palette, 0, sizeof(*_palette) * 256);
}

GLESFakePalette8888Texture::~GLESFakePalette8888Texture() {
	delete[] _buf;
	delete[] _palette;
}

void GLESFakePalette8888Texture::allocBuffer(GLuint w, GLuint h) {
	delete[] _buf;
	_buf = nullptr;

	GLESFakePaletteTexture::allocBuffer(w, h);

	_buf = new uint32[w * h];
}

void *GLESFakePalette8888Texture::prepareTextureBuffer(const Common::Rect &rect) {
	int16 w = rect.width();

	byte *src = _pixels + rect.top * _surface.pitch +
		    rect.left;
	uint32 *dst = _buf;
	uint pitch_delta = _surface.pitch - w;

	for (uint16 j = rect.height(); j > 0; --j) {
		for (uint16 i = 0; i < w; ++i) {
			*dst++ = _palette[*src++];
		}
		src += pitch_delta;
	}

	return _buf;
}

void GLESFakePalette8888Texture::setPalette(const byte *colors, uint start, uint num) {
	// We use _palette as a uint32 to ensure proper alignment but we store bytes in a endian independent fashion
	// because GL_UNSIGNED_BYTE is endian neutral
	byte *p = (byte *)(_palette + start);

	for (uint i = 0; i < num; ++i, colors += 3, p += 4) {
		p[0] = colors[0];
		p[1] = colors[1];
		p[2] = colors[2];
		p[3] = 255;
	}
}

void GLESFakePalette8888Texture::setKeycolor(byte color) {
	// _palette is endian neutral even though it's an uint32
	byte *p = (byte *)(_palette + _keycolor);
	p[3] = 255;

	_keycolor = color;

	p = (byte *)(_palette + _keycolor);
	p[3] = 0;
}

void GLESFakePalette8888Texture::grabPalette(byte *colors, uint start, uint num) const {
	const byte *p = (byte *)(_palette + start);

        for (uint i = 0; i < num; ++i, colors += 3, p += 4) {
		colors[0] = p[0];
		colors[1] = p[1];
		colors[2] = p[2];
        }
}
