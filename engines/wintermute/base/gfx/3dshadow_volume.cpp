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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/xskinmesh.h"
#include "engines/wintermute/dcgf.h"
#include "graphics/opengl/system_headers.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ShadowVolume::ShadowVolume(BaseGame *inGame) : BaseClass(inGame), _color(0x7f000000) {
}

//////////////////////////////////////////////////////////////////////////
ShadowVolume::~ShadowVolume() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::reset() {
	_vertices.clear();
	return true;
}

//////////////////////////////////////////////////////////////////////////^M
bool ShadowVolume::addMesh(DXMesh *mesh, uint32 *adjacency, DXMatrix *modelMat, DXVector3 *light, float extrusionDepth) {
	if (!mesh || !adjacency)
		return false;

	DXVector3 invLight;
	DXMatrix matInverseModel;
	DXMatrixInverse(&matInverseModel, nullptr, modelMat);
	DXVec3TransformNormal(&invLight, light, &matInverseModel);

	float *points = (float *)mesh->getVertexBuffer().ptr();
	if (points == nullptr) {
		return false;
	}

	uint32 *indices = (uint32 *)mesh->getIndexBuffer().ptr();
	if (indices == nullptr) {
		return false;
	}

	uint32 numFaces = mesh->getNumFaces();

	// Allocate a temporary edge list
	uint32 *edges = new uint32[numFaces * 6];
	if (edges == nullptr) {
		return false;
	}

	uint32 numEdges = 0;
	uint32 fvfSize = DXGetFVFVertexSize(mesh->getFVF()) / sizeof(float);

	bool *isFront = new bool[numFaces];

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < numFaces; i++) {
		uint32 index0 = indices[3 * i + 0];
		uint32 index1 = indices[3 * i + 1];
		uint32 index2 = indices[3 * i + 2];

		DXVector3 v0(points + index0 * fvfSize);
		DXVector3 v1(points + index1 * fvfSize);
		DXVector3 v2(points + index2 * fvfSize);

		// Transform vertices or transform light?
		DXVector3 vNormal, vec1, vec2;
		vec1 = v2 - v1;
		vec2 = v1 - v0;
		DXVec3Cross(&vNormal, &vec1, &vec2);

		if (DXVec3Dot(&vNormal, &invLight) >= 0.0f) {
			isFront[i] = false; // back face
		} else {
			isFront[i] = true; // front face
		}
	}

	// First pass : for each face, record if it is front or back facing the light
	for (uint32 i = 0; i < numFaces; i++) {
		if (isFront[i]) {
			uint32 wFace0 = indices[3 * i + 0];
			uint32 wFace1 = indices[3 * i + 1];
			uint32 wFace2 = indices[3 * i + 2];

			uint32 adjacent0 = adjacency[3 * i + 0];
			uint32 adjacent1 = adjacency[3 * i + 1];
			uint32 adjacent2 = adjacency[3 * i + 2];

			if (adjacent0 == 0xFFFFFFFF || isFront[adjacent0] == false) {
				// add edge v0-v1
				edges[2 * numEdges + 0] = wFace0;
				edges[2 * numEdges + 1] = wFace1;
				numEdges++;
			}
			if (adjacent1 == 0xFFFFFFFF || isFront[adjacent1] == false) {
				// add edge v1-v2
				edges[2 * numEdges + 0] = wFace1;
				edges[2 * numEdges + 1] = wFace2;
				numEdges++;
			}
			if (adjacent2 == 0xFFFFFFFF || isFront[adjacent2] == false) {
				// add edge v2-v0
				edges[2 * numEdges + 0] = wFace2;
				edges[2 * numEdges + 1] = wFace0;
				numEdges++;
			}
		}
	}

	for (uint32 i = 0; i < numEdges; i++) {
		DXVector3 v1(points + edges[2 * i + 0] * fvfSize);
		DXVector3 v2(points + edges[2 * i + 1] * fvfSize);
		DXVector3 v3 = v1 - invLight * extrusionDepth;
		DXVector3 v4 = v2 - invLight * extrusionDepth;

		// Add a quad (two triangles) to the vertex list
		addVertex(v1);
		addVertex(v2);
		addVertex(v3);

		addVertex(v2);
		addVertex(v4);
		addVertex(v3);
	}

	// Delete the temporary edge list
	delete[] edges;
	delete[] isFront;

	return true;
}
//////////////////////////////////////////////////////////////////////////
void ShadowVolume::addVertex(DXVector3 &vertex) {
	_vertices.add(vertex);
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::setColor(uint32 color) {
	if (color != _color) {
		_color = color;
		return initMask();
	} else {
		return true;
	}
}

} // namespace Wintermute
