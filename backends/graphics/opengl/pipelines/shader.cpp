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

#include "backends/graphics/opengl/pipelines/shader.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"

namespace OpenGL {

#if !USE_FORCED_GLES
ShaderPipeline::ShaderPipeline(Shader *shader)
    : _activeShader(shader) {
	_vertexAttribLocation = shader->getAttributeLocation("position");
	_texCoordAttribLocation = shader->getAttributeLocation("texCoordIn");
	_colorAttribLocation = shader->getAttributeLocation("blendColorIn");
}

void ShaderPipeline::activateInternal() {
	GL_CALL(glEnableVertexAttribArray(_vertexAttribLocation));
	GL_CALL(glEnableVertexAttribArray(_texCoordAttribLocation));

	if (g_context.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}

	_activeShader->activate();
}

void ShaderPipeline::deactivateInternal() {
	GL_CALL(glDisableVertexAttribArray(_vertexAttribLocation));
	GL_CALL(glDisableVertexAttribArray(_texCoordAttribLocation));

	_activeShader->deactivate();
}

void ShaderPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GL_CALL(glVertexAttrib4f(_colorAttribLocation, r, g, b, a));
}

void ShaderPipeline::drawTexture(const GLTexture &texture, const GLfloat *coordinates) {
	texture.bind();

	GL_CALL(glVertexAttribPointer(_texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, texture.getTexCoords()));
	GL_CALL(glVertexAttribPointer(_vertexAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, coordinates));
	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void ShaderPipeline::setProjectionMatrix(const GLfloat *projectionMatrix) {
	_activeShader->setUniform("projection", new ShaderUniformMatrix44(projectionMatrix));
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
