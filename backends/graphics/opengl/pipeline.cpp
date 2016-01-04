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

#include "backends/graphics/opengl/pipeline.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"

namespace OpenGL {

Pipeline::Pipeline()
    : _activeFramebuffer(nullptr) {
}

Framebuffer *Pipeline::setFramebuffer(Framebuffer *framebuffer) {
	Framebuffer *oldFramebuffer = _activeFramebuffer;
	if (oldFramebuffer) {
		oldFramebuffer->deactivate();
	}

	_activeFramebuffer = framebuffer;
	if (_activeFramebuffer) {
		_activeFramebuffer->activate();
		setProjectionMatrix(_activeFramebuffer->getProjectionMatrix());
	}

	return oldFramebuffer;
}

#if !USE_FORCED_GLES2
void FixedPipeline::activate() {
	GL_CALL(glDisable(GL_LIGHTING));
	GL_CALL(glDisable(GL_FOG));
	GL_CALL(glShadeModel(GL_FLAT));
	GL_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST));

	GL_CALL(glEnableClientState(GL_VERTEX_ARRAY));
	GL_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

#if !USE_FORCED_GLES
	if (g_context.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}
#endif
	GL_CALL(glEnable(GL_TEXTURE_2D));
}

void FixedPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GL_CALL(glColor4f(r, g, b, a));
}

void FixedPipeline::setDrawCoordinates(const GLfloat *vertices, const GLfloat *texCoords) {
	GL_CALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
	GL_CALL(glVertexPointer(2, GL_FLOAT, 0, vertices));
}

void FixedPipeline::setProjectionMatrix(const GLfloat *projectionMatrix) {
	GL_CALL(glMatrixMode(GL_PROJECTION));
	GL_CALL(glLoadMatrixf(projectionMatrix));

	GL_CALL(glMatrixMode(GL_MODELVIEW));
	GL_CALL(glLoadIdentity());
}
#endif // !USE_FORCED_GLES2

#if !USE_FORCED_GLES
ShaderPipeline::ShaderPipeline()
    : _activeShader(nullptr) {
}

void ShaderPipeline::activate() {
	GL_CALL(glEnableVertexAttribArray(kPositionAttribLocation));
	GL_CALL(glEnableVertexAttribArray(kTexCoordAttribLocation));

	if (g_context.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}
}

Shader *ShaderPipeline::setShader(Shader *shader) {
	Shader *oldShader = _activeShader;

	_activeShader = shader;
	if (_activeShader && _activeFramebuffer) {
		_activeShader->activate(_activeFramebuffer->getProjectionMatrix());
	}

	return oldShader;
}

void ShaderPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GL_CALL(glVertexAttrib4f(kColorAttribLocation, r, g, b, a));
}

void ShaderPipeline::setDrawCoordinates(const GLfloat *vertices, const GLfloat *texCoords) {
	GL_CALL(glVertexAttribPointer(kTexCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texCoords));
	GL_CALL(glVertexAttribPointer(kPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, vertices));
}

void ShaderPipeline::setProjectionMatrix(const GLfloat *projectionMatrix) {
	if (_activeShader) {
		_activeShader->activate(projectionMatrix);
	}
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
