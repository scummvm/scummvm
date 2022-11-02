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

#ifndef BACKENDS_GRAPHICS3D_OPENGL_TEXTURE_H
#define BACKENDS_GRAPHICS3D_OPENGL_TEXTURE_H

#include "graphics/opengl/system_headers.h"

#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

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
