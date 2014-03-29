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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGL_TEXTURE_H
#define BACKENDS_GRAPHICS_OPENGL_TEXTURE_H

#include "backends/graphics/opengl/opengl-sys.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/rect.h"

namespace OpenGL {

/**
 * An OpenGL texture wrapper. It automatically takes care of all OpenGL
 * texture handling issues and also provides access to the texture data.
 */
class Texture {
public:
	/**
	 * Create a new texture with the specific internal format.
	 *
	 * @param glIntFormat The internal format to use.
	 * @param glFormat    The input format.
	 * @param glType      The input type.
	 * @param format      The format used for the texture input.
	 */
	Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format);
	virtual ~Texture();

	/**
	 * Destroy the OpenGL texture name.
	 */
	void releaseInternalTexture();

	/**
	 * Create the OpenGL texture name and flag the whole texture as dirty.
	 */
	void recreateInternalTexture();

	/**
	 * Enable or disable linear texture filtering.
	 *
	 * @param enable true to enable and false to disable.
	 */
	void enableLinearFiltering(bool enable);

	/**
	 * Allocate texture space for the desired dimensions. This wraps any
	 * handling of requirements for POT textures.
	 *
	 * @param width  The desired logical width.
	 * @param height The desired logical height.
	 */
	virtual void allocate(uint width, uint height);

	void copyRectToTexture(uint x, uint y, uint w, uint h, const void *src, uint srcPitch);

	void fill(uint32 color);

	void draw(GLfloat x, GLfloat y, GLfloat w, GLfloat h);

	void flagDirty() { _allDirty = true; }
	bool isDirty() const { return _allDirty || !_dirtyArea.isEmpty(); }

	uint getWidth() const { return _userPixelData.w; }
	uint getHeight() const { return _userPixelData.h; }

	/**
	 * @return The hardware format of the texture data.
	 */
	const Graphics::PixelFormat &getHardwareFormat() const { return _format; }

	/**
	 * @return The logical format of the texture data.
	 */
	virtual Graphics::PixelFormat getFormat() const { return _format; }

	virtual Graphics::Surface *getSurface() { return &_userPixelData; }
	virtual const Graphics::Surface *getSurface() const { return &_userPixelData; }

	/**
	 * @return Whether the texture data is using a palette.
	 */
	virtual bool hasPalette() const { return false; }

	virtual void setPalette(uint start, uint colors, const byte *palData) {}

	virtual void *getPalette() { return 0; }
	virtual const void *getPalette() const { return 0; }

	/**
	 * Query texture related OpenGL information from the context. This only
	 * queries the maximum texture size for now.
	 */
	static void queryTextureInformation();

	/**
	 * @return Return the maximum texture dimensions supported.
	 */
	static GLint getMaximumTextureSize() { return _maxTextureSize; }
protected:
	virtual void updateTexture();

	Common::Rect getDirtyArea() const;
private:
	const GLenum _glIntFormat;
	const GLenum _glFormat;
	const GLenum _glType;
	const Graphics::PixelFormat _format;

	GLint _glFilter;
	GLuint _glTexture;

	Graphics::Surface _textureData;
	Graphics::Surface _userPixelData;

	bool _allDirty;
	Common::Rect _dirtyArea;
	void clearDirty() { _allDirty = false; _dirtyArea = Common::Rect(); }

	static GLint _maxTextureSize;
};

class TextureCLUT8 : public Texture {
public:
	TextureCLUT8(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format);
	virtual ~TextureCLUT8();

	virtual void allocate(uint width, uint height);

	virtual Graphics::PixelFormat getFormat() const;

	virtual bool hasPalette() const { return true; }

	virtual void setPalette(uint start, uint colors, const byte *palData);

	virtual void *getPalette() { return _palette; }
	virtual const void *getPalette() const { return _palette; }

	virtual Graphics::Surface *getSurface() { return &_clut8Data; }
	virtual const Graphics::Surface *getSurface() const { return &_clut8Data; }

protected:
	virtual void updateTexture();

private:
	Graphics::Surface _clut8Data;
	byte *_palette;
};

} // End of namespace OpenGL

#endif
