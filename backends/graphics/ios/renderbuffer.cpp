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

#include "backends/graphics/ios/renderbuffer.h"
#include "graphics/opengl/debug.h"

namespace OpenGL {

//
// Render to backbuffer target implementation
//
RenderbufferTarget::RenderbufferTarget(GLuint renderbufferID)
	: _glRBO(renderbufferID), _glFBO(0) {
}

RenderbufferTarget::~RenderbufferTarget() {
	GL_CALL_SAFE(glDeleteFramebuffers, (1, &_glFBO));
}

void RenderbufferTarget::activateInternal() {
	bool needUpdate = false;

	// Allocate framebuffer object if necessary.
	if (!_glFBO) {
		GL_CALL(glGenFramebuffers(1, &_glFBO));
		needUpdate = true;
	}

	// Attach FBO to rendering context.
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, _glFBO));

	// Attach render buffer to newly created FBO.
	if (needUpdate) {
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _glRBO));
	}
}

bool RenderbufferTarget::setSize(uint width, uint height, Common::RotationMode rotation) {
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

	// Directly apply changes when we are active.
	if (isActive()) {
		applyViewport();
		applyProjectionMatrix();
	}
	return true;
}

} // End of namespace OpenGL
