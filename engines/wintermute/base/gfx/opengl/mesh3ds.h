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

#ifndef WINTERMUTE_MESH_3DS_H
#define WINTERMUTE_MESH_3DS_H

#include "common/memstream.h"
#include "common/scummsys.h"
#include "graphics/opengl/system_headers.h"
#include "math/vector3d.h"

namespace Wintermute {

class Mesh3DS {
public:
	virtual ~Mesh3DS();
	virtual void computeNormals() = 0;
	virtual void fillVertexBuffer(uint32 color) = 0;
	virtual bool loadFrom3DS(Common::MemoryReadStream &fileStream) = 0;
	virtual void render() = 0;
	virtual void dumpVertexCoordinates(const char *filename) = 0;
	virtual int faceCount() = 0;
	virtual uint16 *getFace(int index) = 0;

	virtual int vertexCount() = 0;
	virtual float *getVertexPosition(int index) = 0;
};

} // namespace Wintermute

#endif
