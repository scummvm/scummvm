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

Mesh3DSOpenGLShader::Mesh3DSOpenGLShader(BaseGame *inGame, OpenGL::Shader *shader) : Mesh3DS(inGame), _shader(shader) {
	_vertexCount = 0;
	_vertexData = nullptr;

	glGenBuffers(1, &_vertexBuffer);
	glGenBuffers(1, &_indexBuffer);
}

Mesh3DSOpenGLShader::~Mesh3DSOpenGLShader() {
	glDeleteBuffers(1, &_vertexBuffer);
	glDeleteBuffers(1, &_indexBuffer);
}

void Mesh3DSOpenGLShader::fillVertexBuffer() {
	_vertexCount = _numFaces * 3;
	_vertexData = (Mesh3DSVertex *)_vb.ptr();

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh3DSVertex) * _vertexCount, _vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh3DSOpenGLShader::render() {
	if (_vertexCount == 0)
		return;

	_shader->enableVertexAttribute("position", _vertexBuffer, 3, GL_FLOAT, false, sizeof(Mesh3DSVertex), 0);
	_shader->enableVertexAttribute("color", _vertexBuffer, 4, GL_FLOAT, false, sizeof(Mesh3DSVertex), 24);

	_shader->use(true);

	glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
}

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)
