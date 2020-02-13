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

class Shader;

/**
 * A simple GL texture object abstraction.
 *
 * This is used for low-level GL texture handling.
 */
class GLTexture {
public:
	/**
	 * Constrcut a new GL texture object.
	 *
	 * @param glIntFormat The internal format to use.
	 * @param glFormat    The input format.
	 * @param glType      The input type.
	 */
	GLTexture(GLenum glIntFormat, GLenum glFormat, GLenum glType);
	~GLTexture();

	/**
	 * Enable or disable linear texture filtering.
	 *
	 * @param enable true to enable and false to disable.
	 */
	void enableLinearFiltering(bool enable);

	/**
	 * Test whether linear filtering is enabled.
	 */
	bool isLinearFilteringEnabled() const { return (_glFilter == GL_LINEAR); }

	/**
	 * Destroy the OpenGL texture name.
	 */
	void destroy();

	/**
	 * Create the OpenGL texture name.
	 */
	void create();

	/**
	 * Bind the texture to the active texture unit.
	 */
	void bind() const;

	/**
	 * Sets the size of the texture in pixels.
	 *
	 * The internal OpenGL texture might have a different size. To query the
	 * actual size use getWidth()/getHeight().
	 *
	 * @param width  The desired logical width.
	 * @param height The desired logical height.
	 */
	void setSize(uint width, uint height);

	/**
	 * Copy image data to the texture.
	 *
	 * @param area     The area to update.
	 * @param src      Surface for the whole texture containing the pixel data
	 *                 to upload. Only the area described by area will be
	 *                 uploaded.
	 */
	void updateArea(const Common::Rect &area, const Graphics::Surface &src);

	/**
	 * Query the GL texture's width.
	 */
	uint getWidth() const { return _width; }

	/**
	 * Query the GL texture's height.
	 */
	uint getHeight() const { return _height; }

	/**
	 * Query the logical texture's width.
	 */
	uint getLogicalWidth() const { return _logicalWidth; }

	/**
	 * Query the logical texture's height.
	 */
	uint getLogicalHeight() const { return _logicalHeight; }

	/**
	 * Obtain texture coordinates for rectangular drawing.
	 */
	const GLfloat *getTexCoords() const { return _texCoords; }

	/**
	 * Obtain texture name.
	 *
	 * Beware that the texture name changes whenever create is used.
	 * destroy will invalidate the texture name.
	 */
	GLuint getGLTexture() const { return _glTexture; }
private:
	const GLenum _glIntFormat;
	const GLenum _glFormat;
	const GLenum _glType;

	uint _width, _height;
	uint _logicalWidth, _logicalHeight;
	GLfloat _texCoords[4*2];

	GLint _glFilter;

	GLuint _glTexture;
};

/**
 * Interface for OpenGL implementations of a 2D surface.
 */
class Surface {
public:
	Surface();
	virtual ~Surface() {}

	/**
	 * Destroy OpenGL description of surface.
	 */
	virtual void destroy() = 0;

	/**
	 * Recreate OpenGL description of surface.
	 */
	virtual void recreate() = 0;

	/**
	 * Enable or disable linear texture filtering.
	 *
	 * @param enable true to enable and false to disable.
	 */
	virtual void enableLinearFiltering(bool enable) = 0;

	/**
	 * Allocate storage for surface.
	 *
	 * @param width  The desired logical width.
	 * @param height The desired logical height.
	 */
	virtual void allocate(uint width, uint height) = 0;

	/**
	 * Copy image data to the surface.
	 *
	 * The format of the input data needs to match the format returned by
	 * getFormat.
	 *
	 * @param x        X coordinate of upper left corner to copy data to.
	 * @param y        Y coordinate of upper left corner to copy data to.
	 * @param w        Width of the image data to copy.
	 * @param h        Height of the image data to copy.
	 * @param src      Pointer to image data.
	 * @param srcPitch The number of bytes in a row of the image data.
	 */
	void copyRectToTexture(uint x, uint y, uint w, uint h, const void *src, uint srcPitch);

	/**
	 * Fill the surface with a fixed color.
	 *
	 * @param color Color value in format returned by getFormat.
	 */
	void fill(uint32 color);

	void flagDirty() { _allDirty = true; }
	virtual bool isDirty() const { return _allDirty || !_dirtyArea.isEmpty(); }

	virtual uint getWidth() const = 0;
	virtual uint getHeight() const = 0;

	/**
	 * @return The logical format of the texture data.
	 */
	virtual Graphics::PixelFormat getFormat() const = 0;

	virtual Graphics::Surface *getSurface() = 0;
	virtual const Graphics::Surface *getSurface() const = 0;

	/**
	 * @return Whether the surface is having a palette.
	 */
	virtual bool hasPalette() const { return false; }

	/**
	 * Set color key for paletted textures.
	 *
	 * This needs to be called after any palette update affecting the color
	 * key. Calling this multiple times will result in multiple color indices
	 * to be treated as color keys.
	 */
	virtual void setColorKey(uint colorKey) {}
	virtual void setPalette(uint start, uint colors, const byte *palData) {}

	/**
	 * Update underlying OpenGL texture to reflect current state.
	 */
	virtual void updateGLTexture() = 0;

	/**
	 * Obtain underlying OpenGL texture.
	 */
	virtual const GLTexture &getGLTexture() const = 0;
protected:
	void clearDirty() { _allDirty = false; _dirtyArea = Common::Rect(); }

	Common::Rect getDirtyArea() const;
private:
	bool _allDirty;
	Common::Rect _dirtyArea;
};

/**
 * An OpenGL texture wrapper. It automatically takes care of all OpenGL
 * texture handling issues and also provides access to the texture data.
 */
class Texture : public Surface {
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

	virtual void destroy();

	virtual void recreate();

	virtual void enableLinearFiltering(bool enable);

	virtual void allocate(uint width, uint height);

	virtual uint getWidth() const { return _userPixelData.w; }
	virtual uint getHeight() const { return _userPixelData.h; }

	/**
	 * @return The logical format of the texture data.
	 */
	virtual Graphics::PixelFormat getFormat() const { return _format; }

	virtual Graphics::Surface *getSurface() { return &_userPixelData; }
	virtual const Graphics::Surface *getSurface() const { return &_userPixelData; }

	virtual void updateGLTexture();
	virtual const GLTexture &getGLTexture() const { return _glTexture; }
protected:
	const Graphics::PixelFormat _format;

private:
	GLTexture _glTexture;

	Graphics::Surface _textureData;
	Graphics::Surface _userPixelData;
};

class TextureCLUT8 : public Texture {
public:
	TextureCLUT8(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format);
	virtual ~TextureCLUT8();

	virtual void allocate(uint width, uint height);

	virtual Graphics::PixelFormat getFormat() const;

	virtual bool hasPalette() const { return true; }

	virtual void setColorKey(uint colorKey);
	virtual void setPalette(uint start, uint colors, const byte *palData);

	virtual Graphics::Surface *getSurface() { return &_clut8Data; }
	virtual const Graphics::Surface *getSurface() const { return &_clut8Data; }

	virtual void updateGLTexture();
private:
	Graphics::Surface _clut8Data;
	byte *_palette;
};

#if !USE_FORCED_GL
class FakeTexture : public Texture {
public:
	FakeTexture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format);
	virtual ~FakeTexture();

	virtual void allocate(uint width, uint height);

	virtual Graphics::PixelFormat getFormat() const = 0;

	virtual Graphics::Surface *getSurface() { return &_rgbData; }
	virtual const Graphics::Surface *getSurface() const { return &_rgbData; }
protected:
	Graphics::Surface _rgbData;
};

class TextureRGB555 : public FakeTexture {
public:
	TextureRGB555();
	virtual ~TextureRGB555() {};

	virtual Graphics::PixelFormat getFormat() const;

	virtual void updateGLTexture();
};

class TextureRGBA8888Swap : public FakeTexture {
public:
	TextureRGBA8888Swap();
	virtual ~TextureRGBA8888Swap() {};

	virtual Graphics::PixelFormat getFormat() const;

	virtual void updateGLTexture();
};
#endif // !USE_FORCED_GL

#if !USE_FORCED_GLES
class TextureTarget;
class CLUT8LookUpPipeline;

class TextureCLUT8GPU : public Surface {
public:
	TextureCLUT8GPU();
	virtual ~TextureCLUT8GPU();

	virtual void destroy();

	virtual void recreate();

	virtual void enableLinearFiltering(bool enable);

	virtual void allocate(uint width, uint height);

	virtual bool isDirty() const { return _paletteDirty || Surface::isDirty(); }

	virtual uint getWidth() const { return _userPixelData.w; }
	virtual uint getHeight() const { return _userPixelData.h; }

	virtual Graphics::PixelFormat getFormat() const;

	virtual bool hasPalette() const { return true; }

	virtual void setColorKey(uint colorKey);
	virtual void setPalette(uint start, uint colors, const byte *palData);

	virtual Graphics::Surface *getSurface() { return &_userPixelData; }
	virtual const Graphics::Surface *getSurface() const { return &_userPixelData; }

	virtual void updateGLTexture();
	virtual const GLTexture &getGLTexture() const;

	static bool isSupportedByContext() {
		return g_context.shadersSupported
		    && g_context.multitextureSupported
		    && g_context.framebufferObjectSupported;
	}
private:
	void lookUpColors();

	GLTexture _clut8Texture;
	GLTexture _paletteTexture;

	TextureTarget *_target;
	CLUT8LookUpPipeline *_clut8Pipeline;

	GLfloat _clut8Vertices[4*2];

	Graphics::Surface _clut8Data;
	Graphics::Surface _userPixelData;

	byte _palette[4 * 256];
	bool _paletteDirty;
};
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL

#endif
