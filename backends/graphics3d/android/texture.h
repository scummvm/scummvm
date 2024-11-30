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

#ifndef BACKENDS_GRAPHICS3D_ANDROID_TEXTURE_H
#define BACKENDS_GRAPHICS3D_ANDROID_TEXTURE_H

#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>

#include "backends/graphics3d/opengl/framebuffer.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include "common/rect.h"
#include "common/array.h"

namespace OpenGL {
class Shader;
}

class AndroidFrameBuffer : public OpenGL::FrameBuffer {
public:
	AndroidFrameBuffer(GLenum glIntFormat, GLenum glFormat, GLenum glType, GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height);
	~AndroidFrameBuffer();
};

class GLESBaseTexture {
public:
	static void initGL();
	static void unbindShader();

protected:
	GLESBaseTexture(GLenum glFormat, GLenum glType,
	                Graphics::PixelFormat pixelFormat);

public:
	virtual ~GLESBaseTexture();

	void release();
	void reinit();

	void setLinearFilter(bool value);

	virtual void allocBuffer(GLuint w, GLuint h);

	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
	                          const void *buf, int pitch_buf) = 0;
	virtual void fillBuffer(uint32 color) = 0;

	void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
		drawTexture(x, y, w, h, Common::Rect(0, 0, width(), height()));
	}
	void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h, const Common::Rect &clip);

	inline void setDrawRect(const Common::Rect &rect) {
		_draw_rect = rect;
	}

	inline void setDrawRect(int16 w, int16 h) {
		_draw_rect = Common::Rect(w, h);
	}

	inline void setDrawRect(int16 x1, int16 y1, int16 x2, int16 y2) {
		_draw_rect = Common::Rect(x1, y1, x2, y2);
	}

	inline const Common::Rect &getDrawRect() const {
		return _draw_rect;
	}

	inline void drawTextureRect() {
		drawTexture(_draw_rect.left, _draw_rect.top,
		            _draw_rect.width(), _draw_rect.height());
	}

	inline void drawTextureOrigin() {
		drawTexture(0, 0, _surface.w, _surface.h);
	}

	inline GLuint width() const {
		return _surface.w;
	}

	inline GLuint height() const {
		return _surface.h;
	}

	inline GLuint texWidth() const {
		return _texture_width;
	}

	inline GLuint texHeight() const {
		return _texture_height;
	}

	inline uint16 pitch() const {
		return _surface.pitch;
	}

	inline bool isEmpty() const {
		return _surface.w == 0 || _surface.h == 0;
	}

	inline const Graphics::Surface *surface_const() const {
		return &_surface;
	}

	inline Graphics::Surface *surface() {
		setDirty();
		return &_surface;
	}

	virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	virtual void setKeycolor(byte color) = 0;
	virtual void grabPalette(byte *colors, uint start, uint num) const = 0;

	inline bool hasPalette() const {
		return _palettePixelFormat.bytesPerPixel > 0;
	}

	inline bool dirty() const {
		return _all_dirty || !_dirty_rect.isEmpty();
	}

	virtual const Graphics::PixelFormat &getPixelFormat() const;

	inline const Graphics::PixelFormat &getPalettePixelFormat() const {
		return _palettePixelFormat;
	}

	GLuint getTextureName() const {
		return _texture_name;
	}

	GLenum getTextureFormat() const {
		return _glFormat;
	}

	GLenum getTextureType() const {
		return _glType;
	}

	void setGameTexture() {
		_is_game_texture = true;
	}

	void setAlpha(float alpha) {
		_alpha = alpha;
	}

protected:
	void initSize();

	virtual void *prepareTextureBuffer(const Common::Rect &rect) = 0;

	inline void setDirty() {
		_all_dirty = true;
	}

	inline void clearDirty() {
		_all_dirty = false;
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = 0;
		_dirty_rect.right = 0;
	}

	inline void setDirtyRect(const Common::Rect &r) {
		if (!_all_dirty) {
			if (_dirty_rect.isEmpty()) {
				_dirty_rect = r;
			} else {
				_dirty_rect.extend(r);
			}
		}
	}

	GLenum _glFormat;
	GLenum _glType;
	GLint _glFilter;

	GLuint _texture_name;
	Graphics::Surface _surface;
	GLuint _texture_width;
	GLuint _texture_height;

	Common::Rect _draw_rect;

	bool _all_dirty;
	Common::Rect _dirty_rect;

	Graphics::PixelFormat _pixelFormat;
	Graphics::PixelFormat _palettePixelFormat;

	bool _is_game_texture;

	GLfloat _alpha;

	static bool _npot_supported;
	static OpenGL::Shader *_box_shader;
	static GLuint _verticesVBO;

};

class GLESTexture : public GLESBaseTexture {
protected:
	GLESTexture(GLenum glFormat, GLenum glType,
	            Graphics::PixelFormat pixelFormat);

public:
	virtual ~GLESTexture();

	void allocBuffer(GLuint w, GLuint h) override;

	void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
	                          const void *buf, int pitch_buf) override;
	void fillBuffer(uint32 color) override;

	void setPalette(const byte *colors, uint start, uint num) override {}
	void setKeycolor(byte color) override {};
	void grabPalette(byte *colors, uint start, uint num) const override {}

	void readPixels();

protected:
	void *prepareTextureBuffer(const Common::Rect &rect) override;

	byte *_pixels;
	byte *_buf;
};

// RGBA4444 texture
class GLES4444Texture : public GLESTexture {
public:
	GLES4444Texture() : GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixelFormat()) {}
	virtual ~GLES4444Texture() {}

	static Graphics::PixelFormat pixelFormat() {
		return Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0);
	}
};

// RGBA5551 texture
class GLES5551Texture : public GLESTexture {
public:
	GLES5551Texture() : GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixelFormat()) {}
	virtual ~GLES5551Texture() {}

	static inline Graphics::PixelFormat pixelFormat() {
		return Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0);
	}
};

// RGB565 texture
class GLES565Texture : public GLESTexture {
public:
	GLES565Texture() : GLESTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixelFormat()) {}
	virtual ~GLES565Texture() {}

	static inline Graphics::PixelFormat pixelFormat() {
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	}
};

class GLES888Texture : public GLESTexture {
public:
	GLES888Texture() : GLESTexture(GL_RGB, GL_UNSIGNED_BYTE, pixelFormat()) {}
	virtual ~GLES888Texture() {}

	static Graphics::PixelFormat pixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
#else
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
#endif
	}
};

class GLES8888Texture : public GLESTexture {
public:
	GLES8888Texture() : GLESTexture(GL_RGBA, GL_UNSIGNED_BYTE, pixelFormat()) {}
	virtual ~GLES8888Texture() {}

	static Graphics::PixelFormat pixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
	}
};

class GLESFakePaletteTexture : public GLESBaseTexture {
protected:
	GLESFakePaletteTexture(GLenum glFormat, GLenum glType,
	                       Graphics::PixelFormat pixelFormat);

public:
	virtual ~GLESFakePaletteTexture();

	void allocBuffer(GLuint w, GLuint h) override;
	void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
	                          const void *buf, int pitch_buf) override;
	void fillBuffer(uint32 color) override;

	const Graphics::PixelFormat &getPixelFormat() const override {
		return _fake_format;
	}

protected:
	Graphics::PixelFormat _fake_format;
	byte *_pixels;
};

class GLESFakePalette16Texture : public GLESFakePaletteTexture {
protected:
	GLESFakePalette16Texture(GLenum glFormat, GLenum glType,
	                       Graphics::PixelFormat pixelFormat);
public:
	virtual ~GLESFakePalette16Texture();

	void allocBuffer(GLuint w, GLuint h) override;

	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

protected:
	void *prepareTextureBuffer(const Common::Rect &rect) override;

	uint16 *_palette;
	uint16 *_buf;
};

class GLESFakePalette565Texture : public GLESFakePalette16Texture {
public:
	GLESFakePalette565Texture();
	virtual ~GLESFakePalette565Texture() {}

	void setKeycolor(byte color) override {};
};

class GLESFakePalette5551Texture : public GLESFakePalette16Texture {
public:
	GLESFakePalette5551Texture();
	virtual ~GLESFakePalette5551Texture() {}

	void setKeycolor(byte color) override;

protected:
	byte _keycolor;
};

class GLESFakePalette888Texture : public GLESFakePaletteTexture {
public:
	GLESFakePalette888Texture();
	virtual ~GLESFakePalette888Texture();

	void allocBuffer(GLuint w, GLuint h) override;

	void setPalette(const byte *colors, uint start, uint num) override;
	void setKeycolor(byte color) override {};
	void grabPalette(byte *colors, uint start, uint num) const override;

protected:
	void *prepareTextureBuffer(const Common::Rect &rect) override;

	byte *_palette;
	byte *_buf;
};

class GLESFakePalette8888Texture : public GLESFakePaletteTexture {
public:
	GLESFakePalette8888Texture();
	virtual ~GLESFakePalette8888Texture();

	void allocBuffer(GLuint w, GLuint h) override;

	void setPalette(const byte *colors, uint start, uint num) override;
	void setKeycolor(byte color) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

protected:
	void *prepareTextureBuffer(const Common::Rect &rect) override;

	uint32 *_palette;
	uint32 *_buf;
	byte _keycolor;
};

#endif
