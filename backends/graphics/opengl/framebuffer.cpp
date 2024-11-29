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

#include "backends/graphics/opengl/framebuffer.h"
#include "backends/graphics/opengl/pipelines/pipeline.h"
#include "graphics/opengl/debug.h"
#include "graphics/opengl/texture.h"
#include "common/rotationmode.h"

namespace OpenGL {

Framebuffer::Framebuffer()
	: _viewport(), _projectionMatrix(), _pipeline(nullptr), _clearColor(),
	  _blendState(kBlendModeDisabled), _scissorTestState(false), _scissorBox() {
}

void Framebuffer::activate(Pipeline *pipeline) {
	assert(pipeline);
	_pipeline = pipeline;

	applyViewport();
	applyProjectionMatrix();
	applyClearColor();
	applyBlendState();
	applyScissorTestState();
	applyScissorBox();

	activateInternal();
}

void Framebuffer::deactivate() {
	deactivateInternal();

	_pipeline = nullptr;
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
	assert(_pipeline);
	_pipeline->setProjectionMatrix(_projectionMatrix);
}

void Framebuffer::applyClearColor() {
	GL_CALL(glClearColor(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]));
}

void Framebuffer::applyBlendState() {
	switch (_blendState) {
		case kBlendModeDisabled:
			GL_CALL(glDisable(GL_BLEND));
			break;
		case kBlendModeOpaque:
			if (!glBlendColor) {
				// If glBlendColor is not available (old OpenGL) fallback on disabling blending
				GL_CALL(glDisable(GL_BLEND));
				break;
			}
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendColor(1.f, 1.f, 1.f, 0.f));
			GL_CALL(glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR));
			break;
		case kBlendModeTraditionalTransparency:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			break;
		case kBlendModePremultipliedTransparency:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
			break;
		case kBlendModeAdditive:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_ONE, GL_ONE));
			break;
		case kBlendModeMaskAlphaAndInvertByColor:
			GL_CALL(glEnable(GL_BLEND));
			GL_CALL(glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA));
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

void Framebuffer::copyRenderStateFrom(const Framebuffer &other, uint copyMask) {
	if (copyMask & kCopyMaskClearColor) {
		memcpy(_clearColor, other._clearColor, sizeof(_clearColor));
	}
	if (copyMask & kCopyMaskBlendState) {
		_blendState = other._blendState;
	}
	if (copyMask & kCopyMaskScissorState) {
		_scissorTestState = other._scissorTestState;
	}
	if (copyMask & kCopyMaskScissorBox) {
		memcpy(_scissorBox, other._scissorBox, sizeof(_scissorBox));
	}

	if (isActive()) {
		applyClearColor();
		applyBlendState();
		applyScissorTestState();
		applyScissorBox();
	}
}

//
// Backbuffer implementation
//

void Backbuffer::activateInternal() {
#if !USE_FORCED_GLES
	if (OpenGLContext.framebufferObjectSupported) {
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
#endif
}

bool Backbuffer::setSize(uint width, uint height, Common::RotationMode rotation) {
	// Set viewport dimensions.
	_viewport[0] = 0;
	_viewport[1] = 0;
	_viewport[2] = width;
	_viewport[3] = height;

	// Setup orthogonal projection matrix.
	_projectionMatrix(0, 0) =  2.0f / width;
	_projectionMatrix(0, 1) =  0.0f;
	_projectionMatrix(0, 2) =  0.0f;
	_projectionMatrix(0, 3) =  0.0f;

	_projectionMatrix(1, 0) =  0.0f;
	_projectionMatrix(1, 1) = -2.0f / height;
	_projectionMatrix(1, 2) =  0.0f;
	_projectionMatrix(1, 3) =  0.0f;

	_projectionMatrix(2, 0) =  0.0f;
	_projectionMatrix(2, 1) =  0.0f;
	_projectionMatrix(2, 2) =  0.0f;
	_projectionMatrix(2, 3) =  0.0f;

	_projectionMatrix(3, 0) = -1.0f;
	_projectionMatrix(3, 1) =  1.0f;
	_projectionMatrix(3, 2) =  0.0f;
	_projectionMatrix(3, 3) =  1.0f;

	switch (rotation) {
	default:
		_projectionMatrix(0, 0) =  2.0f / width;
		_projectionMatrix(0, 1) =  0.0f;
		_projectionMatrix(1, 0) =  0.0f;
		_projectionMatrix(1, 1) = -2.0f / height;
		_projectionMatrix(3, 0) = -1.0f;
		_projectionMatrix(3, 1) =  1.0f;
		break;
	case Common::kRotation90:
		_projectionMatrix(0, 0) =  0.0f;
		_projectionMatrix(0, 1) =  -2.0f / height;
		_projectionMatrix(1, 0) =  -2.0f / width;
		_projectionMatrix(1, 1) =  0.0f;
		_projectionMatrix(3, 0) =  1.0f;
		_projectionMatrix(3, 1) =  1.0f;
		break;
	case Common::kRotation180:
		_projectionMatrix(0, 0) =  -2.0f / width;
		_projectionMatrix(0, 1) =  0.0f;
		_projectionMatrix(1, 0) =  0.0f;
		_projectionMatrix(1, 1) =  2.0f / height;
		_projectionMatrix(3, 0) =  1.0f;
		_projectionMatrix(3, 1) = -1.0f;
		break;
	case Common::kRotation270:
		_projectionMatrix(0, 0) =  0.0f;
		_projectionMatrix(0, 1) =  2.0f / height;
		_projectionMatrix(1, 0) =  2.0f / width;
		_projectionMatrix(1, 1) =  0.0f;
		_projectionMatrix(3, 0) = -1.0f;
		_projectionMatrix(3, 1) = -1.0f;
		break;
	}

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
	return true;
}

//
// Render to texture target implementation
//

#if !USE_FORCED_GLES
TextureTarget::TextureTarget()
	: _texture(new Texture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE)), _glFBO(0), _needUpdate(true) {
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

bool TextureTarget::setSize(uint width, uint height, Common::RotationMode rotation) {
	if (!_texture->setSize(width, height)) {
		return false;
	}

	const uint texWidth  = _texture->getWidth();
	const uint texHeight = _texture->getHeight();

	// Set viewport dimensions.
	_viewport[0] = 0;
	_viewport[1] = 0;
	_viewport[2] = texWidth;
	_viewport[3] = texHeight;

	// Setup orthogonal projection matrix.
	_projectionMatrix(0, 0) =  2.0f / texWidth;
	_projectionMatrix(0, 1) =  0.0f;
	_projectionMatrix(0, 2) =  0.0f;
	_projectionMatrix(0, 3) =  0.0f;

	_projectionMatrix(1, 0) =  0.0f;
	_projectionMatrix(1, 1) =  2.0f / texHeight;
	_projectionMatrix(1, 2) =  0.0f;
	_projectionMatrix(1, 3) =  0.0f;

	_projectionMatrix(2, 0) =  0.0f;
	_projectionMatrix(2, 1) =  0.0f;
	_projectionMatrix(2, 2) =  0.0f;
	_projectionMatrix(2, 3) =  0.0f;

	_projectionMatrix(3, 0) = -1.0f;
	_projectionMatrix(3, 1) = -1.0f;
	_projectionMatrix(3, 2) =  0.0f;
	_projectionMatrix(3, 3) =  1.0f;

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
	return true;
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
