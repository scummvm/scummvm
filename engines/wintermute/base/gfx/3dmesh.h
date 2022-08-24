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

#ifndef WINTERMUTE_3D_MESH_H
#define WINTERMUTE_3D_MESH_H

#include "common/memstream.h"

#include "math/vector4d.h"

namespace Wintermute {

struct GeometryVertex {
	float x;
	float y;
	float z;
};

class Mesh3DS {
public:
	Mesh3DS();
	virtual ~Mesh3DS();
	virtual void fillVertexBuffer(uint32 color) = 0;
	virtual bool loadFrom3DS(Common::MemoryReadStream &fileStream);
	virtual void render() = 0;
	virtual void dumpVertexCoordinates(const char *filename);
	virtual int faceCount();
	virtual uint16 *getFace(int index);

	virtual int vertexCount();
	virtual float *getVertexPosition(int index);

protected:
	GeometryVertex *_vertexData;
	uint16 _vertexCount;
	uint16 *_indexData;
	uint16 _indexCount;
	Math::Vector4d _color;
};

} // namespace Wintermute

#endif
