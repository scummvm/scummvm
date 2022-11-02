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

#include "engines/wintermute/wintypes.h"

#include "graphics/opengl/system_headers.h"

#if defined(USE_OPENGL_SHADERS)

#include "engines/wintermute/base/gfx/opengl/mesh3ds_opengl_shader.h"

namespace Wintermute {

Mesh3DSOpenGLShader::Mesh3DSOpenGLShader(OpenGL::Shader *shader) : _shader(shader) {
	glGenBuffers(1, &_vertexBuffer);
	glGenBuffers(1, &_indexBuffer);
}

Mesh3DSOpenGLShader::~Mesh3DSOpenGLShader() {
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_indexBuffer);
}

void Mesh3DSOpenGLShader::fillVertexBuffer(uint32 color) {
	_color.x() = RGBCOLGetR(color) / 255.0f;
	_color.y() = RGBCOLGetG(color) / 255.0f;
	_color.z() = RGBCOLGetB(color) / 255.0f;
	_color.w() = RGBCOLGetA(color) / 255.0f;

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GeometryVertex) * _vertexCount, _vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * _indexCount, _indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh3DSOpenGLShader::render() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

	_shader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, sizeof(GeometryVertex), 0);

	_shader->use(true);
	_shader->setUniform("color", _color);

	glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
