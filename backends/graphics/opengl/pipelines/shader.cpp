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
    : _activeShader(shader), _colorAttributes() {
	_vertexAttribLocation = shader->getAttributeLocation("position");
	_texCoordAttribLocation = shader->getAttributeLocation("texCoordIn");
	_colorAttribLocation = shader->getAttributeLocation("blendColorIn");

	assert(_vertexAttribLocation   != -1);
	assert(_texCoordAttribLocation != -1);
	assert(_colorAttribLocation    != -1);

	// One of the attributes needs to be passed through location 0, otherwise
	// we get no output for GL contexts due to GL compatibility reasons. Let's
	// check whether this ever happens. If this ever gets hit, we need to
	// enable location 0 and pass some dummy values through it to fix output.
	assert(   _vertexAttribLocation == 0
	       || _texCoordAttribLocation == 0
	       || _colorAttribLocation == 0);
}

void ShaderPipeline::activateInternal() {
	GL_CALL(glEnableVertexAttribArray(_vertexAttribLocation));
	GL_CALL(glEnableVertexAttribArray(_texCoordAttribLocation));
	GL_CALL(glEnableVertexAttribArray(_colorAttribLocation));

	if (g_context.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}

	_activeShader->activate();

	GL_CALL(glVertexAttribPointer(_colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 0, _colorAttributes));
}

void ShaderPipeline::deactivateInternal() {
	GL_CALL(glDisableVertexAttribArray(_vertexAttribLocation));
	GL_CALL(glDisableVertexAttribArray(_texCoordAttribLocation));
	GL_CALL(glDisableVertexAttribArray(_colorAttribLocation));

	_activeShader->deactivate();
}

void ShaderPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GLfloat *dst = _colorAttributes;
	for (uint i = 0; i < 4; ++i) {
		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	}
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
