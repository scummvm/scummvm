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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef _ANDROID_TEXTURE_H_
#define _ANDROID_TEXTURE_H_

#if defined(__ANDROID__)

#include <GLES/gl.h>

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

#include "common/rect.h"
#include "common/array.h"

class GLESTexture {
public:
	static void initGLExtensions();

protected:
	GLESTexture(byte bytesPerPixel, GLenum glFormat, GLenum glType,
				Graphics::PixelFormat pixelFormat);

public:
	virtual ~GLESTexture();

	void release();
	void reinit();
	void initSize();

	virtual void allocBuffer(GLuint width, GLuint height);

	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
								const void *buf, int pitch_buf);
	virtual void fillBuffer(uint32 color);

	virtual void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);

	inline void drawTexture() {
		drawTexture(0, 0, _surface.w, _surface.h);
	}

	inline GLuint width() const {
		return _surface.w;
	}

	inline GLuint height() const {
		return _surface.h;
	}

	inline uint16 pitch() const {
		return _surface.pitch;
	}

	inline const Graphics::Surface *surface_const() const {
		return &_surface;
	}

	inline Graphics::Surface *surface() {
		setDirty();
		return &_surface;
	}

	virtual const byte *palette_const() const {
		return 0;
	};

	virtual byte *palette() {
		return 0;
	};

	inline bool hasPalette() const {
		return palette_const() != 0;
	}

	inline bool dirty() const {
		return _all_dirty || !_dirty_rect.isEmpty();
	}

	inline const Graphics::PixelFormat &getPixelFormat() const {
		return _pixelFormat;
	}

protected:
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

	inline void setDirtyRect(const Common::Rect& r) {
		if (!_all_dirty) {
			if (_dirty_rect.isEmpty())
				_dirty_rect = r;
			else
				_dirty_rect.extend(r);
		}
	}

	byte _bytesPerPixel;
	GLenum _glFormat;
	GLenum _glType;

	GLuint _texture_name;
	Graphics::Surface _surface;
	GLuint _texture_width;
	GLuint _texture_height;

	// Covers dirty area
	bool _all_dirty;
	Common::Rect _dirty_rect;

	Graphics::PixelFormat _pixelFormat;
};

// RGBA4444 texture
class GLES4444Texture : public GLESTexture {
public:
	GLES4444Texture();
	virtual ~GLES4444Texture();

	static inline Graphics::PixelFormat getPixelFormat() {
		return Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0);
	}
};

// RGBA5551 texture
class GLES5551Texture : public GLESTexture {
public:
	GLES5551Texture();
	virtual ~GLES5551Texture();

	static inline Graphics::PixelFormat getPixelFormat() {
		return Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0);
	}
};

// RGB565 texture
class GLES565Texture : public GLESTexture {
public:
	GLES565Texture();
	virtual ~GLES565Texture();

	static inline Graphics::PixelFormat getPixelFormat() {
		return Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	}
};

class GLESPaletteTexture : public GLESTexture {
protected:
	GLESPaletteTexture(byte bytesPerPixel, GLenum glFormat, GLenum glType,
						size_t paletteSize);

public:
	virtual ~GLESPaletteTexture();

	virtual void allocBuffer(GLuint width, GLuint height);
	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
								const void *buf, int pitch_buf);
	virtual void fillBuffer(uint32 color);

	virtual void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);

	inline void drawTexture() {
		drawTexture(0, 0, _surface.w, _surface.h);
	}

	virtual const byte *palette_const() const {
		return _texture;
	};

	virtual byte *palette() {
		setDirty();
		return _texture;
	};

protected:
	byte *_texture;
	size_t _paletteSize;
};

// RGB888 256-entry paletted texture
class GLESPalette888Texture : public GLESPaletteTexture {
public:
	GLESPalette888Texture();
	virtual ~GLESPalette888Texture();
};

// RGBA8888 256-entry paletted texture
class GLESPalette8888Texture : public GLESPaletteTexture {
public:
	GLESPalette8888Texture();
	virtual ~GLESPalette8888Texture();
};

#endif
#endif

