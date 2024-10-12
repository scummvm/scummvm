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

XSkinMeshLoader::XSkinMeshLoader(DXMesh *dxmesh) {
	_dxmesh = dxmesh;
	
	_vertexCount = dxmesh->getNumVertices();
	// vertex format for .X meshes will be position + normals + textures
	_vertexData = new float[kVertexComponentCount * _vertexCount]();
	
	auto fvf = _dxmesh->getFVF();
	uint32 vertexSize = DXGetFVFVertexSize(fvf) / sizeof(float);
	float *vertexBuffer = (float *)_dxmesh->getVertexBuffer().ptr();
	uint32 offset = 0, normalOffset = 0, textureOffset = 0;

	if (fvf & DXFVF_XYZ) {
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_NORMAL) {
		normalOffset = offset;
		offset += sizeof(DXVector3) / sizeof(float);
	}
	if (fvf & DXFVF_DIFFUSE) {
		offset += sizeof(DXColorValue) / sizeof(float);
	}
	if (fvf & DXFVF_TEX1) {
		textureOffset = offset;
		offset += sizeof(DXVector2) / sizeof(float);
	}

	for (uint i = 0; i < _vertexCount; ++i) {
		for (int j = 0; j < 3; ++j) {
			_vertexData[i * kVertexComponentCount + kPositionOffset + j] = vertexBuffer[i * vertexSize + j];
		}

		// mirror z coordinate to change to OpenGL coordinate system
		_vertexData[i * kVertexComponentCount + kPositionOffset + 2] *= -1.0f;

		if (fvf & DXFVF_NORMAL) {
			for (int j = 0; j < 3; ++j) {
				_vertexData[i * kVertexComponentCount + kNormalOffset + j] = vertexBuffer[i * vertexSize + normalOffset + j];
			}

			// mirror z coordinate to change to OpenGL coordinate system
			_vertexData[i * kVertexComponentCount + kNormalOffset + 2] *= -1.0f;
		}

		if (fvf & DXFVF_TEX1) {
			_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 0] = vertexBuffer[i * vertexSize + textureOffset + 0];
			_vertexData[i * kVertexComponentCount + kTextureCoordOffset + 1] = vertexBuffer[i * vertexSize + textureOffset + 1];
		}
	}
}

XSkinMeshLoader::~XSkinMeshLoader() {
	delete[] _vertexData;
}

} // namespace Wintermute
