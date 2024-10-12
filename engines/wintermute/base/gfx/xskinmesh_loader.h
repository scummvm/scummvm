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

#ifndef WINTERMUTE_XSKINMESH_LOADER_H
#define WINTERMUTE_XSKINMESH_LOADER_H

#include "engines/wintermute/base/gfx/xmodel.h"

#include "math/matrix4.h"
#include "math/vector3d.h"

namespace Wintermute {

class Material;
class XModel;
class XMesh;
class ShadowVolume;
class SkinMeshHelper;
class VideoTheoraPlayer;
struct XMeshObject;
class DXMesh;
class DXSkinInfo;

struct SkinWeights {
	Common::String _boneName;
	Math::Matrix4 _offsetMatrix;
	BaseArray<uint32> _vertexIndices;
	BaseArray<float> _vertexWeights;
};

class XSkinMeshLoader {
	friend class XMesh;
	friend class XMeshOpenGL;
	friend class XMeshOpenGLShader;
	friend class SkinMeshHelper;

public:
	XSkinMeshLoader(XMesh *mesh, DXMesh *dxmesh);
	virtual ~XSkinMeshLoader();
	void loadMesh(const Common::String &filename, XFileData *xobj);

protected:
	static const int kVertexComponentCount = 8;
	static const int kPositionOffset = 5;
	static const int kTextureCoordOffset = 0;
	static const int kNormalOffset = 2;

protected:

	float *_vertexData;
	float *_vertexPositionData;
	float *_vertexNormalData;
	uint32 _vertexCount;

	BaseArray<Math::Matrix4 *> _boneMatrices;
	BaseArray<SkinWeights> _skinWeightsList;

	XMesh *_mesh;
	DXMesh *_dxmesh;
};

} // namespace Wintermute

#endif
