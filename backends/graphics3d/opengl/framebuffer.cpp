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

#include "common/textconsole.h"
#include "common/util.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics3d/opengl/framebuffer.h"

#include "graphics/opengl/context.h"

#if !defined(GL_DEPTH24_STENCIL8)
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif
#if !defined(GL_DEPTH_COMPONENT24)
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

namespace OpenGL {

static bool usePackedBuffer() {
	return OpenGLContext.packedDepthStencilSupported;
}

static bool useDepthComponent24() {
	return OpenGLContext.OESDepth24;
}

FrameBuffer::FrameBuffer(uint width, uint height) :
		Texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE) {
	if (!OpenGLContext.framebufferObjectSupported) {
		error("FrameBuffer Objects are not supported by the current OpenGL context");
	}

	setSize(width, height);
	enableLinearFiltering(true);

	init();
}

FrameBuffer::~FrameBuffer() {
	glDeleteRenderbuffers(2, _renderBuffers);
	glDeleteFramebuffers(1, &_frameBuffer);
}

void FrameBuffer::init() {
	glGenFramebuffers(1, &_frameBuffer);
	glGenRenderbuffers(2, _renderBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, getGLTexture(), 0);

	if (usePackedBuffer()) {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, getWidth(), getHeight());
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);

		const char *glVersion = (const char *)glGetString(GL_VERSION);
		if (strstr(glVersion, "WebGL 1.0") != NULL) {
			// See https://www.khronos.org/registry/webgl/specs/latest/1.0/#FBO_ATTACHMENTS
			// and https://github.com/emscripten-core/emscripten/issues/4832
#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL 0x84F9
#endif
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, getWidth(), getHeight());

#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#endif
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		} else {
			glRenderbufferStorage(GL_RENDERBUFFER, useDepthComponent24() ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16, getWidth(), getHeight());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);

			glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[1]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWidth(), getHeight());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[1]);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		error("Framebuffer is not complete! status: %d", status);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attach() {
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
	glViewport(0, 0, getLogicalWidth(), getLogicalHeight());
}

void FrameBuffer::detach() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#if !USE_FORCED_GLES2 || defined(USE_GLAD)
MultiSampleFrameBuffer::MultiSampleFrameBuffer(uint width, uint height, int samples)
		: FrameBuffer(width,height) {
	if (!OpenGLContext.framebufferObjectMultisampleSupported) {
		error("The current OpenGL context does not support multisample framebuffer objects!");
	}

	if (samples > OpenGLContext.multisampleMaxSamples) {
		warning("Requested anti-aliasing with '%d' samples, but the current OpenGL context supports '%d' samples at most",
		        samples, OpenGLContext.multisampleMaxSamples);
	}

	_msSamples = MIN(samples, OpenGLContext.multisampleMaxSamples);

	init();
}

MultiSampleFrameBuffer::~MultiSampleFrameBuffer() {
	glDeleteRenderbuffers(1, &_msColorId);
	glDeleteRenderbuffers(1, &_msDepthId);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &_msFrameBufferId);
}

void MultiSampleFrameBuffer::init() {
	glGenFramebuffers(1, &_msFrameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, _msFrameBufferId);

	glGenRenderbuffers(1, &_msColorId);
	glBindRenderbuffer(GL_RENDERBUFFER, _msColorId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, _msSamples, GL_RGBA8, getWidth(), getHeight());

	glGenRenderbuffers(1, &_msDepthId);
	glBindRenderbuffer(GL_RENDERBUFFER, _msDepthId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, _msSamples, GL_DEPTH24_STENCIL8, getWidth(), getHeight());

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msColorId);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _msDepthId);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _msDepthId);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		error("Framebuffer is not complete! status: %d", status);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MultiSampleFrameBuffer::attach() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, getFrameBufferName());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _msFrameBufferId);
	glViewport(0, 0, getLogicalWidth(), getLogicalHeight());
}

void MultiSampleFrameBuffer::detach() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _msFrameBufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getFrameBufferName());
	glBlitFramebuffer(0, 0, getLogicalWidth(), getLogicalHeight(), 0, 0, getLogicalWidth(), getLogicalHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif // !USE_FORCED_GLES2 || defined(USE_GLAD)

} // End of namespace OpenGL

#endif
