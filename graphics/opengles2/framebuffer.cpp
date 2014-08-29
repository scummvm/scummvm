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

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)

#include "graphics/opengles2/framebuffer.h"
#include "graphics/opengles2/extensions.h"

#ifdef USE_GLES2
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

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


static bool usePackedBuffer() {
#ifdef USE_GLES2
	return Graphics::isExtensionSupported("GL_OES_packed_depth_stencil");
#endif
	return true;
}

FrameBuffer::FrameBuffer(uint width, uint height)
	: _managedTexture(true), _width(width), _height(height),
	  _texWidth(nextHigher2(width)), _texHeight(nextHigher2(height)) {
	glGenTextures(1, &_colorTexture);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	init();
}

FrameBuffer::FrameBuffer(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height)
	: _managedTexture(false), _colorTexture(texture_name), _width(width), _height(height),
	  _texWidth(texture_width), _texHeight(texture_height) {
	init();
}

FrameBuffer::~FrameBuffer() {
	glDeleteRenderbuffers(2, &_renderBuffers[0]);
	glDeleteFramebuffers(1, &_frameBuffer);
	if (_managedTexture)
		glDeleteTextures(1, &_colorTexture);
}

void FrameBuffer::init() {
	glGenFramebuffers(1, &_frameBuffer);
	glGenRenderbuffers(2, &_renderBuffers[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);

	if (usePackedBuffer()) {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _texWidth, _texHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _texWidth, _texHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);

		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[1]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, _texWidth, _texHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[1]);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	GLenum status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		error("Framebuffer is not complete! status: %d", status);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attach() {
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glViewport(0,0, _width, _height);

	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::detach() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}

#endif
