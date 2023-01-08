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

#ifndef TETRAEDGE_TE_TE_MESH_TINYGL_H
#define TETRAEDGE_TE_TE_MESH_TINYGL_H

#if defined(USE_TINYGL)

#include "tetraedge/te/te_mesh.h"

namespace Tetraedge {

class TeMeshTinyGL : public TeMesh {
public:
	TeMeshTinyGL();

	void copy(const TeMesh &other);
	void draw() override;
	TeMesh::Mode getMode() const override;
	void setMode(enum Mode mode) override;

	void setConf(unsigned long vertexCount, unsigned long indexCount, enum Mode mode, uint materialCount, uint materialIndexCount);

	void setglTexEnvBlend() override;
	uint32 getTexEnvMode() const override;

private:
	uint _glMeshMode;
	uint32 _gltexEnvMode;
};

} // end namespace Tetraedge

#endif // USE_TINYGL

#endif // TETRAEDGE_TE_TE_MESH_H
