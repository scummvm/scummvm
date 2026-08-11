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

#ifndef GRAPHICS_OPENGL_TEXTURE_H
#define GRAPHICS_OPENGL_TEXTURE_H

#include "graphics/opengl/system_headers.h"
#include "graphics/opengl/context.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/rect.h"
#include "common/rotationmode.h"

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
class Texture {
public:
	/**
	 * Constrcut a new GL texture object.
	 *
	 * @param glIntFormat The internal format to use.
	 * @param glFormat    The input format.
	 * @param glType      The input type.
	 */
	Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, bool autoCreate = true);
	~Texture();

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
	 *
	 * @return Whether a texture really exists
	 */
	bool bind() const;

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
	 * Sets the flip and rotate parameters of the texture
	 *
	 * @param flip     Whether to flip vertically the texture when displaying it.
	 */
	void setFlip(bool flip) { if (_flip != flip) { _flip = flip; computeTexCoords(); } }

	/**
	 * Sets the rotate parameter of the texture
	 *
	 * @param rotation How to rotate the texture
	 */
	void setRotation(Common::RotationMode rotation) { if (_rotation != rotation) { _rotation = rotation; computeTexCoords(); } }

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

	static inline const Graphics::PixelFormat getRGBPixelFormat() {
		return Graphics::PixelFormat::createFormatRGB24();
	}

	static inline const Graphics::PixelFormat getRGBAPixelFormat() {
		return Graphics::PixelFormat::createFormatRGBA32();
	}

	static inline const Graphics::PixelFormat getBGRAPixelFormat() {
		return Graphics::PixelFormat::createFormatBGRA32();
	}

protected:
	void computeTexCoords();

	const GLenum _glIntFormat;
	const GLenum _glFormat;
	const GLenum _glType;

	uint _width, _height;
	uint _logicalWidth, _logicalHeight;
	bool _flip;
	Common::RotationMode _rotation;

	GLfloat _texCoords[4*2];

	GLint _glFilter;

	GLuint _glTexture;
};

} // End of namespace OpenGL

#endif
