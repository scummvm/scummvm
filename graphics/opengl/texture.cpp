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

#include "graphics/opengl/system_headers.h"

#ifdef USE_OPENGL

#include "graphics/opengl/texture.h"
#include "graphics/opengl/debug.h"

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/textconsole.h"

namespace OpenGL {

Texture::Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, bool autoCreate)
	: _glIntFormat(glIntFormat), _glFormat(glFormat), _glType(glType),
	  _width(0), _height(0), _logicalWidth(0), _logicalHeight(0),
	  _flip(false), _rotation(Common::kRotationNormal),
	  _texCoords(), _glFilter(GL_NEAREST), _glTexture(0) {
	if (autoCreate)
		create();
}

Texture::~Texture() {
	GL_CALL_SAFE(glDeleteTextures, (1, &_glTexture));
}

void Texture::enableLinearFiltering(bool enable) {
	if (enable) {
		_glFilter = GL_LINEAR;
	} else {
		_glFilter = GL_NEAREST;
	}

	if (!bind()) {
		return;
	}

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void Texture::setWrapMode(WrapMode wrapMode) {
	GLuint glwrapMode;

	switch(wrapMode) {
		case kWrapModeBorder:
			if (OpenGLContext.textureBorderClampSupported) {
				glwrapMode = GL_CLAMP_TO_BORDER;
				break;
			}
		// fall through
		case kWrapModeEdge:
			if (OpenGLContext.textureEdgeClampSupported) {
				glwrapMode = GL_CLAMP_TO_EDGE;
				break;
			} else {
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
				// Fallback on clamp
				glwrapMode = GL_CLAMP;
#else
				// This fallback should never happen in real life (GLES/GLES2 have border/edge clamp)
				glwrapMode = GL_REPEAT;
#endif
				break;
			}
		case kWrapModeMirroredRepeat:
			if (OpenGLContext.textureMirrorRepeatSupported) {
				glwrapMode = GL_MIRRORED_REPEAT;
				break;
			}
		// fall through
		case kWrapModeRepeat:
		default:
			glwrapMode = GL_REPEAT;
	}


	if (!bind()) {
		return;
	}

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glwrapMode));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glwrapMode));
}

void Texture::destroy() {
	if (!_glTexture) {
		return;
	}
	GL_CALL(glDeleteTextures(1, &_glTexture));
	_glTexture = 0;
}

void Texture::create() {
	// Release old texture name in case it exists.
	destroy();

	// Get a new texture name.
	GL_CALL(glGenTextures(1, &_glTexture));

	// Set up all texture parameters.
	bind();
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	if (OpenGLContext.textureEdgeClampSupported) {
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	} else {
#if !USE_FORCED_GLES && !USE_FORCED_GLES2
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
#endif
	}

	// If a size is specified, allocate memory for it.
	if (_width != 0 && _height != 0) {
		// Allocate storage for OpenGL texture.
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _width, _height,
		                     0, _glFormat, _glType, nullptr));
	}
}

bool Texture::bind() const {
	if (!_glTexture) {
		return false;
	}
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _glTexture));
	return true;
}

bool Texture::setSize(uint width, uint height) {
	const uint oldWidth  = _width;
	const uint oldHeight = _height;

	_logicalWidth  = width;
	_logicalHeight = height;

	if (!OpenGLContext.NPOTSupported) {
		_width  = Common::nextHigher2(width);
		_height = Common::nextHigher2(height);
	} else {
		_width  = width;
		_height = height;
	}

	// If a size is specified, allocate memory for it.
	if (width != 0 && height != 0) {
		computeTexCoords();

		// Allocate storage for OpenGL texture if necessary.
		if (oldWidth != _width || oldHeight != _height) {
			if (!bind()) {
				return false;
			}

			bool error;
			GL_CALL_CHECK(error, glTexImage2D(GL_TEXTURE_2D, 0, _glIntFormat, _width, _height,
			             0, _glFormat, _glType, nullptr));
			if (error) {
				return false;
			}
		}
	}
	return true;
}

void Texture::computeTexCoords() {
	const GLfloat texWidth = (GLfloat)_logicalWidth / _width;
	const GLfloat texHeight = (GLfloat)_logicalHeight / _height;

	if (_flip) {
		_texCoords[0] = 0;
		_texCoords[1] = texHeight;

		_texCoords[2] = texWidth;
		_texCoords[3] = texHeight;

		_texCoords[4] = 0;
		_texCoords[5] = 0;

		_texCoords[6] = texWidth;
		_texCoords[7] = 0;
	} else {
		_texCoords[0] = 0;
		_texCoords[1] = 0;

		_texCoords[2] = texWidth;
		_texCoords[3] = 0;

		_texCoords[4] = 0;
		_texCoords[5] = texHeight;

		_texCoords[6] = texWidth;
		_texCoords[7] = texHeight;
	}

	switch(_rotation) {
	default:
	case Common::kRotationNormal:
		// Nothing to do
		break;
	case Common::kRotation90:
		// LT -> LB and RB -> RT
		SWAP(_texCoords[1], _texCoords[7]);
		// RT -> LT and LB -> RB
		SWAP(_texCoords[2], _texCoords[4]);
		break;
	case Common::kRotation180:
		// LT -> RT and RT -> LT
		SWAP(_texCoords[0], _texCoords[2]);
		// RT -> RB and LB -> LT
		SWAP(_texCoords[1], _texCoords[5]);
		// LT -> LB and RB -> RT
		SWAP(_texCoords[3], _texCoords[7]);
		// LT -> RT and RT -> LT
		SWAP(_texCoords[4], _texCoords[6]);
		break;
	case Common::kRotation270:
		// LT -> RT and RB -> LB
		SWAP(_texCoords[0], _texCoords[6]);
		// RT -> RB and LB -> LT
		SWAP(_texCoords[3], _texCoords[5]);
		break;
	}
}

void Texture::updateArea(const Common::Rect &area, const Graphics::Surface &src) {
	// Set the texture on the active texture unit.
	if (!bind()) {
		return;
	}

	// Update the actual texture.
	// Although we have the area of the texture buffer we want to update we
	// cannot take advantage of the left/right boundaries here because it is
	// not possible to specify a pitch to glTexSubImage2D. To be precise, with
	// plain OpenGL we could set GL_UNPACK_ROW_LENGTH to achieve this. However,
	// OpenGL ES 1.0 does not support GL_UNPACK_ROW_LENGTH. Thus, we are left
	// with the following options:
	//
	// 1) (As we do right now) Simply always update the whole texture lines of
	//    rect changed. This is simplest to implement. In case performance is
	//    really an issue we can think of switching to another method.
	//
	// 2) Copy the dirty rect to a temporary buffer and upload that by using
	//    glTexSubImage2D. This is what the Android backend does. It is more
	//    complicated though.
	//
	// 3) Use glTexSubImage2D per line changed. This is what the old OpenGL
	//    graphics manager did but it is much slower! Thus, we do not use it.
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, area.top, src.w, area.height(),
	                       _glFormat, _glType, src.getBasePtr(0, area.top)));
}

} // End of namespace OpenGL

#endif

