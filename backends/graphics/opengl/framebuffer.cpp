/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "backends/graphics/opengl/framebuffer.h"
#include "backends/graphics/opengl/texture.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"

namespace OpenGL {

Framebuffer::Framebuffer()
    : _viewport(), _projectionMatrix(), _isActive(false), _clearColor(),
      _blendState(kBlendModeDisabled), _scissorTestState(false), _scissorBox() {
}

void Framebuffer::activate() {
	_isActive = true;

	applyViewport();
	applyProjectionMatrix();
	applyClearColor();
	applyBlendState();
	applyScissorTestState();
	applyScissorBox();

	activateInternal();
}

void Framebuffer::deactivate() {
	_isActive = false;

	deactivateInternal();
}

void Framebuffer::setClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	_clearColor[0] = r;
	_clearColor[1] = g;
	_clearColor[2] = b;
	_clearColor[3] = a;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyClearColor();
	}
}

void Framebuffer::enableBlend(BlendMode mode) {
	_blendState = mode;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyBlendState();
	}
}

void Framebuffer::enableScissorTest(bool enable) {
	_scissorTestState = enable;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyScissorTestState();
	}
}

void Framebuffer::setScissorBox(GLint x, GLint y, GLsizei w, GLsizei h) {
	_scissorBox[0] = x;
	_scissorBox[1] = y;
	_scissorBox[2] = w;
	_scissorBox[3] = h;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyScissorBox();
	}
}

void Framebuffer::applyViewport() {
	GL_CALL(glViewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]));
}

void Framebuffer::applyProjectionMatrix() {
	g_context.getActivePipeline()->setProjectionMatrix(_projectionMatrix);
}

void Framebuffer::applyClearColor() {
	GL_CALL(glClearColor(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]));
}

void Framebuffer::applyBlendState() {
	switch (_blendState) {
		case kBlendModeDisabled:
			GL_CALL(glDisable(GL_BLEND));
			break;
		case kBlendModeTraditionalTransparency:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			break;
		case kBlendModePremultipliedTransparency:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
			break;
		default:
			break;
	}
}

void Framebuffer::applyScissorTestState() {
	if (_scissorTestState) {
		GL_CALL(glEnable(GL_SCISSOR_TEST));
	} else {
		GL_CALL(glDisable(GL_SCISSOR_TEST));
	}
}

void Framebuffer::applyScissorBox() {
	GL_CALL(glScissor(_scissorBox[0], _scissorBox[1], _scissorBox[2], _scissorBox[3]));
}

//
// Backbuffer implementation
//

void Backbuffer::activateInternal() {
#if !USE_FORCED_GLES
	if (g_context.framebufferObjectSupported) {
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
#endif
}

void Backbuffer::setDimensions(uint width, uint height) {
	// Set viewport dimensions.
	_viewport[0] = 0;
	_viewport[1] = 0;
	_viewport[2] = width;
	_viewport[3] = height;

	// Setup orthogonal projection matrix.
	_projectionMatrix[ 0] =  2.0f / width;
	_projectionMatrix[ 1] =  0.0f;
	_projectionMatrix[ 2] =  0.0f;
	_projectionMatrix[ 3] =  0.0f;

	_projectionMatrix[ 4] =  0.0f;
	_projectionMatrix[ 5] = -2.0f / height;
	_projectionMatrix[ 6] =  0.0f;
	_projectionMatrix[ 7] =  0.0f;

	_projectionMatrix[ 8] =  0.0f;
	_projectionMatrix[ 9] =  0.0f;
	_projectionMatrix[10] =  0.0f;
	_projectionMatrix[11] =  0.0f;

	_projectionMatrix[12] = -1.0f;
	_projectionMatrix[13] =  1.0f;
	_projectionMatrix[14] =  0.0f;
	_projectionMatrix[15] =  1.0f;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
}

//
// Render to texture target implementation
//

#if !USE_FORCED_GLES
TextureTarget::TextureTarget()
    : _texture(new GLTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE)), _glFBO(0), _needUpdate(true) {
}

TextureTarget::~TextureTarget() {
	delete _texture;
	GL_CALL_SAFE(glDeleteFramebuffers, (1, &_glFBO));
}

void TextureTarget::activateInternal() {
	// Allocate framebuffer object if necessary.
	if (!_glFBO) {
		GL_CALL(glGenFramebuffers(1, &_glFBO));
		_needUpdate = true;
	}

	// Attach destination texture to FBO.
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _glFBO));

	// If required attach texture to FBO.
	if (_needUpdate) {
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->getGLTexture(), 0));
		_needUpdate = false;
	}
}

void TextureTarget::destroy() {
	GL_CALL(glDeleteFramebuffers(1, &_glFBO));
	_glFBO = 0;

	_texture->destroy();
}

void TextureTarget::create() {
	_texture->create();

	_needUpdate = true;
}

void TextureTarget::setSize(uint width, uint height) {
	_texture->setSize(width, height);

	const uint texWidth  = _texture->getWidth();
	const uint texHeight = _texture->getHeight();

	// Set viewport dimensions.
	_viewport[0] = 0;
	_viewport[1] = 0;
	_viewport[2] = texWidth;
	_viewport[3] = texHeight;

	// Setup orthogonal projection matrix.
	_projectionMatrix[ 0] =  2.0f / texWidth;
	_projectionMatrix[ 1] =  0.0f;
	_projectionMatrix[ 2] =  0.0f;
	_projectionMatrix[ 3] =  0.0f;

	_projectionMatrix[ 4] =  0.0f;
	_projectionMatrix[ 5] =  2.0f / texHeight;
	_projectionMatrix[ 6] =  0.0f;
	_projectionMatrix[ 7] =  0.0f;

	_projectionMatrix[ 8] =  0.0f;
	_projectionMatrix[ 9] =  0.0f;
	_projectionMatrix[10] =  0.0f;
	_projectionMatrix[11] =  0.0f;

	_projectionMatrix[12] = -1.0f;
	_projectionMatrix[13] = -1.0f;
	_projectionMatrix[14] =  0.0f;
	_projectionMatrix[15] =  1.0f;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
