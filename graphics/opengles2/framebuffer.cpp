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

static bool usePackedBuffer() {
#ifdef USE_GLES2
	return Graphics::isExtensionSupported("GL_OES_packed_depth_stencil");
#endif
	return true;
}

FrameBuffer::FrameBuffer(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height) : _colorTexture(texture_name), _width(width), _height(height) {
	glGenFramebuffers(1, &_frameBuffer);
	glGenRenderbuffers(2, &_renderBuffers[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_name, 0);

if (usePackedBuffer()) {
	glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, texture_width, texture_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
} else {
	glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture_width, texture_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);

	glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, texture_width, texture_height);
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

FrameBuffer::~FrameBuffer() {
	glDeleteRenderbuffers(2, &_renderBuffers[0]);
	glDeleteFramebuffers(1, &_frameBuffer);
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
