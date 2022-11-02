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

#ifndef WINTERMUTE_MESH_OPENGL_H
#define WINTERMUTE_MESH_OPENGL_H

#include "engines/wintermute/base/gfx/3dmesh.h"

#if defined(USE_OPENGL_SHADERS)

#include "graphics/opengl/shader.h"

namespace Wintermute {

class Mesh3DSOpenGLShader : public Mesh3DS {
public:
	Mesh3DSOpenGLShader(OpenGL::Shader *shader);
	~Mesh3DSOpenGLShader();
	void fillVertexBuffer(uint32 color) override;
	void render() override;

private:
	GLuint _vertexBuffer;
	GLuint _indexBuffer;
	OpenGL::Shader *_shader;
};

} // namespace Wintermute

#endif // defined(USE_OPENGL_SHADERS)

#endif
