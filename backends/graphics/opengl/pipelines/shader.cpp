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

#include "backends/graphics/opengl/pipelines/shader.h"
#include "backends/graphics/opengl/shader.h"
#include "backends/graphics/opengl/framebuffer.h"
#include "graphics/opengl/debug.h"

namespace OpenGL {

// A 4 elements with 2 components vector of floats
static const int kCoordinatesSize = 4 * 2 * sizeof(float);

#if !USE_FORCED_GLES
ShaderPipeline::ShaderPipeline(Shader *shader)
	: _activeShader(shader), _colorAttributes(), _colorDirty(true) {
	// Use the same VBO for vertices and texcoords as we modify them at the same time
	_coordsVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, kCoordinatesSize, nullptr, GL_STATIC_DRAW);
	_activeShader->enableVertexAttribute("position", _coordsVBO, 2, GL_FLOAT, GL_FALSE, 0, 0);
	_texcoordsVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, kCoordinatesSize, nullptr, GL_STATIC_DRAW);
	_activeShader->enableVertexAttribute("texCoordIn", _texcoordsVBO, 2, GL_FLOAT, GL_FALSE, 0, 0);
	_colorVBO = OpenGL::Shader::createBuffer(GL_ARRAY_BUFFER, sizeof(_colorAttributes), nullptr, GL_DYNAMIC_DRAW);
	_activeShader->enableVertexAttribute("blendColorIn", _colorVBO, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

ShaderPipeline::~ShaderPipeline() {
	delete _activeShader;

	OpenGL::Shader::freeBuffer(_coordsVBO);
	OpenGL::Shader::freeBuffer(_texcoordsVBO);
	OpenGL::Shader::freeBuffer(_colorVBO);
}

void ShaderPipeline::activateInternal() {
	Pipeline::activateInternal();

	if (OpenGLContext.multitextureSupported) {
		GL_CALL(glActiveTexture(GL_TEXTURE0));
	}

	_activeShader->use();
}

void ShaderPipeline::deactivateInternal() {
	_activeShader->unbind();

	Pipeline::deactivateInternal();
}

void ShaderPipeline::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GLfloat *dst = _colorAttributes;
	for (uint i = 0; i < 4; ++i) {
		*dst++ = r;
		*dst++ = g;
		*dst++ = b;
		*dst++ = a;
	}
	_colorDirty = true;
}

void ShaderPipeline::drawTextureInternal(const Texture &texture, const GLfloat *coordinates, const GLfloat *texcoords) {
	assert(isActive());

	texture.bind();

	if (_colorDirty) {
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _colorVBO));
		GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_colorAttributes), _colorAttributes));
		_colorDirty = false;
	}
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _coordsVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, kCoordinatesSize, coordinates, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _texcoordsVBO));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, kCoordinatesSize, texcoords, GL_STATIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void ShaderPipeline::setProjectionMatrix(const Math::Matrix4 &projectionMatrix) {
	assert(isActive());

	_activeShader->setUniform("projection", projectionMatrix);
}
#endif // !USE_FORCED_GLES

} // End of namespace OpenGL
