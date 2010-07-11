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

#ifdef WIN32
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE
#endif

#if defined(USE_GLES)
#include <GLES/gl.h>
#include <GLES/glext.h>
#elif defined(MACOSX)
#include <gl.h>
#include <glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "graphics/surface.h"

#include "common/rect.h"
#include "common/array.h"

/**
 * OpenGL texture manager class
 */
class GLTexture {
public:
	static void initGLExtensions();

	GLTexture();
	virtual ~GLTexture();

	virtual void refresh();

	virtual void allocBuffer(GLuint width, GLuint height);
	virtual void fillBuffer(byte x);
	virtual void updateBuffer(GLuint x, GLuint y, GLuint width, GLuint height,
							  const void* buf, int pitch);

	virtual void drawTexture() { drawTexture(0, 0, _surface.w, _surface.h); }
	virtual void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);

	const Graphics::Surface* getSurface() const { return &_surface; }

	GLuint getWidth() const { return _surface.w; }
	GLuint getHeight() const { return _surface.h; }
	GLuint getTextureName() const { return _textureName; }

protected:
	virtual byte bytesPerPixel() const = 0;
	virtual GLenum glFormat() const = 0;
	virtual GLenum glType() const = 0;
	Graphics::Surface _surface;
	GLuint _textureName;
	GLuint _textureWidth;
	GLuint _textureHeight;
};

/**
 * OpenGL RGBA 32 bit texture
 */
class GL32Texture : public GLTexture {
protected:
	virtual byte bytesPerPixel() const { return 4; }
	virtual GLenum glFormat() const { return GL_RGBA; }
	virtual GLenum glType() const { return GL_UNSIGNED_BYTE; }
};

/**
 * OpenGL RGBA4444 texture
 */
class GL4444Texture : public GLTexture {
protected:
	virtual byte bytesPerPixel() const { return 2; }
	virtual GLenum glFormat() const { return GL_RGBA; }
	virtual GLenum glType() const { return GL_UNSIGNED_SHORT_4_4_4_4; }
};

/**
 * OpenGL RGB565 texture
 */
class GL565Texture : public GLTexture {
protected:
	virtual byte bytesPerPixel() const { return 2; }
	virtual GLenum glFormat() const { return GL_RGB; }
	virtual GLenum glType() const { return GL_UNSIGNED_SHORT_5_6_5; }
};
