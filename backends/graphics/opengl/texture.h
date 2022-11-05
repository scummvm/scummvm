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

#ifndef BACKENDS_GRAPHICS_OPENGL_TEXTURE_H
#define BACKENDS_GRAPHICS_OPENGL_TEXTURE_H

#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/context.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/rect.h"

class Scaler;

namespace OpenGL {

enum WrapMode {
	kWrapModeBorder,
	kWrapModeEdge,
	kWrapModeRepeat,
	kWrapModeMirroredRepeat
};

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
	 * Enable or disable linear texture filtering.
	 *
	 * @param enable true to enable and false to disable.
	 */
	void setWrapMode(WrapMode wrapMode);

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
	 * @return Whether the call was successful
	 */
	bool setSize(uint width, uint height);

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

	virtual void setScaler(uint scalerIndex, int scaleFactor) {}

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
	~Texture() override;

	void destroy() override;

	void recreate() override;

	void enableLinearFiltering(bool enable) override;

	void allocate(uint width, uint height) override;

	uint getWidth() const override { return _userPixelData.w; }
	uint getHeight() const override { return _userPixelData.h; }

	/**
	 * @return The logical format of the texture data.
	 */
	Graphics::PixelFormat getFormat() const override { return _format; }

	Graphics::Surface *getSurface() override { return &_userPixelData; }
	const Graphics::Surface *getSurface() const override { return &_userPixelData; }

	void updateGLTexture() override;
	const GLTexture &getGLTexture() const override { return _glTexture; }
protected:
	const Graphics::PixelFormat _format;

	void updateGLTexture(Common::Rect &dirtyArea);

private:
	GLTexture _glTexture;

	Graphics::Surface _textureData;
	Graphics::Surface _userPixelData;
};

class FakeTexture : public Texture {
public:
	FakeTexture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format, const Graphics::PixelFormat &fakeFormat);
	~FakeTexture() override;

	void allocate(uint width, uint height) override;

	Graphics::PixelFormat getFormat() const override { return _fakeFormat; }

	bool hasPalette() const override { return (_palette != nullptr); }

	void setColorKey(uint colorKey) override;
	void setPalette(uint start, uint colors, const byte *palData) override;

	Graphics::Surface *getSurface() override { return &_rgbData; }
	const Graphics::Surface *getSurface() const override { return &_rgbData; }

	void updateGLTexture() override;
protected:
	Graphics::Surface _rgbData;
	Graphics::PixelFormat _fakeFormat;
	uint32 *_palette;
};

class TextureRGB555 : public FakeTexture {
public:
	TextureRGB555();
	~TextureRGB555() override {}

	void updateGLTexture() override;
};

class TextureRGBA8888Swap : public FakeTexture {
public:
	TextureRGBA8888Swap();
	~TextureRGBA8888Swap() override {}

	void updateGLTexture() override;
};

#ifdef USE_SCALERS
class ScaledTexture : public FakeTexture {
public:
	ScaledTexture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format, const Graphics::PixelFormat &fakeFormat);
	~ScaledTexture() override;

	void allocate(uint width, uint height) override;

	uint getWidth() const override { return _rgbData.w; }
	uint getHeight() const override { return _rgbData.h; }
	Graphics::PixelFormat getFormat() const override { return _fakeFormat; }

	bool hasPalette() const override { return (_palette != nullptr); }

	Graphics::Surface *getSurface() override { return &_rgbData; }
	const Graphics::Surface *getSurface() const override { return &_rgbData; }

	void updateGLTexture() override;

	void setScaler(uint scalerIndex, int scaleFactor) override;
protected:
	Graphics::Surface *_convData;
	Scaler *_scaler;
	uint _scalerIndex;
	uint _extraPixels;
	uint _scaleFactor;
};
#endif

#if !USE_FORCED_GLES
class TextureTarget;
class CLUT8LookUpPipeline;

class TextureCLUT8GPU : public Surface {
public:
	TextureCLUT8GPU();
	~TextureCLUT8GPU() override;

	void destroy() override;

	void recreate() override;

	void enableLinearFiltering(bool enable) override;

	void allocate(uint width, uint height) override;

	bool isDirty() const override { return _paletteDirty || Surface::isDirty(); }

	uint getWidth() const override { return _userPixelData.w; }
	uint getHeight() const override { return _userPixelData.h; }

	Graphics::PixelFormat getFormat() const override;

	bool hasPalette() const override { return true; }

	void setColorKey(uint colorKey) override;
	void setPalette(uint start, uint colors, const byte *palData) override;

	Graphics::Surface *getSurface() override { return &_userPixelData; }
	const Graphics::Surface *getSurface() const override { return &_userPixelData; }

	void updateGLTexture() override;
	const GLTexture &getGLTexture() const override;

	static bool isSupportedByContext() {
		return OpenGLContext.shadersSupported
		    && OpenGLContext.multitextureSupported
		    && OpenGLContext.framebufferObjectSupported;
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
