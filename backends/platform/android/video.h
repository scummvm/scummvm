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

#if defined(ANDROID)

#include <GLES/gl.h>

#include "graphics/surface.h"

#include "common/rect.h"
#include "common/array.h"

class GLESTexture {
public:
	static void initGLExtensions();

	GLESTexture();
	virtual ~GLESTexture();
	virtual void reinitGL();
	virtual void allocBuffer(GLuint width, GLuint height);
	const Graphics::Surface* surface_const() const { return &_surface; }
	GLuint width() const { return _surface.w; }
	GLuint height() const { return _surface.h; }
	GLuint texture_name() const { return _texture_name; }
	bool dirty() const { return _all_dirty || !_dirty_rect.isEmpty(); }
	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
							  const void* buf, int pitch);
	virtual void fillBuffer(byte x);
	virtual void drawTexture() {
		drawTexture(0, 0, _surface.w, _surface.h);
	}
	virtual void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);

protected:
	virtual byte bytesPerPixel() const = 0;
	virtual GLenum glFormat() const = 0;
	virtual GLenum glType() const = 0;
	virtual size_t paletteSize() const { return 0; };
	void setDirty() {
		_all_dirty = true;
		_dirty_rect = Common::Rect();
	}
	void setDirtyRect(const Common::Rect& r) {
		if (!_all_dirty) {
			if (_dirty_rect.isEmpty())
				_dirty_rect = r;
			else
				_dirty_rect.extend(r);
		}
	}
	GLuint _texture_name;
	Graphics::Surface _surface;
	GLuint _texture_width;
	GLuint _texture_height;
	bool _all_dirty;
	Common::Rect _dirty_rect;  // Covers dirty area
};

// RGBA4444 texture
class GLES4444Texture : public GLESTexture {
protected:
	virtual byte bytesPerPixel() const { return 2; }
	virtual GLenum glFormat() const { return GL_RGBA; }
	virtual GLenum glType() const { return GL_UNSIGNED_SHORT_4_4_4_4; }
};

// RGB565 texture
class GLES565Texture : public GLESTexture {
protected:
	virtual byte bytesPerPixel() const { return 2; }
	virtual GLenum glFormat() const { return GL_RGB; }
	virtual GLenum glType() const { return GL_UNSIGNED_SHORT_5_6_5; }
};

// RGB888 256-entry paletted texture
class GLESPaletteTexture : public GLESTexture {
public:
	GLESPaletteTexture();
	virtual ~GLESPaletteTexture();
	virtual void allocBuffer(GLuint width, GLuint height);
	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
							  const void* buf, int pitch);
	Graphics::Surface* surface() {
		setDirty();
		return &_surface;
	}
	void* pixels() {
		setDirty();
		return _surface.pixels;
	}
	const byte* palette_const() const { return _texture; };
	byte* palette() {
		setDirty();
		return _texture;
	};
	virtual void drawTexture() {
		drawTexture(0, 0, _surface.w, _surface.h);
	}
	virtual void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);
	virtual void fillBuffer(byte x);
protected:
	virtual byte bytesPerPixel() const { return 1; }
	virtual GLenum glFormat() const { return GL_RGB; }
	virtual GLenum glType() const { return GL_PALETTE8_RGB8_OES; }
	virtual size_t paletteSize() const { return 256 * 3; };
	virtual void uploadTexture() const;
	byte* _texture;
};

// RGBA8888 256-entry paletted texture
class GLESPaletteATexture : public GLESPaletteTexture {
protected:
	virtual GLenum glFormat() const { return GL_RGBA; }
	virtual GLenum glType() const { return GL_PALETTE8_RGBA8_OES; }
	virtual size_t paletteSize() const { return 256 * 4; };
};

#endif
