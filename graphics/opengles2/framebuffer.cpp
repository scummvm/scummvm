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

#if defined(USE_OPENGL) && !defined(AMIGAOS)

#ifdef USE_OPENGL_SHADERS
#include "graphics/opengles2/framebuffer.h"
#elif defined(SDL_BACKEND)
#define GL_GLEXT_PROTOTYPES // For the GL_EXT_framebuffer_object extension
#include "graphics/opengles2/framebuffer.h"
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#define GL_STENCIL_ATTACHMENT GL_STENCIL_ATTACHMENT_EXT
#define GL_STENCIL_INDEX8 GL_STENCIL_INDEX8_EXT
#define GL_DEPTH24_STENCIL8 0x88F0
#include "backends/platform/sdl/sdl-sys.h"
#endif // defined(SDL_BACKEND)

#include "graphics/opengles2/extensions.h"

#ifdef USE_GLES2
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

namespace Graphics {

#if defined(SDL_BACKEND) && !defined(USE_GLEW)
static bool framebuffer_object_functions = false;
static PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
static PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
static PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
static PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffers;
static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
static PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
static PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
static PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;

static void grabFramebufferObjectPointers() {
	if (framebuffer_object_functions)
		return;
	framebuffer_object_functions = true;

	union {
		void *obj_ptr;
		void (APIENTRY *func_ptr)();
	} u;
	// We're casting from an object pointer to a function pointer, the
	// sizes need to be the same for this to work.
	assert(sizeof(u.obj_ptr) == sizeof(u.func_ptr));
	u.obj_ptr = SDL_GL_GetProcAddress("glBindFramebuffer");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glBindRenderbuffer");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glCheckFramebufferStatus");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glDeleteFramebuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glDeleteRenderbuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glGenFramebuffers");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glGenRenderbuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)u.func_ptr;
	u.obj_ptr = SDL_GL_GetProcAddress("glRenderbufferStorage");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)u.func_ptr;
}
#endif // defined(SDL_BACKEND) && !defined(USE_OPENGL_SHADERS)

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
#ifndef USE_OPENGL_SHADERS
	return Graphics::isExtensionSupported("GL_EXT_packed_depth_stencil");
#endif
	return true;
}

FrameBuffer::FrameBuffer(uint width, uint height) :
		_managedTexture(true), _width(width), _height(height),
		_texWidth(nextHigher2(width)), _texHeight(nextHigher2(height)) {
#ifdef SDL_BACKEND
	if (!Graphics::isExtensionSupported("GL_EXT_framebuffer_object")) {
		error("GL_EXT_framebuffer_object extension is not supported!");
	}
#endif
#if defined(SDL_BACKEND) && !defined(USE_GLEW)
	grabFramebufferObjectPointers();
#endif
	glGenTextures(1, &_colorTexture);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	init();
}

FrameBuffer::FrameBuffer(GLuint texture_name, uint width, uint height, uint texture_width, uint texture_height) :
		_managedTexture(false), _colorTexture(texture_name), _width(width), _height(height),
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
