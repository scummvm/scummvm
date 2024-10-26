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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/file.h"

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/3dloader_3ds.h"
#include "engines/wintermute/base/gfx/3dmesh.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
Mesh3DS::Mesh3DS(BaseGame *inGame) : BaseNamedObject(inGame) {
	_vertices = nullptr;
	_faces = nullptr;
	_numFaces = _numVertices = 0;
	_visible = true;
}

//////////////////////////////////////////////////////////////////////////
Mesh3DS::~Mesh3DS() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void Mesh3DS::cleanup() {
	delete[] _vertices;
	_vertices = nullptr;
	_numVertices = 0;

	delete[] _faces;
	_faces = nullptr;
	_numFaces = 0;

	_vb.free();
}


//////////////////////////////////////////////////////////////////////////
bool Mesh3DS::createVertexBuffer() {
	_vb.free();

	if (_numFaces == 0)
		return true;

	int vbSize = _numFaces * sizeof(Mesh3DSVertex) * 3;
	_vb = DXBuffer(vbSize);
	if (_vb.ptr() == nullptr) {
		_gameRef->LOG(0, "Error creating vertex buffer.");
		return false;
	} else
		return true;
}


//////////////////////////////////////////////////////////////////////////
bool Mesh3DS::fillVertexBuffer(uint32 color) {
	_vb.free();

	if (_numFaces == 0)
		return true;

	int vbSize = _numFaces * sizeof(Mesh3DSVertex) * 3;
	_vb = DXBuffer(vbSize);
	if (_vb.ptr() == nullptr) {
		_gameRef->LOG(0, "Error creating vertex buffer.");
		return false;
	}

	Mesh3DSVertex *verts = (Mesh3DSVertex *)_vb.ptr();

	for (int i = 0; i < _numFaces; i++) {
		for (int j = 0; j < 3; j++) {
			int outVert = i * 3 + j;
			int vertex = _faces[i]._vertices[j];

			verts[outVert]._x  = _vertices[vertex]._pos._x;
			verts[outVert]._y  = _vertices[vertex]._pos._y;
			verts[outVert]._z  = _vertices[vertex]._pos._z;

			verts[outVert]._nx = _faces[i]._normals[j]._x;
			verts[outVert]._ny = _faces[i]._normals[j]._y;
			verts[outVert]._nz = _faces[i]._normals[j]._z;

			verts[outVert]._r = RGBCOLGetR(color) / 255.0f;
			verts[outVert]._g = RGBCOLGetG(color) / 255.0f;
			verts[outVert]._b = RGBCOLGetB(color) / 255.0f;
			verts[outVert]._a = RGBCOLGetA(color) / 255.0f;
		}
	}

	fillVertexBuffer();

	return true;
}


//////////////////////////////////////////////////////////////////////////
void Mesh3DS::computeNormals() {
	DXVector3 *normals = new DXVector3[_numVertices];
	for (int i = 0; i < _numVertices; ++i) {
		normals[i]._x = 0.0f;
		normals[i]._y = 0.0f;
		normals[i]._z = 0.0f;
	}

	for (int i = 0; i < _numFaces; ++i) {
		uint16 a = _faces[i]._vertices[0];
		uint16 b = _faces[i]._vertices[1];
		uint16 c = _faces[i]._vertices[2];

		DXVector3 *v1 = &_vertices[a]._pos;
		DXVector3 *v2 = &_vertices[b]._pos;
		DXVector3 *v3 = &_vertices[c]._pos;

		DXVector3 edge1 = *v2 - *v1;
		DXVector3 edge2 = *v3 - *v2;
		DXVector3 normal;
		DXVec3Cross(&normal, &edge1, &edge2);
		DXVec3Normalize(&normal, &normal);

		normals[a] += normal;
		normals[b] += normal;
		normals[c] += normal;
	}

	// Assign the newly computed normals back to the vertices
	for (int i = 0; i < _numFaces; ++i) {
		for (int j = 0; j < 3; j++) {
			DXVec3Normalize(&_faces[i]._normals[j], &normals[_faces[i]._vertices[j]]);
			//_faces[i]._normals[j] = normals[_faces[i]._vertices[j]];
		}
	}

	delete[] normals;
}

//////////////////////////////////////////////////////////////////////////
bool Mesh3DS::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferBool(TMEMBER(_visible));
	return true;
}

} // namespace Wintermute
