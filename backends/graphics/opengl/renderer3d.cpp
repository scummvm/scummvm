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

#include "common/scummsys.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)

#include "backends/graphics/opengl/renderer3d.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"

#include "common/system.h"
#include "common/textconsole.h"

namespace OpenGL {

static void setupRenderbufferStorage(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
#if !USE_FORCED_GLES2 || defined(USE_GLAD)
	if (samples > 1) {
		glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
		return;
	}
#endif
	glRenderbufferStorage(target, internalformat, width, height);
}

// This constructor must not depend on any existing GL context
Renderer3D::Renderer3D() :
	_texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false),
	_renderBuffers{0, 0, 0}, _frameBuffers{0, 0}, _renderToFrameBuffer(false),
	_samples(0), _stackLevel(0), _inOverlay(false),
	_pendingScreenChangeWidth(-1), _pendingScreenChangeHeight(-1) {
	_texture.enableLinearFiltering(true);
	_texture.setFlip(true);
}

void Renderer3D::destroy() {
	while (_stackLevel) {
		enter3D();
	}

	if (_frameBuffers[0]) {
		// Check that we did allocated some framebuffer before destroying them
		// This avoids to call glDeleteFramebuffers and glDeleteRenderbuffers
		// on platforms not supporting it
		glDeleteFramebuffers(ARRAYSIZE(_frameBuffers), _frameBuffers);
		glDeleteRenderbuffers(ARRAYSIZE(_renderBuffers), _renderBuffers);
		memset(_renderBuffers, 0, sizeof(_renderBuffers));
		memset(_frameBuffers, 0, sizeof(_frameBuffers));
	}

	_texture.destroy();
}

void Renderer3D::initSize(uint w, uint h, int samples, bool renderToFrameBuffer) {
	_samples = samples;
	_renderToFrameBuffer = renderToFrameBuffer;

	if (!renderToFrameBuffer) {
		destroy();
		_texture.setSize(0, 0);
		return;
	}

	_texture.setSize(w, h);
	recreate();
}

void Renderer3D::resize(uint w, uint h) {
	assert(!_stackLevel);

	if (!_renderToFrameBuffer) {
		return;
	}

	if (_inOverlay) {
		// While the (GUI) overlay is active, the game doesn't renders
		// So, instead of loosing the contents of the FBO because of a resize,
		// just delay it to when we close the GUI.
		_pendingScreenChangeWidth = w;
		_pendingScreenChangeHeight = h;
		return;
	}

	_texture.setSize(w, h);
	setup();

	// Rebind the framebuffer
	if (_frameBuffers[1]) {
		// We are using multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffers[1]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffers[0]);
	} else if (_frameBuffers[0]) {
		// Draw on framebuffer if one was setup
		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[0]);
	}
}

void Renderer3D::recreate() {
	destroy();

	if (!_renderToFrameBuffer) {
		// No framebuffer was requested
		return;
	}

	// A 1x antialiasing is not an antialiasing
	if (_samples > 1) {
#if !USE_FORCED_GLES2 || defined(USE_GLAD)
		if (!OpenGLContext.framebufferObjectMultisampleSupported) {
			warning("The current OpenGL context does not support multisample framebuffer objects");
			_samples = 0;
		}
		if (_samples > OpenGLContext.multisampleMaxSamples) {
			warning("Requested anti-aliasing with '%d' samples, but the current OpenGL context supports '%d' samples at most",
					_samples, OpenGLContext.multisampleMaxSamples);
			_samples = OpenGLContext.multisampleMaxSamples;
		}
#else
		warning("multisample framebuffer objects support is not compiled in");
		_samples = 0;
#endif
	} else {
		_samples = 0;
	}

	setup();

	// Context got destroyed
	_stackLevel = 0;
}

void Renderer3D::setup() {
	const bool multiSample = _samples > 1;
	const uint w = _texture.getLogicalWidth();
	const uint h = _texture.getLogicalHeight();

	if (!_texture.getGLTexture()) {
		_texture.create();
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!_frameBuffers[0]) {
		glGenFramebuffers(multiSample ? 2 : 1, _frameBuffers);
		glGenRenderbuffers(multiSample ? 3 : 2, _renderBuffers);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[multiSample ? 1 : 0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture.getGLTexture(), 0);

	if (multiSample) {
		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[0]);
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[2]);
		setupRenderbufferStorage(GL_RENDERBUFFER, _samples, GL_RGBA8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderBuffers[2]);
	}

#ifdef EMSCRIPTEN
	// See https://www.khronos.org/registry/webgl/specs/latest/1.0/#FBO_ATTACHMENTS
	glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
	setupRenderbufferStorage(GL_RENDERBUFFER, _samples, GL_DEPTH_STENCIL, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
#else
	if (OpenGLContext.packedDepthStencilSupported) {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
		setupRenderbufferStorage(GL_RENDERBUFFER, _samples, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);
	} else {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[0]);
		setupRenderbufferStorage(GL_RENDERBUFFER, _samples, OpenGLContext.OESDepth24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT16, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[0]);

		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffers[1]);
		setupRenderbufferStorage(GL_RENDERBUFFER, _samples, GL_STENCIL_INDEX8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _renderBuffers[1]);
	}
#endif
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		error("Framebuffer is not complete! status: %d", status);
	}

	if (multiSample) {
		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[1]);
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			error("Target framebuffer is not complete! status: %d", status);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer3D::leave3D() {
	if (OpenGLContext.type == kContextGL) {
		// Save current state (only available on OpenGL)
		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT |
		             GL_LIGHTING_BIT | GL_PIXEL_MODE_BIT | GL_SCISSOR_BIT |
		             GL_TEXTURE_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT | GL_CLIENT_VERTEX_ARRAY_BIT);

		// prepare view
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
	} else {
		// Save context by ourselves
#define CTX_STATE(gl_param) _save ## gl_param = glIsEnabled(gl_param)
#define CTX_BOOLEAN(gl_param) glGetBooleanv(gl_param, &_save ## gl_param)
#define CTX_INTEGER(gl_param, count) glGetIntegerv(gl_param, _save ## gl_param)

		CTX_STATE(GL_BLEND);
		CTX_STATE(GL_CULL_FACE);
		CTX_STATE(GL_DEPTH_TEST);
		CTX_STATE(GL_DITHER);
		CTX_STATE(GL_POLYGON_OFFSET_FILL);
		CTX_STATE(GL_SCISSOR_TEST);
		CTX_STATE(GL_STENCIL_TEST);

		CTX_BOOLEAN(GL_DEPTH_WRITEMASK);

		CTX_INTEGER(GL_BLEND_SRC_RGB, 1);
		CTX_INTEGER(GL_BLEND_DST_RGB, 1);
		CTX_INTEGER(GL_BLEND_SRC_ALPHA, 1);
		CTX_INTEGER(GL_BLEND_DST_ALPHA, 1);

		CTX_INTEGER(GL_SCISSOR_BOX, 4);
		CTX_INTEGER(GL_VIEWPORT, 4);

#undef CTX_INTEGER
#undef CTX_BOOLEAN
#undef CTX_STATE
	}
	_stackLevel++;

	if (_frameBuffers[1]) {
		// Frambuffer blit is impacted by scissor test, disable it
		glDisable(GL_SCISSOR_TEST);
		// We are using multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffers[0]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffers[1]);
		const uint w = _texture.getLogicalWidth();
		const uint h = _texture.getLogicalHeight();
		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else if (_frameBuffers[0]) {
		// Don't mess with the framebuffer if one was setup
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer3D::enter3D() {
	if (_frameBuffers[1]) {
		// We are using multisampling
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffers[1]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffers[0]);
	} else if (_frameBuffers[0]) {
		// Draw on framebuffer if one was setup
		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffers[0]);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	Pipeline::disable();

	if (_stackLevel) {
		if (OpenGLContext.type == kContextGL) {
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glPopClientAttrib();
			glPopAttrib();
		} else {
#define CTX_STATE(gl_param) _save ## gl_param ? glEnable(gl_param) : glDisable(gl_param)

			CTX_STATE(GL_BLEND);
			CTX_STATE(GL_CULL_FACE);
			CTX_STATE(GL_DEPTH_TEST);
			CTX_STATE(GL_DITHER);
			CTX_STATE(GL_POLYGON_OFFSET_FILL);
			CTX_STATE(GL_SCISSOR_TEST);
			CTX_STATE(GL_STENCIL_TEST);

			glDepthMask(_saveGL_DEPTH_WRITEMASK);

			glBlendFuncSeparate(_saveGL_BLEND_SRC_RGB[0], _saveGL_BLEND_DST_RGB[0],
					_saveGL_BLEND_SRC_ALPHA[0], _saveGL_BLEND_DST_ALPHA[0]);

			glScissor(_saveGL_SCISSOR_BOX[0], _saveGL_SCISSOR_BOX[1],
				_saveGL_SCISSOR_BOX[2], _saveGL_SCISSOR_BOX[3]);

			glViewport(_saveGL_VIEWPORT[0], _saveGL_VIEWPORT[1],
				_saveGL_VIEWPORT[2], _saveGL_VIEWPORT[3]);
#undef CTX_STATE
		}
		_stackLevel--;
	} else {
		// 3D engine just starts, make sure the state is clean
		glDisable(GL_BLEND);
		if (OpenGLContext.imagingSupported) {
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ZERO);
		}

		glDisable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_DITHER);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.f, 0.f);

		glDisable(GL_SCISSOR_TEST);
		glScissor(0, 0, g_system->getWidth(), g_system->getHeight());

		glDisable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, -1u);
		glStencilMask(-1u);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glLineWidth(1.f);

		glViewport(0, 0, g_system->getWidth(), g_system->getHeight());

		if (OpenGLContext.type == kContextGL) {
			glDisable(GL_ALPHA_TEST);
			glAlphaFunc(GL_ALWAYS, 0);

			glDisable(GL_COLOR_MATERIAL);
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

			glDisable(GL_FOG);
			glDisable(GL_LIGHTING);
			glDisable(GL_LINE_SMOOTH);
			glEnable(GL_MULTISAMPLE);
			glDisable(GL_NORMALIZE);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glDisable(GL_POLYGON_STIPPLE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_CUBE_MAP);
			glDisable(GL_TEXTURE_GEN_Q);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);

			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_EDGE_FLAG_ARRAY);
			glDisableClientState(GL_FOG_COORD_ARRAY);
			glDisableClientState(GL_INDEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);

			// The others targets are not modified by engines
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_DONT_CARE);
			glLogicOp(GL_COPY);
			glPointSize(1.f);
			glShadeModel(GL_SMOOTH);
		}
	}
}

void Renderer3D::presentBuffer() {
	if (!_frameBuffers[1]) {
		// We are not using multisampling: contents are readily available
		// The engine just has to read from the FBO or the backbuffer
		return;
	}

	assert(_stackLevel == 0);
	bool saveScissorTest = glIsEnabled(GL_SCISSOR_TEST);

	// Frambuffer blit is impacted by scissor test, disable it
	glDisable(GL_SCISSOR_TEST);
	// Swap the framebuffers and blit
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffers[0]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffers[1]);
	const uint w = _texture.getLogicalWidth();
	const uint h = _texture.getLogicalHeight();
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Put back things as they were before
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _frameBuffers[1]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _frameBuffers[0]);

	saveScissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
}

void Renderer3D::showOverlay(uint w, uint h) {
	_inOverlay = true;

	if (_frameBuffers[0]) {
		// We have a framebuffer: the texture already contains an image
		return;
	}

	_texture.create();
	_texture.setSize(w, h);
	Graphics::Surface background;
	background.create(w, h, Texture::getRGBAPixelFormat());
	glReadPixels(0, 0, background.w, background.h, GL_RGBA, GL_UNSIGNED_BYTE, background.getPixels());
	_texture.updateArea(Common::Rect(w, h), background);
	background.free();
}

void Renderer3D::hideOverlay() {
	_inOverlay = false;

	if (!_frameBuffers[0]) {
		// We don't have a framebuffer: destroy the texture we used to store the background
		_texture.destroy();
		return;
	}

	// We have a framebuffer: resize the screen if we have a pending change
	if (_pendingScreenChangeWidth >= 0 && _pendingScreenChangeHeight >= 0) {
		resize(_pendingScreenChangeWidth, _pendingScreenChangeHeight);
		_pendingScreenChangeWidth = -1;
		_pendingScreenChangeHeight = -1;
	}
}

} // End of namespace OpenGL

#endif
