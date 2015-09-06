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

#include "common/textconsole.h"

#if defined(USE_OPENGL)

#include "graphics/opengl/texture.h"
#include "graphics/opengl/extensions.h"


namespace Graphics {

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * 8; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

static const Graphics::PixelFormat getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

Texture::Texture(const Surface &srf) :
		_managedTexture(true), _width(srf.w), _height(srf.h) {
	if (Graphics::isExtensionSupported("GL_ARB_texture_non_power_of_two")) {
		_texWidth  = _width;
		_texHeight = _height;
	} else {
		_texWidth  = nextHigher2(_width);
		_texHeight = nextHigher2(_height);
	}
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	if (srf.format != getRGBAPixelFormat()) {
		Graphics::Surface *srf2 = srf.convertTo(getRGBAPixelFormat());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, srf2->getPixels());

		srf2->free();
		delete srf2;
	} else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, srf.getPixels());
	}
}

Texture::Texture(uint width, uint height) :
		_managedTexture(true), _width(width), _height(height) {
	if (Graphics::isExtensionSupported("GL_ARB_texture_non_power_of_two")) {
		_texWidth  = _width;
		_texHeight = _height;
	} else {
		_texWidth  = nextHigher2(_width);
		_texHeight = nextHigher2(_height);
	}
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

Texture::Texture(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height) :
		_managedTexture(false), _texture(texture_name), _width(width), _height(height),
		_texWidth(texture_width), _texHeight(texture_height) {
}

Texture::~Texture() {
	if (_managedTexture) {
		glDeleteTextures(1, &_texture);
	}
}

}

#endif
