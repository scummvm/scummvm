/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_OPENGL_TEXTURE_H
#define GRAPHICS_OPENGL_TEXTURE_H

#include "graphics/opengl/system_headers.h"

#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)

namespace OpenGL {

class TextureGL {
public:
	TextureGL(const Graphics::Surface &srf);
	TextureGL(uint width, uint height);
	TextureGL(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height);
	virtual ~TextureGL();

	GLuint getTextureName() const { return _texture; }
	uint getWidth() const { return _width; }
	uint getHeight() const { return _height; }
	uint getTexWidth() const { return _texWidth; }
	uint getTexHeight() const { return _texHeight; }

	static const Graphics::PixelFormat getRGBAPixelFormat();
	static const Graphics::PixelFormat get565PixelFormat();

protected:
	bool _managedTexture;
	GLuint _texture;
	uint _width, _height;
	uint _texWidth, _texHeight;
};

} // End of namespace OpenGL

#endif

#endif
