/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WINTERMUTE_MESH_OPENGL_H
#define WINTERMUTE_MESH_OPENGL_H

#include "common/memstream.h"
#include "common/scummsys.h"
#include "engines/wintermute/base/gfx/opengl/mesh3ds.h"
#include "graphics/opengl/shader.h"
#include "graphics/opengl/system_headers.h"
#include "math/vector4d.h"

namespace Wintermute {

#include "common/pack-start.h"

struct GeometryVertexShader {
	float x;
	float y;
	float z;
} PACKED_STRUCT;

#include "common/pack-end.h"

class Mesh3DSOpenGLShader : public Mesh3DS {
public:
	// vertex size in bytes, for the moment we only have
	// position and color components
	static const int kVertexSize = 28;
	Mesh3DSOpenGLShader(OpenGL::Shader *shader);
	~Mesh3DSOpenGLShader();
	void computeNormals();
	void fillVertexBuffer(uint32 color);
	bool loadFrom3DS(Common::MemoryReadStream &fileStream);
	void render();
	void dumpVertexCoordinates(const char *filename);
	int faceCount();
	uint16 *getFace(int index);

	int vertexCount();
	float *getVertexPosition(int index);

private:
	GeometryVertexShader *_vertexData;
	uint16 _vertexCount;
	uint16 *_indexData;
	uint16 _indexCount;
	GLuint _vertexBuffer;
	GLuint _indexBuffer;
	OpenGL::Shader *_shader;
	Math::Vector4d _color;
};

} // namespace Wintermute

#endif
