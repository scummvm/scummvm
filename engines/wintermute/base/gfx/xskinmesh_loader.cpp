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

#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xfile_loader.h"
#include "engines/wintermute/base/gfx/xskinmesh_loader.h"
#include "engines/wintermute/base/gfx/xskinmesh.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/math/math_util.h"

namespace Wintermute {

// define constant to make it available to the linker
const uint32 XSkinMeshLoader::kNullIndex;

XSkinMeshLoader::XSkinMeshLoader(XMesh *mesh, XMeshObject *meshObject, DXMesh *dxmesh, DXSkinInfo *skinInfo) {
	_mesh = mesh;
	_meshObject = meshObject;
	_dxmesh = dxmesh;
	_skinInfo = skinInfo;

	_vertexCount = meshObject->_numVertices;
	// vertex format for .X meshes will be position + normals + textures
	_vertexData = new float[kVertexComponentCount * _vertexCount]();
	_vertexPositionData = new float[3 * _vertexCount]();
	// we already know how big this is supposed to be
	// TODO: might have to generate normals if file does not contain any
	_vertexNormalData = new float[3 * _vertexCount]();
}

XSkinMeshLoader::~XSkinMeshLoader() {
	delete[] _vertexData;
	delete[] _vertexPositionData;
	delete[] _vertexNormalData;
}

void XSkinMeshLoader::loadMesh(const Common::String &filename, XFileData *xobj) {
	auto fvf = _dxmesh->getFVF();
	uint32 vertexSize = DXGetFVFVertexSize(fvf) / sizeof(float);
	float *vertexBuffer = (float *)_dxmesh->getVertexBuffer().ptr();
	uint32 offset = 0, normalOffset = 0, /*diffuseOffset = 0, */textureOffset = 0;

	if (fvf & DXFVF_XYZ) {
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_NORMAL) {
		normalOffset = offset;
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_DIFFUSE) {
		//diffuseOffset = offset;
		offset += sizeof(DXColorValue) / sizeof(float);
	}
	if (fvf & DXFVF_TEX1) {
		textureOffset = offset;
		offset += sizeof(DXVector2) / sizeof(float);
	}

	for (uint i = 0; i < _vertexCount; ++i) {
		_vertexPositionData[i * 3 + 0] = vertexBuffer[i * vertexSize + 0];
		_vertexPositionData[i * 3 + 1] = vertexBuffer[i * vertexSize + 1];
		_vertexPositionData[i * 3 + 2] = vertexBuffer[i * vertexSize + 2];
		for (int j = 0; j < 3; ++j) {
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = _vertexPositionData[i * 3 + j];
		}
		// mirror z coordinate to change to OpenGL coordinate system
		_vertexPositionData[i * 3 + 2] *= -1.0f;
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;

		if (fvf & DXFVF_NORMAL) {
			_vertexNormalData[i * 3 + 0] = vertexBuffer[i * vertexSize + normalOffset + 0];
			_vertexNormalData[i * 3 + 1] = vertexBuffer[i * vertexSize + normalOffset + 1];
			_vertexNormalData[i * 3 + 2] = vertexBuffer[i * vertexSize + normalOffset + 2];
			for (int j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kNormalOffset + j] = _vertexNormalData[i * 3 + j];
			}
			// mirror z coordinate to change to OpenGL coordinate system
			_vertexNormalData[i * 3 + 2] *= -1.0f;
			_vertexData[i * kVertexComponentCount + kNormalOffset + 2] *= -1.0f;
		}

		if (fvf & DXFVF_DIFFUSE) {
			// nothing
		}

		if (fvf & DXFVF_TEX1) {
			_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = vertexBuffer[i * vertexSize + textureOffset + 0];
			_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = vertexBuffer[i * vertexSize + textureOffset + 1];
		}
	}


	uint32 *indexPtr = (uint32 *)_dxmesh->getIndexBuffer().ptr();
	uint numFaces = _meshObject->_numFaces;
	for (uint i = 0; i < numFaces; ++i) {
		XMeshFace *face = &_meshObject->_faces[i];
		int indexCount = face->_numFaceVertexIndices;
		uint16 index1, index2, index3, index4, index5, index6;
		if (indexCount == 3) {
			index1 = *indexPtr++;
			index2 = *indexPtr++;
			index3 = *indexPtr++;
			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);
		} else {
			index1 = *indexPtr++;
			index2 = *indexPtr++;
			index3 = *indexPtr++;
			index4 = *indexPtr++;
			index5 = *indexPtr++;
			index6 = *indexPtr++;
			_indexData.push_back(index3);
			_indexData.push_back(index2);
			_indexData.push_back(index1);
			_indexData.push_back(index6);
			_indexData.push_back(index5);
			_indexData.push_back(index4);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool XSkinMeshLoader::generateAdjacency(Common::Array<uint32> &adjacency) {
	adjacency = Common::Array<uint32>(_indexData.size(), XSkinMeshLoader::kNullIndex);

	for (uint32 i = 0; i < _indexData.size() / 3; ++i) {
		for (uint32 j = i + 1; j < _indexData.size() / 3; ++j) {
			for (int edge1 = 0; edge1 < 3; ++edge1) {
				uint16 index1 = _indexData[i * 3 + edge1];
				uint16 index2 = _indexData[i * 3 + (edge1 + 1) % 3];

				for (int edge2 = 0; edge2 < 3; ++edge2) {
					uint16 index3 = _indexData[j * 3 + edge2];
					uint16 index4 = _indexData[j * 3 + (edge2 + 1) % 3];

					if (adjacency[i * 3 + edge1] == XSkinMeshLoader::kNullIndex && adjacency[j * 3 + edge2] == XSkinMeshLoader::kNullIndex && adjacentEdge(index1, index2, index3, index4)) {
						adjacency[i * 3 + edge1] = j;
						adjacency[j * 3 + edge2] = i;
						break;
					}
				}
			}
		}
	}

	return true;
}

bool XSkinMeshLoader::adjacentEdge(uint16 index1, uint16 index2, uint16 index3, uint16 index4) {
	Math::Vector3d vertex1(_vertexPositionData + 3 * index1);
	Math::Vector3d vertex2(_vertexPositionData + 3 * index2);
	Math::Vector3d vertex3(_vertexPositionData + 3 * index3);
	Math::Vector3d vertex4(_vertexPositionData + 3 * index4);

	// wme uses a function from the D3DX library, which takes in an epsilon for floating point comparison
	// wme passes in zero, so we just do a direct comparison
	if (vertex1 == vertex3 && vertex2 == vertex4) {
		return true;
	} else if (vertex1 == vertex4 && vertex2 == vertex3) {
		return true;
	}

	return false;
}

} // namespace Wintermute
